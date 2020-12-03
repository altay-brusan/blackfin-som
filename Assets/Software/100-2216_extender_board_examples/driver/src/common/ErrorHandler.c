#include <stdio.h>
#include "ErrorHandler.h"
#include "global_int_id.h"

#ifndef _DONT_SHOW_HW_ERRORS_ 
	#include "../../../blacksheep/common/syslog/syslog.h"
#endif

void errorHandler(T_ERROR_SOURCE pa_nSource, T_ERROR_CODE pa_nCode, T_ERROR_LEVEL pa_nLevel) {
	// TODO think
	/*
#ifndef _DONT_SHOW_HW_ERRORS_	
	printf("Error: source(%d, 0x%X), code(%d, 0x%X), level(%d)\n", pa_nSource, pa_nSource, pa_nCode, pa_nCode, pa_nLevel);
#endif
	*/	
#ifndef _DONT_SHOW_HW_ERRORS_	
	char acMessage[50];
	sprintf (acMessage, "ERROR on UART, code 0x%x.", pa_nCode);

#ifndef _SYSLOG_DISABLE_

#ifdef _USE_VDK_
	syslog_post (acMessage, SYSLOG_LEVEL_WARNING, pa_nSource, 0);
#endif
#endif

#endif
}

/*
ADI_INT_HANDLER_RESULT ErrorIntHandler(void *pa_pClientArg) {
	
	T_INT_HEADER *pHeader = (T_INT_HEADER*)pa_pClientArg;
	// check if the source of the interrupt (error) is known
	switch( pHeader->nIntType ) {
		case INT_TYPE_UART:
			//TODO clear the interrupt flags
			errorHandler(pHeader->nDeviceID, *(pHeader->pErrorReg), ERR_LEV_HARDWARE);
			return ADI_INT_RESULT_PROCESSED;			
							
		case INT_TYPE_SPI:
			//TODO clear the interrupt flags
			errorHandler(pHeader->nDeviceID, *(pHeader->pErrorReg), ERR_LEV_HARDWARE);
			return ADI_INT_RESULT_PROCESSED;			
		
		default:
			// unknown source - dont process
			return ADI_INT_RESULT_NOT_PROCESSED;
	}
	
}
*/
