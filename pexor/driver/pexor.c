#include "pexor.h"


static atomic_t pexor_numdevs=ATOMIC_INIT(0);

static struct pci_device_id ids[] = {
	{ PCI_DEVICE(MY_VENDOR_ID, MY_DEVICE_ID), },
	{ 0, }
};
MODULE_DEVICE_TABLE(pci, ids);





static struct file_operations pexor_fops = {
	.owner =	THIS_MODULE,
	.llseek =   pexor_llseek,
	.read =       	pexor_read,
	.write =      	pexor_write,
	.ioctl =      	pexor_ioctl,
	.mmap  = 		pexor_mmap,
	.open =       	pexor_open,
	.release =    	pexor_release,
};

static int my_major_nr=0;


/* we support sysfs class only for new kernels to avoid backward incompatibilities here */

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,18)
static struct class* pexor_class;
#endif


int pexor_open(struct inode *inode, struct file *filp)
{
    struct pexor_privdata *privdata;
    pexor_dbg(KERN_NOTICE "** starting pexor_open...\n");
	/* Set the private data area for the file */
	privdata = container_of( inode->i_cdev, struct pexor_privdata, cdev);
	filp->private_data = privdata;
    return 0;

}

int pexor_release(struct inode *inode, struct file *filp)
{
    pexor_dbg(KERN_NOTICE "** starting pexor_release...\n");
    return 0;
}

loff_t pexor_llseek(struct file *filp, loff_t off, int whence)
{
	loff_t newpos;
	/* set cursor in mapped board RAM for read/write*/
    pexor_dbg(KERN_NOTICE "** starting pexor_llseek ...\n");
    	/* may use struct scull_dev *dev = filp->private_data; */
    	switch(whence) {
    	  case 0: /* SEEK_SET */
    		newpos = off;
    		break;

    	  case 1: /* SEEK_CUR */
    		newpos = filp->f_pos + off;
    		break;

    	  case 2: /* SEEK_END */
    		newpos = PEXOR_RAMSIZE + off;
    		break;

    	  default: /* can't happen */
    		return -EINVAL;
    	}
    	if (newpos < 0) return -EINVAL;
    	filp->f_pos = newpos;
    	return newpos;



    return 0;
}


ssize_t pexor_read(struct file *filp, char __user *buf, size_t count,
                loff_t *f_pos)
{
	/* here we read from mapped pexor memory into user buffer*/
	int i;
	ssize_t retval = 0;
    struct pexor_privdata *privdata;
    void* memstart;
    int lcount=count>>2;
    u32* kbuf=0;
  /*  u32 kbuf[lcount];*/
    pexor_dbg(KERN_NOTICE "** starting pexor_read for f_pos=%d count=%d\n", (int) *f_pos, (int) count);
    privdata= get_privdata(filp);
    if(!privdata) return -EFAULT;

    if (down_interruptible(&privdata->ramsem))
    		return -ERESTARTSYS;
    if (*f_pos >= PEXOR_RAMSIZE)
    	goto out;
    kbuf= (u32*) kmalloc(count,GFP_KERNEL);
    if(!kbuf)
    		{
    			pexor_msg(KERN_ERR "pexor_read: could not alloc %d buffer space! \n",lcount);
    			retval = -ENOMEM;
    			goto out;
    		}
	if (*f_pos + count > PEXOR_RAMSIZE)
		{
			/* TODO: better return error to inform user we exceed ram size?*/
			count = PEXOR_RAMSIZE - *f_pos;
			lcount=count>>2;
			pexor_dbg(KERN_NOTICE "** pexor_read truncates count to =%d\n", (int) count);
		}
   		memstart=(void*) (privdata->pexor.ram_start) + *f_pos;
		/* try to use intermediate kernel buffer here:*/
	pexor_dbg(KERN_NOTICE "** pexor_read begins io loop at memstart=%lx\n", (long) memstart);
	/*wmb();
	memcpy_fromio(&kbuf, memstart, count);*/
	mb();
	for(i=0;i<lcount;++i)
		{
			/*pexor_dbg(KERN_NOTICE "%x from %lx..", i,memstart+(i<<2));*/
			/*pexor_dbg(KERN_NOTICE "%d ..", i);
			if((i%10)==0) pexor_msg(KERN_NOTICE "\n");
			mb();*/
			kbuf[i]=ioread32(memstart+(i<<2));
			mb();
			/*udelay(1);*/
		}


	pexor_dbg(KERN_NOTICE "** pexor_read begins copy to user from stack buffer=%lx\n", (long) kbuf);
	if (copy_to_user(buf, kbuf, count)) {
			pexor_dbg(KERN_ERR "** pexor_read copytouser error!\n");
			retval = -EFAULT;
			goto out;
		}
	*f_pos += count;
	retval = count;
out:
	kfree(kbuf);
   	up(&privdata->ramsem);
   	return retval;

}

ssize_t pexor_write(struct file *filp, const char __user *buf, size_t count,
                loff_t *f_pos)
{
	int i;
	ssize_t retval = -ENOMEM; /* value used in "goto out" statements */
	struct pexor_privdata *privdata;
	void* memstart;
	int lcount=count>>2;
	u32* kbuf=0;
	/*u32 kbuf[lcount];*/
	pexor_dbg(KERN_NOTICE "** starting pexor_write for f_pos=%d count=%d\n", (int) *f_pos, (int) count);
	privdata= get_privdata(filp);
	if(!privdata) return -EFAULT;
	if (down_interruptible(&privdata->ramsem))
		return -ERESTARTSYS;
	if (*f_pos >= PEXOR_RAMSIZE)
		goto out;
	kbuf= (u32*) kmalloc(count,GFP_KERNEL);
	if(!kbuf)
			{
				pexor_msg(KERN_ERR "pexor_write: could not alloc %d buffer space! \n",lcount);
				retval = -ENOMEM;
				goto out;
			}

	if (*f_pos + count >= PEXOR_RAMSIZE )
		{
		/* TODO: better return error to inform user we exceed ram size?*/
			count = PEXOR_RAMSIZE  - *f_pos;
			lcount=count>>2;
			pexor_dbg(KERN_NOTICE "** pexor_write truncates count to =%d\n", (int) count);
		}
		memstart=(void*) (privdata->pexor.ram_start) + *f_pos;
	pexor_dbg(KERN_NOTICE "** pexor_write begins copy to user at stack buffer=%lx\n", (long) kbuf);
	mb();
	if (copy_from_user(kbuf, buf, count))
		{
			retval = -EFAULT;
			goto out;
		}
	pexor_dbg(KERN_NOTICE "** pexor_write begins copy loop at memstart=%lx\n", (long) memstart);
	/*memcpy_toio(memstart, kbuf , count);*/
	mb();
	for(i=0;i<lcount;++i)
	{

		/*pexor_dbg(KERN_NOTICE "kbuf[%d]=%x to %lx..", i,kbuf[i],memstart+(i<<2));*/
		/*pexor_dbg(KERN_NOTICE "%d..", i);
		if((i%10)==0) pexor_msg(KERN_NOTICE "\n");*/
		iowrite32(kbuf[i], memstart+(i<<2));
		mb();
		ndelay(20);
	}
	*f_pos += count;
	retval = count;
out:
	kfree(kbuf);
	up(&privdata->ramsem);
	return retval;



}


int pexor_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
{
	struct pexor_privdata *privdata;
	/* here validity check for magic number etc.*/

	privdata= get_privdata(filp);
	if(!privdata) return -EFAULT;
	/* Select the appropiate command */
	switch (cmd) {


	case PEXOR_IOC_RESET:
		pexor_dbg(KERN_NOTICE "** pexor_ioctl reset\n");
		return pexor_ioctl_reset(privdata,arg);
		break;

	case PEXOR_IOC_FREEBUFFER:
		pexor_dbg(KERN_NOTICE "** pexor_ioctl free buffer\n");
		return pexor_ioctl_freebuffer(privdata, arg);
		break;

	case PEXOR_IOC_DELBUFFER:
		pexor_dbg(KERN_NOTICE "** pexor_ioctl delete buffer\n");
		return pexor_ioctl_deletebuffer(privdata, arg);
		break;

	case PEXOR_IOC_WAITBUFFER:
		pexor_dbg(KERN_NOTICE "** pexor_ioctl waitbuffer\n");
		return pexor_ioctl_waitreceive(privdata, arg);
		break;


	case PEXOR_IOC_USEBUFFER:
		pexor_dbg(KERN_NOTICE "** pexor_ioctl usebuffer\n");
		return pexor_ioctl_usebuffer(privdata, arg);
		break;



	case PEXOR_IOC_CLEAR_RCV_BUFFERS:
		pexor_dbg(KERN_NOTICE "** pexor_ioctl clear receive buffers\n");
		return pexor_ioctl_clearreceivebuffers(privdata, arg);
		break;

	case PEXOR_IOC_SETSTATE:
		pexor_dbg(KERN_NOTICE "** pexor_ioctl set\n");
		return pexor_ioctl_setrunstate(privdata, arg);
		break;

	case PEXOR_IOC_TEST:
		pexor_dbg(KERN_NOTICE "** pexor_ioctl test\n");
		return pexor_ioctl_test(privdata, arg);
		break;

	case PEXOR_IOC_WRITE_BUS:
		pexor_dbg(KERN_NOTICE "** pexor_ioctl write bus\n");
		return pexor_ioctl_write_bus(privdata, arg);
		break;

	case PEXOR_IOC_READ_BUS:
		pexor_dbg(KERN_NOTICE "** pexor_ioctl read bus\n");
		return pexor_ioctl_read_bus(privdata, arg);
		break;

	case PEXOR_IOC_INIT_BUS:
		pexor_dbg(KERN_NOTICE "** pexor_ioctl init bus\n");
		return pexor_ioctl_init_bus(privdata, arg);
		break;

	case PEXOR_IOC_WRITE_REGISTER:
		pexor_dbg(KERN_NOTICE "** pexor_ioctl write register\n");
		return pexor_ioctl_write_register(privdata, arg);
		break;

	case PEXOR_IOC_READ_REGISTER:
		pexor_dbg(KERN_NOTICE "** pexor_ioctl read register\n");
		return pexor_ioctl_read_register(privdata, arg);
		break;

	case PEXOR_IOC_REQUEST_TOKEN:
		pexor_dbg(KERN_NOTICE "** pexor_ioctl request token\n");
		return pexor_ioctl_request_token(privdata, arg);
		break;

	case PEXOR_IOC_WAIT_TOKEN:
		pexor_dbg(KERN_NOTICE "** pexor_ioctl wait token\n");
		return pexor_ioctl_wait_token(privdata, arg);

	case PEXOR_IOC_WAIT_TRIGGER:
		pexor_dbg(KERN_NOTICE "** pexor_ioctl wait trigger\n");
		return pexor_ioctl_wait_trigger(privdata, arg);
		break;

	case	PEXOR_IOC_SET_TRIXOR:
                pexor_dbg(KERN_NOTICE "** pexor_ioctl set trixor\n");
	        return pexor_ioctl_set_trixor(privdata, arg);
	        break;


	default:
			return -ENOTTY;
	}
	return -ENOTTY;
}

