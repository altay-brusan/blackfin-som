/**
 *	@file 		gpio_global.c
 *	@ingroup 	GPIO_HAL_561
 *	
 *	@brief 		GPIO Bank Definition for the BF561 Processor
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
/** @defgroup GPIO_HAL_561 GPIO HAL for BF-561
 *  @ingroup 	GPIO
 *  @brief 		HAL for BF-561
 *	
 *						GPIO Hardware Abstraction Layer for BF-561
 */

#include "../common/GPIOconfig.h"

#include <cdefbf561.h>
#include <services/services.h>
/*
T_GPIO_SPEC g_stGPIOspec = {
		ADI_INT_PF0_15_A,
		ADI_INT_PF0_15_B
	};
*/
T_GPIO_BANKS g_aGPIObanks[] = {
		{
			ADI_INT_PF0_15_A,
			ADI_INT_PF0_15_B,
			pFIO0_FLAG_C,
			pFIO0_FLAG_D,
			pFIO0_FLAG_S,
			pFIO0_FLAG_T,
			pFIO0_INEN,
			pFIO0_DIR,
			pFIO0_EDGE,
			pFIO0_POLAR,
			pFIO0_BOTH,
			pFIO0_MASKA_C,
			pFIO0_MASKA_S,
			pFIO0_MASKB_C,
			pFIO0_MASKB_S,
			0,
			0
		},
		{
			ADI_INT_PF16_31_A,
			ADI_INT_PF16_31_B,
			pFIO1_FLAG_C,
			pFIO1_FLAG_D,
			pFIO1_FLAG_S,
			pFIO1_FLAG_T,
			pFIO1_INEN,
			pFIO1_DIR,
			pFIO1_EDGE,
			pFIO1_POLAR,
			pFIO1_BOTH,
			pFIO1_MASKA_C,
			pFIO1_MASKA_S,
			pFIO1_MASKB_C,
			pFIO1_MASKB_S,
			0,
			0			
		},
		{
			ADI_INT_PF32_47_A,
			ADI_INT_PF32_47_B,			
			pFIO2_FLAG_C,
			pFIO2_FLAG_D,
			pFIO2_FLAG_S,
			pFIO2_FLAG_T,
			pFIO2_INEN,
			pFIO2_DIR,
			pFIO2_EDGE,
			pFIO2_POLAR,
			pFIO2_BOTH,
			pFIO2_MASKA_C,
			pFIO2_MASKA_S,
			pFIO2_MASKB_C,
			pFIO2_MASKB_S,
			0,
			0			
		}
	};


unsigned int g_nGPIObankCount = sizeof (g_aGPIObanks) / sizeof(T_GPIO_BANKS);

