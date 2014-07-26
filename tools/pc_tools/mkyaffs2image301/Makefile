#Makefile for mkyaffs
#
# NB this is not yet suitable for putting into the kernel tree.
# YAFFS: Yet another Flash File System. A NAND-flash specific file system. 
#
# Copyright (C) 2002 Aleph One Ltd.
#   for Toby Churchill Ltd and Brightstar Engineering
#
# Created by Charles Manning <charles@aleph1.co.uk>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation.

## Change or override  KERNELDIR to your kernel

CFLAGS =   -I/usr/include -O2 -Wall -DCONFIG_YAFFS_UTIL -static
CFLAGS+=   -Wshadow -Wpointer-arith -Wwrite-strings -Wstrict-prototypes -Wmissing-declarations
CFLAGS+=   -Wmissing-prototypes -Wredundant-decls -Wnested-externs -Winline

CC=$(MAKETOOLS)gcc

MKYAFFS2SOURCES = mkyaffs2image.c yaffs_ecc.c yaffs_packedtags2.c yaffs_tagsvalidity.c oob_config_v301.c
MKYAFFS2IMAGEOBJS = $(MKYAFFS2SOURCES:.c=.o)

all: mkyaffs2image

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

mkyaffs2image: $(MKYAFFS2IMAGEOBJS)
	$(CC) -o $@ $(MKYAFFS2IMAGEOBJS) -static

clean:
	rm -f $(MKYAFFS2IMAGEOBJS) mkyaffs2image
