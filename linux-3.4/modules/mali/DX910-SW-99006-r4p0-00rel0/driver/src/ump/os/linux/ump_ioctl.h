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

#ifndef __UMP_IOCTL_H__
#define __UMP_IOCTL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <linux/types.h>
#include <linux/ioctl.h>

#include <ump_uk_types.h>

#ifndef __user
#define __user
#endif


/**
 * @file UMP_ioctl.h
 * This file describes the interface needed to use the Linux device driver.
 * The interface is used by the userpace UMP driver.
 */

#define UMP_IOCTL_NR 0x90


#define UMP_IOC_QUERY_API_VERSION _IOR(UMP_IOCTL_NR, _UMP_IOC_QUERY_API_VERSION, _ump_uk_api_version_s)
#define UMP_IOC_ALLOCATE  _IOWR(UMP_IOCTL_NR,  _UMP_IOC_ALLOCATE,  _ump_uk_allocate_s)
#define UMP_IOC_RELEASE  _IOR(UMP_IOCTL_NR,  _UMP_IOC_RELEASE,  _ump_uk_release_s)
#define UMP_IOC_SIZE_GET  _IOWR(UMP_IOCTL_NR,  _UMP_IOC_SIZE_GET, _ump_uk_size_get_s)
#define UMP_IOC_MSYNC     _IOW(UMP_IOCTL_NR,  _UMP_IOC_MSYNC, _ump_uk_msync_s)

#define UMP_IOC_CACHE_OPERATIONS_CONTROL _IOW(UMP_IOCTL_NR,  _UMP_IOC_CACHE_OPERATIONS_CONTROL, _ump_uk_cache_operations_control_s)
#define UMP_IOC_SWITCH_HW_USAGE   _IOW(UMP_IOCTL_NR,  _UMP_IOC_SWITCH_HW_USAGE, _ump_uk_switch_hw_usage_s)
#define UMP_IOC_LOCK          _IOW(UMP_IOCTL_NR,  _UMP_IOC_LOCK, _ump_uk_lock_s)
#define UMP_IOC_UNLOCK        _IOW(UMP_IOCTL_NR,  _UMP_IOC_UNLOCK, _ump_uk_unlock_s)


#ifdef __cplusplus
}
#endif

#endif /* __UMP_IOCTL_H__ */
