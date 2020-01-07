

#include  "libgalapagos.h"

#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdlib.h>



#define RON  "\x1B[7m"
#define RES  "\x1B[0m"



//typedef struct{
//    galapagos_cmd_id id;       /**< command identifier*/
//    char             opcode;        /**< operation code*/
//    uint64_t       argument;        /**< contains command parameters after compilation*/
//    char commandname[GALAPAGOS_MAXCOMMANDNAME]; /** explicit commmand name in source code*/
//    char argnames  [GALAPAGOS_MAXCOMMANDNAME]; /** description of command arguments*/
//    char numargs; /**< number of command arguments required at compile time */
//} galapagos_cmd ;

/* Here is the intialization of all our commands:*/
galapagos_cmd galapagos_commandlist[] =
   {
     {GAPG_NOP,             0x00,   0,  "NOP",          "level",                        1},
     {GAPG_END_KERNEL,      0xFF,   0,  "EndKernel",    "level",                        1},
     {GAPG_RUN_SEQUENCE,    0x01,   0,  "RunSequence",  "startbit stopbit repetitions", 3},
     {GAPG_KEEP_LEVEL,      0x02,   0,  "KeepLevel",    "level bitperiods",             2},
     {GAPG_LOOP,            0x10,   0,  "Loop",         "level target iterations",      3},
     {GAPG_JUMP,            0x11,   0,  "Jump",         "level target",                 2},
     {GAPG_SYNC_CORES,      0x12,   0,  "SyncCores",    "level",                        1},
     {GAPG_SYNC_HOST,       0x13,   0,  "SyncHost",     "level",                        1},
     {GAPG_SYNC_HOSTBRANCH, 0x14,   0,  "SyncHostAndBranch", "level target0,..,target5",7},
     {GAPG_ALERTSTOP,       0x15,   0,  "AlertAndStop", "level code",                   2},
     {GAPG_WAIT_DOCKPATTERN,    0x18,   0,    "WaitDockPattern",  "level mask pattern",           3},
     {GAPG_CHECK_DOCKBRANCH,    0x1C,   0 ,    "CheckDockAndBranch", "level target mask pattern",  4},
     {GAPG_SET_LOCALDELAY,  0x80,   0,  "SetLocalDelay",    "level taps",               2},
     {GAPG_INC_LOCALDELAY,  0x81,   0,  "IncLocalDelay",    "level taps",               2},
     {GAPG_DEC_LOCALDELAY,  0x82,   0,  "DecLocalDelay",    "level taps",               2},
     {GAPG_SET_DOCKDELAY,   0xC0,   0,  "SetDockDelay",     "level coarse fine",        3},
     {GAPG_INC_DOCKDELAY,   0xC1,   0,  "IncDockDelay",     "level coarse fine",        3},
     {GAPG_DEC_DOCKDELAY,   0xC2,   0,  "DecDockDelay",     "level coarse fine",        3},
     {GAPG_SET_ATTEN,       0xC3,   0,  "SetAttenuation",   "level attenuation",        2},
     {GAPG_INC_ATTEN,       0xC4,   0,  "IncAttenuation",   "level attenuation",        2},
     {GAPG_DEC_ATTEN,       0xC5,   0,  "DecAttenuation",   "level attenuation",        3}
 };


int galapagos_numcommands=20;

//typedef struct
//{
//  galapagos_core_t kind; /**< specifies the core this kernel is dedicated for */
//  int is_enabled; /**<     switch execution of this kernel on/off*/
//  int is_compiled; /**<    non-zero value indicates that compiled code has been provided*/
//  char sketch_buffer[GALAPAGOS_KERNELINSTRUCTIONSIZE]; /**< buffer of instruction code*/
//  int sketch_size; /**< length of instruction code actually used*/
//  char pattern_buffer[GALAPAGOS_KERNELPATTERNSIZE]; /**< buffer of bit pattern*/
//  int pattern_size; /**< length of pattern memory actually used*/
//} galapagos_kernel;



