#ifndef __GALAPCMD_H__
#define __GALAPCMD_H__

/**
 * \file
 * Command line interface for galap io protocol with mbxgapg library
 * \author J.Adamczewski-Musch (j.adamczewski@gsi.de)
 * \date 26-Aug_2014
 *
 */


/*#include "../driver/gapg_user.h"*/
#include "galapagos/libgalapagos.h"

#include <string.h>
//#include "timing.h"
#include <libgen.h>




#include <stdio.h>
#include <errno.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <sys/fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdlib.h>

#define GALAP_MAXTEXT 128
#define GALAP_CMD_SIZE 256
#define GALAP_CMD_MAX_ARGS 10

/** toggle configuration mode: send config from file as data block to driver (1), or write with single bus commands (0)*/
//#define GALAPCMD_BLOCKCONFIG 1

typedef enum
{
  GALAP_NOP,
  GALAP_RESET,
//  GALAP_INIT,
  GALAP_READ,
  GALAP_WRITE,
  GALAP_SETBIT,
  GALAP_CLEARBIT,
  GALAP_CONFIGURE,
  GALAP_VERIFY
} gos_cmd_id;


struct galap_cmd {
    char devnum;                 /**< logical device number of the gapg device to open*/
    gos_cmd_id command;                 /**< command identifier*/
    char verboselevel;            /**< level of debug 0=off*/
    char hexformat;               /**< hexoutput(1) or decimal (0)*/
    char verify;                   /**< verify mode*/
    int fd_gapg;                  /**< keep file descriptor here*/
    long address;                /**< address on gapg board*/
    long value;                /**< value to write, or read back*/
    long repeat;               /**< number of words for incremental read*/

    char filename[GALAP_MAXTEXT]; /**< optional name of configuration file*/
    FILE* configfile;              /**< handle to configuration file*/
    int linecount;                /**< configfile linecounter*/
    int errcount;                 /**< errorcount for verify*/
};




void galcmd_close_device(struct galap_cmd* com);

void galcmd_usage(const char *progname);

/** check if parameters are set correctly*/
void galcmd_assert_command(struct galap_cmd* com);

/** check if number of arguments does match the command*/
void galcmd_assert_arguments(struct galap_cmd* com, int arglen);

/** initialize command structure to defaults*/
void galcmd_defaults(struct galap_cmd* com);

/** set command structure from id. assert that no duplicate id setups appear */
void galcmd_set_command(struct galap_cmd* com, gos_cmd_id id);

/** printout current command structure*/
void galcmd_dump_command(struct galap_cmd* com);


/** printout current command structure*/
int galcmd_execute_command(struct galap_cmd* com);


/** open the gapg device, return file descriptor in com*/
int galcmd_open_device(struct galap_cmd* com);

/** open configuration file*/
int galcmd_open_configuration (struct galap_cmd* com);

/** close configuration file*/
int galcmd_close_configuration (struct galap_cmd* com);

/** fill next values from configuration file into com structure.
 * returns -1 if end of config is reached*/
int galcmd_next_config_values (struct galap_cmd* com);


/** write to slave address specified in command*/
int galcmd_write(struct galap_cmd* com);

/** read from slave address specified in command*/
int galcmd_read(struct galap_cmd* com);

/** set or clear bits of given mask in slave address register*/
int galcmd_changebits(struct galap_cmd* com);


/** wrapper for all slave operations with incremental address io capabilities*/
int galcmd_busio(struct galap_cmd* com);

/** initialize sfp chain*/
int galcmd_init(struct galap_cmd* com);

/** load register values from configuration file*/
int galcmd_configure(struct galap_cmd* com);

/** compare register values with configuration file*/
int galcmd_verify(struct galap_cmd* com);

/** compare register values with configuration file, primitive function*/
int galcmd_verify_single (struct galap_cmd* com);

/** broadcast: loop command operation over several slaves*/
//int galcmd_broadcast(struct galap_cmd* com);

/** printout results of operation*/
int galcmd_output(struct galap_cmd* com);

/** evaluate command name string*/
char* galcmd_get_description(struct galap_cmd* com);

int main (int argc, char *argv[]);


#endif
