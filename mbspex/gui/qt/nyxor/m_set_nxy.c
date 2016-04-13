// N.Kurz, EE, GSI, 03-Sep-2010
// N.Kurz, EE, GSI, 27-May-2013: adopted for Linux
// N.Kurz, EE, GSI, 02-Jul-2015: adopted mbspex lib
// J.Adamczewski-Musch, EE, GSI 27-Jul-2015: added filename as argument, included to nyxor gui distribution
// N.Kurz, EE, GSI, 03-Feb-2016: adopted for nxyter version 2 (external dac setting)
// J.Adamczewski-Musch, EE, GSI 11-Apr-2016: merged back to nyxor gui version with file name



#define USE_MBSPEX_LIB       1 // this define will switch on usage of mbspex lib with locked ioctls
                               // instead of direct register mapping usage

#define NXYTER_V2         //       nXYTer version 2
                          // else  nXYTer version 1 (comment above line)

#include "stdio.h"
#include "s_veshe.h"
#include "stdarg.h"
#include <libgen.h>

#include <sys/file.h>
#ifndef Linux
 #include <mem.h>
 #include <smem.h>
 #include <stdarg.h>


#else
 #include "smem_mbs.h"
 #include <unistd.h>
 #include <stdlib.h>
 #include <string.h>
 #include <sys/mman.h>
#endif




#ifdef USE_MBSPEX_LIB
 #include "mbspex/libmbspex.h"

 void printm (char *fmt, ...)
 {
   char c_str[256];
   va_list args;
   va_start(args, fmt);
   vsprintf (c_str, fmt, args);
   printf ("%s", c_str);
   va_end(args);
 }
#else
#include  "./pexor_gosip.h"

#endif

//#define exit (0);

//#define DEBUG
//#define DEBUG2
//#define DEBUG3

#define SFP0_NYX_IN_USE "SFP0_NYX_IN_USE"
#define SFP1_NYX_IN_USE "SFP1_NYX_IN_USE"
#define SFP2_NYX_IN_USE "SFP2_NYX_IN_USE"
#define SFP3_NYX_IN_USE "SFP3_NYX_IN_USE"

#define SETUP_FILE "./nxy_nyxor_set.txt"

#define MAX_INP_LEN 200
#define MAX_PAR      16
#define MAX_SFP       4
#define MAX_NYX      16
#define MAX_nXY       2

//#define MAX_TRG_WIND  0x7ff
#define MAX_TRG_WIND  0xbff

#ifdef DEBUG
 #define debug(x)        printf x
#else // DEBUG
 #define debug(x)
#endif // DEBUG

#ifdef DEBUG2
 #define debug2(x)       printf x
#else // DEBUG2
 #define debug2(x)
#endif // DEBUG2

#ifdef DEBUG3
 #define debug3(x)       printf x
#else // DEBUG3
 #define debug3(x)
#endif // DEBUG3

#define SFP  "SFP"
#define NYX  "NYX"
#define nXY  "nXY"

#ifdef  NXYTER_V2
 #define CHECKBITS   0x3ffff
#else
#define CHECKBITS   0x1ffff
#endif
#define CHECKBITS2  0x1   
#define CHECKBITS3  0x1   

#define GLOBAL_PARAM  "GLOBAL_PARAM"
#define NXY_CTRL      "NXY_CTRL"
#define I2C_ADDR      "I2C_ADDR"
#define RESET         "RESET"     
#define MASK          "MASK"
#define BIAS          "BIAS"
#define CONFIG        "CONFIG"
#define TEST_DELAY    "TEST_DELAY"
#define THR_TEST      "THR_TEST"
#define THR_0_15      "THR_0_15"
#define THR_16_31     "THR_16_31"
#define THR_32_47     "THR_32_47"
#define THR_48_63     "THR_48_63"
#define THR_64_79     "THR_64_79"
#define THR_80_95     "THR_80_95"
#define THR_96_111    "THR_96_111"
#define THR_112_127   "THR_112_127"
#define CLOCK_DELAY   "CLOCK_DELAY" 
#define TE_TRG_DEL    "TE_TRG_DEL"
#define TRG_WIND      "TRG_WIND"
#define ADC_DCO_PHASE "ADC_DCO_PHASE"
#ifdef NXYTER_V2
 #define EXT_DACS     "EXT_DACS"
#endif

#define CLOCK_TIME   31.25 


#define GOS_I2C_DWR  0x8010  // i2c data write reg.   addr 
#define GOS_I2C_DRR1 0x8020  // i2c data read  reg. 1 addr 
#define GOS_I2C_DWR2 0x8040  // i2c data read  reg. 2 addr 
#define GOS_I2C_SR   0x8080  // i2c status     reg.   addr 

#define I2C_CTRL_A   0x01
#define I2C_COTR_A   0x03

#define CHECK_DAT    0xa9000000      


#define GET_BAR0_BASE     0x1234
#define PEXDEV            "/dev/pexor"
#define PCI_BAR0_NAME     "PCI_BAR0_MBS"
#define PCI_BAR0_SIZE     0x800000  // in bytes

#define RON  "\x1B[7m"
#define RES  "\x1B[0m"
#define RED  "\x1B[31m\x1B[1m"
#define BLK  "\x1B[0m"
#define BLU  "\x1b[34m\x1B[1m"
#define MAG  "\x1b[35m\x1B[1m"
#define BLD  "\x1B[1m"

static int  l_i, l_j, l_k, l_l;
//static s_pexor        sPEXOR;
static long           l_stat;
static int            fd_pex; 
//static int            l_bar0_base;
static long volatile *pl_virt_bar0;
static long           l_dat1, l_dat2, l_dat3;
static long           l_data; 
static long           l_check_d; 

int  f_pex_slave_rd (long, long, long, long*);
int  f_pex_slave_wr (long, long, long,  long);
int  f_pex_slave_init (long, long);
void f_i2c_sleep ();

#ifndef USE_MBSPEX_LIB
int  f_pex_send_and_receive_tok (long, long, long*, long*, long*);
int  f_pex_send_tok (long, long);
int  f_pex_receive_tok (long, long*, long*, long*);
static   s_pexor  sPEXOR;
#endif

int usage(const char* progname)
{
   printf ("***************************************************************************\n");
   printf (" %s - set up tool for NYXOR frontends\n", progname);
   printf (" \tv0.8 12-Apr-2016 by N.Kurz, EE GSI\n");
   printf ("***************************************************************************\n");
   printf (
       "  usage: %s [FILENAME] (default:%s)\n", progname,SETUP_FILE);

  return 0;
}




