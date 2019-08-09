/**
 * \file
 *  Command line interface for galap io protocol with galapagos library
 * \author J.Adamczewski-Musch (j.adamczewski@gsi.de)
 * \date 5-Aug_2019
 *
 */


#include "galapcmd.h"
#include <string.h>

static char CommandDescription[GALAP_MAXTEXT];

void galcmd_defaults (struct galap_cmd* com)
{
  com->devnum = 0;
  com->command = GALAP_NOP;
  com->verboselevel = 0;
  com->hexformat = 0;
//  com->broadcast = 0;
  com->verify = 0;
  com->fd_gapg = -1;
  com->address = -1;
  com->value = 0;
  com->repeat = 1;
  com->errcount = 0;
}

void galcmd_set_command (struct galap_cmd* com, gos_cmd_id id)
{
  if (com->command == GALAP_NOP)
  {
    com->command = id;
  }
  else
  {
    printm (" galapcmd ERROR - conflicting command specifiers!\n");
    exit (1);
  }
}

void galcmd_dump_command (struct galap_cmd* com)
{
  //printm (" galapcmd dump: \n");
  com->hexformat == 1 ? printm (" Command  :0x%x (%s)", com->command, galcmd_get_description (com)) :
                        printm (" Command: %d (%s)", com->command, galcmd_get_description (com));
  com->hexformat == 1 ? printm ("\t device: 0x%x", com->devnum) : printm ("\t device: %d", com->devnum);
  com->hexformat == 1 ? printm ("\t address: 0x%x", com->address) : printm ("\t address: %d", com->address);
  com->hexformat == 1 ? printm ("\t value: 0x%x", com->value) : printm ("\t value: %d", com->value);
  com->hexformat == 1 ? printm ("\t repeat: 0x%x", com->repeat) : printm ("\t repeat: %d", com->repeat);

//  if ((com->command == GALAP_CONFIGURE) || (com->command == GALAP_VERIFY))
//  printm (" \t config file    :%s \n", com->filename);
}

void galcmd_assert_arguments (struct galap_cmd* com, int arglen)
{
  int do_exit = 0;
//  if ((com->command == GALAP_INIT) && (arglen < 2))
//    do_exit = 1;
  if ((com->command == GALAP_READ) && (arglen < 1))
    do_exit = 1;
  if ((com->command == GALAP_WRITE) && (arglen < 2))
    do_exit = 1;
  if ((com->command == GALAP_SETBIT) && (arglen < 2))
    do_exit = 1;
  if ((com->command == GALAP_CLEARBIT) && (arglen < 2))
    do_exit = 1;
  if (do_exit)
  {
    printm (" galapcmd ERROR - number of parameters not sufficient for command!\n");
    exit (1);
  }

}

void galcmd_assert_command (struct galap_cmd* com)
{
  int do_exit = 0;
  if (com == GALAP_NOP)
    do_exit = 1;
  if (com->fd_gapg < 0)
    do_exit = 1;
  if ((com->command != GALAP_CONFIGURE) && (com->command != GALAP_VERIFY) && (com->command != GALAP_RESET))
  {
    if ((com->address < -1))
      do_exit = 1;
  }

//  if ((com->command == GALAP_READ || com->command==GALAP_SETBIT || com->command==GALAP_CLEARBIT)
//        && (com->sfp == -1 || com->slave == -1))
//  {
//    com->broadcast = 1;    // allow implicit broadcast read switched by -1 argument for sfp or slave
//  }

  if (do_exit)
  {
    printm (" galapcmd ERROR - illegal parameters \n");
    galcmd_dump_command (com);
    exit (1);
  }
}

char* galcmd_get_description (struct galap_cmd* com)
{
  switch (com->command)
  {
    case GALAP_RESET:
      snprintf (CommandDescription, GALAP_MAXTEXT, "Reset board");
      break;
//    case GALAP_INIT:
//      snprintf (CommandDescription, GALAP_MAXTEXT, "Initialize sfp chain");
//      break;
    case GALAP_READ:
      snprintf (CommandDescription, GALAP_MAXTEXT, "Read value");
      break;
    case GALAP_WRITE:
      snprintf (CommandDescription, GALAP_MAXTEXT, "Write value");
      break;
    case GALAP_CONFIGURE:
      snprintf (CommandDescription, GALAP_MAXTEXT, "Configure");
      break;
    case GALAP_VERIFY:
      snprintf (CommandDescription, GALAP_MAXTEXT, "Verify   ");
      break;

    case GALAP_SETBIT:
      snprintf (CommandDescription, GALAP_MAXTEXT, "Set Bitmask");
      break;
    case GALAP_CLEARBIT:
      snprintf (CommandDescription, GALAP_MAXTEXT, "Clear Bitmask");
      break;
    default:
      snprintf (CommandDescription, GALAP_MAXTEXT, "Unknown command");
      break;
  };
  return CommandDescription;
}

