----------------------------------------------------------------
PCI Express Optical Receiver (PEXOR) 
Linux driver and library package
Version 1.1 - 2-Jul-2014 JAM
----------------------------------------------------------------
Copyright (C) 2011- Gesellschaft f. Schwerionenforschung, GSI
                    Planckstr. 1, 64291 Darmstadt, Germany
Contact:            Joern Adamczewski-Musch, EE-Department, GSI
					j.adamczewski@gsi.de
---------------------------------------------------------------
This is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details (http://www.gnu.org).
---------------------------------------------------------------

1) Contents
- the linux device driver in subfolder driver
- the C++ library to work with the driver in subfolder user/Library/Pexor
- the C library libmbspex (from MBS daq driver) in subfolder user/Library/mbspex
- a shell commandline interface gosipcmd in subfolder user/gosipcmd, uses libmbspex
- some example programs: user/Tests/pexor/pexortest_simple.c - uses plain file and ioctl interface
                          user/Tests/pexor/registertest.c - do elementary register read and write
                         user/Tests/pexor/test_pexor_1.cpp - uses C++ library
                         user/Tests/pexor/sfptest.cpp -      for pexor2/3, uses C++ library
                          user/Tests/pexor/polandtest.cpp -   test of pexor with POLAND/QFW, uses C++ library
                        
2) How to build 
- Unpack the released tarball: tar zxvf PexorDriver.tar.gz
- change to distribution dir: cd pexor_1.01
- start build: make all

This should compile kernel module, library and all test examples.
The kernel module is at driver/pexor.ko
The library is put into user/lib subfolder.
The exexutables are put to user/bin subfolder.
-------------------------------------------------------------------------------------------------------------------
-------------------------------------------------------------------------------------------------------------------
3) Loading and unloading driver
Script driver/load_pexor.sh is prepared to load the compiled module and create appropriate node in /dev filesystem. 
Please edit the absolute path after the "insmod" statement here to match the actual location of the pexor.ko!
Please note that you need root priviliges to execute the script and unload the driver again!
You may copy the modified load_pexor.sh to somewhere in the PATH of root user.
To load driver call script: ". load_pexor.sh"
To unload just call "rmmod pexor"
As an alternative, driver may be installed to system by "make driver-install" (see below) and
loaded at every boot time.
-------------------------------------------------------------------------------------------------------------------
-------------------------------------------------------------------------------------------------------------------
4) Setup of environment
Script user/bin/pexorlogin.sh is an example how to set environment variables to work with 
pexor library and examples. Please edit the PEXORSYS definition to match the actual location of the pexor_0.95
directory where distribution was compiled. 
Then, to set environment, call ". pexorlogin.sh" (don't miss the leading dot blank!)
-------------------------------------------------------------------------------------------------------------------
-------------------------------------------------------------------------------------------------------------------
5) Runing test programs
Once the PATH and LD_LIBRARY_PATH are set correctly, test programs can be invoked:
-------------------------------------------------------------------------------------------------------------------
- registertest: simple utility to read and write values to any address on the board memory (bar 0)

*** PEXOR registertest v0.5 1/2011, JAM GSI:

**** arguments:
         registertest [mode] [address] [value] [Debugmode]
                 mode - read(0) or write(1) [0]
                 address - address on bar 0 [0]
                 value - value to write [0]
                 Debugmode -
                         0:no
                         1:on [0]
**********************************


-------------------------------------------------------------------------------------------------------------------
- pexortest_simple: example with several benchmarks showing how to use the driver with plain C via file
system operations. Call "pexortest_simple -h" to get list of command line arguments:

**** pexortest_simple arguments:
         pexortest_simple [mem] [bufs] [pool] [Debugmode]
                 mem - transferbuffer length (integer len) [1024]
                 bufs - number of buffers for test transfer  [500]
                 poolsize - number of allocated dma buffers [7]
                 Debugmode -
                         0:minimum
                         1:more, check buffers
                         2: also print buf contents
                         3: do IRQ test
                         4: bus io test
                         5: register io test
                         6: triggered io test
                        -1: With copy of DMAbuf->userbuf
                        -2: With dummy copy userbuf->userbuf   [0]
