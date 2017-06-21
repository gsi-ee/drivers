# this is the top level Makefile of the mbspex gui package
# JAM 21-Jun-2017 added at last!
#

GUI_FRAME  = framework
GUI_POLAND = poland
GUI_FEBEX = febex
GUI_NYXOR = nyxor
GUI_APFEL = apfel

.PHONY: all clean install  

all: 
	cd $(GUI_FRAME); make
	cd $(GUI_POLAND); make
	cd $(GUI_NYXOR); make
	cd $(GUI_FEBEX); make
	cd $(GUI_APFEL); make
	

install: all
	cd $(GUI_POLAND); make install
	cd $(GUI_NYXOR); make install
	cd $(GUI_FEBEX); make install
	cd $(GUI_APFEL); make install

clean:

	cd $(GUI_POLAND); make clean
	cd $(GUI_NYXOR); make clean
	cd $(GUI_FEBEX); make clean
	cd $(GUI_APFEL); make clean
	cd $(GUI_FRAME); make clean
	


