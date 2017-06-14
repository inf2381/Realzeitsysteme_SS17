#!/bin/bash

if (whoami != root)
    then echo "Please run as root"
    exit
fi

cd /proc/irq/

for d in */; do
    echo 1 > $d"smp_affinity"
done

exit

