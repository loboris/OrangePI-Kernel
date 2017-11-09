#!/bin/bash

echo "  Building mali drivers..."

export PATH="$PWD/brandy/gcc-linaro/bin":"$PATH"
cross_comp="arm-linux-gnueabi"

SCRIPT_DIR=`pwd`

cd linux-3.4

# ####################################
# Copy config file to config directory
#cp ../build/sun8iw7p1smp_lobo_defconfig arch/arm/configs

make ARCH=arm CROSS_COMPILE=${cross_comp}- sun8iw7p1smp_lobo_defconfig > ../malibuild.log 2>&1
if [ $? -ne 0 ]; then
    echo "  Error: defconfig."
    exit 1
fi

export LICHEE_PLATFORM=linux
export KERNEL_VERSION=`make ARCH=arm CROSS_COMPILE=${cross_comp}- -s kernelversion -C ./`

LICHEE_KDIR=`pwd`
KDIR=`pwd`
export LICHEE_MOD_DIR=${LICHEE_KDIR}/output/lib/modules/${KERNEL_VERSION}
mkdir -p $LICHEE_MOD_DIR/kernel/drivers/gpu/mali >> ../malibuild.log 2>&1
mkdir -p $LICHEE_MOD_DIR/kernel/drivers/gpu/ump >> ../malibuild.log 2>&1

export LICHEE_KDIR
export MOD_DIR=${LICHEE_KDIR}/output/lib/modules/${KERNEL_VERSION}
export KDIR

cd modules/mali
make -j $(nproc) ARCH=arm CROSS_COMPILE=${cross_comp}- clean >> ../malibuild.log 2>&1
if [ $? -ne 0 ]; then
    echo "  Error: clean."
    exit 1
fi
make -j $(nproc) ARCH=arm CROSS_COMPILE=${cross_comp}- build >> ../malibuild.log 2>&1
if [ $? -ne 0 ]; then
    echo "  Error: build."
    exit 1
fi
make -j $(nproc) ARCH=arm CROSS_COMPILE=${cross_comp}- install >> ../malibuild.log 2>&1
if [ $? -ne 0 ]; then
    echo "  Error: install."
    exit 1
fi

cd ..
cd ..
echo "  mali build OK."
exit 0
