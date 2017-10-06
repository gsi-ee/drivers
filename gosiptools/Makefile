# N.Kurz, GSI, 5-Feb-2004
# N.Kurz, EE, GSI, 2-Dec-2008
# N.Kurz, EE, GSI, 1-Mar-2013: adopted for debian linux

ifeq ($(GSI_OS),Linux)
ifeq ($(GSI_OS_VERSION),4.9-64)
CFLAGS = -DGSI__LINUX -g -I./ f_smem.o
endif
endif

ifeq ($(GSI_OS),Linux)
ifeq ($(GSI_OS_VERSION),2.6)
CFLAGS = -DGSI__LINUX -I./ f_smem.o
endif
endif

ifeq ($(GSI_OS),Lynx)
ifeq ($(GSI_OS_VERSION),4.0)
CFLAGS =  -I./ f_smem.o
endif
endif


all_exe= pex_bar0_rw mem_test tk_mem_write ini_chane read_modid write write_loop read_loop clear read_reply read_slave write_slave  write_slave_loop   write_slave_loop_loop rx_status fifo write_slave_loop_debug version tx_reset read

all: exe

map: f_smem.o

exe:  map ${all_exe}

f_smem.o: f_smem.c
	gcc -c -o f_smem.o -I./ f_smem.c

read_slave: read_slave.c pexor_gosip.h
	gcc -o read_slave  $(CFLAGS) read_slave.c

write_slave: write_slave.c pexor_gosip.h
	gcc -o write_slave  $(CFLAGS) write_slave.c

write_slave_loop: write_slave_loop.c pexor_gosip.h
	gcc -o write_slave_loop  $(CFLAGS) write_slave_loop.c

write_slave_loop_debug: write_slave_loop_debug.c pexor_gosip.h
	gcc -o write_slave_loop_debug  $(CFLAGS) write_slave_loop_debug.c

write_slave_loop_loop: write_slave_loop_loop.c pexor_gosip.h
	gcc -o write_slave_loop_loop  $(CFLAGS) write_slave_loop_loop.c

mem_test: mem_test.c pexor_gosip.h
	gcc -o mem_test  $(CFLAGS) mem_test.c

tk_mem_write: tk_mem_write.c pexor_gosip.h
	gcc -o tk_mem_write  $(CFLAGS) tk_mem_write.c

ini_chane: ini_chane.c pexor_gosip.h
	gcc -o ini_chane  $(CFLAGS) ini_chane.c 

read_modid: read_modid.c pexor_gosip.h
	gcc -o read_modid  $(CFLAGS) read_modid.c 

write: write.c pexor_gosip.h
	gcc -o write  $(CFLAGS) write.c 

write_loop: write_loop.c pexor_gosip.h
	gcc -o write_loop  $(CFLAGS) write_loop.c 

read_loop: read_loop.c pexor_gosip.h
	gcc -o read_loop  $(CFLAGS) read_loop.c 

clear: clear.c pexor_gosip.h
	gcc -o clear  $(CFLAGS) clear.c 

tx_reset: tx_reset.c pexor_gosip.h
	gcc -o tx_reset  $(CFLAGS) tx_reset.c 

fifo: fifo.c pexor_gosip.h
	gcc -o fifo  $(CFLAGS) fifo.c 

read: read.c pexor_gosip.h
	gcc -o read  $(CFLAGS) read.c 

read_reply: read_reply.c pexor_gosip.h
	gcc -o read_reply  $(CFLAGS) read_reply.c 

rx_status: rx_status.c pexor_gosip.h
	gcc -o rx_status  $(CFLAGS) rx_status.c 

version: version.c pexor_gosip.h
	gcc -o version  $(CFLAGS) version.c 

clean:
	rm -f *~ *.bak *.jou
	rm -f ${all_exe}
	rm -f *.o
