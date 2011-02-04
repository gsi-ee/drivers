#

include ../../common.mk

INCDIR += ../../include
LDINC += $(addprefix -L ,$(LIBDIR))
LDFLAGS += -lrt
PEXORLIB = -Wl,-rpath,/usr/lib -Wl,-rpath,$(LIBDIR) -lpexor

BINARIES = pexortest_simple test_pexor_1 sfptest registertest

###############################################################
# Target definitions

.PHONY: all dirs depend clean

all: dirs depend $(BINARIES)


# Relate all exec names to it exec in the bin dir
$(BINARIES) : % : $(BINDIR)/% ;

# Target for each exec from the object file
$(BINDIR)/%: $(OBJDIR)/%.o
	@echo -e "LD \t$@"
	$(Q)$(CXX) $(LDINC) $(LDFLAGS) $(PEXORLIB) $(CXXFLAGS) -o $@ $<

$(BINDIR)/pexortest_simple: $(OBJDIR)/pexortest_simple.o $(OBJDIR)/timing.o
	@echo -e "LD \t$@"
	$(Q)$(CC) $(LDINC) $(LDFLAGS) $(CFLAGS) -o $@ $(OBJDIR)/pexortest_simple.o $(OBJDIR)/timing.o




clean:
	@echo -e "CLEAN \t$(shell pwd)"
	-$(Q)rm -f $(addprefix $(BINDIR)/,$(BINARIES))
	-$(Q)rm -f $(BINDIR)/pexortest_simple
	-$(Q)rm -f $(OBJ)
	-$(Q)rm -f $(OBJDIR)/pexortest_simple.o $(OBJDIR)/timing.o
	-$(Q)rm -f $(DEPEND)
