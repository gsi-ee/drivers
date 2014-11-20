#include "pexor_base.h"


#ifdef PEXOR_WITH_TRIXOR
int pexor_ioctl_set_trixor (struct pexor_privdata* priv, unsigned long arg)
{
  int command=0, retval=0;
  struct pexor_trixor_set descriptor;
  retval = copy_from_user (&descriptor, (void __user *) arg, sizeof(struct pexor_trixor_set));
  if (retval)
    return retval;
  command = descriptor.command;
  switch (command)
  {
    case PEXOR_TRIX_RES:
      retval=pexor_trigger_reset(priv);
      pexor_dbg(KERN_ERR "pexor_ioctl_set_trixor did reset trigger!");

      break;

    case PEXOR_TRIX_GO:

      retval=pexor_trigger_start_acq(priv);
      pexor_dbg(KERN_ERR "pexor_ioctl_set_trixor did start acquisition!");
      break;

    case PEXOR_TRIX_HALT:

      retval=pexor_trigger_stop_acq(priv);
      pexor_dbg(KERN_ERR "pexor_ioctl_set_trixor did stop acquisition!");
       break;

    case PEXOR_TRIX_TIMESET:

//      if ((ps_args->l_slave == 0) && (ps_args->l_multi == 0))
//       {
// //        printf ("\033[7m  master trigger module, single mode! \033[0m \n");
//         *ps_trig->pl_ctrl_wr = 0x1000;    // disable trigger bus
//         *ps_trig->pl_ctrl_wr = HALT;
//         *ps_trig->pl_ctrl_wr = MASTER;
//         *ps_trig->pl_ctrl_wr = CLEAR;
//
//         if ((ps_setup->bh_meb_trig_mode != 0) && (ps_setup->bh_crate_nr == 0))
//         {
//           *ps_trig->pl_fcatr_wr = 0x10000 - ps_setup->bl_trig_fct[0];
//           *ps_trig->pl_ctimr_wr = 0x10000 - ps_setup->bl_trig_cvt[0];
//           sprintf (c_line, "set up SPECIAL trigger module as MASTER");
//           F_ERROR(ERR__MSG_INFO, 0, c_line, MASK__PRTT);
//         }
//         else
//         {
//           *ps_trig->pl_fcatr_wr = 0x10000 - ps_setup->bl_trig_fct[ps_setup->bh_crate_nr];
//           *ps_trig->pl_ctimr_wr = 0x10000 - ps_setup->bl_trig_cvt[ps_setup->bh_crate_nr];
//
//           sprintf (c_line, "trigger module set up as MASTER, crate nr: %d", ps_setup->bh_crate_nr);
//           F_ERROR(ERR__MSG_INFO, 0, c_line, MASK__PRTT);
//         }
//         ps_daqst->bh_trig_master = 1; /* master trigger module */
//      //////////////////////////////////////////////////////////////////////

      // for the moment, initialize trigger module as single branch master here, too
      // later we may add special set trixor commands for this-
//      iowrite32(TRIX_BUS_DISABLE , priv->pexor.irq_control);// disable trigger bus?
//      mb();
//      ndelay(20);
//      iowrite32(TRIX_HALT , priv->pexor.irq_control);
//      mb();
//      ndelay(20);
//      iowrite32(TRIX_MASTER , priv->pexor.irq_control);
//      mb();
//      ndelay(20);
//      iowrite32(TRIX_CLEAR , priv->pexor.irq_control);
//      mb();
//      ndelay(20);

      iowrite32 (0x10000 - descriptor.fct, priv->pexor.trix_fcti);
      mb();
      ndelay(20);
      iowrite32 (0x10000 - descriptor.cvt, priv->pexor.trix_cvti);
      mb();
      ndelay(20);

//      iowrite32(TRIX_BUS_ENABLE , priv->pexor.irq_control);// enable trigger bus only for multi mode?
//      mb();
//      ndelay(20);

      pexor_dbg(KERN_ERR "pexor_ioctl_set_trixor configured trixor as master with fct=0x%x cvt=0x%x!",descriptor.fct, descriptor.cvt);
      break;

    default:
      pexor_dbg(KERN_ERR "pexor_ioctl_set_trixor unknown command %x\n", command);
      return -EFAULT;

  };

  return retval;
}
#endif


int pexor_trigger_reset(struct pexor_privdata* priv)
{
  // taken from corresonding section of mbs m_read_meb:
  //      iowrite32(TRIX_EV_IRQ_CLEAR | TRIX_IRQ_CLEAR , priv->pexor.irq_status);
  //      mb();
  //      ndelay(20);
  // ? ? do not clear interrupts here, already done in irq handler

  //      iowrite32(TRIX_BUS_DISABLE, priv->pexor.irq_control);
  //      mb();
  //      ndelay(20);

  iowrite32 (TRIX_FC_PULSE, priv->pexor.irq_status);
  mb();
  ndelay(20);
  iowrite32 (0x100, priv->pexor.irq_control);
  mb();
  ndelay(20);
  iowrite32 (TRIX_DT_CLEAR, priv->pexor.irq_status);
  mb();
  ndelay(20);
  pexor_dbg(KERN_NOTICE "pexor_trigger_reset done.\n");
  return 0;
}

int pexor_trigger_start_acq(struct pexor_privdata* priv)
{
// raise interrupt with trigger type 14 for start acquisition
  iowrite32 (TRIX_HALT, priv->pexor.irq_control);
  mb();
  udelay(1000);
  // do we need this not to hang up trixor?
  iowrite32 (TRIX_CLEAR, priv->pexor.irq_control);    // this will also clear local event counter
  mb();
  ndelay(20);
  iowrite32 (PEXOR_TRIGTYPE_START, priv->pexor.irq_status);    // put start acquisition trigger 14
  mb();
  ndelay(20);
  iowrite32 ((TRIX_EN_IRQ | TRIX_GO), priv->pexor.irq_control);
  mb();
  ndelay(20);
  //pexor_dbg(KERN_ERR "pexor_trigger_start_acq done .!\n");
return 0;
}



