#!/bin/bash
time_sleep=5
#reset qfw
write_slave 0 0 200000 0
write_slave 0 1 200000 0
#write_slave 2 0 200000 0
write_slave 0 0 200000 1
write_slave 0 1 200000 1
#write_slave 2 0 200000 1
