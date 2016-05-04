#include "pexornet.h"


int pexornet_ioctl_init_bus (struct pexornet_privdata* priv, unsigned long arg)
{
  int retval = 0;
  unsigned long flags=0;
  u32 sfp = 0;/*,comm=0;*/
  int slave = 0;
  struct pexornet_bus_io descriptor;
  struct pexornet_sfp* sfpregisters = &(priv->registers.sfp);
  retval = copy_from_user (&descriptor, (void __user *) arg, sizeof(struct pexornet_bus_io));
  if (retval) goto out;

  sfp = (u32) descriptor.sfp;    // sfp connection to initialize chain
  slave = descriptor.slave;    // maximum # of connected slave boards
  // for pex standard sfp code, we use this ioctl to initalize chain of slaves:
  pexornet_gosip_lock(&(priv->gosip_lock), flags);
  retval = pexornet_sfp_clear_channel (priv, sfp);
  if (retval) goto gosip_unlock;
  retval = pexornet_sfp_init_request (priv, sfp, slave);
  if (retval) goto gosip_unlock;
  sfpregisters->num_slaves[sfp] = slave; /* keep track of existing slaves for configuration broadcast*/

  gosip_unlock:
    pexornet_gosip_unlock(&(priv->gosip_lock), flags);
  out:
    return retval;

}

int pexornet_ioctl_get_sfp_links (struct pexornet_privdata* priv, unsigned long arg)
{
  int retval = 0;
  u32 sfp = 0;
  struct pexornet_sfp_links descriptor;
  struct pexornet_sfp* sfpregisters = &(priv->registers.sfp);
  for (sfp = 0; sfp < PEXORNET_SFP_NUMBER; ++sfp)
  {
    descriptor.numslaves[sfp] = sfpregisters->num_slaves[sfp];
  }
  retval = copy_to_user ((void __user *) arg, &descriptor, sizeof(struct pexornet_sfp_links));
  return retval;
}

int pexornet_ioctl_write_bus (struct pexornet_privdata* priv, unsigned long arg)
{
  int retval = 0;
  unsigned long flags=0;
  struct pexornet_bus_io descriptor;
  retval = copy_from_user (&descriptor, (void __user *) arg, sizeof(struct pexornet_bus_io));
  if (retval)
    return retval;
  pexornet_gosip_lock(&(priv->gosip_lock), flags);
    retval = pexornet_sfp_broadcast_write_bus (priv, &descriptor); /* everything is subfunctions now*/
  pexornet_gosip_unlock(&(priv->gosip_lock), flags);
  if(priv->sfp_buswait) udelay(priv->sfp_buswait); // delay after each user bus ioctl to adjust frontend speed
  if (retval)
    return retval;
  retval = copy_to_user ((void __user *) arg, &descriptor, sizeof(struct pexornet_bus_io));
  return retval;
}

int pexornet_ioctl_read_bus (struct pexornet_privdata* priv, unsigned long arg)
{
  int retval = 0;
  unsigned long flags=0;
  struct pexornet_bus_io descriptor;
  retval = copy_from_user (&descriptor, (void __user *) arg, sizeof(struct pexornet_bus_io));
  if (retval)
    return retval;
  pexornet_gosip_lock(&(priv->gosip_lock), flags);
    retval = pexornet_sfp_read_bus (priv, &descriptor); /* everything is subfunctions now*/
  pexornet_gosip_unlock(&(priv->gosip_lock), flags);
  if(priv->sfp_buswait) udelay(priv->sfp_buswait); // delay after each user bus ioctl to adjust frontend speed
  if (retval)
    return retval;
  retval = copy_to_user ((void __user *) arg, &descriptor, sizeof(struct pexornet_bus_io));

  return retval;
}

