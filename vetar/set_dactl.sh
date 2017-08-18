#!/bin/tcsh
echo direct access control register status;
cat /sys/class/vetar/vetar0/dactl;
echo writing base address of tlu unit to dactl;
echo;
echo 0x4000100 >>  /sys/class/vetar/vetar0/dactl ;
sleep 1;
echo current settings are;
cat /sys/class/vetar/vetar0/dactl;
