#!/bin/bash 

#wichtig! es muss bereits im anderen Terinal: cat /dev/ttyS0 > power.txt ausgeführt werden


echo "M"$1"?" > /dev/ttyS0

sleep 0.2

sed -e '/^ *$/d' /daq/usr/pwieczor/power.txt > powerneu.txt
tail -n1 powerneu.txt

rm powerneu.txt
