----------------------------------------------------------------
PCI Express Optical Receiver (PEXOR) 
Data Acquisition Backbone Core Plug-in with Go4 monitor
Version 0.97 - 03-Aug-2010 JAM
----------------------------------------------------------------
Copyright (C) 2010- Gesellschaft f. Schwerionenforschung, GSI
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
- the DABC plugin in subfolder user/Dabc/plugins/pexor
- the Go4 online analysis for pexor test in subfolder user/Go4

2) Requirements
- the pexor kernel driver pexor.ko and the application library libpexor.so
as distributed in package PexorDriver.tar.gz!
- the installed DABC data acquisition framework as available at http://dabc.gsi.de
- the installed Go4 data analysis and online monitoring framework as available at http://go4.gsi.de. 
   This also requires installed ROOT and Qt libraries. 

3) How to build 
- Unpack the released tarball: tar zxvf PexorDABC.tar.gz
- for DABC plugin:
     - set up DABCSYS environment to your DABC installation directory
     - change to DABC plugin dir: cd pexor_0.96/Dabc/plugins/pexor
	 - start build: make all
- for Go4 monitoring analysis:
     - set up GO4SYS and ROOTSYS environment variables
     - change to Go4 analysis dir: cd pexor_0.95/Go4
     - Important NOTE: be sure that the set up of the readout slave numbers for the sfps matches the
	   array definitions in TPexorMonProc.cxx in variable TPexorMonProc::fNumSlavesSFP[]={0,2,0,0}
	   (these default settings expect 2 slaves at sfp 1 and none at all other channels). You may have to edit this
	   depending on your set up!
	 - start build: make all
	

4) Running the DABC plugin
The DABCSYS environment  must be set to the DABC installation location.
 The configuration file PexorReadout.xml should be in the current directory. Please be sure that
 configuration parameters match the set up (Number of readout slaves for each sfp, buffer sizes. etc.)
 
 To run simple data server plugin, type

dabc_run PexorReadout.xml

This will initialize chain of readout modules, write dummy test data to frontend memories and do a continuous
token readout (synchronous polling mode). The data buffers are packed into Mbs (Sub-)events and provided at
a stream server socket, and/or written to a lmd file (if PexorOutFile value is specified).
For more information on DABC configuration please see DABC user manual.

5) Running the Go4 monitor analysis
The GO4SYS, ROOTSYS, and QTDIR environments must be set correctly for the respective libraries.
Run the dabc data server as described in 4) in different shell.
Start GUI by "go4" and launch analysis client using the compiled libGo4UserAnalysis.so from this distribution.
Set Event source "Mbs stream" to the dabc data server at correct node name, and submit settings.
Run the analysis. The corresponding spectra should be filled with random data from the slave submemories.
NEW: optionally can produce root tree with output event containing the submemory data


README updated 03-Aug-2010 by JAM
----------------------------------------------------------------

