/*
 * Copyright (C) 2011 ARM Limited. All rights reserved.
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
Building the UMP user space library for Linux
---------------------------------------------

A simple Makefile is provided, and the UMP user space library can be built
simply by issuing make. This Makefile is setup to use the ARM GCC compiler
from CodeSourcery, and it builds for ARMv6. Modification to this Makefile
is needed in order to build for other configurations.

In order to use this library from the Mali GPU driver, invoke the Mali GPU
driver build system with the following two make variables set;
- UMP_INCLUDE_DIR should point to the include folder inside this package
- UMP_LIB should point to the built library (libUMP.so)

This does not apply to Android builds, where the Android.mk file for the
Mali GPU driver needs to be manually edited in order to add the correct
include path and link against the correct library.
