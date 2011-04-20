#include "pexor_base.h"

#ifndef PEXOR_WITH_TRBNET



int pexor_ioctl_set_trixor(struct pexor_privdata* priv, unsigned long arg)
{
  int command,retval;
  struct pexor_trixor_set descriptor;
  retval=copy_from_user(&descriptor, (void __user *) arg, sizeof(struct pexor_trixor_set));
  if(retval) return retval;
  command=descriptor.command;
  switch(command)
    {
    case PEXOR_TRIX_RES:
      iowrite32(TRIX_CLEAR, priv->pexor.irq_control);
      mb();
      ndelay(20);
      break;

    case PEXOR_TRIX_GO:
      iowrite32((TRIX_EN_IRQ | TRIX_GO), priv->pexor.irq_control);
      mb();
      ndelay(20);
      break;

    case PEXOR_TRIX_HALT:
      iowrite32(TRIX_HALT , priv->pexor.irq_control);
      mb();
      ndelay(20);
      break;

    case PEXOR_TRIX_TIMESET:
      iowrite32(0x10000 - descriptor.fct , priv->pexor.trix_fcti);
      mb();
      ndelay(20);
      iowrite32(0x10000 - descriptor.cvt , priv->pexor.trix_cvti);
      mb();
      ndelay(20);

      break;

    default:
      pexor_dbg(KERN_ERR "pexor_ioctl_set_trixor unknown command %x\n", command);
      return -EFAULT;


    };

  return 0;
}


int pexor_ioctl_init_bus(struct pexor_privdata* priv, unsigned long arg)
{
  int retval=0;
  u32 sfp=0, slave=0;/*,comm=0;*/
  struct pexor_bus_io descriptor;
  retval=copy_from_user(&descriptor, (void __user *) arg, sizeof(struct pexor_bus_io));
  if(retval) return retval;
  sfp  = (u32) descriptor.sfp; // sfp connection to initialize chain
  slave = (u32) descriptor.slave; // maximum # of connected slave boards
  // for pexor standard sfp code, we use this ioctl to initalize chain of slaves:
  retval=pexor_sfp_clear_channel(priv,sfp);
  if(retval) return retval;
  retval = pexor_sfp_init_request(priv,sfp,slave);
  if(retval) return retval;
  return retval;


}



int pexor_ioctl_write_bus(struct pexor_privdata* priv, unsigned long arg)
{
  int retval=0;
  u32 ad=0,val=0,sfp=0, slave=0,comm=0;

  u32 totaladdress=0;
  u32 rstat=0, radd=0, rdat=0;
  struct pexor_bus_io descriptor;
  retval=copy_from_user(&descriptor, (void __user *) arg, sizeof(struct pexor_bus_io));
  if(retval) return retval;
  ad= (u32) descriptor.address;
  val = (u32) descriptor.value;
  sfp  = (u32) descriptor.sfp;
  slave = (u32) descriptor.slave;
  pexor_dbg(KERN_NOTICE "** pexor_ioctl_write_bus writes value %x to address %x on sfp %x, slave %x\n",val,ad,sfp,slave);


  comm = PEXOR_SFP_PT_AD_W_REQ | ( 0x1 << (16 + sfp) );
  totaladdress = ad + (slave << 24);
  pexor_sfp_clear_all(priv);
  //pexor_sfp_clear_channel(priv,sfp);
  pexor_sfp_request(priv, comm, totaladdress, val);
  //if((retval=pexor_sfp_get_reply(priv, sfp, &rstat, &radd, &rdat, 0))!=0) // debug: no response check
  if((retval=pexor_sfp_get_reply(priv, sfp, &rstat, &radd, &rdat, PEXOR_SFP_PT_AD_W_REP))!=0)
    {
      pexor_msg(KERN_ERR "** pexor_ioctl_write_bus: error %d at sfp_reply \n",retval);
      pexor_msg(KERN_ERR "   pexor_ioctl_write_bus: incorrect reply: 0x%x 0x%x 0x%x \n", rstat, radd, rdat);
      return -EIO;
    }
  descriptor.value=rstat;
  descriptor.address=radd;
  retval=copy_to_user((void __user *) arg, &descriptor, sizeof(struct pexor_bus_io));

  return retval;
}

