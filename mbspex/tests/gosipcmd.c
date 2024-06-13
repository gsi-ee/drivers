/**
 * \file
 *  Command line interface for gosip io protocol with mbxpex library
 * \author J.Adamczewski-Musch (j.adamczewski@gsi.de)
 * \date 26-Aug_2014 - 14-Jun-2024
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
  com->broadcast = 0;
  com->verify = 0;
  com->fd_pex = -1;
  com->sfp = -2;
  com->slave = -2;
  com->address = -1;
  com->value = 0;
  com->repeat = 1;
  com->errcount = 0;
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
  com->hexformat == 1 ? printm ("\t repeat: 0x%x", com->repeat) : printm ("\t repeat: %d", com->repeat);
  printm ("\t broadcast: %d \n", com->broadcast);

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
  if ((com->command == GOSIP_SETSPEED) && (arglen < 2))
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
  if ((com->command != GOSIP_CONFIGURE) && (com->command != GOSIP_VERIFY) && (com->command != GOSIP_RESET) && (com->command != GOSIP_SETSPEED))
  {
    if (com->sfp < -1) /*allow broadcast statements -1*/
      do_exit = 1;
    if ((com->command != GOSIP_INIT) && (com->slave < -1))
      do_exit = 1;
    if ((com->command != GOSIP_INIT) && (com->address < -1))
      do_exit = 1;
  }
  else
  {
    com->broadcast = 0;    // disable broadcast for configure, verify and reset if it was chosen by mistake
  }

  if ((com->command == GOSIP_READ || com->command==GOSIP_SETBIT || com->command==GOSIP_CLEARBIT)
        && (com->sfp == -1 || com->slave == -1))
  {
    com->broadcast = 1;    // allow implicit broadcast read switched by -1 argument for sfp or slave
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
    case GOSIP_SETSPEED:
      snprintf (CommandDescription, GOSIP_MAXTEXT, "Set link speed");
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
  return (fclose (com->configfile));
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
  com->sfp = strtol (cmd[0], NULL, com->hexformat == 1 ? 16 : 0);
  com->slave = strtol (cmd[1], NULL, com->hexformat == 1 ? 16 : 0);
  com->address = strtoul (cmd[2], NULL, com->hexformat == 1 ? 16 : 0);
  com->value = strtoul (cmd[3], NULL, com->hexformat == 1 ? 16 : 0);
  if (cmdlen > 4)
  {
    // parse optional command identifier
    //if (strcasestr (cmd[4], "setbit") != 0)
    //JAM2017 -  this was not posix standard, gcc 6.3 doesnt like it. we do workaround:
    if ((strstr (cmd[4], "setbit") != 0) ||  (strstr (cmd[4], "SETBIT") != 0))
    {
      com->command = GOSIP_SETBIT;
    }
    //else if (strcasestr (cmd[4], "clearbit") != 0)
    if ((strstr (cmd[4], "clearbit") != 0) ||  (strstr (cmd[4], "CLEARBIT") != 0))
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
  goscmd_assert_command (com);
  if (com->verboselevel)
    goscmd_dump_command (com);
  rev = mbspex_slave_wr (com->fd_pex, com->sfp, com->slave, com->address, com->value);
  if (rev != 0)
  {
    //printm ("ERROR on writing!\n"); /* we already have error output from mbspex*/
    //if (com->verboselevel)
    goscmd_dump_command (com);
  }
  return rev;
}

int goscmd_read (struct gosip_cmd* com)
{
  int rev = 0;
  goscmd_assert_command (com);
  if (com->verboselevel > 1)
    goscmd_dump_command (com);
  rev = mbspex_slave_rd (com->fd_pex, com->sfp, com->slave, com->address, &(com->value));
  if (rev == 0)
  {
    if (com->command == GOSIP_READ)
      goscmd_output (com);    // only do output if we have explicit read, suppress during verify
  }
  else
  {
    //printm ("ERROR on reading!\n"); /* we already have error output from mbspex*/
    //if (com->verboselevel)
    goscmd_dump_command (com);
  }
  return rev;
}

int goscmd_changebits (struct gosip_cmd* com)
{
  int rev = 0;
  long bitmask = 0;
  goscmd_assert_command (com);
  if (com->verboselevel)
    goscmd_dump_command (com);
  bitmask = com->value;
  rev = mbspex_slave_rd (com->fd_pex, com->sfp, com->slave, com->address, &(com->value));
  if (rev != 0)
  {
    printm ("ERROR on reading in change bit at address 0x%x!\n", com->address);
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
  }
  com->value= bitmask; // restore in case of broadcast command
  return rev;
}

int goscmd_busio (struct gosip_cmd* com)
{
  int rev = 0;
  int cursor = 0;
  long savedaddress = 0;
  goscmd_assert_command (com);
  if (com->verboselevel > 1)
    goscmd_dump_command (com);
  savedaddress = com->address;
  while (cursor < com->repeat)
  {
    switch (com->command)
    {
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
      default:
        printm ("NEVER COME HERE: goscmd_busio called with wrong command %s", goscmd_get_description (com));
        return -1;
    }

    cursor++;
    com->address += 4;
  }    // while
  com->address = savedaddress;    // restore initial base address for slave broadcast option
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

int goscmd_set_speed (struct gosip_cmd* com)
{
  goscmd_assert_command (com);
  if (com->verboselevel)
    goscmd_dump_command (com);
  // we misuse the slave parameter for the speed specification here
  return (mbspex_set_linkspeed (com->fd_pex, com->sfp, com->slave));
}


int goscmd_configure (struct gosip_cmd* com)
{
  int rev = 0;
#ifdef GOSIPCMD_BLOCKCONFIG
  int numconfs = 0;
  struct pex_bus_config theConfig;
#else

  gos_cmd_id mastercommand;
#endif
  goscmd_assert_command (com);
  if (com->verboselevel > 1)
    goscmd_dump_command (com);
  if (goscmd_open_configuration (com) < 0)
    return -1;
  printm ("Configuring from file %s - \n", com->filename);
  //mastercommand = com->command;
  numconfs = 0;
  while ((rev = goscmd_next_config_values (com)) != -1)
  {
    if (rev == 0)
      continue;    // skip line

    // TODO: put together configuration structure to be executed in kernel module "atomically"
    // requires new ioctl "configure"
#ifdef GOSIPCMD_BLOCKCONFIG
    if (com->verboselevel)
    {
      printm ("Config: %d", numconfs);
      goscmd_dump_command (com);
    }
    theConfig.param[numconfs].sfp = com->sfp;
    theConfig.param[numconfs].slave = com->slave;
    theConfig.param[numconfs].address = com->address;
    theConfig.param[numconfs].value = com->value;
    theConfig.numpars = ++numconfs;
    if (numconfs >= PEX_MAXCONFIG_VALS)
    {
        // JAM 2016: workaround for configurations above 60 entries:
        // need to send it in separate chunks
        rev = mbspex_slave_config (com->fd_pex, &theConfig);
        if(rev) break;
        numconfs = 0; // fill next config bundle
      // break;
    }
#else
    if ((com->command == GOSIP_SETBIT) || (com->command == GOSIP_CLEARBIT))
    {
      if ((rev=goscmd_changebits (com)) != 0)
      break;
      com->command = mastercommand;    // reset command descriptor
    }
    else
    {
      if ((rev=goscmd_write (com)) != 0)
      break;
    }
#endif

  }
#ifdef GOSIPCMD_BLOCKCONFIG
  rev = mbspex_slave_config (com->fd_pex, &theConfig);
#endif

  if (rev)
    printm ("ERROR during configuration!\n");
  else
    printm ("Done.\n");

  goscmd_close_configuration (com);
  return rev;

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
    if (rev == 0)
      continue;    // skip line

    if ((com->sfp == -1 || com->slave == -1))
    {
      com->broadcast = 1;    // allow implicit broadcast read switched by -1 argument for sfp or slave
      com->verify = 1;    // mark special mode
      rev = goscmd_execute_command (com);    // this will handle broadcast
    }
    else
    {
      rev = goscmd_verify_single (com);
    }
    if (rev)
    {
      printm ("Verify read ERROR for line %d from configuration file %s\n", com->linecount, com->filename);
    }
    com->command = mastercommand;    // reset command descriptor

  } // while file

  printm ("Verify found %d errors\n", com->errcount);
  goscmd_close_configuration (com);
  return 0;

}

int goscmd_verify_single (struct gosip_cmd* com)
{
  int haserror = 0;
  int checkvalue = com->value;    // this is reference value from file

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
    com->errcount++;
    if (com->verboselevel)
    {
      printm (" Verify ERROR %d at sfp %d slave %d address 0xd%x : readback=0x%x, config=0x%x", com->errcount, com->sfp,
          com->slave, com->address, com->value, checkvalue);
      if (com->command != GOSIP_VERIFY)
        printm (" (%s)\n", goscmd_get_description (com));
      else
        printm ("\n");
    }
  }
  com->value = checkvalue;    // restore my soul for broadcast mode
  return 0;

}

