/*
 *  plf.c
 *  ARUpdater
 *
 *  Created by f.dhaeyer on 06/07/10.
 *  Copyright 2010 Parrot SA. All rights reserved.
 *
 */
#include <stdio.h>
#include <string.h>
#include "plf.h"

/** Get version numbers of plf
 * @return -1 if error of file, else 0 
 */
int
plf_get_header(const char *plf_filename, plf_phdr *header) 
{
	plf_phdr h;
	int result = -1;
	FILE *f = fopen(plf_filename, "rb");
	if(f != NULL)
	{
		printf("File %s opened\n", plf_filename);
		if(fread(&h, 1, sizeof(plf_phdr), f) == sizeof(plf_phdr))
		{
			memcpy(header, &h, sizeof(plf_phdr));
			result = 0;
		}
		fclose(f);
	}
	
	return result;
}
