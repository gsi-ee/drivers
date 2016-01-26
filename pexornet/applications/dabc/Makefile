## normally should be like this for every plugin
## one can extract complete plugin subdirectory and compile it independent from rest

include $(DABCSYS)/config/Makefile.config

#DOOPTIMIZATION = false

ifndef PEXORSYS
PEXORSYS=../..
endif


ifdef DABCMAINMAKE
PEXNETDIR = plugins/pexornet
else
PEXNETDIR = .
INCLUDES += $(PEXNETDIR)
endif


PEXNETDIRI         = $(PEXNETDIR)/pexornet
PEXNETDIRS         = $(PEXNETDIR)/src

# not yet defined in the Makefile.config :)
DABCPEXNET_LIBNAME = $(LIB_PREFIX)DabcPexornet
DABCPEXNET_LIB     = $(TGTDLLPATH)/$(DABCPEXNET_LIBNAME).$(DllSuf)

PEXNET_H           = $(wildcard $(PEXNETDIRI)/*.$(HedSuf))
PEXNET_S           = $(wildcard $(PEXNETDIRS)/*.$(SrcSuf))
PEXNET_O           = $(patsubst %.$(SrcSuf), $(BLD_DIR)/%.$(ObjSuf), $(PEXNET_S))
PEXNET_D           = $(patsubst %.$(SrcSuf), $(BLD_DIR)/%.$(DepSuf), $(PEXNET_S))

PEXNET_CS          = $(wildcard $(PEXNETDIRS)/*.$(CSuf))
PEXNET_CO          = $(patsubst %.$(CSuf), $(BLD_DIR)/%.$(ObjSuf), $(PEXNET_CS))
PEXNET_CD          = $(patsubst %.$(CSuf), $(BLD_DIR)/%.$(DepSuf), $(PEXNET_CS))


DABCPEXNET_O       = $(PEXNET_O) $(PEXNET_CO)
DABCPEXNET_D       = $(PEXNET_D) $(PEXNET_CD)

# used in the main Makefile

ALLHDRS           += $(patsubst $(PEXNETDIR)/%.h, $(DABCINCPATH)/%.h, $(PEXNET_H))
ALLDEPENDENC      += $(DABCPEXNET_D) 

libs:: $(DABCPEXNET_LIB)


clean::


##### local rules #####



$(DABCINCPATH)/%.h: $(PEXNETDIR)/%.h
	@echo "Header: $@" 
	@cp -f $< $@

$(DABCPEXNET_LIB):   $(DABCPEXNET_O) $(DABCMBS_LIB)
	@$(MakeLib) $(DABCPEXNET_LIBNAME) "$(DABCPEXNET_O)" $(TGTDLLPATH) "-lrt -lDabcMbs"


install: $(DABCPEXNET_LIB) 
	@cp -f $(TGTDLLPATH)/$(DABCPEXNET_LIBNAME).$(DllSuf) $(PEXORSYS)/lib/




########### extra rules #############
$(PEXNET_CO) $(PEXNET_CD): C_INCLUDES += $(PEXNETDIRI) 
$(PEXNET_O) $(PEXNET_D) : INCLUDES += $(PEXORSYS)/include  .  

include $(DABCSYS)/config/Makefile.rules
