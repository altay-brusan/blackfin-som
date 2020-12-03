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
 #include <math.h>
 
 
 
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


void DPM_SetDspClocks(unsigned long coreclk, unsigned long systemclk, unsigned long clkin) {
	unsigned short msel, ssel, imask;
	

	
}

unsigned long ADISSLinit(ADI_DEV_MANAGER_HANDLE *pa_hDevMng, 
                         unsigned char *pa_acIntMngStorage, 
                         unsigned long pa_nIntMngStorageSize,
                         unsigned char *pa_acDevMngStorage, 
                         unsigned long pa_nDevMngStorageSize) {

	unsigned long nResult;
	unsigned long nResponseCount;
	unsigned long nCriticalReg;
	unsigned short msel, ssel, imask;

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
	
    
	setClockIn(PROC_CLOCK_FREQ_IN*1000000);
	
	msel = (unsigned short)ceil(((float)(PROC_CLOCK_FREQ_CORE) / (float)(PROC_CLOCK_FREQ_IN)));
	ssel = (unsigned short)ceil(((float)(PROC_CLOCK_FREQ_CORE) / (float)(PROC_CLOCK_FREQ_SYSTEM)));
	
	asm("cli r0; \
		%0 = r0;"
		:"=d"(imask)
		:
		:"r0"); 			//IMASK bitpattern saved in "imask".
		
//	*pPLL_LOCKCNT = 0x400;
	
	*pSIC_IWR0 |= 0x1;		//Enable PLL-interrupt to wakeup processor from idle-mode.

	*pPLL_DIV = ssel;
	
	msel = msel << 9;
	*pPLL_CTL = msel;
	
							//PLL programming sequence.
	asm ("r0 = %0; \
		idle; \
		sti r0;"
		:
		:"d"(imask)
		:"r0"); 
		      
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
