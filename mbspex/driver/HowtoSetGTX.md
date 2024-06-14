# How to change Kinpex gosip link speed
(JAM, 03-June-2024 - 04-June-2024)

Evaluation of Shizus Scripts gtx_set_*.sh in connection with AMD GTX and clocking documents:
 
   https://docs.amd.com/v/u/en-US/ug476_7Series_Transceivers
       
   https://docs.amd.com/v/u/en-US/ug472_7Series_Clocking
   
   https://docs.amd.com/v/u/en-US/xapp888_7Series_DynamicRecon
    
## PCI communication port
All read/write is done at PCIe address      0x210c8 (i.e. register offset 0xc8)
The data word written specifies a 16 bit(?) address in the transceiver registers, followed by the 16 bit value to change
Example:    
    
    ${DIRTOOLS}/pex_bar0_rw 0x210c8 0x805e1082

## Required gtx2 registers
### Common registers
   * First nibble of drp address indicates the mode (writing: 0xC; reading 0x4)
   * Following address is used from GTX2_COMMON Primitive   

  
         0x036
            QPLL_DMONITOR_SEL          DRP bits 15
            QPLL_FBDIV_MONITOR_EN      DRP bits 14
            QPLL_CP_MONITOR_EN         DRP bits 13
            QPLL_COARSE_FREQ_OVRD_EN   DRP bits 11
            QPLL_FBDIV                 DRP bits 9:0
    
    
The example setup writes value 0x20 ->    QPLL_FBDIV=32 => N=16 (see Table 2-16)      


### Channel registers
   * First nibble of drp address seems to indicate the sfp chain (writing: 0x8..., 0x9..., 0xa..., 0xb; reading 0x0,-,0x3)
   * Following 3 nibbles are relative DRP address registers  for GTX2_CHANNEL Primitive :   
      
      0x05e   
          SATA_CPLL_CFG               DRP bits 15:14, attribute bits 0:1 -   (VCO 750,1500,3000 MHz)
          CPLL_REFCLK_DIV    = M      DRP bits 12:8, attribute bits 4:0      reference clock divider 1...20
          CPLL_FBDIV_45      = N1     DRP bits  7, attribute bits 0          attribute value 4 or 5
          CPLL_FBDIV         = N2     DRP bits  6:0, attribute bits 6:00     division factor 1...20
             
           
      0x088
          DRP address TXOUT_DIV = D:  DRP bits 6:4,     attribute bits 2:0
          DRP address RXOUT_DIV = D:  DRP bits 2:0,     attribute bits 2:0
      


      
see p.47 "Channel PLL" and Table 2-8: CPLL Divider Settings:
             and Eq. 2-1, Eq.2-2:
             
             
      f_pllclkout    = f_pplclkin * CPLL_FBDIV_45 * CPLL_FBDIV / CPLL_REFCLK_DIV (Eq. 2-1)
      
      f_linerate_tx  = 2 * f_pllclkout / TXOUT_DIV
      
      f_linerate_rx  = 2 * f_pllclkout / RXOUT_DIV
      
            with f_pplclkin = 1.6 GHz ... 3.3 GHz
                  
see Table 3-25: TX PLL Output Divider Setting
                 Table 4-23: RX PLL Output Divider Setting

      0x0a9
         DRP address  RXCDR_CFG :     DRP bits 15:0, attribute bits 31:16
         (0xa8...0xac define 80 bit value space of RXCDR_CFG) ????
           
           see p. 199, "RX CDR" - RX clock data recovery      
           see "GTX CDR Recommended Settings.."  tables 4-17, 4-18, 4-19 p.204ff 

 
