/**
 * Test of POLAND qfw frontend readout via pexor
 * with pexornet driver and socket connection.
 * JAM 11-Dec-2015
 * */

#include "pexornet/libpexornet.h"

#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <errno.h>
#include <sys/types.h>

#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <libgen.h>
#include <stdlib.h>
#include <signal.h>

#include <sys/select.h>

#include <sys/socket.h>
#include <netdb.h>


#define VERBOSE 1

#ifdef VERBOSE
#define debprint( args... )                    \
  printm( args );
#else
#define debprint( args... ) ;
#endif


//#define SIMPLE_UDP 1
//#define NOREADOUT 1

// nr of slaves on SFP 0  1  2  3
//                     |  |  |  |
//#define NR_SLAVES     {1, 1, 0, 0}
#define NR_SLAVES     {0, 2, 0, 0}

/** receivebuffer length in bytes:*/
#define READBUFLEN 65536

#define MAX_SFP PEXORNET_SFP_NUMBER
#define MAX_SLAVE         16

#define QFWLOOPS 3
#define QFWCHANS 32
#define QFWNUM 8

// address map for slave (exploder1): this is user specific data concering frontends , so it is not available from libmbspex
#define REG_BUF0     0xFFFFD0 // base address for buffer 0 : 0x0000
#define REG_BUF1     0xFFFFD4  // base address for buffer 1 : 0x20000
#define REG_SUBMEM_NUM   0xFFFFD8 //num of channels 8
#define REG_SUBMEM_OFF   0xFFFFDC // offset of channels 0x4000
#define REG_MODID     0xFFFFE0
#define REG_HEADER    0xFFFFE4
#define REG_FOOTER    0xFFFFE8
#define REG_DATA_LEN  0xFFFFEC
#define REG_DATA_REDUCTION  0xFFFFB0  // Nth bit = 1 enable data reduction of  Nth channel from block transfer readout. (bit0:time, bit1-8:adc)
#define REG_MEM_DISABLE     0xFFFFB4  // Nth bit =1  disable Nth channel from block transfer readout.(bit0:time, bit1-8:adc)
#define REG_MEM_FLAG_0      0xFFFFB8  // read only:
#define REG_MEM_FLAG_1      0xFFFFBc  // read only:

#define REG_BUF0_DATA_LEN     0xFFFD00  // buffer 0 submemory data length
#define REG_BUF1_DATA_LEN     0xFFFE00  // buffer 1 submemory data length

#define REG_DATA_REDUCTION  0xFFFFB0  // Nth bit = 1 enable data reduction of  Nth channel from block transfer readout. (bit0:time, bit1-8:adc)
#define REG_MEM_DISABLE     0xFFFFB4  // Nth bit =1  disable Nth channel from block transfer readout.(bit0:time, bit1-8:adc)
#define REG_MEM_FLAG_0      0xFFFFB8  // read only:
#define REG_MEM_FLAG_1      0xFFFFBc  // read only:
#define REG_RST 0xFFFFF4
#define REG_LED 0xFFFFF8
#define REG_VERSION 0xFFFFFC

#define REG_CTRL       0x200000

#define REG_QFW_OFFSET_BASE 0x200100

static long long l_err_prot_ct = 0;
//static  long  l_qfw_init_ct=0;
static long l_qfw_buf_off[MAX_SFP][MAX_SLAVE][2];
static long l_qfw_n_chan[MAX_SFP][MAX_SLAVE];
static long l_qfw_chan_off[MAX_SFP][MAX_SLAVE];
static long l_qfw_ctrl;

static unsigned int ErrorScaler[QFWNUM] = { 0 };

static int sock = 0;
static pexornet_handle_t* handle;
static char data[READBUFLEN];
static unsigned long evcount=0;
static unsigned long numcorrupt=0;

/* helper macro for check_event to check if payload pointer is still inside delivered region:*/
/* this one to be called at top data processing loop*/
#define  QFWRAW_CHECK_PDATA                                    \
if((pdata - pdatastart) > (opticlen/4)) \
{ \
  printf("############ unexpected end of payload for sfp:%d slave:%d with opticlen:0x%x, skip event\n",sfp_id, device_id, opticlen);\
  return -1; \
}

/* this one just to leave internal loops*/
#define  QFWRAW_CHECK_PDATA_BREAK                                    \
if((pdata - pdatastart) > (opticlen/4)) \
{ \
 break; \
}

#define Debugmode 0


void cleanup (pexornet_handle_t* h, int sock);


