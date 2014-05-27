#!/bin/bash
time_sleep=15
#set DAC mode to manual setting
write_slave 0 0 20002c 1

#set DAC Value for Channel 1 to 32 manual
#Settings for POLAND NR:9
#~20-40kHz per Channel for s2
#~ 2-4kHz per Channel for S3
write_slave 0 0 200050 6300
write_slave 0 0 200054 6300
write_slave 0 0 200058 6300
write_slave 0 0 20005c 6300

write_slave 0 0 200060 6300
write_slave 0 0 200064 6300
write_slave 0 0 200068 6300
write_slave 0 0 20006c 6300

write_slave 0 0 200070 6400
write_slave 0 0 200074 6400
write_slave 0 0 200078 6300
write_slave 0 0 20007c 6300

write_slave 0 0 200080 6300
write_slave 0 0 200084 6300
write_slave 0 0 200088 6300
write_slave 0 0 20008c 6300

write_slave 0 0 200090 6300
write_slave 0 0 200094 6300
write_slave 0 0 200098 6300
write_slave 0 0 20009c 6300

write_slave 0 0 2000a0 6300
write_slave 0 0 2000a4 6300
write_slave 0 0 2000a8 6300
write_slave 0 0 2000ac 6300

write_slave 0 0 2000b0 6300
write_slave 0 0 2000b4 6300
write_slave 0 0 2000b8 6300
write_slave 0 0 2000bc 6300

write_slave 0 0 2000c0 6300
write_slave 0 0 2000c4 6300
write_slave 0 0 2000c8 6300
write_slave 0 0 2000cc 6300
#end set DAC Value for Channel 1 to 32 manual

#Start DAC program
write_slave 0 0 200030 1
write_slave 0 0 200030 0

