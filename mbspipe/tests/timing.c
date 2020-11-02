/*
 * timing.c
 *
 *  Created on: 07.12.2009
 *      Author: adamczew
 */



#include "timing.h"

static cycles_t StartTime=0;

static cycles_t DeltaTime=0;

static double CyclesToSeconds=0;

static struct timespec ClockStart, ClockStop, ClockDiff;


void Pexortest_TimerInit()
{
	double mhz=Pexortest_Linux_get_cpu_mhz();
	if(mhz)
	{
		CyclesToSeconds=1.0e-6 / mhz;
		printf("Pexortest_TimerInit finds  %lf Mhz CPU => %e s/cycle \n", mhz, CyclesToSeconds);
		// JAM20202 - debug why cycle time is wrong
		#if defined (__x86_64__) || defined(__i386__)
		    printf("Pexortest_Timer  uses x86  \n");
		#elif defined(__PPC__) || defined(__PPC64__)
		    printf("Pexortest_Timer  uses ppc  \n");
		#elif defined(__ia64__)
		    printf("Pexortest_Timer  uses ia64  \n");
		#else
		    printf("Pexortest_Timer  unknown arch  \n");

		#endif


	}
	else
	{
		printf("Pexortest_TimerInit could not evaluate CPU frequency!!\n");
		CyclesToSeconds=0;
	}
	StartTime=0;
	DeltaTime=0;
}

void Pexortest_TimerStart()
{
	StartTime=Pexortest_GetCycles();
}

double Pexortest_TimerDelta()
{
	DeltaTime=Pexortest_GetCycles()-StartTime;
	return DeltaTime*CyclesToSeconds;
}


#if defined (__x86_64__) || defined(__i386__)
/* Note: only x86 CPUs which have rdtsc instruction are supported. */
   inline cycles_t Pexortest_GetCycles()
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
   inline cycles_t Pexortest_GetCycles()
   {
      cycles_t ret;

      asm volatile ("mftb %0" : "=r" (ret) : );
      return ret;
   }
#elif defined(__ia64__)
/* Itanium2 and up has ar.itc (Itanium1 has errata) */
   inline cycles_t Pexortest_GetCycles()
   {
      cycles_t ret;
      asm volatile ("mov %0=ar.itc" : "=r" (ret));
      return ret;
   }

#else

#endif



double Pexortest_Linux_get_cpu_mhz()
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
          printf("Conflicting CPU frequency values detected: %lf != %lf\n", mhz, m);
          return 1000.0;
      }
   }
   fclose(f);
   return mhz;
}


/* resets start time value of system realtime clock*/
void Pexortest_ClockStart()
{
	clock_gettime(CLOCK_REALTIME, &ClockStart);

}

/* return value is seconds since ClockStart*/
double Pexortest_ClockDelta()
{

	clock_gettime(CLOCK_REALTIME, &ClockStop);
	Pexortest_tsDiff(&ClockDiff,&ClockStart,&ClockStop);
	return (Pexortest_tsAsSeconds(&ClockDiff));
}




/* these little helpers stolen from mprace lib:*/
void Pexortest_tsDiff(struct timespec *diff, struct timespec* begin, struct timespec* end) {
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

	return;
}

double Pexortest_tsAsSeconds(struct timespec* ts) {
	double f;
	f = (double)( ts->tv_sec );
	f += (double)(ts->tv_nsec) * 1E-9;
	return f;
}


void Pexortest_ShowRate(const char* description, double mem, double time)
{
	double rate=0;
	if(time)
		rate=mem/time;
	printf("Show Rate: %s - time:%e s for %e bytes, speed is %e bytes/s\n", description, time, mem,rate);

}