int pexornet_ioctl_configure_bus (struct pexornet_privdata* priv, unsigned long arg)
{
  int retval = 0, i = 0;
  unsigned long flags=0;
  struct pexornet_bus_config descriptor;
  retval = copy_from_user (&descriptor, (void __user *) arg, sizeof(struct pexornet_bus_config));
  if (retval)
    return retval;
  if (descriptor.numpars > PEXORNET_MAXCONFIG_VALS)
  {
    pexornet_msg(
        KERN_ERR "** pexornet_ioctl_configure_bus: warning too many parameters %d , reduced to %d\n", descriptor.numpars, PEXORNET_MAXCONFIG_VALS);
    descriptor.numpars = PEXORNET_MAXCONFIG_VALS;
  }
  pexornet_dbg(KERN_NOTICE "** pexornet_ioctl_configure_bus with %d parameters\n", descriptor.numpars);
  pexornet_gosip_lock(&(priv->gosip_lock), flags);
    for (i = 0; i < descriptor.numpars; ++i)
    {
      struct pexornet_bus_io data = descriptor.param[i];
      retval = pexornet_sfp_broadcast_write_bus (priv, &data);
      if(priv->sfp_buswait) udelay(priv->sfp_buswait); // delay after each user bus ioctl to adjust frontend speed
      if (retval)
      {
        pexornet_msg(
            KERN_ERR "** pexornet_ioctl_configure_bus: error %d at pexornet_sfp_broadcast_write_bus for value i=%d\n", retval, i);
        pexornet_gosip_unlock(&(priv->gosip_lock), flags);
        return retval;
      }
    }
  pexornet_gosip_unlock(&(priv->gosip_lock), flags);
  mb();
  udelay(1000);
  /* set waitstate after configure*/
  return retval;
}

int pexornet_sfp_broadcast_write_bus (struct pexornet_privdata* priv, struct pexornet_bus_io* data)
{
  int retval = 0, i = 0, sl = 0, sfp = 0;
  char sfpbroadcast = 0, slavebroadcast = 0;
  unsigned long address = 0, value = 0;
  struct pexornet_sfp* sfpregisters = &(priv->registers.sfp);
  address = data->address;
  value = data->value; /* save this because pexornet_bus_io will be changed by write bus!*/
  mb();
  if (data->sfp < 0)
    sfpbroadcast = 1;
  if (data->slave < 0)
    slavebroadcast = 1;
  pexornet_dbg(KERN_NOTICE "** pexornet_sfp_broadcast_write_bus with sfpbroadcast %d slavebroadcast %d \n", sfpbroadcast,slavebroadcast);
  if (sfpbroadcast)
  {
    pexornet_dbg(KERN_NOTICE "** pexornet_sfp_broadcast_write_bus with sfpbroadcast\n");

    for (sfp = 0; sfp < PEXORNET_SFP_NUMBER; ++sfp)
    {
      data->sfp = sfp;
      if (sfpregisters->num_slaves[sfp] == 0)
        continue;
      if (slavebroadcast)
      {
        for (sl = 0; sl < sfpregisters->num_slaves[sfp]; ++sl)
        {
          data->slave = sl;
          data->address = address;
          data->value = value;
          retval = pexornet_sfp_write_bus (priv, data);
          if (retval)
          {
            pexornet_msg(
                KERN_ERR "** pexornet_sfp_broadcast_write_bus: error %d at pexornet_sfp_write_bus for value i=%d, sfp:%d slave:%ld \n", retval, i, data->sfp, data->slave);
            continue;
          }
        }
      }    // slavebroadcast
      else
      {
        data->address = address;
        data->value = value;
        retval = pexornet_sfp_write_bus (priv, data);
        if (retval)
        {
          pexornet_msg(
              KERN_ERR "** pexornet_sfp_broadcast_write_bus: error %d at pexornet_sfp_write_bus for value i=%d, sfp:%d slave:%ld \n", retval, i, data->sfp, data->slave);
          continue;
        }
      }
    }

  }
  else if (slavebroadcast)
  {
    for (sl = 0; sl < sfpregisters->num_slaves[sfp]; ++sl)
    {
      data->slave = sl;
      data->address = address;
      data->value = value;
      retval = pexornet_sfp_write_bus (priv, data);
      if (retval)
      {
        pexornet_msg(
            KERN_ERR "** pexornet_sfp_broadcast_write_bus: error %d at pexornet_sfp_write_bus for value i=%d, sfp:%d slave:%ld \n", retval, i, data->sfp, data->slave);
        continue;
      }
    }

  }
  else
  {
    /* single write, no broadcast loop*/
    retval = pexornet_sfp_write_bus (priv, data);
    if (retval)
    {
      pexornet_msg(KERN_ERR "** pexornet_sfp_broadcast_write_bus: error %d at pexornet_sfp_write_bus for value i=%d\n", retval, i);
      return retval;
    }
  }

  return retval;
}