int pexor_ioctl_freebuffer(struct pexor_privdata* priv, unsigned long arg)
{
	struct pexor_dmabuf* cursor;
	int state,retval=0;
	struct pexor_userbuf bufdescriptor;
	retval=copy_from_user(&bufdescriptor, (void __user *) arg, sizeof(struct pexor_userbuf));

	if(retval) return retval;
	spin_lock( &(priv->buffers_lock) );
	if(list_empty(&(priv->used_buffers)))
		{
			/* this may happen if user calls free buffer without taking or receiving one before*/
			spin_unlock( &(priv->buffers_lock) );
			pexor_dbg(KERN_NOTICE "** pexor_free_buffer: No more used buffers to free!\n");
			return -EFAULT;
		}
	list_for_each_entry(cursor, &(priv->used_buffers), queue_list)
			    {
					 if(cursor->virt_addr==bufdescriptor.addr)
					 {
						 pexor_dbg(KERN_NOTICE "** pexor_ioctl_freebuffer freed buffer %p\n",(void*) cursor);
						 list_move_tail(&(cursor->queue_list) , &(priv->free_buffers));
						 spin_unlock( &(priv->buffers_lock) );
						 /* ? need to sync buffer for next dma */
						 pci_dma_sync_single_for_device( priv->pdev, cursor->dma_addr, cursor->size, PCI_DMA_FROMDEVICE );

						 /* trigger here again dma flow*/
						 state=atomic_read(&(priv->state));
						 if(state==PEXOR_STATE_DMA_SUSPENDED)
							 {
								 /* this state indicates that dma flow was running out of buffer. We enable it again and restart dma*/
								 atomic_set(&(priv->state),PEXOR_STATE_DMA_FLOW);
								 pexor_dbg(KERN_NOTICE "** pexor_ioctl_freebuffer restarts dma flow \n");
								 retval=pexor_next_dma(priv, priv->pexor.ram_dma_cursor, 0 ,0); /* set previous dma source that was tried before suspend*/
								 if(retval)
									{
										atomic_set(&(priv->state),PEXOR_STATE_STOPPED);
										pexor_dbg(KERN_ALERT "** pexor_ioctl_freebuffer     NEVER COME  HERE - next dma fails although free buffers available!\n");
										return retval;
									}
							 }

						 return 0;
					 }

			    }
	spin_unlock( &(priv->buffers_lock) );
	return -EFAULT;
}


int pexor_ioctl_usebuffer(struct pexor_privdata* priv, unsigned long arg)
{
	struct pexor_dmabuf* dmabuf;
	int rev=0;
	struct pexor_userbuf userbuf;
	spin_lock( &(priv->buffers_lock) );
	if(list_empty(&(priv->free_buffers)))
		{
			/* this may happen if user calls take buffer without previous mmap, or if running out of buffers*/
			spin_unlock( &(priv->buffers_lock) );
			pexor_dbg(KERN_NOTICE "** pexor_use_buffer: No more free buffers to take!\n");
			return -EFAULT;
		}
	dmabuf=list_first_entry(&(priv->free_buffers), struct pexor_dmabuf, queue_list);
	list_move_tail(&(dmabuf->queue_list) , &(priv->used_buffers));
	spin_unlock( &(priv->buffers_lock) );
	pci_dma_sync_single_for_cpu( priv->pdev, dmabuf->dma_addr, dmabuf->size, PCI_DMA_FROMDEVICE );
	userbuf.addr=dmabuf->virt_addr;
	userbuf.size=dmabuf->size;
	rev=copy_to_user((void __user *) arg, &userbuf, sizeof(struct pexor_userbuf));
	return rev; /* if address pointers not matching */
}




int pexor_ioctl_deletebuffer(struct pexor_privdata* priv, unsigned long arg)
{
	struct pexor_dmabuf* cursor;
	int retval=0;
	struct pexor_userbuf bufdescriptor;
	retval=copy_from_user(&bufdescriptor, (void __user *) arg, sizeof(struct pexor_userbuf));
	if(retval) return retval;
	retval=pexor_poll_dma_complete(priv);
	if(retval)
            {
              pexor_msg(KERN_NOTICE "**pexor_ioctl_deletebuffer: dma is not finished, do not touch buffers!\n");
              return retval;
            }
	pexor_dma_lock((&(priv->dma_lock)));
	//spin_lock(&(priv->dma_lock));
	spin_lock( &(priv->buffers_lock) );
	if(!list_empty(&(priv->used_buffers)))
		{
			list_for_each_entry(cursor, &(priv->used_buffers), queue_list)
				{
					 if(cursor->virt_addr==bufdescriptor.addr)
					 {
						 pexor_dbg(KERN_NOTICE "** pexor_ioctl_delbuffer deleting used buffer %p\n",cursor);
						 list_del(&(cursor->queue_list));
						 delete_dmabuffer(priv->pdev, cursor);
						 spin_unlock( &(priv->buffers_lock) );
						 pexor_dma_unlock((&(priv->dma_lock)));
						 //spin_unlock(&(priv->dma_lock));
						 return 0;
					 }
				}
		}


	if(!list_empty(&(priv->free_buffers)))
		{
			list_for_each_entry(cursor, &(priv->free_buffers), queue_list)
				{
					 if(cursor->virt_addr==bufdescriptor.addr)
					 {
						 pexor_dbg(KERN_NOTICE "** pexor_ioctl_delbuffer deleting free buffer %p\n",cursor);
						 list_del(&(cursor->queue_list));
						 delete_dmabuffer(priv->pdev, cursor);
						 spin_unlock( &(priv->buffers_lock) );
						 pexor_dma_unlock((&(priv->dma_lock)));
						 //spin_unlock(&(priv->dma_lock));
						 return 0;
					 }
				}
		}
	if(!list_empty(&(priv->received_buffers)))
		{
			 list_for_each_entry(cursor, &(priv->received_buffers), queue_list)
				{
					 if(cursor->virt_addr==bufdescriptor.addr)
					 {
						 pexor_dbg(KERN_NOTICE "** pexor_ioctl_delbuffer deleting receive buffer %p\n",cursor);
						 list_del(&(cursor->queue_list));
						 delete_dmabuffer(priv->pdev, cursor);
						 spin_unlock( &(priv->buffers_lock) );
						 pexor_dma_unlock((&(priv->dma_lock)));
						 //spin_unlock(&(priv->dma_lock));
						 return 0;
					 }
				}
		}
	spin_unlock( &(priv->buffers_lock) );
	pexor_dma_unlock((&(priv->dma_lock)));
	//spin_unlock(&(priv->dma_lock));
	pexor_dbg(KERN_NOTICE "** pexor_ioctl_freebuffer could not find buffer for address %lx\n", bufdescriptor.addr);
	return -EFAULT;
}



int pexor_ioctl_waitreceive(struct pexor_privdata* priv, unsigned long arg)
{
	int rev=0;
	struct pexor_dmabuf dmabuf;
	struct pexor_userbuf userbuf;
	if((rev=pexor_wait_dma_buffer(priv, &dmabuf)) !=0)
		{
			return rev;
		}
	userbuf.addr=dmabuf.virt_addr;
	userbuf.size=dmabuf.size;
	rev=copy_to_user((void __user *) arg, &userbuf, sizeof(struct pexor_userbuf));
	return rev;
}


int pexor_ioctl_setrunstate(struct pexor_privdata* priv, unsigned long arg)
{
	int state,retval;
	retval=get_user(state, (int*) arg);
	if(retval) return retval;
	atomic_set(&(priv->state),state);
	switch(state)
	{
			case PEXOR_STATE_STOPPED:
#ifdef PEXOR_WITH_SFP
				pexor_sfp_clear_all(priv);
#endif
				/* TODO: actively stop the wait queues/tasklet etc for shutdown?*/
			case PEXOR_STATE_DMA_SUSPENDED:
				break;
			case PEXOR_STATE_DMA_FLOW:
			case PEXOR_STATE_DMA_SINGLE:
				retval=pexor_next_dma(priv, 0 , 0 , 0 ); /* TODO: set source address cursor?*/
				if(retval)
					{
						/* error handling, e.g. no more dma buffer available*/
						pexor_dbg(KERN_ERR "pexor_ioctl_setrunstate error %d from nextdma\n", retval);
						atomic_set(&(priv->state),PEXOR_STATE_STOPPED);
						return retval;
					}
				break;
			case PEXOR_STATE_IR_TEST:
				pexor_msg(KERN_NOTICE "pexor_ioctl_setting ir teststate \n");


#ifdef PEXOR_WITH_TRIXOR
				iowrite32(TRIX_CLEAR, priv->pexor.irq_control);
				        mb();
				        ndelay(2000);

				iowrite32((TRIX_EN_IRQ | TRIX_GO), priv->pexor.irq_control);
				        mb();
				        ndelay(20);
				        iowrite32(TRIX_DT_CLEAR, priv->pexor.irq_status);
				        mb();
#else

				        iowrite32(PEXOR_IRQ_USER_BIT, priv->pexor.irq_control);
				        mb();
				        ndelay(20);

				/*iowrite32(1, priv->pexor.irq_status);
				mb();*/
#endif
     			print_pexor(&(priv->pexor));
				break;

			default:
				pexor_dbg(KERN_ERR "pexor_ioctl_setrunstate unknown target state %d\n", state);
				return -EFAULT;

	}
	return 0;
}


int pexor_ioctl_set_trixor(struct pexor_privdata* priv, unsigned long arg)
{
#ifdef PEXOR_WITH_TRIXOR
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

#endif
return 0;
}

int pexor_ioctl_test(struct pexor_privdata* priv, unsigned long arg)
{
	/* curently we test here the pio of pexor ram without copy from user stuff*/
	void* memstart;
	int i,memsize,retval;
	int localbuf=0;
    retval=get_user(memsize, (int*) arg);
    if(retval) return retval;
    memstart=(void*) (priv->pexor.ram_start);
	pexor_msg(KERN_NOTICE "pexor_ioctl_test starting to write %d integers to %p\n", memsize, memstart);
    for(i=0; i<memsize;++i)
    {
    	localbuf=i;
    	iowrite32(localbuf, memstart+(i<<2));
    	mb();
    	pexor_msg(KERN_NOTICE "%d.. ", i);
    	if((i%10)==0) pexor_msg(KERN_NOTICE "\n");
    }
	pexor_msg(KERN_NOTICE "pexor_ioctl_test reading back %d integers from %p\n", memsize, memstart);
	for(i=0; i<memsize;++i)
	    {
	    	localbuf=ioread32(memstart+(i<<2));
	    	mb();
	    	if(localbuf!=i)
	    		pexor_msg(KERN_ERR "Error reading back value %d\n", i);
	    }
	pexor_msg(KERN_NOTICE "pexor_ioctl_test finished. \n");
    return 0;
}

