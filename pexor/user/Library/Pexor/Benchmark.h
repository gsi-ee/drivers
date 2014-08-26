/*
 * \file
 * Benchmark.h
 *
 *  Created on: 27.01.2010
 *      Author: adamczew
 */

#ifndef PEXOR_BENCHMARK_H_
#define PEXOR_BENCHMARK_H_

#include <sys/timex.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <stdio.h>

#if defined (__x86_64__) || defined(__i386__)
/* Note: only x86 CPUs which have rdtsc instruction are supported. */
   typedef unsigned long long cycles_t;
#elif defined(__PPC__) || defined(__PPC64__)
/* Note: only PPC CPUs which have mftb instruction are supported. */
/* PPC64 has mftb */
   typedef unsigned long cycles_t;
 #elif defined(__ia64__)
/* Itanium2 and up has ar.itc (Itanium1 has errata) */
   typedef unsigned long cycles_t;
#else

#endif



namespace pexor {

/**
 * Contains collection of timer and clock tools for performance measurements
 */
class Benchmark {
public:
	Benchmark();

	virtual ~Benchmark();



/** Inits and calibrates timer*/
void TimerInit();


/** resets start time value*/
void TimerStart();

/** return value is seconds since TimerStart*/
double TimerDelta();



/** resets start time value of system realtime clock*/
void ClockStart();

/** return value is seconds since ClockStart*/
double ClockDelta();


static void ShowRate(const char* description, double mem, double time);


double Linux_get_cpu_mhz();


protected:


	void tsDiff(struct timespec *diff, struct timespec* begin, struct timespec* end);

	double tsAsSeconds(struct timespec* ts);

#if defined (__x86_64__) || defined(__i386__)
/* Note: only x86 CPUs which have rdtsc instruction are supported. */
   inline cycles_t GetCycles();
#elif defined(__PPC__) || defined(__PPC64__)
/* Note: only PPC CPUs which have mftb instruction are supported. */
/* PPC64 has mftb */
   inline cycles_t GetCycles();
 #elif defined(__ia64__)
/* Itanium2 and up has ar.itc (Itanium1 has errata) */
   inline cycles_t GetCycles();

#else

#endif



private:







	cycles_t fStartTime;

	cycles_t fDeltaTime;

	double fCyclesToSeconds;

	struct timespec fClockStart;

	struct timespec fClockStop;

	struct timespec fClockDiff;




};

}

#endif /* BENCHMARK_H_ */
