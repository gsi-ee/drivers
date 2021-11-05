# this is the top level Makefile of the mbsmem package
# JAM 4-Nov-2021
#

	MBSMEMSYS ?= $(PWD)
	export MBSMEMSYS	


VERSION = 0.01

MBSMEMMODULE = mbsmem.ko
#MBSMEMHEADER = mbsmem_user.h
#MBSMEMLIBINSTALL = libmbspex.so


MBSMEMDRIVER = $(MBSMEMSYS)/driver
#MBSMEMLIB  = $(MBSMEMSYS)/library
#MBSMEMTEST  = $(MBSMEMSYS)/tests
#MBSMEMFLASH  = $(MBSMEMSYS)/flash

#MBSMEMPACKAGE = MBSPipeDriver
#
#PACKDIR = pexor_$(VERSION)
#PACKAGEFILES = $(PACKDIR)/driver/ $(PACKDIR)/library $(PACKDIR)/tests $(PACKDIR)/flash $(PACKDIR)/common.mk $(PACKDIR)/Makefile $(PACKDIR)/README.txt



INSTALLDIR ?= /lib/modules/$(shell uname -r)/extra

LIBINSTALLDIR ?= /usr/lib64


#.PHONY: all clean driver lib examples install uninstall driver-install driver-uninstall lib-install lib-uninstall pack
.PHONY: all clean driver install uninstall driver-install driver-uninstall pack

all: driver 
#examples
#lib examples 

install: driver-install 
#lib-install

uninstall: driver-uninstall 
#lib-uninstall


clean:
#	cd $(MBSMEMTEST); make clean
	cd $(MBSMEMDRIVER); make clean

driver:
	cd $(MBSMEMDRIVER); make all

#lib: driver
#	cd $(MBSMEMLIB); make all
	
#examples: 
#lib
#	cd $(MBSMEMTEST); make all
#	cd $(MBSMEMFLASH); make all	

driver-install:
	@mkdir -p $(INSTALLDIR)
	@echo "INSTALL $(INSTALLDIR)/$(MBSMEMMODULE)"
	@install -m 755 $(MBSMEMDRIVER)/$(MBSMEMMODULE) $(INSTALLDIR)
	@echo "INSTALL /usr/include/$(MBSMEMHEADER)"
	@mkdir -p /usr/include/
	@install -m 644 $(MBSMEMDRIVER)/$(MBSMEMHEADER) /usr/include/


driver-uninstall:
	@echo "UNINSTALL $(INSTALLDIR)/$(MBSMEMMODULE)"
	@rm -f $(INSTALLDIR)/$(MBSMEMMODULE)
	@echo "UNINSTALL /usr/include/$(MBSMEMHEADER)"
	@rm -rf /usr/include/$(MBSMEMHEADER)


#lib-install:
#	@mkdir -p $(LIBINSTALLDIR)
#	@echo "INSTALL $(LIBINSTALLDIR)/$(MBSMEMLIBINSTALL)"
#	@install -m 755 $(MBSMEMSYS)/user/lib/$(MBSMEMLIBINSTALL) $(LIBINSTALLDIR)
#	@echo "INSTALL /usr/include/pexor"
#	@mkdir -p /usr/include/pexor
#	@install -m 644 $(MBSMEMSYS)/user/include/pexor/* /usr/include/pexor
#
#
#lib-uninstall:
#	@echo "UNINSTALL $(LIBINSTALLDIR)/$(MBSMEMLIBINSTALL)"
#	@rm -f $(LIBINSTALLDIR)/$(MBSMEMLIBINSTALL)
#	@echo "UNINSTALL /usr/include/pexor"
#	@rm -rf /usr/include/pexor



pack: clean 
	@mkdir -p $(PACKDIR)
	@cp -rfd  $(MBSMEMSYS)/driver $(PACKDIR)
#	@cp -rfd  $(MBSMEMSYS)/tests $(PACKDIR)
#	@cp -rfd  $(MBSMEMSYS)/flash $(PACKDIR)	
	@cp -rfd  $(MBSMEMSYS)/Makefile $(PACKDIR)
	@cp -rfd  $(MBSMEMSYS)/README.txt $(PACKDIR)
#	@cp -rfd  $(MBSMEMSYS)/README_DABC.txt $(PACKDIR)
	@rm -f $(MBSMEMPACKAGE).*
	@tar chvf $(MBSMEMPACKAGE).tar  $(PACKAGEFILES) --exclude=".svn" 
	@gzip $(MBSMEMPACKAGE).tar
	@rm -rf $(PACKDIR)