int pexor_ioctl_read_bus(struct pexor_privdata* priv, unsigned long arg)
{
  int retval=0;
  u32 ad=0, chan=0, slave=0,comm=0;
  u32 rstat=0, radd=0, rdat=0;
  u32 totaladdress=0;
  struct pexor_bus_io descriptor;
  retval=copy_from_user(&descriptor, (void __user *) arg, sizeof(struct pexor_bus_io));
  if(retval) return retval;
  ad= (u32) descriptor.address;
  chan  = (u32) descriptor.sfp;
  slave = (u32) descriptor.slave;



  pexor_dbg(KERN_NOTICE "** pexor_ioctl_read_bus from_address %x on sfp %x, slave %x\n",ad,chan,slave);



  comm = PEXOR_SFP_PT_AD_R_REQ | ( 0x1 << (16 + chan) );
  totaladdress = ad + (slave << 24);
  pexor_sfp_clear_channel(priv,chan);
  pexor_sfp_request(priv, comm, totaladdress, 0);
  //if((retval=pexor_sfp_get_reply(priv, chan, &rstat, &radd, &rdat, 0))!=0) // debug:  no check
  if((retval=pexor_sfp_get_reply(priv, chan, &rstat, &radd, &rdat, PEXOR_SFP_PT_AD_R_REP))!=0)
    {
      pexor_msg(KERN_ERR "** pexor_ioctl_read_bus: error %d at sfp_reply \n",retval);
      pexor_msg(KERN_ERR "    incorrect reply: 0x%x 0x%x 0x%x \n", rstat, radd, rdat)
	return -EIO;
    }

  descriptor.value=rdat;
  retval=copy_to_user((void __user *) arg, &descriptor, sizeof(struct pexor_bus_io));

  return retval;
}

int pexor_ioctl_request_token(struct pexor_privdata* priv, unsigned long arg)
{
  int retval=0;
  u32 comm=0, chan=0, bufid=0;
  /*u32 rstat=0, radd=0, rdat=0;*/
  /*u32 tkreply=0, tkhead=0, tkfoot =0;*/
  /*u32 dmasize=0,oldsize=0;
    struct pexor_dmabuf dmabuf;*/
  struct pexor_token_io descriptor;
  /*
    #ifdef PEXOR_WITH_SFP
    struct pexor_sfp* sfp=&(priv->pexor.sfp);
    #endif
  */
  retval=copy_from_user(&descriptor, (void __user *) arg, sizeof(struct pexor_token_io));
  if(retval) return retval;
  chan  = (u32) descriptor.sfp;
  bufid = (u32) descriptor.bufid;
  /* send token request
     pexor_msg(KERN_NOTICE "** pexor_ioctl_request_token from_sfp 0x%x, bufid 0x%x\n",chan,bufid);*/
  pexor_sfp_assert_channel(chan);
  comm = PEXOR_SFP_PT_TK_R_REQ | (0x1 << (16+ chan) );
  pexor_sfp_clear_channel(priv,chan);
  pexor_sfp_request(priv, comm, bufid, 0); /* note: slave is not specified; the chain of all slaves will send everything to receive buffer*/
  if(descriptor.sync != 0)
    {
      /* only wait here for dma buffer if synchronous*/
      return (pexor_ioctl_wait_token(priv, arg));
    }
  return retval;
}



