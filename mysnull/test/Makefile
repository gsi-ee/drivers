#

include ./common.mk

INCDIR += ..
LDINC += $(addprefix -L ,$(LIBDIR))
LDFLAGS += -Wl,-rpath,/usr/lib -lrt
# avoid shared lib for the moment due to more easy testing
#PEXLIB = -Wl,-rpath,/usr/lib -Wl,-rpath,$(LIBDIR) -lmbspex
#PEXLIB = $(LIBDIR)/libmbspex.a

BINARIES = ioctl_test

###############################################################
# Target definitions

.PHONY: exall dirs depend clean

exall: dirs depend $(BINARIES)


# Relate all exec names to it exec in the bin dir
$(BINARIES) : % : $(BINDIR)/% ;

# Target for each exec from the object file
$(BINDIR)/%: $(OBJDIR)/%.o
	@echo -e "LD \t$@"
#	$(Q)$(CXX) $(LDINC) $(LDFLAGS) $(PEXLIB) $(CFLAGS) -o $@ $<
	$(Q)$(CXX) $(LDFLAGS) $(CFLAGS) -o  $@  $< 




# dummy for included common.mk
liball: exall


clean:
	@echo -e "CLEAN \t$(shell pwd)"
	-$(Q)rm -f $(addprefix $(BINDIR)/,$(BINARIES))
	-$(Q)rm -f $(BINDIR)/ioctl_test
	-$(Q)rm -f $(OBJ)
	-$(Q)rm -f $(OBJDIR)/ioctl_test.o 
	-$(Q)rm -f $(DEPEND)
