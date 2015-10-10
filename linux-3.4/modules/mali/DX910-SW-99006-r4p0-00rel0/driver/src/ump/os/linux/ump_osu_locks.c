/*
 * Copyright (C) 2010-2013 ARM Limited. All rights reserved.
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *       http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#if ((!defined _XOPEN_SOURCE) || ((_XOPEN_SOURCE - 0) < 600))
#undef _XOPEN_SOURCE
#define _XOPEN_SOURCE 600
#endif

#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200112L
#elif _POSIX_C_SOURCE < 200112L
#undef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200112L
#endif

#include <ump/ump_osu.h>
#include <ump/ump_debug.h>

#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <errno.h>

/**
 * @file ump_osu_locks.c
 * File implements the user side of the OS interface
 */

/** @opt Most of the time, we use the plain mutex type of osu_lock, and so
 * only require the flags and mutex members. This costs 2 extra DWORDS, but
 * most of the time we don't use those DWORDS.
 * Therefore, ANY_UNLOCK type osu_locks can be implemented as a second
 * structure containing the member _ump_osu_lock_t lock_t, plus the extra
 * state required. Then, we use &container->lock_t when passing out of the
 * OSU api, and CONTAINER_OF() when passing back in to recover the original
 * structure. */

/** Private declaration of the OSU lock type */
struct _ump_osu_lock_t_struct
{
	/** At present, only two types of mutex, so we store this information as
	 * the flags supplied at init time */
	_ump_osu_lock_flags_t flags;

	pthread_mutex_t mutex; /**< Used in both plain and ANY_UNLOCK osu_locks */

	/* Extra State for ANY_UNLOCK osu_locks. These are UNINITIALIZED when
	 * flags does not contain _UMP_OSU_LOCKFLAG_ANYUNLOCK: */
	pthread_cond_t condition;  /**< The condition object to use while blocking */
	ump_bool state;  /**< The boolean which indicates the event's state */

	UMP_DEBUG_CODE(
	    /** debug checking of locks */
	    _ump_osu_lock_mode_t locked_as;
	) /* UMP_DEBUG_CODE */

};

/* Provide two statically initialized locks */
UMP_STATIC _ump_osu_lock_t _ump_osu_static_locks[] =
{
	{
		_UMP_OSU_LOCKFLAG_STATIC,
		PTHREAD_MUTEX_INITIALIZER,
		PTHREAD_COND_INITIALIZER,
		UMP_FALSE,
		UMP_DEBUG_CODE(_UMP_OSU_LOCKMODE_UNDEF)
	},
	{
		_UMP_OSU_LOCKFLAG_STATIC,
		PTHREAD_MUTEX_INITIALIZER,
		PTHREAD_COND_INITIALIZER,
		UMP_FALSE,
		UMP_DEBUG_CODE(_UMP_OSU_LOCKMODE_UNDEF)
	},
	{
		_UMP_OSU_LOCKFLAG_STATIC,
		PTHREAD_MUTEX_INITIALIZER,
		PTHREAD_COND_INITIALIZER,
		UMP_FALSE,
		UMP_DEBUG_CODE(_UMP_OSU_LOCKMODE_UNDEF)
	},
	{
		_UMP_OSU_LOCKFLAG_STATIC,
		PTHREAD_MUTEX_INITIALIZER,
		PTHREAD_COND_INITIALIZER,
		UMP_FALSE,
		UMP_DEBUG_CODE(_UMP_OSU_LOCKMODE_UNDEF)
	},
};

/* Critical section for auto_init */
UMP_STATIC pthread_mutex_t  static_auto_init_mutex = PTHREAD_MUTEX_INITIALIZER;


