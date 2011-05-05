# this is the top level Makefile of the pexor package
# JAM 1-Mar-2010
#

	PEXORSYS ?= $(PWD)
	export PEXORSYS	


VERSION = 0.992

PEXORMODULE = pexor.ko
PEXORHEADER = pexor_user.h
PEXORLIBINSTALL = libpexor.so


PEXORDRIVER = $(PEXORSYS)/driver
PEXORLIB  = $(PEXORSYS)/user/Library/Pexor
PEXORTEST  = $(PEXORSYS)/user/Tests/pexor
PEXORDABC =  $(PEXORSYS)/user/Dabc/plugins/pexor
PEXORGO4 =  $(PEXORSYS)/user/Go4

PEXORPACKAGE = PexorDriver
PEXORPACKDABC = PexorDABC

PACKDIR = pexor_$(VERSION)
PACKAGEFILES = $(PACKDIR)/driver/ $(PACKDIR)/user/Library/Pexor/ $(PACKDIR)/user/Tests/pexor/ $(PACKDIR)/user/bin/pexorlogin.sh $(PACKDIR)/user/common.mk $(PACKDIR)/Makefile $(PACKDIR)/README.txt
DABCFILES = $(PACKDIR)/user/Dabc/plugins/pexor $(PACKDIR)/user/Go4 $(PACKDIR)/README_DABC.txt



INSTALLDIR ?= /lib/modules/$(shell uname -r)/extra

LIBINSTALLDIR ?= /usr/lib


.PHONY: all clean driver lib examples install uninstall driver-install driver-uninstall lib-install lib-uninstall pack

all: driver lib examples 

install: driver-install lib-install

uninstall: driver-uninstall lib-uninstall


clean:

	cd $(PEXORTEST); make clean
	cd $(PEXORLIB); make clean
	cd $(PEXORDRIVER); make clean
	
clean-dabc:
	cd $(PEXORDABC); make clean
	cd $(PEXORGO4); make clean

driver:
	mkdir -p $(PEXORSYS)/user/include
	cd $(PEXORDRIVER); make all

lib: driver
	mkdir -p $(PEXORSYS)/user/lib
	mkdir -p $(PEXORSYS)/user/depend
	mkdir -p $(PEXORSYS)/user/obj	
	cd $(PEXORLIB); make all
	
examples: lib
	mkdir -p $(PEXORSYS)/user/bin
	cd $(PEXORTEST); make all

driver-install:
	@mkdir -p $(INSTALLDIR)
	@echo "INSTALL $(INSTALLDIR)/$(PEXORMODULE)"
	@install -m 755 $(PEXORDRIVER)/$(PEXORMODULE) $(INSTALLDIR)
	@echo "INSTALL /usr/include/$(PEXORHEADER)"
	@mkdir -p /usr/include/
	@install -m 644 $(PEXORDRIVER)/$(PEXORHEADER) /usr/include/


driver-uninstall:
	@echo "UNINSTALL $(INSTALLDIR)/$(PEXORMODULE)"
	@rm -f $(INSTALLDIR)/$(PEXORMODULE)
	@echo "UNINSTALL /usr/include/$(PEXORHEADER)"
	@rm -rf /usr/include/$(PEXORHEADER)


lib-install:
	@mkdir -p $(LIBINSTALLDIR)
	@echo "INSTALL $(LIBINSTALLDIR)/$(PEXORLIBINSTALL)"
	@install -m 755 $(PEXORSYS)/user/lib/$(PEXORLIBINSTALL) $(LIBINSTALLDIR)
	@echo "INSTALL /usr/include/pexor"
	@mkdir -p /usr/include/pexor
	@install -m 644 $(PEXORSYS)/user/include/pexor/* /usr/include/pexor


lib-uninstall:
	@echo "UNINSTALL $(LIBINSTALLDIR)/$(PEXORLIBINSTALL)"
	@rm -f $(LIBINSTALLDIR)/$(PEXORLIBINSTALL)
	@echo "UNINSTALL /usr/include/pexor"
	@rm -rf /usr/include/pexor



pack: clean clean-dabc
	@mkdir -p $(PACKDIR)
	@cp -rfd  $(PEXORSYS)/driver $(PACKDIR)
	@cp -rfd  $(PEXORSYS)/user $(PACKDIR)
	@cp -rfd  $(PEXORSYS)/Makefile $(PACKDIR)
	@cp -rfd  $(PEXORSYS)/README.txt $(PACKDIR)
	@cp -rfd  $(PEXORSYS)/README_DABC.txt $(PACKDIR)
	@rm -f $(PEXORPACKAGE).*
	@tar chvf $(PEXORPACKAGE).tar  $(PACKAGEFILES) --exclude=".svn" 
	@gzip $(PEXORPACKAGE).tar
	@rm -f $(PEXORPACKDABC).*
	@tar chvf $(PEXORPACKDABC).tar  $(DABCFILES) --exclude=".svn" 
	@gzip $(PEXORPACKDABC).tar	
	@rm -rf $(PACKDIR)

