## Описание

Драйвер (модуль ядра) Linux для виртуального устройства класса RTC.

Модуль может:
- эмулировать устройство с неравномерным подсчетом времени (ускоренное/замедленное/случайное);
- поддерживать операции установки/чтения значений;
- иметь возможность настойки параметров и получения статистики через интерфейс /proc;
- иметь возможность динамической загрузки (insmod/rmmod);
- встраиваться в /dev как дополнительное устройство rtcN.

## Использование

### Сборка и запуск теста

Собрать модуль с помощью:
```
make
```
Запустить скрипт, выполняющий загрузку/выгрузку и тестирование модуля:
```
sudo ./rtc_test.sh
```

### Интерфейс /proc
Вывести статистику модуля:
```
cat /proc/driver/virt_rtc
```
Статистика содержит:
rtc mode - режим (реальный/ускоренный/замедленный/случайный);
read time cnt - количество чтений времени;
set time cnt - количество установок времени.

Возможные значения режимов:
1 - реальный;
2 - ускоренный (время идет в ~2 раза быстрее);
3 - замедленный (время идет в ~2 раза медленнее);
4 - случайный.

Пример установки ускоренного режима:
```
echo 2 > /proc/driver/virt_rtc
```