int pexor_trigger_stop_acq (struct pexor_privdata* priv)
{
  int retval = 0;
  iowrite32 (TRIX_HALT, priv->pexor.irq_control);
  mb();
  ndelay(20);
  // TODO: check for last trigger event finished like in mbs? only for multi branches...
  udelay(10000);
  // do we need this not to hang up trixor?

  iowrite32 (TRIX_CLEAR, priv->pexor.irq_control);
  mb();
  ndelay(20);

  iowrite32 (PEXOR_TRIGTYPE_STOP, priv->pexor.irq_status);    // put special stop trigger
  mb();
  ndelay(20);
  iowrite32 ((TRIX_EN_IRQ | TRIX_GO), priv->pexor.irq_control);
  mb();
  ndelay(20);
  pexor_dbg(KERN_NOTICE "pexor_trigger_stop_acq done.\n");

  // note: the actual halt on trixor register is performed in pexor_trigger_do_stop
  // on handling the special trigger 15

  return retval;
}


int pexor_trigger_do_stop (struct pexor_privdata* priv)
{
  int retval = 0;
  iowrite32 (TRIX_HALT, priv->pexor.irq_control);
  mb();
  udelay(10000);
  // do we need this not to hang up trixor?
  iowrite32 (TRIX_CLEAR, priv->pexor.irq_control);
  mb();
  ndelay(20);
  atomic_set(&(priv->state), PEXOR_STATE_STOPPED);
  // reset auto trigger readout mode

  return retval;
}




int pexor_ioctl_init_bus (struct pexor_privdata* priv, unsigned long arg)
{
  int retval = 0;
  u32 sfp = 0;/*,comm=0;*/
  int slave = 0;
  struct pexor_bus_io descriptor;
  struct pexor_sfp* sfpregisters = &(priv->pexor.sfp);
  retval = copy_from_user (&descriptor, (void __user *) arg, sizeof(struct pexor_bus_io));
  if (retval)
    return retval;

  sfp = (u32) descriptor.sfp;    // sfp connection to initialize chain
  slave = descriptor.slave;    // maximum # of connected slave boards
  // for pex standard sfp code, we use this ioctl to initalize chain of slaves:
  retval = pexor_sfp_clear_channel (priv, sfp);
  if (retval)
    return retval;
  retval = pexor_sfp_init_request (priv, sfp, slave);
  if (retval)
    return retval;
  sfpregisters->num_slaves[sfp] = slave; /* keep track of existing slaves for configuration broadcast*/
  return retval;

}

int pexor_ioctl_get_sfp_links (struct pexor_privdata* priv, unsigned long arg)
{
  int retval = 0;
  u32 sfp = 0;
  struct pexor_sfp_links descriptor;
  struct pexor_sfp* sfpregisters = &(priv->pexor.sfp);
  for (sfp = 0; sfp < PEXOR_SFP_NUMBER; ++sfp)
  {
    descriptor.numslaves[sfp] = sfpregisters->num_slaves[sfp];
  }
  retval = copy_to_user ((void __user *) arg, &descriptor, sizeof(struct pexor_sfp_links));
  return retval;
}

int pexor_ioctl_write_bus (struct pexor_privdata* priv, unsigned long arg)
{
  int retval = 0;
  struct pexor_bus_io descriptor;
  retval = copy_from_user (&descriptor, (void __user *) arg, sizeof(struct pexor_bus_io));
  if (retval)
    return retval;
  retval = pexor_sfp_broadcast_write_bus (priv, &descriptor); /* everything is subfunctions now*/
  if (retval)
    return retval;
  retval = copy_to_user ((void __user *) arg, &descriptor, sizeof(struct pexor_bus_io));
  return retval;
}

int pexor_ioctl_read_bus (struct pexor_privdata* priv, unsigned long arg)
{
  int retval = 0;
  struct pexor_bus_io descriptor;
  retval = copy_from_user (&descriptor, (void __user *) arg, sizeof(struct pexor_bus_io));
  if (retval)
    return retval;
  retval = pexor_sfp_read_bus (priv, &descriptor); /* everything is subfunctions now*/
  if (retval)
    return retval;
  retval = copy_to_user ((void __user *) arg, &descriptor, sizeof(struct pexor_bus_io));
  return retval;
}

int pexor_ioctl_configure_bus (struct pexor_privdata* priv, unsigned long arg)
{
  int retval = 0, i = 0;
  struct pexor_bus_config descriptor;
  retval = copy_from_user (&descriptor, (void __user *) arg, sizeof(struct pexor_bus_config));
  if (retval)
    return retval;
  if (descriptor.numpars > PEXOR_MAXCONFIG_VALS)
  {
    pexor_msg(
        KERN_ERR "** pexor_ioctl_configure_bus: warning too many parameters %d , reduced to %d\n", descriptor.numpars, PEXOR_MAXCONFIG_VALS);
    descriptor.numpars = PEXOR_MAXCONFIG_VALS;
  }
  pexor_dbg(KERN_NOTICE "** pexor_ioctl_configure_bus with %d parameters\n", descriptor.numpars);
  for (i = 0; i < descriptor.numpars; ++i)
  {
    struct pexor_bus_io data = descriptor.param[i];
    retval = pexor_sfp_broadcast_write_bus (priv, &data);
    if (retval)
    {
      pexor_msg(
          KERN_ERR "** pexor_ioctl_configure_bus: error %d at pexor_sfp_broadcast_write_bus for value i=%d\n", retval, i);
      return retval;
    }
  }
  mb();
  udelay(1000);
  /* set waitstate after configure*/
  return retval;
}

