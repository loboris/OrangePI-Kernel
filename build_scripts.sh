#!/bin/bash


# **********************************************************************************
# build/orange_pi.h                                                                *
# build/orange_pi2.fex contains orange pi2 specific configuration                  *
# build/orange_pi_plus.fex contains orange pi PLUS specific configuration          *
# build/orange_pi_pc.fex contains orange pi PC specific configuration              *
# **********************************************************************************

### RUN Under lichee/tools/pack !!

# ==================================================================================
# !!! Before building check env settings in:
# orange_pi2.h -> #define CONFIG_EXTRA_ENV_SETTINGS !!!

# ==================================================================================
# Fex file compiler requires fex file in CRLF format
# dos2unix build/sys_config.fex

#=============
build_script() {
	# === Building boot0 image =========================================================
	echo "Building script.bin.${1}_${2}_${3}"
	
	if [ "${1}" = "OPI-PLUS" ]; then
		cp build/orange_pi_plus.fex build/sys_config.fex
	elif [ "${1}" = "OPI-2" ]; then
		cp build/orange_pi2.fex build/sys_config.fex
	elif [ "${1}" = "OPI-PC" ]; then
		cp build/orange_pi_pc.fex build/sys_config.fex
	else
		return 0
	fi
        if [ "${3}" = "dvi" ]; then
                cat build/sys_config.fex | sed s/";hdcp_enable            = 0"/"hdcp_enable            = 0"/g > build/_sys_config.fex
                cat build/sys_config.fex | sed s/";hdmi_cts_compatibility = 1"/"hdmi_cts_compatibility = 1"/g > build/_sys_config.fex
                mv build/_sys_config.fex build/sys_config.fex
        fi
	if [ "${2}" = "720p50" ]; then
		cat build/sys_config.fex | sed s/"screen0_output_mode      = 10"/"screen0_output_mode      = 4"/g > build/_sys_config.fex
		mv build/_sys_config.fex build/sys_config.fex
		cat build/sys_config.fex | sed s/"screen1_output_mode      = 10"/"screen1_output_mode      = 4"/g > build/_sys_config.fex
		mv build/_sys_config.fex build/sys_config.fex
	elif [ "${2}" = "720p60" ]; then
		cat build/sys_config.fex | sed s/"screen0_output_mode      = 10"/"screen0_output_mode      = 5"/g > build/_sys_config.fex
		mv build/_sys_config.fex build/sys_config.fex
		cat build/sys_config.fex | sed s/"screen1_output_mode      = 10"/"screen1_output_mode      = 5"/g > build/_sys_config.fex
		mv build/_sys_config.fex build/sys_config.fex
	elif [ "${2}" = "1080p50" ]; then
		cat build/sys_config.fex | sed s/"screen0_output_mode      = 10"/"screen0_output_mode      = 9"/g > build/_sys_config.fex
		mv build/_sys_config.fex build/sys_config.fex
		cat build/sys_config.fex | sed s/"screen1_output_mode      = 10"/"screen1_output_mode      = 9"/g > build/_sys_config.fex
		mv build/_sys_config.fex build/sys_config.fex
	elif [ "${2}" = "480p" ]; then
                cat build/sys_config.fex | sed s/"screen0_output_mode      = 10"/"screen0_output_mode      = 31"/g > build/_sys_config.fex
		mv build/_sys_config.fex build/sys_config.fex
                cat build/sys_config.fex | sed s/"screen1_output_mode      = 10"/"screen1_output_mode      = 31"/g > build/_sys_config.fex
		mv build/_sys_config.fex build/sys_config.fex
	fi

	cp build/sys_config.fex chips/sun8iw7p1/configs/dolphin-p1/sys_config.fex

	cp build/orange_pi.h brandy/u-boot-2011.09/include/configs/sun8iw7p1.h

	cp chips/sun8iw7p1/bin/boot0_sdcard_sun8iw7p1.bin build/boot0_sdcard.fex

	fex2bin build/sys_config.fex build/sys_config.bin
	if [ $? -ne 0 ]; then
	  echo "  Error."
	  exit 1
	fi
	pctools/linux/mod_update/update_boot0 build/boot0_sdcard.fex build/sys_config.bin SDMMC_CARD > build_script_${1}_${2}.log 2>&1
	if [ $? -ne 0 ]; then
	  echo "  Error."
	  exit 1
	fi
	cp build/sys_config.bin build/script.bin.${1}_${2}_${3}
	rm build_script_${1}_${2}.log > /dev/null 2>&1
}


echo ""
rm build_script_*.log > /dev/null 2>&1
rm build/script.bin.OPI-* > /dev/null 2>&1
rm build/sys_config* > /dev/null 2>&1
if [ "${1}" = "clean" ]; then
    echo "Cleaning."
    exit 0
fi

build_script "OPI-2" "1080p60" "hdmi"
build_script "OPI-2" "1080p50" "hdmi"
build_script "OPI-2" "720p50" "hdmi"
build_script "OPI-2" "720p60" "hdmi"
build_script "OPI-2" "480p" "hdmi"

build_script "OPI-PLUS" "1080p60" "hdmi"
build_script "OPI-PLUS" "1080p50" "hdmi"
build_script "OPI-PLUS" "720p50" "hdmi"
build_script "OPI-PLUS" "720p60" "hdmi"
build_script "OPI-PLUS" "480p" "hdmi"

build_script "OPI-PC" "1080p60" "hdmi"
build_script "OPI-PC" "1080p50" "hdmi"
build_script "OPI-PC" "720p50" "hdmi"
build_script "OPI-PC" "720p60" "hdmi"
build_script "OPI-PC" "480p" "hdmi"

build_script "OPI-2" "1080p60" "dvi"
build_script "OPI-2" "1080p50" "dvi"
build_script "OPI-2" "720p50" "dvi"
build_script "OPI-2" "720p60" "dvi"
build_script "OPI-2" "480p" "dvi"

build_script "OPI-PLUS" "1080p60" "dvi"
build_script "OPI-PLUS" "1080p50" "dvi"
build_script "OPI-PLUS" "720p50" "dvi"
build_script "OPI-PLUS" "720p60" "dvi"
build_script "OPI-PLUS" "480p" "dvi"

build_script "OPI-PC" "1080p60" "dvi"
build_script "OPI-PC" "1080p50" "dvi"
build_script "OPI-PC" "720p50" "dvi"
build_script "OPI-PC" "720p60" "dvi"
build_script "OPI-PC" "480p" "dvi"

echo "END."

# ==================================================================================
