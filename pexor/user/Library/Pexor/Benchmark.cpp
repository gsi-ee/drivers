/*
 * Benchmark.cpp
 *
 *  Created on: 27.01.2010
 *      Author: adamczew
 */

#include "Benchmark.h"

#include "Logger.h"

namespace pexor {

Benchmark::Benchmark() :
fStartTime(0), fDeltaTime(0), fCyclesToSeconds(0)
{
}

Benchmark::~Benchmark()
{
}




void Benchmark::TimerInit()
{
	double mhz=Linux_get_cpu_mhz();
	if(mhz)
	{
		fCyclesToSeconds=1.0e-6 / mhz;
		PexorInfo("Benchmark::TimerInit finds  %lf Mhz CPU => %e s/cycle \n", mhz, fCyclesToSeconds);
	}
	else
	{
		PexorError("Benchmark::TimerInit could not evaluate CPU frequency!!\n");
		fCyclesToSeconds=0;
	}
	fStartTime=0;
	fDeltaTime=0;
}

void Benchmark::TimerStart()
{
	fStartTime=GetCycles();
}

double Benchmark::TimerDelta()
{
	fDeltaTime=GetCycles()-fStartTime;
	return fDeltaTime*fCyclesToSeconds;
}


#if defined (__x86_64__) || defined(__i386__)
/* Note: only x86 CPUs which have rdtsc instruction are supported. */
inline cycles_t Benchmark::GetCycles()
   {
      unsigned low, high;
      unsigned long long val;
      asm volatile ("rdtsc" : "=a" (low), "=d" (high));
      val = high;
      val = (val << 32) | low;
      return val;
   }
#elif defined(__PPC__) || defined(__PPC64__)
/* Note: only PPC CPUs which have mftb instruction are supported. */
/* PPC64 has mftb */
   inline cycles_t Benchmark::GetCycles()
   {
      cycles_t ret;

      asm volatile ("mftb %0" : "=r" (ret) : );
      return ret;
   }
#elif defined(__ia64__)
/* Itanium2 and up has ar.itc (Itanium1 has errata) */
   inline cycles_t Benchmark::GetCycles()
   {
      cycles_t ret;
      asm volatile ("mov %0=ar.itc" : "=r" (ret));
      return ret;
   }

#else

#endif



double Benchmark::Linux_get_cpu_mhz()
{
   FILE* f;
   char buf[256];
   double mhz = 0.0;

   f = fopen("/proc/cpuinfo","r");
   if (!f)
      return 0.0;
   while(fgets(buf, sizeof(buf), f)) {
      double m;
      int rc;
      rc = sscanf(buf, "cpu MHz : %lf", &m);
      if (rc != 1) {   // PPC has a different format
         rc = sscanf(buf, "clock : %lf", &m);
         if (rc != 1)
            continue;
      }
      if (mhz == 0.0) {
         mhz = m;
         continue;
      }
      if (mhz != m) {
    	  PexorError("Conflicting CPU frequency values detected: %lf != %lf\n", mhz, m);
          return 1000.0;
      }
   }
   fclose(f);
   return mhz;
}


/* resets start time value of system realtime clock*/
void Benchmark::ClockStart()
{
	clock_gettime(CLOCK_REALTIME, &fClockStart);

}

/* return value is seconds since ClockStart*/
double Benchmark::ClockDelta()
{

	clock_gettime(CLOCK_REALTIME, &fClockStop);
	Benchmark::tsDiff(&fClockDiff,&fClockStart,&fClockStop);
	return (Benchmark::tsAsSeconds(&fClockDiff));
}




/* these little helpers stolen from mprace lib:*/
void Benchmark::tsDiff(struct timespec *diff, struct timespec* begin, struct timespec* end) {
	if (end->tv_nsec < begin->tv_nsec) {
		diff->tv_nsec = (end->tv_nsec + 1000000000L) - begin->tv_nsec;
		diff->tv_sec = end->tv_sec - begin->tv_sec - 1;
	}
	else {
		diff->tv_nsec = end->tv_nsec - begin->tv_nsec;
		diff->tv_sec = end->tv_sec - begin->tv_sec;
	}

	if (diff->tv_nsec > 1000000000L) {
		diff->tv_sec++;
		diff->tv_nsec -= 1000000000L;
	}
}

double Benchmark::tsAsSeconds(struct timespec* ts) {
	double f;
	f = (double)( ts->tv_sec );
	f += (double)(ts->tv_nsec) * 1E-9;
	return f;
}


void Benchmark::ShowRate(const char* description, double mem, double time)
{
	double rate=0;
	if(time)
		rate=mem/time;
	PexorInfo("Show Rate: %s - time:%e s for %e bytes, speed is %e bytes/s\n", description, time, mem,rate);
}




} // namespace