int pexornet_sfp_write_bus (struct pexornet_privdata* priv, struct pexornet_bus_io* descriptor)
{

  int retval = 0;
  u32 ad = 0, val = 0, sfp = 0, slave = 0, comm = 0;
  u32 rstat = 0, radd = 0, rdat = 0;
  u32 totaladdress = 0;
  ad = (u32) descriptor->address;
  val = (u32) descriptor->value;
  sfp = (u32) descriptor->sfp;
  slave = (u32) descriptor->slave;
  pexornet_dbg(KERN_NOTICE "** pexornet_sfp_write_bus writes value %x to address %x on sfp %x, slave %x\n", val, ad, sfp, slave);

  comm = PEXORNET_SFP_PT_AD_W_REQ | (0x1 << (16 + sfp));
  totaladdress = ad + (slave << 24);
  pexornet_sfp_clear_all (priv);
  //pexornet_sfp_clear_channel(priv,sfp);
  pexornet_sfp_request (priv, comm, totaladdress, val);
  //if((retval=pexornet_sfp_get_reply(priv, sfp, &rstat, &radd, &rdat, 0))!=0) // debug: no response check
  if ((retval = pexornet_sfp_get_reply (priv, sfp, &rstat, &radd, &rdat, PEXORNET_SFP_PT_AD_W_REP)) != 0)
  {
    pexornet_msg(KERN_ERR "** pexornet_sfp_write_bus: error %d at sfp_reply \n", retval);
    pexornet_msg(KERN_ERR "   pexornet_sfp_write_bus: incorrect reply: 0x%x 0x%x 0x%x \n", rstat, radd, rdat);
    return -EIO;
  }
  descriptor->value = rstat;
  descriptor->address = radd;
  return 0;
}

int pexornet_sfp_read_bus (struct pexornet_privdata* priv, struct pexornet_bus_io* descriptor)
{
  int retval = 0;
  u32 ad = 0, chan = 0, slave = 0, comm = 0;
  u32 rstat = 0, radd = 0, rdat = 0;
  u32 totaladdress = 0;
  ad = (u32) descriptor->address;
  chan = (u32) descriptor->sfp;
  slave = (u32) descriptor->slave;
  pexornet_dbg(KERN_NOTICE "** pexornet_sfp_read_bus from_address %x on sfp %x, slave %x\n", ad, chan, slave);
  comm = PEXORNET_SFP_PT_AD_R_REQ | (0x1 << (16 + chan));
  totaladdress = ad + (slave << 24);
  pexornet_sfp_clear_channel (priv, chan);
  pexornet_sfp_request (priv, comm, totaladdress, 0);
  //if((retval=pexornet_sfp_get_reply(priv, chan, &rstat, &radd, &rdat, 0))!=0) // debug:  no check
  if ((retval = pexornet_sfp_get_reply (priv, chan, &rstat, &radd, &rdat, PEXORNET_SFP_PT_AD_R_REP)) != 0)
  {
    pexornet_msg(KERN_ERR "** pexornet_sfp_read_bus: error %d at sfp_reply \n", retval);
    pexornet_msg(KERN_ERR "    incorrect reply: 0x%x 0x%x 0x%x \n", rstat, radd, rdat)
    return -EIO;
  }

  descriptor->value = rdat;
  return 0;
}



void pexornet_sfp_reset (struct pexornet_privdata* privdata)
{
  int i;
  struct pexornet_sfp* sfp = &(privdata->registers.sfp);
  pexornet_dbg(KERN_NOTICE "**pexornet_sfp_reset\n");
  iowrite32 (0xF, sfp->reset);
  mb();
  pexornet_sfp_delay()
  ;
  iowrite32 (0, sfp->reset);
  mb();
  pexornet_sfp_delay()
  ;
  /* reset number of slaves counter for broadcast*/
  for (i = 0; i < PEXORNET_SFP_NUMBER; ++i)
  {
    sfp->num_slaves[i] = 0;
  }

  udelay(10000);
  /* wait a while after reset until sfp is ready again!*/

}

