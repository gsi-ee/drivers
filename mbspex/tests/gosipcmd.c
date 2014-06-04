/*
 * Command line interface for gosip io protocol with mbxpex library
 * J.Adamczewski-Musch, gsi, 26-May_2014
 *
 */

#include "gosipcmd.h"
#include <string.h>

static char CommandDescription[GOSIP_MAXTEXT];

void goscmd_defaults (struct gosip_cmd* com)
{
  com->devnum = 0;
  com->command = GOSIP_NOP;
  com->verboselevel = 0;
  com->hexformat = 0;
  com->fd_pex = -1;
  com->sfp = -1;
  com->slave = -1;
  com->address = -1;
  com->value = 0;
  com->repeat = 1;
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

void goscmd_dump_command (struct gosip_cmd* com)
{
  //printm (" gosipcmd dump: \n");
  com->hexformat == 1 ? printm (" Command  :0x%x (%s)", com->command, goscmd_get_description (com)) :
                        printm (" Command: %d (%s)", com->command, goscmd_get_description (com));
  com->hexformat == 1 ? printm ("\t device: 0x%x", com->devnum) : printm ("\t device: %d", com->devnum);
  com->hexformat == 1 ? printm ("\t sfp: 0x%x", com->sfp) : printm ("\t sfp: %d", com->sfp);
  com->hexformat == 1 ? printm ("\t slave: 0x%x", com->slave) : printm (" \t slave: %d", com->slave);
  com->hexformat == 1 ? printm ("\t address: 0x%x", com->address) : printm ("\t address: %d", com->address);
  com->hexformat == 1 ? printm ("\t value: 0x%x", com->value) : printm ("\t value: %d", com->value);
  com->hexformat == 1 ? printm ("\t repeat: 0x%x \n", com->repeat) : printm ("\t repeat: %d \n", com->repeat);

//  if ((com->command == GOSIP_CONFIGURE) || (com->command == GOSIP_VERIFY))
//  printm (" \t config file    :%s \n", com->filename);
}

void goscmd_assert_arguments (struct gosip_cmd* com, int arglen)
{
  int do_exit = 0;
  if ((com->command == GOSIP_INIT) && (arglen < 2))
    do_exit = 1;
  if ((com->command == GOSIP_READ) && (arglen < 3))
    do_exit = 1;
  if ((com->command == GOSIP_WRITE) && (arglen < 4))
    do_exit = 1;
  if ((com->command == GOSIP_SETBIT) && (arglen < 4))
    do_exit = 1;
  if ((com->command == GOSIP_CLEARBIT) && (arglen < 4))
    do_exit = 1;
  if (do_exit)
  {
    printm (" gosipcmd ERROR - number of parameters not sufficient for command!\n");
    exit (1);
  }

}

void goscmd_assert_command (struct gosip_cmd* com)
{
  int do_exit = 0;
  if (com == GOSIP_NOP)
    do_exit = 1;
  if (com->fd_pex < 0)
    do_exit = 1;
  if ((com->command != GOSIP_CONFIGURE) && (com->command != GOSIP_VERIFY) && (com->command != GOSIP_RESET))
  {
    if (com->sfp < 0)
      do_exit = 1;
    if ((com->command != GOSIP_INIT) && (com->slave < 0))
      do_exit = 1;
    if ((com->command != GOSIP_INIT) && (com->address < 0))
      do_exit = 1;
  }
  if (do_exit)
  {
    printm (" gosipcmd ERROR - illegal parameters \n");
    goscmd_dump_command (com);
    exit (1);
  }
}

char* goscmd_get_description (struct gosip_cmd* com)
{
  switch (com->command)
  {
    case GOSIP_RESET:
      snprintf (CommandDescription, GOSIP_MAXTEXT, "Reset pexor/kinpex board");
      break;
    case GOSIP_INIT:
      snprintf (CommandDescription, GOSIP_MAXTEXT, "Initialize sfp chain");
      break;
    case GOSIP_READ:
      snprintf (CommandDescription, GOSIP_MAXTEXT, "Read value");
      break;
    case GOSIP_WRITE:
      snprintf (CommandDescription, GOSIP_MAXTEXT, "Write value");
      break;
    case GOSIP_CONFIGURE:
      snprintf (CommandDescription, GOSIP_MAXTEXT, "Configure");
      break;
    case GOSIP_VERIFY:
      snprintf (CommandDescription, GOSIP_MAXTEXT, "Verify   ");
      break;

    case GOSIP_SETBIT:
      snprintf (CommandDescription, GOSIP_MAXTEXT, "Set Bitmask");
      break;
    case GOSIP_CLEARBIT:
      snprintf (CommandDescription, GOSIP_MAXTEXT, "Clear Bitmask");
      break;
    default:
      snprintf (CommandDescription, GOSIP_MAXTEXT, "Unknown command");
      break;
  };
  return CommandDescription;
}

int goscmd_open_device (struct gosip_cmd* com)
{
  com->fd_pex = mbspex_open (com->devnum);
  if (com->fd_pex < 0)
  {
    printm ("ERROR>> open /dev/pexor%d \n", com->devnum);
    exit (1);
  }
  return 0;
}

void goscmd_close_device (struct gosip_cmd* com)
{
  close (com->fd_pex);
}

int goscmd_open_configuration (struct gosip_cmd* com)
{
  if (strncmp (com->filename, "-", 256) == 0)
  {
    com->configfile = stdin; /* do we need this? line input like trbcmd */
  }
  else
  {
    com->configfile = fopen (com->filename, "r");
    if (com->configfile == NULL )
    {
      printm (" Error opening Configuration File '%s': %s\n", com->filename, strerror (errno));
      return -1;
    }
  }
  com->linecount = 0;
  return 0;
}

int goscmd_close_configuration (struct gosip_cmd* com)
{
  fclose (com->configfile);
}

int goscmd_next_config_values (struct gosip_cmd* com)
{
  /* file parsing code was partially stolen from trbcmd.c Thanks Ludwig Maier et al. for this!*/
  int status = 0;
  size_t linelen = 0;
  char* cmdline;
  char cmd[GOSIP_CMD_MAX_ARGS][GOSIP_CMD_SIZE];
  int i, cmdlen;

  char *c = NULL;
  for (i = 0; i < GOSIP_CMD_MAX_ARGS; i++)
  {
    cmd[i][0] = '\0';
  }
  com->linecount++;
  status = getline (&cmdline, &linelen, com->configfile);
  //printm("DDD got from file %s line %s !\n",com->filename, cmdline);
  if (status == -1)
  {
    if (feof (com->configfile) != 0)
    {
      /* EOF reached */
      rewind (com->configfile);
      return -1;
    }
    else
    {
      /* Error reading line */
      printm ("Error reading script-file\n");
      return -1;
    }
  }

  /* Remove newline and comments */
  if ((c = strchr (cmdline, '\n')) != NULL )
  {
    *c = '\0';
  }
  if ((c = strchr (cmdline, '#')) != NULL )
  {
    *c = '\0';
  }

  /* Split up cmdLine */
  sscanf (cmdline, "%s %s %s %s %s %s %s %s %s %s", cmd[0], cmd[1], cmd[2], cmd[3], cmd[4], cmd[5], cmd[6], cmd[7],
      cmd[8], cmd[9]);

  for (i = 0, cmdlen = 0; i < GOSIP_CMD_MAX_ARGS; i++, cmdlen++)
  {
    if (cmd[i][0] == '\0')
    {
      //printm("got commandlen(%d)=%d\n",i,cmdlen);
      break;
    }
  }
  if (cmdlen == 0)
  {
    /* Empty Line */
    //printm("got empty line\n");
    return 0;
  }

  if (com->verboselevel > 1)
  {
    printm ("#Line %d:  %s\n", com->linecount, cmdline);
  }

  if (!(cmdlen >= 4))
  {
    printm ("Invalid configuration data at line %d, (cmdlen=%d)stopping configuration\n", com->linecount, cmdlen);
    return -1;
  }
  com->sfp = strtoul (cmd[0], NULL, com->hexformat == 1 ? 16 : 0);
  com->slave = strtoul (cmd[1], NULL, com->hexformat == 1 ? 16 : 0);
  com->address = strtoul (cmd[2], NULL, com->hexformat == 1 ? 16 : 0);
  com->value = strtoul (cmd[3], NULL, com->hexformat == 1 ? 16 : 0);
  if (cmdlen > 4)
  {
    // parse optional command identifier
    if (strcasestr (cmd[4], "setbit") != 0)
    {
      com->command = GOSIP_SETBIT;
    }
    else if (strcasestr (cmd[4], "clearbit") != 0)
    {
      com->command = GOSIP_CLEARBIT;
    }
    else
    {
      // do not change mastercommand id (configure or verify)
    }

  }
  return status;
}

int goscmd_write (struct gosip_cmd* com)
{
  int rev = 0;
  int cursor = 0;
  goscmd_assert_command (com);
  if (com->verboselevel)
       goscmd_dump_command (com);
  while (cursor < com->repeat)
  {
    rev = mbspex_slave_wr (com->fd_pex, com->sfp, com->slave, com->address, com->value);
    if (rev != 0)
    {
      printm ("ERROR on writing!\n");
      //if (com->verboselevel)
      goscmd_dump_command (com);
      break;
    }
    cursor++;
    com->address+=4;
  }
  return rev;
}

int goscmd_read (struct gosip_cmd* com)
{
  int rev = 0;
  int cursor = 0;
  goscmd_assert_command (com);
  if (com->verboselevel)
    goscmd_dump_command (com);

  while (cursor < com->repeat)
  {
    rev = mbspex_slave_rd (com->fd_pex, com->sfp, com->slave, com->address, &(com->value));
    if (rev == 0)
    {
      if (com->command == GOSIP_READ)
        goscmd_output (com);    // only do output if we have explicit read, suppress during verify
    }
    else
    {
      printm ("ERROR on reading!\n");
      //if (com->verboselevel)
      goscmd_dump_command (com);
      break;
    }
    cursor++;
    com->address+=4;
  }    // repeat
  return rev;
}

int goscmd_changebits (struct gosip_cmd* com)
{
  int rev = 0;
  int cursor = 0;
  long bitmask = 0;
  goscmd_assert_command (com);
  if (com->verboselevel)
    goscmd_dump_command (com);
  bitmask = com->value;

  while (cursor < com->repeat)
  {

    rev = mbspex_slave_rd (com->fd_pex, com->sfp, com->slave, com->address, &(com->value));
    if (rev != 0)
    {
      printm ("ERROR on reading in change bit at address 0x%x!\n", com->address);
      break;
    }
    switch (com->command)
    {
      case GOSIP_SETBIT:
        com->value |= bitmask;
        break;
      case GOSIP_CLEARBIT:
        com->value &= ~bitmask;
        break;
      default:
        break;
    }
    rev = mbspex_slave_wr (com->fd_pex, com->sfp, com->slave, com->address, com->value);
    if (rev != 0)
    {
      printm ("ERROR on writing in change bit at address 0x%x!\n", com->address);
      break;
    }
    cursor++;
    com->address+=4;
  }    // while repeat

  return rev;
}

int goscmd_reset (struct gosip_cmd* com)
{
  goscmd_assert_command (com);
  if (com->verboselevel)
    goscmd_dump_command (com);
  return (mbspex_reset (com->fd_pex));
}

int goscmd_init (struct gosip_cmd* com)
{
  goscmd_assert_command (com);
  if (com->verboselevel)
    goscmd_dump_command (com);
  return (mbspex_slave_init (com->fd_pex, com->sfp, com->slave));
}

int goscmd_configure (struct gosip_cmd* com)
{
  int rev = 0;
  gos_cmd_id mastercommand;
  goscmd_assert_command (com);
  if (com->verboselevel > 1)
    goscmd_dump_command (com);
  if (goscmd_open_configuration (com) < 0)
    return -1;
  printm ("Configuring from file %s - \n", com->filename);
  mastercommand = com->command;
  while ((rev = goscmd_next_config_values (com)) != -1)
  {
    if (rev == 0)
      continue;    // skip line
    if ((com->command == GOSIP_SETBIT) || (com->command == GOSIP_CLEARBIT))
    {
      if (goscmd_changebits (com) != 0)
        return -1;
      com->command = mastercommand;    // reset command descriptor
    }
    else
    {
      if (goscmd_write (com) != 0)
        return -1;
    }
  }
  printm ("Done.\n");
  goscmd_close_configuration (com);
  return 0;

}

int goscmd_verify (struct gosip_cmd* com)
{
  long checkvalue = 0;
  int errcount = 0;
  int rev = 0;
  gos_cmd_id mastercommand;

  goscmd_assert_command (com);
  mastercommand = com->command;
  if (com->verboselevel > 1)
    goscmd_dump_command (com);
  if (goscmd_open_configuration (com) < 0)
    return -1;
  printm ("Verified actual configuration with file %s - \n", com->filename);
  while ((rev = goscmd_next_config_values (com)) != -1)
  {
    int haserror = 0;
    if (rev == 0)
      continue;    // skip line
    checkvalue = com->value;    // this is reference value from file

    if (goscmd_read (com) != 0)
      return -1;

    switch (com->command)
    {
      case GOSIP_SETBIT:
        if ((checkvalue & com->value) != checkvalue)
          haserror = 1;
        break;
      case GOSIP_CLEARBIT:
        if ((checkvalue & ~com->value) != checkvalue)
          haserror = 1;
        break;
      default:
        if (checkvalue != com->value)
          haserror = 1;
        break;

    };

    if (haserror)
    {
      errcount++;
      if (com->verboselevel)
      {
        printm (" Verify ERROR %d at sfp %d slave %d address 0xd%x : readback=0x%x, config=0x%x", errcount, com->sfp,
            com->slave, com->address, com->value, checkvalue);
        if (com->command != mastercommand)
          printm (" (%s)\n", goscmd_get_description (com));
        else
          printm ("\n");
      }
    }
    com->command = mastercommand;    // reset command descriptor
  }

  printm ("Verify found %d errors\n", errcount);
  goscmd_close_configuration (com);
  return 0;
  return -1;

}

int goscmd_output (struct gosip_cmd* com)
{
  if (com->verboselevel)
  {
    com->hexformat ?
        printm ("SFP: 0x%x Module: 0x%x Address: 0x%x  Data: 0x%x \n", com->sfp, com->slave, com->address, com->value) :
        printm ("SFP: %d Module: %d Address: %d  Data: %d \n", com->sfp, com->slave, com->address, com->value);
  }
  else
  {
    com->hexformat ? printm ("0x%x \n", com->value) : printm ("%d \n", com->value);
  }
}

void goscmd_usage (const char *progname)
{
  printf ("***************************************************************************\n");

  printf (" %s for mbspex library  \n", progname);
  printf (" v0.2 3-Jun-2014 by JAM (j.adamczewski@gsi.de)\n");
  printf ("***************************************************************************\n");
  printf (
      "  usage: %s [-h|-z|-i|-r|-w|-s|-u] [-c|-v FILE] [-n DEVICE |-d|-x] sfp slave [address [value [words]|[words]]] \n",
      progname);
  printf ("\t Options:\n");
  printf ("\t\t -h        : display this help\n");
  printf ("\t\t -z        : reset (zero) pexor/kinpex board \n");
  printf ("\t\t -i        : initialize sfp chain \n");
  printf ("\t\t -r        : read from register \n");
  printf ("\t\t -w        : write to  register\n");
  printf ("\t\t -s        : set bits of given mask in  register\n");
  printf ("\t\t -u        : unset bits of given mask in  register\n");
  printf ("\t\t -c FILE   : configure registers with values from FILE.gos\n");
  printf ("\t\t -v FILE   : verify register contents (compare with FILE.gos)\n");
  printf ("\t\t -n DEVICE : specify device number N (/dev/pexorN, default:0) \n");
  printf ("\t\t -d        : debug mode \n");
  printf ("\t\t -x        : results in hex format \n");
  printf ("\t Arguments:\n");
  printf ("\t\t sfp      - sfp chain \n");
  printf ("\t\t slave    - slave id at chain, or total number of slaves\n");
  printf ("\t\t address  - register on slave \n");
  printf ("\t\t value    - value to write on slave \n");
  printf ("\t\t words    - number of words to read/write/set incrementally\n");
  printf ("\t Examples:\n");
  printf ("\t  %s -z -n 1                   : master gosip reset of board /dev/pexor1 \n", progname);
  printf ("\t  %s -i 0 24                   : initialize chain at sfp 0 with 24 slave devices\n", progname);
  printf ("\t  %s -r -x 1 0 0x1000          : read from sfp 1, slave 0, address 0x1000 and printout value\n", progname);
  printf ("\t  %s -r -x 0 3 0x1000 5        : read from sfp 0, slave 3, address 0x1000 next 5 words\n", progname);
  printf ("\t  %s -w -x 0 3 0x1000 0x2A     : write value 0x2A to sfp 0, slave 3, address 0x1000\n", progname);
  printf ("\t  %s -w -x 1 0 20000 AB FF     : write value 0xAB to sfp 1, slave 0, to addresses 0x20000-0x200FF\n",
      progname);
  printf ("\t  %s -s  0 0 0x200000 0x4      : set bit 100 on sfp0, slave 0, address 0x200000\n", progname);
  printf ("\t  %s -u  0 0 0x200000 0x4 0xFF : unset bit 100 on sfp0, slave 0, address 0x200000-0x2000FF\n", progname);
  printf ("*****************************************************************************\n");
  exit (0);
}

int goscmd_execute_command (struct gosip_cmd* com)
{
  int rev = 0;
  switch (com->command)
  {
    case GOSIP_RESET:
      rev = goscmd_reset (com);
      break;
    case GOSIP_INIT:
      rev = goscmd_init (com);
      break;
    case GOSIP_READ:
      rev = goscmd_read (com);
      break;
    case GOSIP_WRITE:
      rev = goscmd_write (com);
      break;
    case GOSIP_SETBIT:
    case GOSIP_CLEARBIT:
      rev = goscmd_changebits (com);
      break;
    case GOSIP_CONFIGURE:
      rev = goscmd_configure (com);
      break;
    case GOSIP_VERIFY:
      rev = goscmd_verify (com);
      break;
    default:
      printm ("Error: Unknown command %d \n", com->command);
      rev = -2;
      break;
  };
  return rev;
}

int main (int argc, char *argv[])
{
  int l_status;
  int opt;
  char cmd[GOSIP_CMD_MAX_ARGS][GOSIP_CMD_SIZE];
  unsigned int cmdLen = 0;
  unsigned int i;
  struct gosip_cmd theCommand;
  goscmd_defaults (&theCommand);

  /* get arguments*/
  optind = 1;
  while ((opt = getopt (argc, argv, "hzwrsuin:c:v:dx")) != -1)
  {
    switch (opt)
    {
      case '?':
        goscmd_usage (basename (argv[0]));
        exit (EXIT_FAILURE);
      case 'h':
        goscmd_usage (basename (argv[0]));
        exit (EXIT_SUCCESS);
      case 'n':
        theCommand.devnum = strtol (optarg, NULL, 0);
        break;
      case 'w':
        goscmd_set_command (&theCommand, GOSIP_WRITE);
        break;
      case 'r':
        goscmd_set_command (&theCommand, GOSIP_READ);
        break;
      case 's':
        goscmd_set_command (&theCommand, GOSIP_SETBIT);
        break;
      case 'u':
        goscmd_set_command (&theCommand, GOSIP_CLEARBIT);
        break;
      case 'z':
        goscmd_set_command (&theCommand, GOSIP_RESET);
        break;
      case 'i':
        goscmd_set_command (&theCommand, GOSIP_INIT);
        break;
      case 'c':
        goscmd_set_command (&theCommand, GOSIP_CONFIGURE);
        strncpy (theCommand.filename, optarg, GOSIP_MAXTEXT);
        break;
      case 'v':
        goscmd_set_command (&theCommand, GOSIP_VERIFY);
        strncpy (theCommand.filename, optarg, GOSIP_MAXTEXT);
        break;
      case 'd':
        theCommand.verboselevel = 1; /*strtol(optarg, NULL, 0); later maybe different verbose level*/
        break;
      case 'x':
        theCommand.hexformat = 1;
        break;
      default:
        break;
    }
  }

  /* get parameters:*/
  cmdLen = argc - optind;
  /*printf("- argc:%d optind:%d cmdlen:%d \n",argc, optind, cmdLen);*/
  goscmd_assert_arguments (&theCommand, cmdLen);
  for (i = 0; (i < cmdLen) && (i < GOSIP_CMD_MAX_ARGS); i++)
  {
    if (argv[optind + i])
      strncpy (cmd[i], argv[optind + i], GOSIP_CMD_SIZE);
    else
      printm ("warning: argument at position %d is empty!", optind + i);
  }
  if ((theCommand.command == GOSIP_CONFIGURE) || (theCommand.command == GOSIP_VERIFY))
  {
    // get list of addresses and values from file later

  }
  else
  {
    theCommand.sfp = strtoul (cmd[0], NULL, theCommand.hexformat == 1 ? 16 : 0);
    theCommand.slave = strtoul (cmd[1], NULL, theCommand.hexformat == 1 ? 16 : 0);

    // if ((theCommand.command == GOSIP_READ) || (theCommand.command == GOSIP_WRITE))
    theCommand.address = strtoul (cmd[2], NULL, theCommand.hexformat == 1 ? 16 : 0);
    if (cmdLen > 3)
    {
      if (theCommand.command == GOSIP_READ)
        theCommand.repeat = strtoul (cmd[3], NULL, theCommand.hexformat == 1 ? 16 : 0);
      else
        theCommand.value = strtoul (cmd[3], NULL, theCommand.hexformat == 1 ? 16 : 0);
    }
    if (cmdLen > 4)
    {
      theCommand.repeat = strtoul (cmd[4], NULL, theCommand.hexformat == 1 ? 16 : 0);
    }

  }
  goscmd_open_device (&theCommand);
  goscmd_assert_command (&theCommand);
  l_status = goscmd_execute_command (&theCommand);
  goscmd_close_device (&theCommand);

  return l_status;
}

