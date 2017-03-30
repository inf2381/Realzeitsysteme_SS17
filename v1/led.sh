#!/bin/bash
#set -o xtrace

PIN=18
BASEPATH=/sys/class/gpio
PINPATH=$BASEPATH/gpio$PIN

echo "1Hz Ledkram"
echo "$PIN" > $BASEPATH/export
echo "out" > $PINPATH/direction

trap "echo \"$PIN\" > $BASEPATH/unexport" EXIT

VAL=`cat $PINPATH/value`
echo $VAL

while true; do
	if [[ "$VAL" == "0" ]]; then
		VAL=1
	else
		VAL=0
	fi
	
	echo $VAL > $PINPATH/value
	sleep 1
done