/** This is host memory buffer of compiled kernel before uploading*/
static galapagos_kernel galapagos_kernelbuffers[]={
    {GAPG_CORE_CH, 1, 0, "" ,0 , "", 0},
    {GAPG_CORE_CH, 1, 0, "" ,0 , "", 0},
    {GAPG_CORE_CH, 1, 0, "" ,0 , "", 0},
    {GAPG_CORE_CH, 1, 0, "" ,0 , "", 0},
    {GAPG_CORE_CH, 1, 0, "" ,0 , "", 0},
    {GAPG_CORE_CH, 1, 0, "" ,0 , "", 0},
    {GAPG_CORE_CH, 1, 0, "" ,0 , "", 0},
    {GAPG_CORE_CH, 1, 0, "" ,0 , "", 0},
    {GAPG_CORE_CH, 1, 0, "" ,0 , "", 0},
    {GAPG_CORE_CH, 1, 0, "" ,0 , "", 0},
    {GAPG_CORE_CH, 1, 0, "" ,0 , "", 0},
    {GAPG_CORE_CH, 1, 0, "" ,0 , "", 0},
    {GAPG_CORE_CH, 1, 0, "" ,0 , "", 0},
    {GAPG_CORE_CH, 1, 0, "" ,0 , "", 0},
    {GAPG_CORE_CH, 1, 0, "" ,0 , "", 0},
    {GAPG_CORE_CH, 1, 0, "" ,0 , "", 0},
    {GAPG_CORE_CH, 1, 0, "" ,0 , "", 0},
    {GAPG_CORE_CH, 1, 0, "" ,0 , "", 0},
    {GAPG_CORE_CH, 1, 0, "" ,0 , "", 0},
    {GAPG_CORE_CH, 1, 0, "" ,0 , "", 0},
    {GAPG_CORE_CH, 1, 0, "" ,0 , "", 0},
    {GAPG_CORE_CH, 1, 0, "" ,0 , "", 0},
    {GAPG_CORE_CH, 1, 0, "" ,0 , "", 0},
    {GAPG_CORE_CH, 1, 0, "" ,0 , "", 0},
    {GAPG_CORE_CH, 1, 0, "" ,0 , "", 0},
    {GAPG_CORE_CH, 1, 0, "" ,0 , "", 0},
    {GAPG_CORE_CH, 1, 0, "" ,0 , "", 0},
    {GAPG_CORE_CH, 1, 0, "" ,0 , "", 0},
    {GAPG_CORE_CH, 1, 0, "" ,0 , "", 0},
    {GAPG_CORE_CH, 1, 0, "" ,0 , "", 0},
    {GAPG_CORE_CH, 1, 0, "" ,0 , "", 0},
    {GAPG_CORE_CH, 1, 0, "" ,0 , "", 0},
    {GAPG_CORE_CH, 1, 0, "" ,0 , "", 0},
    {GAPG_CORE_CH, 1, 0, "" ,0 , "", 0},
    {GAPG_CORE_CH, 1, 0, "" ,0 , "", 0},
    {GAPG_CORE_CH, 1, 0, "" ,0 , "", 0},
    {GAPG_CORE_CH, 1, 0, "" ,0 , "", 0},
    {GAPG_CORE_CH, 1, 0, "" ,0 , "", 0},
    {GAPG_CORE_CH, 1, 0, "" ,0 , "", 0},
    {GAPG_CORE_CH, 1, 0, "" ,0 , "", 0},
    {GAPG_CORE_CH, 1, 0, "" ,0 , "", 0},
    {GAPG_CORE_CH, 1, 0, "" ,0 , "", 0},
    {GAPG_CORE_CH, 1, 0, "" ,0 , "", 0},
    {GAPG_CORE_CH, 1, 0, "" ,0 , "", 0},
    {GAPG_CORE_CH, 1, 0, "" ,0 , "", 0},
    {GAPG_CORE_CH, 1, 0, "" ,0 , "", 0},
    {GAPG_CORE_CH, 1, 0, "" ,0 , "", 0},
    {GAPG_CORE_CH, 1, 0, "" ,0 , "", 0},
    {GAPG_CORE_CH, 1, 0, "" ,0 , "", 0},
    {GAPG_CORE_CH, 1, 0, "" ,0 , "", 0},
    {GAPG_CORE_CH, 1, 0, "" ,0 , "", 0},
    {GAPG_CORE_CH, 1, 0, "" ,0 , "", 0},
    {GAPG_CORE_CH, 1, 0, "" ,0 , "", 0},
    {GAPG_CORE_CH, 1, 0, "" ,0 , "", 0},
    {GAPG_CORE_CH, 1, 0, "" ,0 , "", 0},
    {GAPG_CORE_CH, 1, 0, "" ,0 , "", 0},
    {GAPG_CORE_CH, 1, 0, "" ,0 , "", 0},
    {GAPG_CORE_CH, 1, 0, "" ,0 , "", 0},
    {GAPG_CORE_CH, 1, 0, "" ,0 , "", 0},
    {GAPG_CORE_CH, 1, 0, "" ,0 , "", 0},
    {GAPG_CORE_CH, 1, 0, "" ,0 , "", 0},
    {GAPG_CORE_CH, 1, 0, "" ,0 , "", 0},
    {GAPG_CORE_CH, 1, 0, "" ,0 , "", 0},
    {GAPG_CORE_CH, 1, 0, "" ,0 , "", 0},
     // here the special cores:
    {GAPG_CORE_TRG, 1, 0, "" ,0 , "", 0},
    {GAPG_CORE_USP, 1, 0, "" ,0 , "", 0},
    {GAPG_CORE_LJP, 1, 0, "" ,0 , "", 0},
    {GAPG_CORE_RFDAC, 1, 0, "" ,0 , "", 0},
};