_ump_osu_errcode_t _ump_osu_lock_auto_init(_ump_osu_lock_t **pplock, _ump_osu_lock_flags_t flags, u32 initial, u32 order)
{
	int call_result;
	/* Validate parameters: */
	UMP_DEBUG_ASSERT_POINTER(pplock);

	/** @opt We don't lock the Critical Section or do anything if this is already non-null */
	if (NULL != *pplock)
	{
		return _UMP_OSU_ERR_OK;
	}

	/* We MIGHT need to initialize it, lock the Critical Section and check again */
	call_result = pthread_mutex_lock(&static_auto_init_mutex);
	/* It would be a programming error for this to fail: */
	UMP_DEBUG_ASSERT(0 == call_result,
	                 ("failed to lock critical section\n"));

	if (NULL != *pplock)
	{
		/*
		    We caught a race condition to initialize this osu_lock.
		    The other thread won the race, so the osu_lock is now initialized.
		*/
		call_result = pthread_mutex_unlock(&static_auto_init_mutex);

		UMP_DEBUG_ASSERT(0 == call_result,
		                 ("failed to unlock critical section\n"));

		return _UMP_OSU_ERR_OK;
	}

	/* We're the first thread in: initialize the osu_lock */
	*pplock = _ump_osu_lock_init(flags, initial, order);

	if (NULL == *pplock)
	{
		/* osu_lock creation failed */
		call_result = pthread_mutex_unlock(&static_auto_init_mutex);
		UMP_DEBUG_ASSERT(0 == call_result,
		                 ("failed to unlock critical section\n"));

		return _UMP_OSU_ERR_FAULT;
	}


	/* osu_lock created OK */
	call_result = pthread_mutex_unlock(&static_auto_init_mutex);

	UMP_DEBUG_ASSERT(0 == call_result,
	                 ("failed to unlock critical section\n"));

	UMP_IGNORE(call_result);

	return _UMP_OSU_ERR_OK;
}


_ump_osu_lock_t *_ump_osu_lock_init(_ump_osu_lock_flags_t flags, u32 initial, u32 order)
{
	_ump_osu_lock_t *lock;
	pthread_mutexattr_t mutex_attributes;

	UMP_IGNORE(order); /* order isn't implemented yet, for now callers should set it to zero. */

	/* Validate parameters: */
	/* Flags acceptable */
	UMP_DEBUG_ASSERT(0 == (flags & ~(_UMP_OSU_LOCKFLAG_ANYUNLOCK)),
	                 ("incorrect flags or trying to initialise a statically initialized lock, %.8X\n", flags));

	/* Parameter initial SBZ - for future expansion */
	UMP_DEBUG_ASSERT(0 == initial,
	                 ("initial must be zero\n"));

	if (0 != pthread_mutexattr_init(&mutex_attributes))
	{
		return NULL;
	}

#if UMP_DEBUG_EXTENDED_MUTEX_LOCK_CHECKING
#define UMP_PTHREADS_MUTEX_TYPE PTHREAD_MUTEX_ERRORCHECK
#else
#define UMP_PTHREADS_MUTEX_TYPE PTHREAD_MUTEX_DEFAULT
#endif

	if (0 != pthread_mutexattr_settype(&mutex_attributes, UMP_PTHREADS_MUTEX_TYPE))
	{
		/** Return NULL on failure */
		pthread_mutexattr_destroy(&mutex_attributes);
		return NULL;

	}

#undef UMP_PTHREADS_MUTEX_TYPE

	/** @opt use containing structures for the ANY_UNLOCK type, to
	 * save 2 DWORDS when not in use */
	lock = _ump_osu_malloc(sizeof(_ump_osu_lock_t));

	if (NULL == lock)
	{
		/** Return NULL on failure */
		pthread_mutexattr_destroy(&mutex_attributes);
		return NULL;
	}

	if (0 != pthread_mutex_init(&lock->mutex, &mutex_attributes))
	{
		pthread_mutexattr_destroy(&mutex_attributes);
		_ump_osu_free(lock);
		return NULL;
	}

	/* done with the mutexattr object */
	pthread_mutexattr_destroy(&mutex_attributes);

	/* ANY_UNLOCK type */
	if (flags & _UMP_OSU_LOCKFLAG_ANYUNLOCK)
	{
		if (0 != pthread_cond_init(&lock->condition, NULL))
		{
			/* cleanup */
			pthread_mutex_destroy(&lock->mutex);
			_ump_osu_free(lock);
			return NULL;
		}

		lock->state = UMP_FALSE; /* mark as unlocked by default */
	}

	lock->flags = flags;

	/** Debug lock checking */
	UMP_DEBUG_CODE(lock->locked_as = _UMP_OSU_LOCKMODE_UNDEF);

	return lock;
}

