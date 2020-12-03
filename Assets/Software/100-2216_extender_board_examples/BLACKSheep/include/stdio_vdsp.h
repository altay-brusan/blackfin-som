#pragma once
#ifndef __NO_BUILTIN
#pragma system_header /* stdio.h */
#endif
/************************************************************************
 *
 * stdio.h
 *
 * (c) Copyright 2003-2006 Analog Devices, Inc.  All rights reserved.
 *
 * Copyright (c) 1992-2001 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
 *
 ************************************************************************/
/* stdio.h standard header */


#ifndef _STDIO
#define _STDIO
#ifndef _YVALS
 #include <yvals.h>
#endif

#ifdef _MISRA_RULES
#pragma diag(push)
#pragma diag(suppress:misra_rule_5_1:"Disable rule 5.1 which bars use of long identifiers (>31 chars). This header has many such long macros but they have been available for a long time so cannot be modified to be compliant.")
#pragma diag(suppress:misra_rule_5_2:"ADI header will re-use identfiers")
#pragma diag(suppress:misra_rule_5_3:"ADI header will re-use identfiers")
#pragma diag(suppress:misra_rule_5_4:"ADI header will re-use identfiers")
#pragma diag(suppress:misra_rule_5_5:"ADI header will re-use identfiers")
#pragma diag(suppress:misra_rule_5_6:"ADI header will re-use identfiers")
#pragma diag(suppress:misra_rule_5_7:"ADI header will re-use identfiers")
#pragma diag(suppress:misra_rule_6_3:"ADI header allows use of basic types")
#pragma diag(suppress:misra_rule_16_1:"ADI header allows variable number of arguments")
#pragma diag(suppress:misra_rule_19_1:"ADI header requires later include")
#pragma diag(suppress:misra_rule_19_4:"ADI header allows any substitution")
#pragma diag(suppress:misra_rule_19_7:"ADI header allows function macros")
#pragma diag(suppress:misra_rule_20_1:"Library file that defines library names")
#pragma diag(suppress:misra_rule_20_2:"Library file that defines library names")
#endif /* _MISRA_RULES */

_C_STD_BEGIN

		/* macros */
#ifndef NULL
 #define NULL	 	_NULL
#endif

#define _IOFBF		0
#define _IOLBF		1
#define _IONBF		2

#define BUFSIZ		512
#define EOF			(-1)
#define FILENAME_MAX	_FNAMAX
#define FOPEN_MAX		_FOPMAX
#define L_tmpnam		_TNAMAX
#define TMP_MAX			32

#define SEEK_SET	0
#define SEEK_CUR	1
#define SEEK_END	2

#define stdin		(&_CSTD _Stdin)
#define stdout		(&_CSTD _Stdout)
#define stderr		(&_CSTD _Stderr)

 #ifdef _MULTI_THREAD
  #define _Lockfile(str)	_Lockfilelock(str)
  #define _Unlockfile(str)	_Unlockfilelock(str)

 #else /* _MULTI_THREAD */
  #define _Lockfile(x)		(void)0
  #define _Unlockfile(x)	(void)0
 #endif /* _MULTI_THREAD */

		/* type definitions */
 #if !defined(_SIZE_T) && !defined(_SIZET)
  #define _SIZE_T
  #define _SIZET
typedef _Sizet size_t;
 #endif

typedef _Fpost fpos_t;

#ifndef _FD_TYPE
 #define _FD_TYPE	signed char
#endif

typedef struct _Filet
	{	/* file control information */
    unsigned short  _Mode;
#if defined(_ADI_LIBIO)
    _FD_TYPE  fileID;
#else
     _FD_TYPE _Handle;
#endif
#if defined(__ADSPBLACKFIN__) /* common for libio and dinkumware. */
    unsigned char *_Next;
    unsigned char *_Rend;
    unsigned char *_Wend;
    unsigned char *_Buf;
    unsigned char *_Bend;
#endif
#if defined(_ADI_LIBIO)
#if defined(__ADSP21XX__) || defined(__ADSP21000__) || \
    (defined(__ADSPTS__) && !defined(__TS_BYTE_ADDRESS))
    unsigned long   _Next, _Rend, _Wend;
    unsigned char*  bufadr;
    unsigned long   _Buf;
    unsigned char*  _Bend;
    unsigned long   rsave;
    unsigned char   _Back[2];
    unsigned char   onechar;
    char            nback;
#else /* Blackfin - Fast IO */
    unsigned char *bufadr;
    unsigned char *rsave;
    unsigned char  onechar;
    char           nback;
    unsigned char  _Back[2];
    unsigned char  pBlock[36];
#endif
#else /* Blackfin Conventional/Full I/O */
   unsigned char _Lockno;
   unsigned char *_Rback;
   _Wchart *_WRback;
   _Wchart _WBack[2];
   unsigned char *_Rsave;
   unsigned char *_WRend;
   unsigned char *_WWend;
   struct _Mbstatet _Wstate;
   char *_Tmpnam;
   unsigned char _Back[2];
   unsigned char _Cbuf;
#endif
	} FILE;