int galcmd_open_device (struct galap_cmd* com)
{
  com->fd_gapg = galapagos_open (com->devnum);
  if (com->fd_gapg < 0)
  {
    printm ("ERROR>> open /dev/gapgor%d \n", com->devnum);
    exit (1);
  }
  return 0;
}

void galcmd_close_device (struct galap_cmd* com)
{
  close (com->fd_gapg);
}

int galcmd_open_configuration (struct galap_cmd* com)
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

int galcmd_close_configuration (struct galap_cmd* com)
{
  fclose (com->configfile);
}

int galcmd_next_config_values (struct galap_cmd* com)
{
  /* file parsing code was partially stolen from trbcmd.c Thanks Ludwig Maier et al. for this!*/
  int status = 0;
  size_t linelen = 0;
  char* cmdline;
  char cmd[GALAP_CMD_MAX_ARGS][GALAP_CMD_SIZE];
  int i, cmdlen;

  char *c = NULL;
  for (i = 0; i < GALAP_CMD_MAX_ARGS; i++)
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

  for (i = 0, cmdlen = 0; i < GALAP_CMD_MAX_ARGS; i++, cmdlen++)
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
//  com->sfp = strtol (cmd[0], NULL, com->hexformat == 1 ? 16 : 0);
//  com->slave = strtol (cmd[1], NULL, com->hexformat == 1 ? 16 : 0);
  com->address = strtoul (cmd[0], NULL, com->hexformat == 1 ? 16 : 0);
  com->value = strtoul (cmd[1], NULL, com->hexformat == 1 ? 16 : 0);
  if (cmdlen > 1)
  {
    // parse optional command identifier
    //if (strcasestr (cmd[4], "setbit") != 0)
    //JAM2017 -  this was not posix standard, gcc 6.3 doesnt like it. we do workaround:
    if ((strstr (cmd[2], "setbit") != 0) ||  (strstr (cmd[2], "SETBIT") != 0))
    {
      com->command = GALAP_SETBIT;
    }
    //else if (strcasestr (cmd[4], "clearbit") != 0)
    if ((strstr (cmd[2], "clearbit") != 0) ||  (strstr (cmd[2], "CLEARBIT") != 0))
    {
      com->command = GALAP_CLEARBIT;
    }
    else
    {
      // do not change mastercommand id (configure or verify)
    }

  }
  return status;
}

int galcmd_write (struct galap_cmd* com)
{
  int rev = 0;
  galcmd_assert_command (com);
  if (com->verboselevel)
    galcmd_dump_command (com);
  rev = galapagos_register_wr (com->fd_gapg,GAPG_REGISTERS_BAR,  com->address, com->value);
  if (rev != 0)
  {
    //printm ("ERROR on writing!\n"); /* we already have error output from galapagos*/
    //if (com->verboselevel)
    galcmd_dump_command (com);
  }
  return rev;
}

int galcmd_read (struct galap_cmd* com)
{
  int rev = 0;
  galcmd_assert_command (com);
  if (com->verboselevel > 1)
    galcmd_dump_command (com);
  rev = galapagos_register_rd (com->fd_gapg, GAPG_REGISTERS_BAR, com->address, &(com->value));
  if (rev == 0)
  {
    if (com->command == GALAP_READ)
      galcmd_output (com);    // only do output if we have explicit read, suppress during verify
  }
  else
  {
    //printm ("ERROR on reading!\n"); /* we already have error output from galapagos*/
    //if (com->verboselevel)
    galcmd_dump_command (com);
  }
  return rev;
}

