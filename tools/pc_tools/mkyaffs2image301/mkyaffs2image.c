/*
* YAFFS: Yet Another Flash File System. A NAND-flash specific file system.
*
* Copyright (C) 2002-2007 Aleph One Ltd.
*   for Toby Churchill Ltd and Brightstar Engineering
*
* Created by Charles Manning <charles@aleph1.co.uk>
* Nick Bane modifications flagged NCB
* Endian handling patches by James Ng.
* mkyaffs2image hacks by NCB
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 as
* published by the Free Software Foundation.
*/

/*
* makeyaffs2image.c 
*
* Makes a YAFFS2 file system image that can be used to load up a file system.
* Uses default Linux MTD layout - change if you need something different.
*/

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include "yaffs_ecc.h"
#include "yaffs_guts.h"
#include "yaffs_tagsvalidity.h"
#include "yaffs_packedtags2.h"
#include "oob_config.h"

static unsigned int verbose = 0;  
    
unsigned yaffs_traceMask=0;

#define MAX_OBJECTS 20000
static int chunkSize = 2048;
static int spareSize = 64;
static struct oob_info *sel_oob_info = NULL;

const char yaffs2_version[] = "Linux-2.6.35-Google";

/*****************************************************************************/
typedef struct
{
    dev_t dev;
    ino_t ino;
    int   obj;
} objItem;
/*****************************************************************************/
static objItem obj_list[MAX_OBJECTS];

static int n_obj = 0;

static int obj_id = YAFFS_NOBJECT_BUCKETS + 1;

static int nObjects, nDirectories, nPages;

static int outFile;

static int error;
/*****************************************************************************/
static int obj_compare(const void *a, const void * b)
{
    objItem *oa, *ob;

    oa = (objItem *)a;
    ob = (objItem *)b;

    if(oa->dev < ob->dev) return -1;
    if(oa->dev > ob->dev) return 1;
    if(oa->ino < ob->ino) return -1;
    if(oa->ino > ob->ino) return 1;

    return 0;
}
/*****************************************************************************/
static void add_obj_to_list(dev_t dev, ino_t ino, int obj)
{
    if(n_obj < MAX_OBJECTS)
    {
        obj_list[n_obj].dev = dev;
        obj_list[n_obj].ino = ino;
        obj_list[n_obj].obj = obj;
        n_obj++;
        qsort(obj_list,n_obj,sizeof(objItem),obj_compare);
    }
    else
    {
        fprintf(stderr,"Not enough space in object array\n");
        exit(2);
    }
}
/*****************************************************************************/
static int find_obj_in_list(dev_t dev, ino_t ino)
{
    objItem *i = NULL;
    objItem test;

    test.dev = dev;
    test.ino = ino;

    if (n_obj > 0)
    {
        i = bsearch(&test,obj_list,n_obj,sizeof(objItem),obj_compare);
    }
    if (i)
    {
        return i->obj;
    }
    return -1;
}
/*****************************************************************************/
static void hisilicon_nandmtd2_pt2buf(unsigned char * buffer, yaffs_PackedTags2 * pt_tmp)
{
    int i;
    int offset, length;
    unsigned char * ptab = (unsigned char *)pt_tmp; /* packed tags as bytes */
    struct nand_oob_free * oobfree = sel_oob_info->freeoob;;

    memset(buffer, 0xFF, spareSize);

    length = 0;
    for (i = 0; i < sizeof(yaffs_PackedTags2); i++) 
    {
        if (length == 0) 
        {
            offset = oobfree->offset;
            length = oobfree->length;
            if (length == 0) 
            {
                fprintf(stderr, "No OOB space for tags!\n");
                exit(-1);
            }
            oobfree++;
        }
        buffer[offset] = ptab[i];
        offset++;
        length--;
    }
}
/*****************************************************************************/
static int write_chunk(__u8 *data, __u32 objId, __u32 chunkId, __u32 nBytes)
{
    yaffs_ExtendedTags t;
    yaffs_PackedTags2 pt;
    unsigned char oobbuf[512];

    if (sizeof(oobbuf) < sel_oob_info->oobsize)
    {
        fprintf(stderr, "oobbuf too small.\n");
        exit (-1);
    }

    error = write(outFile,data,chunkSize);
    if(error < 0) 
        return error;

    yaffs_InitialiseTags(&t);

    t.chunkId = chunkId;
    t.serialNumber = 1;	// **CHECK**
    t.byteCount = nBytes;
    t.objectId = objId;

    t.sequenceNumber = YAFFS_LOWEST_SEQUENCE_NUMBER;

    t.chunkUsed = 1;

    nPages++;

    yaffs_PackTags2(&pt, &t, 1);

    hisilicon_nandmtd2_pt2buf(oobbuf, &pt);	

    return write(outFile, oobbuf, spareSize);
}
/*****************************************************************************/
static int write_object_header(int objId, yaffs_ObjectType t, struct stat *s, 
                               int parent, const char *name, int equivalentObj, 
                               const char * alias)
{
    __u8 bytes[8192];
    yaffs_ObjectHeader *oh = (yaffs_ObjectHeader *)bytes;

    memset(bytes,0xff,chunkSize);

    oh->type = t;

    oh->parentObjectId = parent;

    strncpy(oh->name,name,YAFFS_MAX_NAME_LENGTH);


    if(t != YAFFS_OBJECT_TYPE_HARDLINK)
    {
        oh->yst_mode = s->st_mode;
        oh->yst_uid = s->st_uid;
        // NCB 12/9/02		oh->yst_gid = s->yst_uid;
        oh->yst_gid = s->st_gid;
        oh->yst_atime = s->st_atime;
        oh->yst_mtime = s->st_mtime;
        oh->yst_ctime = s->st_ctime;
        oh->yst_rdev  = s->st_rdev;
    }

    if(t == YAFFS_OBJECT_TYPE_FILE)
    {
        oh->fileSize = s->st_size;
    }

    if(t == YAFFS_OBJECT_TYPE_HARDLINK)
    {
        oh->equivalentObjectId = equivalentObj;
    }

    if(t == YAFFS_OBJECT_TYPE_SYMLINK)
    {
        strncpy(oh->alias,alias,YAFFS_MAX_ALIAS_LENGTH);
    }

    return write_chunk(bytes,objId,0,0xffff);
}