int pexor_ioctl_wait_token(struct pexor_privdata* priv, unsigned long arg)
{
  int retval=0;
  u32 chan=0;
  u32 rstat=0, radd=0, rdat=0;
  /*u32 tkreply=0, tkhead=0, tkfoot =0;*/
  u32 dmasize=0,oldsize=0;
  u32 dmabufid,woffset;
  struct pexor_dmabuf dmabuf;
  struct pexor_token_io descriptor;
  struct pexor_sfp* sfp=&(priv->pexor.sfp);
  retval=copy_from_user(&descriptor, (void __user *) arg, sizeof(struct pexor_token_io));
  if(retval) return retval;
  chan  = (u32) descriptor.sfp;
  //bufid = (u32) descriptor.bufid;
  /* send token request
     pexor_msg(KERN_NOTICE "** pexor_ioctl_request_token from_sfp 0x%x, bufid 0x%x\n",chan,bufid);*/
  pexor_sfp_assert_channel(chan);






  if((retval=pexor_sfp_get_reply(priv, chan, &rstat, &radd, &rdat, 0))!=0) // debug: do not check reply status
    //if((retval=pexor_sfp_get_reply(priv, chan, &rstat, &radd, &rdat, PEXOR_SFP_PT_TK_R_REP))!=0)
    {
      pexor_msg(KERN_ERR "** pexor_ioctl_request_token: error %d at sfp_reply \n",retval);
      pexor_msg(KERN_ERR "    incorrect reply: 0x%x 0x%x 0x%x \n", rstat, radd, rdat)
	return -EIO;
    }




  /* poll for return status: not necessary, since token request command is synchronous
   * token data will be ready after sfp_get_reply */
  /*	if((retval=pexor_sfp_get_token_reply(priv, chan, &tkreply, &tkhead, &tkfoot))!=0)
	{
	pexor_msg(KERN_ERR "** pexor_ioctl_read_token: error %d at token_reply \n",retval);
	pexor_msg(KERN_ERR "    incorrect reply:0x%x head:0x%x foot:0x%x \n", tkreply, tkhead, tkfoot);
	return -EIO;
	}*/

  /* issue DMA of token data from pexor to dma buffer */
  /* find out real package length :*/
  dmasize =  ioread32(sfp->tk_memsize[chan]);
  mb();
  ndelay(20);
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
  pexor_dbg(KERN_NOTICE "** pexor_ioctl_request_token uses dma buffer 0x%x with write offset  0x%x\n",dmabufid,woffset);


  atomic_set(&(priv->state),PEXOR_STATE_DMA_SINGLE);
  retval=pexor_next_dma( priv, sfp->tk_mem_dma[chan], 0 , woffset, dmasize, dmabufid);
  if(retval)
    {
      /* error handling, e.g. no more dma buffer available*/
      pexor_dbg(KERN_ERR "pexor_ioctl_read_token error %d from nextdma\n", retval);
      atomic_set(&(priv->state),PEXOR_STATE_STOPPED);
      return retval;
    }

  if((retval=pexor_wait_dma_buffer(priv, &dmabuf)) !=0)
    {
      pexor_msg(KERN_ERR "pexor_ioctl_read_token error %d from wait_dma_buffer\n", retval);
      return retval;
    }
  descriptor.tkbuf.addr=dmabuf.virt_addr;
  descriptor.tkbuf.size=dmasize; /* account used payload size disregarding offset.*/
  retval=copy_to_user((void __user *) arg, &descriptor, sizeof(struct pexor_token_io));

  return retval;

}




int pexor_ioctl_wait_trigger(struct pexor_privdata* priv, unsigned long arg)
{
  int wjifs=0;
  wjifs=wait_event_interruptible_timeout( priv->irq_trig_queue, atomic_read( &(priv->trig_outstanding) ) > 0, PEXOR_WAIT_TIMEOUT );
  pexor_dbg(KERN_NOTICE "** pexor_wait_trigger after wait_event_interruptible_timeout with TIMEOUT %d, waitjiffies=%d, outstanding=%d \n",PEXOR_WAIT_TIMEOUT, wjifs, atomic_read( &(priv->trig_outstanding)));
  if(wjifs==0)
    {
      pexor_msg(KERN_NOTICE "** pexor_wait_trigger TIMEOUT %d jiffies expired on wait_event_interruptible_timeout... \n",PEXOR_WAIT_TIMEOUT);
      return PEXOR_TRIGGER_TIMEOUT;
    }
  else if(wjifs==-ERESTARTSYS)
    {
      pexor_msg(KERN_NOTICE "** pexor_wait_trigger after wait_event_interruptible_timeout woken by signal. abort wait\n");
      return -EFAULT;
    }
  else{}
  atomic_dec(&(priv->trig_outstanding));
  return PEXOR_TRIGGER_FIRED;
}