_ump_osu_errcode_t _ump_osu_lock_timed_wait(_ump_osu_lock_t *lock, _ump_osu_lock_mode_t mode, u64 timeout)
{
	/* absolute time specifier */
	struct timespec ts;
	struct timeval tv;

	/* Parameter validation */
	UMP_DEBUG_ASSERT_POINTER(lock);

	UMP_DEBUG_ASSERT(_UMP_OSU_LOCKMODE_RW == mode,
	                 ("unrecognised mode, %.8X\n", mode));
	UMP_DEBUG_ASSERT(_UMP_OSU_LOCKFLAG_ANYUNLOCK == lock->flags, ("Timed operations only implemented for ANYUNLOCK type locks"));

	/* calculate the realtime timeout value */

	if (0 != gettimeofday(&tv, NULL))
	{
		UMP_DEBUG_PRINT(1, ("Could not get the current realtime value to calculate the absolute value for a timed mutex lock with a timeout"));
		return _UMP_OSU_ERR_FAULT;
	}

	tv.tv_usec += timeout;

#define UMP_USECS_PER_SECOND 1000000LL
#define UMP_NANOSECS_PER_USEC 1000LL

	/* did we overflow a second in the usec part? */
	while (tv.tv_usec >= UMP_USECS_PER_SECOND)
	{
		tv.tv_usec -= UMP_USECS_PER_SECOND;
		tv.tv_sec++;
	}

	/* copy to the correct struct */
	ts.tv_sec = tv.tv_sec;
	ts.tv_nsec = (tv.tv_usec * UMP_NANOSECS_PER_USEC);

#undef UMP_USECS_PER_SECOND
#undef UMP_NANOSECS_PER_USEC

	/* lock the mutex protecting access to the state field */
	pthread_mutex_lock(&lock->mutex);

	/* loop while locked (state is UMP_TRUE) */
	/* pthread_cond_timedwait unlocks the mutex, wait, and locks the mutex once unblocked (either due to the event or the timeout) */
	while (UMP_TRUE == lock->state)
	{
		int res;
		res = pthread_cond_timedwait(&lock->condition, &lock->mutex, &ts);

		if (0 == res)
		{
			continue;    /* test the state variable again (loop condition) */
		}
		else if (ETIMEDOUT == res)
		{
			/* timeout, need to clean up and return the correct error code */
			pthread_mutex_unlock(&lock->mutex);
			return _UMP_OSU_ERR_TIMEOUT;
		}
		else
		{
			UMP_DEBUG_PRINT(1, ("Unexpected return from pthread_cond_timedwait 0x%08X\n", res));

			pthread_mutex_unlock(&lock->mutex);
			return _UMP_OSU_ERR_FAULT;
		}

	}

	/* DEBUG tracking of previously locked state - occurs while lock is obtained */
	UMP_DEBUG_ASSERT(_UMP_OSU_LOCKMODE_UNDEF == lock->locked_as,
	                 ("This lock was already locked\n"));
	UMP_DEBUG_CODE(lock->locked_as = mode);

	/* the state is UMP_FALSE (unlocked), so we set it to UMP_TRUE to indicate that it's locked and can return knowing that we own the lock */
	lock->state = UMP_TRUE;
	/* final unlock of the mutex */
	pthread_mutex_unlock(&lock->mutex);

	return _UMP_OSU_ERR_OK;

}

