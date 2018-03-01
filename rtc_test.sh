#!/bin/bash

printf "# Kernel version:\n"
uname -r

printf "# /dev/ before loading:\n"
ls /dev/rtc*

printf "# Loading module...\n"
insmod virt_rtc.ko

printf "# Check if module loaded:\n"
lsmod | grep virt_rtc

printf "# /dev/ after loading:\n"
ls /dev/rtc*

printf "# Read virt_rtc time:\n"
hwclock -r -f /dev/rtc1 --localtime

printf "# Set virt_rtc time with 06/15/2018 12:00:00...\n"
hwclock --set --date "06/15/2018 12:00:00" -f /dev/rtc1 --localtime

printf "# Check virt_rtc time:\n"
hwclock -r -f /dev/rtc1 --localtime

printf "# Show /proc statistics:\n"
cat /proc/driver/virt_rtc

printf "# Check fast mode (waits for 10 seconds and shows time):\n"
echo 2 > /proc/driver/virt_rtc
hwclock -r -f /dev/rtc1 --localtime
sleep 10
hwclock -r -f /dev/rtc1 --localtime

printf "# Check slow mode (waits for 10 seconds and shows time):\n"
echo 3 > /proc/driver/virt_rtc
hwclock -r -f /dev/rtc1 --localtime
sleep 10
hwclock -r -f /dev/rtc1 --localtime

printf "# Check random mode (shows 5 different times):\n"
echo 4 > /proc/driver/virt_rtc
for i in {1..5}
do
	hwclock -r -f /dev/rtc1 --localtime
done

printf "Removing module...\n"
rmmod virt_rtc.ko

sleep 3