void pexornet_sfp_request (struct pexornet_privdata* privdata, u32 comm, u32 addr, u32 data)
{
  struct pexornet_sfp* sfp = &(privdata->registers.sfp);
  pexornet_dbg(KERN_NOTICE "**pexornet_sfp_request, comm=%x, addr=%x data=%x\n", comm, addr, data);
  iowrite32 (addr, sfp->req_addr);
  pexornet_sfp_delay()
  ;
  iowrite32 (data, sfp->req_data);
  pexornet_sfp_delay()
  ;
  iowrite32 (comm, sfp->req_comm);
  pexornet_sfp_delay()
  ;
}

int pexornet_sfp_get_reply (struct pexornet_privdata* privdata, int ch, u32* comm, u32 *addr, u32 *data, u32 checkvalue)
{
  u32 status = 0, loopcount = 0;
  struct pexornet_sfp* sfp = &(privdata->registers.sfp);
  pexornet_dbg(KERN_NOTICE "**pexornet_sfp_get_reply ***\n");
  pexornet_sfp_assert_channel(ch);

  do
  {
    if (loopcount > privdata->sfp_maxpolls)/* 1000000*/
    {
      pexornet_msg(KERN_WARNING "**pexornet_sfp_get_reply polled %d times = %d ns without success, abort\n", loopcount, (loopcount* PEXORNET_SFP_DELAY));
      print_register (" ... status after FAILED pex_sfp_get_reply:", sfp->rep_stat[ch]);
	return -EIO;
    }
    status = ioread32 (sfp->rep_stat[ch]);
    pexornet_sfp_delay()
    ;

//    pexornet_dbg(KERN_NOTICE "**pexornet_sfp_get_reply in loop, count=%d \n",loopcount);
//    if (PEXORNET_DMA_POLL_SCHEDULE)
//      schedule (); /* probably this also may help, but must not be used from tasklet*/
//    pexornet_dbg(KERN_NOTICE "**pexornet_sfp_get_reply after schedule\n",loopcount);
	loopcount++;
  } while (((status & 0x3000) >> 12) != 0x02); /* packet received bit is set*/
  pexornet_dbg(KERN_NOTICE "**pexornet_sfp_get_reply after while loop with count=%d\n", loopcount);
  *comm = ioread32 (sfp->rep_stat[ch]);
  pexornet_sfp_delay()
  ;
  //pexornet_dbg(KERN_NOTICE "**pexornet_sfp_get_reply after reading comm \n");
  *addr = ioread32 (sfp->rep_addr[ch]);
  pexornet_sfp_delay()
  ;
  //pexornet_dbg(KERN_NOTICE "**pexornet_sfp_get_reply after reading addr \n");
  *data = ioread32 (sfp->rep_data[ch]);
  pexornet_sfp_delay()
  ;
  //pexornet_dbg(KERN_NOTICE "**pexornet_sfp_get_reply after reading dat \n");
  pexornet_dbg(KERN_NOTICE "pexornet_sfp_get_reply from SFP: %x got status:%x address:%x data: %x \n", ch,*comm, *addr, *data);
  if (checkvalue == 0)
    return 0;    // no check of reply structure

  //if ((*comm & 0xfff) == checkvalue)
  if ((*comm & checkvalue) == checkvalue)
  {
    if ((*comm & 0x4000) != 0)
    {
      pexornet_msg(KERN_ERR "pexornet_sfp_get_reply: ERROR: Packet Structure : Command Reply 0x%x \n", *comm);
      return -EIO;
    }
  }
  else
  {
    pexornet_msg(KERN_ERR "pexornet_sfp_get_reply: ERROR : Command Reply  0x%x is not matching expected value 0x%x\n", (*comm & 0xfff), checkvalue);
    return -EIO;
  }
  return 0;

}

