
##########################################################################################
#	prepare param
##########################################################################################
export OSDRV_DIR=$(shell pwd)
export OSDRV_CROSS
export ARCH=arm
export CROSS_COMPILE=$(OSDRV_CROSS)-
export OSDRV_CROSS_CFLAGS

export NR_CPUS=8

ifeq ($(OSDRV_CROSS), arm-hisiv100-linux)
$(error You chose an obsolete toolchain!)
endif

ifeq ($(OSDRV_CROSS),arm-hisiv100nptl-linux)
LIB_TYPE:=uclibc
BUSYBOX_CFG:=busybox_cfg_hfp_nptl
TOOLCHAIN_DIR:=arm-hisiv100nptl-linux
TOOLCHAIN_RUNTIME_LIB_C:=lib.uClibc.tgz
CROSS_SPECIFIED:=y
endif

ifeq ($(OSDRV_CROSS),arm-hisiv200-linux)
LIB_TYPE:=glibc
BUSYBOX_CFG:=busybox_cfg_hfp_glibc
TOOLCHAIN_DIR:=arm-hisiv200-linux
TOOLCHAIN_RUNTIME_LIB_C:=lib.glibc.tgz
CROSS_SPECIFIED:=y
endif

ifeq ($(CHIP),hi3518a)
	UBOOT_CONFIG:=hi3518a_config
	KERNEL_CFG:=hi3518a_full_defconfig
	CHIP_SPECIFIED:=y
endif

ifeq ($(CHIP),hi3518c)
	UBOOT_CONFIG:=hi3518c_config
	KERNEL_CFG:=hi3518c_full_defconfig
	CHIP_SPECIFIED:=y
endif

ifeq ($(CHIP),hi3516c)
	UBOOT_CONFIG:=hi3516c_config
	KERNEL_CFG:=hi3516c_full_defconfig
	CHIP_SPECIFIED:=y
endif

ifeq ($(CHIP),hi3516c)
	UBOOT:=u-boot-220MHZ.bin
else
	UBOOT:=u-boot-200MHZ.bin
endif

ifneq ($(CROSS_SPECIFIED),y)
all:
	@echo "---------------------------------------------------------------------"
	@echo "ERROR: you should have specified an OSDRV_CROSS!                     "
	@echo "e.g., make OSDRV_CROSS=arm-hisiv100nptl-linux CHIP=hi3518a all 	    "
	@echo "e.g., make OSDRV_CROSS=arm-hisiv200-linux CHIP=hi3518a all           "
	@echo "---------------------------------------------------------------------"
else
ifneq ($(CHIP_SPECIFIED),y)
all:
	@echo "---------------------------------------------------------------------"
	@echo "ERROR: you should have specified the right CHIP(hi3518a or hi3518c)! "
	@echo "e.g., make OSDRV_CROSS=arm-hisiv100nptl-linux CHIP=hi3518a all 	    "
	@echo "e.g., make OSDRV_CROSS=arm-hisiv200-linux CHIP=hi3518a all           "
	@echo "---------------------------------------------------------------------"
endif
endif

UIMAGE:=uImage
UBOOT_REG_BIN:=reg_info_$(CHIP).bin

export PUB_BOARD:=board_$(LIB_TYPE)
export PUB_IMAGE:=image_$(LIB_TYPE)
export PUB_ROOTFS:=rootfs_$(LIB_TYPE)
TOOLCHAIN_RUNTIME_LIB:=armv5te_soft
OSDRV_CROSS_CFLAGS:= -march=armv5te -mcpu=arm926ej-s

##########################################################################################
#	set task
##########################################################################################
ifeq ($(CROSS_SPECIFIED),y)
ifeq ($(CHIP_SPECIFIED),y)
all: prepare hiboot hikernel hirootfs_prepare hibusybox hipctools hiboardtools hirootfs_build $(UIMAGE)-initramfs

notools: prepare hiboot hikernel hirootfs_prepare hibusybox hirootfs_build

