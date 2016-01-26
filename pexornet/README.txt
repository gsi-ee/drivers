------------------------------------------------------------------------
---------- PEXOR network kernel driver, library and tools
---------- v0.5 26-01-2016 by JAM
------------------------------------------------------------------------
Copyright (C) 2015- Gesellschaft f. Schwerionenforschung, GSI
                    Planckstr. 1, 64291 Darmstadt, Germany
Contact:            Joern Adamczewski-Musch (JAM), RBEE-Department, GSI
					j.adamczewski@gsi.de
-----------------------------------------------------------------------
This is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details (http://www.gnu.org).
---------------------------------------------------------------

1) Abstract
The pexornet driver package delivers another approach to communicate gsi's pexor/kinpex 
PCIe readout boards for data acquisition. In contrast to the existing drivers pexor (for dabc/FESA frameworks) and
mbspex (for mbs DAQ framework), the kernel module is based on the network driver interface. Configuration
of front-end modules can still be done via ioctls, but now these will address a socket, not a file handle like for the
character drivers. However, the command line interface gosipcmd and the underlying C-library will make
this difference almost transparent for the user, and they work alike for all 3 variants of the pexor driver.
The actual data read-out is realized by a regular udp socket at the pex0 network device.
This socket will receive the data packets acquired by the pexor hardware as if they would be originated by a virtual
remote data sender node (rempex0). So the regular methods of the Linux network stack (ifconfig, ethtool, netstat,..) may be
applied to tune this data link, and network tools like wireshark may be used to inspect the data packets.
Moreover, the triggered read out of the pexor/trixor system is handled completely within the 
interrupt service routines of the pexornet kernel module. No explicit "wait for trigger" and data request
calls are necessary from the userland code. Instead, the filled udp buffers are just received on a normal socket 
as the kernel delivers them. This promises a performance benefit concerning latency for small data packets, compared 
with the userland "wait-and-request" technique as implemented for mbspex character driver.

 
2) Contents
- the linux device driver pexornet.ko in subfolder driver

- the C user library libpexornet in subfolder library

- the command line tool gosipcmd for libpexornet in subfolder gosipcmd

- examples and tools in subfolder applications (binaries are compiled into bin):
 	- ioctl_test 		: test of ioctl command to network driver
	- readout_test      : test of data readout from udp socket for POLAND frontend example
	
- example of dabc plug-in in subfolder applications/dabc

3) Building:
Top level Makefile will	build kernel module, library and all test programs by invoking "make" in top directory. 
Libraries are compiled to folder lib, all binaries appear in bin. The dabc plug-in requires 
to set the environment for an existing dabc V2 installation (". dabclogin"). The subfolder applications/dabc provides
dedicated dabc Makefile that is not automatically invoked by the top level Makefile!

4) Installation:
Example set up of network configuration is provided by calling skript "preparenetwork" in driver directory as 
user root. This will replace the existing /etc/networks and /etc/hosts by the entries as specified in drivers/etcnetworks
and drivers/etchosts file. This is necessary for the diskless MBS linux systems that do not yet provide such host and network
entries persistently.
The script driver/load will load kernel module and configure network interface pex0, and node names
pexor0 (for the local host) and rempexor0 (for the virtual data sender node) by means of ifconfig tool. 
Script driver/unload will tear down the network interface and unload the driver.
NOTE: all scripts require to be in current directory pexornet/driver.

5) Environment set-up:
To setup PATH and LD_LIBRARY_PATH environment to run the applications, 
a script pexornet/bin/pexornetlogin.sh is provided. Please replace definition of PEXORSYS by the actual
installation location. Then invoke ". bin/pexornetlogin.sh" (as sourced bash script).
For dabc plug-in, additionally the DABC environment is required (". dabclogin" of DABC installation)


6) Applications:

6.1) ioctl_test:
Simple proof of principle how an ioctl can be send to a network kernel module. Type "ioctl_test -h" for a list
of available command options. This is a very simplified version of the gosipcmd tool.
	
6.2) gosipcmd:
A shell command line tool to interactively read and write front-end register values. Type "gosipcmd -h" for
a list of available commands. Syntax and usage is almost the same as the previously existing gosipcmd for 
mbspex and pexor drivers. So all existing set up scripts and GUI frontends will also work with the pexornet
driver. Additionally, some commands were added that allow to control the internal acquisition state of the
kernel module:
gosipcmd -a        : stArt acquisition (pexornet interrupt readout) 
gosipcmd -o        : stOp  acquisition (pexornet interrupt readout) 

6.3) readout_test:
A simple readout example written in C, using the libpexornet library for configuration and a regular udp socket
to receive data. type "readout_test -h" for brief help:
usage: readout_test [-h][-i][-n IF] [numevents]] 
          reads out numevents from POLAND via pex0 socket interface. 
         Options:
                 -h        : display this help
                 -i        : initialize before readout
                 -n IF     : specify interface unit number N (pexN, default:0) 

The example has been tailored to configure the POLAND beam diagnostic frontend prototypes and check the corresponding
data for validity. To adjust it for a special sfp set up, please change the definitions in source code of readout_test.c :
// nr of slaves on SFP 0  1  2  3
//                     |  |  |  |
#define NR_SLAVES     {0, 2, 0, 0}

This is practically the same as it is well known from the  f_user.c code of the mbs DAQ system.
Then recompile by calling "make" in pexornet top directory.
After all requested number of events numevents have been read, the program terminates and displays statistics
about the received data (number of valid and corrupt events, data and event rates).
Keyboard interrupt (Ctrl-C) will also stop acquisition regularly and display this statistics.
Note: to provide more verbose output with some data print outs, please enable definition
//#define VERBOSE 1
in readout_test.c and recompile. (beware for high trigger rates!)



6.4) dabc plug-in:
The dabc plug-in allows receive the data from the udp socket, put it into mbs format
and write it into lmd file, or provide it on an mbs stream server for online monitoring.
Note that dabc V2 installation environment is required (. dabclogin which sets DABCSYS, PATH and
LD_LIBRARY_PATH). 
To compile:
$ cd applications/dabc
$ make install
To run:
$dabc_exe PexornetReadout.xml

Configuration is specified in PexornetReadout.xml, especially mbs and http ports and default file behaviour can 
be adjusted here. Note that frontend configuration and start of acquisiton is not provided by dabc, 
this must be done in advance by means of gosipcmd tool. Additionally, there is also no unpacking
and checking of specific frontend data, as in case of readout_test example. So this dabc plug-in should
work for any kind of pexor/kinpex readout. By means of mbs/lmd format, an existing analysis code like Go4 may easily
verify the acquired data. 
Note that the regular dabc web server (default port http://localhost:8091) offers monitoring and control
of a running acquisition. By dabc commands StartFile/StopFile, and StartServer/StopServer the lmd file or the
stream server socket may be opened and closed on the fly.

-----------------------------------------------------------
7) Changes:

14-Dec-2015 --- v0.3  J.Adamczewski-Musch
 first implementation of gosipcmd for pexornet driver
21-Jan-2016 --- v0.4  J.Adamczewski-Musch
 readout_test is working properly with POLAND frontends	
26-Jan-2016 --- v0.5  J.Adamczewski-Musch
 First version with working dabc read out, tested with POLAND frontends	
	