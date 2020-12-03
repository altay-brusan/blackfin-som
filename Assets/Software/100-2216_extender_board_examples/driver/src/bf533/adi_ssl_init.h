/**
 *	@file 		adi_ssl_init.h
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
 
 #ifndef _ADI_SSH_INIT_H_
 #define _ADI_SSH_INIT_H_
 
#include <services/services.h>
#include <drivers/adi_dev.h>

 
 /**
*	@public
*	@brief Initialize the ADI System Service Libraries
*	
*	Initialize the interrupt manager, the power management the device manager 
*   and sets the processor frequencies.
*	
*	@param  pa_hDevMng              Handle to the Device Manager
*   @param  pa_acIntMngStorage      An area of memory used by the interrupt manager
*   @param  pa_nIntMngStorageSize   Size of memory used by interrupt manager
*   @param  pa_acDevMngStorage      An area of memory used by the device manager
*   @param  pa_nDevMngStorageSize   Size of memory used by device manager
*
*	@return Error code
*
**/
unsigned long ADISSLinit(ADI_DEV_MANAGER_HANDLE *pa_hDevMng, 
                         unsigned char *pa_acIntMngStorage, 
                         unsigned long pa_nIntMngStorageSize,
                         unsigned char *pa_acDevMngStorage, 
                         unsigned long pa_nDevMngStorageSize);

 
#endif