#define _FILET
		/* declarations */

_C_LIB_DECL
  extern FILE _Stdin, _Stdout, _Stderr;

#pragma misra_func(io)
void clearerr(FILE *_str);
#pragma misra_func(io)
int fclose(FILE *_str);
#pragma misra_func(io)
int feof(FILE *_str);
#pragma misra_func(io)
int ferror(FILE *_str);
#pragma misra_func(io)
int fflush(FILE *_str);
#pragma misra_func(io)
int fgetc(FILE *_str);
#pragma misra_func(io)
int fgetpos(FILE *_str, fpos_t *_p);
#pragma misra_func(io)
char * fgets(char *_buf, int _n, FILE *_str);
#pragma misra_func(io)
FILE * fopen(const char *_filename, const char *_mods);

#if !defined(__DOUBLES_ARE_FLOATS__)
#pragma linkage_name __fprintf64
#endif
#pragma misra_func(io)
int fprintf(FILE *_str, const char *_fmt, ...);

#pragma misra_func(io)
int fputc(int _ci, FILE *_str);
#pragma misra_func(io)
int fputs(const char *_s, FILE *_str);
#pragma misra_func(io)
size_t fread(void *_ptr, size_t _size, size_t _nelem, FILE *_str);
#pragma misra_func(io)
FILE * freopen(const char *_filename, const char *_mods,
	FILE *_str);

#if !defined(__DOUBLES_ARE_FLOATS__)
#pragma linkage_name __fscanf64
#endif
#pragma misra_func(io)
int fscanf(FILE *_str, const char *_fmt, ...);
#pragma misra_func(io)
int fseek(FILE *_str, long _off, int _smode);
#pragma misra_func(io)
int fsetpos(FILE *_str, const fpos_t *_p);
#pragma misra_func(io)
long ftell(FILE *_str);
#pragma misra_func(io)
size_t fwrite(const void *_ptr, size_t _size, size_t _nelem, FILE *_str);
#pragma misra_func(io)
char * gets(char *_buf);
#pragma misra_func(io)
void perror(const char *_s);

#if !defined(__DOUBLES_ARE_FLOATS__)
#pragma linkage_name __printf64
#endif
#pragma misra_func(io)
int printf(const char *_fmt, ...);
#pragma misra_func(io)
int puts(const char *_s);
#pragma misra_func(io)
int remove(const char *_filename);
#pragma misra_func(io)
int rename(const char *_oldnm, const char *_newnm);
#pragma misra_func(io)
void rewind(FILE *_str);

#if !defined(__DOUBLES_ARE_FLOATS__)
#pragma linkage_name __scanf64
#endif
#pragma misra_func(io)
int scanf(const char *_fmt, ...);
#pragma misra_func(io)
void setbuf(FILE *_str, char *_buf);
#pragma misra_func(io)
int setvbuf(FILE *_str, char *_abuf, int _smode, size_t _size);

#if !defined(__DOUBLES_ARE_FLOATS__)
#pragma linkage_name __sprintf64
#endif
#pragma misra_func(io)
int sprintf(char *_s, const char *_fmt, ...);

#if !defined(__DOUBLES_ARE_FLOATS__)
#pragma linkage_name __sscanf64
#endif
#pragma misra_func(io)
int sscanf(const char *_buf, const char *_fmt, ...);

 #ifndef _ADI_LIBIO
#pragma misra_func(io)
FILE * tmpfile(void); 
#pragma misra_func(io)
char * tmpnam(char *_s);
 #endif

#pragma misra_func(io)
int ungetc(int _c, FILE *_str);

#if !defined(__DOUBLES_ARE_FLOATS__)
#pragma linkage_name __vfprintf64
#endif
#pragma misra_func(io)
int vfprintf(FILE *_str, const char *_fmt, _Va_list _ap);

#if !defined(__DOUBLES_ARE_FLOATS__)
#pragma linkage_name __vprintf64
#endif
#pragma misra_func(io)
int vprintf(const char *_fmt, _Va_list _ap);

#if !defined(__DOUBLES_ARE_FLOATS__)
#pragma linkage_name __vsprintf64
#endif
#pragma misra_func(io)
int vsprintf(char *_s, const char *_fmt, _Va_list _ap);

 #ifdef _SOLARIS_C_LIB
FILE * fdopen(_FD_TYPE, const char *);
_FD_TYPE fileno(FILE *);
 #endif /* _SOLARIS_C_LIB */

#pragma misra_func(io)
long _Fgpos(FILE *_str, fpos_t *_ptr);
#pragma misra_func(io)
int _Flocale(FILE *_str, const char *_fmt, int _cat);
#pragma misra_func(io)
void _Fsetlocale(FILE *_locname, int _cat);
#pragma misra_func(io)
int _Fspos(FILE *_str, const fpos_t *_ptr, long _loff, int _way);

 #ifdef _MULTI_THREAD
