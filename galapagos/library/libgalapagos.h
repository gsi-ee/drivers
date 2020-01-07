#ifndef _GAPG_LIBGALAPAGOS_H_
#define _GAPG_LIBGALAPAGOS_H_

/**
 * \file
 * C user library to work with galapagos.ko kernel module
 *
 * \author JAM (Joern Adamczewski-Musch, GSI Darmstadt, Germany -- j.adamczewski@gsi.de)
 * \date 26-August-2014 -- 10-Sep-2015
 *
 */


#include <stddef.h>
#include <stdint.h>


#include "../include/gapg_user.h"



#ifdef GALAPAGOS_NOMBS
#define printm printf
#else
void printm (char *, ...); /* use mbs logger, or for gosipcmd this will be reimplemented  as printf for link time switching*/
#endif


//////////////



#define galapagos_assert_handle(handle)                                    \
  if(handle < 0)                                  \
    {                                                                   \
      printm("Error: illegal file handle %d \n", \
                handle);                            \
      return -1;                                                   \
    }


#define GALAPAGOS_MAXCOMMANDNAME 128
#define GALAPAGOS_NUMCOMMMANDS 32
#define GALAPAGOS_MAXSOURCELENGTH 65536
#define GALAPAGOS_MAXLABELS 64

#define GALAPAGOS_NUMKERNELS 68
#define GALAPAGOS_KERNELINSTRUCTIONSIZE 0x9000
#define GALAPAGOS_KERNELPATTERNSIZE     0x80000




/** the allowed command ids*/
typedef enum
{
  GAPG_NOP,             //NoOperation
  GAPG_END_KERNEL,      //EndKernel
  GAPG_RUN_SEQUENCE,    //RunSequence
  GAPG_KEEP_LEVEL,      //KeepLevel
  GAPG_LOOP,            //Loop
  GAPG_JUMP,            //Jump
  GAPG_SYNC_CORES,      //SyncCores
  GAPG_SYNC_HOST,       //SyncHost
  GAPG_SYNC_HOSTBRANCH, //SyncHostAndBranch
  GAPG_ALERTSTOP,       //AlertAndStop
  GAPG_WAIT_DOCKPATTERN,    //WaitDockPattern
  GAPG_CHECK_DOCKBRANCH,    //CheckDockAndBranch
  GAPG_SET_LOCALDELAY,  //SetLocalDelay
  GAPG_INC_LOCALDELAY,  //IncLocalDelay
  GAPG_DEC_LOCALDELAY,   //DecLocalDelay
  GAPG_SET_DOCKDELAY,   //SetDockDelay
  GAPG_INC_DOCKDELAY,   //IncDockDelay
  GAPG_DEC_DOCKDELAY,   //DecDockDelay
  GAPG_SET_ATTEN,       //SetAttenuation
  GAPG_INC_ATTEN,       //IncAttenuation
  GAPG_DEC_ATTEN        //DecAttenuation
}
galapagos_cmd_id;



  /** command structure*/
typedef struct{
    galapagos_cmd_id id;       /**< command identifier*/
    char             opcode;        /**< operation code*/
    uint64_t       argument;        /**< contains command parameters after compilation*/
    char commandname[GALAPAGOS_MAXCOMMANDNAME]; /** explicit commmand name in source code*/
    char argnames  [GALAPAGOS_MAXCOMMANDNAME]; /** description of command arguments*/
    char numargs; /**< number of command arguments required at compile time */
} galapagos_cmd ;



/** type of kernel. must match the core*/
typedef enum
{
  GAPG_CORE_NO,         //No kernel
  GAPG_CORE_CH,         //regular signal output channel
  GAPG_CORE_TRG,        // trigger
  GAPG_CORE_USP,        // ultra short pulse
  GAPG_CORE_LJP,        // low jitter pulse
  GAPG_CORE_RFDAC       // 2 RF DACs
}
galapagos_core_t;