int pexor_sfp_broadcast_write_bus (struct pexor_privdata* priv, struct pexor_bus_io* data)
{
  int retval = 0, i = 0, sl = 0, sfp = 0;
  char sfpbroadcast = 0, slavebroadcast = 0;
  unsigned long address = 0, value = 0;
  struct pexor_sfp* sfpregisters = &(priv->pexor.sfp);
  address = data->address;
  value = data->value; /* save this because pexor_bus_io will be changed by write bus!*/
  mb();
  if (data->sfp < 0)
    sfpbroadcast = 1;
  if (data->slave < 0)
    slavebroadcast = 1;
  pexor_dbg(KERN_NOTICE "** pexor_sfp_broadcast_write_bus with sfpbroadcast %d slavebroadcast %d \n", sfpbroadcast,slavebroadcast);
  if (sfpbroadcast)
  {
    pexor_dbg(KERN_NOTICE "** pexor_sfp_broadcast_write_bus with sfpbroadcast\n");

    for (sfp = 0; sfp < PEXOR_SFP_NUMBER; ++sfp)
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
          retval = pexor_sfp_write_bus (priv, data);
          if (retval)
          {
            pexor_msg(
                KERN_ERR "** pexor_sfp_broadcast_write_bus: error %d at pexor_sfp_write_bus for value i=%d, sfp:%d slave:%ld \n", retval, i, data->sfp, data->slave);
            continue;
          }
        }
      }    // slavebroadcast
      else
      {
        data->address = address;
        data->value = value;
        retval = pexor_sfp_write_bus (priv, data);
        if (retval)
        {
          pexor_msg(
              KERN_ERR "** pexor_sfp_broadcast_write_bus: error %d at pexor_sfp_write_bus for value i=%d, sfp:%d slave:%ld \n", retval, i, data->sfp, data->slave);
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
      retval = pexor_sfp_write_bus (priv, data);
      if (retval)
      {
        pexor_msg(
            KERN_ERR "** pexor_sfp_broadcast_write_bus: error %d at pexor_sfp_write_bus for value i=%d, sfp:%d slave:%ld \n", retval, i, data->sfp, data->slave);
        continue;
      }
    }

  }
  else
  {
    /* single write, no broadcast loop*/
    retval = pexor_sfp_write_bus (priv, data);
    if (retval)
    {
      pexor_msg(KERN_ERR "** pexor_sfp_broadcast_write_bus: error %d at pexor_sfp_write_bus for value i=%d\n", retval, i);
      return retval;
    }
  }

  return retval;
}

int pexor_sfp_write_bus (struct pexor_privdata* priv, struct pexor_bus_io* descriptor)
{

  int retval = 0;
  u32 ad = 0, val = 0, sfp = 0, slave = 0, comm = 0;
  u32 rstat = 0, radd = 0, rdat = 0;
  u32 totaladdress = 0;
  ad = (u32) descriptor->address;
  val = (u32) descriptor->value;
  sfp = (u32) descriptor->sfp;
  slave = (u32) descriptor->slave;
  pexor_dbg(KERN_NOTICE "** pexor_sfp_write_bus writes value %x to address %x on sfp %x, slave %x\n", val, ad, sfp, slave);

  comm = PEXOR_SFP_PT_AD_W_REQ | (0x1 << (16 + sfp));
  totaladdress = ad + (slave << 24);
  pexor_sfp_clear_all (priv);
  //pexor_sfp_clear_channel(priv,sfp);
  pexor_sfp_request (priv, comm, totaladdress, val);
  //if((retval=pexor_sfp_get_reply(priv, sfp, &rstat, &radd, &rdat, 0))!=0) // debug: no response check
  if ((retval = pexor_sfp_get_reply (priv, sfp, &rstat, &radd, &rdat, PEXOR_SFP_PT_AD_W_REP)) != 0)
  {
    pexor_msg(KERN_ERR "** pexor_sfp_write_bus: error %d at sfp_reply \n", retval);
    pexor_msg(KERN_ERR "   pexor_sfp_write_bus: incorrect reply: 0x%x 0x%x 0x%x \n", rstat, radd, rdat);
    return -EIO;
  }
  descriptor->value = rstat;
  descriptor->address = radd;
  return 0;
}

int pexor_sfp_read_bus (struct pexor_privdata* priv, struct pexor_bus_io* descriptor)
{
  int retval = 0;
  u32 ad = 0, chan = 0, slave = 0, comm = 0;
  u32 rstat = 0, radd = 0, rdat = 0;
  u32 totaladdress = 0;
  ad = (u32) descriptor->address;
  chan = (u32) descriptor->sfp;
  slave = (u32) descriptor->slave;
  pexor_dbg(KERN_NOTICE "** pexor_sfp_read_bus from_address %x on sfp %x, slave %x\n", ad, chan, slave);
  comm = PEXOR_SFP_PT_AD_R_REQ | (0x1 << (16 + chan));
  totaladdress = ad + (slave << 24);
  pexor_sfp_clear_channel (priv, chan);
  pexor_sfp_request (priv, comm, totaladdress, 0);
  //if((retval=pexor_sfp_get_reply(priv, chan, &rstat, &radd, &rdat, 0))!=0) // debug:  no check
  if ((retval = pexor_sfp_get_reply (priv, chan, &rstat, &radd, &rdat, PEXOR_SFP_PT_AD_R_REP)) != 0)
  {
    pexor_msg(KERN_ERR "** pexor_sfp_read_bus: error %d at sfp_reply \n", retval);
    pexor_msg(KERN_ERR "    incorrect reply: 0x%x 0x%x 0x%x \n", rstat, radd, rdat)
    return -EIO;
  }

  descriptor->value = rdat;
  return 0;
}

int pexor_ioctl_request_token (struct pexor_privdata* priv, unsigned long arg)
{
  int retval = 0;
  u32 comm = 0, chan = 0, chanpattern, bufid = 0;
  u32 channelmask = 0;
  struct pexor_token_io descriptor;

  u32 woffset;
  unsigned long dmabufid = 0;

  retval = copy_from_user (&descriptor, (void __user *) arg, sizeof(struct pexor_token_io));
  if (retval)
    return retval;
  chan = ((u32) descriptor.sfp) & 0xFFFF;
  chanpattern = (((u32) descriptor.sfp) & 0xFFFF0000) >> 16; /* optionally use sfp pattern in upper bytes*/
  bufid = (u32) descriptor.bufid;
  /* send token request
   pexor_msg(KERN_NOTICE "** pexor_ioctl_request_token from_sfp 0x%x, bufid 0x%x\n",chan,bufid);*/
  pexor_sfp_assert_channel(chan);

if(descriptor.directdma!=0)
{
  dmabufid = descriptor.tkbuf.addr;
  woffset = descriptor.offset;
  if (chanpattern != 0)
  {
    channelmask = (chanpattern << 1);
  }
  else
  {
    channelmask = 1 << (chan + 1);    // select SFP for PCI Express DMA
  }
  pexor_dbg(KERN_NOTICE "** pexor_ioctl_request_token uses dma buffer 0x%lx with write offset  0x%x, channelmask=0x%x\n",
      dmabufid,woffset,channelmask);

  atomic_set(&(priv->state), PEXOR_STATE_DMA_SINGLE);
  retval = pexor_next_dma (priv, 0, 0, woffset, 0, dmabufid, channelmask);
  if (retval)
  {
    /* error handling, e.g. no more dma buffer available*/
    pexor_dbg(KERN_ERR "pexor_ioctl_read_token error %d from nextdma\n", retval);
    atomic_set(&(priv->state), PEXOR_STATE_STOPPED);
    return retval;
  }
} // if directdma

if (chanpattern != 0)
  {
    pexor_dbg(KERN_NOTICE "** pexor_ioctl_request_token with channelpattern 0x%x\n", (unsigned) chanpattern);
    comm = PEXOR_SFP_PT_TK_R_REQ | (chanpattern << 16); /* token broadcast mode*/
    pexor_sfp_clear_channelpattern (priv, chanpattern);
  }
  else
  {
    pexor_dbg(KERN_NOTICE "** pexor_ioctl_request_token for channel 0x%x\n", (unsigned) chan);
    comm = PEXOR_SFP_PT_TK_R_REQ | (0x1 << (16 + chan)); /* single sfp token mode*/
    pexor_sfp_clear_channel (priv, chan);
  }

  pexor_sfp_request (priv, comm, bufid, 0); /* note: slave is not specified; the chain of all slaves will send everything to receive buffer*/
   ndelay(10000); /* give pexor time to evaluate requests?*/
  if (descriptor.sync != 0)
  {

    /* only wait here for dma buffer if synchronous*/
    return (pexor_ioctl_wait_token (priv, arg));
  }
  return retval;
}