int pexor_ioctl_reset(struct pexor_privdata* priv, unsigned long arg)
{
	pexor_dbg(KERN_NOTICE "** pexor_ioctl_reset...\n");

#ifdef PEXOR_WITH_SFP
	pexor_sfp_clear_all(priv);
	//pexor_sfp_clear_channel(priv, 1); // TODO: ioctl
	//pexor_sfp_init_request(priv,1,2); // TODO: put this into ioctl with user parameters
#endif
	cleanup_buffers(priv);
        atomic_set(&(priv->irq_count),0);

	atomic_set(&(priv->dma_outstanding), 0);
	atomic_set(&(priv->state),PEXOR_STATE_STOPPED);
#ifdef PEXOR_WITH_TRIXOR
	pexor_dbg(KERN_NOTICE "Initalizing TRIXOR... \n");
	atomic_set(&(priv->trig_outstanding), 0);
	
        iowrite32(TRIX_EV_IRQ_CLEAR | TRIX_IRQ_CLEAR, priv->pexor.irq_status);   /*reset interrupt source*/
        mb();
        ndelay(20);

	
	iowrite32(TRIX_BUS_DISABLE, priv->pexor.irq_control);
	mb();
	ndelay(20);
	
	iowrite32(TRIX_HALT , priv->pexor.irq_control);
	mb();
	ndelay(20);
	
	iowrite32(TRIX_MASTER , priv->pexor.irq_control);
	mb();
	ndelay(20);
	
	iowrite32(TRIX_CLEAR, priv->pexor.irq_control);
	mb();
	ndelay(20);
	
	
	iowrite32(0x10000 - 0x20 , priv->pexor.trix_fcti);
	mb();
	ndelay(20);
	iowrite32(0x10000 - 0x40 , priv->pexor.trix_cvti);
	mb();
	ndelay(20);
	
	iowrite32(TRIX_DT_CLEAR, priv->pexor.irq_status);
	mb();
	ndelay(20);
	
	iowrite32(TRIX_BUS_ENABLE, priv->pexor.irq_control);
	mb();
	ndelay(20);
	
	iowrite32(TRIX_HALT , priv->pexor.irq_control);
	mb();
	ndelay(20);
	
	iowrite32(TRIX_MASTER , priv->pexor.irq_control);
	mb();
	ndelay(20);
	
	iowrite32(TRIX_CLEAR, priv->pexor.irq_control);
	mb();
	ndelay(20);
	
	
			
        pexor_dbg(KERN_NOTICE " ... TRIXOR done.\n");
#else

	iowrite32(0, priv->pexor.irq_control);
	mb();
	iowrite32(0, priv->pexor.irq_status);
	mb();
#endif
	print_pexor(&(priv->pexor));
	return 0;
}

int pexor_ioctl_clearreceivebuffers(struct pexor_privdata* priv, unsigned long arg)
{
	int i=0,innerwaitcount=0, outstandingbuffers=0;
	unsigned long wjifs=0;
	struct pexor_dmabuf* cursor;
	struct pexor_dmabuf* next;
	pexor_dbg(KERN_NOTICE "** pexor_ioctl_clearreceivebuffers...\n");
	spin_lock( &(priv->buffers_lock) );
	list_for_each_entry_safe(cursor, next, &(priv->received_buffers), queue_list)
		{
			pexor_dbg(KERN_NOTICE "** pexor_ioctl_clearreceivebuffers moved %lx to free list..\n", (long) cursor);
			list_move_tail(&(cursor->queue_list) , &(priv->free_buffers));
		}
	spin_unlock( &(priv->buffers_lock) );
	/* empty possible wait queue events and dec the outstanding counter*/
	outstandingbuffers=atomic_read( &(priv->dma_outstanding));
	for(i=0;i<outstandingbuffers;++i)
		{
			while((wjifs=wait_event_interruptible_timeout( priv->irq_dma_queue, atomic_read( &(priv->dma_outstanding) ) > 0, PEXOR_WAIT_TIMEOUT )) == 0 )
				{
					pexor_msg(KERN_NOTICE "** pexor_ioctl_clearreceivebuffers TIMEOUT %d jiffies expired on wait_event_interruptible_timeout... \n",PEXOR_WAIT_TIMEOUT);
					if(innerwaitcount++ > PEXOR_WAIT_MAXTIMEOUTS) return -EFAULT;
				}
			pexor_dbg(KERN_NOTICE "** pexor_ioctl_clearreceivebuffers after wait_event_interruptible_timeout with TIMEOUT %d, waitjiffies=%ld, outstanding=%d \n",PEXOR_WAIT_TIMEOUT, wjifs, atomic_read( &(priv->dma_outstanding)));
			if(wjifs==-ERESTARTSYS)
				{
					pexor_msg(KERN_NOTICE "** pexor_ioctl_clearreceivebuffers after wait_event_interruptible_timeout woken by signal. abort wait\n");
					return -EFAULT;
				}
			atomic_dec(&(priv->dma_outstanding));
		}

#ifdef PEXOR_WITH_TRIXOR
	/* empty possible wait queue events for interrupts and dec the outstanding counter*/
	        outstandingbuffers=atomic_read( &(priv->trig_outstanding));
	        for(i=0;i<outstandingbuffers;++i)
	                {
	                        while((wjifs=wait_event_interruptible_timeout( priv->irq_trig_queue, atomic_read( &(priv->trig_outstanding) ) > 0, PEXOR_WAIT_TIMEOUT )) == 0 )
	                                {
	                                        pexor_msg(KERN_NOTICE "** pexor_ioctl_clearreceivebuffers TIMEOUT %d jiffies expired on wait_event_interruptible_timeout for trigger queue... \n",PEXOR_WAIT_TIMEOUT);
	                                        if(innerwaitcount++ > PEXOR_WAIT_MAXTIMEOUTS) return -EFAULT;
	                                }
	                        pexor_dbg(KERN_NOTICE "** pexor_ioctl_clearreceivebuffers after wait_event_interruptible_timeout with TIMEOUT %d, waitjiffies=%ld, outstanding=%d \n",PEXOR_WAIT_TIMEOUT, wjifs, atomic_read( &(priv->dma_outstanding)));
	                        if(wjifs==-ERESTARTSYS)
	                                {
	                                        pexor_msg(KERN_NOTICE "** pexor_ioctl_clearreceivebuffers after wait_event_interruptible_timeout woken by signal. abort wait\n");
	                                        return -EFAULT;
	                                }
	                        atomic_dec(&(priv->trig_outstanding));
	                }

#endif
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
#ifdef PEXOR_WITH_SFP
	// for pexor standard sfp code, we use this ioctl to initalize chain of slaves:
	retval=pexor_sfp_clear_channel(priv,sfp);
	if(retval) return retval;
	retval = pexor_sfp_init_request(priv,sfp,slave);
	if(retval) return retval;
#endif /* PEXOR_WITH_SFP*/

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

#ifdef PEXOR_WITH_SFP
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

#endif /* PEXOR_WITH_SFP*/


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


#ifdef PEXOR_WITH_SFP
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

#endif /* PEXOR_WITH_SFP */

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
#ifdef PEXOR_WITH_SFP
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
#endif /* PEXOR_WITH_SFP*/
	return retval;
}



int pexor_ioctl_wait_token(struct pexor_privdata* priv, unsigned long arg)
{
	int retval=0;
	u32 chan=0;
	u32 rstat=0, radd=0, rdat=0;
	/*u32 tkreply=0, tkhead=0, tkfoot =0;*/
	u32 dmasize=0,oldsize=0;
	struct pexor_dmabuf dmabuf;
	struct pexor_token_io descriptor;
#ifdef PEXOR_WITH_SFP
	struct pexor_sfp* sfp=&(priv->pexor.sfp);
#endif
	retval=copy_from_user(&descriptor, (void __user *) arg, sizeof(struct pexor_token_io));
	if(retval) return retval;
#ifdef PEXOR_WITH_SFP
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

	atomic_set(&(priv->state),PEXOR_STATE_DMA_SINGLE);
	retval=pexor_next_dma( priv, sfp->tk_mem_dma[chan], 0 , dmasize);
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
	descriptor.tkbuf.size=dmasize; /* this we will use also for asynch mode!*/
	retval=copy_to_user((void __user *) arg, &descriptor, sizeof(struct pexor_token_io));

#endif /* PEXOR_WITH_SFP*/

	return retval;

}


int pexor_ioctl_write_register(struct pexor_privdata* priv, unsigned long arg)
{
	int retval=0;
	u32* ad=0;
	u32 val=0;
	int bar=0;
	struct pexor_reg_io descriptor;
	retval=copy_from_user(&descriptor, (void __user *) arg, sizeof(struct pexor_reg_io));
	if(retval) return retval;
	/* here we assume something for this very connection, to be adjusted later*/
	ad= (u32*) (ptrdiff_t) descriptor.address;
	val = (u32) descriptor.value;
	bar = descriptor.bar;
	if((bar > 5) || priv->iomem[bar]==0)
		{
			pexor_msg(KERN_ERR "** pexor_ioctl_write_register: no mapped bar %d\n",bar);
			return -EIO;
		}
	pexor_dbg(KERN_NOTICE "** pexor_ioctl_write_register writes value %x to address %p within bar %d \n",val,ad,bar);
	if((unsigned long) ad  > priv->reglen[bar])
		{
			pexor_msg(KERN_ERR "** pexor_ioctl_write_register: address %p is exceeding length %lx of bar %d\n",ad, priv->reglen[bar], bar);
			return -EIO;
		}
	ad= (u32*) ( (unsigned long) priv->iomem[bar] + (unsigned long) ad);
	pexor_dbg(KERN_NOTICE "** pexor_ioctl_write_register writes value %x to mapped PCI address %p !\n",val,ad);
	iowrite32(val, ad);
	mb();
	ndelay(20);
	return retval;
}