int pexornet_sfp_get_token_reply (struct pexornet_privdata* privdata, int ch, u32* stat, u32* head, u32* foot)
{
  u32 status = 0, loopcount = 0;
  struct pexornet_sfp* sfp = &(privdata->registers.sfp);
  pexornet_dbg(KERN_NOTICE "**pexornet_sfp_get_token_reply ***\n");
  pexornet_sfp_assert_channel(ch);

  do
  {
    if (loopcount > privdata->sfp_maxpolls)
    {
      pexornet_msg(KERN_WARNING "**pexornet_sfp_get_token reply polled %d times = %d ns without success, abort\n", loopcount, (loopcount* PEXORNET_SFP_DELAY));
      print_register (" ... status after FAILED pex_sfp_get_token_reply:0x%x", sfp->tk_stat[ch]);
	  return -EIO;
    }
    status = ioread32 (sfp->tk_stat[ch]);
    pexornet_sfp_delay()
    ;

    loopcount++;
  } while (((status & 0x3000) >> 12) != 0x02); /* packet received bit is set*/

  *stat = ioread32 (sfp->tk_stat[ch]);
  pexornet_sfp_delay()
  ;
  *head = ioread32 (sfp->tk_head[ch]);
  pexornet_sfp_delay()
  ;
  *foot = ioread32 (sfp->tk_foot[ch]);
  pexornet_sfp_delay()
  ;
  pexornet_dbg(KERN_NOTICE "pexornet_sfp_get_token_reply from SFP: %x got token status:%x header:%x footer: %x \n", ch,*stat, *head, *foot );

  return 0;
}

int pexornet_sfp_init_request (struct pexornet_privdata* privdata, int ch, int numslaves)
{
  int retval = 0;
  u32 sfp = 0, comm = 0, maxslave = 0;
  u32 rstat = 0, radd = 0, rdat = 0;
  sfp = (u32) ch;
  maxslave = (u32) numslaves - 1; /* changed api: pass index of max slave, not number of slaves*/
  if (numslaves <= 0)
    maxslave = 0; /* catch possible user workaround for changed api*/
  pexornet_sfp_assert_channel(ch);
  comm = PEXORNET_SFP_INI_REQ | (0x1 << (16 + sfp));
  pexornet_dbg(KERN_NOTICE "**pexornet_sfp_init_request for channel %d with maxslave index=%d ***\n",ch, maxslave);
  pexornet_sfp_request (privdata, comm, 0, maxslave);
  //if ((retval = pexornet_sfp_get_reply (privdata, sfp, &rstat, &radd, &rdat, 0)) != 0)
  if((retval=pexornet_sfp_get_reply(privdata, sfp, &rstat, &radd, &rdat, PEXORNET_SFP_PT_INI_REP))!=0)
  {
    pexornet_msg(KERN_ERR "** pexornet_sfp_init_request: error %d at sfp_reply \n",retval);
    pexornet_msg(KERN_ERR "   pexornet_sfp_init_request: incorrect reply: 0x%x 0x%x 0x%x \n", rstat, radd, rdat);
    return -EIO;
  }
  return retval;
}

int pexornet_sfp_clear_all (struct pexornet_privdata* privdata)
{
  u32 status = 0, loopcount = 0, clrval;
  struct pexornet_sfp* sfp = &(privdata->registers.sfp);
  clrval = 0xf;
  pexornet_dbg(KERN_NOTICE "**pexornet_sfp_clear_all ***\n");
  /*iowrite32(clrval, sfp->rep_stat_clr);
   pexornet_sfp_delay();*/
  do
  {
    if (loopcount > privdata->sfp_maxpolls)
    {
      pexornet_msg(KERN_WARNING "**pexornet_sfp_clear_all tried  %d times = %d ns  without success, abort\n", loopcount, (loopcount* 2 * PEXORNET_SFP_DELAY));
      print_register (" ... stat_clr after FAILED pex_sfp_clear_all: 0x%x", sfp->rep_stat_clr);
      return -EIO;
    }
    iowrite32 (clrval, sfp->rep_stat_clr);
    pexornet_sfp_delay()
    ;
    status = ioread32 (sfp->rep_stat_clr);
    pexornet_sfp_delay()
    ;
    loopcount++;
  } while (status != 0x0);
  pexornet_dbg(KERN_INFO "**after pexornet_sfp_clear_all: loopcount:%d \n",loopcount);
  print_register (" ... stat_clr after pexornet_sfp_clear_all:", sfp->rep_stat_clr);
  return 0;
}