int pexor_ioctl_wait_token (struct pexor_privdata* priv, unsigned long arg)
{
  int retval = 0;
  u32 chan = 0, chanpattern = 0, ci = 0;
  u32 rstat = 0, radd = 0, rdat = 0;
  struct pexor_dmabuf dmabuf;
  struct pexor_token_io descriptor;
  u32 woffset , oldsize=0;
  u32 dmasize=0;
  unsigned long dmabufid=0;
  struct pexor_sfp* sfp=&(priv->pexor.sfp);

  retval = copy_from_user (&descriptor, (void __user *) arg, sizeof(struct pexor_token_io));
  if (retval)
    return retval;
  chan = (u32) descriptor.sfp;

  chan = ((u32) descriptor.sfp) & 0xFFFF;
  chanpattern = (((u32) descriptor.sfp) & 0xFFFF0000) >> 16; /* optionally use sfp pattern in upper bytes*/

  //bufid = (u32) descriptor.bufid;
  /* send token request
   pexor_msg(KERN_NOTICE "** pexor_ioctl_request_token from_sfp 0x%x, bufid 0x%x\n",chan,bufid);*/

  if (chanpattern > 0)
  {
    pexor_dbg(KERN_NOTICE "** pexor_ioctl_wait_token with channel pattern 0x%x \n",chanpattern);
    // here evaluate replies of all channels belonging to pattern
    for (ci = 0; ci < PEXOR_SFP_NUMBER; ++ci)
    {
      pexor_dbg(KERN_NOTICE "** pexor_ioctl_wait_token at ci=0x%x ,mask=0x%x\n",ci, (1 << ci));
      if ((chanpattern & (1 << ci)) == 0)
        continue;
      pexor_dbg(KERN_NOTICE "** pexor_ioctl_wait_token waits for reply of ci=0x%x\n",ci);
      if ((retval = pexor_sfp_get_reply (priv, ci, &rstat, &radd, &rdat, 0)) != 0)    // debug: do not check reply status
      //if((retval=pexor_sfp_get_reply(priv, chan, &rstat, &radd, &rdat, PEXOR_SFP_PT_TK_R_REP))!=0)
      {
        pexor_msg(KERN_ERR "** pexor_ioctl_wait_token: error %d at sfp_%d reply \n",retval,ci);
        pexor_msg(KERN_ERR "    incorrect reply: 0x%x 0x%x 0x%x \n", rstat, radd, rdat)
        return -EIO;
      }
       pexor_dbg(KERN_NOTICE "** pexor_ioctl_wait_token succeeds with reply of ci=0x%x\n",ci);
       ndelay(1000);
    } // for
  }
  else
  {

    pexor_dbg(KERN_NOTICE "** pexor_ioctl_wait_token with channel0x%x \n",chan);
    pexor_sfp_assert_channel(chan);

    if ((retval = pexor_sfp_get_reply (priv, chan, &rstat, &radd, &rdat, 0)) != 0)    // debug: do not check reply status
    //if((retval=pexor_sfp_get_reply(priv, chan, &rstat, &radd, &rdat, PEXOR_SFP_PT_TK_R_REP))!=0)
    {
      pexor_msg(KERN_ERR "** pexor_ioctl_wait_token: error %d at sfp_%d reply \n",retval,chan);
      pexor_msg(KERN_ERR "    incorrect reply: 0x%x 0x%x 0x%x \n", rstat, radd, rdat)
      return -EIO;
    }
    ndelay(1000);
  }    // end wait for channelpattern reply

  /* poll for return status: not necessary, since token request command is synchronous
   * token data will be ready after sfp_get_reply */
  /*	if((retval=pexor_sfp_get_token_reply(priv, chan, &tkreply, &tkhead, &tkfoot))!=0)
   {
   pexor_msg(KERN_ERR "** pexor_ioctl_read_token: error %d at token_reply \n",retval);
   pexor_msg(KERN_ERR "    incorrect reply:0x%x head:0x%x foot:0x%x \n", tkreply, tkhead, tkfoot);
   return -EIO;
   }*/

if(descriptor.directdma ==0)
{
  if (chanpattern > 0)
  {
    pexor_msg(KERN_ERR "** pexor_ioctl_wait_token: channel pattern mode not supported without direct dma enabled! \n");
    return -EFAULT;
  }

  /* issue DMA of token data from pexor to dma buffer */
  /* find out real package length :*/
  dmasize = ioread32(sfp->tk_memsize[chan]);
  mb();
  ndelay(20);
  dmasize +=4; /* like in mbs code: empirically, real token data size is 4 bytes more!*/
  if(dmasize > PEXOR_SFP_TK_MEM_RANGE)
  {
    oldsize=dmasize;
    dmasize=PEXOR_SFP_TK_MEM_RANGE - (PEXOR_SFP_TK_MEM_RANGE % PEXOR_BURST); /* align on last proper burst interval*/
    pexor_msg(KERN_NOTICE "** pexor_ioctl_request_token reduces dma size from 0x%x to 0x%x \n",oldsize, dmasize);
  }
  pexor_dbg(KERN_NOTICE "** pexor_ioctl_request_token uses dma size 0x%x of channel %x\n",dmasize,chan);

  print_register("DUMP token dma size", sfp->tk_memsize[chan]);

  /*	pexor_msg(KERN_NOTICE "** pexor_ioctl_read_token  uses token memory %x (dma:%x)\n",sfp->tk_mem[chan],sfp->tk_mem_dma[chan]);*/
  print_register("DUMP token memory first content", sfp->tk_mem[chan]);
  print_register("DUMP token memory second content", (sfp->tk_mem[chan]+1));

  /* now handle dma buffer id and user write offset:*/
  dmabufid=descriptor.tkbuf.addr;
  woffset=descriptor.offset;
  pexor_dbg(KERN_NOTICE "** pexor_ioctl_request_token uses dma buffer 0x%lx with write offset  0x%x\n", dmabufid,woffset);

  atomic_set(&(priv->state),PEXOR_STATE_DMA_SINGLE);
  retval=pexor_next_dma( priv, sfp->tk_mem_dma[chan], 0 , woffset, dmasize, dmabufid,0);
  if(retval)
  {
    /* error handling, e.g. no more dma buffer available*/
    pexor_dbg(KERN_ERR "pexor_ioctl_read_token error %d from nextdma\n", retval);
    atomic_set(&(priv->state),PEXOR_STATE_STOPPED);
    return retval;
  }

}/* not PEXOR_DIRECT_DMA*/


  if((retval=pexor_receive_dma_buffer(priv,0,0))!=0) /* poll for dma completion and wake up "DMA wait queue""*/
  return retval;

  if ((retval = pexor_wait_dma_buffer (priv, &dmabuf)) != 0) /* evaluate wait queue of received buffers*/
  {
    pexor_msg(KERN_ERR "pexor_ioctl_read_token error %d from wait_dma_buffer\n", retval);
    return retval;
  }
  descriptor.tkbuf.addr = dmabuf.virt_addr;

if(descriptor.directdma !=0)
{
  /* find out real package length after dma has completed:*/
  descriptor.tkbuf.size = ioread32 (priv->pexor.dma_len);
  mb();
  ndelay(20);
  pexor_dbg(KERN_NOTICE "pexor_ioctl_wait_token finds token size %x bytes after direct dma\n", descriptor.tkbuf.size);
}
else
{
  descriptor.tkbuf.size=dmasize; /* account used payload size disregarding offset and DMA burst corrections.*/
} // if(descriptor.directdma !=0)

  retval = copy_to_user ((void __user *) arg, &descriptor, sizeof(struct pexor_token_io));

  return retval;

}