//galapagos_label label[GALAPAGOS_MAXLABELS]; /** <list of source labels with corresponding positions in sketch buffer */
// size_t size; /**< entries in label list actually used*/

static galapagos_label_list galapagos_current_labels ={};

//=       {
//           .label[0]       = {0,"DUMMY"},
//           .size      = 1
//       };






int galapagos_open(int devnum)
{
  int filehandle,errsv;
  char devname[64];
  char fname[256];

  snprintf(devname,64,GAPGNAMEFMT, devnum);
  snprintf(fname,256,"/dev/%s",devname);
  /*printm("galapagos: opening %s...\n",fname);*/
  filehandle = open(fname, O_RDWR );
  errsv = errno;

  if (filehandle < 0)
  {
    printm("galapagos: error %d (%s) opening device %s...\n",errsv, strerror(errsv), fname);
  }
  return filehandle;
}

int galapagos_close(int handle)
{
  galapagos_assert_handle(handle);
  close(handle);
  /* add all cleanup actions here*/
}


int galapagos_reset (int handle)
{
  int rev, errsv=0;;
  galapagos_assert_handle(handle);
  printm ("galapagos: resetting gapg device...");
  rev = ioctl (handle, GAPG_IOC_RESET);
  errsv = errno;
  if (rev)
    {
      printm ("\n\nError %d reseting gapg device", errsv, strerror (errsv));
    }
  else
    printm(" done!\n");
  return rev;
}

/*****************************************************************************/





