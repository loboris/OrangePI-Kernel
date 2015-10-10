#
# Copyright (C) 2011 ARM Limited. All rights reserved.
# 
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# 
#       http://www.apache.org/licenses/LICENSE-2.0
# 
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

ifeq ($(UMP_NO_UMP),1)

UMP_SRCS = \
	$(UMP_DIR)/arch_999_no_ump/ump_frontend.c \
	$(UMP_DIR)/arch_999_no_ump/ump_ref_drv.c

else

UMP_SRCS = \
	$(UMP_DIR)/arch_011_udd/ump_frontend.c \
	$(UMP_DIR)/arch_011_udd/ump_ref_drv.c \
	$(UMP_DIR)/arch_011_udd/ump_arch.c \
	$(UMP_DIR)/os/$(UDD_OS)/ump_uku.c \
	$(UMP_DIR)/os/$(UDD_OS)/ump_osu_memory.c \
	$(UMP_DIR)/os/$(UDD_OS)/ump_osu_locks.c

endif