int main (int argc, char **argv)
{




  char c_input[MAX_INP_LEN];
  FILE *l_fd;
  char c_pars[32];
  char c_global[16];
  char c_nxy_ctrl[16];
  char c_sfpx[16];
  char c_sfp[4];
  char c_nyx[4];
  char c_nxy[4];
  char c_par[80];

  long l_sfp;
  long l_nyx;
  long l_nxy; 
  long l_par[MAX_PAR];

  long l_nxy_ctrl     [MAX_SFP][MAX_NYX]; 

  long l_i2c_addr     [MAX_SFP][MAX_NYX][MAX_nXY];
  long l_reset        [MAX_SFP][MAX_NYX][MAX_nXY];
  long l_mask         [MAX_SFP][MAX_NYX][MAX_nXY][MAX_PAR];
  long l_bias         [MAX_SFP][MAX_NYX][MAX_nXY][14];
  long l_config       [MAX_SFP][MAX_NYX][MAX_nXY][2];
  long l_te_del       [MAX_SFP][MAX_NYX][MAX_nXY][2];
  long l_clk_del      [MAX_SFP][MAX_NYX][MAX_nXY][3];
  long l_thr_te       [MAX_SFP][MAX_NYX][MAX_nXY];
  long l_thr          [MAX_SFP][MAX_NYX][MAX_nXY][128];
  long l_adc_dco_phase[MAX_SFP][MAX_NYX][MAX_nXY];
  long l_nxy_v2=0;
#ifdef NXYTER_V2
  long l_ext_dac      [MAX_SFP][MAX_NYX][MAX_nXY][4];
#endif

  long l_pre_trg_wind=0;
  long l_pos_trg_wind=0;
  long l_test_pul_del=0;
  long l_test_trg_del=0; 

  long l_check  [MAX_SFP][MAX_NYX][MAX_nXY]; 
  long l_check2=0; 
  long l_check3 [MAX_SFP][MAX_NYX];  
  long l_check_err=0;

  long unsigned  l_sfp0_in_use[MAX_NYX];
  long unsigned  l_sfp1_in_use[MAX_NYX];
  long unsigned  l_sfp2_in_use[MAX_NYX];
  long unsigned  l_sfp3_in_use[MAX_NYX];
  long unsigned  l_in_use[MAX_SFP][MAX_NYX];
 
  long unsigned  l_wr_d;  

  int            prot;
  int            flags;

  char c_filename[1024];

  #ifdef NXYTER_V2
  printf (BLU"ATTENTION>> program version is for nXYTer version 2 (NYXOR) "RES"\n");  
  #else
  printf (BLU"ATTENTION>> program version is for nXYTer version 1 (GEMEX) "RES"\n");  
  #endif



  if ((argc==2) && (!strcmp(argv[1],"?") || !strcmp(argv[1],"-h")  || !strcmp(argv[1],"-help") || !strcmp(argv[1],"--help"))) return usage(basename(argv[0]));

  if(argc>=2)
    strncpy(c_filename,argv[1],1024);
  else
    strncpy(c_filename,SETUP_FILE,1024);





  // GEMEX/NYXOR nXYter setup file
  if ((l_fd = fopen (c_filename, "r")) == NULL)
  {
    printf ("input file %s not found \n", c_filename);
    //perror ("fopen");
    exit (0);
  }
  else
  {
    debug (("opened file: %s \n", c_filename));
  }

  for (l_i=0; l_i<MAX_SFP; l_i++)
  {
    for (l_j=0; l_j<MAX_NYX; l_j++)
    {
      l_sfp0_in_use[l_j] = 0;
      l_sfp1_in_use[l_j] = 0;
      l_sfp2_in_use[l_j] = 0;
      l_sfp3_in_use[l_j] = 0;

      l_nxy_ctrl[l_i][l_j] = 0;
      l_check3  [l_i][l_j] = 0;
       for (l_k=0; l_k<MAX_nXY; l_k++)
      {
        l_i2c_addr [l_i][l_j][l_k] = 0;
        l_reset    [l_i][l_j][l_k] = 0;
         for (l_l=0; l_l<16; l_l++)
        {
          l_mask[l_i][l_j][l_k][l_l] = 0;
        }
         for (l_l=0; l_l<14; l_l++)
        {
          l_bias[l_i][l_j][l_k][l_l] = 0;
        }
        l_config [l_i][l_j][l_k][0] = 0;
        l_config [l_i][l_j][l_k][1] = 0;
        l_te_del [l_i][l_j][l_k][0] = 0;
        l_te_del [l_i][l_j][l_k][1] = 0;
        l_clk_del[l_i][l_j][l_k][0] = 0;
        l_clk_del[l_i][l_j][l_k][1] = 0;
        l_clk_del[l_i][l_j][l_k][2] = 0;
        l_thr_te [l_i][l_j][l_k]    = 0;
         for (l_l=0; l_l<128; l_l++)
        {
          l_thr[l_i][l_j][l_k][l_l] = 0;
        }
        l_adc_dco_phase[l_i][l_j][l_k] = 0;
        #ifdef NXYTER_V2
        for (l_l=0; l_l<4; l_l++)
        {
          l_ext_dac[l_i][l_j][l_k][l_l] = 0;
        }
        #endif
        l_check  [l_i][l_j][l_k]    = 0;
      } 
    }
  }

  // loop over all strings contained in the input file
  while (fgets (c_input, MAX_INP_LEN, l_fd) != NULL)
  {
    if (strlen(c_input)>1)        // if not nullstring
    {
      strcpy (c_sfp, "");
      strcpy (c_nyx, "");
      strcpy (c_nxy, "");
      strcpy (c_par, "");
      for (l_i=0; l_i<MAX_PAR; l_i++)
      {
        l_par[l_i] = 0;
      } 

      sscanf (c_input, "%s", c_pars);
      debug (("c_pars: %s \n", c_pars));

      if (strcmp (c_pars, SFP0_NYX_IN_USE) == 0)
      {
        sscanf (c_input, "%s %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d", c_sfpx,
             &l_par [0], &l_par [1], &l_par [2], &l_par [3],
             &l_par [4], &l_par [5], &l_par [6], &l_par [7],
             &l_par [8], &l_par [9], &l_par[10], &l_par[11],
             &l_par[12], &l_par[13], &l_par[14], &l_par[15]);
        for (l_i=0; l_i<MAX_PAR; l_i++)
        { 
          l_sfp0_in_use[l_i] = l_par[l_i] & 0x3;
        } 
        debug2 (("SFP 0: Nr. of nXYter in use per GEMEX/NYXOR \n"));
        for (l_i=0; l_i<16; l_i++)
        { 
          debug2 (("%4d", l_sfp0_in_use[l_i]));
        }
        debug2 (("\n")); 
      }   

      else if (strcmp (c_pars, SFP1_NYX_IN_USE) == 0)
      {
        sscanf (c_input, "%s %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d", c_sfpx,
             &l_par [0], &l_par [1], &l_par [2], &l_par [3],
             &l_par [4], &l_par [5], &l_par [6], &l_par [7],
             &l_par [8], &l_par [9], &l_par[10], &l_par[11],
             &l_par[12], &l_par[13], &l_par[14], &l_par[15]);
        for (l_i=0; l_i<MAX_PAR; l_i++)
        { 
          l_sfp1_in_use[l_i] = l_par[l_i] & 0x3;
        } 
        debug2 (("SFP 1: Nr. of nXYter in use per GEMEX/NYXOR \n"));
        for (l_i=0; l_i<16; l_i++)
        { 
          debug2 (("%4d", l_sfp1_in_use[l_i]));
        }
        debug2 (("\n")); 
      }   

      else if (strcmp (c_pars, SFP2_NYX_IN_USE) == 0)
      {
        sscanf (c_input, "%s %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d", c_sfpx,
             &l_par [0], &l_par [1], &l_par [2], &l_par [3],
             &l_par [4], &l_par [5], &l_par [6], &l_par [7],
             &l_par [8], &l_par [9], &l_par[10], &l_par[11],
             &l_par[12], &l_par[13], &l_par[14], &l_par[15]);
        for (l_i=0; l_i<MAX_PAR; l_i++)
        { 
          l_sfp2_in_use[l_i] = l_par[l_i] & 0x3;
        } 
        debug2 (("SFP 2: Nr. of nXYter in use per GEMEX/NYXOR \n"));
        for (l_i=0; l_i<16; l_i++)
        { 
          debug2 (("%4d", l_sfp2_in_use[l_i]));
        }
        debug2 (("\n")); 
      }   

      else if (strcmp (c_pars, SFP3_NYX_IN_USE) == 0)
      {
        sscanf (c_input, "%s %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d", c_sfpx,
             &l_par [0], &l_par [1], &l_par [2], &l_par [3],
             &l_par [4], &l_par [5], &l_par [6], &l_par [7],
             &l_par [8], &l_par [9], &l_par[10], &l_par[11],
             &l_par[12], &l_par[13], &l_par[14], &l_par[15]);
        for (l_i=0; l_i<MAX_PAR; l_i++)
        { 
          l_sfp3_in_use[l_i] = l_par[l_i] & 0x3;
        } 
        debug2 (("SFP 3: Nr. of nXYter in use per GEMEX/NYXOR \n"));
        for (l_i=0; l_i<16; l_i++)
        { 
          debug2 (("%4d", l_sfp3_in_use[l_i]));
        }
        debug2 (("\n")); 
      }   


      else if (strcmp (c_pars, GLOBAL_PARAM) == 0)
      {
        if (l_check2 == 0)
        {
          debug (("input string: %s", c_input));
          sscanf (c_input, "%s %x %x %x %x",
                               c_global, &l_par [0], &l_par [1], &l_par [2], &l_par [3]);
          l_pre_trg_wind = l_par[0] & 0xfff;
          l_pos_trg_wind = l_par[1] & 0xfff;
          l_test_pul_del = l_par[2] & 0xff;
          l_test_trg_del = l_par[3] & 0xff;
          l_check2 += 1;

          if ( (l_pre_trg_wind + l_pos_trg_wind) > MAX_TRG_WIND)
          {
            printf (RED"ERROR>> sum of pre - and post trigger window too big (max: %d) "RES"\n", MAX_TRG_WIND);
            printf ("        pre: %d, post %d, sum: %d \n",
                    l_pre_trg_wind, l_pos_trg_wind, l_pre_trg_wind + l_pos_trg_wind);       
            printf ("exiting.. \n");
            exit (0);  
          } 
       
          debug2 (("Global system parameters: \n"));
          debug2 (("pre  trigger window: %d * 31.25 ns = %6.1f ns \n",
                     l_pre_trg_wind, (float) l_pre_trg_wind * (float) CLOCK_TIME));
          debug2 (("post trigger window: %d * 31.25 ns = %6.1f ns \n",
                     l_pos_trg_wind, (float) l_pos_trg_wind * (float) CLOCK_TIME));  
  
          debug2 (("test pulse delay:    %d * 31.25 ns = %6.1f ns \n",
                     l_test_pul_del, (float) l_test_pul_del * (float) CLOCK_TIME));  
          debug2 (("test trigger delay:  %d * 31.25 ns = %6.1f ns \n",
                     l_test_trg_del, (float) l_test_trg_del * (float) CLOCK_TIME));
          debug2 (("\n"));
        }  
        else
        {
            printf ("\nERROR>> global parameters already specified, take previsous \n\n");
        }           
      }

      else if (strcmp (c_pars, NXY_CTRL) == 0)
      {
        debug (("input string: %s", c_input));
        sscanf (c_input, "%s %s %d %s %d %x",  c_nxy_ctrl, c_sfp, &l_sfp, c_nyx, &l_nyx, &l_par[0]);
        debug (("%s: %s: %x, %s: %x, 0x%x \n", c_nxy_ctrl, c_sfp,  l_sfp, c_nyx,  l_nyx,  l_par[0]));
        l_nxy_ctrl[l_sfp][l_nyx] = l_par[0] & 0x3fff;
        l_check3[l_sfp][l_nyx] += 1;  
      }
      else if (strcmp (c_pars, SFP) == 0)
      {
        debug (("input string: %s", c_input));
        sscanf (c_input, "%s %d %s %d %s %d %s %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x",
             c_sfp, &l_sfp, c_nyx, &l_nyx, c_nxy, &l_nxy, c_par,
             &l_par [0], &l_par [1], &l_par [2], &l_par [3],
             &l_par [4], &l_par [5], &l_par [6], &l_par [7],
             &l_par [8], &l_par [9], &l_par[10], &l_par[11],
             &l_par[12], &l_par[13], &l_par[14], &l_par[15]);

        debug (("%s: %d, %s: %d, %s: %d: %s: \n",
              c_sfp, l_sfp, c_nyx, l_nyx, c_nxy, l_nxy, c_par));
        debug (("0-7  (hex):  %4x %4x %4x %4x %4x %4x %4x %4x \n",
              l_par [0], l_par [1], l_par [2], l_par [3],
              l_par [4], l_par [5], l_par [6], l_par [7]));
        debug (("8-15 (hex):  %4x %4x %4x %4x %4x %4x %4x %4x \n",
              l_par  [8], l_par  [9], l_par [10], l_par [11],
              l_par [12], l_par [13], l_par [14], l_par [15]));

        // fill setup parameter values found in file into setup variables 

        if (    (strcmp (c_sfp, SFP) == 0)
             && (strcmp (c_nyx, NYX) == 0)
             && (strcmp (c_nxy, nXY) == 0)
             && ((l_sfp >=0) && (l_sfp < MAX_SFP)) 
             && ((l_nyx >=0) && (l_nyx < MAX_NYX)) 
             && ((l_nxy >=0) && (l_nxy < MAX_nXY)) )
        {
          if (strcmp (c_par, I2C_ADDR) == 0)
          {
            l_i2c_addr[l_sfp][l_nyx][l_nxy] = l_par[0] & 0xff;
            debug2 (("SFP %d Exp %d nXY %d: I2C write address (hex):                         %x\n",
                  l_sfp, l_nyx, l_nxy, l_i2c_addr[l_sfp][l_nyx][l_nxy]));
            l_check[l_sfp][l_nyx][l_nxy] += 1;  
            debug2 (("SFP %d Exp %d nXY %d, checkbits: 0x%x \n",
                            l_sfp, l_nyx, l_nxy, l_check[l_sfp][l_nyx][l_nxy]));
          }   
          else if (strcmp (c_par, RESET) == 0)
          {
            l_reset[l_sfp][l_nyx][l_nxy] = l_par[0] & 0xff;
            debug2 (("SFP %d Exp %d nXY %d: I2C and nXYter reset parameter (hex):            %x\n",
                  l_sfp, l_nyx, l_nxy, l_reset[l_sfp][l_nyx][l_nxy]));
            l_check[l_sfp][l_nyx][l_nxy] += 2;  
            debug2 (("SFP %d Exp %d nXY %d, checkbits: 0x%x \n",
                            l_sfp, l_nyx, l_nxy, l_check[l_sfp][l_nyx][l_nxy]));
          }   
          else if (strcmp (c_par, MASK) == 0)
          {
            for (l_i=0; l_i<MAX_PAR; l_i++)
            { 
              l_mask[l_sfp][l_nyx][l_nxy][l_i] = l_par[l_i] & 0xff;
            } 
            debug2 (("SFP %d Exp %d nXY %d: 16 nXY mask register (hex):\n",
                  l_sfp, l_nyx, l_nxy)); 
            for (l_i=0; l_i<MAX_PAR; l_i++)
            {   
              debug2 (("%4x", l_mask[l_sfp][l_nyx][l_nxy][l_i]));
            }
            debug2 (("\n")); 
            l_check[l_sfp][l_nyx][l_nxy] += 4;  
            debug2 (("SFP %d Exp %d nXY %d, checkbits: 0x%x \n",
                            l_sfp, l_nyx, l_nxy, l_check[l_sfp][l_nyx][l_nxy]));
          }   
          else if (strcmp (c_par, BIAS) == 0)
          {
            for (l_i=0; l_i<14; l_i++)
            { 
              l_bias[l_sfp][l_nyx][l_nxy][l_i] = l_par[l_i] & 0xff;
            } 
            debug2 (("SFP %d Exp %d nXY %d: 14 nXY bias register (hex):\n",
                  l_sfp, l_nyx, l_nxy)); 
            for (l_i=0; l_i<14; l_i++)
            { 
              debug2 (("%4x", l_bias[l_sfp][l_nyx][l_nxy][l_i]));
            }
            debug2 (("\n")); 
            l_check[l_sfp][l_nyx][l_nxy] += 8;  
            debug2 (("SFP %d Exp %d nXY %d, checkbits: 0x%x \n",
                            l_sfp, l_nyx, l_nxy, l_check[l_sfp][l_nyx][l_nxy]));
          }   
          else if (strcmp (c_par, CONFIG) == 0)
          {
            for (l_i=0; l_i<2; l_i++)
            { 
              l_config[l_sfp][l_nyx][l_nxy][l_i] = l_par[l_i] & 0xff;
            } 
            debug2 (("SFP %d Exp %d nXY %d: 2 nXY config register (hex):                   ",
                  l_sfp, l_nyx, l_nxy)); 
            for (l_i=0; l_i<2; l_i++)
            { 
              debug2 (("%4x", l_config[l_sfp][l_nyx][l_nxy][l_i]));
            }
            debug2 (("\n")); 
            l_check[l_sfp][l_nyx][l_nxy] += 0x10;  
            debug2 (("SFP %d Exp %d nXY %d, checkbits: 0x%x \n",
                            l_sfp, l_nyx, l_nxy, l_check[l_sfp][l_nyx][l_nxy]));
          }   
          else if (strcmp (c_par, TEST_DELAY) == 0)
          {
            for (l_i=0; l_i<2; l_i++)
            { 
              l_te_del[l_sfp][l_nyx][l_nxy][l_i] = l_par[l_i] & 0xff;
            } 
            debug2 (("SFP %d Exp %d nXY %d: 2 nXY test pulse/trigger delay register (hex): ",
                  l_sfp, l_nyx, l_nxy)); 
            for (l_i=0; l_i<2; l_i++)
            { 
              debug2 (("%4x", l_te_del[l_sfp][l_nyx][l_nxy][l_i]));
            }
            debug2 (("\n")); 
            l_check[l_sfp][l_nyx][l_nxy] += 0x20;  
            debug2 (("SFP %d Exp %d nXY %d, checkbits: 0x%x \n",
                            l_sfp, l_nyx, l_nxy, l_check[l_sfp][l_nyx][l_nxy]));
          }   
          else if (strcmp (c_par, CLOCK_DELAY) == 0)
          {
            for (l_i=0; l_i<3; l_i++)
            { 
              l_clk_del[l_sfp][l_nyx][l_nxy][l_i] = l_par[l_i] & 0xff;
            } 
            debug2 (("SFP %d Exp %d nXY %d: 3 nXY clock delay register (hex):              ",
                    l_sfp, l_nyx, l_nxy)); 
            for (l_i=0; l_i<3; l_i++)
            { 
              debug2 (("%4x", l_clk_del[l_sfp][l_nyx][l_nxy][l_i]));
            }
            debug2 (("\n")); 
            l_check[l_sfp][l_nyx][l_nxy] += 0x40;  
            debug2 (("SFP %d Exp %d nXY %d, checkbits: 0x%x \n",
                            l_sfp, l_nyx, l_nxy, l_check[l_sfp][l_nyx][l_nxy]));
          }   
          else if (strcmp (c_par, THR_TEST) == 0)
          {
            l_thr_te[l_sfp][l_nyx][l_nxy] = l_par[0] & 0xff;
            debug2 (("SFP %d Exp %d nXY %d: Test threshold (hex):                            %x\n",
                  l_sfp, l_nyx, l_nxy, l_thr_te[l_sfp][l_nyx][l_nxy])); 
            l_check[l_sfp][l_nyx][l_nxy] += 0x80;  
            debug2 (("SFP %d Exp %d nXY %d, checkbits: 0x%x \n",
                            l_sfp, l_nyx, l_nxy, l_check[l_sfp][l_nyx][l_nxy]));
          }   
          else if (strcmp (c_par, THR_0_15) == 0)
          {
            for (l_i=0; l_i<MAX_PAR; l_i++)
            { 
              l_thr[l_sfp][l_nyx][l_nxy][l_i] = l_par[l_i] & 0xff;
            } 
            debug2 (("SFP %d Exp %d nXY %d: 16 nXY thresholds for channel    0-15  (hex):\n",
                  l_sfp, l_nyx, l_nxy)); 
            for (l_i=0; l_i<16; l_i++)
            { 
              debug2 (("%4x", l_thr[l_sfp][l_nyx][l_nxy][l_i]));
            }
            debug2 (("\n")); 
            l_check[l_sfp][l_nyx][l_nxy] += 0x100;  
            debug2 (("SFP %d Exp %d nXY %d, checkbits: 0x%x \n",
                            l_sfp, l_nyx, l_nxy, l_check[l_sfp][l_nyx][l_nxy]));
          }   
          else if (strcmp (c_par, THR_16_31) == 0)
          {
            for (l_i=0; l_i<MAX_PAR; l_i++)
            { 
              l_thr[l_sfp][l_nyx][l_nxy][l_i+MAX_PAR] = l_par[l_i] & 0xff;
            } 
            debug2 (("SFP %d Exp %d nXY %d: 16 nXY thresholds for channel   16-31 (hex):\n",
                  l_sfp, l_nyx, l_nxy)); 
            for (l_i=(MAX_PAR); l_i<=31; l_i++)
            { 
              debug2 (("%4x", l_thr[l_sfp][l_nyx][l_nxy][l_i]));
            }
            debug2 (("\n")); 
            l_check[l_sfp][l_nyx][l_nxy] += 0x200;  
            debug2 (("SFP %d Exp %d nXY %d, checkbits: 0x%x \n",
                            l_sfp, l_nyx, l_nxy, l_check[l_sfp][l_nyx][l_nxy]));
          }   
          else if (strcmp (c_par, THR_32_47) == 0)
          {
            for (l_i=0; l_i<MAX_PAR; l_i++)
            { 
              l_thr[l_sfp][l_nyx][l_nxy][l_i+2*MAX_PAR] = l_par[l_i] & 0xff;
            } 
            debug2 (("SFP %d Exp %d nXY %d: 16 nXY thresholds for channel   32-47 (hex):\n",
                  l_sfp, l_nyx, l_nxy)); 
            for (l_i=(2*MAX_PAR); l_i<=47; l_i++)
            { 
              debug2 (("%4x", l_thr[l_sfp][l_nyx][l_nxy][l_i]));
            }
            debug2 (("\n")); 
            l_check[l_sfp][l_nyx][l_nxy] += 0x400;  
            debug2 (("SFP %d Exp %d nXY %d, checkbits: 0x%x \n",
                            l_sfp, l_nyx, l_nxy, l_check[l_sfp][l_nyx][l_nxy]));
          }   
          else if (strcmp (c_par, THR_48_63) == 0)
          {
            for (l_i=0; l_i<MAX_PAR; l_i++)
            { 
              l_thr[l_sfp][l_nyx][l_nxy][l_i+3*MAX_PAR] = l_par[l_i] & 0xff ;
            } 
            debug2 (("SFP %d Exp %d nXY %d: 16 nXY thresholds for channel   48-63 (hex):\n",
                    l_sfp, l_nyx, l_nxy)); 
            for (l_i=(3*MAX_PAR); l_i<=63; l_i++)
            { 
              debug2 (("%4x", l_thr[l_sfp][l_nyx][l_nxy][l_i]));
            }
            debug2 (("\n")); 
            l_check[l_sfp][l_nyx][l_nxy] += 0x800;  
            debug2 (("SFP %d Exp %d nXY %d, checkbits: 0x%x \n",
                            l_sfp, l_nyx, l_nxy, l_check[l_sfp][l_nyx][l_nxy]));
          }   
          else if (strcmp (c_par, THR_64_79) == 0)
          {
            for (l_i=0; l_i<MAX_PAR; l_i++)
            { 
              l_thr[l_sfp][l_nyx][l_nxy][l_i+4*MAX_PAR] = l_par[l_i] & 0xff;
            } 
            debug2 (("SFP %d Exp %d nXY %d: 16 nXY thresholds for channel   64-79 (hex):\n",
                  l_sfp, l_nyx, l_nxy)); 
            for (l_i=(4*MAX_PAR); l_i<=79; l_i++)
            { 
              debug2 (("%4x", l_thr[l_sfp][l_nyx][l_nxy][l_i]));
            }
            debug2 (("\n")); 
            l_check[l_sfp][l_nyx][l_nxy] += 0x1000;  
            debug2 (("SFP %d Exp %d nXY %d, checkbits: 0x%x \n",
                            l_sfp, l_nyx, l_nxy, l_check[l_sfp][l_nyx][l_nxy]));
          }   
          else if (strcmp (c_par, THR_80_95) == 0)
          {
            for (l_i=0; l_i<MAX_PAR; l_i++)
            { 
              l_thr[l_sfp][l_nyx][l_nxy][l_i+5*MAX_PAR] = l_par[l_i] & 0xff;
            } 
            debug2 (("SFP %d Exp %d nXY %d: 16 nXY thresholds for channel   80-95 (hex):\n",
                  l_sfp, l_nyx, l_nxy)); 
            for (l_i=(5*MAX_PAR); l_i<=95; l_i++)
            { 
              debug2 (("%4x", l_thr[l_sfp][l_nyx][l_nxy][l_i]));
            }
            debug2 (("\n")); 
            l_check[l_sfp][l_nyx][l_nxy] += 0x2000;  
            debug2 (("SFP %d Exp %d nXY %d, checkbits: 0x%x \n",
                            l_sfp, l_nyx, l_nxy, l_check[l_sfp][l_nyx][l_nxy]));
          }   
          else if (strcmp (c_par, THR_96_111) == 0)
          {
            for (l_i=0; l_i<MAX_PAR; l_i++)
            { 
              l_thr[l_sfp][l_nyx][l_nxy][l_i+6*MAX_PAR] = l_par[l_i] & 0xff;
            } 
            debug2 (("SFP %d Exp %d nXY %d: 16 nXY thresholds for channel  96-111 (hex):\n",
                  l_sfp, l_nyx, l_nxy)); 
            for (l_i=(6*MAX_PAR); l_i<=111; l_i++)
            { 
              debug2 (("%4x", l_thr[l_sfp][l_nyx][l_nxy][l_i]));
            }
            debug2 (("\n")); 
            l_check[l_sfp][l_nyx][l_nxy] += 0x4000;  
            debug2 (("SFP %d Exp %d nXY %d, checkbits: 0x%x \n",
                            l_sfp, l_nyx, l_nxy, l_check[l_sfp][l_nyx][l_nxy]));
          }   
          else if (strcmp (c_par, THR_112_127) == 0)
          {
            for (l_i=0; l_i<MAX_PAR; l_i++)
            { 
              l_thr[l_sfp][l_nyx][l_nxy][l_i+7*MAX_PAR] = l_par[l_i] & 0xff;
            } 
            debug2 (("SFP %d Exp %d nXY %d: 16 nXY thresholds for channel 112-127 (hex):\n",
                  l_sfp, l_nyx, l_nxy)); 
            for (l_i=(7*MAX_PAR); l_i<=127; l_i++)
            { 
              debug2 (("%4x", l_thr[l_sfp][l_nyx][l_nxy][l_i]));
            }
            debug2 (("\n")); 
            l_check[l_sfp][l_nyx][l_nxy] += 0x8000;  
            debug2 (("SFP %d Exp %d nXY %d, checkbits: 0x%x \n",
                            l_sfp, l_nyx, l_nxy, l_check[l_sfp][l_nyx][l_nxy]));
          }
          else if (strcmp (c_par, ADC_DCO_PHASE) == 0)
          {
            l_adc_dco_phase[l_sfp][l_nyx][l_nxy] = l_par[0] & 0xff;
            debug2 (("SFP %d Exp %d nXY %d: ADC DCO PHASE (hex):                             %x\n",
                  l_sfp, l_nyx, l_nxy, l_adc_dco_phase[l_sfp][l_nyx][l_nxy]));
            l_check[l_sfp][l_nyx][l_nxy] += 0x10000;  
            debug2 (("SFP %d Exp %d nXY %d, checkbits: 0x%x \n",
                            l_sfp, l_nyx, l_nxy, l_check[l_sfp][l_nyx][l_nxy]));
          }   

          #ifdef NXYTER_V2
          else if (strcmp (c_par, EXT_DACS) == 0)
          {
            for (l_i=0; l_i<4; l_i++)
            { 
              l_ext_dac[l_sfp][l_nyx][l_nxy][l_i] = l_par[l_i] & 0x3ff;
        }
            debug2 (("SFP %d Exp %d nXY %d: 4 external dac registers (hex):",
                    l_sfp, l_nyx, l_nxy)); 
            for (l_i=0; l_i<4; l_i++)
            { 
              debug2 (("%4x", l_ext_dac[l_sfp][l_nyx][l_nxy][l_i]));
      }
            debug2 (("\n")); 
            l_check[l_sfp][l_nyx][l_nxy] += 0x20000;  
            debug2 (("SFP %d Exp %d nXY %d, checkbits: 0x%x \n",
                            l_sfp, l_nyx, l_nxy, l_check[l_sfp][l_nyx][l_nxy]));
          }   
          #endif
        }
      }
      else if (   (strcmp (c_pars, "#") == 0)
                 || (strcmp (c_pars, "#SFP0_NYX_IN_USE") == 0)
                 || (strcmp (c_pars, "#SFP1_NYX_IN_USE") == 0)
                 || (strcmp (c_pars, "#SFP2_NYX_IN_USE") == 0)
                 || (strcmp (c_pars, "#SFP3_NYX_IN_USE") == 0)
                 || (strcmp (c_pars, "#GLOBAL_PARAM") == 0)
                 || (strcmp (c_pars, "#NXY_CTRL") == 0)
                 || (strcmp (c_pars, "#SFP") == 0) )
      {}
      else
      {
        printf (RED"ERROR>> wrong line in input file %s "RES"\n", SETUP_FILE);
        printf ("input string: %s", c_input);
        printf ("\nexiting.. \n");
        exit (0);
      }
     }
  }

  for (l_i=0; l_i<MAX_NYX; l_i++)
  {
    l_in_use[0][l_i] = l_sfp0_in_use[l_i];
    l_in_use[1][l_i] = l_sfp1_in_use[l_i];
    l_in_use[2][l_i] = l_sfp2_in_use[l_i];
    l_in_use[3][l_i] = l_sfp3_in_use[l_i];
  }


  if (l_check2 == 0)
  {
    printf ("\n");
    printf (RED"ERROR>> global parameters not specified. exiting.. "RES"\n\n");
    exit (0); 
  }

  // check consistency and completeness of nXYter setup (somehow)
  l_check_err = 0;
  for (l_i=0; l_i<MAX_SFP; l_i++)          // loop over SFPs
  {
    if (l_in_use[l_i][0] > 0)              // sfp l_i used
    {
      for (l_j=0; l_j<MAX_NYX; l_j++)      // loop over GEMEX/NYXOR
      {
        if (l_in_use[l_i][l_j] != 0) 
        {
          if (l_check3[l_i][l_j] != CHECKBITS3)
          {
            printf ("l_i %d, l_j %d \n", l_i, l_j);

            l_check_err++;
            printf (RED"ERROR>> for SFP: %d, NYX: %d "RES"", l_i, l_j);  
            printf ("no entry for nXYTer control register found (NXY_CTRL)\n");
            printf ("\n"); 
          }
        }

        //printf ("SFP: %d, NYX: %d, nXY: %d  \n", l_i, l_j, l_in_use[l_i][l_j]);
        if (l_in_use[l_i][l_j] == 0) {break;}  
        if (l_in_use[l_i][l_j] > MAX_nXY)
        {
          printf (RED"ERROR>> for SFP: %d, NYX: %d too many nXYter specified "RES"\n", l_i, l_j);
          printf ("        allowed nXYter: %d, specified: %d \n", MAX_nXY, l_in_use[l_i][l_j]);
          printf ("exiting.. \n"); exit (0); 
        }
        for (l_k=0; l_k<l_in_use[l_i][l_j]; l_k++)  // loop over nXYter
        {
          printf ("USED: SFP: %d, NYX: %d, nXY: %d \n", l_i, l_j, l_k);
          // check if for all SFP, NYX, nXY in use a full parameter setup
          // was found in input file
          if (l_check[l_i][l_j][l_k] != CHECKBITS)
          {
            l_check_err++;
            printf (RED"ERROR>> for SFP: %d, NYX: %d, nXY: %d "RES"", l_i, l_j, l_k);  
            printf ("no complete setup parameter set found in input file \n");
            printf ("\n"); 
          }
        }
      }
    }  
  }
  if (l_check_err != 0)
  {
    printf ("please correct setup input file and try again, exiting.. \n");
    exit (1);
  }

  #ifdef USE_MBSPEX_LIB
  
  if ((fd_pex = mbspex_open (0)) == -1)
  {
    printf (RED"ERROR>>"RES" could not open mbspex device "RES"\n");
    exit (0);
  }

  #else // USE_MBSPEX_LIB 

  // open PEXOR device 
  if ((fd_pex = open (PEXDEV, O_RDWR)) == -1)
  {
    printf (RED"ERROR>> could not open %s device "RES"\n", PEXDEV);
    exit (0);
  }
  else
  {
    printf ("opened device: %s, fd = %d \n", PEXDEV, fd_pex);
  }

  #ifdef Linux
  // map bar0 directly via pexor driver and access trixor base
  prot  = PROT_WRITE | PROT_READ;
  flags = MAP_SHARED | MAP_LOCKED;
  if ((pl_virt_bar0 = (long *) mmap (NULL, PCI_BAR0_SIZE, prot, flags, fd_pex, 0)) == MAP_FAILED)
  {
    printf (RON"failed to mmap bar0 from pexor"RES", return: 0x%x, %d \n", pl_virt_bar0, pl_virt_bar0);
    perror ("mmap"); 
    exit (0);
  } 
  #ifdef DEBUG
  printf ("first mapped virtual address of bar0: 0x%p \n", pl_virt_bar0);
  #endif // DEBUG

  #else // Linux

  // get bar0 base:
  l_stat = ioctl (fd_pex, GET_BAR0_BASE, &l_bar0_base);
  if (l_stat == -1 )
  {
    printf (RED"ERROR>> ioctl GET_BAR0_BASE failed "RES"\n");
  }
  else
  {
    printf ("PEXOR bar0 base: 0x%x \n", l_bar0_base);
  } 
  // open shared segment
  smem_remove(PCI_BAR0_NAME);
  if((pl_virt_bar0 = (long *) smem_create (PCI_BAR0_NAME,
            (char*) l_bar0_base, PCI_BAR0_SIZE, SM_READ | SM_WRITE))==NULL)
  {
    printf ("smem_create for PEXOR BAR0 failed");
    exit (0);
  }
  #endif // Linux

  // close pexor device
  l_stat = close (fd_pex);
  if (l_stat == -1 )
  {
    printf (RED"ERROR>> could not close PEXOR device "RES"\n");
  }

  PEXOR_GetPointer(0, pl_virt_bar0, &sPEXOR); 

  #endif // (else) USE_MBSPEX_LIB

  // initialize gosip on all connected GEMEX/NYXOR
  for (l_i=0; l_i<MAX_SFP; l_i++)          // loop over SFPs
  {
    if (l_in_use[l_i][0] > 0)              // sfp l_i used
    {
      for (l_j=0; l_j<MAX_NYX; l_j++)      // find nr. of GEMEX/NYXOR in this SFP
      {
        if (l_in_use[l_i][l_j] == 0) {break;}
      }

      l_stat = f_pex_slave_init (l_i, l_j);  
      if (l_stat == -1)
      {
        printf (RED"ERROR>> slave address initialization failed "RES"\n");
        printf ("exiting...\n"); 
        exit (0); 
      }
    }  
  }

  // setup GEMEX/NYXOR and nXYter 
  printf ("\nstart setting up ... \n");
  for (l_i=0; l_i<MAX_SFP; l_i++)          // loop over SFPs
  {
    if (l_in_use[l_i][0] > 0)              // sfp l_i used
    {
      for (l_j=0; l_j<MAX_NYX; l_j++)      // loop over GEMEX/NYXOR
      {
        debug3 (("\n\n"));
        if (l_in_use[l_i][l_j] == 0) {break;}  
        for (l_k=0; l_k<l_in_use[l_i][l_j]; l_k++)  // loop over nXYter
        {
          printf ("setup SFP: %d, NYX: %d, nXY: %d \n", l_i, l_j, l_k);

          // has to be changed a bit if more than one nxy are connected to
          // one GEMEX/NYXOR 
          l_stat = f_pex_slave_wr (l_i, l_j, GOS_I2C_DWR, 0x7f000000);
          if (l_stat == -1)
          {
            printf (RED"ERROR>> GEMEX/NYXOR and nXYter reset failed: "RES"\n"); 
            printf ("        SFP: %d, NYX: %d => A: 0x%x, D: 0x%x\n",
                                 l_i, l_j, GOS_I2C_DWR, 0x7f000000);
            printf ("exiting.. \n");
            exit (0);  
          }

          // activate I2C core and reset nXYter
          l_wr_d  = l_reset[l_i][l_j][l_k];
          l_wr_d += 0x0              <<  8;
          l_wr_d += 0x0              << 16;
          l_wr_d += I2C_CTRL_A       << 24;

          l_stat = f_pex_slave_wr (l_i, l_j, GOS_I2C_DWR, l_wr_d);
          if (l_stat == -1)
          {
            printf (RED"ERROR>> activating I2C core failed: "RES"\n"); 
            printf ("        SFP: %d, NYX: %d => A: 0x%x, D: 0x%x\n",
                                 l_i, l_j, GOS_I2C_DWR, l_wr_d);
            printf ("exiting.. \n");
            exit (0);  
          }
          else
          {
            debug3 (("set SFP: %d, NYX: %d         =>      A: 0x%x, D: 0x%x\n",
                                 l_i, l_j, GOS_I2C_DWR, l_wr_d)); 
            debug3 (("\n"));
          }

          // set nXYter mask registers
          for (l_l=0; l_l<16; l_l++)
          {
            l_wr_d  = l_mask[l_i][l_j][l_k][l_l];
            l_wr_d += (l_l                        <<  8);
            l_wr_d += (l_i2c_addr[l_i][l_j][l_k]  << 16);
            l_wr_d += (I2C_COTR_A                 << 24);

            l_stat = f_pex_slave_wr (l_i, l_j, GOS_I2C_DWR, l_wr_d);
            if (l_stat == -1)
            {
              printf (RED"ERROR>> setting nXYter masks failed: "RES"\n"); 
              printf ("        SFP: %d, NYX: %d, nXY: %d => %3d  A: 0x%x, D: 0x%x\n",
                                    l_i, l_j, l_k, l_l, GOS_I2C_DWR, l_wr_d); 
              printf ("exiting.. \n");
              exit (0);  
            }
            else
            {
              debug3 (("set SFP: %d, NYX: %d, nXY: %d => %3d  A: 0x%x, D: 0x%x\n",
                                    l_i, l_j, l_k, l_l, GOS_I2C_DWR, l_wr_d)); 
            }
          }   
          debug3 (("\n"));

          // set nXYter bias registers
          #ifdef NXYTER_V2
          printf (BLU"ATTENTION>> bias voltage registers 17,20,21,22 will not be set for nXYTer version 2!"RES"\n");
          printf (BLU"            these registers are replaced with external dacs  "RES"\n");
          l_nxy_v2 = 1;  
          #endif

          for (l_l=0; l_l<14; l_l++)
          {
            if ( (l_nxy_v2 == 0) || ((l_nxy_v2 == 1) && (l_l != 1) && (l_l != 4) && (l_l != 5) && (l_l != 6))) 
            {
              //printf ("inside  l_l: %d \n", l_l);

            l_wr_d  = l_bias[l_i][l_j][l_k][l_l];
            l_wr_d += ((l_l+0x10)                 <<  8);
            l_wr_d += (l_i2c_addr[l_i][l_j][l_k]  << 16);
            l_wr_d += (I2C_COTR_A                 << 24);

            l_stat = f_pex_slave_wr (l_i, l_j, GOS_I2C_DWR, l_wr_d);
            if (l_stat == -1)
            {
                printf (RED"ERROR>> setting nXYter bias failed: "RES"\n"); 
              printf ("        SFP: %d, NYX: %d, nXY: %d => %3d  A: 0x%x, D: 0x%x\n",
                                    l_i, l_j, l_k, l_l, GOS_I2C_DWR, l_wr_d); 
              printf ("exiting.. \n");
              exit (0);  
            }
            else
            {
              debug3 (("set SFP: %d, NYX: %d, nXY: %d => %3d  A: 0x%x, D: 0x%x\n",
                                    l_i, l_j, l_k, l_l, GOS_I2C_DWR, l_wr_d)); 
            }
          }   
          }    
          debug3 (("\n"));

          // set nXYter configuration registers
          for (l_l=0; l_l<2; l_l++)
          {
            l_wr_d  = l_config[l_i][l_j][l_k][l_l];
            l_wr_d += ((l_l+0x20)                   <<  8);
            l_wr_d += (l_i2c_addr[l_i][l_j][l_k]    << 16);
            l_wr_d += (I2C_COTR_A                   << 24);

            l_stat = f_pex_slave_wr (l_i, l_j, GOS_I2C_DWR, l_wr_d);
            if (l_stat == -1)
            {
              printf (RED"ERROR>> setting nXYter configuration failed: "RES"\n"); 
              printf ("        SFP: %d, NYX: %d, nXY: %d => %3d  A: 0x%x, D: 0x%x\n",
                                    l_i, l_j, l_k, l_l, GOS_I2C_DWR, l_wr_d); 
              printf ("exiting.. \n");
              exit (0);  
            }
            else
            {
              debug3 (("set SFP: %d, NYX: %d, nXY: %d => %3d  A: 0x%x, D: 0x%x\n",
                                    l_i, l_j, l_k, l_l, GOS_I2C_DWR, l_wr_d)); 
            }
          }
          debug3 (("\n"));

          // set nXYter test pulse - and test trigger delay registers
          for (l_l=0; l_l<2; l_l++)
          {
            l_wr_d  = l_te_del[l_i][l_j][l_k][l_l];
            l_wr_d += ((l_l+0x26)                  <<  8);
            l_wr_d += (l_i2c_addr[l_i][l_j][l_k]   << 16);
            l_wr_d += (I2C_COTR_A                  << 24);

            l_stat = f_pex_slave_wr (l_i, l_j, GOS_I2C_DWR, l_wr_d);
            if (l_stat == -1)
            {
              printf (RED"ERROR>> setting nXYter test pulse - and test trigger delay failed: "RES"\n"); 
              printf ("        SFP: %d, NYX: %d, nXY: %d => %3d  A: 0x%x, D: 0x%x\n",
                                    l_i, l_j, l_k, l_l, GOS_I2C_DWR, l_wr_d); 
              printf ("exiting.. \n");
              exit (0);  
            }
            else
            {
              debug3 (("set SFP: %d, NYX: %d, nXY: %d => %3d  A: 0x%x, D: 0x%x\n",
                                    l_i, l_j, l_k, l_l, GOS_I2C_DWR, l_wr_d)); 
            }
          }
          debug3 (("\n"));   

          // set test channel threshold
          l_wr_d  = l_thr_te[l_i][l_j][l_k];
          l_wr_d += 0x2a                       <<  8;
          l_wr_d += (l_i2c_addr[l_i][l_j][l_k] << 16);
          l_wr_d += (I2C_COTR_A                << 24);

          l_stat = f_pex_slave_wr (l_i, l_j, GOS_I2C_DWR, l_wr_d);
          if (l_stat == -1)
          {
            printf (RED"ERROR>> setting nXYter test channel treshhold failed: "RES"\n"); 
            printf ("        SFP: %d, NYX: %d, nXY: %d => %3d  A: 0x%x, D: 0x%x\n",
                                  l_i, l_j, l_k, l_l, GOS_I2C_DWR, l_wr_d); 
            printf ("exiting.. \n");
            exit (0);  
          }
          else
          {
            debug3 (("set SFP: %d, NYX: %d, nXY: %d => %3d  A: 0x%x, D: 0x%x\n",
                                    l_i, l_j, l_k, l_l, GOS_I2C_DWR, l_wr_d)); 
          }
          debug3 (("\n"));

          // set 128 channel thresholds
          for (l_l=0; l_l<128; l_l++)
          {
            l_wr_d  = l_thr[l_i][l_j][l_k][l_l];
            l_wr_d += 0x2a                       <<  8;
            l_wr_d += (l_i2c_addr[l_i][l_j][l_k] << 16);
            l_wr_d += (I2C_COTR_A                << 24);

            l_stat = f_pex_slave_wr (l_i, l_j, GOS_I2C_DWR, l_wr_d);
            if (l_stat == -1)
            {
              printf (RED"ERROR>> setting nXYter channel thresholds failed: "RES"\n"); 
              printf ("        SFP: %d, NYX: %d, nXY: %d => %3d  A: 0x%x, D: 0x%x\n",
                                    l_i, l_j, l_k, l_l, GOS_I2C_DWR, l_wr_d); 
              printf ("exiting.. \n");
              exit (0);  
            }
            else
            {
              debug3 (("set SFP: %d, NYX: %d, nXY: %d => %3d  A: 0x%x, D: 0x%x\n",
                                    l_i, l_j, l_k, l_l, GOS_I2C_DWR, l_wr_d)); 
            }
          }
          debug3 (("\n"));

          // set clock delay registers
          for (l_l=0; l_l<3; l_l++)
          {
            l_wr_d  = l_clk_del[l_i][l_j][l_k][l_l];
            l_wr_d += ((l_l+0x2b)                 <<  8);
            l_wr_d += (l_i2c_addr[l_i][l_j][l_k]  << 16);
            l_wr_d += (I2C_COTR_A                 << 24);

            l_stat = f_pex_slave_wr (l_i, l_j, GOS_I2C_DWR, l_wr_d);
            if (l_stat == -1)
            {
              printf (RED"ERROR>> setting nXYter clock delays failed: "RES"\n"); 
              printf ("        SFP: %d, NYX: %d, nXY: %d => %3d  A: 0x%x, D: 0x%x\n",
                                    l_i, l_j, l_k, l_l, GOS_I2C_DWR, l_wr_d); 
              printf ("exiting.. \n");
              exit (0);  
            }
            else
            {
              debug3 (("set SFP: %d, NYX: %d, nXY: %d => %3d  A: 0x%x, D: 0x%x\n",
                                    l_i, l_j, l_k, l_l, GOS_I2C_DWR, l_wr_d)); 
            }
          }
          debug3 (("\n"));   

          // de-activate I2C core
          l_wr_d  = 0;
          l_wr_d += 0x0              <<  8;
          l_wr_d += 0x0              << 16;
          l_wr_d += I2C_CTRL_A       << 24;

          l_stat = f_pex_slave_wr (l_i, l_j, GOS_I2C_DWR, l_wr_d);
          if (l_stat == -1)
          {
            printf (RED"ERROR>> de-activating I2C core failed: "RES"\n"); 
            printf ("        SFP: %d, NYX: %d => A: 0x%x, D: 0x%x\n",
                                 l_i, l_j, GOS_I2C_DWR, l_wr_d);
            printf ("exiting.. \n");
            exit (0);  
          }
          else
          {
            debug3 (("set SFP: %d, NYX: %d         =>      A: 0x%x, D: 0x%x\n",
                                 l_i, l_j, GOS_I2C_DWR, l_wr_d)); 
            debug3 (("\n"));
          }

          // setup ADC: do it for each nXYter, although only one ADC is used at the moment
          //            for one or two nXYter boards. this means, the last nXYter setting
          //            per GEMEX/NYXOR wins. 
          printf ("setup SFP: %d, NYX: %d, nXY: %d ADC DCO phase\n", l_i, l_j, l_k);

          // activate SPI sub core on GEMEX/NYXOR  
          l_wr_d = 0x11000080;  
          l_stat = f_pex_slave_wr (l_i, l_j, GOS_I2C_DWR, l_wr_d);
          if (l_stat == -1)
          {
            printf (RED"ERROR>> activating SPI subcore failed: "RES"\n"); 
            printf ("        SFP: %d, NYX: %d => A: 0x%x, D: 0x%x\n",
                                 l_i, l_j, GOS_I2C_DWR, l_wr_d);
            printf ("exiting.. \n");
            exit (0);  
          }
          else
          {
            debug3 (("set SFP: %d, NYX: %d         =>      A: 0x%x, D: 0x%x\n",
                                 l_i, l_j, GOS_I2C_DWR, l_wr_d)); 
          }

          // set speed on SPI sub core
          l_wr_d = 0x12000044;  
          l_stat = f_pex_slave_wr (l_i, l_j, GOS_I2C_DWR, l_wr_d);
          if (l_stat == -1)
          {
            printf (RED"ERROR>> setting speed of SPI subcore failed: "RES"\n"); 
            printf ("        SFP: %d, NYX: %d => A: 0x%x, D: 0x%x\n",
                                 l_i, l_j, GOS_I2C_DWR, l_wr_d);
            printf ("exiting.. \n");
            exit (0);  
          }
          else
          {
            debug3 (("set SFP: %d, NYX: %d         =>      A: 0x%x, D: 0x%x\n",
                                 l_i, l_j, GOS_I2C_DWR, l_wr_d)); 
          }

          // adjust ADC DCO  phase
          l_wr_d = 0x15001600 | l_adc_dco_phase[l_i][l_j][l_k];  
          l_stat = f_pex_slave_wr (l_i, l_j, GOS_I2C_DWR, l_wr_d);
          if (l_stat == -1)
          {
            printf (RED"ERROR>> setting ADC DCO phase failed: "RES"\n"); 
            printf ("        SFP: %d, NYX: %d => A: 0x%x, D: 0x%x\n",
                                 l_i, l_j, GOS_I2C_DWR, l_wr_d);
            printf ("exiting.. \n");
            exit (0);  
          }
          else
          {
            debug3 (("set SFP: %d, NYX: %d         =>      A: 0x%x, D: 0x%x\n",
                                 l_i, l_j, GOS_I2C_DWR, l_wr_d));
          } 

          // de-activate SPI sub core
          l_wr_d = 0x11000000;  
          l_stat = f_pex_slave_wr (l_i, l_j, GOS_I2C_DWR, l_wr_d);
          if (l_stat == -1)
          {
            printf (RED"ERROR>> de-activating SPI subcore failed: "RES"\n"); 
            printf ("        SFP: %d, NYX: %d => A: 0x%x, D: 0x%x\n",
                                 l_i, l_j, GOS_I2C_DWR, l_wr_d);
            printf ("exiting.. \n");
            exit (0);  
          }
          else
          {
            debug3 (("set SFP: %d, NYX: %d         =>      A: 0x%x, D: 0x%x\n",
                                 l_i, l_j, GOS_I2C_DWR, l_wr_d)); 
          }
        }

        l_wr_d = 0x7e000000;  
        l_stat = f_pex_slave_wr (l_i, l_j, GOS_I2C_DWR, l_wr_d);
        if (l_stat == -1)
        {
          printf (RED"ERROR>> GEMEX/NYXOR reset failed: "RES"\n"); 
          printf ("        SFP: %d, NYX: %d => A: 0x%x, D: 0x%x\n",
                                 l_i, l_j, GOS_I2C_DWR, l_wr_d);
          printf ("exiting.. \n");
          exit (0);  
        }
        else
        {
          debug3 (("set SFP: %d, NYX: %d         =>      A: 0x%x, D: 0x%x\n",
                                 l_i, l_j, GOS_I2C_DWR, l_wr_d)); 
        }

        printf ("setup SFP: %d, NYX: %d         registers\n", l_i, l_j);
        // activate clock reset
        l_wr_d = 0x21000001;  
        l_stat = f_pex_slave_wr (l_i, l_j, GOS_I2C_DWR, l_wr_d);
        if (l_stat == -1)
        {
          printf (RED"ERROR>> activating clock reset failed: "RES"\n"); 
          printf ("        SFP: %d, NYX: %d => A: 0x%x, D: 0x%x\n",
                                 l_i, l_j, GOS_I2C_DWR, l_wr_d);
          printf ("exiting.. \n");
          exit (0);  
        }
        else
        {
          debug3 (("set SFP: %d, NYX: %d         =>      A: 0x%x, D: 0x%x\n",
                                 l_i, l_j, GOS_I2C_DWR, l_wr_d)); 
        }

        // setup pre and post trigger window
        l_wr_d = 0x22000000 + l_pre_trg_wind;
        l_stat = f_pex_slave_wr (l_i, l_j, GOS_I2C_DWR, l_wr_d);
        if (l_stat == -1)
        {
          printf (RED"ERROR>> setting pre trigger window failed: "RES"\n"); 
          printf ("        SFP: %d, NYX: %d => A: 0x%x, D: 0x%x\n",
                                 l_i, l_j, GOS_I2C_DWR, l_wr_d);
          printf ("exiting.. \n");
          exit (0);  
        }
        else
        {
          debug3 (("set SFP: %d, NYX: %d         =>      A: 0x%x, D: 0x%x\n",
                                 l_i, l_j, GOS_I2C_DWR, l_wr_d)); 
        }

        l_wr_d = 0x23000000 + l_pos_trg_wind; 
        l_stat = f_pex_slave_wr (l_i, l_j, GOS_I2C_DWR, l_wr_d);
        if (l_stat == -1)
        {
          printf (RED"ERROR>> setting post trigger window failed: "RES"\n"); 
          printf ("        SFP: %d, NYX: %d => A: 0x%x, D: 0x%x\n",
                                 l_i, l_j, GOS_I2C_DWR, l_wr_d);
          printf ("exiting.. \n");
          exit (0);  
        }
        else
        {
          debug3 (("set SFP: %d, NYX: %d         =>      A: 0x%x, D: 0x%x\n",
                                 l_i, l_j, GOS_I2C_DWR, l_wr_d)); 
        }

        // setup test pulse delay
        l_wr_d = 0x24000000 + l_test_pul_del;
        l_stat = f_pex_slave_wr (l_i, l_j, GOS_I2C_DWR, l_wr_d);
        if (l_stat == -1)
        {
          printf (RED"ERROR>> setting test pulse delay failed: "RES"\n"); 
          printf ("        SFP: %d, NYX: %d => A: 0x%x, D: 0x%x\n",
                                 l_i, l_j, GOS_I2C_DWR, l_wr_d);
          printf ("exiting.. \n");
          exit (0);  
        }
        else
        {
          debug3 (("set SFP: %d, NYX: %d         =>      A: 0x%x, D: 0x%x\n",
                                 l_i, l_j, GOS_I2C_DWR, l_wr_d)); 
        }

        // setup test trigger delay 
        l_wr_d = 0x25000000 + l_test_trg_del;
        l_stat = f_pex_slave_wr (l_i, l_j, GOS_I2C_DWR, l_wr_d);
        if (l_stat == -1)
        {
          printf (RED"ERROR>> setting test trigger delay failed: "RES"\n"); 
          printf ("        SFP: %d, NYX: %d => A: 0x%x, D: 0x%x\n",
                                 l_i, l_j, GOS_I2C_DWR, l_wr_d);
          printf ("exiting.. \n");
          exit (0);  
        }
        else
        {
          debug3 (("set SFP: %d, NYX: %d         =>      A: 0x%x, D: 0x%x\n",
                                 l_i, l_j, GOS_I2C_DWR, l_wr_d)); 
        }

        #ifdef NXYTER_V2
        // setup external dacs
        debug3 (("\n")); 
        printf ("begin setup external dacs \n"); 
        for (l_k=0; l_k<l_in_use[l_i][l_j]; l_k++)  // loop over nXYter 
        {
          // activate i2c to set external dacs
          l_wr_d = 0x1000080 + l_k;
          l_stat = f_pex_slave_wr (l_i, l_j, GOS_I2C_DWR, l_wr_d);
          if (l_stat == -1)
          {
            printf (RED"ERROR>> activating i2c for external dac setting: "RES"\n"); 
            printf ("        SFP: %d, NYX: %d => A: 0x%x, D: 0x%x\n", l_i, l_j, GOS_I2C_DWR, l_wr_d);
            printf ("exiting.. \n");
            exit (0);  
          }
          else
          {
            debug3 (("set SFP: %d, NYX: %d         =>      A: 0x%x, D: 0x%x\n", l_i, l_j, GOS_I2C_DWR, l_wr_d)); 
          }
          for (l_l=0; l_l<4; l_l++)
          { 
            //write external dac values
            l_wr_d = 0xb430000 + (0x100000 * l_k) + (0x1000 << l_l) + l_ext_dac[l_i][l_j][l_k][l_l];
            l_stat = f_pex_slave_wr (l_i, l_j, GOS_I2C_DWR, l_wr_d);
            if (l_stat == -1)
            {
              printf (RED"ERROR>> setting external dac failed: "RES"\n"); 
              printf ("        SFP: %d, NYX: %d, nXY: %d => %3d  A: 0x%x, D: 0x%x\n", l_i, l_j, l_k, l_l, GOS_I2C_DWR, l_wr_d); 
              printf ("exiting.. \n");
              exit (0);  
            }
            else
            {
              debug3 (("set SFP: %d, NYX: %d, nXY: %d => %3d  A: 0x%x, D: 0x%x\n", l_i, l_j, l_k, l_l, GOS_I2C_DWR, l_wr_d)); 
            }
          } 
        }
        // de-activate both i2c used for external dac setting
        l_wr_d = 0x1000000;
        l_stat = f_pex_slave_wr (l_i, l_j, GOS_I2C_DWR, l_wr_d);
        if (l_stat == -1)
        {
          printf (RED"ERROR>> de-activating i2c for external dac setting: "RES"\n"); 
          printf ("        SFP: %d, NYX: %d => A: 0x%x, D: 0x%x\n", l_i, l_j, GOS_I2C_DWR, l_wr_d);
          printf ("exiting.. \n");
          exit (0);  
        }
        else
        {
          debug3 (("set SFP: %d, NYX: %d         =>      A: 0x%x, D: 0x%x\n", l_i, l_j, GOS_I2C_DWR, l_wr_d)); 
        }
        printf ("end setup external dacs \n"); 
        #endif // NXYTER_V2

        // setup nxyter control register (once per nyxor)
        debug3 (("\n")); 
        // some checks
        if ( (l_in_use[l_i][l_j] == 2) && (((l_nxy_ctrl[l_i][l_j]>>3) & 0x1) == 0))
        {  
          printf (BLU"ATTENTION>> contradiction with # nxyters in use and nXYTer ctrl register content "RES"\n");
          printf ("SFP: %d, NYX: %d, # nXYTer in use:   %d \n", l_i, l_j, l_in_use[l_i][l_j]);
          printf ("                  nXYTer ctrl:   0x%x \n", l_nxy_ctrl[l_i][l_j]);
          printf ("change nXYTer ctrl from 0x%x to  0x%x ", l_nxy_ctrl[l_i][l_j], l_nxy_ctrl[l_i][l_j] | 0x8);
          printf ("and activate both receivers \n");
          l_nxy_ctrl[l_i][l_j] = l_nxy_ctrl[l_i][l_j] | 0x8;
        }
        if ( (l_in_use[l_i][l_j] == 1) && (((l_nxy_ctrl[l_i][l_j]>>3) & 0x1) == 1))
        {  
          printf (BLU"ATTENTION>> contradiction with # nxyters in use and nXYTer ctrl register content "RES"\n");
          printf ("SFP: %d, NYX: %d, # nXYTer in use:   %d \n", l_i, l_j, l_in_use[l_i][l_j]);
          printf ("                  nXYTer ctrl:   0x%x ", l_nxy_ctrl[l_i][l_j]);
          printf ("and activate only one receiver \n");
          printf ("change nXYTer ctrl from 0x%x to  0x%x \n",  l_nxy_ctrl[l_i][l_j], l_nxy_ctrl[l_i][l_j] & 0xf7);
          l_nxy_ctrl[l_i][l_j] = l_nxy_ctrl[l_i][l_j] & 0xf7;
        }

        // activate nxyter receiver(s) (set nXYTer ctrl register)
        l_wr_d = 0x21000000 + l_nxy_ctrl[l_i][l_j];
        l_stat = f_pex_slave_wr (l_i, l_j, GOS_I2C_DWR, l_wr_d);
        if (l_stat == -1)
        {
          printf (RED"ERROR>> activating nXYter receiver failed: "RES"\n"); 
          printf ("        SFP: %d, NYX: %d => A: 0x%x, D: 0x%x\n",
                                 l_i, l_j, GOS_I2C_DWR, l_wr_d);
          printf ("exiting.. \n");
          exit (0);  
        }
        else
        {
          debug3 (("set SFP: %d, NYX: %d         =>      A: 0x%x, D: 0x%x\n",
                                 l_i, l_j, GOS_I2C_DWR, l_wr_d)); 
        }
        //sleep (1);
      }
    }  
  }

  printf ("...setup done \n");


  // read back GEMEX/NYXOR and nXYter settings
  printf ("\nread back setup... \n");
  for (l_i=0; l_i<MAX_SFP; l_i++)          // loop over SFPs
  {
    if (l_in_use[l_i][0] > 0)              // sfp l_i used
    {
      for (l_j=0; l_j<MAX_NYX; l_j++)      // loop over GEMEX/NYXOR
      {
        debug3 (("\n\n"));
        if (l_in_use[l_i][l_j] == 0) {break;}  
        for (l_k=0; l_k<l_in_use[l_i][l_j]; l_k++)  // loop over nXYter
        {
          printf ("read back SFP: %d, NYX: %d, nXY: %d registers\n", l_i, l_j, l_k);
          // activate I2C core and don't reset nXYter
          l_wr_d  = l_reset[l_i][l_j][l_k] - 4;  
          //l_wr_d  = 0x80;
          l_wr_d += 0x0              <<  8;
          l_wr_d += 0x0              << 16;
          l_wr_d += I2C_CTRL_A       << 24;

          l_stat = f_pex_slave_wr (l_i, l_j, GOS_I2C_DWR, l_wr_d);
          if (l_stat == -1)
          {
            printf (RED"ERROR>> activating I2C core failed: "RES"\n"); 
            printf ("        SFP: %d, NYX: %d => A: 0x%x, D: 0x%x\n",
                                 l_i, l_j, GOS_I2C_DWR, l_wr_d);
            printf ("exiting.. \n");
            exit (0);  
          }
          else
          {
            debug3 (("set SFP: %d, NYX: %d           =>      A: 0x%x, D: 0x%x\n",
                                 l_i, l_j, GOS_I2C_DWR, l_wr_d)); 
            debug3 (("\n"));
          }

          /*         
          // read back nXYter mask registers
          for (l_l=0; l_l<16; l_l++)
          {
            l_wr_d  =  l_l                             <<  8;
            l_wr_d += ((l_i2c_addr[l_i][l_j][l_k] + 1) << 16);
            l_wr_d += (I2C_COTR_A                      << 24);

            l_stat = f_pex_slave_wr (l_i, l_j, GOS_I2C_DWR, l_wr_d);
            debug3 (("write SFP: %1d, NYX: %1d, nXY: %1d => %3d  A: 0x%x, D: 0x%x\n",
                                    l_i, l_j, l_k, l_l, GOS_I2C_DWR, l_wr_d)); 
            l_stat = f_pex_slave_rd (l_i, l_j, GOS_I2C_DRR1, &l_data);           
            debug3 (("read                         => %3d  A: 0x%x, D: 0x%x\n",
                                                    l_l, GOS_I2C_DRR1, l_data)); 
            l_check_d  = CHECK_DAT;
            l_check_d += l_l << 8;
            l_check_d += l_mask[l_i][l_j][l_k][l_l];
            l_check_d += (l_i2c_addr[l_i][l_j][l_k]+1) << 16;

            if (l_check_d != l_data)
            {
              printf (RED"ERROR>> data error during read back of nXYter mask registers: "RES"\n");
              printf ("        SFP: %1d, NYX: %1d, nXY: %1d => %3d  expect: 0x%x, is: 0x%x \n",
                                       l_i, l_j, l_k, l_l, l_check_d, l_data);
              printf ("exiting.. \n");
              exit (0); 
            }
           }
          debug3 (("\n"));
          */
          printf (BLU"ATTENTION>> mask register not read back for all nXYTer versions"RES"\n");  

          // read back nXYter bias registers
          for (l_l=0; l_l<14; l_l++)
          {
            if ( (l_nxy_v2 == 0) || ((l_nxy_v2 == 1) && (l_l != 1) && (l_l != 4) && (l_l != 5) && (l_l != 6)))
            {
              //printf ("inside  l_l: %d \n", l_l);
            l_wr_d  = ((l_l+0x10)                      <<  8);
            l_wr_d += ((l_i2c_addr[l_i][l_j][l_k] + 1) << 16);
            l_wr_d += (I2C_COTR_A                      << 24);

            l_stat = f_pex_slave_wr (l_i, l_j, GOS_I2C_DWR, l_wr_d);
            debug3 (("write SFP: %1d, NYX: %1d, nXY: %1d => %3d  A: 0x%x, D: 0x%x\n",
                                    l_i, l_j, l_k, l_l, GOS_I2C_DWR, l_wr_d)); 
            
            l_stat = f_pex_slave_rd (l_i, l_j, GOS_I2C_DRR1, &l_data);           
            debug3 (("read                         => %3d  A: 0x%x, D: 0x%x\n",
                                                    l_l, GOS_I2C_DRR1, l_data)); 
            l_check_d  = CHECK_DAT;
            l_check_d += ((l_l+0x10) << 8);
            l_check_d += l_bias[l_i][l_j][l_k][l_l];
            l_check_d += (l_i2c_addr[l_i][l_j][l_k]+1) << 16;

            if (l_check_d != l_data)
            {
                printf (RED"ERROR>> data error during read back of nXYter bias registers: "RES"\n");
              printf ("        SFP: %1d, NYX: %1d, nXY: %1d => %3d  expect: 0x%x, is: 0x%x \n",
                                       l_i, l_j, l_k, l_l, l_check_d, l_data);
              printf ("exiting.. \n");
              exit (0); 
            }
          }   
          }   
          debug3 (("\n"));

          // read back nXYter configuration registers
          for (l_l=0; l_l<2; l_l++)
          {
            l_wr_d  = ((l_l+0x20)                      <<  8);
            l_wr_d += ((l_i2c_addr[l_i][l_j][l_k] + 1) << 16);
            l_wr_d += (I2C_COTR_A                      << 24);

            l_stat = f_pex_slave_wr (l_i, l_j, GOS_I2C_DWR, l_wr_d);
            debug3 (("write SFP: %1d, NYX: %1d, nXY: %1d => %3d  A: 0x%x, D: 0x%x\n",
                                    l_i, l_j, l_k, l_l, GOS_I2C_DWR, l_wr_d)); 
            
            l_stat = f_pex_slave_rd (l_i, l_j, GOS_I2C_DRR1, &l_data);           
            debug3 (("read                         => %3d  A: 0x%x, D: 0x%x\n",
                                                    l_l, GOS_I2C_DRR1, l_data)); 
            l_check_d  = CHECK_DAT;
            l_check_d += ((l_l+0x20) << 8);
            l_check_d += l_config[l_i][l_j][l_k][l_l];
            l_check_d += (l_i2c_addr[l_i][l_j][l_k]+1) << 16;

            if (l_check_d != l_data)
            {
              printf (RED"ERROR>> data error during read back of nXYter configuration registers: "RES"\n");
              printf ("        SFP: %1d, NYX: %1d, nXY: %1d => %3d  expect: 0x%x, is: 0x%x \n",
                                       l_i, l_j, l_k, l_l, l_check_d, l_data);
              printf ("exiting.. \n");
              exit (0); 
            }
          }
          debug3 (("\n"));

          // read back nXYter test pulse - and test trigger delay registers
          for (l_l=0; l_l<2; l_l++)
          {
            l_wr_d  = ((l_l+0x26)                      <<  8);
            l_wr_d += ((l_i2c_addr[l_i][l_j][l_k] + 1) << 16);
            l_wr_d += (I2C_COTR_A                      << 24);

            l_stat = f_pex_slave_wr (l_i, l_j, GOS_I2C_DWR, l_wr_d);
            debug3 (("write SFP: %1d, NYX: %1d, nXY: %1d => %3d  A: 0x%x, D: 0x%x\n",
                                    l_i, l_j, l_k, l_l, GOS_I2C_DWR, l_wr_d)); 
            
            l_stat = f_pex_slave_rd (l_i, l_j, GOS_I2C_DRR1, &l_data);           
            debug3 (("read                         => %3d  A: 0x%x, D: 0x%x\n",
                                                    l_l, GOS_I2C_DRR1, l_data)); 
            l_check_d  = CHECK_DAT;
            l_check_d += ((l_l+0x26) << 8);
            l_check_d += l_te_del[l_i][l_j][l_k][l_l];
            l_check_d += (l_i2c_addr[l_i][l_j][l_k]+1) << 16;

            if (l_check_d != l_data)
            {
              printf (RED"ERROR>> data error during read back of nXYter test pulse /trigger regs: "RES"\n");
              printf ("        SFP: %1d, NYX: %1d, nXY: %1d => %3d  expect: 0x%x, is: 0x%x \n",
                                       l_i, l_j, l_k, l_l, l_check_d, l_data);
              printf ("exiting.. \n");
              exit (0); 
            }
          }
          debug3 (("\n"));   


          // don't read back thresholds (nXYter register 0x2a) shift register !

          // read back clock delay registers
          for (l_l=0; l_l<3; l_l++)
          {
            l_wr_d  = ((l_l+0x2b)                      <<  8);
            l_wr_d += ((l_i2c_addr[l_i][l_j][l_k] + 1) << 16);
            l_wr_d += (I2C_COTR_A                      << 24);

            l_stat = f_pex_slave_wr (l_i, l_j, GOS_I2C_DWR, l_wr_d);
            debug3 (("write SFP: %1d, NYX: %1d, nXY: %1d => %3d  A: 0x%x, D: 0x%x\n",
                                    l_i, l_j, l_k, l_l, GOS_I2C_DWR, l_wr_d)); 
            
            l_stat = f_pex_slave_rd (l_i, l_j, GOS_I2C_DRR1, &l_data);           
            debug3 (("read                         => %3d  A: 0x%x, D: 0x%x\n",
                                                    l_l, GOS_I2C_DRR1, l_data)); 
            l_check_d  = CHECK_DAT;
            l_check_d += ((l_l+0x2b) << 8);
            l_check_d += l_clk_del[l_i][l_j][l_k][l_l];
            l_check_d += (l_i2c_addr[l_i][l_j][l_k]+1) << 16;

            if (l_check_d != l_data)
            {
              printf (RED"ERROR>> data error during read back of nXYter clock delay regs: "RES"\n");
              printf ("        SFP: %1d, NYX: %1d, nXY: %1d => %3d  expect: 0x%x, is: 0x%x \n",
                                       l_i, l_j, l_k, l_l, l_check_d, l_data);
              printf ("exiting.. \n");
              exit (0); 
            }
          }
          debug3 (("\n"));   

          // de-activate I2C core
          l_wr_d  = 0;
          l_wr_d += 0x0              <<  8;
          l_wr_d += 0x0              << 16;
          l_wr_d += I2C_CTRL_A       << 24;

          l_stat = f_pex_slave_wr (l_i, l_j, GOS_I2C_DWR, l_wr_d);
          if (l_stat == -1)
          {
            printf (RED"ERROR>> de-activating I2C core failed: "RES"\n"); 
            printf ("        SFP: %d, NYX: %d => A: 0x%x, D: 0x%x\n",
                                 l_i, l_j, GOS_I2C_DWR, l_wr_d);
            printf ("exiting.. \n");
            exit (0);  
          }
          else
          {
            debug3 (("set   SFP: %d, NYX: %d         =>      A: 0x%x, D: 0x%x\n",
                                 l_i, l_j, GOS_I2C_DWR, l_wr_d)); 
            debug3 (("\n"));
          }

        }

        printf ("read back SFP: %d, NYX: %d         registers \n", l_i, l_j);
        // read back control register
        l_wr_d = 0xa1000000;  
        l_stat = f_pex_slave_wr (l_i, l_j, GOS_I2C_DWR, l_wr_d);
        debug3 (("write SFP: %1d, NYX: %1d         =>      A: 0x%x, D: 0x%x\n", l_i, l_j, GOS_I2C_DWR, l_wr_d)); 
        l_stat = f_pex_slave_rd (l_i, l_j, GOS_I2C_DRR1, &l_data);           
        debug3 (("read                         =>      A: 0x%x, D: 0x%x\n", GOS_I2C_DRR1, l_data)); 
        l_check_d =  0x89a10000 + l_nxy_ctrl[l_i][l_j];
        if (l_check_d != l_data)
        {
          printf (RED"ERROR>> data error during read back GEMEX/NYXOR control register "RES"\n");
          printf ("        SFP: %1d, NYX: %1d => expect: 0x%x, is: 0x%x \n", l_i, l_j, l_check_d, l_data);
          printf ("exiting.. \n");
          exit (0); 
        }

        // read back pre and post trigger window
        l_wr_d = 0xa2000000;  
        l_stat = f_pex_slave_wr (l_i, l_j, GOS_I2C_DWR, l_wr_d);
        debug3 (("write SFP: %1d, NYX: %1d         =>      A: 0x%x, D: 0x%x\n", l_i, l_j, GOS_I2C_DWR, l_wr_d)); 
        l_stat = f_pex_slave_rd (l_i, l_j, GOS_I2C_DRR1, &l_data);           
        debug3 (("read                         =>      A: 0x%x, D: 0x%x\n", GOS_I2C_DRR1, l_data)); 
        l_check_d  = 0x89a20000 + l_pre_trg_wind;
        if (l_check_d != l_data)
        {
          printf (RED"ERROR>> data error during read back GEMEX/NYXOR pre trigger window: "RES"\n");
          printf ("        SFP: %1d, NYX: %1d => expect: 0x%x, is: 0x%x \n", l_i, l_j, l_check_d, l_data);
          printf ("exiting.. \n");
          exit (0); 
        }

        l_wr_d = 0xa3000000;  
        l_stat = f_pex_slave_wr (l_i, l_j, GOS_I2C_DWR, l_wr_d);
        debug3 (("write SFP: %1d, NYX: %1d         =>      A: 0x%x, D: 0x%x\n", l_i, l_j, GOS_I2C_DWR, l_wr_d)); 
        l_stat = f_pex_slave_rd (l_i, l_j, GOS_I2C_DRR1, &l_data);           
        debug3 (("read                         =>      A: 0x%x, D: 0x%x\n", GOS_I2C_DRR1, l_data)); 
        l_check_d  = 0x89a30000 + l_pos_trg_wind;
        if (l_check_d != l_data)
        {
          printf (RED"ERROR>> data error during read back GEMEX/NYXOR post trigger window: "RES"\n");
          printf ("        SFP: %1d, NYX: %1d => expect: 0x%x, is: 0x%x \n", l_i, l_j, l_check_d, l_data);
          printf ("exiting.. \n");
          exit (0); 
        }
        // setup test pulse delay
        l_wr_d = 0xa4000000;
        l_stat = f_pex_slave_wr (l_i, l_j, GOS_I2C_DWR, l_wr_d);
        debug3 (("write SFP: %1d, NYX: %1d         =>      A: 0x%x, D: 0x%x\n", l_i, l_j, GOS_I2C_DWR, l_wr_d)); 
        l_stat = f_pex_slave_rd (l_i, l_j, GOS_I2C_DRR1, &l_data);           
        debug3 (("read                         =>      A: 0x%x, D: 0x%x\n", GOS_I2C_DRR1, l_data)); 
        l_check_d  = 0x89a40000 + l_test_pul_del;
        if (l_check_d != l_data)
        {
          printf (RED"ERROR>> data error during read back GEMEX/NYXOR test pulse delay: "RES"\n");
          printf ("        SFP: %1d, NYX: %1d => expect: 0x%x, is: 0x%x \n", l_i, l_j, l_check_d, l_data);
          printf ("exiting.. \n");
          exit (0); 
        }

        // setup test trigger delay 
        l_wr_d = 0xa5000000;  
        l_stat = f_pex_slave_wr (l_i, l_j, GOS_I2C_DWR, l_wr_d);
        debug3 (("write SFP: %1d, NYX: %1d         =>      A: 0x%x, D: 0x%x\n", l_i, l_j, GOS_I2C_DWR, l_wr_d)); 
        l_stat = f_pex_slave_rd (l_i, l_j, GOS_I2C_DRR1, &l_data);           
        debug3 (("read                         =>      A: 0x%x, D: 0x%x\n", GOS_I2C_DRR1, l_data)); 
        l_check_d  = 0x89a50000 + l_test_trg_del;
        if (l_check_d != l_data)
        {
          printf (RED"ERROR>> data error during read back GEMEX/NYXOR test trigger delay: "RES"\n");
          printf ("        SFP: %1d, NYX: %1d => expect: 0x%x, is: 0x%x \n", l_i, l_j, l_check_d, l_data);
          printf ("exiting.. \n");
          exit (0); 
        }

        #ifdef NXYTER_V2
        // read back external dacs
        debug3 (("\n"));
        printf ("begin read back external dacs \n");  
        for (l_k=0; l_k<l_in_use[l_i][l_j]; l_k++)  // loop over nXYter 
        {
          // activate i2c to read external dacs
          l_wr_d = 0x1000080 + l_k;
          l_stat = f_pex_slave_wr (l_i, l_j, GOS_I2C_DWR, l_wr_d);
          if (l_stat == -1)
          {
            printf (RED"ERROR>> activating i2c for external dac reading: "RES"\n"); 
            printf ("        SFP: %d, NYX: %d => A: 0x%x, D: 0x%x\n", l_i, l_j, GOS_I2C_DWR, l_wr_d);
            printf ("exiting.. \n");
            exit (0);  
          }
          else
          {
            debug3 (("set SFP: %d, NYX: %d           =>      A: 0x%x, D: 0x%x\n", l_i, l_j, GOS_I2C_DWR, l_wr_d)); 
          }
          // read back external dacs
          for (l_l=0; l_l<4; l_l++)
          {
            l_wr_d = 0xbc00000 + (0x100000 * l_k) + (0x1000 << l_l);
            l_stat = f_pex_slave_wr (l_i, l_j, GOS_I2C_DWR, l_wr_d);
            debug3 (("write SFP: %1d, NYX: %1d, nXY: %1d => %3d  A: 0x%x, D: 0x%x\n",
                                    l_i, l_j, l_k, l_l, GOS_I2C_DWR, l_wr_d));
            l_stat = f_pex_slave_wr (l_i, l_j, GOS_I2C_DWR, 0x84000000);
            debug3 (("write SFP: %1d, NYX: %1d, nXY: %1d => %3d  A: 0x%x, D: 0x%x\n",
                                    l_i, l_j, l_k, l_l, GOS_I2C_DWR, 0x84000000));
            
            l_stat = f_pex_slave_rd (l_i, l_j, GOS_I2C_DRR1, &l_data);           
            debug3 (("read                         => %3d  A: 0x%x, D: 0x%x\n",
                                                    l_l, GOS_I2C_DRR1, l_data)); 
            l_data    = (l_data >> 6) & 0x3ff;
            l_check_d = l_ext_dac[l_i][l_j][l_k][l_l];

            if (l_check_d != l_data)
            {
              printf (RED"ERROR>> data error during read back of external dac registers: "RES"\n");
              printf ("        SFP: %1d, NYX: %1d, nXY: %1d => %3d  expect: 0x%x, is: 0x%x \n",
                                       l_i, l_j, l_k, l_l, l_check_d, l_data);
              printf ("exiting.. \n");
              exit (0); 
            }
          }
        }
        // de-activate both i2c used for external dac setting
        l_wr_d = 0x1000000;
        l_stat = f_pex_slave_wr (l_i, l_j, GOS_I2C_DWR, l_wr_d);
        if (l_stat == -1)
        {
          printf (RED"ERROR>> de-activating i2c for external dac reading: "RES"\n"); 
          printf ("        SFP: %d, NYX: %d => A: 0x%x, D: 0x%x\n", l_i, l_j, GOS_I2C_DWR, l_wr_d);
          printf ("exiting.. \n");
          exit (0);  
        }
        else
        {
          debug3 (("set SFP: %d, NYX: %d           =>      A: 0x%x, D: 0x%x\n", l_i, l_j, GOS_I2C_DWR, l_wr_d)); 
        }
        printf ("end read back external dacs \n"); 
        #endif // NXYTER_V2
      }
    }  
  }
  printf ("...read back done \n");

  #ifdef NXYTER_V2
  printf (BLU"ATTENTION>> program version was for nXYTer version 2 (NYXOR) "RES"\n");  
  #else
  printf (BLU"ATTENTION>> program version was for nXYTer version 1 (GEMEX) "RES"\n");  
  #endif
}

