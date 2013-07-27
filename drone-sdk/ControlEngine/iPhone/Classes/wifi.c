/*
 *  wifi.c
 *  ARDroneEngine
 *
 *  Created by f.dhaeyer on 30/03/11.
 *  Copyright 2011 Parrot SA. All rights reserved.
 *
 */
#include "wifi.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/sockio.h>
#include <net/if.h>
#include <errno.h>
#include <net/if_dl.h>
#include <net/ethernet.h>

#define max(a,b)	((a > b) ? a : b)

#define BUFFERSIZE	4000

char iphone_mac_address[] = "00:00:00:00:00:00";

void get_iphone_mac_address(const char *itfName)
{
	if(itfName)
	{
		struct ifconf ifc;
		struct ifreq *ifr;
		int sockfd;
		char buffer[BUFFERSIZE], *cp, *cplim;
		char temp[80];
		
		sockfd = socket(AF_INET, SOCK_DGRAM, 0);
		if (sockfd >= 0)
		{
			ifc.ifc_len = BUFFERSIZE;
			ifc.ifc_buf = buffer;
			
			if (ioctl(sockfd, SIOCGIFCONF, (char *)&ifc) >= 0)
			{		
				ifr = ifc.ifc_req;
				
				cplim = buffer + ifc.ifc_len;
				for (cp = buffer ; cp < cplim ; )
				{
					ifr = (struct ifreq *)cp;
					if (ifr->ifr_addr.sa_family == AF_LINK)
					{
						struct sockaddr_dl *sdl = (struct sockaddr_dl *)&ifr->ifr_addr;
						int a,b,c,d,e,f;
						
						strcpy(temp, (char *)ether_ntoa((const struct ether_addr *)LLADDR(sdl)));
						sscanf(temp, "%x:%x:%x:%x:%x:%x", &a, &b, &c, &d, &e, &f);
						sprintf(temp, "%02X:%02X:%02X:%02X:%02X:%02X",a,b,c,d,e,f);
						if(strcmp(ifr->ifr_name, itfName) == 0)
						{
							strcpy(iphone_mac_address, temp);
							break;
						}
					}
					cp += sizeof(ifr->ifr_name) + max(sizeof(ifr->ifr_addr), ifr->ifr_addr.sa_len);
				}
			}
			
			close(sockfd);
		}
	}
}
