/*
 * Copyright (C) 2010-2011, 2013 ARM Limited. All rights reserved.
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

/**
 * @file ump_frontend.c
 *
 * This file implements the user space API of the UMP API.
 * It relies heavily on a arch backend to do the communication with the UMP device driver.
 */

#include <ump/ump.h>
#include "ump_internal.h"
#include "ump_arch.h"
#include <ump/ump_debug.h>
#include <ump/ump_osu.h>

UMP_API_EXPORT ump_result ump_open(void)
{
	return ump_arch_open();
}

UMP_API_EXPORT void ump_close(void)
{
	ump_arch_close();
}

UMP_API_EXPORT ump_secure_id ump_secure_id_get(ump_handle memh)
{
	ump_mem *mem = (ump_mem *)memh;

	UMP_DEBUG_ASSERT(UMP_INVALID_MEMORY_HANDLE != memh, ("Handle is invalid"));
	UMP_DEBUG_ASSERT(UMP_INVALID_SECURE_ID != mem->secure_id, ("Secure ID is inavlid"));
	UMP_DEBUG_ASSERT(0 < mem->ref_count, ("Reference count too low"));
	UMP_DEBUG_ASSERT(0 < mem->size, ("Memory size of passed handle too low"));

	return mem->secure_id;
}

UMP_API_EXPORT ump_handle ump_handle_create_from_secure_id(ump_secure_id secure_id)
{
	unsigned long size;

	UMP_DEBUG_ASSERT(UMP_INVALID_SECURE_ID != secure_id, ("Secure ID is invalid"));

	size = ump_arch_size_get(secure_id);

	if (0 != size)
	{
		unsigned long cookie;
		/*
		 * The UMP memory which the secure_id referes to could now be deleted and re-created
		 * since we don't have any references to it yet. The mapping below will however fail if
		 * we have supplied incorrect size, so we are safe.
		 */
		void *mapping = ump_arch_map(secure_id, size, UMP_CACHE_DISABLE, &cookie);

		if (NULL != mapping)
		{
			ump_mem *mem = _ump_osu_calloc(1, sizeof(*mem));

			if (NULL != mem)
			{
				mem->secure_id = secure_id;
				mem->mapped_mem = mapping;
				mem->size = size;
				mem->cookie = cookie;
				mem->is_cached = UMP_CACHE_ENABLE; /* Is set to actually check in the ump_cpu_msync_now() function */

				_ump_osu_lock_auto_init(&mem->ref_lock, _UMP_OSU_LOCKFLAG_DEFAULT, 0, 0);
				UMP_DEBUG_ASSERT(NULL != mem->ref_lock, ("Failed to initialize lock\n"));
				mem->ref_count = 1;

				/* This is called only to set the cache settings in this handle */
				ump_cpu_msync_now((ump_handle)mem, UMP_MSYNC_READOUT_CACHE_ENABLED, NULL, 0);

				UMP_DEBUG_PRINT(4, ("UMP handle created for ID %u of size %lu, mapped into address 0x%08lx", mem->secure_id, mem->size, (unsigned long)mem->mapped_mem));

				return (ump_handle)mem;
			}

			ump_arch_unmap(mapping, size, cookie);
		}
	}

	UMP_DEBUG_PRINT(2, ("UMP handle creation failed for ID %u", secure_id));

	return UMP_INVALID_MEMORY_HANDLE;
}

UMP_API_EXPORT unsigned long ump_size_get(ump_handle memh)
{
	ump_mem *mem = (ump_mem *)memh;

	UMP_DEBUG_ASSERT(UMP_INVALID_MEMORY_HANDLE != memh, ("Handle is invalid"));
	UMP_DEBUG_ASSERT(UMP_INVALID_SECURE_ID != mem->secure_id, ("Secure ID is inavlid"));
	UMP_DEBUG_ASSERT(0 < mem->ref_count, ("Reference count too low"));
	UMP_DEBUG_ASSERT(0 < mem->size, ("Memory size of passed handle too low"));

	return mem->size;
}

UMP_API_EXPORT void ump_read(void *dst, ump_handle srch, unsigned long offset, unsigned long length)
{
	ump_mem *src = (ump_mem *)srch;

	UMP_DEBUG_ASSERT(UMP_INVALID_MEMORY_HANDLE != srch, ("Handle is invalid"));
	UMP_DEBUG_ASSERT(UMP_INVALID_SECURE_ID != src->secure_id, ("Secure ID is inavlid"));
	UMP_DEBUG_ASSERT(0 < src->ref_count, ("Reference count too low"));
	UMP_DEBUG_ASSERT(0 < src->size, ("Memory size of passed handle too low"));
	UMP_DEBUG_ASSERT(NULL != src->mapped_mem, ("UMP Memory is not mapped"));
	UMP_DEBUG_ASSERT((src->size) >= (offset + length), ("Requested read beyond end of UMP memory"));

	_ump_osu_memcpy(dst, (char *)(src->mapped_mem) + offset, length);
}

