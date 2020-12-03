/**
 *	@file 		gpio_global.c
 *	@ingroup 	GPIO_HAL_518
 *	
 *	@brief 		GPIO Bank Definition for the BF518 Processor
 *	
 *		
 *	BLT_DISCLAIMER
 *	
 *	@author 	Roland Oberhammer, 
 *	
 *	@cond svn
 *	
 *	Information of last commit
*   $Rev::               $:  Revision of last commit
*   $Author::            $:  Author of last commit
*   $Date::              $:  Date of last commit
 *	
 *	@endcond
 **/
/** @defgroup GPIO_HAL_527 GPIO HAL for BF-518
 *  @ingroup 	GPIO
 *  @brief 		HAL for BF-518
 *	
 *						GPIO Hardware Abstraction Layer for BF-518
 */


#include "../common/GPIOconfig.h"

#include <cdefbf518.h>
#include <services/services.h>


T_GPIO_BANKS g_aGPIObanks[] = {
		{
			ADI_INT_PORTF_INTA,
			ADI_INT_PORTF_INTB,
			pPORTFIO_CLEAR,
			pPORTFIO,
			pPORTFIO_SET,
			pPORTFIO_TOGGLE,
			pPORTFIO_INEN,
			pPORTFIO_DIR,
			pPORTFIO_EDGE,
			pPORTFIO_POLAR,
			pPORTFIO_BOTH,
			pPORTFIO_MASKA_CLEAR,
			pPORTFIO_MASKA_SET,
			pPORTFIO_MASKB_CLEAR,
			pPORTFIO_MASKB_SET,
			pPORTF_FER,
			pPORTF_MUX
		},
		{
			ADI_INT_PORTG_INTA,
			ADI_INT_PORTG_INTB,
			pPORTGIO_CLEAR,
			pPORTGIO,
			pPORTGIO_SET,
			pPORTGIO_TOGGLE,
			pPORTGIO_INEN,
			pPORTGIO_DIR,
			pPORTGIO_EDGE,
			pPORTGIO_POLAR,
			pPORTGIO_BOTH,
			pPORTGIO_MASKA_CLEAR,
			pPORTGIO_MASKA_SET,
			pPORTGIO_MASKB_CLEAR,
			pPORTGIO_MASKB_SET,
			pPORTG_FER,
			pPORTG_MUX			
		},
		{
			ADI_INT_PORTH_INTA,
			ADI_INT_PORTH_INTB,
			pPORTHIO_CLEAR,
			pPORTHIO,
			pPORTHIO_SET,
			pPORTHIO_TOGGLE,
			pPORTHIO_INEN,
			pPORTHIO_DIR,
			pPORTHIO_EDGE,
			pPORTHIO_POLAR,
			pPORTHIO_BOTH,
			pPORTHIO_MASKA_CLEAR,
			pPORTHIO_MASKA_SET,
			pPORTHIO_MASKB_CLEAR,
			pPORTHIO_MASKB_SET,
			pPORTH_FER,
			pPORTH_MUX	
		}
	};


unsigned int g_nGPIObankCount = sizeof(g_aGPIObanks) / sizeof(T_GPIO_BANKS);

