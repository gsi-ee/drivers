
include ../../common.mk

CFLAGS += -fPIC
CXXFLAGS += -fPIC


LIB ?= $(LIBDIR)/libpexor.so
LIBSTATIC ?= $(LIBDIR)/libpexor.a


LDINC += -L $(LIBDIR) 
CXXFLAGS += $(addprefix -I ,$(INCDIR))

###############################################################
# Target definitions

.PHONY: all depend clean install 

all: depend $(LIB) $(LIBSTATIC) install 


# Target for dynamic  library
$(LIB): $(OBJ)
	@echo -e "LD [.so] \t$@"
	$(Q)$(CXX) -shared $(LDINC) $(LDFLAGS) $(CXXFLAGS) -o $@ $^


# Target for static the library
$(LIBSTATIC): $(OBJ)
	@echo -e "LD [.a] \t$@"
	$(Q)ar -r $(LIBSTATIC) $^
	$(Q)ar -s $(LIBSTATIC)

install:
	@echo "Installing pexor headers ..."
	-$(Q)mkdir -p $(INCDIR)/pexor
	-$(Q)cp $(HDR) $(INCDIR)/pexor


clean:
	@echo -e "CLEAN \t$(shell pwd)"
	-$(Q)rm -f $(LIB)  $(LIBSTATIC)
	-$(Q)rm -f $(OBJ)
	-$(Q)rm -f $(DEPEND)
	-$(Q)rm -rf $(INCDIR)/pexor
