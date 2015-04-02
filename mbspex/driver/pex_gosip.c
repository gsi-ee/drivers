#include "pex_gosip.h"
#include "pex_base.h"

int pex_ioctl_init_bus (struct pex_privdata* priv, unsigned long arg)
{
  int retval = 0;
  u32 sfp = 0, slave = 0;/*,comm=0;*/

  struct pex_bus_io descriptor;
  struct pex_sfp* sfpregisters = &(priv->regs.sfp);
  retval = pex_copy_from_user (&descriptor, (void __user *) arg, sizeof(struct pex_bus_io));
  if (retval)
    return retval;

  sfp = (u32) descriptor.sfp;    // sfp connection to initialize chain
  slave = (u32) descriptor.slave;    // maximum # of connected slave boards
  // for pex standard sfp code, we use this ioctl to initalize chain of slaves:
  retval = pex_sfp_clear_channel (priv, sfp);
  if (retval)
    return retval;
  retval = pex_sfp_init_request (priv, sfp, slave);
  if (retval)
    return retval;
  sfpregisters->num_slaves[sfp] = slave; /* keep track of existing slaves for configuration broadcast*/
  return retval;

}

int pex_ioctl_get_sfp_links (struct pex_privdata* priv, unsigned long arg)
{
  int retval = 0;
  u32 sfp = 0;
  struct pex_sfp_links descriptor;
  struct pex_sfp* sfpregisters = &(priv->regs.sfp);
  for (sfp = 0; sfp < PEX_SFP_NUMBER; ++sfp)
  {
    descriptor.numslaves[sfp] = sfpregisters->num_slaves[sfp];
  }
  retval = pex_copy_to_user ((void __user *) arg, &descriptor, sizeof(struct pex_sfp_links));
  return retval;
}



int pex_ioctl_write_bus (struct pex_privdata* priv, unsigned long arg)
{
  int retval = 0;
  struct pex_bus_io descriptor;
  retval = pex_copy_from_user (&descriptor, (void __user *) arg, sizeof(struct pex_bus_io));
  if (retval)
    return retval;
  retval = pex_sfp_broadcast_write_bus (priv, &descriptor); /* everything is subfunctions now*/
  if (retval)
    return retval;
  retval = pex_copy_to_user ((void __user *) arg, &descriptor, sizeof(struct pex_bus_io));
  return retval;
}

int pex_ioctl_read_bus (struct pex_privdata* priv, unsigned long arg)
{
  int retval = 0;
  struct pex_bus_io descriptor;
  retval = pex_copy_from_user (&descriptor, (void __user *) arg, sizeof(struct pex_bus_io));
  if (retval)
    return retval;
  retval = pex_sfp_read_bus (priv, &descriptor); /* everything is subfunctions now*/
  if (retval)
    return retval;
  retval = pex_copy_to_user ((void __user *) arg, &descriptor, sizeof(struct pex_bus_io));
  return retval;
}

int pex_ioctl_configure_bus (struct pex_privdata* priv, unsigned long arg)
{
  int retval = 0, i = 0;
  struct pex_bus_config descriptor;
  retval = pex_copy_from_user (&descriptor, (void __user *) arg, sizeof(struct pex_bus_config));
  if (retval)
    return retval;
  if (descriptor.numpars > PEX_MAXCONFIG_VALS)
  {
    pex_msg(
        KERN_ERR "** pex_ioctl_configure_bus: warning too many parameters %d , reduced to %d\n", descriptor.numpars, PEX_MAXCONFIG_VALS);
    descriptor.numpars = PEX_MAXCONFIG_VALS;
  }
  pex_dbg(KERN_NOTICE "** pex_ioctl_configure_bus with %d parameters\n", descriptor.numpars);
  for (i = 0; i < descriptor.numpars; ++i)
  {
    struct pex_bus_io data = descriptor.param[i];
    retval = pex_sfp_broadcast_write_bus (priv, &data);
    if (retval)
    {
      pex_msg(
          KERN_ERR "** pex_ioctl_configure_bus: error %d at pex_sfp_broadcast_write_bus for value i=%d\n", retval, i);
      return retval;
    }
  }
  mb();
  udelay(1000); /* set waitstate after configure*/
  return retval;
}


