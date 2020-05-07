#

include ../common.mk

INCDIR += ../include
LDINC += $(addprefix -L ,$(LIBDIR))
LDFLAGS += -Wl,-rpath,/usr/lib -lrt
# avoid shared lib for the moment due to more easy testing
#PEXLIB = -Wl,-rpath,/usr/lib -Wl,-rpath,$(LIBDIR) -lmbspex
PEXLIB = $(LIBDIR)/libmbspex.a

BINARIES = m_load_poland_flash_mbspex m_reset_poland_fpga

###############################################################
# Target definitions

.PHONY: all liball dirs depend clean

all: liball

liball: dirs depend $(BINARIES)




# Relate all exec names to it exec in the bin dir
$(BINARIES) : % : $(BINDIR)/% ;

# Target for each exec from the object file
$(BINDIR)/%: $(OBJDIR)/%.o
	@echo -e "LD \t$@"
#	$(Q)$(CXX) $(LDINC) $(LDFLAGS) $(PEXLIB) $(CFLAGS) -o $@ $<
	$(Q)$(CXX) $(LDFLAGS) $(CFLAGS) -o  $@  $< $(PEXLIB)



clean:
	@echo -e "CLEAN \t$(shell pwd)"
	-$(Q)rm -f $(addprefix $(BINDIR)/,$(BINARIES))
	-$(Q)rm -f $(OBJ)
	-$(Q)rm -f $(DEPEND)