void pexor_sfp_request( struct pexor_privdata* privdata,  u32 comm, u32 addr, u32 data )
{
  struct pexor_sfp* sfp=&(privdata->pexor.sfp);
  pexor_dbg(KERN_NOTICE "**pexor_sfp_request, comm=%x, addr=%x data=%x\n", comm, addr, data);
  iowrite32(addr, sfp->req_addr);
  pexor_sfp_delay();
  iowrite32(data, sfp->req_data);
  pexor_sfp_delay();
  iowrite32(comm, sfp->req_comm);
  pexor_sfp_delay();
}


int pexor_sfp_get_reply ( struct pexor_privdata* privdata, int ch,  u32* comm, u32 *addr, u32 *data ,u32 checkvalue)
{
  u32 status=0, loopcount=0;
  struct pexor_sfp* sfp=&(privdata->pexor.sfp);
  pexor_dbg(KERN_NOTICE "**pexor_sfp_get_reply ***\n");
  pexor_sfp_assert_channel(ch);

  do
    {
      if(loopcount++ > 1000000)/* 1000000*/
	{
	  pexor_msg(KERN_WARNING "**pexor_sfp_get_reply polled %d x without success, abort\n",loopcount);
	  print_register(" ... status after FAILED pexor_sfp_get_reply:", sfp->rep_stat[ch]);
	  return -EIO;
	}
      status= ioread32(sfp->rep_stat[ch]);
      pexor_sfp_delay();

    }
  while(((status & 0x3000) >> 12) != 0x02); /* packet received bit is set*/

  *comm =  ioread32(sfp->rep_stat[ch]);
  pexor_sfp_delay();
  *addr =  ioread32(sfp->rep_addr[ch]);
  pexor_sfp_delay();
  *data =  ioread32(sfp->rep_data[ch]);
  pexor_sfp_delay();
  pexor_dbg(KERN_NOTICE "pexor_sfp_get_reply from SFP: %x got status:%x address:%x data: %x \n", ch,*comm, *addr, *data);
  if(checkvalue==0) return 0; // no check of reply structure
  if( (*comm & 0xfff) == checkvalue)
    {
      if((*comm & 0x4000) !=0)
	{
	  pexor_msg(KERN_ERR "pexor_sfp_get_reply: ERROR: Packet Structure : Command Reply 0x%x \n", *comm);
	  return -EIO;
	}
    }
  else
    {
      pexor_msg(KERN_ERR "pexor_sfp_get_reply: ERROR : Command Reply  0x%x is not matching expected value 0x%x\n", (*comm & 0xfff),  checkvalue);
      return -EIO;
    }
  return 0;

}


int  pexor_sfp_get_token_reply ( struct pexor_privdata* privdata, int ch,  u32* stat, u32* head, u32* foot)
{
  u32 status=0, loopcount=0;
  struct pexor_sfp* sfp=&(privdata->pexor.sfp);
  pexor_dbg(KERN_NOTICE "**pexor_sfp_get_reply ***\n");
  pexor_sfp_assert_channel(ch);

  do
    {
      if(loopcount++ > 1000000)
	{
	  pexor_msg(KERN_WARNING "**pexor_sfp_get_token reply polled %d x 20 ns without success, abort\n",loopcount);
	  print_register(" ... status after FAILED pexor_sfp_get_token_reply:0x%x", sfp->tk_stat[ch]);
	  return -EIO;
	}
      status= ioread32(sfp->tk_stat[ch]);
      pexor_sfp_delay();

    }
  while(( (status & 0x3000) >> 12) != 0x02); /* packet received bit is set*/

  *stat =  ioread32(sfp->tk_stat[ch]);
  pexor_sfp_delay();
  *head =  ioread32(sfp->tk_head[ch]);
  pexor_sfp_delay();
  *foot =  ioread32(sfp->tk_foot[ch]);
  pexor_sfp_delay();
  pexor_dbg(KERN_NOTICE "pexor_sfp_get_token_reply from SFP: %x got token status:%x header:%x footer: %x \n", ch,*stat, *head, *foot );

  return 0;
}


