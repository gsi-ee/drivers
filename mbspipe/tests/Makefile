#

include ../common.mk

INCDIR += ..
INCDIR +=  /mbs/driv/ifc/VmeDirect_1.00/include
LDINC += $(addprefix -L ,$(LIBDIR))
LDFLAGS += -Wl,-rpath,/usr/lib -lrt
# avoid shared lib for the moment due to more easy testing
#PEXLIB = -Wl,-rpath,/usr/lib -Wl,-rpath,$(LIBDIR) -lmbspex
#PEXLIB = $(LIBDIR)/libpexornet.a

BINARIES = producer consumer producer_ioxos consumer_ioxos

###############################################################
# Target definitions

.PHONY: exall dirs depend clean

exall: dirs depend $(BINARIES)


# Relate all exec names to it exec in the bin dir
$(BINARIES) : % : $(BINDIR)/% ;

# Target for each exec from the object file
#$(BINDIR)/%: $(OBJDIR)/%.o
#	@echo -e "LD \t$@"
##	$(Q)$(CXX) $(LDINC) $(LDFLAGS) $(PEXLIB) $(CFLAGS) -o $@ $<
#	$(Q)$(CXX) $(LDFLAGS) $(CFLAGS) -o  $@  $< $(PEXLIB)

$(BINDIR)/producer: $(OBJDIR)/m_producer.o $(OBJDIR)/timing.o $(OBJDIR)/f_map_pipe.o $(OBJDIR)/pipe_sync.o $(OBJDIR)/f_map_vme.o
	@echo -e "LD \t$@"
	$(Q)$(CC) $(LDINC) $(LDFLAGS) $(CFLAGS) -o $@ $(OBJDIR)/m_producer.o  $(OBJDIR)/timing.o $(OBJDIR)/f_map_pipe.o $(OBJDIR)/pipe_sync.o $(OBJDIR)/f_map_vme.o

$(BINDIR)/consumer: $(OBJDIR)/m_consumer.o $(OBJDIR)/timing.o $(OBJDIR)/f_map_pipe.o $(OBJDIR)/pipe_sync.o $(OBJDIR)/f_map_vme.o 
	@echo -e "LD \t$@"
	$(Q)$(CC) $(LDINC) $(LDFLAGS) $(CFLAGS) -o $@ $(OBJDIR)/m_consumer.o $(OBJDIR)/timing.o $(OBJDIR)/f_map_pipe.o $(OBJDIR)/pipe_sync.o $(OBJDIR)/f_map_vme.o 

## below are alternative tests provided by Jean-Francois Gilot of ioxos:
$(BINDIR)/producer_ioxos: $(OBJDIR)/m_producer_ioxos.o $(OBJDIR)/f_map_pipe.o
	@echo -e "LD \t$@"
	$(Q)$(CC) $(LDINC) $(LDFLAGS) $(CFLAGS) -o $@ $(OBJDIR)/m_producer_ioxos.o $(OBJDIR)/f_map_pipe.o

$(BINDIR)/consumer_ioxos: $(OBJDIR)/m_consumer_ioxos.o $(OBJDIR)/f_map_pipe.o 
	@echo -e "LD \t$@"
	$(Q)$(CC) $(LDINC) $(LDFLAGS) $(CFLAGS) -o $@ $(OBJDIR)/m_consumer_ioxos.o $(OBJDIR)/f_map_pipe.o



# dummy for included common.mk
liball: exall


clean:
	@echo -e "CLEAN \t$(shell pwd)"
	-$(Q)rm -f $(addprefix $(BINDIR)/,$(BINARIES))
	-$(Q)rm -f $(BINDIR)/producer
	-$(Q)rm -f $(BINDIR)/consumer
	-$(Q)rm -f $(OBJ)
	-$(Q)rm -f $(OBJDIR)/*.o 
	-$(Q)rm -f $(DEPEND)