int pexor_ioctl_wait_trigger (struct pexor_privdata* priv, unsigned long arg)
{
  int wjifs = 0;
  int retval = 0;
  struct pexor_trigger_buf* trigstat;
  struct pexor_trigger_status descriptor;
  wjifs = wait_event_interruptible_timeout (priv->irq_trig_queue, atomic_read( &(priv->trig_outstanding) ) > 0,
      priv->wait_timeout * HZ);
  pexor_dbg(KERN_NOTICE "** pexor_wait_trigger after wait_event_interruptible_timeout with TIMEOUT %d s (=%d jiffies), waitjiffies=%d, outstanding=%d \n",priv->wait_timeout,priv->wait_timeout * HZ, wjifs, atomic_read( &(priv->trig_outstanding)));
  if (wjifs == 0)
  {
    pexor_msg(KERN_NOTICE "** pexor_wait_trigger TIMEOUT %d jiffies expired on wait_event_interruptible_timeout... \n",priv->wait_timeout * HZ);
    return PEXOR_TRIGGER_TIMEOUT;
  }
  else if (wjifs == -ERESTARTSYS)
  {
    pexor_msg(KERN_NOTICE "** pexor_wait_trigger after wait_event_interruptible_timeout woken by signal. abort wait\n");
    return -EFAULT;
  }
  else
  {
  }
  atomic_dec (&(priv->trig_outstanding));
  /* read triggerstatus of this trigger interrupt from buffering queue:*/
  spin_lock( &(priv->trigstat_lock));
  if (list_empty (&(priv->trig_status)))
  {
    spin_unlock( &(priv->trigstat_lock));
    pexor_msg(KERN_ERR "pexor_ioctl_wait_trigger never come here - list of trigger status buffers is empty! \n");
    return -EFAULT;
  }
  trigstat=list_first_entry(&(priv->trig_status), struct pexor_trigger_buf, queue_list);
  /* the oldest triggerstatus to process is always at the front of list.
   * if the trigger waitqueue is woken up by isr, there must be a valid triggerstatus unless something is very wrong*/
  if (trigstat->trixorstat == 0)
  {
    spin_unlock( &(priv->trigstat_lock));
    pexor_msg(KERN_ERR "pexor_ioctl_wait_trigger never come here - first trigger status is zero! \n");
    return -EFAULT;
  }

  pexor_decode_triggerstatus(trigstat->trixorstat, &descriptor);

  trigstat->trixorstat = 0;    // mark status object as free
  list_move_tail (&(trigstat->queue_list), &(priv->trig_status));    // move to end of list
  spin_unlock( &(priv->trigstat_lock));

  pexor_dbg(KERN_NOTICE "pexor_ioctl_wait_trigger receives typ:0x%x si:0x%x mis:0x%x lec:0x%x di:0x%x tdt:0x%x eon:0x%x \n",
      descriptor.typ, descriptor.si, descriptor.mis, descriptor.lec, descriptor.di, descriptor.tdt, descriptor.eon);
  // here we have to perform the real halt if trigger was stop acquisition!
  if (descriptor.typ == PEXOR_TRIGTYPE_STOP)
  {
    pexor_trigger_do_stop(priv);
    pexor_dbg(KERN_NOTICE "pexor_ioctl_wait_trigger has trigger 0x%x, did trixor halt and clear!\n",PEXOR_TRIGTYPE_STOP);
    // to do: in this case, we have to discard all entries in interrupt queue, since they will be never read out!?
    // do we want to immediately read out old data when start acquisition occurs?
    // or wait for new trigger interrupts?
  }

  retval = copy_to_user ((void __user *) arg, &descriptor, sizeof(struct pexor_trigger_status));
  if (retval)
    return retval;
  return PEXOR_TRIGGER_FIRED;
}


