#include <linux/module.h>
#include <linux/err.h>
#include <linux/rtc.h>
#include <linux/platform_device.h>
#include <linux/proc_fs.h>
#include <linux/random.h>

#define VIRT_RTC_MODE_REAL 1
#define VIRT_RTC_MODE_FAST 2
#define VIRT_RTC_MODE_SLOW 3
#define VIRT_RTC_MODE_RAND 4

static unsigned int virt_rtc_mode;

static struct timespec begin_time;
static unsigned long time = 0;

static unsigned int set_time_cnt = 0;
static unsigned int read_time_cnt = 0;

static struct platform_device *virt_rtc_dev = NULL;

#define PROC_BUF_SIZE 128
static char virt_rtc_proc_buf[PROC_BUF_SIZE];

static inline unsigned long timespec_to_ulong(struct timespec *ts)
{
	return ts->tv_nsec < NSEC_PER_SEC/2 ? ts->tv_sec : ts->tv_sec + 1;
}

static int virt_rtc_read_time(struct device *dev, struct rtc_time *tm)
{
	struct timespec curr_time, diff_time;		
	unsigned short rand_diff = 0;	

	getrawmonotonic(&curr_time);
	diff_time = timespec_sub(curr_time, begin_time);

	switch (virt_rtc_mode)
	{
		case VIRT_RTC_MODE_REAL:
			rtc_time_to_tm(time + diff_time.tv_sec, tm);
			break;

		case VIRT_RTC_MODE_FAST:
			diff_time.tv_sec = diff_time.tv_sec << 1;
			rtc_time_to_tm(time + timespec_to_ulong(&diff_time), tm);
			break;

		case VIRT_RTC_MODE_SLOW:
			diff_time.tv_sec = diff_time.tv_sec >> 1;
			rtc_time_to_tm(time + timespec_to_ulong(&diff_time), tm);
			break;

		case VIRT_RTC_MODE_RAND:
			get_random_bytes(&rand_diff, sizeof(rand_diff));
			rtc_time_to_tm(time + rand_diff, tm);
			break;

		default:
			break;
	}

	read_time_cnt += 1;
	
	return rtc_valid_tm(tm);
}

static int virt_rtc_set_time(struct device *dev, struct rtc_time *tm)
{
	getrawmonotonic(&begin_time);
	rtc_tm_to_time(tm, &time);
	set_time_cnt += 1;
	return 0;
}

static struct rtc_class_ops virt_rtc_ops = {
	.read_time = virt_rtc_read_time,
	.set_time  = virt_rtc_set_time,
};

static int virt_rtc_probe(struct platform_device *plat_dev)
{
	struct rtc_device *rtc;

	virt_rtc_mode = VIRT_RTC_MODE_REAL;
	getrawmonotonic(&begin_time);

	rtc = devm_rtc_device_register(&plat_dev->dev, "virt_rtc", &virt_rtc_ops, THIS_MODULE);
	if (IS_ERR(rtc)) 
	{
		return PTR_ERR(rtc);
	}

	platform_set_drvdata(plat_dev, rtc);

	return 0;
}

static int virt_rtc_remove(struct platform_device *plat_dev)
{
	return 0;
}

static struct platform_driver virt_rtc_driver = {
	.probe	= virt_rtc_probe,
	.remove = virt_rtc_remove,
	.driver = {
		.name = "virt_rtc",
	},
};

static ssize_t virt_rtc_proc_read(struct file *file,
                                  char *buffer, 
                                  size_t len, 
                                  loff_t *offfset)
{
	static int finished = 0;

	if (finished == 0)
	{
		sprintf(buffer, "rtc mode: \t %d\n"
                                "read time cnt: \t %d\n"
                                "set time cnt: \t %d\n",
                                 virt_rtc_mode, read_time_cnt, set_time_cnt);
		finished = 1;	
	
		return strlen(buffer);
	}
	finished = 0;
	return 0;
}

static ssize_t virt_rtc_proc_write(struct file *file,
                                   const char *buffer, 
                                   size_t len, 
                                   loff_t *offfset)
{
	int res;
	unsigned int temp_mode;
	len = (len < PROC_BUF_SIZE) ? len : PROC_BUF_SIZE;
	res = copy_from_user(&virt_rtc_proc_buf, (void *)buffer, len);
	if (res)
	{
		pr_info("copy_from_user_failed");
		return res;
	}

	temp_mode = simple_strtol(virt_rtc_proc_buf, NULL, PROC_BUF_SIZE);

	if ((VIRT_RTC_MODE_REAL <= temp_mode) && 
            (temp_mode <= VIRT_RTC_MODE_RAND))
	{
		virt_rtc_mode = temp_mode;
	}

	return len;
}


static struct file_operations virt_rtc_proc_fops = {
	.owner = THIS_MODULE,
	.read  = virt_rtc_proc_read,
	.write = virt_rtc_proc_write
};

static int __init test_init(void)
{
	int err;
	struct proc_dir_entry *ent;

	if ((err = platform_driver_register(&virt_rtc_driver)))
		return err;

	if ((virt_rtc_dev = platform_device_alloc("virt_rtc", 1)) == NULL) {
		err = -ENOMEM;
		platform_driver_unregister(&virt_rtc_driver);
		return err;
	}

	if ((err = platform_device_add(virt_rtc_dev)))
		platform_device_put(virt_rtc_dev);

	ent = proc_create("driver/virt_rtc", 0666, NULL, &virt_rtc_proc_fops);
	if (!ent)
		printk(KERN_WARNING "virt_rtc: Failed to register with procfs.\n");

	return 0;
}

static void __exit test_exit(void)
{
	platform_device_unregister(virt_rtc_dev);
	platform_driver_unregister(&virt_rtc_driver);
	remove_proc_entry("driver/virt_rtc", NULL);
}

MODULE_AUTHOR("Zakirov Damir");
MODULE_DESCRIPTION("Virtual RTC device/driver");
MODULE_LICENSE("GPL");

module_init(test_init);
module_exit(test_exit);
