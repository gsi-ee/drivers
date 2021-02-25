#

include ../common.mk

INCDIR += ../include
#following s to test without mbspex lib:
INCDIR += $(MBSROOT)/inc
CFLAGS += -DLinux -Dunix -DV40 -DGSI_MBS -D$(GSI_CPU_ENDIAN) -D$(GSI_CPU_PLATFORM)
NOBMSPEXFLAG = -DPEX_FAILSAVE
### end no mbspex test


LDINC += $(addprefix -L ,$(LIBDIR))
LDFLAGS += -Wl,-rpath,/usr/lib -lrt
# avoid shared lib for the moment due to more easy testing
#PEXLIB = -Wl,-rpath,/usr/lib -Wl,-rpath,$(LIBDIR) -lmbspex
PEXLIB = $(LIBDIR)/libmbspex.a

BINARIES = flash_poland reset_poland flash_febex3 reset_febex3 flash_tamex reset_tamex flash_febex4 reset_febex4  flash_clock_tdc reset_clock_tdc flash_kilom reset_kilom flash_mppc_rob  reset_mppc_rob  \
flash_poland_failsave reset_poland_failsave flash_febex3_failsave reset_febex3_failsave flash_tamex_failsave reset_tamex_failsave flash_febex4_failsave reset_febex4_failsave  flash_clock_tdc_failsave reset_clock_tdc_failsave flash_kilom_failsave reset_kilom_failsave flash_mppc_rob_failsave  reset_mppc_rob_failsave

###############################################################
# Target definitions

.PHONY: exall dirs depend clean

exall: dirs depend $(BINARIES)


$

$(OBJDIR)/m_reset_poland_fpga_failsave.o: m_reset_poland_fpga.c
	@echo -e "CC \t$<"
	$(Q)$(CC) $(CFLAGS) $(NOBMSPEXFLAG) -c -o $@ $<

$(OBJDIR)/m_reset_febex3tamex_fpga_failsave.o: m_reset_febex3tamex_fpga.c
	@echo -e "CC \t$<"
	$(Q)$(CC) $(CFLAGS) $(NOBMSPEXFLAG) -c -o $@ $<

$(OBJDIR)/m_reset_febex4_fpga_failsave.o: m_reset_febex4_fpga.c
	@echo -e "CC \t$<"
	$(Q)$(CC) $(CFLAGS) $(NOBMSPEXFLAG) -c -o $@ $<
	
$(OBJDIR)/m_reset_clock_tdc_fpga_failsave.o: m_reset_clock_tdc_fpga.c
	@echo -e "CC \t$<"
	$(Q)$(CC) $(CFLAGS) $(NOBMSPEXFLAG) -c -o $@ $<
	
$(OBJDIR)/m_load_poland_flash_failsave.o: m_load_poland_flash.c
	@echo -e "CC \t$<"
	$(Q)$(CC) $(CFLAGS) $(NOBMSPEXFLAG) -c -o $@ $<

$(OBJDIR)/m_load_febex3tamex_flash_failsave.o: m_load_febex3tamex_flash.c
	@echo -e "CC \t$<"
	$(Q)$(CC) $(CFLAGS) $(NOBMSPEXFLAG) -c -o $@ $<

$(OBJDIR)/m_load_febex4_flash_failsave.o: m_load_febex4_flash.c
	@echo -e "CC \t$<"
	$(Q)$(CC) $(CFLAGS) $(NOBMSPEXFLAG) -c -o $@ $<
	
$(OBJDIR)/m_load_clock_tdc_flash_failsave.o: m_load_clock_tdc_flash.c
	@echo -e "CC \t$<"
	$(Q)$(CC) $(CFLAGS) $(NOBMSPEXFLAG) -c -o $@ $<	
	
	


# Relate all exec names to it exec in the bin dir
$(BINARIES) : % : $(BINDIR)/% ;

# Target for each exec from the object file
$(BINDIR)/%: $(OBJDIR)/%.o
	@echo -e "LD \t$@"
#	$(Q)$(CXX) $(LDINC) $(LDFLAGS) $(PEXLIB) $(CFLAGS) -o $@ $<
	$(Q)$(CXX) $(LDFLAGS) $(CFLAGS) -o  $@  $< $(PEXLIB)






$(BINDIR)/flash_poland: $(OBJDIR)/m_load_poland_flash.o 
	@echo -e "LD \t$@"
	$(Q)$(CC) $(LDFLAGS) $(CFLAGS)  -o $@ $(OBJDIR)/m_load_poland_flash.o  $(PEXLIB) 
	
$(BINDIR)/flash_poland_failsave: $(OBJDIR)/m_load_poland_flash_failsave.o 
	@echo -e "LD \t$@"
	$(Q)$(CC) $(LDFLAGS) $(CFLAGS)  -o $@ $(OBJDIR)/m_load_poland_flash_failsave.o  	
	