void pexor_decode_triggerstatus(u32 trixorstat, struct pexor_trigger_status* result)
{
  /* decode into descriptor for consumer convenience:*/
    result->typ = (trixorstat & SEC__MASK_TRG_TYP) >> 16;
    result->si = (trixorstat & SEC__MASK_SI) >> 23;
    result->mis = (trixorstat & SEC__MASK_MIS) >> 22;
    result->lec = (trixorstat & SEC__MASK_LEC) >> 24;
    result->di = (trixorstat & SEC__MASK_DI) >> 29;
    result->tdt = (trixorstat & SEC__MASK_TDT) >> 21;
    result->eon = (trixorstat & SEC__MASK_EON) >> 31;
}



void pexor_sfp_reset (struct pexor_privdata* privdata)
{
  int i;
  struct pexor_sfp* sfp = &(privdata->pexor.sfp);
  pexor_dbg(KERN_NOTICE "**pexor_sfp_reset\n");
  iowrite32 (0xF, sfp->reset);
  mb();
  pexor_sfp_delay()
  ;
  iowrite32 (0, sfp->reset);
  mb();
  pexor_sfp_delay()
  ;
  /* reset number of slaves counter for broadcast*/
  for (i = 0; i < PEXOR_SFP_NUMBER; ++i)
  {
    sfp->num_slaves[i] = 0;
  }

  udelay(10000);
  /* wait a while after reset until sfp is ready again!*/

}

void pexor_sfp_request (struct pexor_privdata* privdata, u32 comm, u32 addr, u32 data)
{
  struct pexor_sfp* sfp = &(privdata->pexor.sfp);
  pexor_dbg(KERN_NOTICE "**pexor_sfp_request, comm=%x, addr=%x data=%x\n", comm, addr, data);
  iowrite32 (addr, sfp->req_addr);
  pexor_sfp_delay()
  ;
  iowrite32 (data, sfp->req_data);
  pexor_sfp_delay()
  ;
  iowrite32 (comm, sfp->req_comm);
  pexor_sfp_delay()
  ;
}

int pexor_sfp_get_reply (struct pexor_privdata* privdata, int ch, u32* comm, u32 *addr, u32 *data, u32 checkvalue)
{
  u32 status = 0, loopcount = 0;
  struct pexor_sfp* sfp = &(privdata->pexor.sfp);
  pexor_dbg(KERN_NOTICE "**pexor_sfp_get_reply ***\n");
  pexor_sfp_assert_channel(ch);

  do
  {
    if (loopcount++ > 100000)/* 1000000*/
    {
      pexor_msg(KERN_WARNING "**pexor_sfp_get_reply polled %d x without success, abort\n",loopcount-1);
      print_register (" ... status after FAILED pexor_sfp_get_reply:", sfp->rep_stat[ch]);
      return -EIO;
    }
    status = ioread32 (sfp->rep_stat[ch]);
    pexor_sfp_delay()
    ;

//    pexor_dbg(KERN_NOTICE "**pexor_sfp_get_reply in loop, count=%d \n",loopcount);
//    if (PEXOR_DMA_POLL_SCHEDULE)
//      schedule (); /* probably this also may help, but must not be used from tasklet*/
//    pexor_dbg(KERN_NOTICE "**pexor_sfp_get_reply after schedule\n",loopcount);

  } while (((status & 0x3000) >> 12) != 0x02); /* packet received bit is set*/
  pexor_dbg(KERN_NOTICE "**pexor_sfp_get_reply after while loop with count=%d\n", loopcount);
  *comm = ioread32 (sfp->rep_stat[ch]);
  pexor_sfp_delay()
  ;
  //pexor_dbg(KERN_NOTICE "**pexor_sfp_get_reply after reading comm \n");
  *addr = ioread32 (sfp->rep_addr[ch]);
  pexor_sfp_delay()
  ;
  //pexor_dbg(KERN_NOTICE "**pexor_sfp_get_reply after reading addr \n");
  *data = ioread32 (sfp->rep_data[ch]);
  pexor_sfp_delay()
  ;
  //pexor_dbg(KERN_NOTICE "**pexor_sfp_get_reply after reading dat \n");
  pexor_dbg(KERN_NOTICE "pexor_sfp_get_reply from SFP: %x got status:%x address:%x data: %x \n", ch,*comm, *addr, *data);
  if (checkvalue == 0)
    return 0;    // no check of reply structure
  if ((*comm & 0xfff) == checkvalue)
  {
    if ((*comm & 0x4000) != 0)
    {
      pexor_msg(KERN_ERR "pexor_sfp_get_reply: ERROR: Packet Structure : Command Reply 0x%x \n", *comm);
      return -EIO;
    }
  }
  else
  {
    pexor_msg(KERN_ERR "pexor_sfp_get_reply: ERROR : Command Reply  0x%x is not matching expected value 0x%x\n", (*comm & 0xfff), checkvalue);
    return -EIO;
  }
  return 0;

}

int pexor_sfp_get_token_reply (struct pexor_privdata* privdata, int ch, u32* stat, u32* head, u32* foot)
{
  u32 status = 0, loopcount = 0;
  struct pexor_sfp* sfp = &(privdata->pexor.sfp);
  pexor_dbg(KERN_NOTICE "**pexor_sfp_get_reply ***\n");
  pexor_sfp_assert_channel(ch);

  do
  {
    if (loopcount++ > 1000000)
    {
      pexor_msg(KERN_WARNING "**pexor_sfp_get_token reply polled %d x 20 ns without success, abort\n",loopcount);
      print_register (" ... status after FAILED pexor_sfp_get_token_reply:0x%x", sfp->tk_stat[ch]);
      return -EIO;
    }
    status = ioread32 (sfp->tk_stat[ch]);
    pexor_sfp_delay()
    ;

  } while (((status & 0x3000) >> 12) != 0x02); /* packet received bit is set*/

  *stat = ioread32 (sfp->tk_stat[ch]);
  pexor_sfp_delay()
  ;
  *head = ioread32 (sfp->tk_head[ch]);
  pexor_sfp_delay()
  ;
  *foot = ioread32 (sfp->tk_foot[ch]);
  pexor_sfp_delay()
  ;
  pexor_dbg(KERN_NOTICE "pexor_sfp_get_token_reply from SFP: %x got token status:%x header:%x footer: %x \n", ch,*stat, *head, *foot );

  return 0;
}

