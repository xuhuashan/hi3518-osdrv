
INST=$(OSDRV_DIR)/tools/board_tools/e2fsprogs/e2fsprogs-1.41.11/inst

all:
	rm $(OSDRV_DIR)/tools/board_tools/e2fsprogs/e2fsprogs-1.41.11 -rf
	tar xzf $(OSDRV_DIR)/tools/board_tools/e2fsprogs/e2fsprogs-1.41.11.tar.gz -C $(OSDRV_DIR)/tools/board_tools/e2fsprogs/
	mkdir $(INST) -p
	pushd $(OSDRV_DIR)/tools/board_tools/e2fsprogs/e2fsprogs-1.41.11;./configure --host=$(OSDRV_CROSS) CFLAGS="$(OSDRV_CROSS_CFLAGS)" --prefix=$(INST) --disable-tls >/dev/null;make -j 20 >/dev/null;make install >/dev/null;popd
	cp $(INST)/sbin/mkfs.ext2 $(OSDRV_DIR)/pub/$(PUB_ROOTFS)/bin
	cp $(INST)/sbin/mkfs.ext3 $(OSDRV_DIR)/pub/$(PUB_ROOTFS)/bin
	cp $(INST)/sbin/mkfs.ext4 $(OSDRV_DIR)/pub/$(PUB_ROOTFS)/bin
	cp $(INST)/sbin/mke2fs $(OSDRV_DIR)/pub/$(PUB_ROOTFS)/bin
clean:
	rm $(OSDRV_DIR)/tools/board_tools/e2fsprogs/e2fsprogs-1.41.11 -rf
 