$(BINDIR)/reset_poland: $(OBJDIR)/m_reset_poland_fpga.o 
	@echo -e "LD \t$@"
	$(Q)$(CC) $(LDFLAGS) $(CFLAGS)  -o $@ $(OBJDIR)/m_reset_poland_fpga.o  $(PEXLIB) 
	
$(BINDIR)/reset_poland_failsave: $(OBJDIR)/m_reset_poland_fpga_failsave.o 
	@echo -e "LD \t$@"
	$(Q)$(CC) $(LDFLAGS) $(CFLAGS)  -o $@ $(OBJDIR)/m_reset_poland_fpga_failsave.o 	
	
	

$(BINDIR)/flash_febex3: $(OBJDIR)/m_load_febex3tamex_flash.o 
	@echo -e "LD \t$@"
	$(Q)$(CC) $(LDFLAGS) $(CFLAGS)  -o $@ $(OBJDIR)/m_load_febex3tamex_flash.o  $(PEXLIB)
	
$(BINDIR)/flash_febex3_failsave: $(OBJDIR)/m_load_febex3tamex_flash_failsave.o 
	@echo -e "LD \t$@"
	$(Q)$(CC) $(LDFLAGS) $(CFLAGS)  -o $@ $(OBJDIR)/m_load_febex3tamex_flash_failsave.o
	 

$(BINDIR)/reset_febex3: $(OBJDIR)/m_reset_febex3tamex_fpga.o 
	@echo -e "LD \t$@"
	$(Q)$(CC) $(LDFLAGS) $(CFLAGS)  -o $@ $(OBJDIR)/m_reset_febex3tamex_fpga.o  $(PEXLIB) 	

$(BINDIR)/reset_febex3_failsave: $(OBJDIR)/m_reset_febex3tamex_fpga_failsave.o 
	@echo -e "LD \t$@"
	$(Q)$(CC) $(LDFLAGS) $(CFLAGS)  -o $@ $(OBJDIR)/m_reset_febex3tamex_fpga_failsave.o  $(PEXLIB) 	


$(BINDIR)/flash_tamex: $(OBJDIR)/m_load_febex3tamex_flash.o 
	@echo -e "LD \t$@"
	$(Q)$(CC) $(LDFLAGS) $(CFLAGS)  -o $@ $(OBJDIR)/m_load_febex3tamex_flash.o  $(PEXLIB) 
	
$(BINDIR)/flash_tamex_failsave: $(OBJDIR)/m_load_febex3tamex_flash_failsave.o 
	@echo -e "LD \t$@"
	$(Q)$(CC) $(LDFLAGS) $(CFLAGS)  -o $@ $(OBJDIR)/m_load_febex3tamex_flash_failsave.o 	
	
	
$(BINDIR)/reset_tamex: $(OBJDIR)/m_reset_febex3tamex_fpga.o 
	@echo -e "LD \t$@"
	$(Q)$(CC) $(LDFLAGS) $(CFLAGS)  -o $@ $(OBJDIR)/m_reset_febex3tamex_fpga.o  $(PEXLIB) 	
	
$(BINDIR)/reset_tamex_failsave: $(OBJDIR)/m_reset_febex3tamex_fpga_failsave.o 
	@echo -e "LD \t$@"
	$(Q)$(CC) $(LDFLAGS) $(CFLAGS)  -o $@ $(OBJDIR)/m_reset_febex3tamex_fpga_failsave.o
		

$(BINDIR)/flash_febex4: $(OBJDIR)/m_load_febex4_flash.o 
	@echo -e "LD \t$@"
	$(Q)$(CC) $(LDFLAGS) $(CFLAGS)  -o $@ $(OBJDIR)/m_load_febex4_flash.o  $(PEXLIB) 
	
$(BINDIR)/flash_febex4_failsave: $(OBJDIR)/m_load_febex4_flash_failsave.o 
	@echo -e "LD \t$@"
	$(Q)$(CC) $(LDFLAGS) $(CFLAGS)  -o $@ $(OBJDIR)/m_load_febex4_flash_failsave.o
		

$(BINDIR)/reset_febex4: $(OBJDIR)/m_reset_febex4_fpga.o 
	@echo -e "LD \t$@"
	$(Q)$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $(OBJDIR)/m_reset_febex4_fpga.o  $(PEXLIB) 	

$(BINDIR)/reset_febex4_failsave: $(OBJDIR)/m_reset_febex4_fpga_failsave.o 
	@echo -e "LD \t$@"
	$(Q)$(CC) $(LDFLAGS) $(CFLAGS)  -o $@ $(OBJDIR)/m_reset_febex4_fpga_failsave.o

