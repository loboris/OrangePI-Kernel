Linux Kernel for OrangePI H3 boards
===================================

About
-----

The repository contains Linux kernel sources (3.4.39) adapted for OrangePI H3 boards, gcc toolchain, adapted rootfs and building scripts.

Building
--------

Kernel config files and the files specific to OPI board are placed in **build** directory.

The included build script *build_linux_kernel.sh* can be used to build the kernel
`./build_linux_kernel.sh [clean | all | 2 | plus] [clean]`

**clean** as 1st parameter cleans the kernell tree and build directories

**clean** as 2nd parameter cleans the kernell tree before build

**all** builds the uImage for OPI-2 & OPI-PLUS

**2** builds the uImage for OPI-2

**plus** builds the uImage for OPI-PLUS

After the build the resulting kernel files (uImage and kernel modules) are placed into **build** directory.

To build script.bin for all OPI boards and resolutions run:
`./build_scripts`