/*****************************************************************************/
static int process_directory(int parent, const char *path)
{
    DIR *dir;
    struct dirent *entry;

    nDirectories++;

    dir = opendir(path);

    if (dir)
    {
        while ((entry = readdir(dir)) != NULL)
        {

            /* Ignore . and .. */
            if (strcmp(entry->d_name, ".")
                && strcmp(entry->d_name, ".."))
            {
                char full_name[500];
                struct stat stats;
                int equivalentObj;
                int newObj;

                sprintf(full_name, "%s/%s", path,entry->d_name);

                lstat(full_name,&stats);

                if(S_ISLNK(stats.st_mode) 
                    || S_ISREG(stats.st_mode) 
                    || S_ISDIR(stats.st_mode) 
                    || S_ISFIFO(stats.st_mode) 
                    || S_ISBLK(stats.st_mode) 
                    || S_ISCHR(stats.st_mode) 
                    || S_ISSOCK(stats.st_mode))
                {

                    newObj = obj_id++;
                    nObjects++;
			
                    if (verbose)
                        printf("Object %d, %s is a ",newObj,full_name);

                    /* We're going to create an object for it */
                    if((equivalentObj = find_obj_in_list(stats.st_dev, stats.st_ino)) > 0)
                    {
                        /* we need to make a hard link */
                        if (verbose)
                            printf("hard link to object %d\n",equivalentObj);
                        error =  write_object_header(newObj, YAFFS_OBJECT_TYPE_HARDLINK, 
                            &stats, parent, entry->d_name, equivalentObj, NULL);
                    }
                    else 
                    {

                        add_obj_to_list(stats.st_dev,stats.st_ino,newObj);

                        if(S_ISLNK(stats.st_mode))
                        {
                            char symname[500];

                            memset(symname, 0, sizeof(symname));

                            error = readlink(full_name, symname, sizeof(symname)-1);
                            
                            if (verbose)
                                printf("symlink to \"%s\"\n",symname);

                            error =  write_object_header(newObj, YAFFS_OBJECT_TYPE_SYMLINK, 
                                &stats, parent, entry->d_name, -1, symname);

                        }
                        else if(S_ISREG(stats.st_mode))
                        {
                            if (verbose)
                                printf("file, ");
                            error =  write_object_header(newObj, YAFFS_OBJECT_TYPE_FILE, 
                                &stats, parent, entry->d_name, -1, NULL);

                            if(error >= 0)
                            {
                                int h;
                                __u8 bytes[8192];
                                int nBytes;
                                int chunk = 0;

                                h = open(full_name, O_RDONLY);
                                if (h >= 0)
                                {
                                    memset(bytes, 0xff, chunkSize);
                                    while((nBytes = read(h, bytes, chunkSize)) > 0)
                                    {
                                        chunk++;
                                        write_chunk(bytes, newObj, chunk,nBytes);
                                        memset(bytes, 0xff, chunkSize);
                                    }
                                    if(nBytes < 0) 
                                        error = nBytes;

                                    if (verbose)
                                        printf("%d data chunks written\n",chunk);
                                }
                                else
                                {
                                    perror("Error opening file");
                                }
                                close(h);
                            }							
                        }
                        else if(S_ISSOCK(stats.st_mode))
                        {
                            if (verbose)
                                printf("socket\n");
                            error =  write_object_header(newObj, YAFFS_OBJECT_TYPE_SPECIAL, 
                                &stats, parent, entry->d_name, -1, NULL);
                        }
                        else if(S_ISFIFO(stats.st_mode))
                        {
                            if (verbose)
                                printf("fifo\n");
                            error =  write_object_header(newObj, YAFFS_OBJECT_TYPE_SPECIAL,
                                &stats, parent, entry->d_name, -1, NULL);
                        }
                        else if(S_ISCHR(stats.st_mode))
                        {
                            if (verbose)
                                 printf("character device\n");
                            error =  write_object_header(newObj, YAFFS_OBJECT_TYPE_SPECIAL,
                                &stats, parent, entry->d_name, -1, NULL);
                        }
                        else if(S_ISBLK(stats.st_mode))
                        {
                            if (verbose)
                                printf("block device\n");
                            error =  write_object_header(newObj, YAFFS_OBJECT_TYPE_SPECIAL,
                                &stats, parent, entry->d_name, -1, NULL);
                        }
                        else if(S_ISDIR(stats.st_mode))
                        {
                            if (verbose)
                                printf("directory\n");
                            error =  write_object_header(newObj, YAFFS_OBJECT_TYPE_DIRECTORY,
                                &stats, parent, entry->d_name, -1, NULL);
                            // NCB modified 10/9/2001				process_directory(1,full_name);
                            process_directory(newObj,full_name);
                        }
                    }
                }
                else
                {
                    printf(" we don't handle this type\n");
                }
            }
        }
    }

    return 0;
}
/*****************************************************************************/
/* this follow must be consistent with fastboot !!! */
struct yaffs2_tags
{
#define YAFFS2_SIGN_MAGIC    "YFSS!V10"
    unsigned char magic[8];    
    unsigned int nandip;   
    unsigned char yaffs2ver[4];
    unsigned int pagesize; 
    unsigned int ecctype; 
};
/*****************************************************************************/

