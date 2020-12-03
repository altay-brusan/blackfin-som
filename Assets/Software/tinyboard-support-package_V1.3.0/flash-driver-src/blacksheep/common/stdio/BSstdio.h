/** *********************************************************************
 * @file bsio.h
 * @author Daniel Weber, Roland Oberhammer
 * @date 2005-10-23
 * @version 1.0
 * @brief file system functions for the BLACKSheep environment.
 *
 *
 * @see BSstdio.c
  ********************************************************************* **/
#ifndef _BS_STDIO_H_INCLUDED_
#define _BS_STDIO_H_INCLUDED_

#include <stddef.h>
#include <Datatypes.h>

#define PARTITION_MARKER 	':'
#define DIR_SEPARATOR		'\\'
#define DIR_SEPARATOR_STR	"\\"

/*********************************************/
/*               ERROR Codes                 */
/*********************************************/
#define ERR_BS_STDIO_PARTITION_NOT_FOUND        (-1)        ///< Specified partition not found
#define ERR_BS_STDIO_DST_PARTITION_NOT_FOUND    (-1)        ///< Specified dst partition not found
#define ERR_BS_STDIO_SRC_PARTITION_NOT_FOUND    (-1)        ///< Specified src partition not found



typedef struct {	
		void *pPartition;
		void *pFile;
	} BS_FILE;
	
typedef struct {
	BYTE nHour;
	BYTE nMinute;
	BYTE nSecond;
	WORD nYear;
	BYTE nMonth;
	BYTE nDay;
	} T_FF_DATE;
	
typedef struct {
		void *pPartition;
		void *pInfo;
		char acName[255];
		WORD nAttr;
		QWORD nSize;
		T_FF_DATE stCrDate;	///< creation date
		T_FF_DATE stMoDate;	///< modification date
		T_FF_DATE stAcDate; ///< last access date
		unsigned short cEntry;	///< EFSL Library Specific
		unsigned short rEntry;	///< EFSL Library Specific
		char cache[21];			///< EFSL Library Specific
		void *pParam;
} T_FF_INFO;
	
#define EOF 		(-1)
#define SEEK_SET 	0
#define SEEK_CUR 	1
#define SEEK_END 	2

#define BUFSIZ		512
	
/// corresponds to the classical fopen function from <stdio.h>
extern "C" BS_FILE *bs_fopen( const char *fname, const char *mode );
/// corresponds to the classical fclose function from <stdio.h>
extern "C" int bs_fclose( BS_FILE *stream );
/// corresponds to the classical feof function from <stdio.h>
extern "C" int bs_feof( BS_FILE *stream );
/// corresponds to the classical fread function from <stdio.h>
extern "C" int bs_fread( void *buffer, size_t size, size_t num, BS_FILE *stream );
/// corresponds to the classical fwrite function from <stdio.h>
extern "C" int bs_fwrite( const void *buffer, size_t size, size_t count, BS_FILE *stream );
/// corresponds to the classical fputc function from <stdio.h>
extern "C" int bs_fputc( int ch, BS_FILE *stream );
/// corresponds to the classical fgetc function from <stdio.h>
extern "C" int bs_fgetc( BS_FILE *stream );
/// corresponds to the classical fgets function from <stdio.h>
extern "C" char *bs_fgets( char *s, int n, BS_FILE *stream );

extern "C" int bs_fputs( const char * str, BS_FILE * stream );	// Added a missing fputs() function (James W)

/// corresponds to the classical fseek function from <stdio.h>
extern "C" int bs_fseek( BS_FILE *stream, long offset, int origin );
/// corresponds to the classical ftell function from <stdio.h>
extern "C" long bs_ftell( BS_FILE *stream );
/// deletes a file
extern "C" int bs_fdelete(const char *pa_acFilename);
/// changes the current working directory.
extern "C" int bs_chdir(const char *pa_acDirectory);
extern "C" int bs_mkdir(const char *pa_acDirectory);
extern "C" int bs_rmdir(const char *pa_acDirectory);
extern "C" int bs_ferror (BS_FILE *stream);
extern "C" int bs_move(const char *pa_acSourceFilename, const char *pa_acDestFilename);

bool expandFullPath(char *pa_acTarget, bool pa_bIncludePartition);

extern "C" int bs_findfirst(const char *pathname, T_FF_INFO *ffblk, int attrib);
extern "C" int bs_findnext(T_FF_INFO *ffblk);
extern "C" int bs_findclose(T_FF_INFO *ffblk);

extern "C" int bs_getchar( void );
extern "C" int bs_putchar( int ch );

#if defined (_USE_VDK_) && defined (_USE_BUFFERED_IO_)

// these prototypes are only accessible with VDK usage
extern "C" int bs_vdk_getchar( void );
extern "C" int bs_vdk_putchar( int ch );

#endif

extern "C" BS_FILE *bs_stdin;
extern "C" BS_FILE *bs_stdout;
extern "C" BS_FILE *bs_stderr;

#endif

