#ifndef _DATATYPES_H_INCLUDED_
#define _DATATYPES_H_INCLUDED_

typedef char T_ERROR_FLAG;
typedef char T_ERROR_CODE;

#define ERR_NONE 		0
#define ERR_GENERIC		(-1)


typedef unsigned long T_ADDRESS;

typedef unsigned long long QWORD;
typedef unsigned long DWORD;
typedef unsigned short WORD;
typedef unsigned char BYTE;

#include <misra_types.h>

typedef bool  bit;

typedef unsigned char u8_t;
typedef unsigned short u16_t;
typedef unsigned long u32_t;
typedef signed char s8_t;
typedef signed short s16_t;
typedef signed long s32_t;

#endif

