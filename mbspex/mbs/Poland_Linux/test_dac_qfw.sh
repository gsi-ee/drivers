#set DAC mode to test structure
write_slave 0 0 20002c 2
write_slave 1 0 20002c 2
#write_slave 2 0 20002c 2
#write_slave 3 0 20002c 2
#Start DAC program
write_slave 0 0 200030 1
write_slave 1 0 200030 1
#write_slave 2 0 200030 1
#write_slave 3 0 200030 1
write_slave 0 0 200030 0
write_slave 1 0 200030 0
#write_slave 2 0 200030 0
#write_slave 3 0 200030 0