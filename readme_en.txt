1.How to use Makefile of directory osdrv:
Either one of the two toolchains(arm-hisiv100nptl-linux and arm-hisiv200-linux) could be used to compile the source code under current directory. So a parameter to specify the right toolchain should be given while compiling osdrv. The arm-hisiv100nptl-linux toolchain is for uclibc, and the arm-hisiv200-linux toolchain for glibc. Parameter CHIP is to specify the right type of chips, users can select one amoung hi3518a, hi3518c or hi3516c. Specific compiling commands are as follows:

(1)Compile the entire osdrv directory:
	Compile source code with arm-hisiv100nptl-linux toolchain.
	make OSDRV_CROSS=arm-hisiv100nptl-linux CHIP=hi3518a all
	Compile source code with arm-hisiv200-linux toolchain.
	make OSDRV_CROSS=arm-hisiv200-linux CHIP=hi3518a all

(2)Clear all compiled files under osdrv directory:
	Parameters to specify the right toolchain should be given.
	make OSDRV_CROSS=arm-hisiv100nptl-linux CHIP=hi3518a clean
	make OSDRV_CROSS=arm-hisiv200-linux CHIP=hi3518a clean

(3)Completely remove all compiled files under osdrv directory, and the generated images:
	Parameters to specify the right toolchain should be given.
	make OSDRV_CROSS=arm-hisiv100nptl-linux CHIP=hi3518a distclean
	make OSDRV_CROSS=arm-hisiv200-linux CHIP=hi3518a distclean

(4)Separately compile kernel:
	Enter the top directory the kernel source code, do the following
	Compile kernel with arm-hisiv100nptl-linux:
	cp arch/arm/configs/hi3518a_full_defconfig .config
	make ARCH=arm CROSS_COMPILE=arm-hisiv100nptl-linux- menuconfig
	make ARCH=arm CROSS_COMPILE=arm-hisiv100nptl-linux- uImage
	Compile kernel with arm-hisiv200-linux:
	cp arch/arm/configs/hi3518a_full_defconfig .config
	make ARCH=arm CROSS_COMPILE=arm-hisiv200-linux- menuconfig
	make ARCH=arm CROSS_COMPILE=arm-hisiv200-linux- uImage

(5)Separately compile uboot:
	Enter the top directory of boot source code, do the following:
	make ARCH=arm CROSS_COMPILE=arm-hisiv100nptl-linux- hi3518a_config
	make ARCH=arm CROSS_COMPILE=arm-hisiv100nptl-linux-
	The generated u-boot.bin is copied to osdrv/tools/pc_tools/uboot_tools/directory
	./mkboot.sh reg_info_.bin u-boot-ok.bin
	The generated u-boot-ok.bin is available for u-boot image
	or:
	make ARCH=arm CROSS_COMPILE=arm-hisiv200-linux- hi3518a_config
	make ARCH=arm CROSS_COMPILE=arm-hisiv200-linux-
	The generated u-boot.bin is copied to osdrv/tools/pc_tools/uboot_tools/directory
	./mkboot.sh reg_info_.bin u-boot-ok.bin
	The generated u-boot-ok.bin is available for u-boot image

(6)Build file system image:
        A compiled file system has already been in osdrv/pub/, so no need to re-build file system. What you need to do is to build the right file system image according to the flash specification of the single-board. 
	Filesystem images of jffs2 format and squashfs format are available for spi flash. While making jffs2 image or squashfs image, you need to specify the spi flash block size. flash block size will be printed when uboot runs. run mkfs.jffs2 first to get the right parameters from it's printed information. Here the block size is 256KB, for example:

	jffs2 filesystem image:
	osdrv/pub/bin/pc/mkfs.jffs2 -d osdrv/pub/rootfs_uclibc -l -e 0x40000 -o osdrv/pub/rootfs_uclibc_256k.jffs2
	or:
	osdrv/pub/bin/pc/mkfs.jffs2 -d osdrv/pub/rootfs_glibc -l -e 0x40000 -o osdrv/pub/rootfs_glibc_256k.jffs2
	squashfs filesystem image:
	osdrv/pub/bin/pc/mksquashfs osdrv/pub/rootfs_uclibc osdrv/pub/rootfs_256k.squashfs -b 256K -comp xz

