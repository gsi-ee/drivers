/*
 * timing.h
 *
 *  Created on: 07.12.2009
 *      Author: adamczew
 */

#ifndef TIMING_H_
#define TIMING_H_

#include <sys/timex.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <stdio.h>




#if defined (__x86_64__) || defined(__i386__)
/* Note: only x86 CPUs which have rdtsc instruction are supported. */
   typedef unsigned long long cycles_t;
   inline cycles_t Pexortest_GetCycles();
#elif defined(__PPC__) || defined(__PPC64__)
/* Note: only PPC CPUs which have mftb instruction are supported. */
/* PPC64 has mftb */
   typedef unsigned long cycles_t;
   inline cycles_t Pexortest_GetCycles();
 #elif defined(__ia64__)
/* Itanium2 and up has ar.itc (Itanium1 has errata) */
   typedef unsigned long cycles_t;
   inline cycles_t Pexortest_GetCycles();

#else

#endif






/* Inits and calibrates timer*/
void Pexortest_TimerInit();


/* resets start time value*/
void Pexortest_TimerStart();

/* return value is seconds since TimerStart*/
double Pexortest_TimerDelta();




/* resets start time value of system realtime clock*/
void Pexortest_ClockStart();

/* return value is seconds since ClockStart*/
double Pexortest_ClockDelta();


void Pexortest_ShowRate(const char* description, double mem, double time);

double Pexortest_Linux_get_cpu_mhz();

void Pexortest_tsDiff(struct timespec *diff, struct timespec* begin, struct timespec* end);

double Pexortest_tsAsSeconds(struct timespec* ts);

#endif /* TIMING_H_ */
