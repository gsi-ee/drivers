/*
 * timing.h
 * *
 *  Collection of common timing functions for mbspex io benchmark
 *  Based on previously exisiting code
 */

#ifndef MBSPEXTIMING_H_
#define MBSPEXTIMING_H_

#include <sys/timex.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>




#if defined (__x86_64__) || defined(__i386__)
/* Note: only x86 CPUs which have rdtsc instruction are supported. */
   typedef unsigned long long cycles_t;
   cycles_t MbsPextest_GetCycles();
#elif defined(__PPC__) || defined(__PPC64__)
/* Note: only PPC CPUs which have mftb instruction are supported. */
/* PPC64 has mftb */
   typedef unsigned long cycles_t;
   inline cycles_t MbsPextest_GetCycles();
 #elif defined(__ia64__)
/* Itanium2 and up has ar.itc (Itanium1 has errata) */
   typedef unsigned long cycles_t;
   inline cycles_t MbsPextest_GetCycles();

#else

#endif






/* Inits and calibrates timer*/
void MbsPextest_TimerInit();


/* resets start time value*/
void MbsPextest_TimerStart();

/* return value is seconds since TimerStart*/
double MbsPextest_TimerDelta();




/* resets start time value of system realtime clock*/
void MbsPextest_ClockStart();

/* return value is seconds since ClockStart*/
double MbsPextest_ClockDelta();


void MbsPextest_ShowRate(const char* description, double mem, double time);

double MbsPextest_Linux_get_cpu_mhz();

void MbsPextest_tsDiff(struct timespec *diff, struct timespec* begin, struct timespec* end);

double MbsPextest_tsAsSeconds(struct timespec* ts);

#endif /* TIMING_H_ */