_ump_osu_errcode_t _ump_osu_lock_wait(_ump_osu_lock_t *lock, _ump_osu_lock_mode_t mode)
{
	/* Parameter validation */
	UMP_DEBUG_ASSERT_POINTER(lock);

	UMP_DEBUG_ASSERT(_UMP_OSU_LOCKMODE_RW == mode,
	                 ("unrecognised mode, %.8X\n", mode));

	/** @note since only one flag can be set, we use a switch statement here.
	 * Otherwise, MUST add an enum into the _ump_osu_lock_t to store the
	 * implemented lock type */
	switch (lock->flags)
	{
		case _UMP_OSU_LOCKFLAG_STATIC:
		case _UMP_OSU_LOCKFLAG_DEFAULT:
			/* Usual Mutex type */
		{
			int call_result;
			call_result = pthread_mutex_lock(&lock->mutex);
			UMP_DEBUG_ASSERT(0 == call_result,
			                 ("pthread_mutex_lock call failed with error code %d\n", call_result));
			UMP_IGNORE(call_result);
		}

			/* DEBUG tracking of previously locked state - occurs while lock is obtained */
		UMP_DEBUG_ASSERT(_UMP_OSU_LOCKMODE_UNDEF == lock->locked_as,
		                 ("This lock was already locked\n"));
		UMP_DEBUG_CODE(lock->locked_as = mode);
		break;

		case _UMP_OSU_LOCKFLAG_ANYUNLOCK:
			/** @note Use of bitflags in a case statement ONLY works because this
			 * is the ONLY flag that is supported */

			/* lock the mutex protecting access to the state field */
			pthread_mutex_lock(&lock->mutex);

			/* loop while locked (state is UMP_TRUE) */
			/* pthread_cond_wait unlocks the mutex, wait, and locks the mutex once unblocked */
			while (UMP_TRUE == lock->state)
			{
				pthread_cond_wait(&lock->condition, &lock->mutex);
			}

			/* DEBUG tracking of previously locked state - occurs while lock is obtained */
			UMP_DEBUG_ASSERT(_UMP_OSU_LOCKMODE_UNDEF == lock->locked_as,
			                 ("This lock was already locked\n"));
			UMP_DEBUG_CODE(lock->locked_as = mode);

			/* the state is UMP_FALSE (unlocked), so we set it to UMP_TRUE to indicate that it's locked and can return knowing that we own the lock */
			lock->state = UMP_TRUE;
			/* final unlock of the mutex */
			pthread_mutex_unlock(&lock->mutex);
			break;

		default:
			UMP_DEBUG_ERROR(("lock has incorrect flags==%.8X\n", lock->flags));
			break;
	}

	return _UMP_OSU_ERR_OK;
}

_ump_osu_errcode_t _ump_osu_lock_trywait(_ump_osu_lock_t *lock, _ump_osu_lock_mode_t mode)
{
	_ump_osu_errcode_t err = _UMP_OSU_ERR_FAULT;
	/* Parameter validation */
	UMP_DEBUG_ASSERT_POINTER(lock);

	UMP_DEBUG_ASSERT(_UMP_OSU_LOCKMODE_RW == mode,
	                 ("unrecognised mode, %.8X\n", mode));

	/** @note since only one flag can be set, we use a switch statement here.
	 * Otherwise, MUST add an enum into the _ump_osu_lock_t to store the
	 * implemented lock type */
	switch (lock->flags)
	{
		case _UMP_OSU_LOCKFLAG_STATIC:
		case _UMP_OSU_LOCKFLAG_DEFAULT:
			/* Usual Mutex type */
		{
			/* This is not subject to UMP_CHECK - overriding the result would cause a programming error */
			if (0 == pthread_mutex_trylock(&lock->mutex))
			{
				err = _UMP_OSU_ERR_OK;

				/* DEBUG tracking of previously locked state - occurs while lock is obtained */
				UMP_DEBUG_ASSERT(_UMP_OSU_LOCKMODE_UNDEF == lock->locked_as
				                 || mode == lock->locked_as,
				                 ("tried as mode==%.8X, but was locked as %.8X\n", mode, lock->locked_as));
				UMP_DEBUG_CODE(lock->locked_as = mode);
			}
		}
		break;

		case _UMP_OSU_LOCKFLAG_ANYUNLOCK:
			/** @note Use of bitflags in a case statement ONLY works because this
			 * is the ONLY flag that is supported */

			/* lock the mutex protecting access to the state field */
			pthread_mutex_lock(&lock->mutex);

			if (UMP_FALSE == lock->state)
			{
				/* unlocked, take the lock */
				lock->state = UMP_TRUE;
				err = _UMP_OSU_ERR_OK;
			}

			/* DEBUG tracking of previously locked state - occurs while lock is obtained */
			/* Can do this regardless of whether we obtained ANYUNLOCK: */


			UMP_DEBUG_ASSERT(_UMP_OSU_LOCKMODE_UNDEF == lock->locked_as
			                 || mode == lock->locked_as,
			                 ("tried as mode==%.8X, but was locked as %.8X\n", mode, lock->locked_as));
			/* If we were already locked, this does no harm, because of the above assert: */
			UMP_DEBUG_CODE(lock->locked_as = mode);

			pthread_mutex_unlock(&lock->mutex);
			break;

		default:
			UMP_DEBUG_ERROR(("lock has incorrect flags==%.8X\n", lock->flags));
			break;
	}

	return err;
}


