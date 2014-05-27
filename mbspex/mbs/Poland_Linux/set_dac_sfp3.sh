#!/bin/bash
time_sleep=15
#set DAC mode to manual setting
write_slave 3 0 20002c 1

#set DAC Value for Channel 1 to 32 manual
#Settings for POLAND NR:9
#~20-40kHz per Channel for s2
#~ 2-4kHz per Channel for S3
write_slave 3 0 200050 7060
write_slave 3 0 200054 7050
write_slave 3 0 200058 7040
write_slave 3 0 20005c 7020

write_slave 3 0 200060 7045
write_slave 3 0 200064 7015
write_slave 3 0 200068 7100
write_slave 3 0 20006c 7060

write_slave 3 0 200070 7000
write_slave 3 0 200074 7060
write_slave 3 0 200078 7000
write_slave 3 0 20007c 7000

write_slave 3 0 200080 7000
write_slave 3 0 200084 7000
write_slave 3 0 200088 7050
write_slave 3 0 20008c 7030

write_slave 3 0 200090 7120
write_slave 3 0 200094 7045
write_slave 3 0 200098 7085
write_slave 3 0 20009c 7050

write_slave 3 0 2000a0 6fe0
write_slave 3 0 2000a4 6fe9
write_slave 3 0 2000a8 6fe9
write_slave 3 0 2000ac 6fe0

write_slave 3 0 2000b0 7088
write_slave 3 0 2000b4 7085
write_slave 3 0 2000b8 7022
write_slave 3 0 2000bc 7065

write_slave 3 0 2000c0 6f8f
write_slave 3 0 2000c4 7000
write_slave 3 0 2000c8 6f8f
write_slave 3 0 2000cc 7010
#end set DAC Value for Channel 1 to 32 manual

#Start DAC program
write_slave 3 0 200030 1
write_slave 3 0 200030 0