galapagos_cmd* galapagos_compile_command(const char* command, int* arguments, char numargs)
{
  int cmdindex=-1;
  cmdindex=galapagos_check_command(command, arguments, numargs);
  if(cmdindex<0) return 0;
  galapagos_cmd* cmd=&galapagos_commandlist[cmdindex]; // we use command object in list to temporarily store the binary code
  galapagos_cmd_id id=galapagos_commandlist[cmdindex].id;
  switch(id)
  {

    case  GAPG_NOP:
    {
        cmd->argument = ((uint64_t)(arguments[0]) & 0xFF) << 56; // level
    }
    break;

    case GAPG_END_KERNEL:
    {
         cmd->argument = ((uint64_t)(arguments[0]) & 0xFF) << 56;
    }
    break;

    case GAPG_RUN_SEQUENCE:
    {
        cmd->argument =     ((uint64_t)(arguments[0]) & 0x7FFFF) << 45;   // start bit
        cmd->argument |=    ((uint64_t)(arguments[1]) & 0x7FFFF) << 26; // stop bit
        cmd->argument |=    ((uint64_t)(arguments[2]) & 0x3FFFFFF) ;  // number of repetitions
    }
    break;

    case GAPG_KEEP_LEVEL:
    {
        cmd->argument =     ((uint64_t)(arguments[0]) & 0xFF) << 56; // level
        cmd->argument |=    ((uint64_t)(arguments[1]) & 0xFFFFFFFF); // number of periods
    }
    break;

    case GAPG_LOOP:
    {
        cmd->argument =     ((uint64_t)(arguments[0]) & 0xFF) << 56; // level
        cmd->argument |=    ((uint64_t)(arguments[1]) & 0x1FF) << 47; // target  have used list of labels to calculate real offset here
        cmd->argument |=    ((uint64_t)(arguments[2]) & 0xFFFFFFFF); // number of iterations
    }
    break;

    case GAPG_JUMP:
    {
        cmd->argument =     ((uint64_t)(arguments[0]) & 0xFF) << 56; // level
        cmd->argument |=    ((uint64_t)(arguments[1]) & 0x1FF) << 47; // target have used list of labels to calculate real offset here
    }
    break;

    case GAPG_SYNC_CORES:
    {
        cmd->argument = ((uint64_t)(arguments[0]) & 0xFF) << 56; // level


    }
    break;

    case GAPG_SYNC_HOST:
    {
        cmd->argument = ((uint64_t)(arguments[0]) & 0xFF) << 56; // level
    }
    break;

    case GAPG_SYNC_HOSTBRANCH:
    {
        cmd->argument = ((uint64_t)(arguments[0]) & 0xFF) << 56; // level

        cmd->argument |= ((uint64_t)(arguments[1]) & 0x1FF) << 2; // target 0
        cmd->argument |= ((uint64_t)(arguments[2]) & 0x1FF) << 11; // target 1
        cmd->argument |= ((uint64_t)(arguments[3]) & 0x1FF) << 20; // target 2
        cmd->argument |= ((uint64_t)(arguments[4]) & 0x1FF) << 29; // target 3
        cmd->argument |= ((uint64_t)(arguments[5]) & 0x1FF) << 38; // target 4
        cmd->argument |= ((uint64_t)(arguments[6]) & 0x1FF) << 47; // target 5

    }
    break;

    case  GAPG_ALERTSTOP:
    {
      cmd->argument = ((uint64_t)(arguments[0]) & 0xFF) << 56; // level
      cmd->argument |= ((uint64_t)(arguments[1]) & 0xFF) << 48; // code
    }
    break;

    case  GAPG_WAIT_DOCKPATTERN:
    {
      cmd->argument = ((uint64_t)(arguments[0]) & 0xFF) << 56; // level
      cmd->argument |= ((uint64_t)(arguments[1]) & 0xFFFFFFF) << 28; // mask
      cmd->argument |= ((uint64_t)(arguments[2]) & 0xFFFFFFF) ; //pattern

    }
    break;

    case  GAPG_CHECK_DOCKBRANCH:
    {
      cmd->argument = ((uint64_t)(arguments[0]) & 0xFF) << 56; // level
      cmd->argument |= ((uint64_t)(arguments[1]) & 0x1FF) << 47; // target
      cmd->argument |= ((uint64_t)(arguments[2]) & 0x3F) << 6; // mask
      cmd->argument |= ((uint64_t)(arguments[3]) & 0x3F) ; // pattern

    }
    break;

    case  GAPG_SET_LOCALDELAY:
    {
      cmd->argument = ((uint64_t)(arguments[0]) & 0xFF) << 56; // level
      cmd->argument |= ((uint64_t)(arguments[1]) & 0x1F) ; // taps
    }
    break;

    case  GAPG_INC_LOCALDELAY:
    {
      cmd->argument = ((uint64_t)(arguments[0]) & 0xFF) << 56; // level
      cmd->argument |= ((uint64_t)(arguments[1]) & 0x1F) ; // taps
    }
    break;

    case  GAPG_DEC_LOCALDELAY:
    {
      cmd->argument = ((uint64_t)(arguments[0]) & 0xFF) << 56; // level
      cmd->argument |= ((uint64_t)(arguments[1]) & 0x1F) ; // taps
    }
    break;

    case  GAPG_SET_DOCKDELAY:
    {
      cmd->argument = ((uint64_t)(arguments[0]) & 0xFF) << 56; // level
      cmd->argument |= ((uint64_t)(arguments[1]) & 0x3FF) <<16; // coarse delay
      cmd->argument |= ((uint64_t)(arguments[2]) & 0xFFFF) ; // fine delay

    }
    break;

    case   GAPG_INC_DOCKDELAY:
    {
      cmd->argument = ((uint64_t)(arguments[0]) & 0xFF) << 56; // level
      cmd->argument |= ((uint64_t)(arguments[1]) & 0x3FF) <<16; // coarse delay
      cmd->argument |= ((uint64_t)(arguments[2]) & 0xFFFF) ; // fine delay
    }
    break;

    case  GAPG_DEC_DOCKDELAY:
    {
      cmd->argument = ((uint64_t)(arguments[0]) & 0xFF) << 56; // level
      cmd->argument |= ((uint64_t)(arguments[1]) & 0x3FF) <<16; // coarse delay
      cmd->argument |= ((uint64_t)(arguments[2]) & 0xFFFF) ; // fine delay
    }
    break;

    case  GAPG_SET_ATTEN:
    {
      cmd->argument = ((uint64_t)(arguments[0]) & 0xFF) << 56; // level
      cmd->argument |= ((uint64_t)(arguments[1]) & 0xFF) ; // attenuation (n*0,5 dB)
    }
    break;

    case  GAPG_INC_ATTEN:
    {
      cmd->argument = ((uint64_t)(arguments[0]) & 0xFF) << 56; // level
      cmd->argument |= ((uint64_t)(arguments[1]) & 0xFF) ; // attenuation (n*0,5 dB)

    }
    break;

    case  GAPG_DEC_ATTEN :
    {
      cmd->argument = ((uint64_t)(arguments[0]) & 0xFF) << 56; // level
      cmd->argument |= ((uint64_t)(arguments[1]) & 0xFF) ; // attenuation (n*0,5 dB)
    }
    break;

    default:
    {
      printm ("galapagos_compile_command: unknown commandid %d, NEVER COME HERE!!!",id);
      return 0;
    }
        break;

  }
  return cmd;
}