/** a label with absolute position in the kernel colde*/
typedef struct
{
  size_t jump_position; /**< position of label in kernel instruction code from the beginning*/
  char labelname[GALAPAGOS_MAXCOMMANDNAME]; /**< explicit label name in source code*/
} galapagos_label;


/** This list will keep source labels and jump offsets during compilation*/
typedef struct
{
  galapagos_label label[GALAPAGOS_MAXLABELS]; /** <list of source labels with corresponding positions in sketch buffer */
  size_t size; /**< entries in label list actually used*/
 } galapagos_label_list;



/** the compiled kernel code for a single core*/
typedef struct
{
  galapagos_core_t kind; /**< specifies the core this kernel is dedicated for */
  int is_enabled; /**<     switch execution of this kernel on/off*/
  int is_compiled; /**<    non-zero value indicates that compiled code has been provided*/
  char sketch_buffer[GALAPAGOS_KERNELINSTRUCTIONSIZE]; /**< buffer of instruction code*/
  size_t sketch_size; /**< length of instruction code actually used*/
  char pattern_buffer[GALAPAGOS_KERNELPATTERNSIZE]; /**< buffer of bit pattern*/
  size_t pattern_size; /**< length of pattern memory actually used*/
} galapagos_kernel;




/** The list of all registered commands*/
extern galapagos_cmd galapagos_commandlist[GALAPAGOS_NUMCOMMMANDS];

extern int galapagos_numcommands;


/** This is host memory buffer of compiled kernel before uploading*/
static galapagos_kernel galapagos_kernelbuffers[GALAPAGOS_NUMKERNELS];



static galapagos_label_list galapagos_current_labels;


/** open file handle of pex device number devnum. Return value is handle*/
int galapagos_open (int devnum);

/** close file handle*/
int galapagos_close (int handle);

/** reset dma and sfp engines */
int galapagos_reset (int handle);




/** write value of l_dat to board l_address on mapped bar*/
int galapagos_register_wr (int handle, unsigned char s_bar, long l_address, long l_dat);

/** read value of &l_dat from board l_address on mapped bar*/
int galapagos_register_rd (int handle, unsigned char s_bar, long l_address, long * l_dat);


/** check syntax/validty of source command with variable list of arguments.
 * Will return index of command in commandlist if expression is valid, or a negative value otherwise*/
int galapagos_check_command(const char* command, int* arguments, char numargs);

/** check if label of name is known to the table of current compilation.
 * returns the corresponding label position in sketch buffer, otherwise -1 if label not found*/
int galapagos_check_label(const char* label);

/** compile source command with variable list of arguments.
 * Will return reference to command object with compiled code*/
galapagos_cmd* galapagos_compile_command(const char* command, int* arguments, char numargs);

/** compiles text buffer with sourcecode and puts result into the binary code buffer of galapagos kernel object.
 * returns 0 on success, or any other value in case of problems (TODO: error codes?)*/
int galapagos_compile_kernel(const char* sourcecode, int length, galapagos_kernel* target);


/** compiles text buffer with sourcecode and puts result into the binary code buffer for upload to fpga core.
 * returns 0 on success, or any other value in case of problems (TODO: error codes?)*/
int galapagos_compile_core(int corenum, const char* sourcecode, int length);

/** copy contents of kernel code objects from source to target*/
int galapagos_copy_kernel(const galapagos_kernel* source, galapagos_kernel* target);

/** set compiled kernel code source for core corenum into library buffer.
 * The kernel Will be applied no sooner than call of galapagos_upload_kernel*/
int galapagos_set_kernel(int corenum, const galapagos_kernel* source);

/** retrieve compiled kernel code source for core corenum into library buffer.
 * */
int galapagos_get_kernel(int corenum, galapagos_kernel* target);

/** upload precompiled kernel code to core of given number.
 * Returns 0 on success, or other value in case of errors*/
int galapagos_upload_kernel(int corenum);







#endif