int galcmd_changebits (struct galap_cmd* com)
{
  int rev = 0;
  long bitmask = 0;
  galcmd_assert_command (com);
  if (com->verboselevel)
    galcmd_dump_command (com);
  bitmask = com->value;
  rev = galapagos_register_rd (com->fd_gapg, GAPG_REGISTERS_BAR, com->address, &(com->value));
  if (rev != 0)
  {
    printm ("ERROR on reading in change bit at address 0x%x!\n", com->address);
  }
  switch (com->command)
  {
    case GALAP_SETBIT:
      com->value |= bitmask;
      break;
    case GALAP_CLEARBIT:
      com->value &= ~bitmask;
      break;
    default:
      break;
  }
  rev = galapagos_register_wr (com->fd_gapg, GAPG_REGISTERS_BAR, com->address, com->value);
  if (rev != 0)
  {
    printm ("ERROR on writing in change bit at address 0x%x!\n", com->address);
  }
  com->value= bitmask; // restore in case of broadcast command
  return rev;
}

int galcmd_busio (struct galap_cmd* com)
{
  int rev = 0;
  int cursor = 0;
  long savedaddress = 0;
  galcmd_assert_command (com);
  if (com->verboselevel > 1)
    galcmd_dump_command (com);
  savedaddress = com->address;
  while (cursor < com->repeat)
  {
    switch (com->command)
    {
      case GALAP_READ:
        rev = galcmd_read (com);
        break;
      case GALAP_WRITE:
        rev = galcmd_write (com);
        break;
      case GALAP_SETBIT:
      case GALAP_CLEARBIT:
        rev = galcmd_changebits (com);
        break;
      default:
        printm ("NEVER COME HERE: galcmd_busio called with wrong command %s", galcmd_get_description (com));
        return -1;
    }

    cursor++;
    com->address += 4;
  }    // while
  com->address = savedaddress;    // restore initial base address for slave broadcast option
  return rev;
}

int galcmd_reset (struct galap_cmd* com)
{
  galcmd_assert_command (com);
  if (com->verboselevel)
    galcmd_dump_command (com);
  return (galapagos_reset (com->fd_gapg));
}

//int galcmd_init (struct galap_cmd* com)
//{
//  galcmd_assert_command (com);
//  if (com->verboselevel)
//    galcmd_dump_command (com);
//  return (galapagos_slave_init (com->fd_gapg, com->sfp, com->slave));
//}

int galcmd_configure (struct galap_cmd* com)
{
  int rev = 0;
#ifdef GALAPCMD_BLOCKCONFIG
  int numconfs = 0;
  struct gapg_bus_config theConfig;
#else

  gos_cmd_id mastercommand;
#endif
  galcmd_assert_command (com);
  if (com->verboselevel > 1)
    galcmd_dump_command (com);
  if (galcmd_open_configuration (com) < 0)
    return -1;
  printm ("Configuring from file %s - \n", com->filename);
  //mastercommand = com->command;
  //numconfs = 0;
  while ((rev = galcmd_next_config_values (com)) != -1)
  {
    if (rev == 0)
      continue;    // skip line

    // TODO: put together configuration structure to be executed in kernel module "atomically"
    // requires new ioctl "configure"
#ifdef GALAPCMD_BLOCKCONFIG
    if (com->verboselevel)
    {
      printm ("Config: %d", numconfs);
      galcmd_dump_command (com);
    }
    theConfig.param[numconfs].sfp = com->sfp;
    theConfig.param[numconfs].slave = com->slave;
    theConfig.param[numconfs].address = com->address;
    theConfig.param[numconfs].value = com->value;
    theConfig.numpars = ++numconfs;
    if (numconfs >= GAPG_MAXCONFIG_VALS)
    {
        // JAM 2016: workaround for configurations above 60 entries:
        // need to send it in separate chunks
        rev = galapagos_slave_config (com->fd_gapg, &theConfig);
        if(rev) break;
        numconfs = 0; // fill next config bundle
      // break;
    }
#else
    if ((com->command == GALAP_SETBIT) || (com->command == GALAP_CLEARBIT))
    {
      if ((rev=galcmd_changebits (com)) != 0)
      break;
      com->command = mastercommand;    // reset command descriptor
    }
    else
    {
      if ((rev=galcmd_write (com)) != 0)
      break;
    }
#endif

  }
#ifdef GALAPCMD_BLOCKCONFIG
  rev = galapagos_slave_config (com->fd_gapg, &theConfig);
#endif

  if (rev)
    printm ("ERROR during configuration!\n");
  else
    printm ("Done.\n");

  galcmd_close_configuration (com);
  return rev;

}

