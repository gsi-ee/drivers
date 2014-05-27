#reset & load QFW
#sh s3_ini_qfw.sh
#sh s2_ini_qfw.sh
#sh s13_ini_qfw.sh
#sh s12_ini_qfw.sh


#set DAC mode to calibration
write_slave 0 0 20002c 3
write_slave 1 0 20002c 3

#write dac start value for dacs in cali mode
#s12####wwrite_slave 0 0 2000d0 6500
#s12####wwrite_slave 1 0 2000d0 6c00

#for the moment only for s2 ans s3 possible
#offset Value
#C8 =200

#s12####wwrite_slave 0 0 200034 710
#s12####wwrite_slave 1 0 200034 710

#offset Value + max XXX
#s12####write_slave 0 0 20000c c8
#s12####write_slave 1 0 20000c c8


#write dac start value for dacs in cali mode
write_slave 0 0 2000d0 f500
write_slave 1 0 2000d0 f500

#for the moment only for s2 ans s3 possible
#offset Value
#C8 =200

write_slave 0 0 200034 c8
write_slave 1 0 200034 c8

#offset Value + max XXX
write_slave 0 0 20000c c8
write_slave 1 0 20000c c8

##cali_time = 100ms
write_slave 0 0 200038 4c4b40
write_slave 1 0 200038 4c4b40


#Start DAC program
write_slave 0 0 200030 1
write_slave 1 0 200030 1
#write_slave 2 0 200030 1
#write_slave 3 0 200030 1
write_slave 0 0 200030 0
write_slave 1 0 200030 0
#write_slave 2 0 200030 0
#write_slave 3 0 200030 0