#!/bin/bash
time_sleep=5
#reset qfw
write_slave 0 0 200010 0
write_slave 1 0 200010 0
#write_slave 2 0 200010 0
#write_slave 3 0 200010 0
write_slave 0 0 200010 1
write_slave 1 0 200010 1
#write_slave 2 0 200010 1
#write_slave 3 0 200010 1
#QFW mode 3
write_slave 0 0 200004 3
write_slave 1 0 200004 3
#write_slave 2 0 200004 3
#write_slave 3 0 200004 3
#program qfw
write_slave 0 0 200008 1
write_slave 1 0 200008 1
#write_slave 2 0 200008 1
#write_slave 3 0 200008 1
write_slave 0 0 200008 0
write_slave 1 0 200008 0
#write_slave 2 0 200008 0
#write_slave 3 0 200008 0
#QFW mode 2
write_slave 0 0 200004 2
write_slave 1 0 200004 2
#write_slave 2 0 200004 2
#write_slave 3 0 200004 2
#program qfw
write_slave 0 0 200008 1
write_slave 1 0 200008 1
#write_slave 2 0 200008 1
#write_slave 3 0 200008 1
write_slave 0 0 200008 0
write_slave 1 0 200008 0
#write_slave 2 0 200008 0
#write_slave 3 0 200008 0