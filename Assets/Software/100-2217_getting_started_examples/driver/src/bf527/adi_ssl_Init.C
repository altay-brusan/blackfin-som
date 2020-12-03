/**
 *	@file 		adi_ssl_init.c
 *	@ingroup 	ADI SSL
 *	
 *	@brief 		ADI System Service Library initialization SD Card controlled by Secure Digital Host driver
 *	
 *						
 *		
 *	BLT_DISCLAIMER
 *	
 *	@author 	Walter Craffonara
 *	
 *	@cond svn
 *	
 *	Information of last commit
 *	$Rev::               $:  Revision of last commit
 *	$Author::            $:  Author of last commit
 *	$Date::              $:  Date of last commit
 *	
 *	@endcond
 **/

 #include "adi_ssl_init.h"
 #include <environment.h>
 #include <VDK.h>
 
 
 
 void adjustCoreVoltage(void) {
	unsigned long nImask;
	asm("cli r0; \
		%0 = r0;"
		:"=d"(nImask)
		:
		:"r0"); 

	*pVR_CTL &= ~0xf0;
	*pVR_CTL |= 0xc0;
	//PLL Programming sequence to lock the pll.
	asm ("r0 = %0; \
		idle; \
		sti r0;"
		:
		:"d"(nImask)
		:"r0");
}

 
/**
*	@public
*	@brief Initialize the ADI System Service Libraries
*	
*	Initialize the interrupt manager, the power management the device manager 
*   and sets the processor frequencies.
*	
*	@param  pa_hDevMng          Handle to the Device Manager
*   @param  pa_acIntMngStorage  An area of memory used by the interrupt manager
*   @param  pa_acDevMngStorage  An area of memory used by the device manager
*
*	@return Error code
*
**/
unsigned long ADISSLinit(ADI_DEV_MANAGER_HANDLE *pa_hDevMng, 
                         unsigned char *pa_acIntMngStorage, 
                         unsigned long pa_nIntMngStorageSize,
                         unsigned char *pa_acDevMngStorage, 
                         unsigned long pa_nDevMngStorageSize) {

	unsigned long nResult;
	unsigned long nResponseCount;
	unsigned long nCriticalReg;

	// initialize the interrupt manager
	nResult = adi_int_Init(pa_acIntMngStorage,
                           pa_nIntMngStorageSize,
                           &nResponseCount,
                           &nCriticalReg);                                                                                
	if (nResult != ADI_DEV_RESULT_SUCCESS) {
		return nResult;
	}
	

	
	// configuration table for power management	
	ADI_PWR_COMMAND_PAIR tPowerTable [] = {	       
		{ ADI_PWR_CMD_SET_PROC_VARIANT, 	(void*)PROC_VARIANT },
		{ ADI_PWR_CMD_SET_PACKAGE, 			(void*)PROC_PACKAGE },
		{ ADI_PWR_CMD_SET_VDDEXT, 			(void*)PROC_VDDEXT }, 
		{ ADI_PWR_CMD_SET_CLKIN, 			(void*)PROC_CLOCK_FREQ_IN},	
		{ ADI_PWR_CMD_END,			        NULL},
	};
	
	// initialize power management
	nResult = adi_pwr_Init(tPowerTable);
	if (nResult != ADI_DEV_RESULT_SUCCESS) {
		return nResult;
	}

	//set optimal frequencies
#ifdef PROC_CLOCK_USE_PLL_INPUT_DIV
	nResult = adi_pwr_SetFreq(PROC_CLOCK_FREQ_CORE, PROC_CLOCK_FREQ_SYSTEM, ADI_PWR_DF_ON);
#else    
	nResult = adi_pwr_SetFreq(PROC_CLOCK_FREQ_CORE, PROC_CLOCK_FREQ_SYSTEM, ADI_PWR_DF_OFF);
#endif

	if (nResult != ADI_DEV_RESULT_SUCCESS) {
		return nResult;
	}
	    
    nCriticalReg = 0;
    ADI_INT_IMASK    imask_storage;
	// initialize the device manager
	nResult = adi_dev_Init( pa_acDevMngStorage,
                            pa_nDevMngStorageSize,
                            &nResponseCount,
                            pa_hDevMng,
                            &imask_storage);
	if (nResult != ADI_DEV_RESULT_SUCCESS) {
        *pa_hDevMng = 0;
		return nResult;
	}

	return 0;	
}