void _ump_osu_lock_signal(_ump_osu_lock_t *lock, _ump_osu_lock_mode_t mode)
{
	/* Parameter validation */
	UMP_DEBUG_ASSERT_POINTER(lock);

	UMP_DEBUG_ASSERT(_UMP_OSU_LOCKMODE_RW == mode,
	                 ("unrecognised mode, %.8X\n", mode));

	/** @note since only one flag can be set, we use a switch statement here.
	 * Otherwise, MUST add an enum into the _ump_osu_lock_t to store the
	 * implemented lock type */
	switch (lock->flags)
	{
		case _UMP_OSU_LOCKFLAG_STATIC:
		case _UMP_OSU_LOCKFLAG_DEFAULT:
			/* Usual Mutex type */

			/* DEBUG tracking of previously locked state - occurs while lock is obtained */
			UMP_DEBUG_ASSERT(mode == lock->locked_as,
			                 ("This lock was locked as==%.8X, but tried to unlock as mode==%.8X\n", lock->locked_as, mode));
			UMP_DEBUG_CODE(lock->locked_as = _UMP_OSU_LOCKMODE_UNDEF);

			{
				int call_result;
				call_result = pthread_mutex_unlock(&lock->mutex);
				UMP_DEBUG_ASSERT(0 == call_result,
				                 ("pthread_mutex_lock call failed with error code %d\n", call_result));
				UMP_IGNORE(call_result);
			}
			break;

		case _UMP_OSU_LOCKFLAG_ANYUNLOCK:
			/** @note Use of bitflags in a case statement ONLY works because this
			 * is the ONLY flag that is supported */

			pthread_mutex_lock(&lock->mutex);
			UMP_DEBUG_ASSERT(UMP_TRUE == lock->state, ("Unlocking a _ump_osu_lock_t %p which is not locked\n", lock));

			/* DEBUG tracking of previously locked state - occurs while lock is obtained */
			UMP_DEBUG_ASSERT(mode == lock->locked_as,
			                 ("This lock was locked as==%.8X, but tried to unlock as %.8X\n", lock->locked_as, mode));
			UMP_DEBUG_CODE(lock->locked_as = _UMP_OSU_LOCKMODE_UNDEF);

			/* mark as unlocked */
			lock->state = UMP_FALSE;

			/* signal the condition, only wake a single thread */
			pthread_cond_signal(&lock->condition);

			pthread_mutex_unlock(&lock->mutex);
			break;

		default:
			UMP_DEBUG_ERROR(("lock has incorrect flags==%.8X\n", lock->flags));
			break;
	}
}

void _ump_osu_lock_term(_ump_osu_lock_t *lock)
{
	int call_result;
	UMP_DEBUG_ASSERT_POINTER(lock);

	/** Debug lock checking: */
	/* Lock is signalled on terminate - not a guarantee, since we could be locked immediately beforehand */
	UMP_DEBUG_ASSERT(_UMP_OSU_LOCKMODE_UNDEF == lock->locked_as,
	                 ("cannot terminate held lock\n"));

	call_result = pthread_mutex_destroy(&lock->mutex);
	UMP_DEBUG_ASSERT(0 == call_result,
	                 ("Incorrect mutex use detected: pthread_mutex_destroy call failed with error code %d\n", call_result));

	/* Destroy extra state for ANY_UNLOCK type osu_locks */
	if (lock->flags & _UMP_OSU_LOCKFLAG_ANYUNLOCK)
	{
		UMP_DEBUG_ASSERT(UMP_FALSE == lock->state, ("terminate called on locked object %p\n", lock));
		call_result = pthread_cond_destroy(&lock->condition);
		UMP_DEBUG_ASSERT(0 == call_result,
		                 ("Incorrect condition-variable use detected: pthread_cond_destroy call failed with error code %d\n", call_result));
	}

	UMP_IGNORE(call_result);

	_ump_osu_free(lock);
}

_ump_osu_lock_t *_ump_osu_lock_static(u32 nr)
{
	UMP_DEBUG_ASSERT(nr < UMP_OSU_STATIC_LOCK_COUNT,
	                 ("provided static lock index (%d) out of bounds (0 < nr < %d)\n", nr, UMP_OSU_STATIC_LOCK_COUNT));
	return &_ump_osu_static_locks[nr];
}
