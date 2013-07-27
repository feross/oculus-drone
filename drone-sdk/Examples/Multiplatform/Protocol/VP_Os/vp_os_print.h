// ----------------------------------------------
//
//  Author : <sylvain.gaeremynck@parrot.fr>
//  Date   : 03/01/2007
//
//  Parrot Video SDK : Os API
//
// ---------------------------------------------- 

#ifndef _OS_PRINT_H_
#define _OS_PRINT_H_

#ifdef _ECOS
  #include <cyg/kernel/diag.h>
  #define PRINT diag_printf
#else
  #include <stdio.h>
  #define PRINT printf
#endif

#ifdef DEBUG_MODE
  #ifdef _ECOS
    #define DEBUG_PRINT_SDK(...) diag_printf(__VA_ARGS__)
  #else
    #define DEBUG_PRINT_SDK(...) printf(__VA_ARGS__)
  #endif
#else
  #ifdef _ECOS
    #define DEBUG_PRINT_SDK(...)
  #else
    #define DEBUG_PRINT_SDK(...)
  #endif
#endif

#endif // _OS_PRINT_H_
