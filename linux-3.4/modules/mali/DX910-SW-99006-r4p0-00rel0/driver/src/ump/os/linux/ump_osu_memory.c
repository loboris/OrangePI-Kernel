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

#include <ump/ump_osu.h>

#include <stdlib.h>
#include <string.h> /* memcmp, memchr, memset */

/**
 * @file ump_osu_memory.c
 * File implements the user side of the OS interface
 */

void *_ump_osu_calloc(u32 n, u32 size)
{
	return calloc(n, size);
}

void *_ump_osu_malloc(u32 size)
{
	return malloc(size);
}

void *_ump_osu_realloc(void *ptr, u32 size)
{
	return realloc(ptr, size);
}

void _ump_osu_free(void *ptr)
{
	free(ptr);
}

void *_ump_osu_memcpy(void *dst, const void *src, u32   len)
{
	return memcpy(dst, src, len);
}

void *_ump_osu_memset(void *ptr, u32 chr, u32 size)
{
	return memset(ptr, chr, size);
}

int _ump_osu_memcmp(const void *ptr1, const void *ptr2, u32 size)
{
	return memcmp(ptr1, ptr2, size);
}
