/*
 * Mutex.cpp
 *
 *  Created on: 02.02.2010
 *      Author: adamczew
 */

#include "Mutex.h"



#ifdef _PEXOR_USE_DABC_

// do nothing, implementation is in linked dabc lib later
#else

#include <errno.h>

namespace pexor {

Mutex::Mutex(bool recursive)
{
   if (recursive) {
      pthread_mutexattr_t attr;
      pthread_mutexattr_init(&attr);
      pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE_NP);
      pthread_mutex_init(&fMutex, &attr);
      pthread_mutexattr_destroy(&attr);
   } else
      pthread_mutex_init(&fMutex, 0);
}

bool Mutex::IsLocked()
{
   int res = pthread_mutex_trylock(&fMutex);
   if (res==EBUSY) return true;
   pthread_mutex_unlock(&fMutex);
   return false;
}



} // namespace


#endif
