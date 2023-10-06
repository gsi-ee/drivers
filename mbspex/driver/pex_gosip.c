#include "pex_gosip.h"
#include "pex_base.h"



#ifdef PEX_SFP_USE_KINPEX_V5
int gosip_version = 5;
#else
int gosip_version = 0;
#endif


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
  if(priv->sfp_buswait) udelay(priv->sfp_buswait); // delay after each user bus ioctl to adjust frontend speed
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
  if(priv->sfp_buswait) udelay(priv->sfp_buswait); // delay after each user bus ioctl to adjust frontend speed
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
    if(priv->sfp_buswait) udelay(priv->sfp_buswait); // delay after each user bus ioctl to adjust frontend speed
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
  //if ((retval = pex_sfp_get_reply (priv, chan, &rstat, &radd, &rdat, 0)) != 0)    // debug: do not check reply status
  if((retval=pex_sfp_get_reply(priv, chan, &rstat, &radd, &rdat, PEX_SFP_PT_TK_R_REP))!=0) // JAM2016 enabled
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


int pex_ioctl_request_receive_token_parallel (struct pex_privdata *priv, unsigned long arg)
{
  int retval = 0, i;
  u32 comm = 0, chan = 0, chanpattern, chmask, bufid = 0, sfp = 0;
  u32 rstat = 0, radd = 0, rdat = 0;
  struct pex_token_io descriptor;
  struct pex_sfp* sfpregisters;
  dma_addr_t dmatarget = 0, dmasource = 0, dmatmp=0;
  phys_addr_t pipephys=0;
  phys_addr_t poff[PEX_SFP_NUMBER] = { 0 };    // pipe pointer offset
  u32 paddington[PEX_SFP_NUMBER] = { 0 };
  u32 dmalen = 0, dmaburst = 0, tokenmemsize = 0, dmalencheck = 0, datalensum = 0, paddingdelta=0;
  u32* pipepartbase = 0;
  u32* pdat = 0;

  retval = pex_copy_from_user (&descriptor, (void __user *) arg, sizeof(struct pex_token_io));
  if (retval)
    return retval;
  chan = ((u32) descriptor.sfp) & 0xFFFF;
  chanpattern = (((u32) descriptor.sfp) & 0xFFFF0000) >> 16; /* optionally use sfp pattern in upper bytes*/
  bufid = (u32) descriptor.bufid;
  dmatarget = (dma_addr_t) descriptor.dmatarget;

  if (chanpattern != 0)
  {
    // TODO: might check here if channelpattern is consistent with initialized chains!
    pex_dbg(
        KERN_NOTICE "** pex_ioctl_request_receive_token_parallel with channelpattern 0x%x\n", (unsigned) chanpattern);
    comm = PEX_SFP_PT_TK_R_REQ | (chanpattern << 16); /* token broadcast mode*/
    pex_sfp_clear_channelpattern (priv, chanpattern);
  }
  else
  {
    pex_dbg(KERN_NOTICE "** pex_ioctl_request_receive_token_parallel for channel 0x%x\n", (unsigned) chan);
    comm = PEX_SFP_PT_TK_R_REQ | (0x1 << (16 + chan)); /* single sfp token mode*/
    pex_sfp_clear_channel (priv, chan);
  }

  pex_sfp_request (priv, comm, bufid, 0); /* note: slave is not specified; the chain of all slaves will send everything to receive buffer*/

  ////////////////////////////////////////////////////////////
  // first loop over all registered sfps for token receive complete
  sfpregisters = &(priv->regs.sfp);
  for (sfp = 0; sfp < PEX_SFP_NUMBER; ++sfp)
  {
    if (sfpregisters->num_slaves[sfp] == 0)
      continue;    // exclude not configured sfps
    if ((chanpattern == 0) && (sfp != chan))
      continue;    //single channel mode
    if (chanpattern != 0)
    {
      chmask = (0x1 << sfp);
      if ((chmask & chanpattern) != chmask)
        continue;    // check for channel pattern
    }
    pex_tdbg(KERN_ERR "pex_ioctl_request_receive_token_parallel waits for reply of sfp %d...\n", sfp);
    //if ((retval = pex_sfp_get_reply (priv, sfp, &rstat, &radd, &rdat, 0)) != 0)    // debug: do not check reply status
    if((retval=pex_sfp_get_reply(priv, sfp, &rstat, &radd, &rdat, PEX_SFP_PT_TK_R_REP))!=0) // JAM2016
    {
      pex_msg(KERN_ERR "** pex_ioctl_request_receive_token_parallel: wait token error %d at sfp_%d reply \n", retval, sfp);
      pex_msg(KERN_ERR "    incorrect reply: 0x%x 0x%x 0x%x \n", rstat, radd, rdat)
      return -EIO;
    }
  }    // for sfp first loop

  ////////////////////////////////////////////////////////////
  // second loop over all registered sfps:
  for (sfp = 0; sfp < PEX_SFP_NUMBER; ++sfp)
  {
    if (sfpregisters->num_slaves[sfp] == 0)
      continue;
    if ((chanpattern == 0) && (sfp != chan))
      continue;    //single channel mode
    if (chanpattern != 0)
    {
      chmask = (0x1 << sfp);
      if ((chmask & chanpattern) != chmask)
        continue;    // check for channel pattern
    }
    poff[sfp] = dmatarget - (dma_addr_t) descriptor.dmatarget;    // at the start of each sfp, current pipe pointer offset refers to intended dma target

    // - get token memsize:
    tokenmemsize = ioread32 (sfpregisters->tk_memsize[sfp]);
    pex_sfp_delay()
    ;
    tokenmemsize += 4;    // wg. shizu !!??
    pex_dbg(
        KERN_NOTICE "** pex_ioctl_request_receive_token_parallel token data len (sfp_%d)=%d bytes\n", sfp, tokenmemsize);

    // choose burst size to accept max. 20% padding size
    if (tokenmemsize < 0xa0)
    {
      dmaburst = 0x10;
    }
    else if (tokenmemsize < 0x140)
    {
      dmaburst = 0x20;
    }
    else if (tokenmemsize < 0x280)
    {
      dmaburst = 0x40;
    }
    else
    {
      dmaburst = 0x80;
    }

    // - calculate DMA transfer size up to full burstlength multiples
    if ((tokenmemsize % dmaburst) != 0)
    {
      dmalen = tokenmemsize + dmaburst     // in bytes
      - (tokenmemsize % dmaburst);
    }
    else
    {
      dmalen = tokenmemsize;
    }
    // - calculate padding offset from current destination pointer:
    paddington[sfp] = 0;
    dmatmp=dmatarget; // do not touch dmatarget from modulo macro!
    paddingdelta = do_div(dmatmp, dmaburst); // this will also work on 32 bit platforms, note that dmatmp will be modified by do_div
    //paddingdelta= dmatarget % dmaburst; // works only on 64 bit arch, 32 bit gives linker error "__umoddi3 undefined!
    if (paddingdelta != 0)
    {
      paddington[sfp] = dmaburst - paddingdelta;
      dmatarget = dmatarget + paddington[sfp];
    }
    // - perform DMA
    dmasource = sfpregisters->tk_mem_dma[sfp];
    if ((retval = pex_start_dma (priv, dmasource, dmatarget, dmalen, 1, dmaburst)) != 0)
      return retval;

    if ((retval = pex_poll_dma_complete (priv)) != 0)
      return retval;
    /* find out real package length after dma:*/
    dmalencheck = ioread32 (priv->regs.dma_len);

/////////// JAM END section
    pex_bus_delay()
    ;
    if (dmalencheck != dmalen)
    {
      pex_msg(KERN_ERR "** pex_ioctl_request_receive_token_parallel: dmalen mismatch at sfp_%d, transferred %d bytes, requested %d bytes\n",
          sfp, dmalencheck, dmalen);
      return -EIO;
    }

    dmatarget =  descriptor.dmatarget + poff[sfp] + paddington[sfp]+ tokenmemsize; // increment pipe data pointer to end of real payload
    datalensum += tokenmemsize + paddington[sfp];    // for ioremap also account possible padding space here

  }    // for sfp second loop dma
  ////////////////////////////////////////////////////////////
  // third loop to put padding words into ioremapped pipe. this may reduce overhead of ioremap call:

  //pipepartbase = ioremap_nocache (descriptor.dmatarget, dmalensum);
  // < JAM this gives error on kernel 3.2.0-4:  ioremap error for 0xa641000-0xa643000, requested 0x10, got 0x0
  pipephys=descriptor.dmatarget;
  pipepartbase = ioremap_cache(pipephys, datalensum);
  // JAM need to sync page cache with phys pipe afterwards?
  if (pipepartbase == NULL )
  {
    pex_msg(KERN_ERR "** pex_ioctl_request_receive_token_parallel: Could not remap %d bytes of pipe memory at 0x%lx  ", datalensum, descriptor.dmatarget);
    return -EIO;
  }
  pex_dbg(KERN_NOTICE "** pex_ioctl_request_receive_token_parallel: remapped %d bytes of pipe memory at 0x%lx, kernel address:0x%x  ",
      datalensum, descriptor.dmatarget, pipepartbase);

  pdat = pipepartbase;
  for (sfp = 0; sfp < PEX_SFP_NUMBER; ++sfp)
  {
    if (sfpregisters->num_slaves[sfp] == 0)
      continue;
    if ((chanpattern == 0) && (sfp != chan))
      continue;    //single channel mode
    if (chanpattern != 0)
    {
      chmask = (0x1 << sfp);
      if ((chmask & chanpattern) != chmask)
        continue;    // check for channel pattern
    }
    pdat =  pipepartbase + (poff[sfp] >> 2);     // set padding start pointer to pipe offset for this sfp's part
    paddington[sfp] = paddington[sfp] >> 2;      // padding length also now in 4 bytes (longs)
    for (i = 0; i < paddington[sfp]; i++)
    {
      pex_dbg(KERN_NOTICE ">>> Fill padding pattern at 0x%x with 0x%x ,l_k=%d times\n", pdat, 0xadd00000 + (paddington[sfp] << 8) + i, i);
      *(pdat++) = 0xadd00000 + (paddington[sfp] << 8) + i;
    }
  }      // for sfp end padding loop

  iounmap (pipepartbase); // seems to be that any cache is sync'ed to phys memory after this...
  // now return new position of data pointer in pipe:
  descriptor.dmasize = datalensum; /* contains sum of token data length and sum of padding fields => new pdat offset */

  retval = pex_copy_to_user ((void __user *) arg, &descriptor, sizeof(struct pex_token_io));
  pex_tdbg(KERN_NOTICE "pex_ioctl_request_receive_token_parallel returns\n");
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
  if (gosip_version>0) //v5 and beyond
  {
    if( (comm & 0xfff) == PEX_SFP_PT_TK_R_REQ )
        {
          //*ps_pexor->req_comm = comm | (data&0xf) << 20 | (addr&0x3) << 24 | 1<<28 ;
          //                  printf ("PEXOR_TX: req_comm  %x \n", *ps_pexor->req_comm);

          iowrite32 ((comm | (data & 0xf) << 20 | (addr & 0x3) << 24 | 1<<28) , sfp->req_comm);
          pex_sfp_delay();
        }
    else
        {
          iowrite32 (addr, sfp->req_addr);
          pex_sfp_delay();
          iowrite32 (data, sfp->req_data);
          pex_sfp_delay();
          iowrite32 (comm, sfp->req_comm);
          pex_sfp_delay();
        }
  }
  else
  {


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
//#endif

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

  if (gosip_version>0) //v5 and beyond
  {
  *comm=status; // JAM: TODO- different to Shizu code which has one variable for status and comm
  if( (status & 0xfff) == PEX_SFP_PT_TK_R_REQ )
      {
        *addr =  (status & 0xf000000)>>24;
        *data =  (status & 0xf0000)>>16;
      }
  else
      {
        *addr = ioread32 (sfp->rep_addr[ch]);
        pex_sfp_delay();
        *data = ioread32 (sfp->rep_data[ch]);
        pex_sfp_delay();
      }
  }
  else
  {
  *comm = ioread32 (sfp->rep_stat[ch]);
  pex_sfp_delay()
  ;
  *addr = ioread32 (sfp->rep_addr[ch]);
  pex_sfp_delay()
  ;
  *data = ioread32 (sfp->rep_data[ch]);
  pex_sfp_delay()
  }
  ;pex_dbg(KERN_NOTICE "pex_sfp_get_reply from SFP: %x got status:%x address:%x data: %x \n", ch, *comm, *addr, *data);
  if (checkvalue == 0)
    return 0;    // no check of reply structure
  if ((*comm & checkvalue) == checkvalue)
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
  //if ((retval = pex_sfp_get_reply (privdata, sfp, &rstat, &radd, &rdat, 0)) != 0)
  // JAM2016: exact check of expected reply status:
  if((retval=pex_sfp_get_reply(privdata, sfp, &rstat, &radd, &rdat, PEX_SFP_PT_INI_REP))!=0)
  {
    pex_msg(KERN_ERR "** pex_sfp_init_request: error %d at sfp_reply \n", retval);
    pex_msg(KERN_ERR "   pex_sfp_init_request: incorrect reply: 0x%x 0x%x 0x%x \n", rstat, radd, rdat);
    return -EIO;
  }
  // JAM new: check reply for correct number of slaves:
  if(maxslave!=rdat)
    {
      pex_msg(KERN_ERR "** pex_sfp_init_request: slave max index mismatch: returned %d != requested %d \n", rdat, maxslave);
      return -EIO;
    }

  return retval;
}

int pex_sfp_clear_all (struct pex_privdata* privdata)
{
  u32 status = 0, loopcount = 0;
  u32 clrval=0xf;
  struct pex_sfp* sfp = &(privdata->regs.sfp);
  pex_dbg(KERN_NOTICE "**pex_sfp_clear_all ***\n");
  if (gosip_version>0) //v5 and beyond
  {
    iowrite32 (clrval, sfp->rep_stat_clr);
    pex_sfp_delay();
  }
  else
  {
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
  }
  return 0;
}

int pex_sfp_clear_channel (struct pex_privdata* privdata, int ch)
{
  u32 repstatus = 0, tokenstatus = 0, chstatus = 0, loopcount = 0;
  u32 clrval;
  struct pex_sfp* sfp = &(privdata->regs.sfp);
  pex_dbg(KERN_NOTICE "**pex_sfp_clear_channel %d ***\n", ch);
  pex_sfp_assert_channel(ch);
  clrval = (0x1 << ch);
  if (gosip_version>0) //v5 and beyond
   {
    iowrite32 (clrval, sfp->rep_stat_clr);
    pex_sfp_delay();
   }
  else
  {
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
    }
    while ((repstatus != 0x0) || (tokenstatus != 0x0) || (chstatus != 0x0));


    pex_dbg(KERN_INFO "**after pex_sfp_clear_channel %d : loopcount:%d \n", ch, loopcount);
    pex_dbg(" ... reply status: 0x%x", readl(sfp->rep_stat[ch]));
    pex_dbg(" ... token reply status: 0x%xx", readl(sfp->tk_stat[ch]));
    pex_dbg(" ... statclr: 0x%x", readl(sfp->rep_stat_clr));
  }
  return 0;
}