int galcmd_verify (struct galap_cmd* com)
{
  long checkvalue = 0;
  int errcount = 0;
  int rev = 0;
  gos_cmd_id mastercommand;

  galcmd_assert_command (com);
  mastercommand = com->command;
  if (com->verboselevel > 1)
    galcmd_dump_command (com);
  if (galcmd_open_configuration (com) < 0)
    return -1;
  printm ("Verified actual configuration with file %s - \n", com->filename);
  while ((rev = galcmd_next_config_values (com)) != -1)
  {
    if (rev == 0)
      continue;    // skip line

//    if ((com->sfp == -1 || com->slave == -1))
//    {
//      com->broadcast = 1;    // allow implicit broadcast read switched by -1 argument for sfp or slave
//      com->verify = 1;    // mark special mode
//      rev = galcmd_execute_command (com);    // this will handle broadcast
//    }
//    else
    {
      rev = galcmd_verify_single (com);
    }
    if (rev)
    {
      printm ("Verify read ERROR for line %d from configuration file %s\n", com->linecount, com->filename);
    }
    com->command = mastercommand;    // reset command descriptor

  } // while file

  printm ("Verify found %d errors\n", com->errcount);
  galcmd_close_configuration (com);
  return 0;

}

int galcmd_verify_single (struct galap_cmd* com)
{
  int haserror = 0;
  int checkvalue = com->value;    // this is reference value from file

  if (galcmd_read (com) != 0)
    return -1;

  switch (com->command)
  {
    case GALAP_SETBIT:
      if ((checkvalue & com->value) != checkvalue)
        haserror = 1;
      break;
    case GALAP_CLEARBIT:
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
    com->errcount++;
    if (com->verboselevel)
    {
      printm (" Verify ERROR %d at address 0xd%x : readback=0x%x, config=0x%x", com->errcount, com->address, com->value, checkvalue);
      if (com->command != GALAP_VERIFY)
        printm (" (%s)\n", galcmd_get_description (com));
      else
        printm ("\n");
    }
  }
  com->value = checkvalue;    // restore my soul for broadcast mode
  return 0;

}

//int galcmd_broadcast (struct galap_cmd* com)
//{
//  int rev = 0;
//  int slavebroadcast, sfpbroadcast = 0;
//  long sfpmax;
//  long slavemax;
//  if (com->verboselevel)
//    galcmd_dump_command (com);
//  com->broadcast = 0;
//  // TODO: add here mode for real broadcast over all configured slaves
//  if (com->sfp < 0)
//    sfpbroadcast = 1;
//  if (com->slave < 0)
//    slavebroadcast = 1;
//  struct gapg_sfp_links slavesetup;
//  if (galapagos_get_configured_slaves (com->fd_gapg, &slavesetup) < 0)
//    return -1;
//  if (sfpbroadcast) /* broadcast over sfps*/
//  {
//    for (com->sfp = 0; com->sfp < GAPG_SFP_NUMBER; ++com->sfp)
//    {
//      slavemax = slavesetup.numslaves[com->sfp];
//      if (slavemax == 0)
//        continue;
//
//      if (slavebroadcast) /* also broadcast all slaves*/
//      {
//        for (com->slave = 0; com->slave < slavemax; ++com->slave)
//        {
//          rev = galcmd_execute_command (com);
//          if (rev != 0)
//          {
//            printm ("Error in sfp and slave broadcast at sfp:%d slave:%d\n", com->sfp, com->slave);
//          }
//        }
//      }
//      else /* single slave at all sfps:*/
//      {
//        rev = galcmd_execute_command (com);
//        if (rev != 0)
//        {
//          printm ("Error in sfp broadcast at sfp:%d slave:%d\n", com->sfp, com->slave);
//        }
//      }
//
//    }    // for com->sfp
//  }    // if sfp broadcast
//  else
//  {
//
//    if (slavebroadcast)
//    {
//      /* broadcast all slaves at single sfp:*/
//      slavemax = slavesetup.numslaves[com->sfp];
//      for (com->slave = 0; com->slave < slavemax; ++com->slave)
//      {
//        rev = galcmd_execute_command (com);
//        if (rev != 0)
//        {
//          printm ("Error in slave broadcast at sfp:%d slave:%d\n", com->sfp, com->slave);
//        }
//      }
//    }
//    else
//    {
//      /* explicit broadcast over given boundaries: (old style)*/
//      sfpmax = com->sfp;
//      slavemax = com->slave;
//      for (com->sfp = 0; com->sfp <= sfpmax; ++com->sfp)
//      {
//        for (com->slave = 0; com->slave <= slavemax; ++com->slave)
//        {
//          rev = galcmd_execute_command (com);
//          if (rev != 0)
//          {
//            printm ("Error in broadcast (0..%d) (0..%d) at sfp:%d slave:%d\n", sfpmax, slavemax, com->sfp, com->slave);
//          }
//        }    // for slave
//      }    // for sfp
//    }    // end no slave broadcast
//  }    // end sfp not broadcast
//  return rev;
//}

