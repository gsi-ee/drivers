
include ../common.mk

CFLAGS += -fPIC
CXXFLAGS += -fPIC


LIB ?= $(LIBDIR)/libpexornet.so
LIBSTATIC ?= $(LIBDIR)/libpexornet.a


LDINC += -L $(LIBDIR) 
CFLAGS += $(addprefix -I ,$(INCDIR))

###############################################################
# Target definitions

.PHONY: all liball depend clean install 

liball: dirs depend $(LIB) $(LIBSTATIC) install 


# Target for dynamic  library
$(LIB): $(OBJ)
	@echo -e "LD [.so] \t$@"
	$(Q)$(CC) -shared $(LDINC) $(LDFLAGS) $(CFLAGS) -o $@ $^


# Target for static the library
$(LIBSTATIC): $(OBJ)
	@echo -e "LD [.a] \t$@"
	$(Q)ar -r $(LIBSTATIC) $^
	$(Q)ar -s $(LIBSTATIC)

install:
	@echo "Installing libpexornet headers ..."
	-$(Q)mkdir -p $(INCDIR)/pexornet
	-$(Q)cp $(HDR) $(INCDIR)/pexornet


clean:
	@echo -e "CLEAN \t$(shell pwd)"
	-$(Q)rm -f $(LIB)  $(LIBSTATIC)
	-$(Q)rm -f $(OBJ)
	-$(Q)rm -f $(DEPEND)
	-$(Q)rm -rf $(INCDIR)/pexornet
