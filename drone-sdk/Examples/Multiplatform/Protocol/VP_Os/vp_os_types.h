/**
 *  \brief    VP OS. Types declaration.
 *  \brief    These types are used to provide clean types declaration for portability between OSes
 *  \author   Sylvain Gaeremynck <sylvain.gaeremynck@parrot.fr>
 *  \version  1.0
 *  \date     first release 19/12/2006
 *  \date     modification  26/03/2007
 */

#ifndef _VP_SDK_TYPES_H_
#define _VP_SDK_TYPES_H_

#include <VP_Os/vp_os.h>

#define C_RESULT        int
#define C_OK            0
#define C_FAIL          -1

#define SUCCESS         (0)
#define FAIL            (!SUCCESS)

#define SUCCEED(a)	(((a) & 0xffff) == SUCCESS)
#define FAILED(a)	(((a) & 0xffff) != SUCCESS)
#ifdef USE_MINGW32
#include <windows.h>
#include <windef.h>
#endif

#if defined(USE_MINGW32)

// Definition des types entiers
typedef signed    __int8    int8_t;
typedef unsigned  __int8    uint8_t;
typedef signed    __int16   int16_t;
typedef unsigned  __int16   uint16_t;
typedef signed    __int32   int32_t;
typedef unsigned  __int32   uint32_t;
typedef signed    __int64   int64_t;
typedef unsigned  __int64   uint64_t;

#endif // < _WIN32

typedef float               float32_t;
typedef double              float64_t;

#if !defined(USE_MINGW32)

#include <stdint.h>
#include <stddef.h>

#define CBOOL int

#endif // < __linux__

typedef volatile uint8_t    vuint8;
typedef volatile uint16_t   vuint16;
typedef volatile uint32_t   vuint32;
typedef volatile uint64_t   vuint64;

typedef volatile int8_t     vint8;
typedef volatile int16_t    vint16;
typedef volatile int32_t    vint32;
typedef volatile int64_t    vint64;

#define bool_t  int32_t

#if !defined(USE_MINGW32)

#ifndef TRUE
#define TRUE    1
#endif

#ifndef FALSE
#define FALSE   0
#endif

#endif // < USE_MINGW32

#define BDADDR_SIZE   6

#if !defined(__BLUETOOTH_H)
typedef struct _bdaddr_t
{
  uint8_t b[BDADDR_SIZE];
} bdaddr_t;
#endif // !defined(__BLUETOOTH_H)

#endif // _TYPES_H_
