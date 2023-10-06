------------------------------------------------------------------------
---------- MBSPEX kernel driver, library and tools
---------- v0.2 03-06-2014 by JAM
---------- v2.0 04-10-2023 by JAM - updated for gosip protocol V5
           v2.1 05-10-2023 by JAM - automatic detection of gosip protocol by fpga version
------------------------------------------------------------------------
Copyright (C) 2014- Gesellschaft f. Schwerionenforschung, GSI
                    Planckstr. 1, 64291 Darmstadt, Germany
Contact:            Joern Adamczewski-Musch (JAM), EEL-Department, GSI
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

1) Contents
- the linux device driver mbspex.ko in subfolder driver
- the C user library libmbspex in subfolder library
- examples and tools ins subfolder tests (binaries compiled to bin):
	- m_test5			: backward compatibility example with basic features required by mbs
 	- gosiptest 		: test of pio write to exploder 1 frontend and token/DMA read back
 	- gosipcmd			: shell command line tool to access frontend registers, see gosipcmd -h for help
	- cmdtest.sh		: example script how to work with gosipcmd
	- testconfig.gos	: example configuration for gosipcmd to test frontend configure/verify mode

- templates of mbs user readout functions using libmbspex in subfolder mbs:
	- streamingDMA: most generic template, first try
	- PolandLinux: readout of beam diagnostic qfw modules (POLAND) tested!
	- febex: generic readout of febex module. contains example how to use locked parallel token read within kernel module
	
- graphical user interfaces in subfolder gui:
    - qt-mainwindow: top level Makefile will build special gosip guis for control of frontends poland, febex, apfel, nyxor, etc.
       for details please see: IEEE TNS Vol.68,Iss:8 (August 2021) doi:10.1109/TNS.2021.3084351
	- qt subfolder contains deprecated old gui!
	
2) Building:
Top level Makefile will	build kernel module, library and all test programs by invoking "make" in top directory. 
Libraries are compiled to folder lib, all binaries appear in bin
The mbs user readout examples are not build by this! They require MBS installation and MBSROOT
environment set and can be compiled with subfolder Makefile.

3) Installation:
kernel module "load" script is in driver subfolder.
Installation of libs and bins to system is not provided yet! May be part of MBS installation later...

4) Documentation:
Developer documentation can be found in internal gsi wiki:
https://wiki.gsi.de/foswiki/bin/view/Daq4FAIR/PexorBoard#Driver_and_library_mbspex_for_standard_MBS_and_external_configuration_tools

	
	
	
	