#ifndef PEXORNET_NOMBS
/*****************************************************************/
/* here separate definition of printm for libpexornet output:*/
#include <stdarg.h>

void printm (char *fmt, ...)
{
  char c_str[256];
  va_list args;
  va_start(args, fmt);
  vsprintf (c_str, fmt, args);
  printf ("%s", c_str);
  va_end(args);
}
#endif

static int l_sfp_slaves[PEXORNET_SFP_NUMBER] = NR_SLAVES;

void my_usage (const char *progname)
{
  printf ("***************************************************************************\n");
  printf (" %s for pexornet network driver of POLAND example  \n", progname);
  printf (" v0.1 21-Jan-2016 by JAM (j.adamczewski@gsi.de)\n");
  printf ("***************************************************************************\n");
  printf ("  usage: %s [-h][-i][-n IF] [numevents]] \n", progname);
  printf ("\t  reads out numevents from POLAND via pex0 socket interface. \n");
  printf ("\t Options:\n");
  printf ("\t\t -h        : display this help\n");
  printf ("\t\t -i        : initialize before readout\n");
  printf ("\t\t -n IF     : specify interface unit number N (pexN, default:0) \n");
}


void my_signal_handler(int sig, siginfo_t *extra, void *cruft)
{
  printf ("Keyboard interrupt, cleaning up acquisition... \n");
  cleanup (handle, sock);
  exit(0);

}

void init_poland (pexornet_handle_t* handle)
{
  /* this one is almost identical to f_user.c  code of mbs, save for the direct usage of libpexornet calls:*/

  int l_i, l_j, l_k, l_stat, l_first = 0;
  printf ("Initializing POLAND readout... \n");

  for (l_i = 0; l_i < MAX_SFP; l_i++)
  {
    if (l_sfp_slaves[l_i] != 0)
    {
      l_stat = pexornet_slave_init (handle, l_i, l_sfp_slaves[l_i]);

      if (l_stat == -1)
      {
        printm (RON"ERROR>>"RES" slave address initialization failed \n");
        printm ("exiting...\n");
        exit (-1);
      }
    }
    printm ("");
  }

  //sleep (1);

//   if (l_first == 0)
//   {
//     l_first = 1;
//     for (l_i=0; l_i<MAX_TRIG_TYPE; l_i++)
//     {
//       l_tr_ct[l_i] = 0;
//     }
//   }

  for (l_i = 0; l_i < MAX_SFP; l_i++)
  {
    if (l_sfp_slaves[l_i] != 0)
    {
      for (l_j = 0; l_j < l_sfp_slaves[l_i]; l_j++)
      {
        // needed for check of meta data, read it in any case
        printm ("SFP: %d, OFW/EXPLODER: %d \n", l_i, l_j);
        // get address offset of qfw buffer 0,1 for each qfw/exploder
        l_stat = pexornet_slave_rd (handle, l_i, l_j, REG_BUF0, &(l_qfw_buf_off[l_i][l_j][0]));
        l_stat = pexornet_slave_rd (handle, l_i, l_j, REG_BUF1, &(l_qfw_buf_off[l_i][l_j][1]));
        // get nr. of channels per qfw
        l_stat = pexornet_slave_rd (handle, l_i, l_j, REG_SUBMEM_NUM, &(l_qfw_n_chan[l_i][l_j]));
        // get buffer per channel offset
        l_stat = pexornet_slave_rd (handle, l_i, l_j, REG_SUBMEM_OFF, &(l_qfw_chan_off[l_i][l_j]));

        printm ("addr offset: buf0: 0x%x, buf1: 0x%x \n", l_qfw_buf_off[l_i][l_j][0], l_qfw_buf_off[l_i][l_j][1]);
        printm ("No. channels: %d \n", l_qfw_n_chan[l_i][l_j]);
        printm ("channel addr offset: 0x%x \n", l_qfw_chan_off[l_i][l_j]);

        // disable test data length
        l_stat = pexornet_slave_wr (handle, l_i, l_j, REG_DATA_LEN, 0x10000000);
        if (l_stat == -1)
        {
          printm (RON"ERROR>>"RES" disabling test data length failed\n");
          l_err_prot_ct++;
        }

        // disable trigger acceptance in exploder2a
        l_stat = pexornet_slave_wr (handle, l_i, l_j, REG_CTRL, 0);
        if (l_stat == -1)
        {
          printm (RON"ERROR>>"RES" PEXOR slave write REG_FEB_CTRL failed\n");
          l_err_prot_ct++;
        }
        l_stat = pexornet_slave_rd (handle, l_i, l_j, REG_CTRL, &l_qfw_ctrl);
        if ((l_qfw_ctrl & 0x1) != 0)
        {
          printm (RON"ERROR>>"RES" disabling trigger acceptance in qfw failed, exiting \n");
          l_err_prot_ct++;
          exit (0);
        }

        // enable trigger acceptance in exploder2a

        l_stat = pexornet_slave_wr (handle, l_i, l_j, REG_CTRL, 1);
        if (l_stat == -1)
        {
          printm (RON"ERROR>>"RES" PEXOR slave write REG_FEB_CTRL failed\n");
          l_err_prot_ct++;
        }
        l_stat = pexornet_slave_rd (handle, l_i, l_j, REG_CTRL, &l_qfw_ctrl);
        if ((l_qfw_ctrl & 0x1) != 1)
        {
          printm (RON"ERROR>>"RES" enabling trigger acceptance in qfw failed, exiting \n");
          l_err_prot_ct++;
          exit (0);
        }

        // write SFP id for channel header
        l_stat = pexornet_slave_wr (handle, l_i, l_j, REG_HEADER, l_i);
        if (l_stat == -1)
        {
          printm (RON"ERROR>>"RES" PEXOR slave write REG_HEADER  failed\n");
          l_err_prot_ct++;
        }

        l_stat = pexornet_slave_wr (handle, l_i, l_j, REG_RST, 1);
        if (l_stat == -1)
        {
          printm (RON"ERROR>>"RES" PEXOR slave write REG_HEADER  failed\n");
          l_err_prot_ct++;
        }
      }
    }
  }

}

