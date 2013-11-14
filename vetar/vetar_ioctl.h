#ifndef __VETAR_IOCTL_H__
#define __VETAR_IOCTL_H__

#include <linux/ioctl.h>

/* JAM: probably we will never need ioctl for etherbone lib.*/



union vetar_get_irqcount_arg {
	struct {
		int clear;
	} in;
	struct {
		int value;
	} out;
};

#define VETAR_MAGIC 0xab /* Magic frei waehlbar */

#define CMD_VETAR_GET_IRQCOUNT         _IOWR(VETAR_MAGIC,  0, union vetar_get_irqcount_arg)
#define CMD_VETAR_VOIDCALL             _IO(VETAR_MAGIC,  1)

#endif /* __VETAR_IOCTL_H__ */