int pexornet_sfp_clear_channel (struct pexornet_privdata* privdata, int ch)
{
  u32 repstatus = 0, tokenstatus = 0, chstatus = 0, loopcount = 0, clrval;
  struct pexornet_sfp* sfp = &(privdata->registers.sfp);
  pexornet_dbg(KERN_NOTICE "**pexornet_sfp_clear_channel %d ***\n",ch);
  pexornet_sfp_assert_channel(ch);
  clrval = (0x1 << ch);
  /*iowrite32(clrval, sfp->rep_stat_clr);
   pexornet_sfp_delay();*/
  do
  {
    if (loopcount > privdata->sfp_maxpolls)
    {
      pexornet_msg(KERN_WARNING "**pexornet_sfp_clear_channel %d tried %d times = %d ns without success, abort\n", ch, loopcount, (loopcount* (2 * PEXORNET_SFP_DELAY+ 2 * PEXORNET_BUS_DELAY)));
      print_register (" ... reply status after FAILED pex_sfp_clear_channel:", sfp->rep_stat[ch]);
      print_register (" ... token reply status after FAILED pex_sfp_clear_channel:", sfp->tk_stat[ch]);
	 return -EIO;
    }

    iowrite32 (clrval, sfp->rep_stat_clr);
    pexornet_sfp_delay()
    ;
    repstatus = ioread32 (sfp->rep_stat[ch]) & 0xf000;
    pexornet_bus_delay();
    tokenstatus = ioread32 (sfp->tk_stat[ch]) & 0xf000;
    pexornet_bus_delay();
    chstatus = ioread32 (sfp->rep_stat_clr) & clrval;
    pexornet_sfp_delay()
    ; 
    loopcount++;

  } while ((repstatus != 0x0) || (tokenstatus != 0x0) || (chstatus != 0x0));

  pexornet_dbg(KERN_INFO "**after pexornet_sfp_clear_channel %d : loopcount:%d \n",ch,loopcount);
  /*print_register(" ... reply status:", sfp->rep_stat[ch]);
   print_register(" ... token reply status:", sfp->tk_stat[ch]);
   print_register(" ... statclr:", sfp->rep_stat_clr);*/

  return 0;
}

int pexornet_sfp_clear_channelpattern (struct pexornet_privdata* privdata, int pat)
{
  u32 repstatus = 0, loopcount = 0, clrval, mask;
  struct pexornet_sfp* sfp = &(privdata->registers.sfp);
  pexornet_dbg(KERN_NOTICE "**pexornet_sfp_clear_channel pattern 0x%x ***\n", pat);
  clrval = pat;
  mask = (pat << 8) | (pat << 4) | pat;
  do
  {
    if (loopcount > privdata->sfp_maxpolls)
    {
      pexornet_msg(
          KERN_WARNING "**pexornet_sfp_clear_channelpattern 0x%x tried %d  times = %d ns without success, abort\n", pat, loopcount, (loopcount* 2 * PEXORNET_SFP_DELAY));
	  print_register (" ... reply status after FAILED pex_sfp_clear_channelpattern:", sfp->rep_stat_clr);
      return -EIO;
    }
    iowrite32 (clrval, sfp->rep_stat_clr);
    pexornet_sfp_delay()
    ;
    repstatus = ioread32 (sfp->rep_stat_clr) & mask;
    pexornet_sfp_delay()
    ;
	loopcount++;
  } while ((repstatus != 0x0));

  pexornet_dbg(KERN_INFO "**after pex_sfp_clear_channelpattern 0x%x : loopcount:%d \n", pat, loopcount);
  /*print_register(" ... reply status:", sfp->rep_stat_clr); */
  return 0;
}

