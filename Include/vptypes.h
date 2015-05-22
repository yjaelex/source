#ifndef __GRTYPES_H__
#define __GRTYPES_H__

#if !defined(_MSC_VER) && !defined(__GNUC__)
#error "Unknown compiler, need to explicitly define data types!!!"
#endif // !MSC_VER && !__GNUC__

#if defined(_WIN32)
#include <stddef.h>
#elif defined(LINUX)
#include <stdint.h>
#include <stdbool.h>
#else
#error "Unsupported OS."
#endif

#include "vpdefines.h"

#if defined(COMP_VC)
/// 64-bit signed integer
typedef signed __int64 int64, *pint64;

/// 64-bit unsigned integer
typedef unsigned __int64 uint64, *puint64;
#endif // COMP_VC


#if defined(COMP_GCC)
/// 64-bit signed integer
typedef signed long long int64, *pint64;;

/// 64-bit unsigned integer
typedef unsigned long long uint64, *puint64;
#endif  // COMP_GCC


/// 8-bit character
typedef unsigned char char8, *pchar8;

/// 8-bit signed integer
typedef signed char int8, *pint8;

/// 16-bit signed integer
typedef signed short int16, *pint16;

/// 32-bit signed integer
typedef signed int int32, *pint32;

/// 8-bit unsigned integer
typedef unsigned char uint8, *puint8;

/// 16-bit unsigned integer
typedef unsigned short uint16, *puint16;

/// 32-bit unsigned integer
typedef unsigned int uint32, *puint32;

/// 32-bit unsigned integer
typedef unsigned long long uint64, *puint64;

/// 32-bit floating point
typedef float float32, *pfloat32;

/// 64-bit floating point
typedef double float64, *pfloat64;

/// pointer to void
typedef void *pvoid;

/// unsigned integer, the same size as a pointer
typedef uintptr_t uintp;

/// signed integer, the same size as a pointer
typedef intptr_t intp;

typedef uint8  uint8_t;
typedef uint16 uint16_t;
typedef uint32 uint32_t;
typedef uint64 uint64_t;
typedef int8  int8_t;
typedef int16 int16_t;
typedef int32 int32_t;
typedef int64 int64_t;
//
/// Standard Boolean Type
//
typedef enum _bool32 {
    ATIGL_FALSE,    ///< False
    ATIGL_TRUE,     ///< True
} bool32;

typedef void* VP_FILEHANDLE;

//
/// float16 type
//
typedef union _float16
{
    uint16 u16;                  ///< 16-bit float (whole)

    //
    ///  Individual bits
    //
    struct 
    {
#if defined(qLittleEndian)
        uint16 mantissa : 10;    ///< mantissa
        uint16 exp      : 5;     ///< exp
        uint16 sign     : 1;     ///< sign
#else
        uint16 sign     : 1;     ///< sign
        uint16 exp      : 5;     ///< exp
        uint16 mantissa : 10;    ///< mantissa
#endif
    } bits;                      ///< individual bits
} float16;

#include <string>
#include <map>
#include <vector>
#include <algorithm>
using namespace std;

/// Name / value parameter pair (first = name, second = value)
typedef map<string, uint32> NameValuePairList;

#ifdef COMP_VC
    #define ALIGNED(X,Y) __declspec(align(Y)) X
#elif defined(COMP_GCC)
    #define ALIGNED(X,Y)  X __attribute__ ((aligned(Y)))
#else
#error Unsupported compiler
#endif

#endif // __GRTYPES_H__



