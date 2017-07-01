# Overview

## Hardware
We've used the provided hardware consisting of a Raspberry Pi 3, 4x Infrared sensors, a RFID reader, a front-directed ultrasonic sensor, piezo speaker and a hardware killswitch.

Picture of the final robot:

<img src="http://i.imgur.com/o6h1Qxb.jpg" height="600" width="500">

## Software
Control/Dataflow of the software: [(larger)](https://github.com/GoneUp/resy-ss17-grp4/raw/master/v3/media/Resy%20Diagram.png)
<img src="https://github.com/GoneUp/resy-ss17-grp4/raw/master/v3/media/Resy%20Diagram.png">

### General
The software itself is divided into several logical modules. Mainly we have the "input" side, the processing side and the executing side. To connect the input part with the logic part we have chosen a simple system based on shared data structures. The data is of course protected with a rw-lock to prevent multi-threaded race-conditions. Furthermore, the structure includes an event timestamp to prevent reading of outdated data. 

The logic thread itself reacts based on this input and the `logicmode ` that is set. The `logicmode` signals which sub-logic module is to run. We made states to test each sensor and for the actual competition (`path`, `search`). This mode is exposed for more convenient usability over a command-line option. 

The engine thread is controlled with a shared variable `engineCtrl` by the logic thread, based on that, the motor movement is changed to the desired mode. This could be a simple full-speed or a PWM-based action.
The required kill-switch functionality was implemented over a thread that is constantly polling the GPIO pin connected to the button. If the button is pressed the thread sends a `SIGINT ` signal to his own thread to shutdown the program. We tried to implement a kernel module for the killswitch but did not made it in time.

We have chosen C as the main programming language of this project due to our confidence in C and our lack of knowledge in writing C++ or Rust. Also we could use and extend our good old [**helper.c**](https://github.com/GoneUp/resy-ss17-grp4/blob/master/v3/helper.h) utility that we wrote during BSYS course.

### Sensor-specific
[**Ultrasonic**](https://github.com/GoneUp/resy-ss17-grp4/blob/master/v3/ultrasonic.c) 

The ultrasonic sensor was the most problematic in the system. It had two major shortcomings: the small detection angle and the strange firmware. We read in some forums that there are manufacturing faults that cause occasional breakdowns of the sensor. This leads to problems because sometimes, the sensor doesn't return anything and sometimes, it detects always an obstacle. The former could be solved by introducing timestamps to the measurements. We heard from other groups that another soultion would be to use the Linux `poll` function instead of actively polling. 

[**RFID**](https://github.com/GoneUp/resy-ss17-grp4/blob/master/v3/rfid.c) 

Due to his complicated binary protocol we had to use a external library that did the low-level work for us. The problem was to get this C++ libary into a C project. So our solution was to write a custom [bridging header](https://github.com/GoneUp/resy-ss17-grp4/blob/master/v3/rfid/bridging_header.cpp) and the usage of `g++` for compiling the library and the linking of the program. 

[**Infrared**](https://github.com/GoneUp/resy-ss17-grp4/blob/master/v3/infrared.c) 

Solid, simple sensors. Straight 10/10.

## Kernel Configuration
The Kernel is based on the Linux kernel 4.4.9. The kernel was patched with the "RT Preempt" patch from 
https://www.kernel.org/pub/linux/kernel/projects/rt/4.4/patch-4.4.9-rt17.patch.gz. In order to document the current status of the kernel please find the debug-outputs below.

Output of `command service --status-all`:

| Enable          |Name of deamon|
|:-------------:|:-----------:|
| [ + ] | alsa-utils |
| [ + ] | avahi-daemon |
| [ + ] | bluetooth |
| [ - ] | bootlogs |
| [ - ] | bootmisc.sh |
| [ - ] | checkfs.sh |
| [ - ] | checkroot-bootclean.sh |
| [ - ] | checkroot.sh |
| [ + ] | console-setup |
| [ + ] | cron |
| [ + ] | dbus |
| [ + ] | dhcpcd |
| [ + ] | dphys-swapfile |
| [ + ] | fake-hwclock |
| [ - ] | hostname.sh |
| [ - ] | hwclock.sh |
| [ + ] | kbd |
| [ + ] | keyboard-setup |
| [ - ] | killprocs |
| [ + ] | kmod |
| [ - ] | motd |
| [ - ] | mountall-bootclean.sh |
| [ - ] | mountall.sh |
| [ - ] | mountdevsubfs.sh  |
| [ - ] | mountkernfs.sh |
| [ - ] | mountnfs-bootclean.sh |
| [ - ] | mountnfs.sh |
| [ + ] | networking |
| [ - ] | nfs-common |
| [ + ] | ntp |
| [ - ] | plymouth |
| [ - ] | plymouth-log |
| [ + ] | procps |
| [ + ] | raspi-config |
| [ + ] | rc.local |
| [ - ] | rmnologin |
| [ - ] | rpcbind |
| [ - ] | rsync |
| [ + ] | rsyslog |
| [ - ] | sendsigs |
| [ + ] | ssh |
| [ - ] | sudo |
| [ + ] | triggerhappy |
| [ + ] | udev |
| [ + ] | udev-finish |
| [ - ] | umountfs |
| [ - ] | umountnfs.sh |
| [ - ] | umountroot |
| [ + ] | urandom |
| [ - ] | x11-common |

As we can see not all unused daemons, like *bluetooth* or *cron* are disabled. 

The output of `cat /proc/config.txt`:

arm_freq=1000
dtparam=spi=on
dtparam=audio=on
device_tree_param=spi=on
dtoverlay=spi-bcm2708
dtoverlay=i2c-rtc
dtoverlay=w1-gpio
isolcpus=1,2,3

Output of `top` command:

top - 17:44:01 up  1:51,  2 users,  load average: 0.16, 0.20, 0.18
Tasks: 121 total,   1 running, 120 sleeping,   0 stopped,   0 zombie
%Cpu(s):  0.0 us,  1.3 sy,  0.0 ni, 98.6 id,  0.0 wa,  0.0 hi,  0.0 si,  0.0 st
KiB Mem:    945516 total,    88984 used,   856532 free,    14108 buffers
KiB Swap:   102396 total,        0 used,   102396 free.    40288 cached Mem

| PID USER  | PR  | NI   | VIRT   | RES   | SHR  | S  | %CPU | %MEM | TIME+   | COMMAND          |
|:---------:|:---:|:----:|:------:|:-----:|:----:|:--:|:----:|:----:|:-------:|:----------------:|
| 1926 pi   | 20  |   0  |  5108  | 2432  | 2088 | R  | 11.8 | 0.3  | 0:00.04 | top
| 1 root    | 20  |   0  | 22852  | 3864  | 2736 | S  | 0.0  | 0.4  | 0:02.98 | systemd
| 2 root    | 20  |   0  |     0  |    0  |    0 | S  | 0.0  | 0.0  | 0:00.00 | kthreadd
| 3 root    | 20  |   0  |     0  |    0  |    0 | S  | 0.0  | 0.0  | 0:00.02 | ksoftirqd/0
| 5 root    |  0  | -20  |     0  |    0  |    0 | S  | 0.0  | 0.0  | 0:00.00 | kworker/0:+
| 7 root    | 20  |   0  |     0  |    0  |    0 | S  | 0.0  | 0.0  | 0:00.11 | rcu_sched
| 8 root    | 20  |   0  |     0  |    0  |    0 | S  | 0.0  | 0.0  | 0:00.00 | rcu_bh
| 9 root    | rt  |   0  |     0  |    0  |    0 | S  | 0.0  | 0.0  | 0:00.01 | migration/0
| 10 root   |  0  | -20  |     0  |    0  |    0 | S  | 0.0  | 0.0  | 0:00.00 | lru-add-dr+
| 11 root   | 20  |   0  |     0  |    0  |    0 | S  | 0.0  | 0.0  | 0:00.00 | cpuhp/0
| 12 root   | 20  |   0  |     0  |    0  |    0 | S  | 0.0  | 0.0  | 0:00.00 | cpuhp/1
| 13 root   | rt  |   0  |     0  |    0  |    0 | S  | 0.0  | 0.0  | 0:00.00 | migration/1
| 14 root   | 20  |   0  |     0  |    0  |    0 | S  | 0.0  | 0.0  | 0:00.01 | ksoftirqd/1
| 16 root   |  0  | -20  |     0  |    0  |    0 | S  | 0.0  | 0.0  | 0:00.00 | kworker/1:+
| 17 root   | 20  |   0  |     0  |    0  |    0 | S  | 0.0  | 0.0  | 0:00.00 | cpuhp/2
| 18 root   | rt  |   0  |     0  |    0  |    0 | S  | 0.0  | 0.0  | 0:00.01 | migration/2
| 19 root   | 20  |   0  |     0  |    0  |    0 | S  |  0.0 | 0.0  | 0:00.10 | ksoftirqd/2


As we can see the kernel is not the absolutely minimum configured kernel, but it works well for our task.
During this project, we spent a lot of time in configuring a new kernel from scratch and failed. Our "emergency solution" was to use this arch kernel with a real-time patch. We did not had the time to disable the unused daemons.


# Real-time capability

## Diagrams
On the y-axis is given the time in nanoseconds. Since we did not manage to configure our own kernel and the due date was coming, we decided to eliminate the peaks (values > 90%). Those peaks can be attributed to unintended interrupts from services we don't need. To measure the execution time, all other threads on the core weren't started. When time measuring is enabled, the threads write the measurements to a pre-allocated buffer. When everything is finished, the buffers are written to files.
<img src="https://github.com/GoneUp/resy-ss17-grp4/raw/master/v3/media/log_engine.png"> 

After ~2000 measurements, the robot left the labyrinth and went on to look for rfid tags. Within the labyrinth, we are using pwm to control the engines whereas the rfid search only sets the gpios once per loop. 
<img src="https://github.com/GoneUp/resy-ss17-grp4/raw/master/v3/media/log_logic_only.png"> 

The logic thread controls the engine thread based on the measured values from the sensing thread. The execution time is pretty constant. Lower execution times orginate from turns. While the robot turns, this thread skips the unnecessary calculations. 
<img src="https://github.com/GoneUp/resy-ss17-grp4/raw/master/v3/media/log_ultrasonic_only.png"> 

We don't know how this sensor works. The behavior will remain inexplicable to us. 
<img src="https://github.com/GoneUp/resy-ss17-grp4/raw/master/v3/media/log_infrared_only.png">
<img src="https://github.com/GoneUp/resy-ss17-grp4/raw/master/v3/media/log_killswitch.png"> 

Polling is a constant process. :) 
<img src="https://github.com/GoneUp/resy-ss17-grp4/raw/master/v3/media/log_rfid_only.png"> 

Everytime, the robot detects a rfid tag, such a peak appears. Because it takes some time to leave from the tag, the library is optimized to skip calculations when the tag is known. 

To measure the execution time when the system is stressed, we used the stress script from [CPU burn arm](https://github.com/ssvb/cpuburn-arm/blob/master/cpuburn-a53.S). 

Since we isolated the cores where our program runs, the exection times increase not that much: 
<img src="https://github.com/GoneUp/resy-ss17-grp4/blob/master/v3/media/log_rfid_stress.png">



## Tasksets
In total, our program creates three tasksets. Each taskset is assigned to an isolated core (isolcpus).
Taskset 1 contains one thread which exploits the measurements of the sensors and decides what to do and the killswitch thread. 
Taskset 2 contains only one thread which controls the two engines.
Taskset 3 contains three threads which read out the sensors (Infrared, RFID, Ultrasonic).

## Measurements

| Task          |Taskset| T\_eMin [ms]    | T\_eMax [ms] | T\_pMin [ms] = T\_dMax |
|:-------------:|:-----:|:-------------:| :-----------:| :-----------:|
| Logic compute | 1     | 0.001927      | 5.071824     | 30           |
| Killswitch listener | 1 | 0.033594    | 0.110103     | 1            |
| Engine control | 2    | 0.119791      | 20.324052    | 100          |
| RFID          | 3     | 0.322289      | 13.108615    | 35           |
| Ultrasonic    | 3     | 0.721034      | 10.275833    | 50           |
| Infrared (all 4)| 3 | 0.130103        | 0.233956     | 35           |

### Sufficient scheduling test EDF
Inserting the measured values from taskset 1 into the formula for the necessary real-time condition, we get maximum coefficient of: 0.228. Seeing that the engine control is the only thread on its cpu and the wcet is lower than its deadline, we forego a scheduling test for this set. Taskset 3 delivers a coefficient of 0.5867.
Hence, all tasksets accomplish the sufficient scheduling test and do not require a necessary scheduling test. 