int galapagos_check_command(const char* command, int* arguments, char numargs)
{
  int ix=0;
  char commandbuffer[GALAPAGOS_MAXCOMMANDNAME];
  snprintf(commandbuffer, GALAPAGOS_MAXCOMMANDNAME, command);
  for(ix=0; ix<GALAPAGOS_NUMCOMMMANDS; ++ix)
  {

    if(strcmp(commandbuffer,galapagos_commandlist[ix].commandname)==0)
    {
      if(numargs!=galapagos_commandlist[ix].numargs) return -2;
      return ix;
    }
  }


  return -1;
}

int galapagos_check_label(const char* label)
{
  if(label==0 ) return -1;
  int i=0;
  for (i=0; i<galapagos_current_labels.size; ++i)
  {
    //printf ("galapagos_check_label loop %d of %d \n", i, galapagos_current_labels.size);
    if(strcmp(label ,galapagos_current_labels.label[i].labelname)==0)
    {
      printm ("galapagos_check_label finds position 0x%x for label %s", galapagos_current_labels.label[i].jump_position, label);
      //printf ("galapagos_check_label finds position 0x%x for label %s\n", galapagos_current_labels.label[i].jump_position, label);
      return galapagos_current_labels.label[i].jump_position;
    }
  }
  return -1;
}

/** compiles text buffer with sourcecode and puts result into the binary code buffer of core corenum.
 * returns 0 on success, or any other value in case of problems (TODO: error codes? TESTING!!!)*/
int galapagos_compile_core(int corenum, const char* sourcecode, int length)
{
  if(corenum<0 || corenum>GALAPAGOS_NUMKERNELS) return -1;
  printm ("galapagos_compile for core %d",corenum);
  galapagos_kernel* target=&galapagos_kernelbuffers[corenum];
  return galapagos_compile_kernel(sourcecode, length, target);
}