## Required mmcm registers
   * The mixed mode clock manager (MMCM) is accessed with first nibble 0xd (write) or 0x5 (read) of port address

   Used DRP addresses:     (see https://docs.amd.com/v/u/en-US/xapp888_7Series_DynamicRecon Tables 1-15, Fig. 1 and explanations)

### CLKOUT0 register 1 (0x08)
    (see Table 1 )
    
    * PHASE MUX [15:13]   Chooses an initial phase offset for the clock output, the resolution is equal to 1/8 VCO period. 
    * RESERVED [12]       Retain the previous value stored here.
    * HIGH TIME [11:6]    Sets the amount of time in VCO cycles that the clock output remains High.
    * LOW TIME [5:0]      Sets the amount of time in VCO cycles that the clock output remains Low.
                           
                          
### CLKOUT0 register 2 (0x09)   
    (see Table 2)

    *   RESERVED     [15]       Reserved.
    *   FRAC         [14:12]    Fractional divide counter setting for CLKOUT0. Equivalent to  additional divide of 1/8.
    *   FRAC_EN      [11]       Enable fractional divider circuitry for CLKOUT0.
    *   FRAC_WF_R    [10]       Adjusts CLKOUT0 rising edge for improved duty cycle accuracy  when using fractional counter.  
    *   MX           [9:8]      Must be set to 2'b00.
    *   EDGE         [7]        Chooses the edge that the High Time counter transitions on.
    *   NO COUNT     [6]        Bypasses the High and Low Time counters.
    *   DELAY TIME   [5:0]      Phase offset with a resolution equal to the VCO period. 
 
 
### CLKBUFOUT register 1 (0x14)
    (see Table 1)
    
    * PHASE MUX [15:13]   Chooses an initial phase offset for the clock output, the resolution is equal to 1/8 VCO period. 
    * RESERVED [12]       Retain the previous value stored here.
    * HIGH TIME [11:6]    Sets the amount of time in VCO cycles that the clock output remains High.
    * LOW TIME [5:0]      Sets the amount of time in VCO cycles that the clock output remains Low.

### CLKBUFOUT register 2 (0x15)
    (see Table 7)

    * RESERVED  [15]        Retain the previous value stored here.
    * FRAC      [14:12]     Fractional divide counter setting for CLKFBOUT. Equivalent to additional divide of 1/8.
    * FRAC_EN   [11]        Enable fractional divider circuitry for CLKFBOUT.
    * FRAC_WF_R [10]        Adjusts CLKFBOUT rising edge for improved duty cycle accuracy when using fractional counter.
    * MX        [9:8]   Must be set to 2'b00.
    * EDGE      [7]     Chooses the edge that the High Time counter transitions on.
    * NO COUNT  [6]     Bypasses the High and Low Time counters.
    * DELAY TIME [5:0]  Phase offset with a resolution equal to the VCO period.





###  DIVCLK register  (0x16)

    * RESERVED  [15:14]     Retain the previous value stored here.
    * EDGE      [13]        Chooses the edge that the High Time counter transitions on.
    * NO COUNT  [12]        Bypasses the High and Low Time counters.
    * HIGH TIME [11:6]      Sets the amount of time in VCO cycles that the clock output remains High.
    * LOW TIME  [5:0]       Sets the amount of time in VCO cycles that the clock output remains Low.


### Lock Register 1 (0x18)
    (see Table 9)

    * RESERVED          [15:10]  Retain the previous value stored here.
    * LKTABLE[29:20]    [9:0]    These bits are pulled from the lookup table provided in the reference design.
 

### Lock Register 2 (0x19)
    (see Table 10)

    * RESERVED          [15]    Retain the previous value stored here.
    * LKTABLE[34:30]    [14:10] These bits are pulled from the lookup table provided in the reference design.
    * LKTABLE[9:0]      [9:0]   These bits are pulled from the lookup table provided in the reference design.

  
### Lock Register 3 (0x1A)
  (see Table 11)

    * RESERVED          [15]    Retain the previous value stored here.
    * LKTABLE[39:35]    [14:10] These bits are pulled from the lookup table provided in the reference design.
    * LKTABLE[19:10]    [9:0]  These bits are pulled from the lookup table provided in the reference design.
 
### Filter Register 1 (0x4E)
    (see Table 12)

    * TABLE[9]      [15]        This bit is pulled from the lookup table provided in the reference design.
    * RESERVED      [14:13]     Retain the previous value stored here.
    * TABLE[8:7]    [12:11]     These bits are pulled from the lookup table provided in the reference design.
    * RESERVED      [10:9]      Retain the previous value stored here.
    * TABLE[6]      [8]         This bit is pulled from the lookup table provided in the reference design.
    * RESERVED      [7:0]       Retain the previous value stored here.


### Filter Register 2 (0x4F)


    * TABLE[5]      [15]        This bit is pulled from the lookup table provided in the reference design.
    * RESERVED      [14:13]     Retain the previous value stored here.
    * TABLE[4:3]    [12:11]     These bits are pulled from the lookup table provided in the reference design.
    * RESERVED      [10:9]      Retain the previous value stored here.
    * TABLE[2:1]    [8:7]       These bits are pulled from the lookup table provided in the reference design.
    * RESERVED      [6:5]       Retain the previous value stored here.
    * TABLE[0]      [4]         This bit is pulled from the lookup table provided in the reference design.
    * RESERVED      [3:0]       Retain the previous value stored here.




   
### Used values for known designs (register differences only!)   
   
      | address | 2 Gbs    | 2.5 Gb  | 3.125 Gb |  5 GB |
      | ------- | ------   | ------  | -------- | ----- |
      | 0x008   |   1187   |   1042  |  1104    | 1042  |
      | 0x009   |   0080   |   0080  |  0000    | 0080  |  
      | 0x014   |   134d   |   10c3  |  1083    | 10c3  |
      | 0x015   |   0000   |   0000  |  0080    | 0000  | 
      | 0x016   |   2083   |   0041  |  1041    | 0041  | 
      | 0x018   |   0177   |   03e8  |  03e8    | 03e8  | 
      | 0x019   |   7c01   |   4401  |  3801    | 4401  | 
      | 0x01a   |   ffe9   |   c7e9  |  bbe9    | c7e9  | 
      | 0x04e   |   0908   |   9808  |  9108    | 9808  | 
      | 0x04f   |   1000   |   8900  |  1900    | 8900  | 
   
   
   

## Example setups


### 2 gbps (125mhz?)
#### GTX registers

   * Register 0x05e -> 0x1002
      * => SATA_CPLL_CFG         = 0b00         = 2  =>     value 750 MHz
      * => M= CPLL_REFCLK_DIV    = 0b10000      = 16 =>     value 1 
      * => N1=CPLL_FBDIV_45      = 0b0          = 0  =>     value 4
      * => N2=CPLL_FBDIV         = 0b10         = 2  =>     value 4
   * Register 0x088 -> 0x0011
      * =>     TXOUT_DIV =    0x1 => value 2
      * =>    RXOUT_DIV = 0x1 => value 2        
   * Register 0x0a9 -> 0x1020
      (recommended settings for "8B/10B Encoded Data"  with RXOUT_DIV=2)!!!    
        
    So:
     f_pllclkout = f_pplclkin * 4 * 4 /1 = 16 * f_pplclkin =16 * 125 MHz = 2.0 Gb.
     f_linerate_tx  = 2* f_pllclkout/2 = 2.0 Gb
     f_linerate_rx  = 2* f_pllclkout/2 = 2.0 Gb
     
 Reference clock f_pplclkin is board feature at 125 MHz here?
 
 
 
 
#### MMCM registers

   * 0x008 -> 0x1187 (CLKOUT0-1)
      * => PHASE MUX                = 0
      * => RESERVED                 = 1
      * => HIGH TIME = 0b000110     = 6
      * => LOW TIME  = 0b111        = 7        
   * 0x009 => 0x0080  (CLKOUT0-2)
      * => EDGE = 1
      * => rest all 0
  
   
   * 0x014 => 0x134d  (CLKBUFOUT-1)
      * => RESERVED                 = 1
      * => HIGH TIME = 0b001101     = 13
      * => LOW TIME  = 0b001101     = 13
   * 0x015 => 0x0000  (CLKBUFOUT-2) 
      * => EDGE = 0
      * => rest all 0
   
   
   * 0x016 => 0x2083 (DIVCLK)
    * => EDGE = 1
    * => HIGH TIME = 0b10     = 2
    * => LOW TIME  = 0b11     = 3

   _Lock group:_ 
    
   * 0x018 => 0x0177 (Lock 1)
    * => LKTABLE[29:20] = 0b101110111
   
   * 0x019 => 0x7c01 (Lock 2)
    * => LKTABLE[34:30] = 0
    * => LKTABLE[9:0]   = 0b101110111
      
   * 0x01a => 0xffe9 (Lock 3)
    * => RESERVED=1
    * => LKTABLE[39:35] = 0b11111
    * => LKTABLE[19:10] = 0b111101001
   
   LKTABLE[]=0b111110000010111011111110100101110111 = 0xF82EFE977
   
   _This group cannot be calculated with an algorithm and is based on lookup tables created from device characterization._ 
   
   _Filter group:_
   
   * 0x04e => 0x0908 (Filter 1)
       =>  TABLE[8:7]   = 0b01
       =>  RESERVED     = 0b00
           TABLE[6]     = 0b1
           RESERVED     = 0b1000
           
   * 0x04f => 0x1000 (Filter 2) 
        TABLE[5] =   0b0
        TABLE[4:3] = 0b10
        TABLE[2:1] = 0b00
        TABLE[0] =   0b0
 
  _This group cannot be calculated with an algorithm and is based on lookup tables created from device characterization._ 

   
### 2.5gbps 125mhz
#### GTX registers
   
    
   * Register 0x05e -> 0x1082 
      * => SATA_CPLL_CFG     = 0b00         = 2  =>     value 750 MHz
      * => M= CPLL_REFCLK_DIV    = 0b10000     = 16 =>     value 1 
      * =>   N1=CPLL_FBDIV_45    = 0b1        = 1  =>        value 5
      * =>   N2=CPLL_FBDIV        = 0b10        = 2  =>     value 4
   
   * Register 0x088 -> 0x0011
      * =>    TXOUT_DIV =    0x1 => value 2
      * =>    RXOUT_DIV = 0x1 => value 2
   * Register 0x0a9 -> 0x1020
        (recommended settings for "8B/10B Encoded Data"  with RXOUT_DIV=2)!!!
 
    
        

    So:
     f_pllclkout = f_pplclkin * 5 * 4 /1 = 20 * f_pplclkin =20 * 125 MHz = 2.5 Gb.
     f_linerate_tx  = 2* f_pllclkout/2 = 2.5 Gb
     f_linerate_rx  = 2* f_pllclkout/2 = 2.5 Gb
     
? OK, but where is 125 MHz f_pplclkin set?-> fixed on board!?

#### MMCM registers
   * 0x008 -> 0x1042 (CLKOUT0-1)
      * => PHASE MUX                = 0
      * => RESERVED                 = 1
      * => HIGH TIME = 0b1          = 1
      *  => LOW TIME  = 0b10         = 2
   * 0x009 => 0x0080  (CLKOUT0-2)
      * => EDGE = 1
      * => rest all 0
   * 0x014 => 0x10c3  (CLKBUFOUT-1)
      * => RESERVED               = 1
      * => HIGH TIME = 0b0011     = 3
      * => LOW TIME  = 0b0011     = 3
   * 0x015 => 0x0000  (CLKBUFOUT-2) 
      *  => EDGE = 0
      *  => rest all 0
   
   
   * 0x016 => 0x0041 (DIVCLK)
      *  => EDGE = 0
      * => HIGH TIME = 0b1     = 1
      * => LOW TIME  = 0b1     = 1

   _Lock group:_ 
    
   * 0x018 => 0x03e8 (Lock 1)
      * => LKTABLE[29:20] = 0b1111101000
   
   * 0x019 => 0x4401 (Lock 2)
      * => LKTABLE[34:30] = 0b10001
      * => LKTABLE[9:0]   = 0b1
      
   * 0x01a => 0xc7e9 (Lock 3)++++
      * => RESERVED=1
      *  => LKTABLE[39:35] = 0b110001 
      * => LKTABLE[19:10] = 0b1111101001
     
   _This group cannot be calculated with an algorithm and is based on lookup tables created from device characterization._ 
   
   _Filter group:_
   
   * 0x04e => 0x0908 (Filter 1)
      *  =>  TABLE[8:7]   = 0b01
      *  =>  RESERVED     = 0b00
      *      TABLE[6]     = 0b1
       *     RESERVED     = 0b1000
           
   * 0x04f => 0x8900 (Filter 2) ++++
       *  TABLE[5] =   0b0
       *  TABLE[4:3] = 0b10
       *  TABLE[2:1] = 0b00
       *  TABLE[0] =   0b0
 
  _This group cannot be calculated with an algorithm and is based on lookup tables created from device characterization._ 





### 5 gbps (125mhz?)
#### GTX registers

   * Register 0x05e -> 0x1082
       *  => SATA_CPLL_CFG     = 0b00         = 2  =>     value 750 MHz
       *  => M= CPLL_REFCLK_DIV    = 0b10000     = 16 =>     value 1 
       *  => N1=CPLL_FBDIV_45    = 0b0        = 1  =>        value 5
       *  => N2=CPLL_FBDIV        = 0b10        = 2  =>     value 4
        
   * Register 0x088 -> 0x0000
       *  =>     TXOUT_DIV =    0x0 => value 1
       *  =>    RXOUT_DIV = 0x0 => value 1
        
   * Register 0x0a9 -> 0x1040
        (recommended settings for "8B/10B Encoded Data"  with RXOUT_DIV=1)!!!    
        
            So:
     f_pllclkout = f_pplclkin * 4 * 5 /1 = 20 * f_pplclkin =20 * 125 MHz = 2.5 Gb.
     f_linerate_tx  = 2* f_pllclkout/1 = 5.0 Gb
     f_linerate_rx  = 2* f_pllclkout/1 = 5.0 Gb
     
#### MMCM registers
(same setup as for 2.5 Gb!)
   
   * 0x008 -> 0x1042 (CLKOUT0-1)
      * => PHASE MUX                = 0
      * => RESERVED                 = 1
      * => HIGH TIME = 0b1          = 1
      * => LOW TIME  = 0b10         = 2
        
   * 0x009 => 0x0080  (CLKOUT0-2)
      * => EDGE = 1
      * => rest all 0
   
   * 0x014 => 0x10c3  (CLKBUFOUT-1)
      * => RESERVED               = 1
      * => HIGH TIME = 0b0011     = 3
      * => LOW TIME  = 0b0011     = 3
   
   * 0x015 => 0x0000  (CLKBUFOUT-2) 
      * => EDGE = 0
      *  => rest all 0
   
   
   * 0x016 => 0x0041 (DIVCLK)
      * => EDGE = 0
      * => HIGH TIME = 0b1     = 1
      * => LOW TIME  = 0b1     = 1

   _Lock group:_ 
    
   * 0x018 => 0x03e8 (Lock 1)
      *  => LKTABLE[29:20] = 0b1111101000
   
   * 0x019 => 0x4401 (Lock 2)
      * => LKTABLE[34:30] = 0b10001
      *  => LKTABLE[9:0]   = 0b1
      
   * 0x01a => 0xc7e9 (Lock 3)++++
      *  => RESERVED=1
      *  => LKTABLE[39:35] = 0b110001 
      *  => LKTABLE[19:10] = 0b1111101001
     
   _This group cannot be calculated with an algorithm and is based on lookup tables created from device characterization._ 
   
   _Filter group:_
   
   * 0x04e => 0x0908 (Filter 1)
      *   =>  TABLE[8:7]   = 0b01
      *  =>  RESERVED     = 0b00
      *      TABLE[6]     = 0b1
      *      RESERVED     = 0b1000
           
   * 0x04f => 0x8900 (Filter 2) ++++
      *   TABLE[5] =   0b0
      *   TABLE[4:3] = 0b10
      *   TABLE[2:1] = 0b00
      *   TABLE[0] =   0b0
 
  _This group cannot be calculated with an algorithm and is based on lookup tables created from device characterization._ 
     
     
