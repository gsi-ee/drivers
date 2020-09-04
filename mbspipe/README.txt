------------------------------------------------------------------------
---------- MBSPIPE kernel driver
---------- v0.1 03-09-2020 by JAM
------------------------------------------------------------------------
Copyright (C) 2020- Gesellschaft f. Schwerionenforschung, GSI
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
- the linux device driver mbspipe.ko in subfolder driver

2) Building:
Top level Makefile will	build kernel module by invoking "make" in top directory. 

3) Installation:
kernel module "load" script is in driver subfolder.

4) Documentation:
This kernel module is a truncated version of the /dev/mem driver (/kernel/driver/char/mem.c) of the ioxos IPC Linux kernel.

Its only purpose is to mmap physical memory outside the Linux system as MBS "pipe" memory 
(contiguous, DMA capable shared memory between user readout process and MBS collector process).
One can not use /dev/mem directly as regular user without setting capability CAP_SYS_RAWIO.
However, this is not possible for the filesystems used by the MBS installation nodes.