/*****************************************************************************/

int f_pex_slave_init (long l_sfp, long l_n_slaves)
{
  #ifdef USE_MBSPEX_LIB
  return mbspex_slave_init (fd_pex, l_sfp, l_n_slaves);
  #else

  int  l_ret;
  long l_comm;

  printf ("initialize SFP chain %d \n", l_sfp);
  l_comm = PEXOR_INI_REQ | (0x1<<16+l_sfp);

  PEXOR_RX_Clear_Ch (&sPEXOR, l_sfp); 
  PEXOR_TX (&sPEXOR, l_comm, 0, l_n_slaves  - 1) ;
  for (l_j=1; l_j<=10; l_j++)
  {
    //printf ("SFP %d: try nr. %d \n", l_sfp, l_j);
    l_dat1 = 0; l_dat2 = 0; l_dat3 = 0;
    l_stat = PEXOR_RX (&sPEXOR, l_sfp, &l_dat1 , &l_dat2, &l_dat3);
    if ( (l_stat != -1) && (l_dat2 > 0) && (l_dat2<=32))
    {
      break;
    }
    #ifndef Linux 
    yield ();
    #else
    sched_yield ();
    #endif
  }
  l_ret = 0;
  if (l_stat == -1)
  {
    l_ret = -1;
    printf (RED"ERROR>> initialization of SFP chain %d failed. "RES"", l_sfp);
    printf ("no reply: 0x%x 0x%x 0x%x \n", l_dat1, l_dat2, l_dat3);
    printf ("exiting.. \n"); exit (0);
  }
  else
  {
    if (l_dat2 != 0)
    { 
      printf ("initialization for SFP chain %d done. \n", l_sfp),
      printf ("No of slaves : %d \n", l_dat2);
    }
    else
    {
      l_ret = -1;
      printf (RED"ERROR>> initialization of SFP chain %d failed."RES" ", l_sfp);
      printf ("no slaves found. \n"); 
      printf ("exiting.. \n"); exit (0);
    }
  }
  return (l_ret);
  #endif // (else) USE_MBSPEX_LIB
}


