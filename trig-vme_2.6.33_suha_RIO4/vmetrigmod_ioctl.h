#ifndef __VMETRIGMOD_IOCTL_H__
#define __VMETRIGMOD_IOCTL_H__

#include <linux/ioctl.h>

union vmetrigmod_get_irqcount_arg {
	struct {
		int clear;
	} in;
	struct {
		int value;
	} out;
};

#define TRIGMOD_MAGIC 0xaa /* Magic frei waehlbar */

#define CMD_TRIGMOD_GET_IRQCOUNT         _IOWR(TRIGMOD_MAGIC,  0, union vmetrigmod_get_irqcount_arg)
#define CMD_TRIGMOD_VOIDCALL             _IO(TRIGMOD_MAGIC,  1)

#endif /* __VMETRIGMOD_IOCTL_H__ */