int open_interface (const char* name)
{
  int i,errsav, rev, fd = 0;
#ifdef SIMPLE_UDP
  // note: this approach will not specify that we use interface pex0
  struct sockaddr_in sa;
  int port=PEXORNET_RECVPORT; //23452; // currently hardcoded in driver todo configure it.
  /** do it like code as in hadaq, we explicitely set port number*/
  printf ("Opening interface with simple udp receiver on port %d\n", port);
  fd = socket(AF_INET, SOCK_DGRAM, 0);
  errsav = errno;
  if (fd == -1)
    {
      printm ("Could not open simple socket. error:%d (%s) Good bye!\n",errsav, strerror (errsav));
      exit (EXIT_FAILURE);
    }

  // here may set socket options as in daqdata/hadaq/nettrans.c

  sa.sin_family = AF_INET;
  sa.sin_port = htons(port);
  sa.sin_addr.s_addr = htonl(INADDR_ANY);
  rev = bind(fd, &sa, sizeof(sa));
  if(rev!=0)
    {
      printm ("Could not bind udp socket. Good bye!\n");
      exit (EXIT_FAILURE);
    }


#else
  struct addrinfo s_sockhint;
  struct addrinfo *p_sockinfo, *p_sockcursor;
  struct hostent host;
  char service[256];
  int rcvBufLenRet;
  int rcvBufLenReq = 1 * (1 << 20);
  size_t rcvBufLenLen = (size_t) sizeof(rcvBufLenReq);
  int udpCheckReq=1;
  size_t udpCheckLen = (size_t) sizeof(udpCheckReq);



  printf ("Opening interface %s\n", name);
  /** JAM the following is mostly stolen from man 3 getaddrinfo : */
  s_sockhint.ai_family = AF_INET;
  s_sockhint.ai_socktype = SOCK_DGRAM;//SOCK_DGRAM;//SOCK_RAW; //SOCK_STREAM
  s_sockhint.ai_protocol = IPPROTO_UDP; //0;// IPPROTO_RAW ;
  s_sockhint.ai_flags = AI_ADDRCONFIG | AI_NUMERICSERV;
  snprintf(service,256,"%d",PEXORNET_RECVPORT);

  rev = getaddrinfo (name, service, &s_sockhint, &p_sockinfo); // service NULL
  if (rev != 0)
  {
    printm ("getaddrinfo: %s\n", gai_strerror (rev));
    exit (EXIT_FAILURE);
  }

  /* getaddrinfo() returns a list of address structures.
   Try each address until we successfully connect(2).
   If socket(2) (or connect(2)) fails, we (close the socket
   and) try the next address. */

  for (p_sockcursor = p_sockinfo; p_sockcursor != NULL ; p_sockcursor = p_sockcursor->ai_next)
  {
    printm ("Try to open socket (family:%d, type:%d, protocol:%d address:", p_sockcursor->ai_family,
        p_sockcursor->ai_socktype, p_sockcursor->ai_protocol, p_sockcursor->ai_addr->sa_data);
    for(i=0; i<p_sockcursor->ai_addrlen;++i)
      printm ("%u .", (unsigned char) (p_sockcursor->ai_addr->sa_data[i]));
     printm("\n");
    fd = socket (p_sockcursor->ai_family, p_sockcursor->ai_socktype, p_sockcursor->ai_protocol);
    if (fd == -1)
      continue;
    if (bind(fd, p_sockcursor->ai_addr, p_sockcursor->ai_addrlen) == 0)
          break;                  /* Success */
    close (fd);
  }    // for

  if (p_sockcursor == NULL )
  { /* No address succeeded */
    printm ("Could not connect to any of them. Good bye!\n");
    exit (EXIT_FAILURE);
  }
  printm ("Successfully opened socket: (family:%d, type:%d, protocol:%d)\n", p_sockcursor->ai_family,
      p_sockcursor->ai_socktype, p_sockcursor->ai_protocol);

  // here specify explicitely the receive buffer, again stolen from hadaq:
  if (setsockopt (fd, SOL_SOCKET, SO_RCVBUF, &rcvBufLenReq, rcvBufLenLen) == -1)
  {
    printm ("setsockopt(..., SO_RCVBUF, ...): %s", strerror (errno));
  }

  if (getsockopt (fd, SOL_SOCKET, SO_RCVBUF, &rcvBufLenRet, (socklen_t *) &rcvBufLenLen) == -1)
  {
    printm ("getsockopt(..., SO_RCVBUF, ...): %s", strerror (errno));
  }
//  if (rcvBufLenRet < rcvBufLenReq)
//  {
//    printm ("UDP receive buffer length (%d) smaller than requested buffer length (%d)", rcvBufLenRet, rcvBufLenReq);
//  }
  printm ("UDP receive buffer length is set to %d bytes\n", rcvBufLenRet);

  freeaddrinfo (p_sockinfo);    // clean up structure created by getaddrinfo

#endif

  return fd;

}