int  pexor_sfp_init_request( struct pexor_privdata* privdata, int ch, int numslaves)
{
  int retval=0;
  u32 sfp=0, comm=0, maxslave=0;
  u32 rstat=0, radd=0, rdat=0;
  sfp= (u32) ch;
  maxslave = (u32) numslaves;
  pexor_sfp_assert_channel(ch);
  comm = PEXOR_SFP_INI_REQ | (0x1 << (16 + sfp) );
  pexor_dbg(KERN_NOTICE "**pexor_sfp_init_request ***\n");
  pexor_sfp_request(privdata, comm, 0, maxslave);
  if((retval=pexor_sfp_get_reply(privdata, sfp, &rstat, &radd, &rdat, 0))!=0)
    //if((retval=pexor_sfp_get_reply(privdata, sfp, &rstat, &radd, &rdat, PEXOR_SFP_PT_INI_REP))!=0)
    {
      pexor_msg(KERN_ERR "** pexor_sfp_init_request: error %d at sfp_reply \n",retval);
      pexor_msg(KERN_ERR "   pexor_sfp_init_request: incorrect reply: 0x%x 0x%x 0x%x \n", rstat, radd, rdat);
      return -EIO;
    }
  return retval;
}





int pexor_sfp_clear_all( struct pexor_privdata* privdata)
{
  u32 status=0, loopcount=0, clrval;
  struct pexor_sfp* sfp=&(privdata->pexor.sfp);
  clrval=0xf;
  pexor_dbg(KERN_NOTICE "**pexor_sfp_clear_all ***\n");
  /*iowrite32(clrval, sfp->rep_stat_clr);
    pexor_sfp_delay();*/
  do
    {
      if(loopcount++ > 1000000)
	{
	  pexor_msg(KERN_WARNING "**pexor_sfp_clear_all tried %d x without success, abort\n",loopcount);
	  print_register(" ... stat_clr after FAILED pexor_sfp_clear_all: 0x%x",sfp->rep_stat_clr);
	  return -EIO;
	}
      iowrite32(clrval, sfp->rep_stat_clr);
      pexor_sfp_delay();
      status=ioread32(sfp->rep_stat_clr);
      pexor_sfp_delay();
    }
  while(status != 0x0);
  pexor_dbg(KERN_INFO "**after pexor_sfp_clear_all: loopcount:%d \n",loopcount);
  print_register(" ... stat_clr after pexor_sfp_clear_all:",sfp->rep_stat_clr);
  return 0;
}


int pexor_sfp_clear_channel( struct pexor_privdata* privdata, int ch )
{
  u32 repstatus=0, tokenstatus=0, chstatus=0, loopcount=0, clrval;
  struct pexor_sfp* sfp=&(privdata->pexor.sfp);
  pexor_dbg(KERN_NOTICE "**pexor_sfp_clear_channel %d ***\n",ch);
  pexor_sfp_assert_channel(ch);
  clrval=(0x1 << ch);
  /*iowrite32(clrval, sfp->rep_stat_clr);
    pexor_sfp_delay();*/
  do
    {
      if(loopcount++ > 1000000)
	{
	  pexor_msg(KERN_WARNING "**pexor_sfp_clear_channel %d tried %d x 20 ns without success, abort\n",ch,loopcount);
	  print_register(" ... reply status after FAILED pexor_sfp_clear_channel:", sfp->rep_stat[ch]);
	  print_register(" ... token reply status after FAILED pexor_sfp_clear_channel:", sfp->tk_stat[ch]);
	  return -EIO;
	}


      iowrite32(clrval, sfp->rep_stat_clr);
      pexor_sfp_delay();
      repstatus= ioread32(sfp->rep_stat[ch]) & 0xf000;
      tokenstatus= ioread32(sfp->tk_stat[ch]) & 0xf000;
      chstatus=ioread32(sfp->rep_stat_clr) & clrval;
      pexor_sfp_delay();

    }
  while( (repstatus!=0x0) || (tokenstatus!=0x0) || (chstatus!=0x0));

  pexor_dbg(KERN_INFO "**after pexor_sfp_clear_channel %d : loopcount:%d \n",ch,loopcount);
  /*print_register(" ... reply status:", sfp->rep_stat[ch]);
    print_register(" ... token reply status:", sfp->tk_stat[ch]);
    print_register(" ... statclr:", sfp->rep_stat_clr);*/



  return 0;
}