clean: hiboot_clean hikernel_clean \
       hibusybox_clean hipctools_clean hiboardtools_clean hirootfs_clean

distclean:clean pub_clean
endif
endif

##########################################################################################
#task [0]	prepare
##########################################################################################
prepare:
	mkdir $(OSDRV_DIR)/pub/$(PUB_IMAGE) -p
	mkdir $(OSDRV_DIR)/pub/bin/$(PUB_BOARD) -p
	mkdir $(OSDRV_DIR)/pub/bin/pc -p
	mkdir $(OSDRV_DIR)/pub/bin/$(PUB_BOARD)/hifat -p

##########################################################################################
#task [1]	build uboot
##########################################################################################
hiboot: prepare
	@echo "---------task [1]	build boot"
	#rm $(OSDRV_DIR)/uboot/u-boot-2010.06 -rf
	pushd $(OSDRV_DIR)/uboot; \
	  for p in $$(ls u-boot-*.patch 2>/dev/null); do \
	    patch -p0 -i $${p}; \
	  done; \
	popd; \
	make -C $(OSDRV_DIR)/uboot/u-boot-2010.06 \
	      ARCH=arm CROSS_COMPILE=$(OSDRV_CROSS)- $(UBOOT_CONFIG); \
	pushd $(OSDRV_DIR)/uboot/u-boot-2010.06; \
	  make ARCH=arm CROSS_COMPILE=$(OSDRV_CROSS)- \
	        -j $(NR_CPUS) >/dev/null ; \
	popd
	cp $(OSDRV_DIR)/uboot/u-boot-2010.06/u-boot.bin \
	   $(OSDRV_DIR)/pub/$(PUB_IMAGE)
	cp $(OSDRV_DIR)/tools/pc_tools/uboot_tools/mkboot.sh \
	   $(OSDRV_DIR)/pub/$(PUB_IMAGE)
	cp $(OSDRV_DIR)/tools/pc_tools/uboot_tools/$(UBOOT_REG_BIN) \
	   $(OSDRV_DIR)/pub/$(PUB_IMAGE)
	chmod 777 $(OSDRV_DIR)/pub/$(PUB_IMAGE)/mkboot.sh
	pushd $(OSDRV_DIR)/pub/$(PUB_IMAGE); \
	  ./mkboot.sh $(UBOOT_REG_BIN) $(UBOOT); \
	popd
	rm $(OSDRV_DIR)/pub/$(PUB_IMAGE)/u-boot.bin
	rm $(OSDRV_DIR)/pub/$(PUB_IMAGE)/mkboot.sh
	rm $(OSDRV_DIR)/pub/$(PUB_IMAGE)/$(UBOOT_REG_BIN)

hiboot_clean:
	make clean -C $(OSDRV_DIR)/uboot/u-boot-2010.06

##########################################################################################
#task [2]	build kernel
##########################################################################################
hikernel: prepare
	@echo "---------task [2] build kernel"
	pushd $(OSDRV_DIR)/kernel; \
	  for p in $$(ls linux-*.patch 2>/dev/null); do \
	    patch -p0 -i $${p}; \
	  done; \
	popd; \
	if [ -f $(OSDRV_DIR)/kernel/defconfig ]; then \
	  KCONFIG_ALLCONFIG=$(OSDRV_DIR)/kernel/defconfig \
	  make -C $(OSDRV_DIR)/kernel/linux-3.0.y \
	        ARCH=arm CROSS_COMPILE=$(OSDRV_CROSS)- allnoconfig; \
	else \
	  make -C $(OSDRV_DIR)/kernel/linux-3.0.y \
	        ARCH=arm CROSS_COMPILE=$(OSDRV_CROSS)- $(KERNEL_CFG); \
	fi
	pushd $(OSDRV_DIR)/kernel/linux-3.0.y; \
	  make ARCH=arm CROSS_COMPILE=$(OSDRV_CROSS)- \
	        -j $(NR_CPUS) 2>&1 >/dev/null; \
	popd
	pushd $(OSDRV_DIR)/kernel/linux-3.0.y; \
	  make ARCH=arm CROSS_COMPILE=$(OSDRV_CROSS)- uImage \
	          -j $(NR_CPUS) 2>&1 >/dev/null; \
	popd
	cp $(OSDRV_DIR)/kernel/linux-3.0.y/arch/arm/boot/uImage \
	   $(OSDRV_DIR)/pub/$(PUB_IMAGE)/$(UIMAGE)

