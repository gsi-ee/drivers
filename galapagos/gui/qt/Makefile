# this is the top level Makefile of the galapagos gui package
# JAM 5-Aug-2019
#

GUI_FRAME  = framework
GUI_GAL = galapagos

.PHONY: all clean install  

all: 
	cd $(GUI_FRAME); make
	cd $(GUI_GAL); make


	

install: 
	cd $(GUI_GAL); make install	

clean:
	cd $(GUI_GAL); make clean
	cd $(GUI_FRAME); make clean
	


