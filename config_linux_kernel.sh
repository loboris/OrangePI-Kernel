#!/bin/bash

export PATH="$PWD/brandy/gcc-linaro/bin":"$PATH"

cd linux-3.4

# === to edit configuration run: =========================
cp ../build/sun8iw7p1smp_lobo_defconfig .config
cp .config ../build/sun8iw7p1smp_lobo_defconfig.old
cp .config .config.bak
make ARCH=arm CROSS_COMPILE=arm-linux-gnueabi- menuconfig
cp .config ../build/sun8iw7p1smp_lobo_defconfig
# ========================================================