int galapagos_compile_kernel(const char* sourcecode, int length, galapagos_kernel* target)
{
  int rev=0, t=0, numargs=0, numinstructions=0, lix=0;
  size_t sizeof_onecommand=sizeof(char) + sizeof(uint64_t);
  char labelbuffer[GALAPAGOS_MAXSOURCELENGTH];
  char sourcebuffer[GALAPAGOS_MAXSOURCELENGTH];
  char commandbuffer[GALAPAGOS_MAXCOMMANDNAME];
  char* commandline=0;;
  char* argumentline=0;
  char* sourceline=0;;
  char* labelline=0;
  char *saveptr1, *saveptr2, *saveptr3, *saveptr4;

  int arguments[10];

  if(length<=0) return -2;
  if(length>GALAPAGOS_MAXSOURCELENGTH) length=GALAPAGOS_MAXSOURCELENGTH;
  snprintf(labelbuffer,length,"%s",sourcecode);
  snprintf(sourcebuffer,length,"%s",labelbuffer);
  // JAM 1-2020: somehow the second buffer is not copied correctly from sourcecode when this is done a second time. Compiler optimization business?
  // with the above way, it works.
//  printm ("galapagos_compile_kernel: labelcode:\n %s,\n length:%d",labelbuffer, length);
//  printm ("galapagos_compile_kernel: sourcebuffer:\n %s,\n length:%d",sourcebuffer, length);
  target->sketch_size=0;
  target->is_compiled=0;

  // first pass scans for label names and generates offset table
  galapagos_current_labels.size=0; // clear label registry from previous compilation
  sourceline = strtok_r (labelbuffer,"\n", &saveptr3);
   while(sourceline!=0)
   {
     labelline = strtok_r (sourceline," ",&saveptr4); // for this we need strtok_r !!!!
     if(labelline==0) break;

     // check if we have a label (by the trailing : character). for the moment, the : may be everywhere in the label name...
     if(strstr(labelline,":")==0)
     {
         numinstructions++; // just count real instructions
     }
     else
     {
       if(lix>=GALAPAGOS_MAXLABELS)
       {
         printm ("galapagos_compile_kernel: ERROR: too many labels in code (%d,  maximum is %d)", lix, GALAPAGOS_MAXLABELS);
         return -5;
       }
       // here check for duplicate label names:
       if(galapagos_check_label(labelline)!=-1)
         {
           printm ("galapagos_compile_kernel: ERROR: duplicate labels in code: %s", labelline);
           return -6;
         }
       galapagos_current_labels.label[lix].jump_position= numinstructions * sizeof_onecommand;
       snprintf( galapagos_current_labels.label[lix].labelname,GALAPAGOS_MAXCOMMANDNAME, "%s", labelline);
       galapagos_current_labels.size++;
       printm ("galapagos_compile_kernel: found label %s at position %d", galapagos_current_labels.label[lix].labelname, galapagos_current_labels.label[lix].jump_position);
       lix++;
     }
     sourceline = strtok_r (NULL,"\n",&saveptr3);
   } //  while(commandline!=0) first pass

// NOTE JAM 2019 - with this first pass, the second will not work correctly -TODO


  // second pass: here we scan all lines of sourcecode:
  numinstructions=0;
  commandline = strtok_r (sourcebuffer,"\n", &saveptr1);
  while(commandline!=0)
  {
    argumentline = strtok_r (commandline," ",&saveptr2); // for this we need strtok_r !!!!
    if(argumentline==0) break;
    //printm ("galapagos_compile_kernel: found argumentline %s ",argumentline);
    if(strstr(argumentline,":")!=0){ 
        commandline = strtok_r (NULL,"\n",&saveptr1);
    	continue; // skip any label at the beginning
    	}
    snprintf(commandbuffer,GALAPAGOS_MAXCOMMANDNAME,"%s",argumentline); // first argument is always command name
    argumentline = strtok_r (NULL," ", &saveptr2);
    numargs=0;
    while(argumentline!=0)
    {
      //printm ("galapagos_compile_kernel: found argument %s of command %s",argumentline, commandbuffer);
      // first check if our argument is a registered label, then get label position as numeric value:
      rev=galapagos_check_label(argumentline);
      arguments[numargs++]= (rev==-1 ? atoi(argumentline) : rev);
      argumentline = strtok_r (NULL," ", &saveptr2);
    }
  //numargs--; // maybe extra part after last argument?
  numinstructions++;
  printm ("galapagos_compile_kernel: found instruction %d - command %s with %d arguments:\t", numinstructions, commandbuffer, numargs);
  for(t=0; t<numargs;++t)
  {
    printm("%d\t",arguments[t]);
  }
  printm("\n");
  // now do actual compilation:
  galapagos_cmd* com=galapagos_compile_command(commandbuffer, arguments, numargs);
  if(com==0)
    {
      printm ("galapagos_compile_kernel: ERROR on compilation!!! abort sourcecode scan");
      return -4;
    }



  // now append actual command code to the kernel binary buffer:
  char* cursor=target->sketch_buffer +target->sketch_size;
  memcpy(cursor, &com->opcode, sizeof(char));
  cursor+=sizeof(char);
  memcpy(cursor, &com->argument, sizeof(uint64_t));
  cursor+=sizeof(uint64_t);
  target->sketch_size += sizeof(char) + sizeof(uint64_t);

  if(target->sketch_size >=GALAPAGOS_KERNELINSTRUCTIONSIZE)
    {
      printm("galapagos_compile_kernel: reached end of instruction memory after %d commands, stop compilation!", numinstructions);
      break;
    }

  commandline = strtok_r (NULL,"\n",&saveptr1);
  } // while

  target->is_compiled=1;
  return 0;
}


