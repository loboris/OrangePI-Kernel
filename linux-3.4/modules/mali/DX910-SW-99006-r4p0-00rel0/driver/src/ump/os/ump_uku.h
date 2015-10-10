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
 * @file ump_uku.h
 * Defines the user-side interface of the user-kernel interface
 */

#ifndef __UMP_UKU_H__
#define __UMP_UKU_H__

#include <ump/ump_osu.h>
#include <ump/ump_debug.h>
#include <ump/ump_uk_types.h>

#ifdef __cplusplus
extern "C" {
#endif

_ump_osu_errcode_t _ump_uku_open(void **context);

_ump_osu_errcode_t _ump_uku_close(void **context);

_ump_osu_errcode_t _ump_uku_allocate(_ump_uk_allocate_s *args);

_ump_osu_errcode_t _ump_uku_release(_ump_uk_release_s *args);

_ump_osu_errcode_t _ump_uku_size_get(_ump_uk_size_get_s *args);

_ump_osu_errcode_t _ump_uku_get_api_version(_ump_uk_api_version_s *args);

int _ump_uku_map_mem(_ump_uk_map_mem_s *args);

void _ump_uku_unmap_mem(_ump_uk_unmap_mem_s *args);

void _ump_uku_msynch(_ump_uk_msync_s *args);

int _ump_uku_map_mem(_ump_uk_map_mem_s *args);

void _ump_uku_cache_operations_control(_ump_uk_cache_operations_control_s *args);
void _ump_uku_switch_hw_usage(_ump_uk_switch_hw_usage_s *dd_msync_call_arg);
void _ump_uku_lock(_ump_uk_lock_s  *dd_msync_call_arg);
void _ump_uku_unlock(_ump_uk_unlock_s *dd_msync_call_arg);



#ifdef __cplusplus
}
#endif

#endif /* __UMP_UKU_H__ */
