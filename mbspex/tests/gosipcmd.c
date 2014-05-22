/*
 * Command line interface for gosip io protocol with mbxpex library
 * J.Adamczewski-Musch, gsi, 22-May_2014
 *
 */

#include "gosipcmd.h"





void goscmd_defaults(struct gosip_cmd* com)
{
  com->devnum=0;
  com->command=GOSIP_NOP;
  com->verboselevel=0;
  com->hexformat=0;
  com->fd_pex=-1;
  com->sfp=-1;
  com->slave=-1;
  com->address=-1;
  com->value=0;
}

void goscmd_set_command (struct gosip_cmd* com, gos_cmd_id id)
{
  if (com->command == GOSIP_NOP)
  {
    com->command = id;
  }
  else
  {
    printm (" gosipcmd ERROR - conflicting command specifiers!\n");
    exit (1);
  }
}


void goscmd_dump_command(struct gosip_cmd* com)
{
    printm(" gosipcmd dump: \n");
    printm(" \t device   :%d \n",com->devnum);
    printm(" \t sfp      :%d \n",com->sfp);
    printm(" \t slave    :%d \n",com->slave);
    printm(" \t address  :0x%x \n",com->address);
    printm(" \t value    :0x%x \n",com->value);

}

void goscmd_assert_arguments(struct gosip_cmd* com, int arglen)
{
  int do_exit=0;
  if((com->command==GOSIP_INIT) && (arglen<2)) do_exit=1;
  if((com->command==GOSIP_READ) && (arglen<3)) do_exit=1;
  if((com->command==GOSIP_WRITE) && (arglen<4)) do_exit=1;
  if(do_exit)
      {
          printm(" gosipcmd ERROR - number of parameters not sufficient for command!\n");
          exit(1);
      }




}

void goscmd_assert_command(struct gosip_cmd* com)
{
  int do_exit=0;
  if(com==GOSIP_NOP) do_exit=1;
  if (com->fd_pex < 0) do_exit=1;
  if(com->sfp <0)  do_exit=1;
  if(com->slave <0)  do_exit=1;
  if(com->address <0)  do_exit=1;

  if(do_exit)
    {
        printm(" gosipcmd ERROR - illegal parameters \n");
        goscmd_dump_command(com);
        exit(1);
    }
}


int goscmd_open_device(struct gosip_cmd* com)
{
  com->fd_pex=mbspex_open(com->devnum);
  if (com->fd_pex < 0) {
    printm("ERROR>> open /dev/pexor%d \n", com->devnum);
    exit(1);
  }
  return 0;
}

void goscmd_close_device(struct gosip_cmd* com)
{
    close(com->fd_pex);
}


int goscmd_write(struct gosip_cmd* com)
{
  goscmd_assert_command(com);
  return (mbspex_slave_wr(com->fd_pex, com->sfp, com->slave, com->address, com->value));
}

int goscmd_read(struct gosip_cmd* com)
{
  int rev=0;
  goscmd_assert_command(com);
  rev=mbspex_slave_rd(com->fd_pex, com->sfp, com->slave, com->address, &(com->value));
  if(rev==0)
    {
      goscmd_output(com);
    }
  else
    {
      printm("ERROR on reading!\n");
      if(com->verboselevel) goscmd_dump_command(com);
    }
    return rev;
}

int goscmd_init(struct gosip_cmd* com)
{
  goscmd_assert_command(com);
  return (mbspex_slave_init(com->fd_pex, com->sfp, com->slave));
}


int goscmd_output(struct gosip_cmd* com)
{
  com->hexformat ? printm("0x%x \n", com->value): printm("%d \n", com->value);
}




