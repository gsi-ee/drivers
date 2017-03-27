#ifndef NYXORSETUP_H
#define NYXORSETUP_H


#include "GosipGui.h"


/** number of nxyters on nyxor board. may not change so soon...*/
#define NYXOR_NUMNX 2

#define GOS_I2C_DWR  0x8010  // i2c data write reg.   addr
#define GOS_I2C_DRR1 0x8020  // i2c data read  reg. 1 addr
#define GOS_I2C_DWR2 0x8040  // i2c data read  reg. 2 addr
#define GOS_I2C_SR   0x8080  // i2c status     reg.   addr

#define I2C_CTRL_A   0x01
#define I2C_COTR_A   0x03
#define I2C_RECEIVE  0x84
#define I2C_STATUS   0x85

#define CHECK_DAT    0xa9000000


// write address modifiers for different nxyters on nyxor
#define I2C_ADDR_NX0 0x12
#define I2C_ADDR_NX1 0x22

// external DAC base address on i2c:
#define I2C_DAC_BASE_R 0xBC00000
#define I2C_DAC_BASE_W 0xB430000


// spi related definitions:
#define SPI_ENABLE_ADDR 0x11        // SPI enable register
#define SPI_BAUD_ADDR   0x12        // SPI baud rate register
#define SPI_TRANS_ADDR  0x15        // SPI transfer register
#define SPI_WRITE       0x00        // SPI write mode slave
#define SPI_READ        0x80        // SPI read mode slave

#define SPI_ADC_DC0PHASE    0x16  // pointer to ADC DC0 phase register
#define SPI_ADC_PATTERNBASE 0x19  // ADC transmit pattern registers base pointer (0x19-0x1C)

// nxyter receiver sub core registers:

#define NXREC_CTRL_W 0x21 // nxyter control register write
#define NXREC_CTRL_R 0xA1 // nxyter control register read
#define NXREC_PRETRIG_W 0x22 //nxyter pre trigger window write
#define NXREC_PRETRIG_R 0xA2 //nxyter pre trigger window read
#define NXREC_POSTTRIG_W 0x23 //nxyter post trigger window write
#define NXREC_POSTTRIG_R 0xA3 //nxyter post trigger window read
#define NXREC_DELAY1_W 0x24 //nxyter Delay Register 1 (Second Test Pulse Delay) write
#define NXREC_DELAY1_R 0xA4 //nxyter Delay Register 1 (Second Test Pulse Delay) read
#define NXREC_DELAY2_W 0x25 //nxyter  Delay Register 2 (Test Acquisition Trigger Delay) write
#define NXREC_DELAY2_R 0xA5 //nxyter  Delay Register 2 (Test Acquisition Trigger Delay) read

#define NXREC_TESTCODE_ADC_W 0x31 //nxyter   ADCs Test Code (NYXOR Self-Test Mode) write
#define NXREC_TESTCODE_ADC_R 0xB1 //nxyter   ADCs Test Code (NYXOR Self-Test Mode) read
#define NXREC_TESTCODE_1_W 0x32 //nxyter    Test Code 1 (NYXOR Self-Test Mode) write
#define NXREC_TESTCODE_1_R 0xB2 //nxyter    Test Code 1 (NYXOR Self-Test Mode) read
#define NXREC_TESTCODE_2_W 0x33 //nxyter    Test Code 1 (NYXOR Self-Test Mode) write
#define NXREC_TESTCODE_2_R 0xB3 //nxyter    Test Code 1 (NYXOR Self-Test Mode) read



// for the moment, this is a dummy to fulfill framework.
// nyxor setup structures are kept in different subwidgets for historical reasons
class NyxorSetup: public GosipSetup
{
public:
  NyxorSetup(): GosipSetup(){;}
};



#endif