/*****************************************************************************/

int f_pex_slave_wr (long l_sfp, long l_slave, long l_slave_off, long l_dat)
{
  int  l_ret;
  #ifdef USE_MBSPEX_LIB
  l_ret = mbspex_slave_wr (fd_pex, l_sfp, l_slave, l_slave_off, l_dat);
  f_i2c_sleep ();
  return (l_ret);
  #else

  long l_comm;
  long l_addr;

  l_comm = PEXOR_PT_AD_W_REQ | (0x1<<16+l_sfp);
  l_addr = l_slave_off + (l_slave << 24);
  PEXOR_RX_Clear_Ch (&sPEXOR, l_sfp); 
  PEXOR_TX (&sPEXOR, l_comm, l_addr, l_dat);
  l_stat = PEXOR_RX (&sPEXOR, l_sfp, &l_dat1 , &l_dat2, &l_dat3); 

  l_ret = 0;   
  if (l_stat == -1)
  {
    l_ret = -1;
    #ifdef DEBUG
    printf (RED"ERROR>> writing to SFP: %d, slave id: %d, addr 0x%d "RES"\n",
                                                l_sfp, l_slave, l_slave_off);
    printf ("  no reply: 0x%x 0x%x 0x%x \n", l_dat1, l_dat2, l_dat3);
    #endif // DEBUG
  }
  else
  {
    // printf ("Reply to PEXOR from SFP: 0x%x ", l_sfp);
    if( (l_dat1 & 0xfff) == PEXOR_PT_AD_W_REP)
    {
      //printf ("SFP: %d, slave id: %d addr: 0x%x  \n",
      //                l_sfp, (l_dat2 & 0xf0000) >> 24, l_dat2 & 0xfffff);
      if ( (l_dat1 & 0x4000) != 0)
      {
        l_ret = -1;
        #ifdef DEBUG
        printf (RED"ERROR>> packet structure: command reply 0x%x "RES"\n", l_dat1);
        #endif // DEBUG
      }
    }
    else
    {
      l_ret = -1;
      #ifdef DEBUG
      printf (RED"ERROR>> writing to empty slave or wrong address: "RES"\n");
      printf ("  SFP: %d, slave id: %d, 0x%x addr: 0x%x,  command reply:  0x%x \n",
           l_sfp, l_slave, (l_addr & 0xf00000) >> 24 , l_addr & 0xfffff, l_dat1);
      #endif // DEBUG
    }
  }
  //usleep (1);             // nXYter I2C needs 500us
  f_i2c_sleep ();
  return (l_ret);
  #endif // (else) USE_MBSPEX_LIB
}