/** read a single event from socket into the buffer data*/
int read_event (int sock, char* data)
{
  debprint ("readout test: read_event...\n");
  struct sockaddr_storage peer_addr;
  socklen_t peer_addr_len;

  int errsav,flags;
  int numbytes = 0;
  struct pexornet_data_header* phead;
  char* cursor;
  int buflen = 0;
  flags=MSG_WAITALL;// MSG_DONTWAIT, MSG_WAITALL, MSG_PEEK
  cursor = data;
  // first need to read the header part of pexor buffer:
  numbytes = recvfrom (sock, cursor, sizeof(struct pexornet_data_header),flags, (struct sockaddr *) &peer_addr, &peer_addr_len);
  errsav = errno;
  if (numbytes != sizeof(struct pexornet_data_header))
  {
    printm ("readout test; could not read data header (%d bytes received), error %d (%s) on socket read\n", numbytes,
        errsav, strerror (errsav));
    return -1;
  }
  debprint ("readout test: read_event header of length %d \n", numbytes);
  phead = (struct pexornet_data_header*) cursor;
  cursor += sizeof(struct pexornet_data_header);
  debprint ("readout test: found trigger type %d, evcount:%d \n", phead->trigger.typ, phead->trigger.lec);


  buflen = phead->datalen;
  debprint ("readout test: found data length %d \n", buflen);
  if(buflen<=0)
    {
      debprint ("readout test: header with zero payload, try next event...\n");
      return 0;
    }
  // then we know how many bytes will follow:
  numbytes = recvfrom(sock, cursor, buflen, flags,0,0); //+ sizeof(struct pexornet_data_header)
  errsav = errno;
  if (numbytes == -1)
  {
    printm ("readout test: error %d (%s) on socket read\n", errsav, strerror (errsav));
    return -1;
  }
  else if (numbytes != buflen ) // + sizeof(struct pexornet_data_header)
  {
    printm ("readout test: did not read complete payload length %d (read %d bytes), error:%d (%s)\n", buflen, numbytes,
        errsav, strerror (errsav));
    // error handling or todo: repeat read  until fragmented event is complete? no sooner than driver supports this!
  }
  return 0;
}


