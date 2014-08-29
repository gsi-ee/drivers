include $(DABCSYS)/config/Makefile.config

## normally should be like this for every module, but can be specific

ifndef PEXORSYS
PEXORSYS=../..
endif

PEXORPLUGIN = ../pexor

ifdef DABCMAINMAKE
PEXORDIR = plugins/explodertest
else
PEXORDIR = .
endif

PEXOR_EXTRALIBS = 

PEXORDIRI         = $(PEXORDIR)/explodertest
PEXORDIRS         = $(PEXORDIR)/src

PEXOR_LIBNAME     = $(LIB_PREFIX)DabcExploderTest
PEXOR_LIB         = $(TGTDLLPATH)/$(PEXOR_LIBNAME).$(DllSuf)
EXTRA_LIB       = $(EXTRA)/$(PEXOR_LIBNAME).$(DllSuf)


## must be similar for every module

PEXOR_H           = $(wildcard $(PEXORDIRI)/*.$(HedSuf))
PEXOR_S           = $(wildcard $(PEXORDIRS)/*.$(SrcSuf))
PEXOR_O           = $(patsubst %.$(SrcSuf), $(BLD_DIR)/%.$(ObjSuf), $(PEXOR_S))
PEXOR_D           = $(patsubst %.$(SrcSuf), $(BLD_DIR)/%.$(DepSuf), $(PEXOR_S))

# used in the main Makefile

ALLDEPENDENC    += $(PEXOR_D)

libs:: $(PEXOR_LIB) 

##### local rules #####

$(PEXOR_O) $(PEXOR_D) : INCLUDES += $(PEXORSYS)/user/include $(PEXORPLUGIN) . 

$(PEXOR_LIB):  $(PEXOR_O) 
	@$(MakeLib) $(PEXOR_LIBNAME) "$(PEXOR_O)" $(TGTDLLPATH) "-L$(PEXORSYS)/user/lib -lpexor "



install: $(PEXOR_LIB) 
	@cp -f $(TGTDLLPATH)/$(PEXOR_LIBNAME).$(DllSuf) $(PEXORSYS)/user/lib/


ifdef EXTRA

libs:: $(EXTRA_LIB)

$(EXTRA_LIB): $(PEXOR_LIB)
	@echo "Extra dabc pexor lib: $@" 
	@cp -f $< $@	

endif

########### extra rules #############

include $(DABCSYS)/config/Makefile.rules