void goscmd_usage(const char *progname)
{
    printf("***************************************************************************\n");

    printf(" %s for mbspex library  \n",progname);
    printf(" v0.1 22-May-2014 by JAM (j.adamczewski@gsi.de)\n");
    printf("***************************************************************************\n");
    printf("  usage: %s [-w|-r|-i|-d DEVICE |-v VERBOSITY] sfp slave [address [value]] \n",progname);
    printf("\t Options:\n");
    printf("\t\t -h        : display this help\n");
    printf("\t\t -w        : write to  register\n");
    printf("\t\t -r        : read from register \n");
    printf("\t\t -i        : initialize sfp chain \n");
    printf("\t\t -d DEVICE : specify device number N (/dev/pexorN) \n");
    printf("\t\t -v VERBOS : verbose debug mode \n");
    printf("\t\t -x        : results in hex format \n");
    printf("\t Arguments:\n");
    printf("\t\t sfp      - sfp chain \n");
    printf("\t\t slave    - slave id at chain, or total number of slaves\n");
    printf("\t\t address  - register on slave \n");
    printf("\t\t value    - value to write on slave \n");
    printf("\t Examples:\n");
    printf("\t  %s -i 0 24: initialize chain at sfp 0 with 24 slave devices\n",progname);
    printf("\t  %s -r -x 0 3 0x1000  : read from sfp0, slave 3, address 0x1000\n",progname);
    printf("\t  %s -w -x 0 3 0x1000 0x2A  : write value 0x2A to sfp 0, slave 3, address 0x1000\n",progname);
    printf("*****************************************************************************\n");
    exit(0);
}


int goscmd_execute_command(struct gosip_cmd* com)
{
  int rev=0;
  switch(com->command)
  {
    case GOSIP_INIT:
      rev=goscmd_init(com);
      break;
    case GOSIP_READ:
      rev=goscmd_read(com);
      break;
    case GOSIP_WRITE:
      rev=goscmd_write(com);
      break;
    default:
      printm("Error: Unknown command %d \n",com->command);
      rev=-2;
      break;
  };
return rev;
}


int main (int argc, char *argv[])
{
  int          l_status;
  int opt;
  char cmd[GOSIP_CMD_MAX_ARGS][GOSIP_CMD_SIZE];
  unsigned int cmdLen = 0;
  unsigned int i;
  struct gosip_cmd theCommand;
  goscmd_defaults(&theCommand);


/* get arguments*/
  optind = 1;
     while ((opt = getopt(argc, argv, "hwrid:v:x")) != -1) {
       switch (opt) {
       case '?':
         goscmd_usage(basename(argv[0]));
         exit(EXIT_FAILURE);
       case 'h':
         goscmd_usage(basename(argv[0]));
         exit(EXIT_SUCCESS);
       case 'd':
         theCommand.devnum = strtol(optarg, NULL, 0);
         break;
       case 'w':
         goscmd_set_command (&theCommand, GOSIP_WRITE);
         break;
       case 'r':
         goscmd_set_command (&theCommand, GOSIP_READ);
         break;
       case 'i':
         goscmd_set_command (&theCommand, GOSIP_INIT);
         break;
       case 'v':
         theCommand.verboselevel = strtol(optarg, NULL, 0);
         break;
       case 'x':
         theCommand.hexformat= 1;
         break;
       default:
         break;
       }
     }

  /* get parameters:*/
      cmdLen = argc - optind;
      goscmd_assert_arguments(&theCommand,cmdLen);
      for (i = 0; (i < cmdLen) && (i < GOSIP_CMD_MAX_ARGS); i++) {
        strncpy(cmd[i], argv[optind + i], GOSIP_CMD_SIZE);
      }
  /* TODO: implement skript processing as in trbcmd*/


      theCommand.sfp=strtoul(cmd[0], NULL, theCommand.hexformat == 1 ? 16 : 0);
      theCommand.slave=strtoul(cmd[1], NULL, theCommand.hexformat == 1 ? 16 : 0);

      if((theCommand.command==GOSIP_READ) || (theCommand.command==GOSIP_WRITE))
        theCommand.address=strtoul(cmd[2], NULL, theCommand.hexformat == 1 ? 16 : 0);
      if(theCommand.command==GOSIP_WRITE)
        theCommand.value=strtoul(cmd[3], NULL, theCommand.hexformat == 1 ? 16 : 0);

      goscmd_open_device(&theCommand);
      goscmd_assert_command(&theCommand);
      l_status=goscmd_execute_command(&theCommand);
      goscmd_close_device(&theCommand);

return l_status;
}

