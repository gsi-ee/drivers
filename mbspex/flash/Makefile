#

include ../common.mk

INCDIR += ../include
LDINC += $(addprefix -L ,$(LIBDIR))
LDFLAGS += -Wl,-rpath,/usr/lib -lrt
# avoid shared lib for the moment due to more easy testing
#PEXLIB = -Wl,-rpath,/usr/lib -Wl,-rpath,$(LIBDIR) -lmbspex
PEXLIB = $(LIBDIR)/libmbspex.a

BINARIES = flash_poland reset_poland

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
	$(Q)$(CXX) $(LDFLAGS) $(CFLAGS) -o  $@  $< $(PEXLIB)

#$(BINDIR)/gosiptest: $(OBJDIR)/gosiptest.o $(OBJDIR)/timing.o
#	@echo -e "LD \t$@"
#	$(Q)$(CC) $(LDFLAGS) $(CFLAGS)  -o $@ $(OBJDIR)/gosiptest.o $(OBJDIR)/timing.o $(PEXLIB)
##	$(Q)$(CC) $(LDINC) $(LDFLAGS) $(PEXLIB) $(CFLAGS) -o $@ $(OBJDIR)/gosiptest.o $(OBJDIR)/timing.o

$(BINDIR)/flash_poland: $(OBJDIR)/m_load_poland_flash.o 
	@echo -e "LD \t$@"
	$(Q)$(CC) $(LDFLAGS) $(CFLAGS)  -o $@ $(OBJDIR)/m_load_poland_flash.o  $(PEXLIB) 

$(BINDIR)/reset_poland: $(OBJDIR)/m_reset_poland_fpga.o 
	@echo -e "LD \t$@"
	$(Q)$(CC) $(LDFLAGS) $(CFLAGS)  -o $@ $(OBJDIR)/m_reset_poland_fpga.o  $(PEXLIB) 


# dummy for included common.mk
liball: exall


clean:
	@echo -e "CLEAN \t$(shell pwd)"
	-$(Q)rm -f $(addprefix $(BINDIR)/,$(BINARIES))
#	-$(Q)rm -f $(BINDIR)/m_test5
	-$(Q)rm -f $(OBJ)
#	-$(Q)rm -f $(OBJDIR)/gosiptest.o $(OBJDIR)/timing.o
	-$(Q)rm -f $(DEPEND)