UMP_API_EXPORT void ump_write(ump_handle dsth, unsigned long offset, const void *src, unsigned long length)
{
	ump_mem *dst = (ump_mem *)dsth;

	UMP_DEBUG_ASSERT(UMP_INVALID_MEMORY_HANDLE != dsth, ("Handle is invalid"));
	UMP_DEBUG_ASSERT(UMP_INVALID_SECURE_ID != dst->secure_id, ("Secure ID is inavlid"));
	UMP_DEBUG_ASSERT(0 < dst->ref_count, ("Reference count too low"));
	UMP_DEBUG_ASSERT(0 < dst->size, ("Memory size of passed handle too low"));
	UMP_DEBUG_ASSERT(NULL != dst->mapped_mem, ("UMP Memory is not mapped"));
	UMP_DEBUG_ASSERT((dst->size) >= (offset + length), ("Requested write beyond end of UMP memory"));

	_ump_osu_memcpy((char *)(dst->mapped_mem) + offset, src, length);
}



UMP_API_EXPORT void *ump_mapped_pointer_get(ump_handle memh)
{
	ump_mem *mem = (ump_mem *)memh;

	UMP_DEBUG_ASSERT(UMP_INVALID_MEMORY_HANDLE != memh, ("Handle is invalid"));
	UMP_DEBUG_ASSERT(UMP_INVALID_SECURE_ID != mem->secure_id, ("Secure ID is inavlid"));
	UMP_DEBUG_ASSERT(0 < mem->ref_count, ("Reference count too low"));
	UMP_DEBUG_ASSERT(0 < mem->size, ("Memory size of passed handle too low"));
	UMP_DEBUG_ASSERT(NULL != mem->mapped_mem, ("Error in mapping pointer (not mapped)"));

	return mem->mapped_mem;
}



UMP_API_EXPORT void ump_mapped_pointer_release(ump_handle memh)
{
	UMP_DEBUG_ASSERT(UMP_INVALID_MEMORY_HANDLE != memh, ("Handle is invalid"));
	UMP_DEBUG_ASSERT(UMP_INVALID_SECURE_ID != ((ump_mem *)memh)->secure_id, ("Secure ID is inavlid"));
	UMP_DEBUG_ASSERT(0 < ((ump_mem *)memh)->ref_count, ("Reference count too low"));
	UMP_DEBUG_ASSERT(0 < ((ump_mem *)memh)->size, ("Memory size of passed handle too low"));
	UMP_DEBUG_ASSERT(NULL != ((ump_mem *)memh)->mapped_mem, ("Error in mapping pointer (not mapped)"));

	/* noop, cos we map in the pointer when handle is created, and unmap it when handle is destroyed */
}



UMP_API_EXPORT void ump_reference_add(ump_handle memh)
{
	ump_mem *mem = (ump_mem *)memh;

	UMP_DEBUG_ASSERT(UMP_INVALID_MEMORY_HANDLE != memh, ("Handle is invalid"));
	UMP_DEBUG_ASSERT(UMP_INVALID_SECURE_ID != mem->secure_id, ("Secure ID is inavlid"));
	UMP_DEBUG_ASSERT(0 < mem->ref_count, ("Reference count too low"));
	UMP_DEBUG_ASSERT(0 < mem->size, ("Memory size of passed handle too low"));

	_ump_osu_lock_wait(mem->ref_lock, _UMP_OSU_LOCKMODE_RW);
	mem->ref_count += 1;
	_ump_osu_lock_signal(mem->ref_lock, _UMP_OSU_LOCKMODE_RW);
}



UMP_API_EXPORT void ump_reference_release(ump_handle memh)
{
	ump_mem *mem = (ump_mem *)memh;

	UMP_DEBUG_ASSERT(UMP_INVALID_MEMORY_HANDLE != memh, ("Handle is invalid"));
	UMP_DEBUG_ASSERT(UMP_INVALID_SECURE_ID != ((ump_mem *)mem)->secure_id, ("Secure ID is inavlid"));
	UMP_DEBUG_ASSERT(0 < (((ump_mem *)mem)->ref_count), ("Reference count too low"));
	UMP_DEBUG_ASSERT(0 < ((ump_mem *)mem)->size, ("Memory size of passed handle too low"));
	UMP_DEBUG_ASSERT(NULL != ((ump_mem *)mem)->mapped_mem, ("Error in mapping pointer (not mapped)"));

	_ump_osu_lock_wait(mem->ref_lock, _UMP_OSU_LOCKMODE_RW);
	mem->ref_count -= 1;

	if (0 == mem->ref_count)
	{
		/* Remove memory mapping, which holds our only reference towards the UMP kernel space driver */
		ump_arch_unmap(mem->mapped_mem, mem->size, mem->cookie);

		_ump_osu_lock_signal(mem->ref_lock, _UMP_OSU_LOCKMODE_RW);

		/* Free the lock protecting the reference count */
		_ump_osu_lock_term(mem->ref_lock);

		/* Free the memory for this handle */
		_ump_osu_free(mem);
	}
	else
	{
		_ump_osu_lock_signal(mem->ref_lock, _UMP_OSU_LOCKMODE_RW);
	}
}
