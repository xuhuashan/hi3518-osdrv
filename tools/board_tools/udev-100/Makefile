
all:
	tar xzf udev-100.tgz
	make -C udev-100 CROSS_COMPILE=$(OSDRV_CROSS)- 
	mkdir $(OSDRV_DIR)/pub/$(rootfs_dir)/bin -p
	cp udev-100/udevd $(OSDRV_DIR)/pub/$(rootfs_dir)/bin
	cp udev-100/udevstart $(OSDRV_DIR)/pub/$(rootfs_dir)/bin
	cp udev-100/udevinfo $(OSDRV_DIR)/pub/$(rootfs_dir)/bin
	cp -af rootfs/. $(OSDRV_DIR)/pub/$(rootfs_dir)/
clean:
	rm udev-100 -rf
 