int pexor_ioctl_read_register(struct pexor_privdata* priv, unsigned long arg)
{
	int retval=0;
	u32* ad=0;
	u32 val=0;
	int bar=0;
	struct pexor_reg_io descriptor;
	retval=copy_from_user(&descriptor, (void __user *) arg, sizeof(struct pexor_reg_io));
	if(retval) return retval;
	ad= (u32*)(ptrdiff_t)descriptor.address;
	pexor_dbg(KERN_NOTICE "** pexor_ioctl_reading from register address %p\n",ad);
	bar = descriptor.bar;
	if((bar > 5) || priv->iomem[bar]==0)
		{
			pexor_msg(KERN_ERR "** pexor_ioctl_read_register: no mapped bar %d\n",bar);
			return -EIO;
		}
	pexor_dbg(KERN_NOTICE "** pexor_ioctl_read_register reads from address %p within bar %d \n",ad,bar);
	if((unsigned long) ad  > priv->reglen[bar])
		{
			pexor_msg(KERN_ERR "** pexor_ioctl_read_register: address %p is exceeding length %lx of bar %d\n",ad, priv->reglen[bar], bar);
			return -EIO;
		}
	ad= (u32*) ( (unsigned long) priv->iomem[bar] + (unsigned long) ad);
	val=ioread32(ad);
	mb();
	ndelay(20);
	pexor_dbg(KERN_NOTICE "** pexor_ioctl_read_register read value %x from mapped PCI address %p !\n",val,ad);
	descriptor.value=val;
	retval=copy_to_user((void __user *) arg, &descriptor, sizeof(struct pexor_reg_io));
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


int pexor_mmap(struct file *filp, struct vm_area_struct *vma)
{
	struct pexor_privdata *privdata;
	struct pexor_dmabuf* buf;
	int ret = 0;
	unsigned long bufsize;
	privdata= get_privdata(filp);
	pexor_dbg(KERN_NOTICE "** starting pexor_mmap...\n");

	if(!privdata) return -EFAULT;

	bufsize=(vma->vm_end - vma->vm_start);
    pexor_dbg(KERN_NOTICE "** starting pexor_mmap for size=%ld \n", bufsize);
    /* create new dma buffer for pci and put it into free list*/
    buf=new_dmabuffer(privdata->pdev,bufsize);
    if(!buf) return -EFAULT;

    /* map kernel addresses to vma*/
    pexor_dbg(KERN_NOTICE "Mapping address %p / PFN %lx\n",
			(void*) virt_to_phys((void*)buf->kernel_addr),
			page_to_pfn(virt_to_page((void*)buf->kernel_addr)));

	vma->vm_flags |= (VM_RESERVED); /* TODO: do we need this?*/
	ret = remap_pfn_range(
					vma,
					vma->vm_start,
					page_to_pfn(virt_to_page((void*)buf->kernel_addr)),
					buf->size,
					vma->vm_page_prot );

	if (ret) {
		pexor_dbg(KERN_ERR "kmem remap failed: %d (%lx)\n", ret,buf->kernel_addr);
		delete_dmabuffer(privdata->pdev, buf);
		return -EFAULT;
	}
	buf->virt_addr=vma->vm_start; /* remember as identifier here*/
	pexor_dbg(KERN_ERR "pexor_mmap mapped kernel buffer %lx, size %lx, to virtual address %lx\n", buf->kernel_addr, buf->size,  buf->virt_addr);
    spin_lock( &(privdata->buffers_lock) );
    /* this list contains only the unused (free) buffers: */
    list_add_tail( &(buf->queue_list), &(privdata->free_buffers));
    spin_unlock( &(privdata->buffers_lock) );



	return ret;
}

struct pexor_dmabuf* new_dmabuffer(struct pci_dev * pdev, size_t size)
{
	struct pexor_dmabuf* descriptor;
	descriptor= kmalloc(sizeof(struct pexor_dmabuf), GFP_KERNEL);
	if(!descriptor)
		{
			pexor_dbg(KERN_ERR "new_dmabuffer: could not alloc dma buffer descriptor! \n");
			return NULL;
		}
	memset(descriptor, 0, sizeof(struct pexor_dmabuf));
	descriptor->size=size;
#ifdef	DMA_MAPPING_STREAMING
/* here we use plain kernel memory which we explicitly map for dma*/
	descriptor->kernel_addr=(unsigned long) kmalloc(size, GFP_KERNEL);
	if(!descriptor->kernel_addr)
			{
				pexor_msg(KERN_ERR "new_dmabuffer: could not alloc streaming dma buffer for size %d \n",size);
				kfree(descriptor);
				return NULL;
			}
	descriptor->dma_addr= dma_map_single(&(pdev->dev), (void*) descriptor->kernel_addr, size, PCI_DMA_FROMDEVICE);
	if(!descriptor->dma_addr)
				{
					pexor_msg(KERN_ERR "new_dmabuffer: could not map streaming dma buffer for size %d \n",size);
					kfree((void*) descriptor->kernel_addr);
					kfree(descriptor);
					return NULL;
				}

	pexor_dbg(KERN_ERR "new_dmabuffer created streaming kernel buffer with dma address %lx\n", descriptor->dma_addr);

#else
	/* here we get readily mapped dma memory which was preallocated for the device*/
	descriptor->kernel_addr= (unsigned long) pci_alloc_consistent(pdev, size, &(descriptor->dma_addr));
	if(!descriptor->kernel_addr)
		{
                  pexor_msg(KERN_ERR "new_dmabuffer: could not alloc pci dma buffer for size %d \n",(int)size);
			kfree(descriptor);
			return NULL;
		}
	/* maybe obsolete here, but we could gain performance by defining the data direction...*/
	pci_dma_sync_single_for_device(pdev, descriptor->dma_addr, descriptor->size, PCI_DMA_FROMDEVICE );
	pexor_dbg(KERN_ERR "new_dmabuffer created coherent kernel buffer with dma address %p\n", (void*) descriptor->dma_addr);

#endif

	INIT_LIST_HEAD(&(descriptor->queue_list));
	pexor_dbg(KERN_NOTICE "**pexor_created new_dmabuffer, size=%d, addr=%lx \n", (int) size, descriptor->kernel_addr);

	return descriptor;
}

int delete_dmabuffer(struct pci_dev * pdev, struct pexor_dmabuf* buf)
{
	pexor_dbg(KERN_NOTICE "**pexor_deleting dmabuffer, size=%ld, addr=%lx \n", buf->size, buf->kernel_addr);
	/* note: unmapping the virtual adresses is done in user application by munmap*/
#ifdef	DMA_MAPPING_STREAMING
	/* release dma mapping and free kernel memory for dma buffer*/
	dma_unmap_single(&(pdev->dev), buf->dma_addr, buf->size, PCI_DMA_FROMDEVICE);
	kfree((void*) buf->kernel_addr);
#else
	/* Release DMA memory */
	pci_free_consistent(pdev, buf->size, (void *)(buf->kernel_addr), buf->dma_addr);
#endif
	/* Release descriptor memory */
	kfree(buf);
return 0;
}


void cleanup_buffers(struct pexor_privdata* priv)
{
	struct pexor_dmabuf* cursor;
	struct pexor_dmabuf* next;
	pexor_dbg(KERN_NOTICE "**pexor_cleanup_buffers...\n");

	if(pexor_poll_dma_complete(priv))
	  {
            pexor_msg(KERN_NOTICE "**pexor_cleanup_buffers: dma is not finished, do not touch buffers!\n");
            return;
	  }
	//if(!(int) (priv->buffers_lock)) return;
	pexor_dma_lock((&(priv->dma_lock)));
	//spin_lock(&(priv->dma_lock));
	spin_lock( &(priv->buffers_lock) );
	/* remove reference in receive queue (discard contents):*/
		list_for_each_entry_safe(cursor, next, &(priv->received_buffers),
				queue_list)
		    {
				 list_del(&(cursor->queue_list)); /* put out of list*/
				 delete_dmabuffer(priv->pdev, cursor);
		    }
	/* remove reference in free list:*/
	list_for_each_entry_safe(cursor, next, &(priv->free_buffers),
	    		queue_list)
	    {
			 list_del(&(cursor->queue_list)); /* put out of list*/
			 delete_dmabuffer(priv->pdev, cursor);
	    }


	/* remove reference in used list:*/
		list_for_each_entry_safe(cursor, next, &(priv->used_buffers),
		    		queue_list)
		    {
				 list_del(&(cursor->queue_list)); /* put out of list*/
				 delete_dmabuffer(priv->pdev, cursor);
		    }
	spin_unlock( &(priv->buffers_lock) );
	pexor_dma_unlock((&(priv->dma_lock)));
	//spin_unlock(&(priv->dma_lock));
	pexor_dbg(KERN_NOTICE "**pexor_cleanup_buffers...done\n");
}


#ifdef PEXOR_WITH_SFP

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






#endif /*  PEXOR_WITH_SFP */





struct pexor_privdata* get_privdata(struct file *filp)
{
	struct pexor_privdata *privdata;
	privdata= (struct pexor_privdata*) filp->private_data;
	if(privdata->pexor.init_done==0)
		{
			pexor_dbg(KERN_ERR "*** PEXOR structure was not initialized!\n");
			return NULL;
		}
	return privdata;
}

void print_register(const char* description, u32* address)
{
	pexor_dbg(KERN_NOTICE "%s:\taddr=%lx cont=%x\n", description, (long unsigned int) address, readl(address));
}


void print_pexor(struct  dev_pexor* pg)
{
if(pg==0) return;
pexor_dbg(KERN_NOTICE "\n##print_pexor: ###################\n");
pexor_dbg(KERN_NOTICE "init: \t=%x\n", pg->init_done);
if(!pg->init_done) return;
print_register("dma control/status", pg->dma_control_stat);
print_register("irq status", pg->irq_status);
print_register("irq control", pg->irq_control);
#ifdef PEXOR_WITH_TRIXOR
/*pexor_dbg(KERN_NOTICE "trixor control add=%x \n",pg->irq_control) ;
pexor_dbg(KERN_NOTICE "trixor status  add =%x \n",pg->irq_status);
pexor_dbg(KERN_NOTICE "trixor fast clear add=%x \n",pg->trix_fcti) ;
pexor_dbg(KERN_NOTICE "trixor conversion time add =%x \n",pg->trix_cvti);*/

print_register("trixor fast clear time", pg->trix_fcti);
print_register("trixor conversion time", pg->trix_cvti);
#endif
print_register("dma source address",pg->dma_source);
print_register("dma dest   address", pg->dma_dest);
print_register("dma len   address", pg->dma_len);
print_register("dma burstsize", pg->dma_burstsize);
print_register("RAM start", pg->ram_start) ;
print_register("RAM end",pg->ram_end);
pexor_dbg(KERN_NOTICE "RAM DMA base add=%x \n",(unsigned) pg->ram_dma_base) ;
pexor_dbg(KERN_NOTICE "RAM DMA cursor add=%x \n",(unsigned) pg->ram_dma_cursor);

#ifdef PEXOR_WITH_SFP
print_sfp(&(pg->sfp));
#endif

}

void clear_pexor(struct  dev_pexor* pg)
{
    if(pg==0) return;
    pg->init_done=0x0;
    pexor_dbg(KERN_NOTICE "** Cleared pexor structure %lx.\n",(long unsigned int) pg);
}


void set_pexor(struct  dev_pexor* pg, void* membase, unsigned long bar)
{
    void* dmabase=0;
    if(pg==0) return;
    dmabase=membase+PEXOR_DMA_BASE;
#ifdef PEXOR_WITH_TRIXOR
         pg->irq_control=(u32*)(membase+ PEXOR_TRIXOR_BASE + PEXOR_TRIX_CTRL);
         pg->irq_status=(u32*)(membase+ PEXOR_TRIXOR_BASE + PEXOR_TRIX_STAT);
         pg->trix_fcti=(u32*)(membase+ PEXOR_TRIXOR_BASE + PEXOR_TRIX_FCTI);
         pg->trix_cvti=(u32*)(membase+ PEXOR_TRIXOR_BASE + PEXOR_TRIX_CVTI);
#else
    pg->irq_control=(u32*)(membase+PEXOR_IRQ_CTRL);
    pg->irq_status=(u32*)(membase+PEXOR_IRQ_STAT);
#endif
    pg->dma_control_stat=(u32*)(dmabase+PEXOR_DMA_CTRLSTAT);
	pg->dma_source=(u32*)(dmabase+PEXOR_DMA_SRC);
	pg->dma_dest=(u32*)(dmabase+PEXOR_DMA_DEST);
	pg->dma_len=(u32*)(dmabase+PEXOR_DMA_LEN);
	pg->dma_burstsize=(u32*)(dmabase+PEXOR_DMA_BURSTSIZE);

	pg->ram_start=(u32*)(membase+PEXOR_DRAM);
	pg->ram_end=(u32*)(membase+PEXOR_DRAM+PEXOR_RAMSIZE);
	pg->ram_dma_base =   (dma_addr_t) (bar+PEXOR_DRAM);
	pg->ram_dma_cursor = (dma_addr_t)(bar+PEXOR_DRAM);
#ifdef PEXOR_WITH_SFP
	set_sfp(&(pg->sfp), membase, bar);
#endif

	pg->init_done=0x1;
    pexor_dbg(KERN_NOTICE "** Set pexor structure %lx.\n",(long unsigned int) pg);

}

#ifdef PEXOR_WITH_SFP

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

#endif /*  PEXOR_WITH_SFP */

irqreturn_t pexor_isr( int irq, void *dev_id)
{
	u32 irtype;
	int state;
	struct pexor_privdata *privdata;
	/*static int count=0;*/
	/*int i=0;
	u32* address;*/

	privdata=(struct pexor_privdata *) dev_id;


#ifdef PEXOR_SHARED_IRQ






#ifdef PEXOR_WITH_TRIXOR
	/* check if this interrupt was raised by our device*/
	irtype=ioread32(privdata->pexor.irq_status);
	mb();
	ndelay(20);
	if(irtype & (TRIX_EV_IRQ_CLEAR | TRIX_DT_CLEAR)) /* test bits */
              {
              /* prepare for trixor interrupts here:*/
              irtype = TRIX_EV_IRQ_CLEAR | TRIX_IRQ_CLEAR;
              iowrite32(irtype, privdata->pexor.irq_status);   /*reset interrupt source*/
              mb();
              ndelay(20);
              /* pexor_dbg(KERN_NOTICE "pexor driver interrupt handler cleared irq status!\n");*/
              /* now find out if we did interrupt test*/
              state=atomic_read(&(privdata->state));
              if(state==PEXOR_STATE_IR_TEST)
                      {
                              pexor_msg(KERN_NOTICE "pexor driver interrupt handler sees ir test!\n");
                              state=PEXOR_STATE_STOPPED;
                              atomic_set(&(privdata->state),state);
                      }

                /* trigger interrupt from trixor. wake up waiting application if any:*/
                /* pexor_dbg(KERN_NOTICE "pexor driver interrupt handler sees trigger ir!\n"); */
                atomic_inc(&(privdata->trig_outstanding));
                wake_up_interruptible(&(privdata->irq_trig_queue));
                /* from mbs driver irqtest:*/
               /* irtype = TRIX_FC_PULSE;
                iowrite32(irtype, privdata->pexor.irq_status);    send fast clear pulse TODO: later in application
                mb();
                ndelay(20);
                irtype = TRIX_DT_CLEAR;
                iowrite32(irtype, privdata->pexor.irq_status);   clear deadtime flag TODO: later in application
                mb();
                ndelay(20);*/
                return IRQ_HANDLED;
              }
#else

	/* check if this interrupt was raised by our device*/
        irtype=ioread32(privdata->pexor.irq_status);
        mb();
        ndelay(20);
        if(irtype & PEXOR_IRQ_USER_BIT)
                {

                        /* OLD for pexor 1*/
                        mb();
                        irtype &= ~(PEXOR_IRQ_USER_BIT);
                        iowrite32(irtype, privdata->pexor.irq_control);  /*reset interrupt source*/
                        iowrite32(irtype, privdata->pexor.irq_status);   /*reset interrupt source*/
                        mb();
                        ndelay(20);
                        pexor_msg(KERN_NOTICE "pexor driver interrupt handler cleared irq status!\n");
                        /* now find out if we did interrupt test, trigger, or  real dma raised interrupt:*/
                        state=atomic_read(&(privdata->state));
                        if(state==PEXOR_STATE_IR_TEST)
                                {
                                        pexor_msg(KERN_NOTICE "pexor driver interrupt handler sees ir test!\n");
                                        state=PEXOR_STATE_STOPPED;
                                        atomic_set(&(privdata->state),state);
                                        return IRQ_HANDLED;
                                }
                        /*else
                                {
                                         ir was raised by dma complete:

                                         inc filled buffers counter
                                        atomic_inc(&(privdata->irq_count));
                                         schedule tasklet
                                        pexor_msg(KERN_NOTICE "pexor test driver interrupt handler schedules tasklet... \n");
                                        tasklet_schedule(&privdata->irq_bottomhalf);
                                         pexor_dbg(KERN_ALERT "pexor test driver interrupt handler executes PEXOR ir!\n");
                                }*/

                }
#endif



	else
		{
			pexor_dbg(KERN_NOTICE "pexor test driver interrupt handler sees unknown ir type %x !\n",irtype);
			return IRQ_NONE;
		}





	return IRQ_HANDLED;






#else
	pexor_msg(KERN_NOTICE "pexor test driver interrupt handler is executed non shared.\n");

	iowrite32(0, privdata->pexor.irq_control);
	return IRQ_HANDLED;  /* for debug*/

#endif



}


void pexor_irq_tasklet(unsigned long arg)
{
	int state,rev;
	struct pexor_privdata *privdata;
	struct pexor_dmabuf* nextbuf;
	privdata= (struct pexor_privdata*) arg;
	pexor_dbg(KERN_NOTICE "pexor_irq_tasklet is executed, irqoutstanding=%d...\n",atomic_read(&(privdata->dma_outstanding)));
	/* can we test wait queue by delaying here?*/
	/*udelay(1000);*/

	/* lock against top half?*/

	/* check how many buffers were filled before tasklet is executed. should be one! */

	if(!atomic_dec_and_test(&(privdata->irq_count)))
		{
			pexor_msg(KERN_ALERT "pexor_irq_tasklet found more than one ir: N.C.H.\n");
		}
	/* transfer buffer from free queue to receive queue*/
		spin_lock( &(privdata->buffers_lock) );

		/* check if free list is empty <- can happen if dma flow gets suspended
		 * and waitreceive is called in polling mode*/
		if(list_empty(&(privdata->free_buffers)))
				{
					spin_unlock( &(privdata->buffers_lock) );
					pexor_dbg(KERN_ERR "pexor_irq_tasklet: list of free buffers is empty. no DMA could have been received!\n");
					/* return;  this would put the waitreceive into timeout, so does not try to read empty receive queue*/
					goto wakeup; /* to have immediate response and error from receive queue as well.*/
				}

		nextbuf=list_first_entry(&(privdata->free_buffers), struct pexor_dmabuf, queue_list);
		list_move_tail(&(nextbuf->queue_list) , &(privdata->received_buffers));
		spin_unlock( &(privdata->buffers_lock) );

	state=atomic_read(&(privdata->state));
	switch(state)
	{
		case PEXOR_STATE_STOPPED:
			pexor_msg(KERN_ALERT "pexor_irq_tasklet finds stopped state before reset! N.C.H.\n");
			break;

		case PEXOR_STATE_DMA_FLOW:
			/*if(atomic_read(&(privdata->dma_outstanding))>PEXOR_MAXOUTSTANDING)
				{
					pexor_msg(KERN_ALERT "pexor_irq_tasklet finds more than %d pending receive buffers! Emergency suspend dma flow!\n",PEXOR_MAXOUTSTANDING);
					atomic_set(&(privdata->state),PEXOR_STATE_DMA_SUSPENDED);
					break;
				}*/
			rev=pexor_next_dma(privdata, 0, 0, 0); /* TODO: inc source address cursor? Handle sfp double buffering?*/
			if(rev)
				{
					/* no more dma buffers at the moment: suspend flow?*/
					atomic_set(&(privdata->state),PEXOR_STATE_DMA_SUSPENDED);
					pexor_dbg(KERN_ALERT "pexor_irq_tasklet suspends DMA flow because no more free buffers!\n");
				}
			break;
		case PEXOR_STATE_DMA_SINGLE:
		default:
			atomic_set(&(privdata->state),PEXOR_STATE_STOPPED);

	};

wakeup:
	/* wake up the waiting ioctl*/
	atomic_inc(&(privdata->dma_outstanding));
	wake_up_interruptible(&(privdata->irq_dma_queue));

}


int pexor_next_dma(struct pexor_privdata* priv, dma_addr_t source, u32 roffset, u32 dmasize)
{
	struct pexor_dmabuf* nextbuf;
	u32 enable=PEXOR_DMA_ENABLED_BIT;

	int rev,rest;
	if(source==0)
		{
			priv->pexor.ram_dma_cursor= (priv->pexor.ram_dma_base + roffset );
		}
	else
		{
			priv->pexor.ram_dma_cursor= (source + roffset );
		}
	/* setup next free buffer as dma target*/
	pexor_dbg(KERN_NOTICE "#### pexor_next_dma...\n");

	spin_lock( &(priv->buffers_lock) );
	if(list_empty(&(priv->free_buffers)))
		{
			spin_unlock( &(priv->buffers_lock) );
			pexor_dbg(KERN_ERR "pexor_next_dma: list of free buffers is empty. try again later! \n");
			return -EINVAL;
			/* TODO: handle dynamically what to do when running out of dma buffers*/
		}
	nextbuf=list_first_entry(&(priv->free_buffers), struct pexor_dmabuf, queue_list);
	spin_unlock( &(priv->buffers_lock) );
	if((dmasize==0) || (dmasize > nextbuf->size))
		{
			pexor_dbg(KERN_NOTICE "#### pexor_next_dma resetting old dma size %x to %lx\n",dmasize,nextbuf->size);
			dmasize=nextbuf->size;
		}
	/*if(priv->pexor.ram_dma_cursor+dmasize > priv->pexor.ram_dma_base + PEXOR_RAMSIZE)
		{
			pexor_dbg(KERN_NOTICE "#### pexor_next_dma resetting old dma size %x...\n",dmasize);
			dmasize=priv->pexor.ram_dma_base + PEXOR_RAMSIZE - priv->pexor.ram_dma_cursor;
		}*/

	/* check if size is multiple of burstsize and correct:*/
	rest=dmasize % PEXOR_BURST;
	if(rest)
		{
			dmasize= dmasize + PEXOR_BURST - rest;
			if(dmasize > nextbuf->size) dmasize -= PEXOR_BURST;  /*avoid exceeding buf limits */

			pexor_dbg(KERN_NOTICE "#### pexor_next_dma correcting dmasize %x for rest:%x, burst:%x\n", dmasize, rest, PEXOR_BURST);

			/*			if(dmasize==nextbuf->size)
			{
				pexor_dbg(KERN_NOTICE "#### pexor_next_dma substracting dmasize rest:%x\n",rest);
				dmasize-=rest;
			}
			else
			{
				dmasize= dmasize + PEXOR_BURST - rest;  if not at buffer end, try to increase to next burst edge
				if(dmasize > nextbuf->size) dmasize -= PEXOR_BURST;  avoid exceeding buf limits anyway
				pexor_dbg(KERN_NOTICE "#### pexor_next_dma correcting dmasize %x for rest:%x, burst:%x\n", dmasize, rest, PEXOR_BURST);
			}*/
		}
	pexor_dbg(KERN_NOTICE "#### pexor_next_dma will initiate dma from %p to %p, len=%x, burstsize=%x...\n",
			(void*) priv->pexor.ram_dma_cursor, (void*) nextbuf->dma_addr,  dmasize, PEXOR_BURST);


	/* DEBUG TEST/
	return 0; */
	//spin_lock(&(priv->dma_lock));
	pexor_dma_lock((&(priv->dma_lock)));
	iowrite32(priv->pexor.ram_dma_cursor, priv->pexor.dma_source);
	mb();
	iowrite32((u32) nextbuf->dma_addr, priv->pexor.dma_dest);
	mb();
	iowrite32(PEXOR_BURST, priv->pexor.dma_burstsize);
	mb();
	iowrite32(dmasize, priv->pexor.dma_len);
	mb();
	iowrite32(enable, priv->pexor.dma_control_stat);
	mb();
	pexor_dbg(KERN_NOTICE "#### pexor_next_dma started dma \n");


#ifdef DMA_BOARD_IR
	/* the dma complete is handled by ir raised from pexor board*/
	return 0;
#endif


#ifdef DMA_WAITPOLLING
	/* the polling of dma complete is done in the ioctl wait function*/
	return 0;
#endif

/* emulate here the completion interrupt when dma is done:*/
	if((rev=pexor_poll_dma_complete(priv))!=0)
		return rev;



#ifndef	DMA_EMULATE_IR
	/* schedule tasklet*/
	atomic_inc(&(priv->irq_count));
	pexor_dbg(KERN_NOTICE "pexor_next_dma schedules tasklet... \n");
	tasklet_schedule(&priv->irq_bottomhalf);
	return 0;

#else
	/* raise a user interrupt to invoke our handlers manually:*/
	pexor_msg(KERN_NOTICE "#### pexor_next_dma raising user interrupt... \n");
	enable=PEXOR_IRQ_USER_BIT;
	mb();
	ndelay(20);
	iowrite32(enable, priv->pexor.irq_control);
	mb();
	ndelay(20);
	return 0;
#endif
}


int pexor_poll_dma_complete(struct pexor_privdata* priv)
{
	int loops=0;
	u32 enable=PEXOR_DMA_ENABLED_BIT;
	while(1)
	{
		enable=ioread32(priv->pexor.dma_control_stat);
		mb();
		if((enable & PEXOR_DMA_ENABLED_BIT) == 0) break;
		/* poll until the dma bit is cleared => dma complete*/

		//pexor_dbg(KERN_NOTICE "#### pexor_poll_dma_complete wait for dma completion #%d\n",loops);
		if(loops++ > PEXOR_DMA_MAXPOLLS)
			{
				pexor_msg(KERN_ERR "pexor_poll_dma_complete: polling longer than %d cycles (delay %d ns) for dma complete!!!\n",PEXOR_DMA_MAXPOLLS, PEXOR_DMA_POLLDELAY );
				pexor_dma_unlock((&(priv->dma_lock)));
				//spin_unlock(&(priv->dma_lock));
				return -EFAULT;
			}
		if(PEXOR_DMA_POLLDELAY) ndelay(PEXOR_DMA_POLLDELAY);
		if(PEXOR_DMA_POLL_SCHEDULE) schedule();
	};
	 pexor_dma_unlock((&(priv->dma_lock)));
	//spin_unlock(&(priv->dma_lock));
	return 0;
}


int pexor_wait_dma_buffer(struct pexor_privdata* priv, struct pexor_dmabuf* result)
{
	int rev=0 ,timeoutcount =0;
    unsigned long wjifs=0;
    struct pexor_dmabuf* dmabuf;
	#ifdef DMA_WAITPOLLING
		/* in case of polling mode, there is no isr raised from board. we will poll
		 * here on the dma complete and call ir bottom half directly which moves the
		 * filled buffer into the receive queue*/
		if((rev=pexor_poll_dma_complete(priv))!=0)
				return rev;
		atomic_inc(&(priv->irq_count)); /* bottom half checks ir count, we emulate this*/
		/*pexor_dbg(KERN_NOTICE "pexor_ioctl_waitreceive calls tasklet... \n"); */
		pexor_irq_tasklet((unsigned long) (priv));
		/* Note that wait_event_interruptible_timeout below will always expire immediately here,
		 * since condition is true*/

	#endif



		/* NOTE: we first have to check our counters, then verify that queue is not empty!*/

		/**
				* wait_event_interruptible_timeout - sleep until a condition gets true or a timeout elapses
				* The process is put to sleep (TASK_INTERRUPTIBLE) until the
				* @condition evaluates to true or a signal is received.
				* The @condition is checked each time the waitqueue @wq is woken up.
				*
				* wake_up() has to be called after changing any variable that could
				* change the result of the wait condition.
				*
				* The function returns 0 if the @timeout elapsed, -ERESTARTSYS if it
				* was interrupted by a signal, and the remaining jiffies otherwise
				* if the condition evaluated to true before the timeout elapsed.
				*/
		while((wjifs=wait_event_interruptible_timeout( priv->irq_dma_queue, atomic_read( &(priv->dma_outstanding) ) > 0, PEXOR_WAIT_TIMEOUT )) == 0 )
			{
				pexor_msg(KERN_NOTICE "** pexor_wait_dma_buffer TIMEOUT %d jiffies expired on wait_event_interruptible_timeout... \n",PEXOR_WAIT_TIMEOUT);
				if(timeoutcount++ > PEXOR_WAIT_MAXTIMEOUTS)
				{
					pexor_msg(KERN_NOTICE "** pexor_wait_dma_buffer reached maximum number of timeouts %d for %d jiffies wait time. abort wait\n",PEXOR_WAIT_MAXTIMEOUTS, PEXOR_WAIT_TIMEOUT);
					//spin_unlock(&(priv->dma_lock));
					return -EFAULT;
				}
			}
		pexor_dbg(KERN_NOTICE "** pexor_wait_dma_buffer after wait_event_interruptible_timeout with TIMEOUT %d, waitjiffies=%ld, outstanding=%d \n",PEXOR_WAIT_TIMEOUT, wjifs, atomic_read( &(priv->dma_outstanding)));
		if(wjifs==-ERESTARTSYS)
			{
				pexor_msg(KERN_NOTICE "** pexor_wait_dma_buffer after wait_event_interruptible_timeout woken by signal. abort wait\n");
				//spin_unlock(&(priv->dma_lock));
				return -EFAULT;
			}

		atomic_dec(&(priv->dma_outstanding));

		/* Take next buffer out of receive queue */
		spin_lock( &(priv->buffers_lock) );
		/* need to check here for empty list, since list_first_entry will crash otherwise!*/
		if(list_empty(&(priv->received_buffers)))
				{
					/* this may happen if user calls waitreceive without a DMA been activated, or at flow DMA suspended*/
					spin_unlock( &(priv->buffers_lock) );
					pexor_msg(KERN_NOTICE "** pexor_wait_dma_buffer: NEVER COME HERE receive queue is empty after wait\n");
					return -EFAULT;
				}

		dmabuf=list_first_entry(&(priv->received_buffers), struct pexor_dmabuf, queue_list);
	    list_move_tail(&(dmabuf->queue_list) , &(priv->used_buffers));
		spin_unlock( &(priv->buffers_lock) );
		pci_dma_sync_single_for_cpu( priv->pdev, dmabuf->dma_addr, dmabuf->size, PCI_DMA_FROMDEVICE );
		*result=*dmabuf;
		//spin_unlock(&(priv->dma_lock));
	return 0;
}

#ifdef PEXOR_SYSFS_ENABLE
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,18)
ssize_t pexor_sysfs_freebuffers_show(struct device *dev, struct device_attribute *attr, char *buf)
{
  int bufcount=0;
  struct pexor_privdata *privdata;
  struct list_head* cursor;
  privdata= (struct pexor_privdata*) dev_get_drvdata(dev);
  spin_lock( &(privdata->buffers_lock) );
  list_for_each(cursor, &(privdata->free_buffers))
    {
      bufcount++;
    }
  spin_unlock( &(privdata->buffers_lock) );
  return snprintf(buf, PAGE_SIZE, "%d\n", bufcount);
}