/*****************************************************************************/

int f_pex_slave_rd (long l_sfp, long l_slave, long l_slave_off, long *l_dat)
{
  int  l_ret;
  #ifdef USE_MBSPEX_LIB
  l_ret =  mbspex_slave_rd (fd_pex, l_sfp, l_slave, l_slave_off, l_dat);
  f_i2c_sleep ();
  return (l_ret);
  #else

  long l_comm;
  long l_addr;

  l_comm = PEXOR_PT_AD_R_REQ | (0x1<<16+l_sfp);
  l_addr = l_slave_off + (l_slave << 24);
  PEXOR_RX_Clear_Ch (&sPEXOR, l_sfp); 
  PEXOR_TX (&sPEXOR, l_comm, l_addr, 0);
  l_stat = PEXOR_RX (&sPEXOR, l_sfp, &l_dat1 , &l_dat2, l_dat); 
  //printf ("f_pex_slave_rd, l_dat: 0x%x, *l_dat: 0x%x \n", l_dat, *l_dat);

  l_ret = 0;
  if (l_stat == -1)
  {
    l_ret = -1;
    #ifdef DEBUG
    printf (RED"ERROR>> reading from SFP: %d, slave id: %d, addr 0x%d "RES"\n",
                                  l_sfp, l_slave, l_slave_off);
    printf ("  no reply: 0x%x 0x%x 0x%x \n", l_dat1, l_dat2, *l_dat);
    #endif // DEBUG
  }
  else
  {
    // printf ("Reply to PEXOR from SFP: 0x%x ", l_sfp);
    if( (l_dat1 & 0xfff) == PEXOR_PT_AD_R_REP)
    {
      //printf ("SFP: %d, slave id: %d addr: 0x%x  \n",
      //     l_sfp, (l_dat2 & 0xf00000) >> 24, l_dat2 & 0xfffff);
      if ( (l_dat1 & 0x4000) != 0)
      {
        l_ret = -1;
        #ifdef DEBUG
        printf (RED"ERROR>> packet structure: command reply 0x%x "RES"\n", l_dat1);
        #endif //DEBUG
      }
    }
    else
    {
      l_ret = -1;
      #ifdef DEBUG 
      printf (RED"ERROR>> Reading from empty slave or wrong address: "RES"\n");
      printf ("  SFP: %d, slave id: %d, 0x%x addr: 0x%x,  command reply:  0x%x \n",
              l_sfp, l_slave, (l_addr & 0xf0000) >> 24 , l_addr & 0xfffff, l_dat1);
      #endif // DEBUG
    }
  }
  //usleep (1);             // nXYter I2C needs 500us
  f_i2c_sleep ();
  return (l_ret);
  #endif // (else) USE_MBSPEX_LIB
}

/*****************************************************************************/

void f_i2c_sleep ()
{
  //#define N_LOOP 300000
  #define N_LOOP 500000

  int l_ii;
  int volatile l_depp=0; 
 
  for (l_ii=0; l_ii<N_LOOP; l_ii++)
  {
    l_depp++;
  }
}

/*****************************************************************************/
