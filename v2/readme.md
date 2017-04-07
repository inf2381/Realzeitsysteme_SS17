V2
====

![Ohne RT-Flag und ohne Belastung](https://github.com/GoneUp/resy-ss17-grp4/blob/master/v2/Measurements/PNG/idle.png)

Wenn das Betriebssystem nicht beschäftigt ist, weicht die tatsächliche Sleeptime nicht sonderlich von der erwarteten ab. Der Peak kommt daher, dass in unserem Kernel noch zu viele Prozesse laufen, die wir eigentlich nicht benötigen. 

![Ohne RT-Flag und unter Belastung](https://github.com/GoneUp/resy-ss17-grp4/blob/master/v2/Measurements/PNG/busy.png)

Bei längerer Schlafenszeit ist die tastächliche Schlafenszeit exorbitant länger als gewollt.

![Mit RT-Flag, aber ohne Belastung](https://github.com/GoneUp/resy-ss17-grp4/blob/master/v2/Measurements/PNG/rt_idle.png)

![Mit RT-Flag und unter Belastung](https://github.com/GoneUp/resy-ss17-grp4/blob/master/v2/Measurements/PNG/rt_busy.png)