int pexor_sfp_init_request (struct pexor_privdata* privdata, int ch, int numslaves)
{
  int retval = 0;
  u32 sfp = 0, comm = 0, maxslave = 0;
  u32 rstat = 0, radd = 0, rdat = 0;
  sfp = (u32) ch;
  maxslave = (u32) numslaves - 1; /* changed api: pass index of max slave, not number of slaves*/
  if (numslaves <= 0)
    maxslave = 0; /* catch possible user workaround for changed api*/
  pexor_sfp_assert_channel(ch);
  comm = PEXOR_SFP_INI_REQ | (0x1 << (16 + sfp));
  pexor_dbg(KERN_NOTICE "**pexor_sfp_init_request for channel %d with maxslave index=%d ***\n",ch, maxslave);
  pexor_sfp_request (privdata, comm, 0, maxslave);
  if ((retval = pexor_sfp_get_reply (privdata, sfp, &rstat, &radd, &rdat, 0)) != 0)
  //if((retval=pexor_sfp_get_reply(privdata, sfp, &rstat, &radd, &rdat, PEXOR_SFP_PT_INI_REP))!=0)
  {
    pexor_msg(KERN_ERR "** pexor_sfp_init_request: error %d at sfp_reply \n",retval);
    pexor_msg(KERN_ERR "   pexor_sfp_init_request: incorrect reply: 0x%x 0x%x 0x%x \n", rstat, radd, rdat);
    return -EIO;
  }
  return retval;
}

int pexor_sfp_clear_all (struct pexor_privdata* privdata)
{
  u32 status = 0, loopcount = 0, clrval;
  struct pexor_sfp* sfp = &(privdata->pexor.sfp);
  clrval = 0xf;
  pexor_dbg(KERN_NOTICE "**pexor_sfp_clear_all ***\n");
  /*iowrite32(clrval, sfp->rep_stat_clr);
   pexor_sfp_delay();*/
  do
  {
    if (loopcount++ > 1000000)
    {
      pexor_msg(KERN_WARNING "**pexor_sfp_clear_all tried %d x without success, abort\n",loopcount);
      print_register (" ... stat_clr after FAILED pexor_sfp_clear_all: 0x%x", sfp->rep_stat_clr);
      return -EIO;
    }
    iowrite32 (clrval, sfp->rep_stat_clr);
    pexor_sfp_delay()
    ;
    status = ioread32 (sfp->rep_stat_clr);
    pexor_sfp_delay()
    ;
  } while (status != 0x0);
  pexor_dbg(KERN_INFO "**after pexor_sfp_clear_all: loopcount:%d \n",loopcount);
  print_register (" ... stat_clr after pexor_sfp_clear_all:", sfp->rep_stat_clr);
  return 0;
}

int pexor_sfp_clear_channel (struct pexor_privdata* privdata, int ch)
{
  u32 repstatus = 0, tokenstatus = 0, chstatus = 0, loopcount = 0, clrval;
  struct pexor_sfp* sfp = &(privdata->pexor.sfp);
  pexor_dbg(KERN_NOTICE "**pexor_sfp_clear_channel %d ***\n",ch);
  pexor_sfp_assert_channel(ch);
  clrval = (0x1 << ch);
  /*iowrite32(clrval, sfp->rep_stat_clr);
   pexor_sfp_delay();*/
  do
  {
    if (loopcount++ > 1000000)
    {
      pexor_msg(KERN_WARNING "**pexor_sfp_clear_channel %d tried %d x 20 ns without success, abort\n",ch,loopcount);
      print_register (" ... reply status after FAILED pexor_sfp_clear_channel:", sfp->rep_stat[ch]);
      print_register (" ... token reply status after FAILED pexor_sfp_clear_channel:", sfp->tk_stat[ch]);
      return -EIO;
    }

    iowrite32 (clrval, sfp->rep_stat_clr);
    pexor_sfp_delay()
    ;
    repstatus = ioread32 (sfp->rep_stat[ch]) & 0xf000;
    pexor_bus_delay();
    tokenstatus = ioread32 (sfp->tk_stat[ch]) & 0xf000;
    pexor_bus_delay();
    chstatus = ioread32 (sfp->rep_stat_clr) & clrval;
    pexor_sfp_delay()
    ;

  } while ((repstatus != 0x0) || (tokenstatus != 0x0) || (chstatus != 0x0));

  pexor_dbg(KERN_INFO "**after pexor_sfp_clear_channel %d : loopcount:%d \n",ch,loopcount);
  /*print_register(" ... reply status:", sfp->rep_stat[ch]);
   print_register(" ... token reply status:", sfp->tk_stat[ch]);
   print_register(" ... statclr:", sfp->rep_stat_clr);*/

  return 0;
}

int pexor_sfp_clear_channelpattern (struct pexor_privdata* privdata, int pat)
{
  u32 repstatus = 0, loopcount = 0, clrval, mask;
  struct pexor_sfp* sfp = &(privdata->pexor.sfp);
  pexor_dbg(KERN_NOTICE "**pexor_sfp_clear_channel pattern 0x%x ***\n", pat);
  clrval = pat;
  mask = (pat << 8) | (pat << 4) | pat;
  do
  {
    if (loopcount++ > 1000000)
    {
      pexor_msg(
          KERN_WARNING "**pex_sfp_clear_channelpattern 0x%x tried %d x 20 ns without success, abort\n", pat, loopcount);
      print_register (" ... reply status after FAILED pex_sfp_clear_channelpattern:", sfp->rep_stat_clr);
      return -EIO;
    }
    iowrite32 (clrval, sfp->rep_stat_clr);
    pexor_sfp_delay()
    ;
    repstatus = ioread32 (sfp->rep_stat_clr) & mask;
    pexor_sfp_delay()
    ;

  } while ((repstatus != 0x0));

  pexor_dbg(KERN_INFO "**after pex_sfp_clear_channelpattern 0x%x : loopcount:%d \n", pat, loopcount);
  /*print_register(" ... reply status:", sfp->rep_stat_clr); */
  return 0;
}

