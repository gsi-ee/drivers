#!/bin/bash
time_sleep=5
#for all
sh trig_off.sh

#for all
#sh test_dac_qfw.sh

#for single Board
sh set_dac_sfp0.sh
#sleep 2
sh set_dac_sfp1.sh
#sleep 2
#sleep 2
#for all
sh loop_time_ini.sh
#sh s2_ini_qfw.sh
sh s3_ini_qfw.sh
sh trig_on.sh 