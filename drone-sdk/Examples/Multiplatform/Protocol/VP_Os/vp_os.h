/**
***************************************************************************
*
* Copyright (C) 2007 Parrot S.A.
*
* \date First Release 03/01/2007
* \date Last Modified 15/06/2009
***************************************************************************
*/

#ifndef _OS_H_
#define _OS_H_

/*
 * Platforms support
 */
#ifdef _WIN32
#include <windows.h>
#include <windef.h>
#endif // _WIN32

#undef INLINE

/*
 * Compilers support
 */
#ifdef _MSC_VER // Microsoft visual C++

#undef  FAILED
#define inline  __inline
#define INLINE  __forceinline

#define WEAK

#endif // _MSC_VER

#ifdef __GNUC__ // The Gnu Compiler Collection

#define _GNU_SOURCE

#ifndef USE_MINGW32
#define WINAPI
#else // USE_MINGW32
#undef  FAILED
#endif // USE_MINGW32

#define INLINE __inline__ __attribute__((always_inline))

#define WEAK __attribute__((weak))
#define NO_INSTRUMENT  __attribute__ ((no_instrument_function))

#endif // __GNUC__

#endif // _OS_H_