int pex_sfp_broadcast_write_bus (struct pex_privdata* priv, struct pex_bus_io* data)
{
  int retval = 0, i = 0, sl = 0, sfp = 0;
  char sfpbroadcast = 0, slavebroadcast = 0;
  unsigned long address=0, value=0;
  struct pex_sfp* sfpregisters = &(priv->regs.sfp);
  address=data->address;
  value=data->value; /* save this because pex_bus_io will be changed by write bus!*/
  mb();
  if (data->sfp < 0)
    sfpbroadcast = 1;
  if (data->slave < 0)
    slavebroadcast = 1;
  pex_dbg(KERN_NOTICE "** pex_sfp_broadcast_write_bus with sfpbroadcast %d slavebroadcast %d \n", sfpbroadcast,slavebroadcast);
  if (sfpbroadcast)
  {
    pex_dbg(KERN_NOTICE "** pex_sfp_broadcast_write_bus with sfpbroadcast\n");

    for (sfp = 0; sfp < PEX_SFP_NUMBER; ++sfp)
    {
      data->sfp = sfp;
      if(sfpregisters->num_slaves[sfp]==0) continue;
      if (slavebroadcast)
      {
        for (sl = 0; sl < sfpregisters->num_slaves[sfp]; ++sl)
        {
          data->slave = sl;
          data->address=address;
          data->value=value;
          retval = pex_sfp_write_bus (priv, data);
          if (retval)
          {
            pex_msg(
                KERN_ERR "** pex_sfp_broadcast_write_bus: error %d at pex_sfp_write_bus for value i=%d, sfp:%d slave:%ld \n", retval, i, data->sfp, data->slave);
            continue;
          }
        }
      }    // slavebroadcast
      else
      {
        data->address=address;
        data->value=value;
        retval = pex_sfp_write_bus (priv, data);
        if (retval)
        {
          pex_msg(
              KERN_ERR "** pex_sfp_broadcast_write_bus: error %d at pex_sfp_write_bus for value i=%d, sfp:%d slave:%ld \n", retval, i, data->sfp, data->slave);
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
      data->address=address;
      data->value=value;
      retval = pex_sfp_write_bus (priv, data);
      if (retval)
      {
        pex_msg(
            KERN_ERR "** pex_sfp_broadcast_write_bus: error %d at pex_sfp_write_bus for value i=%d, sfp:%d slave:%ld \n", retval, i, data->sfp, data->slave);
        continue;
      }
    }

  }
  else
  {
    /* single write, no broadcast loop*/
    retval = pex_sfp_write_bus (priv, data);
    if (retval)
    {
      pex_msg(KERN_ERR "** pex_sfp_broadcast_write_bus: error %d at pex_sfp_write_bus for value i=%d\n", retval, i);
      return retval;
    }
  }

  return retval;
}




int pex_sfp_write_bus (struct pex_privdata* priv, struct pex_bus_io* descriptor)
{

  int retval = 0;
  u32 ad = 0, val = 0, sfp = 0, slave = 0, comm = 0;
  u32 rstat = 0, radd = 0, rdat = 0;
  u32 totaladdress = 0;
  ad = (u32) descriptor->address;
  val = (u32) descriptor->value;
  sfp = (u32) descriptor->sfp;
  slave = (u32) descriptor->slave;
  pex_dbg(KERN_NOTICE "** pex_sfp_write_bus writes value %x to address %x on sfp %x, slave %x\n", val, ad, sfp, slave);

  comm = PEX_SFP_PT_AD_W_REQ | (0x1 << (16 + sfp));
  totaladdress = ad + (slave << 24);
  pex_sfp_clear_all (priv);
  //pex_sfp_clear_channel(priv,sfp);
  pex_sfp_request (priv, comm, totaladdress, val);
  //if((retval=pex_sfp_get_reply(priv, sfp, &rstat, &radd, &rdat, 0))!=0) // debug: no response check
  if ((retval = pex_sfp_get_reply (priv, sfp, &rstat, &radd, &rdat, PEX_SFP_PT_AD_W_REP)) != 0)
  {
    pex_msg(KERN_ERR "** pex_sfp_write_bus: error %d at sfp_reply \n", retval);
    pex_msg(KERN_ERR "   pex_sfp_write_bus: incorrect reply: 0x%x 0x%x 0x%x \n", rstat, radd, rdat);
    return -EIO;
  }
  descriptor->value = rstat;
  descriptor->address = radd;
  return 0;
}

int pex_sfp_read_bus (struct pex_privdata* priv, struct pex_bus_io* descriptor)
{
  int retval = 0;
  u32 ad = 0, chan = 0, slave = 0, comm = 0;
  u32 rstat = 0, radd = 0, rdat = 0;
  u32 totaladdress = 0;
  ad = (u32) descriptor->address;
  chan = (u32) descriptor->sfp;
  slave = (u32) descriptor->slave;
  pex_dbg(KERN_NOTICE "** pex_sfp_read_bus from_address %x on sfp %x, slave %x\n", ad, chan, slave);
  comm = PEX_SFP_PT_AD_R_REQ | (0x1 << (16 + chan));
  totaladdress = ad + (slave << 24);
  pex_sfp_clear_channel (priv, chan);
  pex_sfp_request (priv, comm, totaladdress, 0);
  //if((retval=pex_sfp_get_reply(priv, chan, &rstat, &radd, &rdat, 0))!=0) // debug:  no check
  if ((retval = pex_sfp_get_reply (priv, chan, &rstat, &radd, &rdat, PEX_SFP_PT_AD_R_REP)) != 0)
  {
    pex_msg(KERN_ERR "** pex_sfp_read_bus: error %d at sfp_reply \n", retval);
    pex_msg(KERN_ERR "    incorrect reply: 0x%x 0x%x 0x%x \n", rstat, radd, rdat)
    return -EIO;
  }

  descriptor->value = rdat;
  return 0;
}

int pex_ioctl_request_token (struct pex_privdata* priv, unsigned long arg)
{
  int retval = 0;
  u32 comm = 0, chan = 0, chanpattern, bufid = 0;
  struct pex_token_io descriptor;

  dma_addr_t dmatarget = 0;
  u32 dmalen = 0, dmaburst = 0;
  u32 channelmask = 0;

  retval = pex_copy_from_user (&descriptor, (void __user *) arg, sizeof(struct pex_token_io));
  if (retval)
    return retval;
  chan = ((u32) descriptor.sfp) & 0xFFFF;
  chanpattern = (((u32) descriptor.sfp) & 0xFFFF0000) >> 16; /* optionally use sfp pattern in upper bytes*/
  bufid = (u32) descriptor.bufid;

  /* send token request*/
   pex_tdbg(KERN_NOTICE "** pex_ioctl_request_token from_sfp 0x%x, bufid 0x%x\n",chan,bufid);
  pex_sfp_assert_channel(chan);

  if (descriptor.directdma)
  {
    // setup here dma targets in direct dma mode before initiating gosip transfer
    dmatarget = (dma_addr_t) descriptor.dmatarget;
    dmalen = (u32) descriptor.dmasize;
    dmaburst = (u32) descriptor.dmaburst;
    channelmask = 1 << (chan + 1);    // select SFP for PCI Express DMA
    pex_tdbg(
        KERN_NOTICE "** pex_ioctl_request_token uses dma target 0x%x, channelmask=0x%x\n", (unsigned) dmatarget, channelmask);
    retval = pex_start_dma (priv, 0, dmatarget, 0, channelmask, dmaburst);
    if (retval)
    {
      /* error handling, e.g. no more dma buffer available*/
      pex_dbg(KERN_ERR "pex_ioctl_read_token error %d from startdma\n", retval);
      return retval;
    }

  }    // if directdma

  if (chanpattern != 0)
  {
    pex_dbg(KERN_NOTICE "** pex_ioctl_request_token with channelpattern 0x%x\n", (unsigned) chanpattern);
    comm = PEX_SFP_PT_TK_R_REQ | (chanpattern << 16); /* token broadcast mode*/
    pex_sfp_clear_channelpattern (priv, chanpattern);
  }
  else
  {
    pex_dbg(KERN_NOTICE "** pex_ioctl_request_token for channel 0x%x\n", (unsigned) chan);
    comm = PEX_SFP_PT_TK_R_REQ | (0x1 << (16 + chan)); /* single sfp token mode*/
    pex_sfp_clear_channel (priv, chan);
  }

  pex_sfp_request (priv, comm, bufid, 0); /* note: slave is not specified; the chain of all slaves will send everything to receive buffer*/
  if (descriptor.sync != 0)
  {
    /* only wait here for dma buffer if synchronous*/
    return (pex_ioctl_wait_token (priv, arg));
  }
   pex_tdbg(KERN_NOTICE "** pex_ioctl_request_token returns with no wait\n");
  return retval;
}

int pex_ioctl_wait_token (struct pex_privdata* priv, unsigned long arg)
{
  int retval = 0;
  u32 chan = 0;
  u32 rstat = 0, radd = 0, rdat = 0;
  u32 dmasize = 0, oldsize = 0, dmaburst = 0;
  dma_addr_t dmatarget = 0;

  struct pex_sfp* sfp = &(priv->regs.sfp);
  struct pex_token_io descriptor;
  retval = pex_copy_from_user (&descriptor, (void __user *) arg, sizeof(struct pex_token_io));
  if (retval)
    return retval;

  chan = ((u32) descriptor.sfp) & 0xFFFF;

  /* send token request
   pex_msg(KERN_NOTICE "** pex_ioctl_request_token from_sfp 0x%x, bufid 0x%x\n",chan,bufid);*/
  pex_sfp_assert_channel(chan);
 pex_tdbg(KERN_ERR "pex_ioctl_wait_token waits for reply...\n");
  if ((retval = pex_sfp_get_reply (priv, chan, &rstat, &radd, &rdat, 0)) != 0)    // debug: do not check reply status
  //if((retval=pex_sfp_get_reply(priv, chan, &rstat, &radd, &rdat, PEX_SFP_PT_TK_R_REP))!=0)
  {
    pex_msg(KERN_ERR "** pex_ioctl_wait_token: error %d at sfp_reply \n", retval);
    pex_msg(KERN_ERR "    incorrect reply: 0x%x 0x%x 0x%x \n", rstat, radd, rdat)
    return -EIO;
  }
  descriptor.check_comm = rstat;
  descriptor.check_token = radd;
  descriptor.check_numslaves = rdat;

  /* for not direct dma we have to perform DMA here:*/
  if (descriptor.directdma == 0)
  {
    /* find out real package length :*/
    dmasize = ioread32 (sfp->tk_memsize[chan]);
    dmaburst = (dma_addr_t) descriptor.dmaburst;
    if (dmaburst > PEX_BURST)
      dmaburst = PEX_BURST;
    mb();
    ndelay(20);
    if (dmasize > PEX_SFP_TK_MEM_RANGE)
    {
      oldsize = dmasize;
      dmasize = PEX_SFP_TK_MEM_RANGE - (PEX_SFP_TK_MEM_RANGE % dmaburst); /* align on last proper burst interval*/
      pex_dbg(KERN_NOTICE "** pex_ioctl_wait_token reduces dma size from 0x%x to 0x%x \n", oldsize, dmasize);
    }
    pex_dbg(KERN_NOTICE "** pex_ioctl_wait_token uses dma size 0x%x of channel %x\n", dmasize, chan);

    print_register ("DUMP token dma size", sfp->tk_memsize[chan]);

    /*	pex_msg(KERN_NOTICE "** pex_ioctl_read_token  uses token memory %x (dma:%x)\n",sfp->tk_mem[chan],sfp->tk_mem_dma[chan]);*/
    print_register ("DUMP token memory first content", sfp->tk_mem[chan]);
    print_register ("DUMP token memory second content", (sfp->tk_mem[chan] + 1));

    /* here issue dma to mbs pipe target address:*/
    dmatarget = (dma_addr_t) descriptor.dmatarget;

    pex_dbg(
        KERN_NOTICE "** pex_ioctl_wait_token uses dma target 0x%x, dmasize=0x%x burst=0x%x\n", (unsigned) dmatarget, dmasize, dmaburst);
    retval = pex_start_dma (priv, sfp->tk_mem_dma[chan], dmatarget, dmasize, 0, dmaburst);
    if (retval)
    {
      /* error handling, e.g. no more dma buffer available*/
      pex_dbg(KERN_ERR "pex_ioctl_wait_token error %d from startdma\n", retval);
      return retval;
    }

  } /* not PEX_DIRECT_DMA*/

  if ((retval = pex_poll_dma_complete (priv)) != 0)
  {
    pex_msg(KERN_ERR "pex_ioctl_wait_token error %d from pex_poll_dma_complete\n", retval);
    return retval;
  }

  /* find out real package length after dma:*/
  dmasize = ioread32 (priv->regs.dma_len);
  pex_bus_delay();
  descriptor.dmasize = dmasize; /* account used payload size.*/

  retval = pex_copy_to_user ((void __user *) arg, &descriptor, sizeof(struct pex_token_io));
  pex_tdbg(KERN_NOTICE "pex_ioctl_wait_token returns\n");
  return retval;

}

void pex_sfp_reset (struct pex_privdata* privdata)
{
  int i;
  struct pex_sfp* sfp = &(privdata->regs.sfp);
  pex_dbg(KERN_NOTICE "**pex_sfp_reset\n");
  iowrite32 (0xF, sfp->reset);
  mb();
  pex_sfp_delay()
  ;
  iowrite32 (0, sfp->reset);
  mb();
  pex_sfp_delay()
  ;
  /* reset number of slaves counter for broadcast*/
  for (i = 0; i < PEX_SFP_NUMBER; ++i)
  {
    sfp->num_slaves[i] = 0;
  }

}

void pex_sfp_request (struct pex_privdata* privdata, u32 comm, u32 addr, u32 data)
{
  struct pex_sfp* sfp = &(privdata->regs.sfp);
  pex_dbg(KERN_NOTICE "**pex_sfp_request, comm=%x, addr=%x data=%x\n", comm, addr, data);
  iowrite32 (addr, sfp->req_addr);
  pex_sfp_delay()
  ;
  iowrite32 (data, sfp->req_data);
  pex_sfp_delay()
  ;
  iowrite32 (comm, sfp->req_comm);
  pex_sfp_delay()
  ;
}

int pex_sfp_get_reply (struct pex_privdata* privdata, int ch, u32* comm, u32 *addr, u32 *data, u32 checkvalue)
{
  u32 status = 0, loopcount = 0;
  struct pex_sfp* sfp = &(privdata->regs.sfp);
  pex_dbg(KERN_NOTICE "**pex_sfp_get_reply ***\n");
  pex_sfp_assert_channel(ch);

  do
  {
    if (loopcount > privdata->sfp_maxpolls)/* 1000000*/
    {
      pex_msg(KERN_WARNING "**pex_sfp_get_reply polled %d times = %d ns without success, abort\n", loopcount, (loopcount* PEX_SFP_DELAY));
      print_register (" ... status after FAILED pex_sfp_get_reply:", sfp->rep_stat[ch]);
      return -EIO;
    }
    status = ioread32 (sfp->rep_stat[ch]);
    pex_sfp_delay()
    ;
    //pex_sfp_delay(); /* additional waitstate here?*/
    if(PEX_DMA_POLL_SCHEDULE) schedule(); /* probably this also may help*/
    loopcount++;
  } while (((status & 0x3000) >> 12) != 0x02); /* packet received bit is set*/

  *comm = ioread32 (sfp->rep_stat[ch]);
  pex_sfp_delay()
  ;
  *addr = ioread32 (sfp->rep_addr[ch]);
  pex_sfp_delay()
  ;
  *data = ioread32 (sfp->rep_data[ch]);
  pex_sfp_delay()
  ;pex_dbg(KERN_NOTICE "pex_sfp_get_reply from SFP: %x got status:%x address:%x data: %x \n", ch, *comm, *addr, *data);
  if (checkvalue == 0)
    return 0;    // no check of reply structure
  if ((*comm & 0xfff) == checkvalue)
  {
    if ((*comm & 0x4000) != 0)
    {
      pex_msg(KERN_ERR "pex_sfp_get_reply: ERROR: Packet Structure : Command Reply 0x%x \n", *comm);
      return -EIO;
    }
  }
  else
  {
    pex_msg(
        KERN_ERR "pex_sfp_get_reply: ERROR : Command Reply  0x%x is not matching expected value 0x%x\n", (*comm & 0xfff), checkvalue);
    return -EIO;
  }
  return 0;

}

int pex_sfp_get_token_reply (struct pex_privdata* privdata, int ch, u32* stat, u32* head, u32* foot)
{
  u32 status = 0, loopcount = 0;
  struct pex_sfp* sfp = &(privdata->regs.sfp);
  pex_dbg(KERN_NOTICE "**pex_sfp_get_token_reply ***\n");
  pex_sfp_assert_channel(ch);

  do
  {
    if (loopcount > privdata->sfp_maxpolls)
    {
      pex_msg(KERN_WARNING "**pex_sfp_get_token reply polled %d times = %d ns without success, abort\n", loopcount, (loopcount* PEX_SFP_DELAY));
      print_register (" ... status after FAILED pex_sfp_get_token_reply:0x%x", sfp->tk_stat[ch]);
      return -EIO;
    }
    status = ioread32 (sfp->tk_stat[ch]);
    pex_sfp_delay()
    ;

    loopcount++;
  } while (((status & 0x3000) >> 12) != 0x02); /* packet received bit is set*/

  *stat = ioread32 (sfp->tk_stat[ch]);
  pex_sfp_delay()
  ;
  *head = ioread32 (sfp->tk_head[ch]);
  pex_sfp_delay()
  ;
  *foot = ioread32 (sfp->tk_foot[ch]);
  pex_sfp_delay()
  ;
  pex_dbg(
      KERN_NOTICE "pex_sfp_get_token_reply from SFP: %x got token status:%x header:%x footer: %x \n", ch, *stat, *head, *foot);

  return 0;
}

int pex_sfp_init_request (struct pex_privdata* privdata, int ch, int numslaves)
{
  int retval = 0;
  u32 sfp = 0, comm = 0, maxslave = 0;
  u32 rstat = 0, radd = 0, rdat = 0;
  sfp = (u32) ch;
  maxslave = (u32) numslaves - 1; /* changed api: pass index of max slave, not number of slaves*/
  if (numslaves <= 0)
    maxslave = 0; /* catch possible user workaround for changed api*/
  pex_sfp_assert_channel(ch);
  comm = PEX_SFP_INI_REQ | (0x1 << (16 + sfp));
  pex_dbg(KERN_NOTICE "**pex_sfp_init_request for sfp:%d with maxslave=%d ***\n",sfp, maxslave);
  pex_sfp_request (privdata, comm, 0, maxslave);
  if ((retval = pex_sfp_get_reply (privdata, sfp, &rstat, &radd, &rdat, 0)) != 0)
  //if((retval=pex_sfp_get_reply(privdata, sfp, &rstat, &radd, &rdat, PEX_SFP_PT_INI_REP))!=0)
  {
    pex_msg(KERN_ERR "** pex_sfp_init_request: error %d at sfp_reply \n", retval);
    pex_msg(KERN_ERR "   pex_sfp_init_request: incorrect reply: 0x%x 0x%x 0x%x \n", rstat, radd, rdat);
    return -EIO;
  }
  return retval;
}

int pex_sfp_clear_all (struct pex_privdata* privdata)
{
  u32 status = 0, loopcount = 0, clrval;
  struct pex_sfp* sfp = &(privdata->regs.sfp);
  clrval = 0xf;
  pex_dbg(KERN_NOTICE "**pex_sfp_clear_all ***\n");
  /*iowrite32(clrval, sfp->rep_stat_clr);
   pex_sfp_delay();*/
  do
  {
    if (loopcount > privdata->sfp_maxpolls)
    {
      pex_msg(KERN_WARNING "**pex_sfp_clear_all tried  %d times = %d ns  without success, abort\n", loopcount, (loopcount* 2 * PEX_SFP_DELAY));
      print_register (" ... stat_clr after FAILED pex_sfp_clear_all: 0x%x", sfp->rep_stat_clr);
      return -EIO;
    }
    iowrite32 (clrval, sfp->rep_stat_clr);
    pex_sfp_delay()
    ;
    status = ioread32 (sfp->rep_stat_clr);
    pex_sfp_delay()
    ;
    loopcount++;
  } while (status != 0x0);pex_dbg(KERN_INFO "**after pex_sfp_clear_all: loopcount:%d \n", loopcount);
  print_register (" ... stat_clr after pex_sfp_clear_all:", sfp->rep_stat_clr);
  return 0;
}

int pex_sfp_clear_channel (struct pex_privdata* privdata, int ch)
{
  u32 repstatus = 0, tokenstatus = 0, chstatus = 0, loopcount = 0, clrval;
  struct pex_sfp* sfp = &(privdata->regs.sfp);
  pex_dbg(KERN_NOTICE "**pex_sfp_clear_channel %d ***\n", ch);
  pex_sfp_assert_channel(ch);
  clrval = (0x1 << ch);
  /*iowrite32(clrval, sfp->rep_stat_clr);
   pex_sfp_delay();*/
  do
  {
    if (loopcount > privdata->sfp_maxpolls)
    {
      pex_msg(KERN_WARNING "**pex_sfp_clear_channel %d tried %d times = %d ns without success, abort\n", ch, loopcount, (loopcount* (2 * PEX_SFP_DELAY+ 2 * PEX_BUS_DELAY)));
      print_register (" ... reply status after FAILED pex_sfp_clear_channel:", sfp->rep_stat[ch]);
      print_register (" ... token reply status after FAILED pex_sfp_clear_channel:", sfp->tk_stat[ch]);
      return -EIO;
    }

    iowrite32 (clrval, sfp->rep_stat_clr);
    pex_sfp_delay()
    ;
    repstatus = ioread32 (sfp->rep_stat[ch]) & 0xf000;
    pex_bus_delay();
    tokenstatus = ioread32 (sfp->tk_stat[ch]) & 0xf000;
    pex_bus_delay();
    chstatus = ioread32 (sfp->rep_stat_clr) & clrval;
    pex_sfp_delay()
    ;
    loopcount++;
  } while ((repstatus != 0x0) || (tokenstatus != 0x0) || (chstatus != 0x0));

  pex_dbg(KERN_INFO "**after pex_sfp_clear_channel %d : loopcount:%d \n", ch, loopcount);
  /*print_register(" ... reply status:", sfp->rep_stat[ch]);
   print_register(" ... token reply status:", sfp->tk_stat[ch]);
   print_register(" ... statclr:", sfp->rep_stat_clr);*/

  return 0;
}

int pex_sfp_clear_channelpattern (struct pex_privdata* privdata, int pat)
{
  u32 repstatus = 0, loopcount = 0, clrval, mask;
  struct pex_sfp* sfp = &(privdata->regs.sfp);
  pex_dbg(KERN_NOTICE "**pex_sfp_clear_channel pattern 0x%x ***\n", pat);
  clrval = pat;
  mask = (pat << 8) | (pat << 4) | pat;
  do
  {
    if (loopcount > privdata->sfp_maxpolls)
    {
      pex_msg(
          KERN_WARNING "**pex_sfp_clear_channelpattern 0x%x tried %d  times = %d ns without success, abort\n", pat, loopcount, (loopcount* 2 * PEX_SFP_DELAY));
      print_register (" ... reply status after FAILED pex_sfp_clear_channelpattern:", sfp->rep_stat_clr);
      return -EIO;
    }
    iowrite32 (clrval, sfp->rep_stat_clr);
    pex_sfp_delay()
    ;
    repstatus = ioread32 (sfp->rep_stat_clr) & mask;
    pex_sfp_delay()
    ;
    loopcount++;
  } while ((repstatus != 0x0));

  pex_dbg(KERN_INFO "**after pex_sfp_clear_channelpattern 0x%x : loopcount:%d \n", pat, loopcount);
  /*print_register(" ... reply status:", sfp->rep_stat_clr); */
  return 0;
}



void set_sfp (struct pex_sfp* sfp, void* membase, unsigned long bar)
{
  int i = 0;
  void* sfpbase = 0;
  unsigned long offset;
  if (sfp == 0)
    return;
  sfpbase = membase + PEX_SFP_BASE;
  sfp->version = (u32*) (sfpbase + PEX_SFP_VERSION);
  sfp->req_comm = (u32*) (sfpbase + PEX_SFP_REQ_COMM);
  sfp->req_addr = (u32*) (sfpbase + PEX_SFP_REQ_ADDR);
  sfp->req_data = (u32*) (sfpbase + PEX_SFP_REQ_DATA);
  sfp->rep_stat_clr = (u32*) (sfpbase + PEX_SFP_REP_STAT_CLR);
  sfp->rx_moni = (u32*) (sfpbase + PEX_SFP_RX_MONI);
  sfp->tx_stat = (u32*) (sfpbase + PEX_SFP_TX_STAT);
  sfp->reset = (u32*) (sfpbase + PEX_SFP_RX_RST);
  sfp->disable = (u32*) (sfpbase + PEX_SFP_DISA);
  sfp->fault = (u32*) (sfpbase + PEX_SFP_FAULT);
  for (i = 0; i < PEX_SFP_NUMBER; ++i)
  {
    offset = i * 0x04;
    sfp->rep_stat[i] = (u32*) (sfpbase + PEX_SFP_REP_STAT_0 + offset);
    sfp->rep_addr[i] = (u32*) (sfpbase + PEX_SFP_REP_ADDR_0 + offset);
    sfp->rep_data[i] = (u32*) (sfpbase + PEX_SFP_REP_DATA_0 + offset);
    sfp->fifo[i] = (u32*) (sfpbase + PEX_SFP_FIFO_0 + offset);
    sfp->tk_stat[i] = (u32*) (sfpbase + PEX_SFP_TOKEN_REP_STAT_0 + offset);
    sfp->tk_head[i] = (u32*) (sfpbase + PEX_SFP_TOKEN_REP_HEAD_0 + offset);
    sfp->tk_foot[i] = (u32*) (sfpbase + PEX_SFP_TOKEN_REP_FOOT_0 + offset);
    sfp->tk_dsize[i] = (u32*) (sfpbase + PEX_SFP_TOKEN_DSIZE_0 + offset);
    sfp->tk_dsize_sel[i] = (u32*) (sfpbase + PEX_SFP_TOKEN_DSIZE_SEL_0 + offset);
    sfp->tk_memsize[i] = (u32*) (sfpbase + PEX_SFP_TOKEN_MEM_SIZE_0 + offset);
    offset = i * 0x40000;
    sfp->tk_mem[i] = (u32*) (membase + PEX_SFP_TK_MEM_0 + offset);
    sfp->tk_mem_dma[i] = (dma_addr_t) (bar + PEX_SFP_TK_MEM_0 + offset);
  }

}

void print_sfp (struct pex_sfp* sfp)
{
  int i = 0;
  if (sfp == 0)
    return;pex_dbg(KERN_NOTICE "##print_sfp: ###################\n");
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
  for (i = 0; i < PEX_SFP_NUMBER; ++i)
  {
    pex_dbg(KERN_NOTICE "-------- sfp number %d -------\n", i);
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
    pex_dbg(KERN_NOTICE "token mem start DMA =0x%x \n", (unsigned) sfp->tk_mem_dma[i]);
  }

  pex_show_version (sfp, 0);
}

void pex_show_version (struct pex_sfp* sfp, char* buf)
{
  /* stolen from pex_gosip.h*/
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
  pex_dbg(KERN_NOTICE "%s", txt);
  if (buf)
    snprintf (buf, 1024, "%s", txt);
}

#ifdef PEX_SYSFS_ENABLE
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,18)
ssize_t pex_sysfs_sfpregs_show(struct device *dev, struct device_attribute *attr, char *buf)
{
  ssize_t curs=0;
  int i=0;
  struct regs_pex* pg;
  struct pex_sfp* sfp;
  struct pex_privdata *privdata;
  privdata= (struct pex_privdata*) dev_get_drvdata(dev);
  pg=&(privdata->regs);
  sfp=&(pg->sfp);
  curs+=snprintf(buf+curs, PAGE_SIZE-curs, "*** PEX sfp register dump:\n");
  curs+=snprintf(buf+curs, PAGE_SIZE-curs, "\t request command:           0x%x\n",readl(sfp->req_comm));
  pex_bus_delay();
  curs+=snprintf(buf+curs, PAGE_SIZE-curs, "\t request address:           0x%x\n",readl(sfp->req_addr));
  pex_bus_delay();
  curs+=snprintf(buf+curs, PAGE_SIZE-curs, "\t reply status /clear:       0x%x\n",readl(sfp->rep_stat_clr));
  pex_bus_delay();
  curs+=snprintf(buf+curs, PAGE_SIZE-curs, "\t rx monitor:                0x%x\n",readl(sfp->rx_moni));
  pex_bus_delay();
  curs+=snprintf(buf+curs, PAGE_SIZE-curs, "\t tx status:                 0x%x\n",readl(sfp->tx_stat));
  pex_bus_delay();
  curs+=snprintf(buf+curs, PAGE_SIZE-curs, "\t reset:                     0x%x\n",readl(sfp->reset));
  pex_bus_delay();
  curs+=snprintf(buf+curs, PAGE_SIZE-curs, "\t disable:                   0x%x\n",readl(sfp->disable));
  pex_bus_delay();
  curs+=snprintf(buf+curs, PAGE_SIZE-curs, "\t fault:                     0x%x\n",readl(sfp->fault));
  for(i=0; i<PEX_SFP_NUMBER;++i)
  {
    curs+=snprintf(buf+curs, PAGE_SIZE-curs, "\t  ** sfp %d:\n",i);
    curs+=snprintf(buf+curs, PAGE_SIZE-curs, "\t\t  reply status:  0x%x\n",readl(sfp->rep_stat[i]));
    pex_bus_delay();
    curs+=snprintf(buf+curs, PAGE_SIZE-curs, "\t\t  reply address: 0x%x\n",readl(sfp->rep_addr[i]));
    pex_bus_delay();
    curs+=snprintf(buf+curs, PAGE_SIZE-curs, "\t\t  reply data:    0x%x\n",readl(sfp->rep_data[i]));
    pex_bus_delay();
    curs+=snprintf(buf+curs, PAGE_SIZE-curs, "\t\t  token memsize: 0x%x\n",readl(sfp->tk_memsize[i]));
    pex_bus_delay();
  }

  return curs;
}

#endif
#endif

