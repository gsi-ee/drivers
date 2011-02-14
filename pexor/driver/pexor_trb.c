#include "pexor_base.h"

#ifdef PEXOR_WITH_TRBNET






int pexor_ioctl_trbnet_request(struct pexor_privdata* priv, unsigned long arg)
{
  int command,retval;
  int rcvbuf=0;
  u32 dmastat=0;
  struct pexor_dmabuf* nextbuf;
  struct pexor_dmabuf dmabuf;
  struct pexor_trbnet_io descriptor;
  retval = copy_from_user(&descriptor, (void __user *)arg, sizeof(struct pexor_trbnet_io));
  if (retval) return retval;
  command = descriptor.command;
  switch (command)
    {

    case PEXOR_TRBNETCOM_REG_WRITE:
      pexor_msg(KERN_ERR "pexor_ioctl_trbnet_request not implemented %x\n", command);
      break;

    case PEXOR_TRBNETCOM_REG_WRITE_MEM:
      pexor_msg(KERN_ERR "pexor_ioctl_trbnet_request not implemented %x\n", command);
      break;

    case PEXOR_TRBNETCOM_REG_READ:
      // first send trbnet request
      pexor_dbg(KERN_ERR "pexor_ioctl_trbnet_request writing to %x\n", priv->pexor.trbnet_sender_err[3]);
      iowrite32(0x00000000, priv->pexor.trbnet_sender_err[3]);
      pexor_msg(KERN_ERR "pexor_ioctl_trbnet_request writing %x to %p\n",
		descriptor.reg_address, priv->pexor.trbnet_sender_data[3])
	iowrite32(descriptor.reg_address, priv->pexor.trbnet_sender_data[3]);
      iowrite32(0x00000000, priv->pexor.trbnet_sender_data[3]);
      iowrite32(0x00000000, priv->pexor.trbnet_sender_data[3]);
      iowrite32(0x00000000, priv->pexor.trbnet_sender_data[3]);
      pexor_dbg(KERN_ERR "pexor_ioctl_trbnet_request  writing %x to %x\n",
		(((u32)descriptor.trb_address << 16) | PEXOR_TRB_CMD_REGISTER_READ), priv->pexor.trbnet_sender_ctl[3]);
      iowrite32((((u32)descriptor.trb_address << 16) | PEXOR_TRB_CMD_REGISTER_READ), priv->pexor.trbnet_sender_ctl[3]);
            
      /*priv->pexor.dma_control_stat = priv->pexor.trbnet_dma_ctl[3]; assign dma control register to channel 3 */

      pexor_dbg(KERN_ERR "pexor_ioctl_trbnet_request: dma control is %x\n",  priv->pexor.dma_control_stat);

      // here loop on dma:
      do {
	spin_lock( &(priv->buffers_lock) );
	if(list_empty(&(priv->free_buffers)))
	  {
	    spin_unlock( &(priv->buffers_lock) );
	    pexor_dbg(KERN_ERR "pexor_ioctl_trbnet_request: list of free buffers is empty. try again later! \n");
	    return -EINVAL;
	    /* TODO: handle dynamically what to do when running out of dma buffers*/
	  }
	nextbuf = list_first_entry(&priv->free_buffers, struct pexor_dmabuf, queue_list);
	spin_unlock(&priv->buffers_lock);
	      
	pexor_dbg(KERN_NOTICE "#### pexor_ioctl_trbnet_request will initiate dma %d from "
		  "to %p, len=%lx, burstsize=%x...\n",
		  rcvbuf, (void*) nextbuf->dma_addr,  nextbuf->size, PEXOR_BURST);

	/* OLD for comparison;
	 * iowrite32(0x0, priv->pexor.trbnet_dma_ctl[3]);                  clear dma ctrl first
	iowrite32(nextbuf->dma_addr, priv->pexor.trbnet_dma_add[3]);
	iowrite32((nextbuf->size) >> 2, priv->pexor.trbnet_dma_len[3]);
	iowrite32((PEXOR_BURST << 24), priv->pexor.trbnet_dma_ctl[3]);  set burstsize
	iowrite32(0x1, priv->pexor.trbnet_dma_ctl[3]);                  enable dma */

	iowrite32(nextbuf->dma_addr, priv->pexor.dma_dest);
	iowrite32((nextbuf->size) >> 2, priv->pexor.dma_len);
	iowrite32( PEXOR_BURST, priv->pexor.dma_burstsize);
	iowrite32(0x1, priv->pexor.dma_control_stat);                 /* enable dma */

	// wait for dma complete
	if((retval = pexor_wait_dma_buffer(priv, &dmabuf)) !=0 )
	  {
	    pexor_dbg(KERN_ERR "pexor_ioctl_trbnet_request error %d from wait_dma_buffer "
		      "for buffer %d\n", retval, rcvbuf);
	    break;
	  }
	dmastat = ioread32(priv->pexor.dma_control_stat);   /* check status: size; do we need another dma for data */
	mb();
	descriptor.tkbuf[rcvbuf].addr = dmabuf.virt_addr;
	/*descriptor.tkbuf[rcvbuf].size =
	  (ioread32(priv->pexor.trbnet_dma_ctl[3]) >> 6 );   shift 8 bit and multiply by sizeof(u32) */
	descriptor.tkbuf[rcvbuf].size = (dmastat >> 6 );  /* shift 8 bit and multiply by sizeof(u32) */
	rcvbuf++;
	if(rcvbuf >= TRBNET_MAX_BUFS)
	  {
	    pexor_msg(KERN_ERR "pexor_ioctl_trbnet_request: exceeding maximum defined "
		      "number of buffer parts 0x%x \n", TRBNET_MAX_BUFS);
	    break;
	  }
	      
      } while ((dmastat & PEXOR_TRB_BIT_DMA_MORE) != 0);
	    
      /* set rest of buffer refs to 0 for user convenience*/
      for(; rcvbuf < TRBNET_MAX_BUFS; ++rcvbuf)
	{
	  descriptor.tkbuf[rcvbuf].addr = 0;
	  descriptor.tkbuf[rcvbuf].size = 0;
	}
	    
      retval = copy_to_user((void __user *) arg, &descriptor, sizeof(struct pexor_trbnet_io));
      break;
	    
    case PEXOR_TRBNETCOM_REG_READ_MEM:
      pexor_msg(KERN_ERR "pexor_ioctl_trbnet_request not implemented %x\n", command);
      break;
	  
    default:
      pexor_dbg(KERN_ERR "pexor_ioctl_trbnet_request %x\n", command);
      return -EFAULT;
    };
	
  return 0;
}






#endif /* #ifdef PEXOR_WITH_TRBNET */