2. Output directory description
All compiled images, rootfs are located in directory osdrv/pub/.
pub
│  rootfs_uclibc.tgz ------------------------------------- rootfs compiled hisiv100nptl
│  rootfs_glibc.tgz -------------------------------------- rootfs compiled hisiv200
│
├─image_glibc -------------------------------------------- Images compiled with hisiv200
│      uImage -------------------------------------------- kernel image
│      u-boot-xxxMHZ.bin --------------------------------- u-boot image(xxx: frequency of AXI bus clock)
│      rootfs_64k.squashfs ------------------------------- squashfs rootfs image(spi-flash blocksize = 64K)
│      rootfs_64k.jffs2  --------------------------------- jffs2 rootfs image(spi-flash blocksize = 64K)
│      rootfs_128k.squashfs ------------------------------ squashfs rootfs image(spi-flash blocksize = 128K)
│      rootfs_128k.jffs2 --------------------------------- jffs2 rootfs image(spi-flash blocksize = 128K)
│      rootfs_256k.squashfs ------------------------------ squashfs rootfs image(spi-flash blocksize = 256K)
│      rootfs_256k.jffs2 --------------------------------- jffs2 rootfs image(spi-flash blocksize = 256K)
│      rootfs_2k_1bit.yaffs2 ----------------------------- yaffs2 rootfs image(nand-flash pagesize=2K ecc=1bit)
│
├─image_uclibc ------------------------------------------- Images compiled with hisiv100nptl
│      uImage -------------------------------------------- kernel image
│      u-boot-xxxMHZ.bin --------------------------------- u-boot image(xxx: frequency of AXI bus clock)
│      rootfs_64k.squashfs ------------------------------- squashfs rootfs image(spi-flash blocksize = 64K)
│      rootfs_64k.jffs2  --------------------------------- jffs2 rootfs image(spi-flash blocksize = 64K)
│      rootfs_128k.squashfs ------------------------------ squashfs rootfs image(spi-flash blocksize = 128K)
│      rootfs_128k.jffs2 --------------------------------- jffs2 rootfs image(spi-flash blocksize = 128K)
│      rootfs_256k.squashfs ------------------------------ squashfs rootfs image(spi-flash blocksize = 256K)
│      rootfs_256k.jffs2 --------------------------------- jffs2 rootfs image(spi-flash blocksize = 256K)
│      rootfs_2k_1bit.yaffs2 ----------------------------- yaffs2 rootfs image(nand-flash pagesize=2K ecc=1bit)
│
└─bin
    ├─pc
    │      mkfs.jffs2
    │      mkimage
    │      mkfs.cramfs
    │      mkyaffs2image
    │      mksquashfs
    │      lzma
    │
    ├─board_glibc ---------------------------------------- tools used on board
    │   ├─hifat ------------------------------------------ hifat tool
    │   │    ├─shared
    │   │    │     himount
    │   │    │     libfs_lib.so
    │   │    │     libfuse.so
    │   │    │     libfuse.so.2
    │   │    │     libfuse.so.2.8.3
    │   │    │     libsdparm.so
    │   │    │     libulockmgr.so
    │   │    │     libulockmgr.so.1
    │   │    │     libulockmgr.so.1.0.1
    │   │    │     libhimount_api.so
    │   │    ├─static
    │   │    │     libhimount_api.a
    │   │    │     himount
    │   │    └─how_to_use_[chs].txt
    │   flash_eraseall
    │   flash_erase
    │   nandwrite
    │   mtd_debug
    │   flash_info
    │   sumtool
    │   mtdinfo
    │   flashcp
    │   nandtest
    │   nanddump
    │   gdb-arm-hisiv200-linux
    │
    └─board_uclibc --------------------------------------- tools used on board
        ├─hifat ------------------------------------------ hifat tools
        │    ├─shared
        │    │     himount
        │    │     libfs_lib.so
        │    │     libfuse.so
        │    │     libfuse.so.2
        │    │     libfuse.so.2.8.3
        │    │     libsdparm.so
        │    │     libulockmgr.so
        │    │     libulockmgr.so.1
        │    │     libulockmgr.so.1.0.1
        │    │     libhimount_api.so
        │    ├─static
        │    │     libhimount_api.a
        │    │     himount
        │    └─how_to_use_[chs].txt
        flash_eraseall
        flash_erase
        nandwrite
        mtd_debug
        flash_info
        sumtool
        mtdinfo
        flashcp
        nandtest
        gdb-arm-hisiv100nptl-linux
        nanddump