void set_sfp(struct  pexor_sfp* sfp, void* membase, unsigned long bar)
{
  int i=0;
  void* sfpbase=0;
  unsigned long offset;
  if(sfp==0) return;
  sfpbase=membase+PEXOR_SFP_BASE;
  sfp->version=(u32*)(sfpbase+PEXOR_SFP_VERSION);
  sfp->req_comm=(u32*)(sfpbase+PEXOR_SFP_REQ_COMM);
  sfp->req_addr=(u32*)(sfpbase+PEXOR_SFP_REQ_ADDR);
  sfp->req_data=(u32*)(sfpbase+PEXOR_SFP_REQ_DATA);
  sfp->rep_stat_clr=(u32*)(sfpbase+PEXOR_SFP_REP_STAT_CLR);
  sfp->rx_moni=(u32*)(sfpbase+PEXOR_SFP_RX_MONI);
  sfp->tx_stat=(u32*)(sfpbase+PEXOR_SFP_TX_STAT);
  sfp->reset=(u32*)(sfpbase+PEXOR_SFP_RX_RST);
  sfp->disable=(u32*)(sfpbase+PEXOR_SFP_DISA);
  sfp->fault=(u32*)(sfpbase+PEXOR_SFP_FAULT);
  for(i=0; i<PEXOR_SFP_NUMBER;++i)
    {
      offset= i * 0x04;
      sfp->rep_stat[i]=(u32*)(sfpbase+PEXOR_SFP_REP_STAT_0 + offset);
      sfp->rep_addr[i]=(u32*)(sfpbase+PEXOR_SFP_REP_ADDR_0 + offset);
      sfp->rep_data[i]=(u32*)(sfpbase+PEXOR_SFP_REP_DATA_0 + offset);
      sfp->fifo[i]=(u32*)(sfpbase+PEXOR_SFP_FIFO_0 + offset);
      sfp->tk_stat[i]=(u32*)(sfpbase+PEXOR_SFP_TOKEN_REP_STAT_0 + offset);
      sfp->tk_head[i]=(u32*)(sfpbase+PEXOR_SFP_TOKEN_REP_HEAD_0 + offset);
      sfp->tk_foot[i]=(u32*)(sfpbase+PEXOR_SFP_TOKEN_REP_FOOT_0 + offset);
      sfp->tk_dsize[i]=(u32*)(sfpbase+PEXOR_SFP_TOKEN_DSIZE_0 + offset);
      sfp->tk_dsize_sel[i]=(u32*)(sfpbase+PEXOR_SFP_TOKEN_DSIZE_SEL_0 + offset);
      sfp->tk_memsize[i]=(u32*)(sfpbase+PEXOR_SFP_TOKEN_MEM_SIZE_0 + offset);
      offset= i* 0x40000;
      sfp->tk_mem[i]=(u32*)(membase+PEXOR_SFP_TK_MEM_0 + offset);
      sfp->tk_mem_dma[i]=(dma_addr_t) (bar+ PEXOR_SFP_TK_MEM_0 + offset);
    }

}






void print_sfp(struct  pexor_sfp* sfp)
{
  int i=0;
  if(sfp==0) return;
  pexor_dbg(KERN_NOTICE "##print_sfp: ###################\n");
  print_register("version", sfp->version);
  print_register("request command",sfp->req_comm);
  print_register("request address", sfp->req_addr);
  print_register("request data",sfp->req_data);

  print_register("reply status /clear",sfp->rep_stat_clr);
  print_register("monitor",sfp->rx_moni);
  print_register("tx status", sfp->tx_stat);
  print_register("reset",	sfp->reset);
  print_register("disable",sfp->disable);
  print_register("fault", sfp->fault);
  for(i=0; i<PEXOR_SFP_NUMBER;++i)
    {
      pexor_dbg(KERN_NOTICE "-------- sfp number %d -------\n",i);
      print_register("reply status",sfp->rep_stat[i]);
      print_register("reply address",sfp->rep_addr[i]);
      print_register("reply data",sfp->rep_data[i]);
      print_register("fifo",sfp->fifo[i]);
      print_register("token reply status",sfp->tk_stat[i]);
      print_register("token reply header",sfp->tk_head[i]);
      print_register("token reply footer",sfp->tk_foot[i]);
      print_register("token data size",sfp->tk_dsize[i]);
      print_register("token data size select",sfp->tk_dsize_sel[i]);
      print_register("token mem size",sfp->tk_memsize[i]);
      print_register("token mem start",sfp->tk_mem[i]);
      pexor_dbg(KERN_NOTICE "token mem start DMA =%p \n",(void*) sfp->tk_mem_dma[i]);
    }

  pexor_show_version(sfp,0);
}