ssize_t pexor_sysfs_usedbuffers_show(struct device *dev, struct device_attribute *attr, char *buf)
{
  int bufcount=0;
  struct pexor_privdata *privdata;
  struct list_head* cursor;
  privdata= (struct pexor_privdata*) dev_get_drvdata(dev);
  spin_lock( &(privdata->buffers_lock) );
  list_for_each(cursor, &(privdata->used_buffers))
    {
      bufcount++;
    }
  spin_unlock( &(privdata->buffers_lock) );
  return snprintf(buf, PAGE_SIZE, "%d\n", bufcount);
}

ssize_t pexor_sysfs_rcvbuffers_show(struct device *dev, struct device_attribute *attr, char *buf)
{
  int bufcount=0;
  struct pexor_privdata *privdata;
  struct list_head* cursor;
  privdata= (struct pexor_privdata*) dev_get_drvdata(dev);
  spin_lock( &(privdata->buffers_lock) );
  list_for_each(cursor, &(privdata->received_buffers))
    {
      bufcount++;
    }
  spin_unlock( &(privdata->buffers_lock) );
  return snprintf(buf, PAGE_SIZE, "%d\n", bufcount);
}

ssize_t pexor_sysfs_codeversion_show(struct device *dev, struct device_attribute *attr, char *buf)
{
  char vstring[1024];
  ssize_t curs=0;
  struct  dev_pexor* pg;
  struct pexor_privdata *privdata;
  privdata= (struct pexor_privdata*) dev_get_drvdata(dev);
  curs=snprintf(vstring, 1024, "*** This is PEXOR driver version %s build on %s at %s \n\t", PEXORVERSION, __DATE__, __TIME__);
#ifdef PEXOR_WITH_SFP
  pg=&(privdata->pexor);
  pexor_show_version(&(pg->sfp),vstring+curs);
#endif
  return snprintf(buf, PAGE_SIZE, "%s\n", vstring);
}