int galcmd_output (struct galap_cmd* com)
{
  if (com->verboselevel)
  {
    com->hexformat ?
        printm ("Address: 0x%x  Data: 0x%x \n", com->address, com->value) :
        printm ("Address: %d  Data: %d \n", com->address, com->value);
  }
  else
  {
    com->hexformat ? printm ("0x%x \n", com->value) : printm ("%d \n", com->value);
  }
}

void galcmd_usage (const char *progname)
{
  printf ("***************************************************************************\n");

  printf (" %s for galapagos library  \n", progname);
  printf (" v0.1 5-Aug-2019 by JAM (j.adamczewski@gsi.de)\n");
  printf ("***************************************************************************\n");
  printf (
      "  usage: %s [-h|-z] [[-i|-r|-w|-s|-u] [-b] | [-c|-v FILE] [-n DEVICE |-d|-x] sfp slave [address [value [words]|[words]]]] \n",
      progname);
  printf ("\t Options:\n");
  printf ("\t\t -h        : display this help\n");
  printf ("\t\t -z        : reset (zero) galapagos board \n");
  printf ("\t\t -r        : read from register \n");
  printf ("\t\t -w        : write to  register\n");
  printf ("\t\t -s        : set bits of given mask in  register\n");
  printf ("\t\t -u        : unset bits of given mask in  register\n");
  printf ("\t\t -c FILE   : configure registers with values from FILE.gal\n");
  printf ("\t\t -v FILE   : verify register contents (compare with FILE.gal)\n");
  printf ("\t\t -n DEVICE : specify device number N (/dev/galapagosN, default:0) \n");
  printf ("\t\t -d        : debug mode (verbose output) \n");
  printf ("\t\t -x        : numbers in hex format (defaults: decimal, or defined by prefix 0x) \n");
  printf ("\t Arguments:\n");
  printf ("\t\t address  - register on board \n");
  printf ("\t\t value    - value to write on slave \n");
  printf ("\t\t words    - number of words to read/write/set incrementally\n");
  printf ("\t Examples:\n");
  printf ("\t  %s -z -n 1               : master galap reset of board /dev/galapagos1 \n", progname);
  printf ("\t  %s -r -x 0x1000          : read from  address 0x1000 and printout value\n", progname);
  printf ("\t  %s -r -x 0x1000 5        : read from  address 0x1000 next 5 words\n", progname);
  printf ("\t  %s -w -x 0x1000 0x2A     : write value 0x2A to address 0x1000\n", progname);
  printf ("\t  %s -w -x 20000 AB FF     : write value 0xAB to addresses 0x20000-0x200FF\n",
      progname);
  printf ("\t  %s -s  0x200000 0x4      : set bit 100 on address 0x200000\n", progname);
  printf ("\t  %s -u  0x200000 0x4 0xFF : unset bit 100 on address range 0x200000-0x2000FF\n", progname);
  printf ("\t  %s -x -c run42.gal       : write configuration values from file run42.gal\n", progname);
  printf ("*****************************************************************************\n");
  exit (0);
}