**********************************

By default, a DMA read benchmark with definable buffer parameters and debug output/checking is done. 
More advanced tests will trigger board IRQ (Debugmode=3), and show how to use the bus io (4) and register io (5) 
are invoked by ioctl.

-------------------------------------------------------------------------------------------------------------------
-test_pexor_1 : Offers almost same functionality as pexortest_simple, but uses C++ library libpexor.so.
Demonstration how the classes of the library are intended to be used. "test_pexor_1 -h" again gives help on
command line parameters:

**** test_pexor_1 arguments:
         test_pexor_1 [mem] [bufs] [pool] [Debugmode] 
                 mem - transferbuffer length (integer len) [1024]
                 bufs - number of buffers for test transfer  [500]
                 poolsize - number of allocated dma buffers [7]
                 Debugmode - 
                         0: minimum 
                         1: more, check buffers 
                         2: print buf contents and lib debug output
                         3: do IRQ test 
                         4: bus io test 
                         5: register io test 
                         6: DMA to /dev/mem test
                         7: DMA to pexor mapped physmem test [0] 
**********************************

NEW: added possibility to register dma buffers located in physical memory outside the linux region.
For bigphys mbs pipe testing.



-------------------------------------------------------------------------------------------------------------------
-sfptest : Test pexor2/3 with sfp communication code for exploder/febex as developed at gsi. 
Uses C++ library libpexor.so.

*** sfptest arguments:
         sfptest [Mode] [channel] [slave (token mode: bufid)] [address] [data]
                 Mode -
                         0: i/o looptest over all slaves
                         1: write single address
                         2: read single address
                         3: request token read
                         4: read from address relative to pexor board mem [0]
                 channel - sfp channel [0]
                 slave/bufid - connected device id, or token mode double buffer id  [0]
                 address - address on device [0]
                 data - value to write to address [0]
**********************************

Note that definition WITHTRIGGER in sfptest.cpp source code will enable a triggered token read test.
The number of triggers to process is set in definition MAXTRIGGERS.

-------------------------------------------------------------------------------------------------------------------
-polandtest: Test pexor/kinpex with sfp communiction for POLAND/QFW frontends.
Uses C++ library libpexor.so.


**** polandtest v 0.2 24-Jun-2014 by JAM (j.adamczewski@gsi.de)
* read out data via token DMA from preconfigured poland frontends and unpack/display
Usage:
         polandtest [-i] [-s sfp] [-p slaves ] [-b bufsize] [-n numbufs] [-t numtrigs] [-d level]
Options:
                 -h          : display this help
                 -i          : initialize qfw before test
                 -s sfp      : select sfp chain   (default 0)
                 -p slaves   : number of poland slaves at chain   (default 1)
                 -b bufsize  : size of dma buffer (default 65536 integers)
                 -n numbufs  : number of dma buffers in pool (default 30)
                 -t numtrigs : use triggered readout with number of triggers to read (default polling) 
                 -d level    : verbose output (debug) mode with level 1: print traces, 2: traces+ libpexor debug


-------------------------------------------------------------------------------------------------------------------
-------------------------------------------------------------------------------------------------------------------
-gosipcmd: general shell command line tool for configuration and monitoring.
Features broadcast to all chains/slaves and configuration/verify of slave registers

***************************************************************************
 gosipcmd for mbspex library  
 v0.42 13-Jun-2014 by JAM (j.adamczewski@gsi.de)
