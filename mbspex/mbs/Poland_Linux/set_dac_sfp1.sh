#!/bin/bash
time_sleep=15
#set DAC mode to manual setting
write_slave 1 0 20002c 1

#set DAC Value for Channel 1 to 32 manual
#Settings for POLAND NR:9
#~20-40kHz per Channel for s2
#~ 2-4kHz per Channel for S3
write_slave 1 0 200050 7060
write_slave 1 0 200054 7060
write_slave 1 0 200058 7060
write_slave 1 0 20005c 7060

write_slave 1 0 200060 7060
write_slave 1 0 200064 7060
write_slave 1 0 200068 7060
write_slave 1 0 20006c 7060

write_slave 1 0 200070 7060
write_slave 1 0 200074 7060
write_slave 1 0 200078 7060
write_slave 1 0 20007c 7060

write_slave 1 0 200080 7060
write_slave 1 0 200084 7060
write_slave 1 0 200088 7060
write_slave 1 0 20008c 7060

write_slave 1 0 200090 7060
write_slave 1 0 200094 7060
write_slave 1 0 200098 7060
write_slave 1 0 20009c 7060

write_slave 1 0 2000a0 7060
write_slave 1 0 2000a4 7060
write_slave 1 0 2000a8 7060
write_slave 1 0 2000ac 7060

write_slave 1 0 2000b0 7060
write_slave 1 0 2000b4 7060
write_slave 1 0 2000b8 7060
write_slave 1 0 2000bc 7060

write_slave 1 0 2000c0 7060
write_slave 1 0 2000c4 7060
write_slave 1 0 2000c8 7060
write_slave 1 0 2000cc 7060
#end set DAC Value for Channel 1 to 32 manual

#Start DAC program
write_slave 1 0 200030 1
write_slave 1 0 200030 0

