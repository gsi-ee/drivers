/* MBS version, supporting Lynx, AIX , HP, and Linux!! */
/* H.Essel, 6.6.2008 */
/* This central include file defines data types for all platforms */

#ifndef TYPEDEF_H
#define TYPEDEF_H

/* Platform independent definitions */

typedef   signed char   CHARS;
typedef unsigned char   CHARU;
typedef   signed char   INTS1;
typedef unsigned char   INTU1;
typedef   signed short  INTS2;
typedef unsigned short  INTU2;
typedef   signed int    INTS4;
typedef unsigned int    INTU4;
typedef          float  REAL4;
typedef          double REAL8;

/* Platform specific definitions */

#ifdef Linux
#define GSI__LINUX
typedef unsigned long ADDRS;
#endif

#ifdef _AIX
#define GSI__AIX
typedef          long long INTS8;
typedef unsigned long long INTU8;
typedef unsigned long ADDRS;
#endif

#ifdef _HPUX_SOURCE
#define GSI__HPUX
typedef          long INTS8;
typedef unsigned long INTU8;
typedef unsigned long ADDRS;
#endif

#ifdef Lynx
#define GSI__LYNX
typedef          long INTS8;
typedef unsigned long INTU8;
typedef unsigned long ADDRS;
#endif
#endif