ssize_t pexor_sysfs_dmaregs_show(struct device *dev, struct device_attribute *attr, char *buf)
{
  ssize_t curs=0;
  struct  dev_pexor* pg;
  struct pexor_privdata *privdata;
  privdata= (struct pexor_privdata*) dev_get_drvdata(dev);
  pg=&(privdata->pexor);
  curs+=snprintf(buf+curs, PAGE_SIZE-curs, "*** PEXOR dma/irq register dump:\n");
  curs+=snprintf(buf+curs, PAGE_SIZE-curs, "\t dma control/status: 0x%x\n", readl(pg->dma_control_stat));
  curs+=snprintf(buf+curs, PAGE_SIZE-curs, "\t irq/trixor stat: 0x%x\n", readl(pg->irq_status));
  curs+=snprintf(buf+curs, PAGE_SIZE-curs, "\t irq/trixor ctrl: 0x%x\n", readl(pg->irq_control));
#ifdef PEXOR_WITH_TRIXOR
  curs+=snprintf(buf+curs, PAGE_SIZE-curs, "\t trixor fcti: 0x%x\n", readl(pg->trix_fcti));
  curs+=snprintf(buf+curs, PAGE_SIZE-curs, "\t trixor cvti: 0x%x\n", readl(pg->trix_cvti));
#endif
  curs+=snprintf(buf+curs, PAGE_SIZE-curs, "\t dma source      address: 0x%x\n", readl(pg->dma_source));
  curs+=snprintf(buf+curs, PAGE_SIZE-curs, "\t dma destination address: 0x%x\n", readl(pg->dma_dest));
  curs+=snprintf(buf+curs, PAGE_SIZE-curs, "\t dma length:              0x%x\n", readl(pg->dma_len));
  curs+=snprintf(buf+curs, PAGE_SIZE-curs, "\t dma burst size:          0x%x\n", readl(pg->dma_burstsize));
  curs+=snprintf(buf+curs, PAGE_SIZE-curs, "\t RAM start:               0x%x\n", readl(pg->ram_start));
  curs+=snprintf(buf+curs, PAGE_SIZE-curs, "\t RAM end:                 0x%x\n", readl(pg->ram_end));
  return curs;
}
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

