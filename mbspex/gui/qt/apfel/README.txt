/-----------------------------------------------------------------------
//       The MBSPEX driver project 
//       Experiment Data Processing at EE department, GSI
//-----------------------------------------------------------------------
// Copyright (C) 2014- GSI Helmholtzzentrum für Schwerionenforschung GmbH
//                     Planckstr. 1, 64291 Darmstadt, Germany
// Contact:            http://daq.gsi.de
/////////////////////////////////////////////////////////////////
This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details (http://www.gnu.org).
/////////////////////////////////////////////////////////////////

---------------------------------------------
// GUI for APFEL/FEBEX evaluation 
// by Joern Adamczewski-Musch, EE, GSI Darmstadt
// JAM (j.adamczewski@gsi.de, 06159-71-1337)
---------------------------------------------

_____________
Requirements: 

-Qt 4.8.2 library and development tools installed on linux system
-GSI mbspex driver library installed at default MBS driver location
	/mbs/driv/mbspex_$(GSI_OS_VERSION)_DEB/

_________
Building:

At GSI MBS Linux nodes (x86l-AB), just type "make" to build executable at local working directory of
mbspex/gui/qt/apfel. For other installations, the Makefile has to be adjusted concerning variables
PEX_LIBPATH and PEX_INCPATH to match the actual location of the mbspex library

___________
Change Log:

 V 0.867 04-November-2016:
 	first working version 