void set_sfp (struct pexornet_sfp* sfp, void* membase, unsigned long bar)
{
  int i = 0;
  void* sfpbase = 0;
  unsigned long offset;
  if (sfp == 0)
    return;
  sfpbase = membase + PEXORNET_SFP_BASE;
  sfp->version = (u32*) (sfpbase + PEXORNET_SFP_VERSION);
  sfp->req_comm = (u32*) (sfpbase + PEXORNET_SFP_REQ_COMM);
  sfp->req_addr = (u32*) (sfpbase + PEXORNET_SFP_REQ_ADDR);
  sfp->req_data = (u32*) (sfpbase + PEXORNET_SFP_REQ_DATA);
  sfp->rep_stat_clr = (u32*) (sfpbase + PEXORNET_SFP_REP_STAT_CLR);
  sfp->rx_moni = (u32*) (sfpbase + PEXORNET_SFP_RX_MONI);
  sfp->tx_stat = (u32*) (sfpbase + PEXORNET_SFP_TX_STAT);
  sfp->reset = (u32*) (sfpbase + PEXORNET_SFP_RX_RST);
  sfp->disable = (u32*) (sfpbase + PEXORNET_SFP_DISA);
  sfp->fault = (u32*) (sfpbase + PEXORNET_SFP_FAULT);
  for (i = 0; i < PEXORNET_SFP_NUMBER; ++i)
  {
    offset = i * 0x04;
    sfp->rep_stat[i] = (u32*) (sfpbase + PEXORNET_SFP_REP_STAT_0 + offset);
    sfp->rep_addr[i] = (u32*) (sfpbase + PEXORNET_SFP_REP_ADDR_0 + offset);
    sfp->rep_data[i] = (u32*) (sfpbase + PEXORNET_SFP_REP_DATA_0 + offset);
    sfp->fifo[i] = (u32*) (sfpbase + PEXORNET_SFP_FIFO_0 + offset);
    sfp->tk_stat[i] = (u32*) (sfpbase + PEXORNET_SFP_TOKEN_REP_STAT_0 + offset);
    sfp->tk_head[i] = (u32*) (sfpbase + PEXORNET_SFP_TOKEN_REP_HEAD_0 + offset);
    sfp->tk_foot[i] = (u32*) (sfpbase + PEXORNET_SFP_TOKEN_REP_FOOT_0 + offset);
    sfp->tk_dsize[i] = (u32*) (sfpbase + PEXORNET_SFP_TOKEN_DSIZE_0 + offset);
    sfp->tk_dsize_sel[i] = (u32*) (sfpbase + PEXORNET_SFP_TOKEN_DSIZE_SEL_0 + offset);
    sfp->tk_memsize[i] = (u32*) (sfpbase + PEXORNET_SFP_TOKEN_MEM_SIZE_0 + offset);
    offset = i * 0x40000;
    sfp->tk_mem[i] = (u32*) (membase + PEXORNET_SFP_TK_MEM_0 + offset);
    sfp->tk_mem_dma[i] = (dma_addr_t) (bar + PEXORNET_SFP_TK_MEM_0 + offset);
  }

}

void print_sfp (struct pexornet_sfp* sfp)
{
  int i = 0;
  if (sfp == 0)
    return;
  pexornet_dbg(KERN_NOTICE "##print_sfp: ###################\n");
  print_register ("version", sfp->version);
  print_register ("request command", sfp->req_comm);
  print_register ("request address", sfp->req_addr);
  print_register ("request data", sfp->req_data);

  print_register ("reply status /clear", sfp->rep_stat_clr);
  print_register ("monitor", sfp->rx_moni);
  print_register ("tx status", sfp->tx_stat);
  print_register ("reset", sfp->reset);
  print_register ("disable", sfp->disable);
  print_register ("fault", sfp->fault);
  for (i = 0; i < PEXORNET_SFP_NUMBER; ++i)
  {
    pexornet_dbg(KERN_NOTICE "-------- sfp number %d -------\n",i);
    print_register ("reply status", sfp->rep_stat[i]);
    print_register ("reply address", sfp->rep_addr[i]);
    print_register ("reply data", sfp->rep_data[i]);
    print_register ("fifo", sfp->fifo[i]);
    print_register ("token reply status", sfp->tk_stat[i]);
    print_register ("token reply header", sfp->tk_head[i]);
    print_register ("token reply footer", sfp->tk_foot[i]);
    print_register ("token data size", sfp->tk_dsize[i]);
    print_register ("token data size select", sfp->tk_dsize_sel[i]);
    print_register ("token mem size", sfp->tk_memsize[i]);
    print_register ("token mem start", sfp->tk_mem[i]);
    pexornet_dbg(KERN_NOTICE "token mem start DMA =%p \n",(void*) sfp->tk_mem_dma[i]);
  }

  pexornet_show_version (sfp, 0);
}

