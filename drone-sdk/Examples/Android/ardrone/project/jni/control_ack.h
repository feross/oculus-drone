/*
 *  control_ack.h
 *  ARDroneEngine
 *
 *  Created by Mykonos on 25/03/10.
 *  Copyright 2010 Parrot SA. All rights reserved.
 *
 */
#ifndef _PARROT_CONTROL_ACK_H_
#define _PARROT_CONTROL_ACK_H_
#include <ardrone_api.h>

void control_ack_init(void);

bool_t control_ack_configure_navdata_demo(bool_t navdata_demo);

#endif // _PARROT_CONTROL_ACK_H_
