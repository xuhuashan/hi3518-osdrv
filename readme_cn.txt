1.osdrv 顶层 Makefile 使用说明
本目录下的编译脚本支持选用下文提到的两种工具链中的任何一种进行编译，因此编译时需要带上一个编译参数以指定对应的工具链 -- arm-hisiv100nptl-linux 和 arm-hisiv200-linux。其中，arm-hisiv100nptl-linux工具链对应uclibc库，arm-hisiv200-linux工具链对应glibc库；CHIP参数指定芯片类型，用户可以选择hi3518a、hi3518c或者hi3516c中的任何一种。具体命令如下

(1)编译整个osdrv目录：
	采用arm-hisiv100nptl-linux工具链进行编译：
	make OSDRV_CROSS=arm-hisiv100nptl-linux CHIP=hi3518a all
	采用arm-hisiv200-linux工具链进行编译：
	make OSDRV_CROSS=arm-hisiv200-linux CHIP=hi3518a all

(2)清除整个osdrv目录的编译文件：
	请指定编译参数，以清除某个具体版本的编译文件：
	make OSDRV_CROSS=arm-hisiv100nptl-linux CHIP=hi3518a clean
	或者
	make OSDRV_CROSS=arm-hisiv200-linux CHIP=hi3518a clean

(3)彻底清除整个osdrv目录的编译文件，除清除编译文件外，还删除已编译好的镜像：
	请指定编译参数，以清除某个具体版本的编译文件：
	make OSDRV_CROSS=arm-hisiv100nptl-linux CHIP=hi3518a distclean
	或者
	make OSDRV_CROSS=arm-hisiv200-linux CHIP=hi3518a distclean

(4)单独编译kernel：
	待进入内核源代码目录后，执行以下操作
	采用arm-hisiv100nptl-linux工具链编译内核：
	cp arch/arm/configs/hi3518a_full_defconfig .config
	make ARCH=arm CROSS_COMPILE=arm-hisiv100nptl-linux- menuconfig
	make ARCH=arm CROSS_COMPILE=arm-hisiv100nptl-linux- uImage
	采用arm-hisiv200-linux工具链编译内核：
	cp arch/arm/configs/hi3518a_full_defconfig .config
	make ARCH=arm CROSS_COMPILE=arm-hisiv200-linux- menuconfig
	make ARCH=arm CROSS_COMPILE=arm-hisiv200-linux- uImage

(5)单独编译uboot：
	待进入boot源代码目录后，执行以下操作
	make ARCH=arm CROSS_COMPILE=arm-hisiv100nptl-linux- hi3518a_config
	make ARCH=arm CROSS_COMPILE=arm-hisiv100nptl-linux-
	将生成的u-boot.bin 复制到osdrv/tools/pc_tools/uboot_tools/目录下
	运行./mkboot.sh reg_info_.bin u-boot-ok.bin
	生成的u-boot-ok.bin即为可用的u-boot镜像
	或者
	make ARCH=arm CROSS_COMPILE=arm-hisiv200-linux- hi3518a_config
	make ARCH=arm CROSS_COMPILE=arm-hisiv200-linux-
	将生成的u-boot.bin 复制到osdrv/tools/pc_tools/uboot_tools/目录下
	运行./mkboot.sh reg_info_.bin u-boot-ok.bin
	生成的u-boot-ok.bin即为可用的u-boot镜像	

(6)制作文件系统镜像：
在osdrv/pub/中有已经编译好的文件系统，因此无需再重复编译文件系统，只需要根据单板上flash的规格型号制作文件系统镜像即可。

	spi flash可以使用jffs2格式的文件系统或者squashfs文件系统（只读）。制作jffs2或者squashfs镜像时，需要用到spi flash的块大小。这些信息会在uboot启动时会打印出来,请根据打印信息填写相关参数。下面以块大小为256KB为例：

	jffs2 文件系统镜像：
	osdrv/pub/bin/pc/mkfs.jffs2 -d osdrv/pub/rootfs_uclibc -l -e 0x40000 -o osdrv/pub/rootfs_uclibc_256k.jffs2
	或者
	osdrv/pub/bin/pc/mkfs.jffs2 -d osdrv/pub/rootfs_glibc -l -e 0x40000 -o osdrv/pub/rootfs_glibc_256k.jffs2

	squashfs 文件系统镜像：
	osdrv/pub/bin/pc/mksquashfs osdrv/pub/rootfs_uclibc osdrv/pub/rootfs_256k.squashfs -b 256K -comp xz