hikernel_clean:
	make clean -C $(OSDRV_DIR)/kernel/linux-3.0.y

##########################################################################################
#task [3]	prepare rootfs 
##########################################################################################
hirootfs_prepare: prepare
	@echo "---------task [3] prepare rootfs "
	rm $(OSDRV_DIR)/pub/$(PUB_ROOTFS)* -rf
	## rootfs skeleton
	#tar xzf $(OSDRV_DIR)/rootfs_scripts/rootfs.tgz -C $(OSDRV_DIR)/pub
	#mv $(OSDRV_DIR)/pub/rootfs $(OSDRV_DIR)/pub/$(PUB_ROOTFS)
	cp -af $(OSDRV_DIR)/rootfs_scripts/rootfs $(OSDRV_DIR)/pub/${PUB_ROOTFS}
	## C runtime library
	tar xzf $(OSDRV_DIR)/toolchain/$(TOOLCHAIN_DIR)/runtime_lib/$(TOOLCHAIN_RUNTIME_LIB)/$(TOOLCHAIN_RUNTIME_LIB_C) -C $(OSDRV_DIR)/pub/$(PUB_ROOTFS)
	## C++ runtime library
	tar xzf $(OSDRV_DIR)/toolchain/$(TOOLCHAIN_DIR)/runtime_lib/$(TOOLCHAIN_RUNTIME_LIB)/lib.stdc++.tgz -C $(OSDRV_DIR)/pub/$(PUB_ROOTFS)

