#!/bin/bash

export PATH="$PWD/brandy/gcc-linaro/bin":"$PATH"

if [ "${1}" = "" ]; then
  echo "Usage: config_linux_kernel 2|plus"
  exit 0
fi

cd linux-3.4

if [ "${1}" = "plus" ]; then
  cp .config .config.bak
  cp ../build/sun8iw7p1smp_lobo_defconfig.opiplus ../build/sun8iw7p1smp_lobo_defconfig.opiplus.old
  cp ../build/sun8iw7p1smp_lobo_defconfig.opiplus .config
elif [ "${1}" = "2" ]; then
  cp .config .config.bak
  cp ../build/sun8iw7p1smp_lobo_defconfig.opi2 ../build/sun8iw7p1smp_lobo_defconfig.opi2.old
  cp ../build/sun8iw7p1smp_lobo_defconfig.opi2 .config
else
  echo "Usage: config_linux_kernel 2|plus"
  exit 0
fi

# === to edit configuration run: =========================
make ARCH=arm CROSS_COMPILE=arm-linux-gnueabi- menuconfig

if [ "${1}" = "plus" ]; then
  cp .config ../build/sun8iw7p1smp_lobo_defconfig.opiplus
elif [ "${1}" = "2" ]; then
  cp .config ../build/sun8iw7p1smp_lobo_defconfig.opi2
fi
cp .config ../build/sun8iw7p1smp_lobo_defconfig
# ========================================================