2. 镜像存放目录说明
内核和文件系统按默认配置和最小化配置两个版本进行编译。编译完的image，rootfs等存放在osdrv/pub目录下
pub
│  rootfs_uclibc.tgz ------------------------------------------ hisiv100nptl编译出的rootfs文件系统
│  rootfs_glibc.tgz ------------------------------------------- hisiv200编译出的rootfs文件系统
│
├─image_glibc ------------------------------------------------- hisiv200编译出的镜像文件
│      uImage ------------------------------------------------- kernel镜像
│      u-boot-xxxMHZ.bin -------------------------------------- u-boot镜像(其中xxx表示AXI总线频率)
│      rootfs_64k.squashfs ------------------------------------ 64K squashfs 文件系统镜像
│      rootfs_64k.jffs2 --------------------------------------- 64K jffs2 文件系统镜像
│      rootfs_128k.squashfs ----------------------------------- 128K squashfs 文件系统镜像
│      rootfs_128k.jffs2 -------------------------------------- 128K jffs2 文件系统镜像
│      rootfs_256k.squashfs ----------------------------------- 256K squashfs 文件系统镜像
│      rootfs_256k.jffs2 -------------------------------------- 256K jffs2 文件系统镜像
│      rootfs_2k_1bit.yaffs2 ---------------------------------- yaffs2 文件系统镜像
│
├─image_uclibc ------------------------------------------------ hisiv100nptl编译出的镜像文件
│      uImage ------------------------------------------------- kernel镜像
│      u-boot-xxxMHZ.bin -------------------------------------- u-boot镜像(其中xxx表示AXI总线频率)
│      rootfs_64k.squashfs ------------------------------------ 64K squashfs 文件系统镜像
│      rootfs_64k.jffs2 --------------------------------------- 64K jffs2 文件系统镜像
│      rootfs_128k.squashfs ----------------------------------- 128K squashfs 文件系统镜像
│      rootfs_128k.jffs2 -------------------------------------- 128K jffs2 文件系统镜像
│      rootfs_256k.squashfs ----------------------------------- 256K squashfs 文件系统镜像
│      rootfs_256k.jffs2 -------------------------------------- 256K jffs2 文件系统镜像
│      rootfs_2k_1bit.yaffs2 ---------------------------------- yaffs2 文件系统镜像
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
    ├─board_glibc -------------------------------------------- hisiv200编译出的单板用工具
    │   ├─hifat ----------------------------------------------- hifat 工具
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
    └─board_uclibc ------------------------------------------- hisiv100nptl编译出的单板用工具
        ├─hifat ----------------------------------------------- hifat 工具
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

3.osdrv目录结构说明：
osdrv
├─Makefile ------------------------------ osdrv目录编译脚本
├─busybox ------------------------------- 存放busybox源代码的目录
├─tools --------------------------------- 存放各种工具的目录
│  ├─board_tools ------------------------ 各种单板上使用工具
│  │  ├─reg-tools-1.0.0 ----------------- 寄存器读写工具
│  │  ├─mtd-utils ----------------------- flash裸读写工具
│  │  ├─udev-100 ------------------------ udev工具集
│  │  ├─gdb ----------------------------- gdb工具
│  │  └─e2fsprogs ----------------------- mkfs工具集
│  └─pc_tools --------------------------- 各种pc上使用工具
│      ├─mkfs.cramfs -------------------- cramfs文件系统制作工具
│      ├─squashfs4.2 -------------------- squashfs文件系统制作工具
│      ├─mkfs.xfs ----------------------- xfs文件系统制作工具
│      ├─mkimage ------------------------ uImage制作工具
│      ├─lzma-4.32.7 -------------------- lzma压缩工具
│      ├─mkfs.jffs2 --------------------- jffs2文件系统制作工具
│      ├─mkyaffs2image301 --------------- yaffs2文件系统制作工具
│      └─uboot_tools -------------------- uboot镜像制作工具、xls文件及ddr初始化脚本、bootrom工具
├─toolchain ----------------------------- 存放工具链的目录
│  ├─arm-hisiv100nptl-linux ------------- hisiv100nptl交叉工具链
│  └─arm-hisiv200-linux ----------------- hisiv200交叉工具链
├─pub ----------------------------------- 存放各种镜像的目录
│  ├─image_glibc ------------------------ 基于hisiv200工具链编译，可供FLASH烧写的映像文件，包括uboot、内核、文件系统
│  ├─image_uclibc ----------------------- 基于hisiv100nptl工具链编译，可供FLASH烧写的映像文件，包括uboot、内核、文件系统
│  ├─bin -------------------------------- 各种未放入根文件系统的工具
│  │  ├─pc ------------------------------ 在pc上执行的工具
│  │  ├─board_glibc --------------------- 基于hisiv200工具链编译，在单板上执行的工具
│  │  └─board_uclibc -------------------- 基于hisiv100nptl工具链编译，在单板上执行的工具
│  ├─rootfs_uclibc.tgz ------------------ 基于hisiv100nptl工具链编译的根文件系统
│  └─rootfs_glibc.tgz ------------------- 基于hisiv200工具链编译的根文件系统
├─rootfs_scripts ------------------------ 存放根文件系统制作脚本的目录
├─uboot --------------------------------- 存放uboot源代码的目录
└─kernel -------------------------------- 存放kernel源代码的目录


4.注意事项
(1)使用某一工具链编译后，如果需要更换工具链，请先将原工具链编译文件清除，然后再更换工具链编译。
(2)在windows下复制源码包时，linux下的可执行文件可能变为非可执行文件，导致无法编译使用；u-boot或内核下编译后，会有很多符号链接文件，在windows下复制这些源码包, 会使源码包变的巨大，因为linux下的符号链接文件变为windows下实实在在的文件，因此源码包膨胀。因此使用时请注意不要在windows下复制源代码包。
