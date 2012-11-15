----------------------------------------------------------------
PCI Express Optical Receiver (PEXOR) 
Linux driver and library package
Version 1.01 - 14-Nov-2012 JAM
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
- some example programs: user/Tests/pexor/pexortest_simple.c - uses plain file and ioctl interface
 						 user/Tests/pexor/registertest.c - do elementary register read and write
                         user/Tests/pexor/test_pexor_1.cpp - uses C++ library
                         user/Tests/pexor/sfptest.cpp -      for pexor2/3, uses C++ library
                        
2) How to build 
- Unpack the released tarball: tar zxvf PexorDriver.tar.gz
- change to distribution dir: cd pexor_1.01
- start build: make all

This should compile kernel module, library and all test examples.
The kernel module is at driver/pexor.ko
The library is put into user/lib subfolder.
The exexutables are put to user/bin subfolder.


3) Loading and unloading driver
Script driver/load_pexor.sh is prepared to load the compiled module and create appropriate node in /dev filesystem. 
Please edit the absolute path after the "insmod" statement here to match the actual location of the pexor.ko!
Please note that you need root priviliges to execute the script and unload the driver again!
You may copy the modified load_pexor.sh to somewhere in the PATH of root user.
To load driver call script: ". load_pexor.sh"
To unload just call "rmmod pexor"
As an alternative, driver may be installed to system by "make driver-install" (see below) and
loaded at every boot time.

4) Setup of environment
Script user/bin/pexorlogin.sh is an example how to set environment variables to work with 
pexor library and examples. Please edit the PEXORSYS definition to match the actual location of the pexor_0.95
directory where distribution was compiled. 
Then, to set environment, call ". pexorlogin.sh" (don't miss the leading dot blank!)

5) Runing test programs
Once the PATH and LD_LIBRARY_PATH are set correctly, test programs can be invoked:

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


-test_pexor_1 : Offers almost same functionality as pexortest_simple, but uses C++ library libpexor.so.
Demonstration how the classes of the library are intended to be used. "test_pexor_1 -h" again gives help on
command line parameters.



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


README updated 14-Nov-2012 by JAM
----------------------------------------------------------------
                      