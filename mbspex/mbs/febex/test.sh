#!/bin/bash

while true
do
sh set_dac_x.sh 0 0 0x10
sleep 5
sh set_dac_x.sh 0 1 0x10
sleep 5
sh set_dac_x.sh 0 0 0xf0
sleep 5
sh set_dac_x.sh 0 1 0xf0
sleep 5
sh set_dac_x.sh 0 0 0x80
sleep 5
sh set_dac_x.sh 0 1 0x80
sleep 5
sh set_dac_x.sh 0 0 0x30
sleep 5
sh set_dac_x.sh 0 1 0x30
sleep 5
done