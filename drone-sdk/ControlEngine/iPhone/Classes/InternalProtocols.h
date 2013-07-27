/**
 * @file InternalProtocols.h
 *
 * Copyright 2009 Parrot SA. All rights reserved.
 * @author D HAEYER Frederic
 * @date 2009/10/26
 */
#include "ConstantsAndMacros.h"

@protocol NavdataProtocol

- (void)parrotNavdata:(navdata_unpacked_t*)data;

@end