void pexornet_show_version (struct pexornet_sfp* sfp, char* buf)
{
  /* stolen from pexornet_gosip.h*/
  u32 tmp, year, month, day, version[2];
  char txt[512];
  tmp = ioread32 (sfp->version);
  mb();
  ndelay(20);
  year = ((tmp & 0xff000000) >> 24) + 0x2000;
  month = (tmp & 0xff0000) >> 16;
  day = (tmp & 0xff00) >> 8;
  version[0] = (tmp & 0xf0) >> 4;
  version[1] = (tmp & 0xf);
  snprintf (txt, 512, "GOSIP FPGA code compiled at Year=%x Month=%x Date=%x Version=%x.%x \n", year, month, day,
      version[0], version[1]);
  pexornet_dbg(KERN_NOTICE "%s", txt);
  if (buf)
    snprintf (buf, 512, "%s", txt);
}

#ifdef PEXORNET_SYSFS_ENABLE
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,18)
ssize_t pexornet_sysfs_sfpregs_show(struct device *dev, struct device_attribute *attr, char *buf)
{
  ssize_t curs=0;
#ifdef PEXORNET_WITH_SFP
  int i=0;
  struct dev_pexornet* pg;
  struct pexornet_sfp* sfp;
  struct pexornet_privdata *privdata;
  privdata= (struct pexornet_privdata*) dev_get_drvdata(dev);
  pg=&(privdata->registers);
  sfp=&(pg->sfp);
  curs+=snprintf(buf+curs, PAGE_SIZE-curs, "*** PEXORNET sfp register dump:\n");
  curs+=snprintf(buf+curs, PAGE_SIZE-curs, "\t request command:           0x%x\n",readl(sfp->req_comm));
  pexornet_bus_delay();
  curs+=snprintf(buf+curs, PAGE_SIZE-curs, "\t request address:           0x%x\n",readl(sfp->req_addr));
  pexornet_bus_delay();
  curs+=snprintf(buf+curs, PAGE_SIZE-curs, "\t reply status /clear:       0x%x\n",readl(sfp->rep_stat_clr));
  pexornet_bus_delay();
  curs+=snprintf(buf+curs, PAGE_SIZE-curs, "\t rx monitor:                0x%x\n",readl(sfp->rx_moni));
  pexornet_bus_delay();
  curs+=snprintf(buf+curs, PAGE_SIZE-curs, "\t tx status:                 0x%x\n",readl(sfp->tx_stat));
  pexornet_bus_delay();
  curs+=snprintf(buf+curs, PAGE_SIZE-curs, "\t reset:                     0x%x\n",readl(sfp->reset));
  pexornet_bus_delay();
  curs+=snprintf(buf+curs, PAGE_SIZE-curs, "\t disable:                   0x%x\n",readl(sfp->disable));
  pexornet_bus_delay();
  curs+=snprintf(buf+curs, PAGE_SIZE-curs, "\t fault:                     0x%x\n",readl(sfp->fault));
  pexornet_bus_delay();
  for(i=0; i<PEXORNET_SFP_NUMBER;++i)
  {
    curs+=snprintf(buf+curs, PAGE_SIZE-curs, "\t  ** sfp %d:\n",i);
    curs+=snprintf(buf+curs, PAGE_SIZE-curs, "\t\t  reply status:  0x%x\n",readl(sfp->rep_stat[i]));
    pexornet_bus_delay();
    curs+=snprintf(buf+curs, PAGE_SIZE-curs, "\t\t  reply address: 0x%x\n",readl(sfp->rep_addr[i]));
    pexornet_bus_delay();
    curs+=snprintf(buf+curs, PAGE_SIZE-curs, "\t\t  reply data:    0x%x\n",readl(sfp->rep_data[i]));
    pexornet_bus_delay();
    curs+=snprintf(buf+curs, PAGE_SIZE-curs, "\t\t  token memsize: 0x%x\n",readl(sfp->tk_memsize[i]));
    pexornet_bus_delay();
  }

#else
  curs+=snprintf(buf+curs, PAGE_SIZE-curs, "*** PEXORNET: no sfp register support!\n");
#endif
  return curs;
}

#endif
#endif