3.osdrv directory structure：
osdrv
├─Makefile ------------------------------ osdrv compile script
├─busybox ------------------------------- busybox source code
├─tools --------------------------------- directory of all tools
│  ├─board_tools ------------------------ A variety of single-board tools
│  │  ├─reg-tools-1.0.0 ----------------- tools for accessing memory space and io space
│  │  ├─mtd-utils ----------------------- tool to read and write flash nude
│  │  ├─udev-100 ------------------------ udev toolset
│  │  ├─gdb ----------------------------- gdb tools
│  │  └─e2fsprogs ----------------------- mkfs tools
│  └─pc_tools --------------------------- tools used on PC
│      ├─mkfs.cramfs -------------------- tool for making cramfs file system
│      ├─squashfs4.2 -------------------- tool for making mksquashfs file system
│      ├─mkfs.xfs ----------------------- tool for making xfs file system
│      ├─mkimage ------------------------ tool for making uImage
│      ├─lzma-4.32.7 -------------------- lzma compress tool
│      ├─mkfs.jffs2 --------------------- tool for making jffs2 file system
│      ├─mkyaffs2image301 --------------- tool for making yaffs2 file system
│      └─uboot_tools -------------------- tools for creating uboot image,xls files,ddr initialization script
├─toolchain ----------------------------- directory contains toolchains
│  ├─arm-hisiv100nptl-linux ------------- hisiv100nptl cross tool chain
│  └─arm-hisiv200-linux ----------------- hisiv200 cross tool chain
├─pub ----------------------------------- output directory
│  ├─image_glibc ------------------------ images compiled with hisiv200:uboot,uImage and images of filesystem
│  ├─image_uclibc ----------------------- images with hisiv100nptl:uboot,uImage and images of filesystem
│  ├─bin -------------------------------- tools not placed in the rootfs
│  │  ├─pc ------------------------------ tools used on the PC
│  │  ├─board_glibc --------------------- board tools compiled with hisiv100nptl
│  │  └─board_uclibc -------------------- board tools compiled with hisiv200
│  ├─rootfs_uclibc.tgz ------------------ rootfs compiled with hisiv100nptl
│  └─rootfs_glibc.tgz ------------------- rootfs compiled with hisiv200
├─rootfs_scripts ------------------------ scripts to generate rootfs directory
├─uboot --------------------------------- uboot source code
└─kernel -------------------------------- kernel source code


4.Note
(1)If a tool chain needs to be replaced by the other, remove the original compiled files compiled with the former tool chain, and then replace the compiler tool chain with the other. 
(2)Executable files in Linux may become non-executable after copying them to somewhere else under Windows, and result in souce code cannot be compiled. Many symbolic link files will be generated in the souce package after compiling the u-boot or the kernel. The volume of the source package will become very big, because all the symbolic link files are turning out to be real files in Windows. So, DO NOT copy source package in Windows.