##########################################################################################
#task [4]	build busybox 
##########################################################################################
hibusybox: prepare
	@echo "---------task [4] build busybox "
	#rm $(OSDRV_DIR)/busybox/busybox-1.16.1 -rf
	if [ ! -d $(OSDRV_DIR)/busybox/busybox-1.16.1 ]; then \
	  tar xzf $(OSDRV_DIR)/busybox/busybox-1.16.1.tgz \
	      -C $(OSDRV_DIR)/busybox; \
	  pushd $(OSDRV_DIR)/busybox; \
	    for p in $$(ls busybox-*.patch 2>/dev/null); do \
	      patch -p0 -i $${p}; \
	    done; \
	  popd; \
	fi
	KCONFIG_ALLCONFIG=$(OSDRV_DIR)/busybox/busybox-1.16.1/$(BUSYBOX_CFG) \
        make -C $(OSDRV_DIR)/busybox/busybox-1.16.1 \
              ARCH=arm CROSS_COMPILE=$(OSDRV_CROSS)- allnoconfig; \
	#find $(OSDRV_DIR)/busybox/busybox-1.16.1 | xargs touch
	pushd $(OSDRV_DIR)/busybox/busybox-1.16.1/; \
	  make -j $(NR_CPUS) >/dev/null 2>&1; \
	popd
	make -C $(OSDRV_DIR)/busybox/busybox-1.16.1 install
	cp -af $(OSDRV_DIR)/busybox/busybox-1.16.1/_install/* \
	       $(OSDRV_DIR)/pub/$(PUB_ROOTFS)

hibusybox_clean:
	make clean -C $(OSDRV_DIR)/busybox/busybox-1.16.1

##########################################################################################
#task [5]	build pc tools
##########################################################################################
hipctools: prepare
	@echo "---------task [5] build tools which run on pc"
	make -C $(OSDRV_DIR)/tools/pc_tools/mkyaffs2image301
	cp $(OSDRV_DIR)/tools/pc_tools/mkyaffs2image301/mkyaffs2image \
	   $(OSDRV_DIR)/pub/bin/pc
	cp $(OSDRV_DIR)/tools/pc_tools/mkfs.jffs2 \
	   $(OSDRV_DIR)/pub/bin/pc
	cp $(OSDRV_DIR)/tools/pc_tools/mkfs.cramfs \
	   $(OSDRV_DIR)/pub/bin/pc
	cp $(OSDRV_DIR)/tools/pc_tools/mkimage \
	   $(OSDRV_DIR)/pub/bin/pc
	pushd $(OSDRV_DIR)/tools/pc_tools/squashfs4.2; make; popd
	cp $(OSDRV_DIR)/tools/pc_tools/squashfs4.2/mksquashfs \
	   $(OSDRV_DIR)/pub/bin/pc
	pushd $(OSDRV_DIR)/tools/pc_tools/lzma-4.32.7; \
	  chmod 777 ./configure; \
	popd
	pushd $(OSDRV_DIR)/tools/pc_tools/lzma-4.32.7; \
	  ./configure; \
	popd
	pushd $(OSDRV_DIR)/tools/pc_tools/lzma-4.32.7; \
	  make; \
	popd
	cp $(OSDRV_DIR)/tools/pc_tools/lzma-4.32.7/src/lzma/lzma \
	   $(OSDRV_DIR)/pub/bin/pc

hipctools_clean:
	make -C $(OSDRV_DIR)/tools/pc_tools/mkyaffs2image301 clean
	pushd $(OSDRV_DIR)/tools/pc_tools/lzma-4.32.7; \
	  chmod 777 ./configure; \
	  ./configure >/dev/null; \
	popd
	make -C $(OSDRV_DIR)/tools/pc_tools/lzma-4.32.7 distclean >/dev/null
	pushd $(OSDRV_DIR)/tools/pc_tools/squashfs4.2;make distclean;popd


##########################################################################################
#task [6]	build board tools
##########################################################################################
hiboardtools: hirootfs_prepare hiboardtools_clean
	@echo "---------task [6] build tools which run on board "
	make -C $(OSDRV_DIR)/tools/board_tools/e2fsprogs
	make -C $(OSDRV_DIR)/tools/board_tools/reg-tools-1.0.0
	cp -af $(OSDRV_DIR)/tools/board_tools/reg-tools-1.0.0/btools \
	       $(OSDRV_DIR)/pub/$(PUB_ROOTFS)/bin
	cp -af $(OSDRV_DIR)/tools/board_tools/reg-tools-1.0.0/hi* \
	       $(OSDRV_DIR)/pub/$(PUB_ROOTFS)/bin
	chmod 777 $(OSDRV_DIR)/tools/board_tools/hifat/$(LIB_TYPE)/static/himount
	cp -af $(OSDRV_DIR)/tools/board_tools/hifat/$(LIB_TYPE)/static/himount \
	       $(OSDRV_DIR)/pub/$(PUB_ROOTFS)/bin
	cp -af $(OSDRV_DIR)/tools/board_tools/hifat/$(LIB_TYPE)/static/libhimount_api.a \
	       $(OSDRV_DIR)/pub/$(PUB_ROOTFS)/lib
	make -C $(OSDRV_DIR)/tools/board_tools/udev-100 \
	       rootfs_dir=$(PUB_ROOTFS)
	cp $(OSDRV_DIR)/tools/board_tools/gdb/gdb-$(OSDRV_CROSS) \
	       $(OSDRV_DIR)/pub/bin/$(PUB_BOARD)
	find $(OSDRV_DIR)/tools/board_tools/mtd-utils/ -name 'configure' \
	       -exec chmod +x "{}" \;
	pushd $(OSDRV_DIR)/tools/board_tools/mtd-utils; \
	  make >/dev/null; \
	popd
	cp $(OSDRV_DIR)/tools/board_tools/mtd-utils/bin/arm/* \
	   $(OSDRV_DIR)/pub/bin/$(PUB_BOARD)
	rm $(OSDRV_DIR)/pub/bin/$(PUB_BOARD)/ubi*
	cp $(OSDRV_DIR)/tools/board_tools/hifat/$(LIB_TYPE)/* \
	   $(OSDRV_DIR)/pub/bin/$(PUB_BOARD)/hifat/ -af
	pushd $(OSDRV_DIR)/tools/board_tools/iupdate; \
	  mkdir -p build-hi3518; \
	  pushd build-hi3518; \
	    ../configure --build=$${MACHTYPE} --host=$(OSDRV_CROSS) \
	                 --prefix=/usr || exit 1; \
	    make -j$(NR_CPUS) || exit 1; \
	    make install DESTDIR=$(OSDRV_DIR)/pub/$(PUB_ROOTFS) || exit 1; \
	  popd; \
	popd

hiboardtools_clean:
	make -C $(OSDRV_DIR)/tools/board_tools/e2fsprogs clean
	make -C $(OSDRV_DIR)/tools/board_tools/reg-tools-1.0.0 clean
	make -C $(OSDRV_DIR)/tools/board_tools/udev-100 clean
	pushd $(OSDRV_DIR)/tools/board_tools/mtd-utils;make distclean;popd
	rm -rf $(OSDRV_DIR)/tools/board_tools/iupdate/build-hi3518; \


##########################################################################################
#task [7]	build rootfs
##########################################################################################
hirootfs_build: hibusybox hiboardtools hirootfs_prepare hipctools
	@echo "---------task [8] build rootfs"
	chmod 777 $(OSDRV_DIR)/pub/bin/$(PUB_BOARD)/*
	chmod 777 $(OSDRV_DIR)/pub/bin/pc/*
	rm $(OSDRV_DIR)/pub/$(PUB_ROOTFS)/dev/* -rf
	pushd $(OSDRV_DIR)/pub/$(PUB_ROOTFS); \
	  ln -s sbin/init init; \
	popd

# [!]	If you're building rootfs of minimized-configuration version, 
#       it is highly recommended to uncomment the following line.
#	pushd $(OSDRV_DIR)/pub/$(PUB_ROOTFS)/lib/;$(OSDRV_CROSS)-strip *;popd
	find $(OSDRV_DIR)/pub/$(PUB_ROOTFS)/ -type f -a -executable \
	    -exec $(OSDRV_CROSS)-strip -s "{}" \;
	pushd $(OSDRV_DIR)/pub/bin/pc; \
	  ./mkfs.jffs2 -d $(OSDRV_DIR)/pub/$(PUB_ROOTFS) -l -e 0x40000 \
	               -o $(OSDRV_DIR)/pub/$(PUB_IMAGE)/rootfs_256k.jffs2; \
	  ./mkfs.jffs2 -d $(OSDRV_DIR)/pub/$(PUB_ROOTFS) -l -e 0x20000 \
	               -o $(OSDRV_DIR)/pub/$(PUB_IMAGE)/rootfs_128k.jffs2; \
	  ./mkfs.jffs2 -d $(OSDRV_DIR)/pub/$(PUB_ROOTFS) -l -e 0x10000 \
	               -o $(OSDRV_DIR)/pub/$(PUB_IMAGE)/rootfs_64k.jffs2; \
	  ./mksquashfs $(OSDRV_DIR)/pub/$(PUB_ROOTFS) \
	               $(OSDRV_DIR)/pub/$(PUB_IMAGE)/rootfs_256k.squashfs \
		       -b 256K -comp xz; \
	  ./mksquashfs $(OSDRV_DIR)/pub/$(PUB_ROOTFS) \
	               $(OSDRV_DIR)/pub/$(PUB_IMAGE)/rootfs_128k.squashfs \
		       -b 128K -comp xz; \
	  ./mksquashfs $(OSDRV_DIR)/pub/$(PUB_ROOTFS) \
	               $(OSDRV_DIR)/pub/$(PUB_IMAGE)/rootfs_64k.squashfs \
		       -b 64K -comp xz; \
	popd
	@if [ "$(CHIP)" = "hi3518a" -o "$(CHIP)" = "hi3516c" ];then \
	  pushd $(OSDRV_DIR)/pub/bin/pc; \
	    ./mkyaffs2image $(OSDRV_DIR)/pub/$(PUB_ROOTFS)/ \
	          $(OSDRV_DIR)/pub/$(PUB_IMAGE)/rootfs_2k_1bit.yaffs2 1 1; \
	  popd; \
	  chmod 644 $(OSDRV_DIR)/pub/$(PUB_IMAGE)/rootfs_2k_1bit.yaffs2; \
	fi
	find $(OSDRV_DIR)/pub/$(PUB_ROOTFS)/ -name '*svn' | xargs rm -rf
	pushd $(OSDRV_DIR)/pub;tar czf $(PUB_ROOTFS).tgz $(PUB_ROOTFS);popd

hirootfs_clean:
	rm $(OSDRV_DIR)/pub/$(PUB_ROOTFS)/ -rf


##########################################################################################
#task [8]	build final kernel with initramfs
##########################################################################################
$(UIMAGE)-initramfs: hikernel hirootfs_build
	@echo "---------task [8] build $(UIMAGE)-initramfs"
	pushd ${OSDRV_DIR}/kernel/linux-3.0.y; \
	if grep "^CONFIG_BLK_DEV_INITRD=y" .config; then \
	  sed -i.orig -e "s;^\(CONFIG_INITRAMFS_SOURCE\)=\".*\";\1=\"$(OSDRV_DIR)/pub/${PUB_ROOTFS} $(OSDRV_DIR)/rootfs_scripts/device_table.txt\";" .config; \
	  if grep "^CONFIG_RD_XZ=y" .config; then comp=XZ; \
	  elif grep "^CONFIG_RD_LZMA=y" .config; then comp=LZMA; \
	  elif grep "^CONFIG_RD_BZIP2=y" .config; then comp=BZIP2; \
	  elif grep "^CONFIG_RD_LZO=y" .config; then comp=LZO; \
	  elif grep "^CONFIG_RD_GZIP=y" .config; then comp=GZIP; \
	  else comp= NONE; fi; \
	  sed -i -e "s;^CONFIG_INITRAMFS_COMPRESSION_;# @;" .config; \
	  sed "/^CONFIG_INITRAMFS_SOURCE=.*/a\\CONFIG_INITRAMFS_COMPRESSION_$${comp}=y" .config; \
	  yes "" | make ARCH=arm CROSS_COMPILE=$(OSDRV_CROSS)- oldconfig; \
	  make ARCH=arm CROSS_COMPILE=$(OSDRV_CROSS)- uImage \
	        -j $(NR_CPUS) 2>&1 >/dev/null; \
	  cp arch/arm/boot/uImage $(OSDRV_DIR)/pub/$(PUB_IMAGE)/$(UIMAGE)-initramfs; \
	  mv -f .config.orig .config; \
	fi; \
	popd
	@echo "---------finish osdrv work"


##########################################################################################
#task [13]	clean pub
##########################################################################################
pub_clean:
	rm $(OSDRV_DIR)/pub/$(PUB_IMAGE)/ -rf
	rm $(OSDRV_DIR)/pub/bin/ -rf
	rm $(OSDRV_DIR)/pub/$(PUB_ROOTFS).tgz -rf
	make distclean -C $(OSDRV_DIR)/uboot/u-boot-2010.06
	make distclean -C $(OSDRV_DIR)/kernel/linux-3.0.y
	make distclean -C $(OSDRV_DIR)/busybox/busybox-1.16.1

