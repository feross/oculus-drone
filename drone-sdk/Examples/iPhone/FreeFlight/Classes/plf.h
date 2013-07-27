/*
 *  plf.h
 *  ARUpdater
 *
 *  Created by f.dhaeyer on 06/07/10.
 *  Copyright 2010 Parrot SA. All rights reserved.
 *
 */

#ifndef _PLF_H_
#define _PLF_H_

#define PLF_CURRENT_VERSION  10
#define PLF_HEADER_MAGIC     0x21464c50 //!< PLF magic number

typedef unsigned int   Plf_Word;        //!< Unsigned 32 bits integer
typedef unsigned short Plf_Half;        //!< Unsigned 16 bits integer
typedef void*          Plf_Add;         //!< 32 bits address

//! PLF file header
typedef struct {
	Plf_Word    p_magic;                  //!< PLF magic number
	Plf_Word    p_plfversion;             //!< PLF format version
	Plf_Word    p_phdrsize;               //!< File header size
	Plf_Word    p_shdrsize;               //!< Section header size
	Plf_Word    p_type;                   //!< File type
	Plf_Add     p_entry;                  //!< Executable entry point
	Plf_Word    p_targ;                   //!< Target platform
	Plf_Word    p_app;                    //!< Target application
	Plf_Word    p_hdw;                    //!< Hardware compatibility
	Plf_Word    p_ver;                    //!< Version
	Plf_Word    p_edit;                   //!< Edition
	Plf_Word    p_ext;                    //!< Extension
	Plf_Word    p_lang;                   //!< Language zone
	Plf_Word    p_size;                   //!< File size in bytes
} plf_phdr;

int plf_get_header(const char *plf_filename, plf_phdr *header);
#endif // _PLF_H_