void pexor_show_version(struct  pexor_sfp* sfp, char* buf)
{
  /* stolen from pexor_gosip.h*/
  u32 tmp, year,month, day, version[2];
  char txt[1024];
  tmp=ioread32(sfp->version);
  mb();
  ndelay(20);
  year=((tmp&0xff000000)>>24)+0x2000;
  month=(tmp&0xff0000)>>16;
  day=(tmp&0xff00)>>8;
  version[0]=(tmp&0xf0)>>4;
  version[1]=(tmp&0xf);
  snprintf(txt, 1024,"PEXOR FPGA code compiled at Year=%x Month=%x Date=%x Version=%x.%x \n", year,month,day,version[0],version[1]);
  pexor_dbg(KERN_NOTICE "%s", txt);
  if(buf) snprintf(buf, 1024, "%s",txt);
}



#ifdef PEXOR_SYSFS_ENABLE
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,18)
ssize_t pexor_sysfs_sfpregs_show(struct device *dev, struct device_attribute *attr, char *buf)
{
  ssize_t curs=0;
#ifdef PEXOR_WITH_SFP
  int i=0;
  struct  dev_pexor* pg;
  struct pexor_sfp* sfp;
  struct pexor_privdata *privdata;
  privdata= (struct pexor_privdata*) dev_get_drvdata(dev);
  pg=&(privdata->pexor);
  sfp=&(pg->sfp);
  curs+=snprintf(buf+curs, PAGE_SIZE-curs, "*** PEXOR sfp register dump:\n");
  curs+=snprintf(buf+curs, PAGE_SIZE-curs, "\t request command:           0x%x\n",readl(sfp->req_comm));
  curs+=snprintf(buf+curs, PAGE_SIZE-curs, "\t request address:           0x%x\n",readl(sfp->req_addr));
  curs+=snprintf(buf+curs, PAGE_SIZE-curs, "\t reply status /clear:       0x%x\n",readl(sfp->rep_stat_clr));
  curs+=snprintf(buf+curs, PAGE_SIZE-curs, "\t rx monitor:                0x%x\n",readl(sfp->rx_moni));
  curs+=snprintf(buf+curs, PAGE_SIZE-curs, "\t tx status:                 0x%x\n",readl(sfp->tx_stat));
  curs+=snprintf(buf+curs, PAGE_SIZE-curs, "\t reset:                     0x%x\n",readl(sfp->reset));
  curs+=snprintf(buf+curs, PAGE_SIZE-curs, "\t disable:                   0x%x\n",readl(sfp->disable));
  curs+=snprintf(buf+curs, PAGE_SIZE-curs, "\t fault:                     0x%x\n",readl(sfp->fault));
  for(i=0; i<PEXOR_SFP_NUMBER;++i)
    {
      curs+=snprintf(buf+curs, PAGE_SIZE-curs, "\t  ** sfp %d:\n",i);
      curs+=snprintf(buf+curs, PAGE_SIZE-curs, "\t\t  reply status:  0x%x\n",readl(sfp->rep_stat[i]));
      curs+=snprintf(buf+curs, PAGE_SIZE-curs, "\t\t  reply address: 0x%x\n",readl(sfp->rep_addr[i]));
      curs+=snprintf(buf+curs, PAGE_SIZE-curs, "\t\t  reply data:    0x%x\n",readl(sfp->rep_data[i]));
      curs+=snprintf(buf+curs, PAGE_SIZE-curs, "\t\t  token memsize: 0x%x\n",readl(sfp->tk_memsize[i]));
    }

#else
  curs+=snprintf(buf+curs, PAGE_SIZE-curs, "*** PEXOR: no sfp register support!\n");
#endif
  return curs;
}

#endif
#endif


#endif /*#ifndef PEXOR_WITH_TRBNET*/


