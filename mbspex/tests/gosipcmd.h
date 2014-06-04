#ifndef __GOSIPCMD_H__
#define __GOSIPCMD_H__

/*
 * Command line interface for gosip io protocol with mbxpex library
 * J.Adamczewski-Musch, gsi, 22-May_2014
 *
 */


/*#include "../driver/pex_user.h"*/
#include "mbspex/libmbspex.h"

#include <string.h>
#include "timing.h"
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

#define GOSIP_MAXTEXT 128
#define GOSIP_CMD_SIZE 256
#define GOSIP_CMD_MAX_ARGS 10


typedef enum
{
  GOSIP_NOP,
  GOSIP_RESET,
  GOSIP_INIT,
  GOSIP_READ,
  GOSIP_WRITE,
  GOSIP_SETBIT,
  GOSIP_CLEARBIT,
  GOSIP_CONFIGURE,
  GOSIP_VERIFY
} gos_cmd_id;


struct gosip_cmd {
    char devnum;                 /* logical device number of the pex device to open*/
    gos_cmd_id command;                 /* command identifier*/
    char verboselevel;            /* level of debug 0=off*/
    char hexformat;               /* hexoutput(1) or decimal (0)*/
    int fd_pex;                  /* keep file descriptor here*/
    long sfp;                    /* sfp chain */
    long slave;                  /* slave id*/
    long address;                /* address on slave*/
    long value;                /* value to write, or read back*/
    long repeat;               /* number of words for incremental read*/
    char filename[GOSIP_MAXTEXT]; /* optional name of configuration file*/
    FILE* configfile;              /* handle to configuration file*/
    int linecount;                /* configfile linecounter*/
};




void goscmd_close_device(struct gosip_cmd* com);

void goscmd_usage(const char *progname);

/* check if parameters are set correctly*/
void goscmd_assert_command(struct gosip_cmd* com);

/* check if number of arguments does match the command*/
void goscmd_assert_arguments(struct gosip_cmd* com, int arglen);

/* initialize command structure to defaults*/
void goscmd_defaults(struct gosip_cmd* com);

/* set command structure from id. assert that no duplicate id setups appear */
void goscmd_set_command(struct gosip_cmd* com, gos_cmd_id id);

/* printout current command structure*/
void goscmd_dump_command(struct gosip_cmd* com);


/* printout current command structure*/
int goscmd_execute_command(struct gosip_cmd* com);


/* open the pex device, return file descriptor in com*/
int goscmd_open_device(struct gosip_cmd* com);

/* open configuration file*/
int goscmd_open_configuration (struct gosip_cmd* com);

/* close configuration file*/
int goscmd_close_configuration (struct gosip_cmd* com);

/* fill next values from configuration file into com structure.
 * returns -1 if end of config is reached*/
int goscmd_next_config_values (struct gosip_cmd* com);


/* write to slave address specified in command*/
int goscmd_write(struct gosip_cmd* com);

/* read from slave address specified in command*/
int goscmd_read(struct gosip_cmd* com);

/* set or clear bits of given mask in slave address register*/
int goscmd_changebits(struct gosip_cmd* com);



/* initialize sfp chain*/
int goscmd_init(struct gosip_cmd* com);

/* load register values from configuration file*/
int goscmd_configure(struct gosip_cmd* com);

/* compare register values with configuration file*/
int goscmd_verify(struct gosip_cmd* com);


/* printout results of operation*/
int goscmd_output(struct gosip_cmd* com);

/* evaluate command name string*/
char* goscmd_get_description(struct gosip_cmd* com);

int main (int argc, char *argv[]);


#endif
