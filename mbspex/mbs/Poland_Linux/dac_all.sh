
#::::::::::::::::::::::::::::::
#value for all dacs
write_slave 0 0 2000d4 8400
write_slave 1 0 2000d4 8400

#set DAC mode to same all
write_slave 0 0 20002c 4
write_slave 1 0 20002c 4

#Start DAC program
write_slave 0 0 200030 1
write_slave 1 0 200030 1
#write_slave 2 0 200030 1
#write_slave 3 0 200030 1
write_slave 0 0 200030 0
write_slave 1 0 200030 0
#write_slave 2 0 200030 0
#write_slave 3 0 200030 0