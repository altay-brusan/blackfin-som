#ifndef _STDIO_H_INCLUDED_
#define _STDIO_H_INCLUDED_

//#include <Environment.h>
#include <stddef.h>

#define MAX_PATH	2600		// This is a standard value used by Windows and Unix environments.
								// Before this was defines, max paths were 260 chars by MAX_PATH_NAME
								// 260 is a typical max filename length.
								
// TODO: 	I think that stdio.h also has a define called MAX_FILES or MAX_OPEN which limits the number
//			of fopen() calls which can succeed. The value is usually set to 20, and is something that 
//			technically BlackSHEEP should monitor. FullFAT will just open an unlimited number of files.
//

#if !defined(_USE_JTAG_IO_)

#include "..\common\stdio\BSstdio.h"
#include "..\common\stdio\printf.h"


#define O_ACCMODE	   0x0003
#define O_RDONLY	     0x00
#define O_WRONLY	     0x01
#define O_RDWR		     0x02
#define O_CREAT		   0x0100	/* not fcntl */
#define O_EXCL		   0x0200	/* not fcntl */
#define O_NOCTTY	   0x0400	/* not fcntl */
#define O_TRUNC		  0x01000	/* not fcntl */
#define O_APPEND	  0x02000
#define O_NONBLOCK	  0x04000
#define O_NDELAY	O_NONBLOCK
#define O_SYNC		 0x010000
#define FASYNC		 0x020000	/* fcntl, for BSD compatibility */
#define O_DIRECT	 0x040000	/* direct disk access hint - currently ignored */
#define O_LARGEFILE	0x0100000
#define O_DIRECTORY	0x0200000	/* must be a directory */
#define O_NOFOLLOW	0x0400000 /* don't follow links */
#define O_BINARY    0x0800000

#define FILE	BS_FILE

#define stdin bs_stdin
#define stdout bs_stdout
#define stderr bs_stderr

//globals for stdin, stdout and stderr
extern 	FILE 		*stdin;
extern 	FILE 		*stdout;
extern 	FILE 		*stderr;

#define fopen 		bs_fopen
#define fclose 		bs_fclose
#define feof		bs_feof
#define fread		bs_fread
#define fwrite		bs_fwrite
#define fputc		bs_fputc
#define fgetc		bs_fgetc
#define fgets		bs_fgets
#define fseek		bs_fseek
#define fdelete		bs_fdelete
#define fputs		bs_fputs
#define	ftell		bs_ftell		// ftell call redirection was missing!

#define chdir		bs_chdir
#define mkdir		bs_mkdir
#define rmdir		bs_rmdir

#define fprintf		bs_fprintf

/*#ifdef _USE_VDK_
#define printf		bs_vdk_printf
#else*/
#define printf		bs_printf
//#endif

#if defined(_USE_VDK_)
#define putchar		bs_vdk_putchar
#define getchar		bs_vdk_getchar
#else
#define putchar		bs_putchar
#define getchar		bs_getchar
#endif

int sprintf( char *buffer, const char *format, ... );

// Added a sscanf prototype! James W
int sscanf( char *buffer, const char *format, ...);

#elif defined(_USE_CCES_)
    #include <stdio_cces.h>
#else

// include vdsp++ sdtio.h
#include <stdio_vdsp.h>

#endif  // _USE_JTAG_IO_



#endif