#pragma misra_func(io)
void _Lockfilelock(_Filet *_file);
#pragma misra_func(io)
void _Unlockfilelock(_Filet *_file);
 #endif /* _MULTI_THREAD */

extern FILE *_Files[FOPEN_MAX];
_END_C_LIB_DECL

 #if defined(_ADI_LIBIO) || \
     (defined(_MULTI_THREAD) && defined(_FILE_OP_LOCKS))
		/* declarations only */
_C_LIB_DECL
#pragma misra_func(io)
int getc(FILE *_str);
#pragma misra_func(io)
int getchar(void);
#pragma misra_func(io)
int putc(int _c, FILE *_str);
#pragma misra_func(io)
int putchar(int _c);
_END_C_LIB_DECL

 #else /* _ADI_LIBIO || (_MULTI_THREAD && _FILE_OP_LOCKS) */

  #ifdef __cplusplus
		/* inlines, for C++ */
#pragma always_inline
inline int getc(FILE *_Str)
	{	/* get a character */
	return ((_Str->_Next < _Str->_Rend
		? *_Str->_Next++ : fgetc(_Str)));
	}

#pragma always_inline
inline int getchar()
	{	/* get a character from stdin */
	return ((_Files[0]->_Next < _Files[0]->_Rend
	? *_Files[0]->_Next++ : fgetc(_Files[0])));
	}

#pragma always_inline
inline int putc(int _C, FILE *_Str)
	{	/* put a character */
	return ((_Str->_Next < _Str->_Wend
		? (*_Str->_Next++ = _C) : fputc(_C, _Str)));
	}

#pragma always_inline
inline int putchar(int _C)
	{	/* put a character to stdout */
	return ((_Files[1]->_Next < _Files[1]->_Wend
	? (*_Files[1]->_Next++ = _C) : fputc(_C, _Files[1])));
	}

  #else /* __cplusplus */
		/* declarations and macro overrides, for C */
_C_LIB_DECL
#pragma misra_func(io)
int getc(FILE *_str);
#pragma misra_func(io)
int getchar(void);
#pragma misra_func(io)
int putc(int _c, FILE *_str);
#pragma misra_func(io)
int putchar(int _c);
_END_C_LIB_DECL

 #define getc(str)	((str)->_Next < (str)->_Rend \
	? *(str)->_Next++ : (fgetc)(str))
 #define getchar()	(_Files[0]->_Next < _Files[0]->_Rend \
	? *_Files[0]->_Next++ : (fgetc)(_Files[0]))
 #define putc(c, str)	((str)->_Next < (str)->_Wend \
	? (*(str)->_Next++ = (c)) : (fputc)((c), (str)))
 #define putchar(c)	(_Files[1]->_Next < _Files[1]->_Wend \
	? (*_Files[1]->_Next++ = (c)) : (fputc)((c), _Files[1]))
  #endif /* __cplusplus */
 #endif /* _MULTI_THREAD && _FILE_OP_LOCKS */
_C_STD_END

#ifdef _MISRA_RULES
#pragma diag(pop)
#endif /* _MISRA_RULES */

#endif /* _STDIO */

#if !defined( __NO_ANSI_EXTENSIONS__ )
#if defined(__ADSPBLACKFIN__)

#ifdef _MISRA_RULES
#pragma diag(push)
#pragma diag(suppress:misra_rule_19_1)
#endif /* _MISRA_RULES */

#include "sys/stdio_bf.h"

#ifdef _MISRA_RULES
#pragma diag(pop)
#endif /* _MISRA_RULES */

#endif  /* __ADSPBLACKFIN__ */
#endif  /* __NO_ANSI_EXTENSIONS__ */

#ifdef _STD_USING
using std::size_t; using std::fpos_t; using std::FILE;
using std::clearerr; using std::fclose; using std::feof;
using std::ferror; using std::fflush; using std::fgetc;
using std::fgetpos; using std::fgets; using std::fopen;
using std::fprintf; using std::fputc; using std::fputs;
using std::fread; using std::freopen; using std::fscanf;
using std::fseek; using std::fsetpos; using std::ftell;
using std::fwrite; using std::getc; using std::getchar;
using std::gets; using std::perror;
using std::putc; using std::putchar;
using std::printf; using std::puts; using std::remove;
using std::rename; using std::rewind; using std::scanf;
using std::setbuf; using std::setvbuf; using std::sprintf;
using std::sscanf; using std::tmpfile; using std::tmpnam;
using std::ungetc; using std::vfprintf; using std::vprintf;
using std::vsprintf;
#endif /* _STD_USING */

/*
 * Copyright (c) 1992-2001 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
 * V3.10:1134 
 * $Revision: 1.11.2.1 $
 */
