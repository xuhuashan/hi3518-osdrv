/******************************************************************************
*    Copyright (c) 2009-2010 by czy.
*    All rights reserved.
* ***
*    written by CaiZhiYong. 2010-10-13 14:01:19
*
******************************************************************************/


#ifndef OOB_CONFIGH
#define OOB_CONFIGH
/******************************************************************************/

/* this follow must be consistent with fastboot !!! */
enum ecc_type
{
    et_ecc_none    = 0x00,
    et_ecc_1bit    = 0x01,
    et_ecc_4bytes  = 0x02,
    et_ecc_8bytes  = 0x03,
    et_ecc_24bit1k = 0x04,
    et_ecc_last    = 0x05
};
/*****************************************************************************/

enum page_type
{
    pt_pagesize_512   = 0x00,
    pt_pagesize_2K    = 0x01,
    pt_pagesize_4K    = 0x02,
    pt_pagesize_8K    = 0x03,
    pt_pagesize_last  = 0x04,
};

/*****************************************************************************/

struct nand_oob_free
{
	unsigned long offset;
	unsigned long length;
};
/*****************************************************************************/

struct oob_info
{
    enum page_type pagetype;
    enum ecc_type ecctype;
    unsigned int oobsize;
    struct nand_oob_free *freeoob;
};

/*****************************************************************************/

struct oob_info * get_oob_info(enum page_type pagetype, 
    enum ecc_type ecctype);

char *get_ecctype_str(enum ecc_type ecctype);

char *get_pagesize_str(enum page_type pagetype);

unsigned int get_pagesize(enum page_type pagetype);

extern const char *nand_controller_version;

extern const unsigned int nandip;

/******************************************************************************/
#endif /* OOB_CONFIGH */
