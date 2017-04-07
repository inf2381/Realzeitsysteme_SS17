V2
====

![Ohne RT-Flag und ohne Belastung](https://github.com/GoneUp/resy-ss17-grp4/blob/master/v2/Measurements/PNG/idle.png)

Obwohl die CPU nicht wirklich beansprucht wird, weicht die tatsächliche Schlafenszeit deutlich von der gewünschten ab. Dies liegt vor allem daran, dass wir im Kernel noch zu viele Prozesse laufen, die für uns nicht wichtig sind. Zu bemerken ist vor allem der Peak, der die gewünschte Schlafenszeit um 11 Millionen Nanosekunden überschreitet.

![Ohne RT-Flag und unter Belastung](https://github.com/GoneUp/resy-ss17-grp4/blob/master/v2/Measurements/PNG/busy.png)

Bei längerer Schlafenszeit ist die tastächliche Schlafenszeit exorbitant länger als gewollt und noch schlechter als bei der Messung ohne Beanspruchung der CPU.

![Mit RT-Flag, aber ohne Belastung](https://github.com/GoneUp/resy-ss17-grp4/blob/master/v2/Measurements/PNG/rt_idle.png)

Durch die Priorisierung unseres Prozesses müssen wir die tatsächliche Schlafenszeit nicht mehr in Mikrosekunden umrechnen, um sie überhaupt darstellen zu können. Die maximale Wartezeit ist nun von 11 Millionen auf 50 000 gesunken.

![Mit RT-Flag und unter Belastung](https://github.com/GoneUp/resy-ss17-grp4/blob/master/v2/Measurements/PNG/rt_busy.png)

Der Unterschied zwischen beanspruchter CPU und unbeanspruchter CPU ist bei Priorisierung des Prozesses maximal doppelt so hoch. 