#ifdef PEXOR_DEBUGPRINT
static unsigned char get_pci_revision(struct pci_dev *dev)
{
	u8 revision;
	pci_read_config_byte(dev, PCI_REVISION_ID, &revision);
	return revision;
}

#endif


void test_pci(struct pci_dev *dev)
{
    int bar=0;
    u32 originalvalue=0;
    u32 base=0;
    u16 comstat=0;
    u8 typ=0;
    pexor_dbg(KERN_NOTICE "\n test_pci found PCI revision number %x \n",get_pci_revision(dev));


	/*********** test the address regions*/
	for(bar=0; bar<6; ++bar){
	pexor_dbg(KERN_NOTICE "Resource %d start=%x\n",bar, (unsigned) pci_resource_start( dev,bar ));
	pexor_dbg(KERN_NOTICE "Resource %d end=%x\n",bar,(unsigned) pci_resource_end( dev,bar ));
	pexor_dbg(KERN_NOTICE "Resource %d len=%x\n",bar,(unsigned) pci_resource_len( dev,bar ));
	pexor_dbg(KERN_NOTICE "Resource %d flags=%x\n",bar,(unsigned) pci_resource_flags( dev,bar ));
	if( (pci_resource_flags(dev,bar) & IORESOURCE_IO) ) {
        // Ressource im IO-Adressraum
	   pexor_dbg(KERN_NOTICE " - resource is IO\n");
	}
	if( (pci_resource_flags(dev,bar) & IORESOURCE_MEM) ) {
	    pexor_dbg(KERN_NOTICE " - resource is MEM\n");
	}
	if( (pci_resource_flags(dev,bar) & PCI_BASE_ADDRESS_SPACE_IO) ) {
	    pexor_dbg(KERN_NOTICE " - resource is PCI IO\n");
	}
	if( (pci_resource_flags(dev,bar) & PCI_BASE_ADDRESS_SPACE_MEMORY) ) {
	    pexor_dbg(KERN_NOTICE " - resource is PCI MEM\n");
	}
	if( (pci_resource_flags(dev,bar) & PCI_BASE_ADDRESS_MEM_PREFETCH) ) {
	    pexor_dbg(KERN_NOTICE " - resource prefetch bit is set \n");
	}
	if( (pci_resource_flags(dev,bar) & PCI_BASE_ADDRESS_MEM_TYPE_64) ) {
	    pexor_dbg(KERN_NOTICE " - resource is 64bit address \n");
	}
	if( (pci_resource_flags(dev,bar) & PCI_BASE_ADDRESS_MEM_TYPE_32) ) {
	    pexor_dbg(KERN_NOTICE " - resource is 32bit address \n");
	}
	if( (pci_resource_flags(dev,bar) & IORESOURCE_PREFETCH) ) {
	    pexor_dbg(KERN_NOTICE " - resource is prefetchable \n");
	}
	if( (pci_resource_flags(dev,bar) & PCI_BASE_ADDRESS_MEM_PREFETCH) ) {
	    pexor_dbg(KERN_NOTICE " - resource is PCI mem prefetchable \n");
	}
	if( (pci_resource_flags(dev,bar) & PCI_BASE_ADDRESS_MEM_TYPE_1M) ) {
	    pexor_dbg(KERN_NOTICE " - resource is PCI memtype below 1M \n");
	}



	}
	pci_read_config_dword( dev, PCI_BASE_ADDRESS_0, &originalvalue );
	pci_write_config_dword( dev, PCI_BASE_ADDRESS_0, 0xffffffff );
	pci_read_config_dword( dev, PCI_BASE_ADDRESS_0, &base );
	pci_write_config_dword( dev, PCI_BASE_ADDRESS_0, originalvalue );
	pexor_dbg("size of base address 0: %i\n", ~base+1 );
	pci_read_config_dword( dev, PCI_BASE_ADDRESS_1, &originalvalue );
	pci_write_config_dword( dev, PCI_BASE_ADDRESS_1, 0xffffffff );
	pci_read_config_dword( dev, PCI_BASE_ADDRESS_1, &base );
	pci_write_config_dword( dev, PCI_BASE_ADDRESS_1, originalvalue );
	pexor_dbg("size of base address 1: %i\n", ~base+1 );
	pci_read_config_dword( dev, PCI_BASE_ADDRESS_2, &originalvalue );
	pci_write_config_dword( dev, PCI_BASE_ADDRESS_2, 0xffffffff );
	pci_read_config_dword( dev, PCI_BASE_ADDRESS_2, &base );
	pci_write_config_dword( dev, PCI_BASE_ADDRESS_2, originalvalue );
	pexor_dbg("size of base address 2: %i\n", ~base+1 );
	pci_read_config_dword( dev, PCI_BASE_ADDRESS_3, &originalvalue );
	pci_write_config_dword( dev, PCI_BASE_ADDRESS_3, 0xffffffff );
	pci_read_config_dword( dev, PCI_BASE_ADDRESS_3, &base );
	pci_write_config_dword( dev, PCI_BASE_ADDRESS_3, originalvalue );
	pexor_dbg("size of base address 3: %i\n", ~base+1 );

	/***** here tests of configuration/status register:******/
	pci_read_config_word(dev, PCI_COMMAND , &comstat);
	pexor_dbg("\n****  Command register is: %d\n", comstat );
	pci_read_config_word(dev, PCI_STATUS , &comstat);
	pexor_dbg("\n****  Status register is: %d\n", comstat );
	pci_read_config_byte(dev, PCI_HEADER_TYPE , &typ);
	pexor_dbg("\n****  Header type is: %d\n", typ );
}









void cleanup_device(struct pexor_privdata* priv)
{
int j=0;
struct pci_dev* pcidev;
if(!priv) return;



#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,18)
/* sysfs device cleanup */
if (priv->class_dev)
  {
    #ifdef PEXOR_SYSFS_ENABLE
    device_remove_file(priv->class_dev, &dev_attr_sfpregs);
    device_remove_file(priv->class_dev, &dev_attr_dmaregs);
    device_remove_file(priv->class_dev, &dev_attr_codeversion);
    device_remove_file(priv->class_dev, &dev_attr_rcvbufs);
    device_remove_file(priv->class_dev, &dev_attr_usedbufs);
    device_remove_file(priv->class_dev, &dev_attr_freebufs);
    #endif
    device_destroy(pexor_class, priv->devno);
    priv->class_dev=0;
  }

#endif

/* character device cleanup*/
if(priv->cdev.owner)
	cdev_del(&priv->cdev);
if(priv->devid)
	atomic_dec(&pexor_numdevs);

pcidev = priv->pdev;
if(!pcidev) return;

/* may put disabling device irqs here?*/
#ifdef PEXOR_ENABLE_IRQ
free_irq( pcidev->irq, priv );
#endif

#ifdef IRQ_ENABLE_MSI
pci_disable_msi(pcidev);
#endif

cleanup_buffers(priv);

for (j = 0; j < 6; ++j)
    {
      if (priv->bases[j] == 0)
        continue;
      if ((pci_resource_flags(pcidev, j) & IORESOURCE_IO))
        {
          pexor_dbg(KERN_NOTICE " releasing IO region at:%lx -len:%lx \n",priv->bases[j],priv->reglen[j]);
          release_region(priv->bases[j], priv->reglen[j]);
        }
      else
        {
          if (priv->iomem[j] != 0)
            {
              pexor_dbg(KERN_NOTICE " unmapping virtual MEM region at:%lx -len:%lx \n",(unsigned long) priv->iomem[j],priv->reglen[j]);
              iounmap(priv->iomem[j]);
            }
          pexor_dbg(KERN_NOTICE " releasing MEM region at:%lx -len:%lx \n",priv->bases[j],priv->reglen[j]);
          release_mem_region(priv->bases[j], priv->reglen[j]);
        }
      priv->bases[j] = 0;
      priv->reglen[j] = 0;
    }
kfree(priv);
pci_disable_device(pcidev);
}

