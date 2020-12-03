#ifndef _PRINTF_H_INCLUDED_
#define _PRINTF_H_INCLUDED_

#include <stdarg.h>
#include "BSstdio.h"

extern "C" int bs_printf (const char *sFormat, ...);
extern "C" int bs_fprintf (BS_FILE *stream, const char *sFormat, ...);

#ifdef _USE_VDK_

int bs_vdk_printf (const char *sFormat, ...);

#endif

#endif