/** small unpacker to verify data integrity. May also display something*/
int check_event (const char* data)
{

  ///////////////// this code is mostly taken from Go4 unpacker at https://subversion.gsi.de/go4/app/qfw/pexor
  int i, j, l, loop, sl, ch, qfw;
  unsigned trig_head, trig_type, sfp_id, device_id, channel_id;
  int *pdata, *pdatastart;
  int lwords, usedsize, opticlen=-1, eventcounter, QfwSetup;
  int loopsize[QFWLOOPS];
  int looptime[QFWLOOPS];
  struct pexornet_data_header* phead;
  pdata = (int*) data;
  /* for pexornet, evaluate triggerstatus header:*/
  phead = (struct pexornet_data_header*) pdata;
  usedsize = phead->datalen;
  trig_head = phead->trigger.typ;

  /* check trigger types and exclude special triggers:*/
  if (trig_head == PEXORNET_TRIGTYPE_START)
  {
    printm ("Found START trigger type %d ...\n", trig_head);
    return 0;
  }
  else if (trig_head == PEXORNET_TRIGTYPE_STOP)
  {
    printm ("Found STOP trigger type %d ...\n", trig_head);
    return 0;
  }
  else if (trig_head != PEXORNET_TRIGTYPE_DATA)
  {
    printm ("ERROR: received packet with invalid trigger type %d !\n", trig_head);
    return 1;
  }

  pdata += sizeof(struct pexornet_data_header);
  /** here comes the actual unpacker:*/

  pdatastart = pdata;
  lwords = usedsize / sizeof(int);    // this is true filled size from DMA, not total buffer length
  // loop over single subevent data:
  i=5;
  while (pdata - pdatastart < lwords)
  {
    if ((*pdata & 0xff) != 0x34)    // regular channel data
    {
      if(--i>0) debprint ("**** unpack_qfw: Skipping Non-header format 0x%x - (0x34 are expected) ...\n", (*pdata & 0xff));
      pdata++;
      continue;    // we have to skip it, since the dedicated padding pattern is added by mbs and not available here!
    }

    trig_type = (*pdata & 0xf00) >> 8;
    sfp_id = (*pdata & 0xf000) >> 12;
    device_id = (*pdata & 0xff0000) >> 16;
    channel_id = (*pdata & 0xff000000) >> 24;

    pdata++;

    opticlen = *pdata++;
    debprint ("Token header: trigid:0x%x sfp:0x%x modid:0x%x memid:0x%x opticlen:0x%x\n", trig_type, sfp_id, device_id,
        channel_id, opticlen);
    //
    if (opticlen > lwords * 4)
    {
      printf ("**** unpack_qfw: Mismatch with subevent len %d and optic len %d", lwords * 4, opticlen);
      // avoid that we run second step on invalid raw event!
      return -1;
    }
    QFWRAW_CHECK_PDATA;
    eventcounter = *pdata;
    debprint (" - Internal Event number 0x%x\n", eventcounter);
    // board id calculated from SFP and device id:

    pdata += 1;
    QFWRAW_CHECK_PDATA;
    QfwSetup = *pdata;
    debprint (" - QFW SEtup %d\n", QfwSetup);
    for (j = 0; j < 4; ++j)
    {
      QFWRAW_CHECK_PDATA_BREAK;
      pdata++;

    }
    QFWRAW_CHECK_PDATA;
    for (l = 0; l < QFWLOOPS; l++)
    {
      QFWRAW_CHECK_PDATA_BREAK;
      loopsize[l] = *pdata++;
      debprint (" - Loopsize[%d] = 0x%x\n", l, loopsize[l]);
    }    // first loop loop

    QFWRAW_CHECK_PDATA;
    for (loop = 0; loop < QFWLOOPS; loop++)
    {
      QFWRAW_CHECK_PDATA_BREAK;
      looptime[loop] = *pdata++;
      debprint (" - Looptime[%d] = 0x%x\n", loop, looptime[loop]);
    }    // second loop loop

    for (j = 0; j < 21; ++j)
    {
      QFWRAW_CHECK_PDATA_BREAK;
      pdata++;

    }
    QFWRAW_CHECK_PDATA;
    /** All loops X slices/loop X channels */
    for (loop = 0; loop < QFWLOOPS; loop++)
    {
      for (sl = 0; sl < loopsize[loop]; ++sl)
        for (ch = 0; ch < QFWCHANS; ++ch)
        {
          QFWRAW_CHECK_PDATA_BREAK;
          int value = *pdata++;
          //loopData->fQfwTrace[ch].push_back(value);
          // TODO: pseudo trace graphics on terminal
          if (Debugmode)
            debprint (" -- loop %d slice %d ch %d = 0x%x\n", loop, sl, ch, value);
        }
    }    //loop

    QFWRAW_CHECK_PDATA;
    /* errorcount values: - per QFW CHIPS*/
    for (qfw = 0; qfw < QFWNUM; ++qfw)
    {
      QFWRAW_CHECK_PDATA_BREAK;
      ErrorScaler[qfw] = (unsigned int) (*pdata++);
      debprint (" - ErrorScaler[%d] = 0x%x\n", qfw, ErrorScaler[qfw]);
    }
    QFWRAW_CHECK_PDATA;

    // skip filler words at the end of gosip payload:
    while (pdata - pdatastart <= (opticlen / 4))    // note that trailer is outside opticlen!
    {
      if (Debugmode)
        debprint ("######### skipping word 0x%x\n ", *pdata);
      pdata++;
    }

    // crosscheck if trailer word matches eventcounter header
    if (*pdata != eventcounter)
    {
      printf ("!!!!! Eventcounter 0x%x does not match trailing word 0x%x at position 0x%x!\n", eventcounter, *pdata,
          (opticlen / 4));
    }
    else
    {
      debprint ("Found trailing Eventcounter 0x%x \n", *pdata);
    }
    pdata++;
  }    // while pdata - pdatastart < lwords

  if(opticlen<0) return -1; // no any valid data in packet!
  ////////////////////////////// end go4 unpacker

  return 0;
}

