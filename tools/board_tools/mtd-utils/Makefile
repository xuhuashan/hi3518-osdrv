################################################################################
#    Create By Cai ZhiYong 2010.11.17
#
#    Your SHOULD NOT make with parameter -j
#
################################################################################

CROSS_COMPILE=$(OSDRV_CROSS)-

ZLIB    = $(PWD)/zlib-1.2.5
LZO     = $(PWD)/lzo-2.03
MTDUTIL = $(PWD)/mtd-utils-20090606

################################################################################

UBI_UTILS  = $(MTDUTIL)/ubi-utils
X86_UBI_UTILS = ubinize
ARM_UBI_UTILS = ubiattach mtdinfo ubicrc32 ubidetach ubiformat\
	ubimkvol ubinfo ubirename ubirmvol ubiupdatevol

MKFS_UBIFS = $(MTDUTIL)/mkfs.ubifs
X86_MKFS_UBIFS = mkfs.ubifs

ARM_MTDUTIL = flashcp flash_erase flash_eraseall flash_info \
    mtd_debug nanddump nandtest sumtool nandwrite

	
################################################################################
LIBLZO   = $(MTDUTIL)/lib/liblzo2.a
LIBZ     = $(MTDUTIL)/lib/libz.a
BUILDDIR = $(PWD)

BIN_X86 = $(BUILDDIR)/bin/x86
BIN_ARM = $(BUILDDIR)/bin/arm

################################################################################
all: arm
	@echo;
	@echo ==============================================================================
	@echo "Execute file at $(BUILDDIR)/bin/"
    
################################################################################
ifeq ($(orign PREFIX), undefined)
install:
	@echo Install directory '"'$(PREFIX)'"' is not exist.
else
install:
	@cp -v $(addprefix  $(BIN_ARM)/, $(ARM_UBI_UTILS) $(ARM_MTDUTIL)) $(PREFIX)/
endif

################################################################################
x86:$(LIBLZO) $(LIBZ)
	test -d "$(BIN_X86)" || mkdir -p "$(BIN_X86)"
	cd $(UBI_UTILS);make clean;make -j 128; cp $(X86_UBI_UTILS) $(BIN_X86)/
	cd $(MKFS_UBIFS);make -j 128;cp $(X86_MKFS_UBIFS) $(BIN_X86)/
	strip -v $(BIN_X86)/*
 
################################################################################
arm:
	test -d "$(BIN_ARM)" || mkdir -p "$(BIN_ARM)"
	cd $(UBI_UTILS); make clean; make -j 128 CROSS=$(CROSS_COMPILE);cp $(ARM_UBI_UTILS) $(BIN_ARM)/
	cd $(MTDUTIL); make -j 128 CROSS=$(CROSS_COMPILE);cp $(ARM_MTDUTIL) $(BIN_ARM)/
	$(CROSS_COMPILE)strip $(BIN_ARM)/*
	
################################################################################
clean:
	rm -rf "$(BUILDDIR)/bin"
	cd $(MTDUTIL); make -j 128 clean
	cd $(UBI_UTILS); make -j 128 clean
	cd $(MKFS_UBIFS); make -j 128 clean
	
################################################################################
distclean: clean
	rm -rf $(MTDUTIL)/lib
	-make -C $(ZLIB) distclean

################################################################################
$(LIBLZO):
	cd $(LZO); CFLAGS="-m32" ./configure --prefix=$(MTDUTIL)
	make -C $(LZO) -j 128
	make -C $(LZO) install

################################################################################
$(LIBZ):
	cd $(ZLIB); CFLAGS="-m32" ./configure --prefix=$(MTDUTIL)
	make -C $(ZLIB) -j 128
	make -C $(ZLIB) install	 

################################################################################
	
.PHONY: distclean clean all arm x86
