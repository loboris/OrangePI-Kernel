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
 * @file ump_platform.h
 *
 * This file should define UMP_API_EXPORT,
 * which dictates how the UMP user space API should be exported/imported.
 * Modify this file, if needed, to match your platform setup.
 */

#ifndef __UMP_PLATFORM_H__
#define __UMP_PLATFORM_H__

/** @addtogroup ump_user_space_api
 * @{ */

/**
 * A define which controls how UMP user space API functions are imported and exported.
 * This define should be set by the implementor of the UMP API.
 */
#if defined(_WIN32)

#define UMP_API_EXPORT

#elif defined(__SYMBIAN32__)

#define UMP_API_EXPORT IMPORT_C

#else

#if defined(__GNUC__)
#if __GNUC__ >= 4
#   define MALI_VISIBLE  __attribute__ ((visibility ("default")))       /**< Function should be visible from outside the dll */
#else
#   define MALI_VISIBLE
#endif

#elif defined(__ARMCC_VERSION)
/* ARMCC specific */
#   define MALI_VISIBLE  __declspec(dllexport)

#else
#   define MALI_VISIBLE

#endif

#define UMP_API_EXPORT MALI_VISIBLE

#endif

/** @} */ /* end group ump_user_space_api */


#endif /* __UMP_PLATFORM_H__ */
