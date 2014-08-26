/**
 * \file
 * Mutex.h
 *
 *  Created on: 02.02.2010
 *      Author: adamczew
 */

#ifndef PEXOR_MUTEX_H_
#define PEXOR_MUTEX_H_

/**
 * These utility classes are for convenience, to provide an independent basic thread safety for object lists.
 * The code is completely taken from a part of the dabc/threads implementation.
 * If pexor is compiled together with dabc later, we replace pexor mutex and lockguard by typedefs to dabc classes.
 */

#ifdef _PEXOR_USE_DABC_

#include "dabc/threads.h"
#typedef dabc::Mutex pexor::Mutex
#typedef dabc::LockGuard pexor::LockGuard

#else

#include <pthread.h>

namespace pexor {


 class Mutex {
     friend class LockGuard;
     friend class Condition;
     protected:
        pthread_mutex_t  fMutex;
     public:
        Mutex(bool recursive = false);
        virtual ~Mutex() { pthread_mutex_destroy(&fMutex); }
        inline void Lock() { pthread_mutex_lock(&fMutex); }
        inline void Unlock() { pthread_mutex_unlock(&fMutex); }
        bool IsLocked();
   };

 class LockGuard {
      protected:
         pthread_mutex_t* fMutex;
      public:
         inline LockGuard(pthread_mutex_t& mutex) : fMutex(&mutex)
         {
            pthread_mutex_lock(fMutex);
         }
         inline LockGuard(pthread_mutex_t* mutex) : fMutex(mutex)
         {
            pthread_mutex_lock(fMutex);
         }
         inline LockGuard(const Mutex& mutex) : fMutex((pthread_mutex_t*)&(mutex.fMutex))
         {
            pthread_mutex_lock(fMutex);
         }
         inline LockGuard(const Mutex* mutex) : fMutex(mutex ? (pthread_mutex_t*) &(mutex->fMutex) : 0)
         {
            if (fMutex) pthread_mutex_lock(fMutex);
         }
         inline ~LockGuard()
         {
            if (fMutex) pthread_mutex_unlock(fMutex);
         }
    };



} // namespace

#endif

#endif /* MUTEX_H_ */
