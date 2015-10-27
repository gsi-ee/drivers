#ifndef MYSNULL_IOCTL_H_
#define MYSNULL_IOCTL_H_

/** test how we can do configuration via socket ioctls:*/


#include <linux/sockios.h>

#define SIOCSNULLSTATS            SIOCDEVPRIVATE+0        /* get some stats*/
#define SIOCSNULLREAD             SIOCDEVPRIVATE+1        /* read register*/
#define SIOCSNULLWRITE            SIOCDEVPRIVATE+2        /* write register*/
#define SIOCSNULLRESET            SIOCDEVPRIVATE+3        /* write register*/

//  try to pass this structure into kernel. if this works, we can do gosipcmd.
struct pex_bus_io {
    int sfp;        /**< sfp link id 0..3 (-1 for broadcast to all configured sfps)*/
    long slave;     /**< slave device id at the sfp (-1 ato broadcast to all slaves)*/
    unsigned long address;  /**< address on the "field bus" connected to the optical links*/
    unsigned long value;    /**< value for read/write at bus address. Contains result status after write*/
};



#endif
