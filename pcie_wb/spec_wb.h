#ifndef SPEC_WB_DRIVER_H
#define SPEC_WB_DRIVER_H

#include "wishbone.h"

#define SPEC_WB "spec_wb"
#define SPEC_WB_VERSION	"0.1"

#define SPEC1_WB_VENDOR_ID	0x1a39
#define	SPEC1_WB_DEVICE_ID	0x0004
#define	SPEC1_WB_REVISION_ID	0x0001

#define SPEC4_WB_VENDOR_ID	0x10dc
#define	SPEC4_WB_DEVICE_ID	0x018d
#define	SPEC4_WB_REVISION_ID	0x0003

#define WB_BAR		0
#define WB_OFFSET	0x80000
#define WB_LOW		0x3fffc

/* One per BAR */
struct spec_wb_resource {
	unsigned long start;			/* start addr of BAR */
	unsigned long end;			/* end addr of BAR */
	unsigned long size;			/* size of BAR */
	void *addr;				/* remapped addr */
};

/* One per physical card */
struct spec_wb_dev {
	struct pci_dev* pci_dev;
	struct spec_wb_resource pci_res[3];
	int    pci_irq[4];
	int    msi;
	
	struct wishbone wb;
	unsigned int window_offset;
	unsigned int low_addr, width, shift;
};

#endif