int galapagos_copy_kernel(const galapagos_kernel* source, galapagos_kernel* target)
{
  if(source==target) return 0;
  target->kind=source->kind;
  target->is_enabled=source->is_enabled;
  target->is_compiled = source->is_enabled;
  memset(target->sketch_buffer, 0, GALAPAGOS_KERNELINSTRUCTIONSIZE);
  memcpy(target->sketch_buffer, source->sketch_buffer, source->sketch_size);
  target->sketch_size = source->sketch_size;
  memset(target->pattern_buffer, 0, GALAPAGOS_KERNELPATTERNSIZE);
  memcpy(target->pattern_buffer, source->pattern_buffer, source->pattern_size);
  target->pattern_size = source->pattern_size;
  return 0;
}



int galapagos_set_kernel(int corenum, const galapagos_kernel* source)
{
  printm ("galapagos_set_kernel: core %d ", corenum);
  if(corenum<0 || corenum>GALAPAGOS_NUMKERNELS) return -1;
  galapagos_kernel* target=&galapagos_kernelbuffers[corenum];
  return (galapagos_copy_kernel(source, target));
}

int galapagos_get_kernel(int corenum, galapagos_kernel* target)
{
  printm ("galapagos_get_kernel: core %d ", corenum);
  if(corenum<0 || corenum>GALAPAGOS_NUMKERNELS) return -1;
  galapagos_kernel* source=&galapagos_kernelbuffers[corenum];
  return (galapagos_copy_kernel(source, target));
}




/** upload precompiled kernel code to core of given number.
 * Returns 0 on success, or other value in case of errors*/
int galapagos_upload_kernel(int corenum)
{
  printm ("galapagos_upload_kernel - we will still implement to upload kernel of core %d to GALAPAGOS board!",corenum);
  if(galapagos_kernelbuffers[corenum].is_compiled==0)
    {
      printm ("galapagos_upload_kernel for core %d - ERROR - kernel is not yet compiled!!",corenum);
      return -1;
    }
  return 0;
}














/*****************************************************************************/
int galapagos_register_wr (int handle, unsigned char s_bar, long l_address, long l_dat)
{
  int rev = 0, errsv = 0;
  struct gapg_reg_io descriptor;
  galapagos_assert_handle(handle);
  descriptor.address = l_address;
  descriptor.value = l_dat;
  descriptor.bar = s_bar;
  rev = ioctl (handle, GAPG_IOC_WRITE_REGISTER, &descriptor);
  errsv = errno;
  if (rev)
  {
    printm (RON"ERROR>>"RES"Error %d  on writing value %0xlx to address %0xlx (bar:%d)- %s\n", errsv, l_dat, l_address,
        s_bar, strerror (errsv));
  }
  return rev;
}

int galapagos_register_rd (int handle, unsigned char s_bar, long l_address, long * l_dat)
{
  int rev = 0, errsv = 0;
  struct gapg_reg_io descriptor;
  galapagos_assert_handle(handle);
  descriptor.address = l_address;
  descriptor.bar = s_bar;
  rev = ioctl (handle, GAPG_IOC_READ_REGISTER, &descriptor);
  errsv = errno;
  if (rev)
  {
    printm (RON"ERROR>>"RES"Error %d  on reading from address 0x%lx (bar:%d)- %s\n", errsv, l_address,
        s_bar, strerror (errsv));
  }
  * l_dat=descriptor.value;
  return rev;
}



