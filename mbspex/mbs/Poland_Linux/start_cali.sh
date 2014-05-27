#for all
write_slave 0 0 200000 0
write_slave 1 0 200000 0
#write_slave 2 0 200000 0
#write_slave 3 0 200000 0

write_slave 0 0 200000 1
write_slave 1 0 200000 1
#write_slave 2 0 200000 1
#write_slave 3 0 200000 1

sh trig_off.sh
#sleep 2
#for all
sh cali_dac_qfw.sh

sh trig_on.sh 