int goscmd_broadcast (struct gosip_cmd* com)
{
  int rev = 0;
  int slavebroadcast=0, sfpbroadcast = 0;
  long sfpmax;
  long slavemax;
  if (com->verboselevel)
    goscmd_dump_command (com);
  com->broadcast = 0;
  // TODO: add here mode for real broadcast over all configured slaves
  if (com->sfp < 0)
    sfpbroadcast = 1;
  if (com->slave < 0)
    slavebroadcast = 1;
  struct pex_sfp_links slavesetup;
  if (mbspex_get_configured_slaves (com->fd_pex, &slavesetup) < 0)
    return -1;
  if (sfpbroadcast) /* broadcast over sfps*/
  {
    for (com->sfp = 0; com->sfp < PEX_SFP_NUMBER; ++com->sfp)
    {
      slavemax = slavesetup.numslaves[com->sfp];
      if (slavemax == 0)
        continue;

      if (slavebroadcast) /* also broadcast all slaves*/
      {
        for (com->slave = 0; com->slave < slavemax; ++com->slave)
        {
          rev = goscmd_execute_command (com);
          if (rev != 0)
          {
            printm ("Error in sfp and slave broadcast at sfp:%d slave:%d\n", com->sfp, com->slave);
          }
        }
      }
      else /* single slave at all sfps:*/
      {
        rev = goscmd_execute_command (com);
        if (rev != 0)
        {
          printm ("Error in sfp broadcast at sfp:%d slave:%d\n", com->sfp, com->slave);
        }
      }

    }    // for com->sfp
  }    // if sfp broadcast
  else
  {

    if (slavebroadcast)
    {
      /* broadcast all slaves at single sfp:*/
      slavemax = slavesetup.numslaves[com->sfp];
      for (com->slave = 0; com->slave < slavemax; ++com->slave)
      {
        rev = goscmd_execute_command (com);
        if (rev != 0)
        {
          printm ("Error in slave broadcast at sfp:%d slave:%d\n", com->sfp, com->slave);
        }
      }
    }
    else
    {
      /* explicit broadcast over given boundaries: (old style)*/
      sfpmax = com->sfp;
      slavemax = com->slave;
      for (com->sfp = 0; com->sfp <= sfpmax; ++com->sfp)
      {
        for (com->slave = 0; com->slave <= slavemax; ++com->slave)
        {
          rev = goscmd_execute_command (com);
          if (rev != 0)
          {
            printm ("Error in broadcast (0..%d) (0..%d) at sfp:%d slave:%d\n", sfpmax, slavemax, com->sfp, com->slave);
          }
        }    // for slave
      }    // for sfp
    }    // end no slave broadcast
  }    // end sfp not broadcast
  return rev;
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
  printf (" v0.560 14-Jun-2024 by JAM (j.adamczewski@gsi.de)\n");
  printf ("***************************************************************************\n");
  printf (
      "  usage: %s [-h|-z] [[-i|-l|-r|-w|-s|-u] [-b] | [-c|-v FILE] [-n DEVICE |-d|-x] sfp [slave | speed] [address [value [words]|[words]]]] \n",
      progname);
  printf ("\t Options:\n");
  printf ("\t\t -h        : display this help\n");
  printf ("\t\t -z        : reset (zero) pexor/kinpex board \n");
  printf ("\t\t -i        : initialize sfp chain \n");
  printf ("\t\t -l        : set sfp chain Linkspeed \n");
  printf ("\t\t -r        : read from register \n");
  printf ("\t\t -w        : write to  register\n");
  printf ("\t\t -s        : set bits of given mask in  register\n");
  printf ("\t\t -u        : unset bits of given mask in  register\n");
  printf ("\t\t -b        : broadcast io operations to all slaves in range (0-sfp)(0-slave)\n");
  printf ("\t\t -c FILE   : configure registers with values from FILE.gos\n");
  printf ("\t\t -v FILE   : verify register contents (compare with FILE.gos)\n");
  printf ("\t\t -n DEVICE : specify device number N (/dev/pexorN, default:0) \n");
  printf ("\t\t -d        : debug mode (verbose output) \n");
  printf ("\t\t -x        : numbers in hex format (defaults: decimal, or defined by prefix 0x) \n");
  printf ("\t Arguments:\n");
  printf ("\t\t sfp      - sfp chain- -1 to broadcast all registered chains \n");
  printf ("\t\t slave    - slave id at chain, or total number of slaves. -1 for internal broadcast\n");
  printf ("\t\t address  - register on slave \n");
  printf ("\t\t value    - value to write on slave \n");
  printf ("\t\t words    - number of words to read/write/set incrementally\n");
  printf ("\t Examples:\n");
  printf ("\t  %s -z -n 1                   : master gosip reset of board /dev/pexor1 \n", progname);
  printf ("\t  %s -i 0 24                   : initialize chain at sfp 0 with 24 slave devices\n", progname);
  printf ("\t  %s -l -- -1 2                : set all sfp chains to linkspeed setup 2 (2.5 Gb)\n", progname);
  printf ("\t  %s -r -x 1 0 0x1000          : read from sfp 1, slave 0, address 0x1000 and printout value\n", progname);
  printf ("\t  %s -r -x 0 3 0x1000 5        : read from sfp 0, slave 3, address 0x1000 next 5 words\n", progname);
  printf (
      "\t  %s -r -b  1 3 0x1000 10      : broadcast read from sfp (0..1), slave (0..3), address 0x1000 next 10 words\n",
      progname);
  printf (
        "\t  %s -r -- -1 -1 0x1000 10     : broadcast read from address 0x1000, next 10 words from all registered slaves\n",
        progname);
  printf ("\t  %s -w -x 0 3 0x1000 0x2A     : write value 0x2A to sfp 0, slave 3, address 0x1000\n", progname);
  printf ("\t  %s -w -x 1 0 20000 AB FF     : write value 0xAB to sfp 1, slave 0, to addresses 0x20000-0x200FF\n",
      progname);
  printf (
      "\t  %s -w -b  1  3 0x20004c 1    : broadcast write value 1 to address 0x20004c on sfp (0..1) slaves (0..3)\n",
      progname);
  printf (
      "\t  %s -w -- -1 -1 0x20004c 1    : write value 1 to address 0x20004c on all registered slaves (internal driver broadcast)\n",
      progname);
  printf ("\t  %s -s  0 0 0x200000 0x4      : set bit 100 on sfp0, slave 0, address 0x200000\n", progname);
  printf ("\t  %s -u  0 0 0x200000 0x4 0xFF : unset bit 100 on sfp0, slave 0, address 0x200000-0x2000FF\n", progname);
  printf ("\t  %s -x -c run42.gos           : write configuration values from file run42.gos to slaves \n", progname);
  printf ("*****************************************************************************\n");
  exit (0);
}

