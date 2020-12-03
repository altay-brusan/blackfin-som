#ifndef _ERROR_HANDLER_H_INCLUDED_
#define _ERROR_HANDLER_H_INCLUDED_

#include <services/services.h>
#include <Datatypes.h>


typedef unsigned int T_ERROR_SOURCE;
typedef unsigned int T_ERROR_LEVEL;

#define ERR_LEV_WARNING		0
#define ERR_LEV_CRITICAL	1
#define ERR_LEV_FATAL		2
#define ERR_LEV_HARDWARE    3


void errorHandler(T_ERROR_SOURCE pa_nSource, T_ERROR_CODE pa_nCode, T_ERROR_LEVEL pa_nLevel);

ADI_INT_HANDLER_RESULT ErrorIntHandler(void *pa_pClientArg);

#endif