static int write_tag_file(int pagetype, int ecctype)
{
    char buffer[8192] = {0};
    unsigned short *len;
    char *ptr;
    struct yaffs2_tags *yaffs2_tags = (struct yaffs2_tags *)buffer;
    struct stat status = 
    {
        .st_mode = 0x81FF,
    };
    int newObj = obj_id++;

    memcpy(yaffs2_tags->magic, YAFFS2_SIGN_MAGIC, 8);
    yaffs2_tags->nandip = nandip;
    yaffs2_tags->yaffs2ver[0] = 2;
    yaffs2_tags->yaffs2ver[1] = 6;
    yaffs2_tags->yaffs2ver[2] = 35;
    yaffs2_tags->yaffs2ver[3] = 7;
    yaffs2_tags->pagesize = get_pagesize(pagetype);
    yaffs2_tags->ecctype  = ecctype;

    ptr = buffer + sizeof(struct yaffs2_tags);
   
    // ×Ö·û´®³¤¶È.
    len = (unsigned short *)ptr;
    ptr += sizeof(unsigned short);
    
    time_t now;
    time(&now); 
    // ×Ö·û´®ÄÚÈÝ.
    *len = sprintf(ptr, "mkyaffs2image Version: %s %s, make time: %s.", 
        yaffs2_version, nand_controller_version, ctime(&now));

    ptr += (*len + 1);

    status.st_size = (int)(ptr - buffer);

    write_chunk((unsigned char*)buffer, newObj, 1, status.st_size);

    return write_object_header(newObj, YAFFS_OBJECT_TYPE_FILE, 
        &status, YAFFS_OBJECTID_ROOT, ".flash", -1, NULL);
}
/*****************************************************************************/
static void usage(char *shoftname)
{
    printf("Usage:\n    %s  dir  image  pagesize  ecctype  [-v].\n\n", shoftname);
    printf("    dir          the directory tree to be converted\n");
    printf("    image        the output file to hold the image\n");

    printf("    pagesize     the page size to be converted\n");
    printf("                 0 : 512 Bytes pagesize\n");
    printf("                 1 : 2K Bytes pagesize\n");
    printf("                 2 : 4K Bytes pagesize\n");
    printf("                 3 : 8K Bytes pagesize\n");

    printf("    ecctype      ECC type\n"); 
    printf("                 0 : no ecc\n");
    printf("                 1 : used 1bit ecc\n");
    printf("                 2 : used 4byte ecc\n");
    printf("                 3 : used 8byte ecc\n");
    printf("                 4 : used 24bit/1k ecc\n");
    
    printf("    -v           verbosely list files processed\n");   

    printf("\n");
}
/*****************************************************************************/
struct param_table_t
{
    char *str;
    int   type;
};
/*****************************************************************************/
int main(int argc, char *argv[])
{
    int rel = -1;
    struct stat stats;
    unsigned int ecctype;	
    unsigned int pagetype;
    char *dirpath, *imagefile;
    char *shoftname = strrchr(argv[0], '/') + 1;

    if (shoftname == (char *)1)
        shoftname = argv[0];

    printf("%s: image building tool for YAFFS2, last update %s %s. \nVersion: %s %s\n",
        shoftname, __DATE__, __TIME__, yaffs2_version, nand_controller_version);
    if (argc == 1)
    {
        usage(shoftname);
        goto err;
    }

    if (argc == 5 || argc == 6)
    {
#define _S(_s)   (char *)(_s)

        struct param_table_t page_type_param[] =
        {
            {_S("0"),    pt_pagesize_512},
            {_S("512"),  pt_pagesize_512},

            {_S("1"),    pt_pagesize_2K},
            {_S("2048"), pt_pagesize_2K},
            {_S("2K"),   pt_pagesize_2K},

            {_S("2"),    pt_pagesize_4K},
            {_S("4096"), pt_pagesize_4K},
            {_S("4K"),   pt_pagesize_4K},

            {_S("3"),    pt_pagesize_8K},
            {_S("8192"), pt_pagesize_8K},
            {_S("8K"),   pt_pagesize_8K},

            {0, 0},
        };

        struct param_table_t ecc_type_param[] =
        {
            {_S("0"),        et_ecc_none},
            {_S("none"),     et_ecc_none},

            {_S("1"),        et_ecc_1bit},
            {_S("1bit"),     et_ecc_1bit},
            {_S("1bits"),    et_ecc_1bit},
        
            {_S("2"),        et_ecc_4bytes},
            {_S("4byte"),    et_ecc_4bytes},
            {_S("4bytes"),   et_ecc_4bytes},

            {_S("3"),        et_ecc_8bytes},
            {_S("8byte"),    et_ecc_8bytes},
            {_S("8bytes"),   et_ecc_8bytes},

            {_S("4"),        et_ecc_24bit1k},
            {_S("24bits1k"), et_ecc_24bit1k},
            {_S("24bit1k"),  et_ecc_24bit1k},

            {0,0},
        };

#undef _S

        struct param_table_t *ptp = page_type_param;
        struct param_table_t *etp = ecc_type_param;

        pagetype = pt_pagesize_last;
        for (; ptp->str; ptp++)
        {
            if (!strcasecmp(ptp->str, argv[3]))
            {
                pagetype = ptp->type;
                break;
            }
        }

        ecctype = et_ecc_last;
        for (; etp->str; etp++)
        {
            if (!strcasecmp(etp->str, argv[4]))
            {
                ecctype = etp->type;
                break;
            }
        }
        
        if (argc == 6)
        {
            if (!strcasecmp("-v", argv[5]))
            {
                verbose = 1;
            }
        }
    }
    else
    {
        fprintf(stderr, "bad parameter!\n");
        goto err;
    }

    if ((sel_oob_info = get_oob_info(pagetype, ecctype)) == NULL)
    {
        fprintf(stderr, "not support this parameter.\n");
        goto mkfail;
    }    
    
    dirpath = argv[1];
    if (stat(dirpath, &stats) < 0 || !S_ISDIR(stats.st_mode))
    {
        fprintf(stderr, "can't open dir \"%s\"\n", dirpath);
        goto err;
    }
    imagefile = argv[2];

    chunkSize = get_pagesize(pagetype);
    spareSize = sel_oob_info->oobsize;

    outFile = open(imagefile, O_CREAT | O_TRUNC | O_WRONLY, S_IREAD | S_IWRITE);
    if (outFile < 0)
    {
        fprintf(stderr, "Could not open imagefile file %s\n", imagefile);
        goto mkfail;
    }

    write_tag_file(pagetype, ecctype);

    fprintf(stderr, "Processing dir %s into imagefile %s\n", dirpath, imagefile);
    error =  write_object_header(1, YAFFS_OBJECT_TYPE_DIRECTORY, &stats, 1, "", -1, NULL);
    if (error)
    {
        error = process_directory(YAFFS_OBJECTID_ROOT, dirpath);
    }
    close(outFile);

    if (error < 0)
    {
        perror("operation incomplete");
        rel = 1;
        goto mkfail;
    }
    else
    {
        fprintf(stderr, "Operation success complete. %d objects in %d directories %d NAND pages\n",
            nObjects, nDirectories, nPages);
    }
    rel = 0;

mkfail:
    printf("mkyaffs2image Version: %s %s\n", yaffs2_version, nand_controller_version);
    printf("Input parameter: pagetype: %d(%s), ecctype: %d(%s)\n\n\n",
        pagetype, get_pagesize_str(pagetype),
        ecctype, get_ecctype_str(ecctype));
err:
    return rel;
}	