***************************************************************************
  usage: gosipcmd [-h|-z] [[-i|-r|-w|-s|-u] [-b] | [-c|-v FILE] [-n DEVICE |-d|-x] sfp slave [address [value [words]|[words]]]] 
         Options:
                 -h        : display this help
                 -z        : reset (zero) pexor/kinpex board 
                 -i        : initialize sfp chain 
                 -r        : read from register 
                 -w        : write to  register
                 -s        : set bits of given mask in  register
                 -u        : unset bits of given mask in  register
                 -b        : broadcast io operations to all slaves in range (0-sfp)(0-slave)
                 -c FILE   : configure registers with values from FILE.gos
                 -v FILE   : verify register contents (compare with FILE.gos)
                 -n DEVICE : specify device number N (/dev/pexorN, default:0) 
                 -d        : debug mode (verbose output) 
                 -x        : numbers in hex format (defaults: decimal, or defined by prefix 0x) 
         Arguments:
                 sfp      - sfp chain- -1 to broadcast all registered chains 
                 slave    - slave id at chain, or total number of slaves. -1 for internal broadcast
                 address  - register on slave 
                 value    - value to write on slave 
                 words    - number of words to read/write/set incrementally
         Examples:
          gosipcmd -z -n 1                   : master gosip reset of board /dev/pexor1 
          gosipcmd -i 0 24                   : initialize chain at sfp 0 with 24 slave devices
          gosipcmd -r -x 1 0 0x1000          : read from sfp 1, slave 0, address 0x1000 and printout value
          gosipcmd -r -x 0 3 0x1000 5        : read from sfp 0, slave 3, address 0x1000 next 5 words
          gosipcmd -r -b  1 3 0x1000 10      : broadcast read from sfp (0..1), slave (0..3), address 0x1000 next 10 words
          gosipcmd -r -- -1 -1 0x1000 10     : broadcast read from address 0x1000, next 10 words from all registered slaves
          gosipcmd -w -x 0 3 0x1000 0x2A     : write value 0x2A to sfp 0, slave 3, address 0x1000
          gosipcmd -w -x 1 0 20000 AB FF     : write value 0xAB to sfp 1, slave 0, to addresses 0x20000-0x200FF
          gosipcmd -w -b  1  3 0x20004c 1    : broadcast write value 1 to address 0x20004c on sfp (0..1) slaves (0..3)
          gosipcmd -w -- -1 -1 0x20004c 1    : write value 1 to address 0x20004c on all registered slaves (internal driver broadcast)
          gosipcmd -s  0 0 0x200000 0x4      : set bit 100 on sfp0, slave 0, address 0x200000
          gosipcmd -u  0 0 0x200000 0x4 0xFF : unset bit 100 on sfp0, slave 0, address 0x200000-0x2000FF
          gosipcmd -x -c run42.gos           : write configuration values from file run42.gos to slaves 


-------------------------------------------------------------------------------------------------------------------
-------------------------------------------------------------------------------------------------------------------
6) Installation to system
(NOT RECOMMENDED during development phase!)
Instead of loading module load_pexor.sh manually and setting LD_LIBRARY_PATH by pexorlogin.sh,
it can be convenient to install kernel module and library to the appropriate system locations.
Makefile offers several targets for this:

make driver-install - install pexor.ko to the system modules directory, and provide pexor_user.h in include path. 
                      Note that you may still have to adjust /etc/modprobe.d.local (or resp. configuration file) 
                      to let the module be loaded on boot. There may be tools like modconf to do this.
                      Call "depmod -a" to add pexor.ko to module dependencies file.
                      You also need to create the /dev/pexor on each boot, as shown in load_pexor.sh.
                      
make driver-uninstall - removes pexor.ko and pexor_user.h from system                      
                      
make lib-install - copies libpexor.so to /usr/lib and pexor headers to /usr/include
                      
make lib-uninstall - removes the above again.

make install - installs module and library

make uninstall - uninstalls all.

NOTE that you need root priviliges for these make targets!


README updated 2-Jul-2014 by JAM
----------------------------------------------------------------
                      