static int probe(struct pci_dev *dev, const struct pci_device_id *id)
{
  int err = 0, ix = 0, irnumbercount = 0;
#ifdef PEXOR_ENABLE_IRQ
   unsigned char irpin = 0, irline = 0;
   int irtype=0;
#endif
   struct pexor_privdata *privdata;
  pexor_msg(KERN_NOTICE "PEXOR pci driver starts probe...\n");
  if ((err = pci_enable_device(dev)) != 0)
    {
      pexor_msg(KERN_ERR "PEXOR pci driver probe: Error %d enabling PCI device! \n",err);
      return -ENODEV;
    }
  pexor_dbg(KERN_NOTICE "PEXOR Device is enabled.\n");

  /* Set Memory-Write-Invalidate support */
  if (!pci_set_mwi(dev))
    {
      pexor_dbg(KERN_NOTICE "MWI enabled.\n");
    }
  else
    {
      pexor_dbg(KERN_NOTICE "MWI not supported.\n");
    }
  pci_set_master(dev); /* NNOTE: DMA worked without, but maybe depends on bios...*/

/* Do we need setting            DMA mask? this part stolen from web:*/

  /*if (!pci_set_dma_mask(dev, DMA_64BIT_MASK) && !pci_set_consistent_dma_mask(
      dev, DMA_64BIT_MASK))
    {
      pexor_dbg(KERN_NOTICE "Set 64 bit DMA mask.\n");
       pci_using_dac = 1;
    }
  else
    {
      err = pci_set_dma_mask(dev, DMA_32BIT_MASK);
      if (err)
        {
          err = pci_set_consistent_dma_mask(dev, DMA_32BIT_MASK);
          if (err)
            {
              pexor_msg( KERN_ERR "No usable DMA , aborting\n");
              return -EIO;
            }pexor_dbg(KERN_NOTICE "Set 32 bit DMA mask.\n");
        }
       pci_using_dac = 0;
    }*/

 /* end stolen*/

  test_pci(dev);

  /* Allocate and initialize the private data for this device */
  privdata = kmalloc(sizeof(struct pexor_privdata), GFP_KERNEL);
  if (privdata == NULL)
    {
      cleanup_device(privdata);
      return -ENOMEM;
    }
  memset(privdata, 0, sizeof(struct pexor_privdata));
  pci_set_drvdata(dev, privdata);
  privdata->pdev = dev;
  privdata->magic = MY_DEVICE_ID; /* for isr test TODO: what if multiple pexors share same irq?*/

  atomic_set(&(privdata->state), PEXOR_STATE_STOPPED);

  for (ix = 0; ix < 6; ++ix)
    {
      privdata->bases[ix] = pci_resource_start(dev, ix);
      privdata->reglen[ix] = pci_resource_len(dev, ix);
      if (privdata->bases[ix] == 0)
        continue;
      if ((pci_resource_flags(dev, ix) & IORESOURCE_IO))
        {

          pexor_dbg(KERN_NOTICE " - Requesting io ports for bar %d\n",ix);
          if (request_region(privdata->bases[ix], privdata->reglen[ix],
              dev->dev.kobj.name) == NULL)
            {
              pexor_dbg(KERN_ERR "I/O address conflict at bar %d for device \"%s\"\n",
                  ix, dev->dev.kobj.name);
              cleanup_device(privdata);
              return -EIO;
            }pexor_dbg("requested ioport at %lx with length %lx\n", privdata->bases[ix], privdata->reglen[ix]);
        }
      else if ((pci_resource_flags(dev, ix) & IORESOURCE_MEM))
        {
          pexor_dbg(KERN_NOTICE " - Requesting memory region for bar %d\n",ix);
          if (request_mem_region(privdata->bases[ix], privdata->reglen[ix],
              dev->dev.kobj.name) == NULL)
            {
              pexor_dbg(KERN_ERR "Memory address conflict at bar %d for device \"%s\"\n",
                  ix, dev->dev.kobj.name);
              cleanup_device(privdata);
              return -EIO;
            }pexor_dbg("requested memory at %lx with length %lx\n", privdata->bases[ix], privdata->reglen[ix]);
          privdata->iomem[ix] = ioremap_nocache(privdata->bases[ix],
              privdata->reglen[ix]);
          if (privdata->iomem[ix] == NULL)
            {
              pexor_dbg(KERN_ERR "Could not remap memory  at bar %d for device \"%s\"\n",
                  ix, dev->dev.kobj.name);
              cleanup_device(privdata);
              return -EIO;
            }pexor_dbg("remapped memory to %lx with length %lx\n", (unsigned long) privdata->iomem[ix], privdata->reglen[ix]);
        }
    } //for
  set_pexor(&(privdata->pexor), privdata->iomem[0], privdata->bases[0]);

  print_pexor(&(privdata->pexor));

#ifdef PEXOR_ENABLE_IRQ
  /* reset pexor ir registers if still active from previous crash...*/

#ifdef PEXOR_WITH_TRIXOR
  irtype = TRIX_EV_IRQ_CLEAR | TRIX_IRQ_CLEAR;
  iowrite32(irtype, privdata->pexor.irq_status);
#else
    iowrite32(0, privdata->pexor.irq_control);
#endif
  mb();
  ndelay(20);

#endif

  init_MUTEX(&(privdata->ramsem));
  /* TODO may use rw semaphore instead? init_rwsem(struct rw_semaphore *sem); */

  spin_lock_init(&(privdata->buffers_lock));
  INIT_LIST_HEAD(&(privdata->free_buffers));
  INIT_LIST_HEAD(&(privdata->received_buffers));
  INIT_LIST_HEAD(&(privdata->used_buffers));
  /* the interrupt related stuff:*/
  spin_lock_init(&(privdata->irq_lock));
  atomic_set(&(privdata->irq_count), 0);
  init_waitqueue_head(&(privdata->irq_dma_queue));
  atomic_set(&(privdata->dma_outstanding), 0);
  init_waitqueue_head(&(privdata->irq_trig_queue));
  atomic_set(&(privdata->trig_outstanding), 0);
  tasklet_init(&(privdata->irq_bottomhalf), pexor_irq_tasklet,
      (unsigned long) privdata);
  spin_lock_init(&(privdata->dma_lock));    

  /* pexor_msg(KERN_NOTICE "Initialized ircount to %d.\n",atomic_read( &(privdata->dma_outstanding)));
   */

#ifdef IRQ_ENABLE_MSI
  /* here try to activate MSI ?*/
  if ((err=pci_enable_msi(dev)) == 0 )
    {
      pexor_dbg(KERN_NOTICE "MSI enabled.\n");
    }
  else
    {
      pexor_dbg(KERN_NOTICE "Failed activating MSI with error %d\n",err);
    }

#endif

#ifdef PEXOR_ENABLE_IRQ
  /* debug: do we have valid ir pins/lines here?*/
  if ((err = pci_read_config_byte(dev, PCI_INTERRUPT_PIN, &irpin)) != 0)
    {
      pexor_msg(KERN_ERR "PEXOR pci driver probe: Error %d getting the PCI interrupt pin \n",err);
    }
  if ((err = pci_read_config_byte(dev, PCI_INTERRUPT_LINE, &irline)) != 0)
    {
      pexor_msg(KERN_ERR "PEXOR pci driver probe: Error %d getting the PCI interrupt line.\n",err);
    }

  snprintf(privdata->irqname, 64, PEXORNAMEFMT,atomic_read(&pexor_numdevs));
  /*  if(request_irq(dev->irq,  pexor_isr , IRQF_DISABLED | IRQF_SHARED, privdata->irqname, privdata))
   */

  /* test: assign irq from our preset ir line*/

  /*  	dev->irq=irline;


   if(request_irq(dev->irq,  pexor_isr , 0 , privdata->irqname, privdata))
   */

#ifdef PEXOR_SHARED_IRQ
  if(request_irq(dev->irq, pexor_isr , IRQF_SHARED, privdata->irqname, privdata))
    {
      pexor_msg( KERN_ERR "PEXOR pci_drv: IRQ %d not free.\n", dev->irq );
      irnumbercount=1; /* suppress warnings from unused variable here*/
      cleanup_device(privdata);
      return -EIO;
    }

#else

  dev->irq=irline;
  while(request_irq(dev->irq, pexor_isr , 0 , privdata->irqname, privdata))
    {
      pexor_msg( KERN_ERR "PEXOR pci_drv: IRQ %d not free. try next...\n", dev->irq++ );
      if(irnumbercount++ > 100)
        {
          pexor_msg( KERN_ERR "PEXOR pci_drv: tried to get ir more than %d times, aborting\n", irnumbercount )
          cleanup_device(privdata);
          return -EIO;
        }
    }

#endif

  pexor_msg(KERN_NOTICE " assigned IRQ %d for name %s, pin:%d, line:%d \n",dev->irq, privdata->irqname,irpin,irline);

#endif

  ////////////////// here chardev registering
  privdata->devid = atomic_inc_return(&pexor_numdevs) - 1;
  if (privdata->devid >= PEXOR_MAXDEVS)
    {
      pexor_msg(KERN_ERR "Maximum number of devices reached! Increase MAXDEVICES.\n");
      cleanup_device(privdata);
      return -ENOMSG;
    }

  privdata->devno
      = MKDEV(MAJOR(pexor_devt), MINOR(pexor_devt) + privdata->devid);

  /* Register character device */
  cdev_init(&(privdata->cdev), &pexor_fops);
  privdata->cdev.owner = THIS_MODULE;
  privdata->cdev.ops = &pexor_fops;
  err = cdev_add(&privdata->cdev, privdata->devno, 1);
  if (err)
    {
      pexor_msg( "Couldn't add character device.\n" );
      cleanup_device(privdata);
      return err;
    }

  /* TODO: export special things to class in sysfs ?*/
  

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,18)
  if (!IS_ERR(pexor_class))
    {
      /* driver init had successfully created class, now we create device:*/
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,27)
     privdata->class_dev = device_create(pexor_class, NULL, privdata->devno,
         privdata, PEXORNAMEFMT, MINOR(pexor_devt) + privdata->devid);
#else
    privdata->class_dev = device_create(pexor_class, NULL, privdata->devno,
              PEXORNAMEFMT, MINOR(pexor_devt) + privdata->devid);
#endif
      dev_set_drvdata(privdata->class_dev, privdata);
      pexor_msg(KERN_NOTICE "Added PEXOR device: ");
      pexor_msg(KERN_NOTICE PEXORNAMEFMT, MINOR(pexor_devt) + privdata->devid);
      
#ifdef PEXOR_SYSFS_ENABLE

      if(device_create_file(privdata->class_dev, &dev_attr_freebufs) != 0)
        {
          pexor_msg(KERN_ERR "Could not add device file node for free buffers.\n");
        }
      if(device_create_file(privdata->class_dev, &dev_attr_usedbufs) != 0)
        {
          pexor_msg(KERN_ERR "Could not add device file node for used buffers.\n");
        }
      if(device_create_file(privdata->class_dev, &dev_attr_rcvbufs) != 0)
         {
           pexor_msg(KERN_ERR "Could not add device file node for receive buffers.\n");
         }

      if(device_create_file(privdata->class_dev, &dev_attr_codeversion) != 0)
         {
           pexor_msg(KERN_ERR "Could not add device file node for code version.\n");
         }

      if(device_create_file(privdata->class_dev, &dev_attr_dmaregs) != 0)
        {
          pexor_msg(KERN_ERR "Could not add device file node for dma registers.\n");
        }
      if(device_create_file(privdata->class_dev, &dev_attr_sfpregs) != 0)
         {
           pexor_msg(KERN_ERR "Could not add device file node for sfp registers.\n");
         }
         #endif
    }
  else
    {
      /* something was wrong at class creation, we skip sysfs device support here:*/
      pexor_msg(KERN_ERR "Could not add PEXOR device node to /dev !");
    }

#endif

pexor_msg(KERN_NOTICE "probe has finished.\n");
return 0;



}

static void remove(struct pci_dev *dev)
{
struct pexor_privdata* priv = (struct pexor_privdata*) pci_get_drvdata(dev);
cleanup_device(priv);

pexor_msg(KERN_NOTICE "PEXOR pci driver end remove.\n");
}


static struct pci_driver pci_driver = {
	.name = PEXORNAME,
	.id_table = ids,
	.probe = probe,
	.remove = remove,
};

static int __init pexor_init(void)
{

	int result;
	pexor_msg(KERN_NOTICE "pexor driver init...\n");

	pexor_devt  = MKDEV(my_major_nr, 0);

	/*
	 * Register your major, and accept a dynamic number.
	 */
	if (my_major_nr)
		result = register_chrdev_region(pexor_devt, PEXOR_MAXDEVS, PEXORNAME);
	else {
		result = alloc_chrdev_region(&pexor_devt, 0, PEXOR_MAXDEVS, PEXORNAME);
		my_major_nr = MAJOR(pexor_devt);
	}
	if (result < 0)
		return result;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,18)
        pexor_class = class_create(THIS_MODULE, PEXORNAME);
        if (IS_ERR(pexor_class))
          {
            pexor_msg(KERN_ALERT "Could not create class for sysfs support!\n");
          }

#endif

	if(pci_register_driver(&pci_driver) < 0 )
		{
		  pexor_msg(KERN_ALERT  "pci driver could not register!\n");
		  unregister_chrdev_region(pexor_devt, PEXOR_MAXDEVS);
		  return -EIO;
		}

    pexor_msg(KERN_NOTICE "\t\tdriver init with registration for major no %d done.\n",my_major_nr);
	    return 0;

    /* note: actual assignment will be done on probe time*/


}

static void __exit pexor_exit(void)
{
  pexor_msg(KERN_NOTICE "pexor driver exit...\n");
  

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,18)
      class_destroy(pexor_class);
#endif


    unregister_chrdev_region(pexor_devt, PEXOR_MAXDEVS);
    pci_unregister_driver(&pci_driver);
    pexor_msg(KERN_NOTICE "\t\tdriver exit done.\n");
}



MODULE_LICENSE("GPL");

module_init(pexor_init);
module_exit(pexor_exit);