void set_sfp (struct pexor_sfp* sfp, void* membase, unsigned long bar)
{
  int i = 0;
  void* sfpbase = 0;
  unsigned long offset;
  if (sfp == 0)
    return;
  sfpbase = membase + PEXOR_SFP_BASE;
  sfp->version = (u32*) (sfpbase + PEXOR_SFP_VERSION);
  sfp->req_comm = (u32*) (sfpbase + PEXOR_SFP_REQ_COMM);
  sfp->req_addr = (u32*) (sfpbase + PEXOR_SFP_REQ_ADDR);
  sfp->req_data = (u32*) (sfpbase + PEXOR_SFP_REQ_DATA);
  sfp->rep_stat_clr = (u32*) (sfpbase + PEXOR_SFP_REP_STAT_CLR);
  sfp->rx_moni = (u32*) (sfpbase + PEXOR_SFP_RX_MONI);
  sfp->tx_stat = (u32*) (sfpbase + PEXOR_SFP_TX_STAT);
  sfp->reset = (u32*) (sfpbase + PEXOR_SFP_RX_RST);
  sfp->disable = (u32*) (sfpbase + PEXOR_SFP_DISA);
  sfp->fault = (u32*) (sfpbase + PEXOR_SFP_FAULT);
  for (i = 0; i < PEXOR_SFP_NUMBER; ++i)
  {
    offset = i * 0x04;
    sfp->rep_stat[i] = (u32*) (sfpbase + PEXOR_SFP_REP_STAT_0 + offset);
    sfp->rep_addr[i] = (u32*) (sfpbase + PEXOR_SFP_REP_ADDR_0 + offset);
    sfp->rep_data[i] = (u32*) (sfpbase + PEXOR_SFP_REP_DATA_0 + offset);
    sfp->fifo[i] = (u32*) (sfpbase + PEXOR_SFP_FIFO_0 + offset);
    sfp->tk_stat[i] = (u32*) (sfpbase + PEXOR_SFP_TOKEN_REP_STAT_0 + offset);
    sfp->tk_head[i] = (u32*) (sfpbase + PEXOR_SFP_TOKEN_REP_HEAD_0 + offset);
    sfp->tk_foot[i] = (u32*) (sfpbase + PEXOR_SFP_TOKEN_REP_FOOT_0 + offset);
    sfp->tk_dsize[i] = (u32*) (sfpbase + PEXOR_SFP_TOKEN_DSIZE_0 + offset);
    sfp->tk_dsize_sel[i] = (u32*) (sfpbase + PEXOR_SFP_TOKEN_DSIZE_SEL_0 + offset);
    sfp->tk_memsize[i] = (u32*) (sfpbase + PEXOR_SFP_TOKEN_MEM_SIZE_0 + offset);
    offset = i * 0x40000;
    sfp->tk_mem[i] = (u32*) (membase + PEXOR_SFP_TK_MEM_0 + offset);
    sfp->tk_mem_dma[i] = (dma_addr_t) (bar + PEXOR_SFP_TK_MEM_0 + offset);
  }

}

void print_sfp (struct pexor_sfp* sfp)
{
  int i = 0;
  if (sfp == 0)
    return;
  pexor_dbg(KERN_NOTICE "##print_sfp: ###################\n");
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
  for (i = 0; i < PEXOR_SFP_NUMBER; ++i)
  {
    pexor_dbg(KERN_NOTICE "-------- sfp number %d -------\n",i);
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
    pexor_dbg(KERN_NOTICE "token mem start DMA =%p \n",(void*) sfp->tk_mem_dma[i]);
  }

  pexor_show_version (sfp, 0);
}

void pexor_show_version (struct pexor_sfp* sfp, char* buf)
{
  /* stolen from pexor_gosip.h*/
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
  pexor_dbg(KERN_NOTICE "%s", txt);
  if (buf)
    snprintf (buf, 512, "%s", txt);
}

#ifdef PEXOR_SYSFS_ENABLE
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,18)
ssize_t pexor_sysfs_sfpregs_show(struct device *dev, struct device_attribute *attr, char *buf)
{
  ssize_t curs=0;
#ifdef PEXOR_WITH_SFP
  int i=0;
  struct dev_pexor* pg;
  struct pexor_sfp* sfp;
  struct pexor_privdata *privdata;
  privdata= (struct pexor_privdata*) dev_get_drvdata(dev);
  pg=&(privdata->pexor);
  sfp=&(pg->sfp);
  curs+=snprintf(buf+curs, PAGE_SIZE-curs, "*** PEXOR sfp register dump:\n");
  curs+=snprintf(buf+curs, PAGE_SIZE-curs, "\t request command:           0x%x\n",readl(sfp->req_comm));
  pexor_bus_delay();
  curs+=snprintf(buf+curs, PAGE_SIZE-curs, "\t request address:           0x%x\n",readl(sfp->req_addr));
  pexor_bus_delay();
  curs+=snprintf(buf+curs, PAGE_SIZE-curs, "\t reply status /clear:       0x%x\n",readl(sfp->rep_stat_clr));
  pexor_bus_delay();
  curs+=snprintf(buf+curs, PAGE_SIZE-curs, "\t rx monitor:                0x%x\n",readl(sfp->rx_moni));
  pexor_bus_delay();
  curs+=snprintf(buf+curs, PAGE_SIZE-curs, "\t tx status:                 0x%x\n",readl(sfp->tx_stat));
  pexor_bus_delay();
  curs+=snprintf(buf+curs, PAGE_SIZE-curs, "\t reset:                     0x%x\n",readl(sfp->reset));
  pexor_bus_delay();
  curs+=snprintf(buf+curs, PAGE_SIZE-curs, "\t disable:                   0x%x\n",readl(sfp->disable));
  pexor_bus_delay();
  curs+=snprintf(buf+curs, PAGE_SIZE-curs, "\t fault:                     0x%x\n",readl(sfp->fault));
  pexor_bus_delay();
  for(i=0; i<PEXOR_SFP_NUMBER;++i)
  {
    curs+=snprintf(buf+curs, PAGE_SIZE-curs, "\t  ** sfp %d:\n",i);
    curs+=snprintf(buf+curs, PAGE_SIZE-curs, "\t\t  reply status:  0x%x\n",readl(sfp->rep_stat[i]));
    pexor_bus_delay();
    curs+=snprintf(buf+curs, PAGE_SIZE-curs, "\t\t  reply address: 0x%x\n",readl(sfp->rep_addr[i]));
    pexor_bus_delay();
    curs+=snprintf(buf+curs, PAGE_SIZE-curs, "\t\t  reply data:    0x%x\n",readl(sfp->rep_data[i]));
    pexor_bus_delay();
    curs+=snprintf(buf+curs, PAGE_SIZE-curs, "\t\t  token memsize: 0x%x\n",readl(sfp->tk_memsize[i]));
    pexor_bus_delay();
  }

#else
  curs+=snprintf(buf+curs, PAGE_SIZE-curs, "*** PEXOR: no sfp register support!\n");
#endif
  return curs;
}

#endif
#endif


