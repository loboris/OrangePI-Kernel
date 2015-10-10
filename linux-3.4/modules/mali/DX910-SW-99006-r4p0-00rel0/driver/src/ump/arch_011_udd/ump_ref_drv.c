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

/**
 * @file ump_ref_drv.c
 *
 * Implementation of the user space API extensions provided by the reference implementation.
 */

#include <ump/ump_ref_drv.h>
#include <ump/ump.h>
#include "ump_internal.h"
#include "ump_arch.h"
#include <ump/ump_debug.h>
#include <ump/ump_osu.h>

/* Allocate a buffer which can be used directly by hardware, 4kb aligned */
static ump_handle ump_ref_drv_allocate_internal(unsigned long size, ump_alloc_constraints constraints, ump_cache_enabled cache);



/* Allocate a buffer which can be used directly by hardware, 4kb aligned */
ump_handle ump_ref_drv_allocate(unsigned long size, ump_alloc_constraints constraints)
{
	ump_cache_enabled cache = UMP_CACHE_DISABLE;

	if (0 != (constraints & UMP_REF_DRV_CONSTRAINT_USE_CACHE))
	{
		cache = UMP_CACHE_ENABLE;
	}

	return ump_ref_drv_allocate_internal(size, constraints, cache);
}

UMP_API_EXPORT int ump_cpu_msync_now(ump_handle memh, ump_cpu_msync_op op, void *address, int size)
{
	int offset;
	ump_mem *mem = (ump_mem *)memh;
	UMP_DEBUG_ASSERT(UMP_INVALID_MEMORY_HANDLE != memh, ("Handle is invalid"));

	/* If the op is readout, we do the readout from DD.
	   Else we skip flushing if the userspace handle says that it is uncached */
	if ((UMP_MSYNC_READOUT_CACHE_ENABLED != op) && (0 == mem->is_cached))
	{
		return 0;
	}

	if (NULL == address)
	{
		address = ((ump_mem *)mem)->mapped_mem;
	}

	offset = (int)((unsigned long)address - (unsigned long)((ump_mem *)mem)->mapped_mem);

	if (0 == size)
	{
		size = (int)((ump_mem *)mem)->size;
	}

	UMP_DEBUG_ASSERT(0 < (((ump_mem *)mem)->ref_count), ("Reference count too low"));
	UMP_DEBUG_ASSERT((size >= 0) && (size <= (int)((ump_mem *)mem)->size), ("Memory size of passed handle too low"));
	UMP_DEBUG_ASSERT(NULL != ((ump_mem *)mem)->mapped_mem, ("Error in mapping pointer (not mapped)"));

	if ((offset + size) > (int)mem->size)
	{
		size = mem->size - offset;
	}

	mem->is_cached = ump_arch_msync(mem->secure_id, mem->mapped_mem, mem->cookie, address, size, op);
	return mem->is_cached ;
}

UMP_API_EXPORT int ump_cache_operations_control(ump_cache_op_control op)
{
	return ump_arch_cache_operations_control(op);
}

UMP_API_EXPORT int ump_switch_hw_usage(ump_handle memh, ump_hw_usage new_user)
{
	ump_mem *mem = (ump_mem *)memh;
	UMP_DEBUG_ASSERT(UMP_INVALID_MEMORY_HANDLE != memh, ("Handle is invalid"));
	return ump_arch_switch_hw_usage(mem->secure_id, new_user);
}

UMP_API_EXPORT int ump_lock(ump_handle memh, ump_lock_usage lock_usage)
{
	ump_mem *mem = (ump_mem *)memh;
	UMP_DEBUG_ASSERT(UMP_INVALID_MEMORY_HANDLE != memh, ("Handle is invalid"));
	return ump_arch_lock(mem->secure_id, lock_usage);
}

UMP_API_EXPORT int ump_unlock(ump_handle memh)
{
	ump_mem *mem = (ump_mem *)memh;
	UMP_DEBUG_ASSERT(UMP_INVALID_MEMORY_HANDLE != memh, ("Handle is invalid"));
	return ump_arch_unlock(mem->secure_id);
}

UMP_API_EXPORT int ump_switch_hw_usage_secure_id(ump_secure_id ump_id, ump_hw_usage new_user)
{
	return ump_arch_switch_hw_usage(ump_id, new_user);
}

/** Locking buffer. Blocking call if the buffer is already locked. */
UMP_API_EXPORT int ump_lock_secure_id(ump_secure_id ump_id, ump_lock_usage lock_usage)
{
	return ump_arch_lock(ump_id, lock_usage);
}

/** Unlocking buffer. Let other users lock the buffer for their usage */
UMP_API_EXPORT int ump_unlock_secure_id(ump_secure_id ump_id)
{
	return ump_arch_unlock(ump_id);
}

/* Allocate a buffer which can be used directly by hardware, 4kb aligned */
static ump_handle ump_ref_drv_allocate_internal(unsigned long size, ump_alloc_constraints constraints, ump_cache_enabled cache)
{
	ump_secure_id secure_id;
	unsigned long allocated_size = size;

	UMP_DEBUG_PRINT(4, ("Allocating UMP memory of size %lu", size));

	secure_id = ump_arch_allocate(&allocated_size, constraints);

	if (secure_id != UMP_INVALID_SECURE_ID)
	{
		unsigned long cookie;
		void *mapping;

		mapping = ump_arch_map(secure_id, allocated_size, cache, &cookie);

		if (NULL != mapping)
		{
			/*
			 * PS: By now we have actually increased the ref count in the device driver by 2,
			 * one for the allocation iteself, and one for the mapping.
			 */
			ump_mem *mem;
			mem = _ump_osu_calloc(1, sizeof(*mem));

			if (NULL != mem)
			{
				mem->secure_id = secure_id;
				mem->mapped_mem = mapping;
				mem->size = allocated_size;
				mem->cookie = cookie;
				mem->is_cached = UMP_CACHE_ENABLE; /* Default to ON, is disabled later if not */

				_ump_osu_lock_auto_init(&mem->ref_lock, _UMP_OSU_LOCKFLAG_DEFAULT, 0, 0);
				UMP_DEBUG_ASSERT(NULL != mem->ref_lock, ("Failed to initialize lock\n"));
				mem->ref_count = 1;

				/*
				 * ump_arch_allocate() gave us a kernel space reference, and the same did ump_arch_map()
				 * We release the one from ump_arch_allocate(), and rely solely on the one from the ump_arch_map()
				 * That is, ump_arch_unmap() should now do the final release towards the UMP kernel space driver.
				 */
				ump_arch_reference_release(secure_id);

				/* This is called only to set the cache settings in this handle */
				ump_cpu_msync_now((ump_handle)mem, UMP_MSYNC_READOUT_CACHE_ENABLED, NULL, 0);

				UMP_DEBUG_PRINT(4, ("UMP handle created for ID %u of size %lu, mapped into address 0x%08lx", mem->secure_id, mem->size, (unsigned long)mem->mapped_mem));

				return (ump_handle)mem;
			}

			ump_arch_unmap(mapping, allocated_size, cookie); /* Unmap the memory */
			ump_arch_reference_release(secure_id); /* Release reference added when we allocated the UMP memory */
		}

		ump_arch_reference_release(secure_id); /* Release reference added when we allocated the UMP memory */
	}

	UMP_DEBUG_PRINT(4, ("Allocation of UMP memory failed"));
	return UMP_INVALID_MEMORY_HANDLE;
}
