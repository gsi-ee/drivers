# this is the top level Makefile of the mbspipe package
# JAM 3-Sep-2020
#

	MBSPIPESYS ?= $(PWD)
	export MBSPIPESYS	


VERSION = 0.01

MBSPIPEMODULE = mbspipe.ko
#MBSPIPEHEADER = mbspipe_user.h
#MBSPIPELIBINSTALL = libmbspex.so


MBSPIPEDRIVER = $(MBSPIPESYS)/driver
#MBSPIPELIB  = $(MBSPIPESYS)/library
#MBSPIPETEST  = $(MBSPIPESYS)/tests
#MBSPIPEFLASH  = $(MBSPIPESYS)/flash

#MBSPIPEPACKAGE = MBSPipeDriver
#
#PACKDIR = pexor_$(VERSION)
#PACKAGEFILES = $(PACKDIR)/driver/ $(PACKDIR)/library $(PACKDIR)/tests $(PACKDIR)/flash $(PACKDIR)/common.mk $(PACKDIR)/Makefile $(PACKDIR)/README.txt



INSTALLDIR ?= /lib/modules/$(shell uname -r)/extra

LIBINSTALLDIR ?= /usr/lib64


#.PHONY: all clean driver lib examples install uninstall driver-install driver-uninstall lib-install lib-uninstall pack
.PHONY: all clean driver install uninstall driver-install driver-uninstall pack

all: driver 
#lib examples 

install: driver-install 
#lib-install

uninstall: driver-uninstall 
#lib-uninstall


clean:

	cd $(MBSPIPEDRIVER); make clean

driver:
	cd $(MBSPIPEDRIVER); make all

#lib: driver
#	cd $(MBSPIPELIB); make all
	
#examples: lib
#	cd $(MBSPIPETEST); make all
#	cd $(MBSPIPEFLASH); make all	

driver-install:
	@mkdir -p $(INSTALLDIR)
	@echo "INSTALL $(INSTALLDIR)/$(MBSPIPEMODULE)"
	@install -m 755 $(MBSPIPEDRIVER)/$(MBSPIPEMODULE) $(INSTALLDIR)
	@echo "INSTALL /usr/include/$(MBSPIPEHEADER)"
	@mkdir -p /usr/include/
	@install -m 644 $(MBSPIPEDRIVER)/$(MBSPIPEHEADER) /usr/include/


driver-uninstall:
	@echo "UNINSTALL $(INSTALLDIR)/$(MBSPIPEMODULE)"
	@rm -f $(INSTALLDIR)/$(MBSPIPEMODULE)
	@echo "UNINSTALL /usr/include/$(MBSPIPEHEADER)"
	@rm -rf /usr/include/$(MBSPIPEHEADER)


#lib-install:
#	@mkdir -p $(LIBINSTALLDIR)
#	@echo "INSTALL $(LIBINSTALLDIR)/$(MBSPIPELIBINSTALL)"
#	@install -m 755 $(MBSPIPESYS)/user/lib/$(MBSPIPELIBINSTALL) $(LIBINSTALLDIR)
#	@echo "INSTALL /usr/include/pexor"
#	@mkdir -p /usr/include/pexor
#	@install -m 644 $(MBSPIPESYS)/user/include/pexor/* /usr/include/pexor
#
#
#lib-uninstall:
#	@echo "UNINSTALL $(LIBINSTALLDIR)/$(MBSPIPELIBINSTALL)"
#	@rm -f $(LIBINSTALLDIR)/$(MBSPIPELIBINSTALL)
#	@echo "UNINSTALL /usr/include/pexor"
#	@rm -rf /usr/include/pexor



pack: clean 
	@mkdir -p $(PACKDIR)
	@cp -rfd  $(MBSPIPESYS)/driver $(PACKDIR)
#	@cp -rfd  $(MBSPIPESYS)/tests $(PACKDIR)
#	@cp -rfd  $(MBSPIPESYS)/flash $(PACKDIR)	
	@cp -rfd  $(MBSPIPESYS)/Makefile $(PACKDIR)
	@cp -rfd  $(MBSPIPESYS)/README.txt $(PACKDIR)
#	@cp -rfd  $(MBSPIPESYS)/README_DABC.txt $(PACKDIR)
	@rm -f $(MBSPIPEPACKAGE).*
	@tar chvf $(MBSPIPEPACKAGE).tar  $(PACKAGEFILES) --exclude=".svn" 
	@gzip $(MBSPIPEPACKAGE).tar
	@rm -rf $(PACKDIR)

