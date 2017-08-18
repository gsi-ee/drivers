#!/bin/tcsh
echo direct access control register status:
cat /sys/class/vetar/vetar0/dactl
echo resetting dactl
echo 0xffffffff>>  /sys/class/vetar/vetar0/dactl
sleep 1
echo current settings are
cat /sys/class/vetar/vetar0/dactl