$(BINDIR)/flash_clock_tdc: $(OBJDIR)/m_load_clock_tdc_flash.o 
	@echo -e "LD \t$@"
	$(Q)$(CC) $(LDFLAGS) $(CFLAGS)  -o $@ $(OBJDIR)/m_load_clock_tdc_flash.o  $(PEXLIB) 
	
$(BINDIR)/flash_clock_tdc_failsave: $(OBJDIR)/m_load_clock_tdc_flash_failsave.o 
	@echo -e "LD \t$@"
	$(Q)$(CC) $(LDFLAGS) $(CFLAGS)  -o $@ $(OBJDIR)/m_load_clock_tdc_flash_failsave.o   	

$(BINDIR)/reset_clock_tdc: $(OBJDIR)/m_reset_clock_tdc_fpga.o 
	@echo -e "LD \t$@"
	$(Q)$(CC) $(LDFLAGS) $(CFLAGS)  -o $@ $(OBJDIR)/m_reset_clock_tdc_fpga.o  $(PEXLIB) 
	
$(BINDIR)/reset_clock_tdc_failsave: $(OBJDIR)/m_reset_clock_tdc_fpga_failsave.o 
	@echo -e "LD \t$@"
	$(Q)$(CC) $(LDFLAGS) $(CFLAGS)  -o $@ $(OBJDIR)/m_reset_clock_tdc_fpga_failsave.o   	

$(BINDIR)/flash_kilom: $(OBJDIR)/m_load_clock_tdc_flash.o 
	@echo -e "LD \t$@"
	$(Q)$(CC) $(LDFLAGS) $(CFLAGS)  -o $@ $(OBJDIR)/m_load_clock_tdc_flash.o  $(PEXLIB) 
	
$(BINDIR)/flash_kilom_failsave: $(OBJDIR)/m_load_clock_tdc_flash_failsave.o 
	@echo -e "LD \t$@"
	$(Q)$(CC) $(LDFLAGS) $(CFLAGS)  -o $@ $(OBJDIR)/m_load_clock_tdc_flash_failsave.o   	

$(BINDIR)/reset_kilom: $(OBJDIR)/m_reset_clock_tdc_fpga.o 
	@echo -e "LD \t$@"
	$(Q)$(CC) $(LDFLAGS) $(CFLAGS)  -o $@ $(OBJDIR)/m_reset_clock_tdc_fpga.o  $(PEXLIB) 
	
$(BINDIR)/reset_kilom_failsave: $(OBJDIR)/m_reset_clock_tdc_fpga_failsave.o 
	@echo -e "LD \t$@"
	$(Q)$(CC) $(LDFLAGS) $(CFLAGS)  -o $@ $(OBJDIR)/m_reset_clock_tdc_fpga_failsave.o   	
	

$(BINDIR)/flash_mppc_rob: $(OBJDIR)/m_load_clock_tdc_flash.o 
	@echo -e "LD \t$@"
	$(Q)$(CC) $(LDFLAGS) $(CFLAGS)  -o $@ $(OBJDIR)/m_load_clock_tdc_flash.o  $(PEXLIB)
	
$(BINDIR)/flash_mppc_rob_failsave: $(OBJDIR)/m_load_clock_tdc_flash_failsave.o 
	@echo -e "LD \t$@"
	$(Q)$(CC) $(LDFLAGS) $(CFLAGS)  -o $@ $(OBJDIR)/m_load_clock_tdc_flash_failsave.o	
	 

$(BINDIR)/reset_mppc_rob: $(OBJDIR)/m_reset_clock_tdc_fpga.o 
	@echo -e "LD \t$@"
	$(Q)$(CC) $(LDFLAGS) $(CFLAGS)  -o $@ $(OBJDIR)/m_reset_clock_tdc_fpga.o  $(PEXLIB) 

$(BINDIR)/reset_mppc_rob_failsave: $(OBJDIR)/m_reset_clock_tdc_fpga_failsave.o 
	@echo -e "LD \t$@"
	$(Q)$(CC) $(LDFLAGS) $(CFLAGS)  -o $@ $(OBJDIR)/m_reset_clock_tdc_fpga_failsave.o   



# dummy for included common.mk
liball: exall


clean:
	@echo -e "CLEAN \t$(shell pwd)"
	-$(Q)rm -f $(addprefix $(BINDIR)/,$(BINARIES))
#	-$(Q)rm -f $(BINDIR)/m_test5
	-$(Q)rm -f $(OBJ)
#	-$(Q)rm -f $(OBJDIR)/gosiptest.o $(OBJDIR)/timing.o
	-$(Q)rm -f $(OBJDIR)/*failsave*
	-$(Q)rm -f $(DEPEND)