int pex_sfp_clear_channelpattern (struct pex_privdata* privdata, int pat)
{
  u32 repstatus = 0, loopcount = 0, clrval, mask;
  struct pex_sfp* sfp = &(privdata->regs.sfp);
  pex_dbg(KERN_NOTICE "**pex_sfp_clear_channel pattern 0x%x ***\n", pat);
  if (gosip_version>0) //v5 and beyond
   {
    iowrite32 (pat, sfp->rep_stat_clr);
    pex_sfp_delay();
   }
  else
  {
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
  }
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
  ssize_t curs = 0;
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
  curs+=snprintf (txt+curs, 512, "GOSIP FPGA code compiled at Year=%x Month=%x Date=%x Version=%x.%x \n", year, month, day,
      version[0], version[1]);
#ifdef PEX_SFP_USE_KINPEX_V5
  // this function is called in probe anyway. we use it to check if device understands fpga version 5
  curs+=snprintf (txt+curs, 512-curs, " - kernel module wants to use kinpex gosip version 5....\n");
  if(version[0]<5)
    {
    curs+=snprintf (txt+curs, 512-curs, " !!! FPGA gosip version is only %d.%d !!! downgrading driver features...\n",version[0], version[1]);
    gosip_version=0;
    }
  else
    {
    curs+=snprintf (txt+curs, 512-curs, "   OK! gosip version is %d.%d.\n",version[0], version[1]);
    }
#endif

  pex_msg (KERN_NOTICE "%s", txt); // show the gosip version anyway in syslog
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

