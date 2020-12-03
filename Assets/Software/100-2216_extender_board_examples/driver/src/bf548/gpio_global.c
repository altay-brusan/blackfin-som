/**
 *	@file 		gpio_global.c
 *	@ingroup 	GPIO_HAL_548
 *	
 *	@brief 		GPIO Bank Definition for the BF548 Processor
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
/** @defgroup GPIO_HAL_548 GPIO HAL for BF-548
 *  @ingroup 	GPIO
 *  @brief 		HAL for BF-548
 *	
 *						GPIO Hardware Abstraction Layer for BF-548
 */


 
#include "../common/GPIOconfig.h" 

#include <cdefbf548.h>
#include <services/services.h>

T_GPIO_BANKS g_aGPIObanks[] = {
		{
			pPORTA_FER ,
			pPORTA,
			pPORTA_SET ,
			pPORTA_CLEAR ,
			pPORTA_DIR_SET ,
			pPORTA_DIR_CLEAR ,
			pPORTA_INEN,
			pPORTA_MUX,
			0,
			1				
		},	
		{
			pPORTB_FER ,
			pPORTB,
			pPORTB_SET ,
			pPORTB_CLEAR ,
			pPORTB_DIR_SET ,
			pPORTB_DIR_CLEAR ,
			pPORTB_INEN,
			pPORTB_MUX,
			0,
			1	
		},
		{
			pPORTC_FER ,
			pPORTC,
			pPORTC_SET ,
			pPORTC_CLEAR ,
			pPORTC_DIR_SET ,
			pPORTC_DIR_CLEAR ,
			pPORTC_INEN,
			pPORTC_MUX,
			2,
			3			
		},
		{
			pPORTD_FER ,
			pPORTD,
			pPORTD_SET ,
			pPORTD_CLEAR ,
			pPORTD_DIR_SET ,
			pPORTD_DIR_CLEAR ,
			pPORTD_INEN,
			pPORTD_MUX,
			2,
			3						
		},
		{
			pPORTE_FER ,
			pPORTE,
			pPORTE_SET ,
			pPORTE_CLEAR ,
			pPORTE_DIR_SET ,
			pPORTE_DIR_CLEAR ,
			pPORTE_INEN,
			pPORTE_MUX,
			2,
			3						
		},
		{
			pPORTF_FER ,
			pPORTF,
			pPORTF_SET ,
			pPORTF_CLEAR ,
			pPORTF_DIR_SET ,
			pPORTF_DIR_CLEAR ,
			pPORTF_INEN,
			pPORTF_MUX,
			2,
			3						
		},
		{
			pPORTG_FER ,
			pPORTG,
			pPORTG_SET ,
			pPORTG_CLEAR ,
			pPORTG_DIR_SET ,
			pPORTG_DIR_CLEAR ,
			pPORTG_INEN,
			pPORTG_MUX,
			2,
			3						
		},
		{
			pPORTH_FER ,
			pPORTH,
			pPORTH_SET ,
			pPORTH_CLEAR ,
			pPORTH_DIR_SET ,
			pPORTH_DIR_CLEAR ,
			pPORTH_INEN,
			pPORTH_MUX,
			2,
			3						
		},
		{
			pPORTI_FER ,
			pPORTI,
			pPORTI_SET ,
			pPORTI_CLEAR ,
			pPORTI_DIR_SET ,
			pPORTI_DIR_CLEAR ,
			pPORTI_INEN,
			pPORTI_MUX,
			2,
			3						
		},
		{
			pPORTJ_FER ,
			pPORTJ,
			pPORTJ_SET ,
			pPORTJ_CLEAR ,
			pPORTJ_DIR_SET ,
			pPORTJ_DIR_CLEAR ,
			pPORTJ_INEN,
			pPORTJ_MUX,
			2,
			3						
		}														
	};
	
	
T_GPIO_PINT g_aGPIOpints[] = {
		{
            ADI_INT_PINT0,   		    
		    pPINT0_MASK_SET,
		    pPINT0_MASK_CLEAR,
		    pPINT0_REQUEST,
		    pPINT0_ASSIGN,
		    pPINT0_EDGE_SET,
		    pPINT0_EDGE_CLEAR,
		    pPINT0_INVERT_SET,
		    pPINT0_INVERT_CLEAR,
		    pPINT0_PINSTATE,
		    pPINT0_LATCH
		},
		{
		    ADI_INT_PINT1,
		    pPINT1_MASK_SET,
		    pPINT1_MASK_CLEAR,
		    pPINT1_REQUEST,
		    pPINT1_ASSIGN,
		    pPINT1_EDGE_SET,
		    pPINT1_EDGE_CLEAR,
		    pPINT1_INVERT_SET,
		    pPINT1_INVERT_CLEAR,
		    pPINT1_PINSTATE,
		    pPINT1_LATCH
		},
		{
		    ADI_INT_PINT2,
		    pPINT2_MASK_SET,
		    pPINT2_MASK_CLEAR,
		    pPINT2_REQUEST,
		    pPINT2_ASSIGN,
		    pPINT2_EDGE_SET,
		    pPINT2_EDGE_CLEAR,
		    pPINT2_INVERT_SET,
		    pPINT2_INVERT_CLEAR,
		    pPINT2_PINSTATE,
		    pPINT2_LATCH
		},
		{
		    ADI_INT_PINT3,
		    pPINT3_MASK_SET,
		    pPINT3_MASK_CLEAR,
		    pPINT3_REQUEST,
		    pPINT3_ASSIGN,
		    pPINT3_EDGE_SET,
		    pPINT3_EDGE_CLEAR,
		    pPINT3_INVERT_SET,
		    pPINT3_INVERT_CLEAR,
		    pPINT3_PINSTATE,
		    pPINT3_LATCH
		}
};

unsigned int g_nGPIObankCount = sizeof(g_aGPIObanks) / sizeof(T_GPIO_BANKS);
unsigned int g_nGPIOpintCount = sizeof(g_aGPIOpints) / sizeof(T_GPIO_PINT);

