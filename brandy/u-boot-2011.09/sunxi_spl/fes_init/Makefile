#
# (C) Copyright 2000-2011
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# (C) Copyright 2011
# Daniel Schwierzeck, daniel.schwierzeck@googlemail.com.
#
# (C) Copyright 2011
# Texas Instruments Incorporated - http://www.ti.com/
# Aneesh V <aneesh@ti.com>
#
# This file is released under the terms of GPL v2 and any later version.
# See the file COPYING in the root directory of the source tree for details.
#
# Based on top-level Makefile.
#

include $(TOPDIR)/config.mk

CONFIG_SPL := y
export CONFIG_SPL

FES_LDSCRIPT := $(TOPDIR)/sunxi_spl/fes_init/main/fes_init.lds


# We want the final binaries in this directory
obj := $(OBJTREE)/sunxi_spl/fes_init/

FES_HEAD  := main/fes_head.o
START := main/fes1_entry.o

LIBS-y += sunxi_spl/fes_init/spl/libsource_spl.o
LIBS-y += sunxi_spl/fes_init/main/libmain.o
LIBS-y += sunxi_spl/spl/lib/libgeneric.o
LIBS-y += arch/$(ARCH)/cpu/$(CPU)/$(SOC)/dram/libdram.o

LIBS := $(addprefix $(OBJTREE)/,$(sort $(LIBS-y)))


__LIBS := $(subst $(obj),,$(LIBS))

# Linker Script
# ifdef CONFIG_SPL_LDSCRIPT
# need to strip off double quotes
# LDSCRIPT := $(addprefix $(SRCTREE)/,$(subst ",,$(CONFIG_SPL_LDSCRIPT)))
# endif
# ifeq ($(wildcard $(LDSCRIPT)),)
# 	LDSCRIPT := $(TOPDIR)/board/$(BOARDDIR)/u-boot-spl.lds
# endif
# ifeq ($(wildcard $(LDSCRIPT)),)
# 	LDSCRIPT := $(TOPDIR)/$(CPUDIR)/u-boot-spl.lds
# endif
# ifeq ($(wildcard $(LDSCRIPT)),)
# $(error could not find linker script)
# endif



# Special flags for CPP when processing the linker script.
# Pass the version down so we can handle backwards compatibility
# on the fly.
LDPPFLAGS += \
	-include $(TOPDIR)/include/u-boot/u-boot.lds.h \
	-DFES1ADDR=$(CONFIG_FES1_RUN_ADDR)	 \
	$(shell $(LD) --version | \
	  sed -ne 's/GNU ld version \([0-9][0-9]*\)\.\([0-9][0-9]*\).*/-DLD_MAJOR=\1 -DLD_MINOR=\2/p')

ALL-y	+= $(obj)fes1.bin

all:	$(ALL-y)


$(obj)fes1.bin:	$(obj)fes1.axf
	$(OBJCOPY) $(OBJCFLAGS) -O binary $< $@

GEN_UBOOT = \
	UNDEF_SYM=`$(OBJDUMP) -x $(LIBS) | \
	sed  -n -e 's/.*\($(SYM_PREFIX)__u_boot_cmd_.*\)/-u\1/p'|sort|uniq`;\
	cd $(obj) && $(LD) $(LDFLAGS) -T$(obj)fes_init.lds $(LDFLAGS_$(@F)) $$UNDEF_SYM  \
	--start-group $(__LIBS) --end-group \
	-Map fes1.map -o fes1.axf

$(obj)fes1.axf: depend $(LIBS) $(obj)fes_init.lds
	$(LD) $(LIBS) $(LDFLAGS) -T$(obj)fes_init.lds -o fes1.axf -Map fes1.map
#	$(GEN_UBOOT)

$(LIBS):	depend
	$(MAKE) -C $(SRCTREE)$(dir $(subst $(OBJTREE),,$@))

$(obj)fes_init.lds: $(FES_LDSCRIPT)
	$(CPP) $(CPPFLAGS) $(LDPPFLAGS) -ansi -D__ASSEMBLY__ -P - <$^ >$@

depend:	$(obj).depend
.PHONY: depend

# defines $(obj).depend target
include $(SRCTREE)/rules.mk
