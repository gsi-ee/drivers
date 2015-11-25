------------------------------------------------------------------------
---------- PEXOR network kernel driver, library and tools
---------- v0.1 25-11-2015 by JAM
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

1) Contents
- the linux device driver pexornet.ko in subfolder driver
- to do: the C user library libpexornet in subfolder library


- examples and tools ins subfolder applications (binaries compiled to bin):
 	- ioctl_test 		: test of ioctl command to network driver


2) Building:
Top level Makefile will	build kernel module, library and all test programs by invoking "make" in top directory. 
Libraries are compiled to folder lib, all binaries appear in bin

3) Installation:
kernel module "load" script is in driver subfolder.

4) Documentation:
// TODO, see the wiki
	
	
	
	