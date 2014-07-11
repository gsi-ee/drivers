# N.Kurz, DVEE, GSI, 16-Aug-2001
# N.Kurz, EE,   GSI,  3-MAR-2009: adopted for linux and dim 

ifdef DIMDIR
DIMANN = DIM $(DIMDIR)
DIMCTRL = -DMBS_DIM
DIMINC = -I$(DIMDIR)/dim
 ifeq ($(GSI_OS),Lynx)
 DIMLIB  = $(DIMDIR)/lib_$(GSI_CPU_PLATFORM)/libdim.a
 endif
 ifeq ($(GSI_OS),Linux)
 DIMLIB  = $(DIMDIR)/lib_$(GSI_CPU_PLATFORM)_$(GSI_OS)_$(GSI_OS_VERSION)_$(GSI_OS_TYPE)/libdim.a
 endif
endif

INC = $(MBSROOT)/inc
ifeq ($(GSI_OS),Lynx)
OBJ     = $(MBSROOT)/obj_$(GSI_CPU_PLATFORM)
LIB     = $(MBSROOT)/lib_$(GSI_CPU_PLATFORM)
endif
ifeq ($(GSI_OS),Linux)
OBJ     = $(MBSROOT)/obj_$(GSI_CPU_PLATFORM)_$(GSI_OS)_$(GSI_OS_VERSION)_$(GSI_OS_TYPE)
LIB     = $(MBSROOT)/lib_$(GSI_CPU_PLATFORM)_$(GSI_OS)_$(GSI_OS_VERSION)_$(GSI_OS_TYPE)
endif

SMILIBS      = $(LIB)/lib_mbs.a $(LIB)/lib_tools.a $(LIB)/lib_mbs.a $(LIB)/lib_smi.a
CES_LIB = -L/lib/ces -lvme -lbma -luio 

SHMQ    = -DLYNX__SMEM
ifeq ($(GSI_OS_VERSION),2.5)
POSIX   =  -mthreads
CFLAGS  = $(POSIX) -DLynx -DGSI_MBS -D$(GSI_CPU_ENDIAN) -D$(GSI_CPU_PLATFORM)
LIBS    = $(LIB)/lib_mbs.a $(LIB)/lib_tools.a $(LIB)/lib_mbs.a $(DIMLIB) \
          $(LIB)/libedit.a -llynx -lnetinet $(CES_LIB)
endif

ifeq ($(GSI_OS_VERSION),3.0)
POSIX   =  -mthreads
CFLAGS  = $(POSIX) -D_THREADS_POSIX4ad4 -DLynx -DGSI_MBS -D$(GSI_CPU_ENDIAN) -D$(GSI_CPU_PLATFORM) 
LIBS    = $(LIB)/lib_mbs.a $(LIB)/lib_tools.a $(LIB)/lib_mbs.a $(DIMLIB) \
          $(LIB)/libedit.a -llynx  -lnetinet $(CES_LIB)
endif

ifeq ($(GSI_OS_VERSION),3.1)
POSIX   =  -mthreads
CFLAGS  = $(POSIX) -D_THREADS_POSIX4ad4 -DLynx -DGSI_MBS -D$(GSI_CPU_ENDIAN) -D$(GSI_CPU_PLATFORM) 
LIBS    = $(LIB)/lib_mbs.a $(LIB)/lib_tools.a $(LIB)/lib_mbs.a $(DIMLIB) \
          $(LIB)/libedit.a -llynx -lposix-pre1c -lnetinet $(CES_LIB)
endif

ifeq ($(GSI_OS_VERSION),4.0)
POSIX   =  -mthreads
CFLAGS  = $(POSIX) -D_THREADS_POSIX4ad4 -DLynx -DGSI_MBS -D$(GSI_CPU_ENDIAN) -D$(GSI_CPU_PLATFORM)
LIBS    = $(LIB)/lib_mbs.a $(LIB)/lib_tools.a $(LIB)/lib_mbs.a $(DIMLIB) \
          $(LIB)/libedit.a -llynx -lposix-pre1c
SMILIBS = $(LIB)/lib_mbs.a $(LIB)/lib_tools.a $(LIB)/lib_mbs.a $(LIB)/lib_smi.a \
          $(LIB)/libedit.a -llynx -lposix-pre1c -lnetinet $(CES_LIB)
endif

ifeq ($(GSI_OS),Linux)
ifeq ($(GSI_OS_VERSION),2.6)
POSIX   =
CFLAGS  = $(POSIX) -DLinux -Dunix -DV40 -DGSI_MBS -D$(GSI_CPU_ENDIAN) -D$(GSI_CPU_PLATFORM)
LIBS    = $(LIB)/lib_mbs.a $(LIB)/lib_tools.a $(LIB)/lib_mbs.a $(DIMLIB) \
          $(LIB)/libedit.a  $(LIB)/lib_mbs.a -lpthread -lrt
SMILIBS = $(LIB)/lib_mbs.a $(LIB)/lib_tools.a $(LIB)/lib_mbs.a $(LIB)/lib_smi.a \
          $(LIB)/libedit.a  -lnetinet
endif

ifeq ($(GSI_OS_VERSION),3.2)
POSIX   =
CFLAGS  = $(POSIX) -DLinux -Dunix -DV40 -DGSI_MBS -D$(GSI_CPU_ENDIAN) -D$(GSI_CPU_PLATFORM)
LIBS    = $(LIB)/lib_mbs.a $(LIB)/lib_tools.a $(LIB)/lib_mbs.a $(DIMLIB) \
          $(LIB)/libedit.a  $(LIB)/lib_mbs.a -lpthread -lrt
SMILIBS = $(LIB)/lib_mbs.a $(LIB)/lib_tools.a $(LIB)/lib_mbs.a $(LIB)/lib_smi.a \
          $(LIB)/libedit.a  -lnetinet
endif

#PEX_INC = -I/daq/usr/adamczew/workspace/drivers/mbspex/include
#PEX_LIB = /daq/usr/adamczew/workspace/drivers/mbspex/lib/libmbspex.a
PEX_INC = -I/mbs/driv/mbspex_$(GSI_OS_VERSION)_DEB/include
PEX_LIB = /mbs/driv/mbspex_$(GSI_OS_VERSION)_DEB/lib/libmbspex.a

endif

m_read_meb : $(OBJ)/m_read_meb.o  $(OBJ)/objects f_user.c
	gcc -O2 $(CFLAGS) -o m_read_meb -I$(INC) $(PEX_INC) f_user.c $(OBJ)/m_read_meb.o $(LIBS) $(WR_LIB) $(PEX_LIB)
	strip m_read_meb

clean :
	rm -f core *~ *.bak *.jou m_read_meb mbslog.l
 
