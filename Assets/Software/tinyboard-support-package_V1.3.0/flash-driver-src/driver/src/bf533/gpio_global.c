/**
 *	@file 		gpio_global.c
 *	@ingroup 	GPIO_HAL_533
 *	
 *	@brief 		GPIO Bank Definition for the BF533 Processor
 *	
 *		
 *	BLT_DISCLAIMER
 *	
 *	@author 	Roland Oberhammer, 
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
/** @defgroup GPIO_HAL_533 GPIO HAL for BF-533
 *  @ingroup 	GPIO
 *  @brief 		HAL for BF-533
 *	
 *						GPIO Hardware Abstraction Layer for BF-533
 */


#include "../common/GPIOconfig.h"

#include <cdefbf533.h>
#include <services/services.h>
/*
T_GPIO_SPEC g_stGPIOspec = {
		ADI_INT_PFA,
		ADI_INT_PFB
	};
*/
T_GPIO_BANKS g_aGPIObanks[] = {
		{
			ADI_INT_PFA,
			ADI_INT_PFB,
			pFIO_FLAG_C,
			pFIO_FLAG_D,
			pFIO_FLAG_S,
			pFIO_FLAG_T,
			pFIO_INEN,
			pFIO_DIR,
			pFIO_EDGE,
			pFIO_POLAR,
			pFIO_BOTH,
			pFIO_MASKA_C,
			pFIO_MASKA_S,
			pFIO_MASKB_C,
			pFIO_MASKB_S,
			0,
			0
		}
	};


unsigned int g_nGPIObankCount = sizeof(g_aGPIObanks) / sizeof(T_GPIO_BANKS);