int goscmd_execute_command (struct gosip_cmd* com)
{
  int rev = 0;

  if (com->broadcast)
    return goscmd_broadcast (com); /* dispatch from top level to broadcast mode*/
  if (com->verify)
    return goscmd_verify_single (com); /* for broadcast mode verify, if execute_command is invoked by goscmd_broadcast*/
  switch (com->command)
  {
    case GOSIP_RESET:
      rev = goscmd_reset (com);
      break;
    case GOSIP_INIT:
      rev = goscmd_init (com);
      break;
    case GOSIP_READ:
    case GOSIP_WRITE:
    case GOSIP_SETBIT:
    case GOSIP_CLEARBIT:
      rev = goscmd_busio (com);
      break;
    case GOSIP_CONFIGURE:
      rev = goscmd_configure (com);
      break;
    case GOSIP_VERIFY:
      rev = goscmd_verify (com);
      break;
    case GOSIP_SETSPEED:
      rev = goscmd_set_speed (com);
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
  while ((opt = getopt (argc, argv, "hzwrsuiln:c:v:dxb")) != -1)
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
      case 'l':
        goscmd_set_command (&theCommand, GOSIP_SETSPEED);
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
      case 'b':
        theCommand.broadcast = 1;
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
    theCommand.sfp = strtol (cmd[0], NULL, theCommand.hexformat == 1 ? 16 : 0);
    theCommand.slave = strtol (cmd[1], NULL, theCommand.hexformat == 1 ? 16 : 0); /* note: we might have negative values for broadcast here*/

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

#ifndef MBSPEX_NOMBS
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