int galcmd_execute_command (struct galap_cmd* com)
{
  int rev = 0;

//  if (com->broadcast)
//    return galcmd_broadcast (com); /* dispatch from top level to broadcast mode*/
  if (com->verify)
    return galcmd_verify_single (com); /* for broadcast mode verify, if execute_command is invoked by galcmd_broadcast*/
  switch (com->command)
  {
    case GALAP_RESET:
      rev = galcmd_reset (com);
      break;
//    case GALAP_INIT:
//      rev = galcmd_init (com);
      break;
    case GALAP_READ:
    case GALAP_WRITE:
    case GALAP_SETBIT:
    case GALAP_CLEARBIT:
      rev = galcmd_busio (com);
      break;
    case GALAP_CONFIGURE:
      rev = galcmd_configure (com);
      break;
    case GALAP_VERIFY:
      rev = galcmd_verify (com);
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
  char cmd[GALAP_CMD_MAX_ARGS][GALAP_CMD_SIZE];
  unsigned int cmdLen = 0;
  unsigned int i;
  struct galap_cmd theCommand;
  galcmd_defaults (&theCommand);

  /* get arguments*/
  optind = 1;
  while ((opt = getopt (argc, argv, "hzwrsuin:c:v:dx")) != -1)
  {
    switch (opt)
    {
      case '?':
        galcmd_usage (basename (argv[0]));
        exit (EXIT_FAILURE);
      case 'h':
        galcmd_usage (basename (argv[0]));
        exit (EXIT_SUCCESS);
      case 'n':
        theCommand.devnum = strtol (optarg, NULL, 0);
        break;
      case 'w':
        galcmd_set_command (&theCommand, GALAP_WRITE);
        break;
      case 'r':
        galcmd_set_command (&theCommand, GALAP_READ);
        break;
      case 's':
        galcmd_set_command (&theCommand, GALAP_SETBIT);
        break;
      case 'u':
        galcmd_set_command (&theCommand, GALAP_CLEARBIT);
        break;
      case 'z':
        galcmd_set_command (&theCommand, GALAP_RESET);
        break;
//      case 'i':
//        galcmd_set_command (&theCommand, GALAP_INIT);
//        break;
      case 'c':
        galcmd_set_command (&theCommand, GALAP_CONFIGURE);
        strncpy (theCommand.filename, optarg, GALAP_MAXTEXT);
        break;
      case 'v':
        galcmd_set_command (&theCommand, GALAP_VERIFY);
        strncpy (theCommand.filename, optarg, GALAP_MAXTEXT);
        break;
      case 'd':
        theCommand.verboselevel = 1; /*strtol(optarg, NULL, 0); later maybe different verbose level*/
        break;
      case 'x':
        theCommand.hexformat = 1;
        break;
//      case 'b':
//        theCommand.broadcast = 1;
//        break;
      default:
        break;
    }
  }

  /* get parameters:*/
  cmdLen = argc - optind;
  /*printf("- argc:%d optind:%d cmdlen:%d \n",argc, optind, cmdLen);*/
  galcmd_assert_arguments (&theCommand, cmdLen);
  for (i = 0; (i < cmdLen) && (i < GALAP_CMD_MAX_ARGS); i++)
  {
    if (argv[optind + i])
      strncpy (cmd[i], argv[optind + i], GALAP_CMD_SIZE);
    else
      printm ("warning: argument at position %d is empty!", optind + i);
  }
  if ((theCommand.command == GALAP_CONFIGURE) || (theCommand.command == GALAP_VERIFY))
  {
    // get list of addresses and values from file later
  }
  else
  {
//    theCommand.sfp = strtol (cmd[0], NULL, theCommand.hexformat == 1 ? 16 : 0);
//    theCommand.slave = strtol (cmd[1], NULL, theCommand.hexformat == 1 ? 16 : 0); /* note: we might have negative values for broadcast here*/

    // if ((theCommand.command == GALAP_READ) || (theCommand.command == GALAP_WRITE))
    theCommand.address = strtoul (cmd[0], NULL, theCommand.hexformat == 1 ? 16 : 0);
    if (cmdLen > 1)
    {
      if (theCommand.command == GALAP_READ)
        theCommand.repeat = strtoul (cmd[1], NULL, theCommand.hexformat == 1 ? 16 : 0);
      else
        theCommand.value = strtoul (cmd[1], NULL, theCommand.hexformat == 1 ? 16 : 0);
    }
    if (cmdLen > 2)
    {
      theCommand.repeat = strtoul (cmd[2], NULL, theCommand.hexformat == 1 ? 16 : 0);
    }

  }
  galcmd_open_device (&theCommand);
  galcmd_assert_command (&theCommand);
  l_status = galcmd_execute_command (&theCommand);
  galcmd_close_device (&theCommand);

  return l_status;
}

#ifndef GALAPAGOS_NOMBS
/*****************************************************************/
/* here separate definition of printm:*/
#include <stdarg.h>

void printm (char *fmt, ...)
{
  char c_str[256];
  va_list args;
  va_start(args, fmt);
  vsprintf (c_str, fmt, args);
  printf ("%s", c_str);
  va_end(args);
}
#endif