void cleanup (pexornet_handle_t* h, int sock)
{
  if(h==0)
    h=pexornet_open(0); // TEST: if handle was closed after start acquisition

  pexornet_acquisition_stop (h);
#ifndef NOREADOUT
  if(read_event(sock,data)>=0) // also get the stop acquisition trigger packet here?
  {
    evcount++;
    if(check_event(data)!=0) numcorrupt++;
  }
  if (evcount)
     printm ("Read %d  events, corrupt:%d (ratio %f)\n", evcount, numcorrupt,
         (double) numcorrupt / (double) evcount);
#endif

  pexornet_close (h);
  if(sock) close (sock);
}

int main (int argc, char *argv[])
{
  int rev=0;
  int errsav = 0, cmdlen = 0, i = 0, opt = 0;
  int ifnum = 0;
  int numevents = -1;

  int initfirst = 0;
  char hname[256];
  struct sigaction s_sact;

  optind = 1;
  while ((opt = getopt (argc, argv, "hin:")) != -1)
  {
    switch (opt)
    {
      case '?':
        my_usage (basename (argv[0]));
        exit (1);
      case 'h':
        my_usage (basename (argv[0]));
        exit (0);
      case 'i':
        initfirst = 1;
        break;
      case 'n':
        ifnum = strtol (optarg, NULL, 0);
        break;

    };

  }    // while

  cmdlen = argc - optind;
  for (i = 0; i < cmdlen; i++)
  {
    if (argv[optind + i])
    {
      if (i == 0)
        numevents = strtol (argv[optind + i], NULL, 0);
    }
    else
      printm ("warning: argument at position %d is empty!", optind + i);
  }

  /** ctrl-C signal handler here:*/
  sigemptyset (&s_sact.sa_mask);
  s_sact.sa_flags = SA_SIGINFO;
  s_sact.sa_sigaction = my_signal_handler;
  if (sigaction (SIGINT, &s_sact, NULL ) < 0)
    {
      printm("!!Error calling sigaction, exiting..\n");
      exit (0);
    }

  handle = pexornet_open (ifnum);
  if (!handle)
    exit (0);
  if (initfirst)
    init_poland (handle);
#ifndef NOREADOUT
  snprintf (hname, 64, "pexor%d", ifnum);    // we need pseudo host name, not interface name here
  sock = open_interface (hname);
#endif
  printm ("Reading %d  events... \n", numevents);
  pexornet_acquisition_start (handle);

#ifdef NOREADOUT
    sleep(60);
    printm ("After sleep without readout.\n");
#else
for (evcount = 0; evcount < (unsigned long) numevents; ++evcount)
  {
    rev=read_event(sock,data);
    if(rev<0)
      {
        cleanup (handle, sock);
        exit (EXIT_FAILURE);
      }
    debprint ("Read event %d.\n", evcount);
    if (check_event (data) != 0)
    {
      numcorrupt++;
    }

  }    // for
#endif
  cleanup (handle, sock);
  return 0;
}
