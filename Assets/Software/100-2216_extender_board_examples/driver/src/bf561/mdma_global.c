#include <services/services.h>
#include "../common/mdma/MDMAconfig.h"

#include <cdefbf561.h>

#ifdef _FORCE_MDMA_SPEC_TO_L1_
// force code and data from here on to internal memory
#pragma default_section(CODE, "L1_code")
#pragma default_section(DATA, "L1_data_a")
#pragma default_section(CONSTDATA, "L1_data_a")
#pragma default_section(BSZ, "L1_data_a")
#pragma default_section(ALLDATA, "L1_data_a")

#else

//section ("secure_code") needed to be secure that the bs_loader dont overwrites needed code
#pragma default_section(CODE, "secure_code")
#pragma default_section(DATA, "secure_code")
#pragma default_section(CONSTDATA, "secure_code")
#pragma default_section(BSZ, "secure_code")
#pragma default_section(ALLDATA, "secure_code")

#endif

#pragma align 4
T_MDMA_SPEC g_aMDMAspec[] = {
		{
			false,
			ADI_INT_MDMA1_S0,
			ADI_INT_DMA1_ERROR,
			0,
			0,
			pMDMA1_S0_X_COUNT,
			pMDMA1_S0_X_MODIFY,
			pMDMA1_S0_Y_COUNT,
			pMDMA1_S0_Y_MODIFY,
			pMDMA1_S0_CONFIG,
			pMDMA1_S0_IRQ_STATUS,
			(volatile void **)pMDMA1_S0_START_ADDR,
			pMDMA1_D0_X_COUNT,
			pMDMA1_D0_X_MODIFY,			
			pMDMA1_D0_Y_COUNT,
			pMDMA1_D0_Y_MODIFY,			
			pMDMA1_D0_CONFIG,
			pMDMA1_D0_IRQ_STATUS,
			(volatile void **)pMDMA1_D0_START_ADDR,
			(volatile void **)pMDMA1_S0_CURR_DESC_PTR,
			(volatile void **)pMDMA1_S0_NEXT_DESC_PTR,
			(volatile void **)pMDMA1_D0_CURR_DESC_PTR,
			(volatile void **)pMDMA1_D0_NEXT_DESC_PTR
		},
		{
			false,
			ADI_INT_MDMA1_S1,
			ADI_INT_DMA1_ERROR,
			0,
			0,
			pMDMA1_S1_X_COUNT,
			pMDMA1_S1_X_MODIFY,
			pMDMA1_S1_Y_COUNT,
			pMDMA1_S1_Y_MODIFY,
			pMDMA1_S1_CONFIG,
			pMDMA1_S1_IRQ_STATUS,
			(volatile void **)pMDMA1_S1_START_ADDR,
			pMDMA1_D1_X_COUNT,
			pMDMA1_D1_X_MODIFY,			
			pMDMA1_D1_Y_COUNT,
			pMDMA1_D1_Y_MODIFY,			
			pMDMA1_D1_CONFIG,
			pMDMA1_D1_IRQ_STATUS,
			(volatile void **)pMDMA1_D1_START_ADDR,
			(volatile void **)pMDMA1_S1_CURR_DESC_PTR,
			(volatile void **)pMDMA1_S1_NEXT_DESC_PTR,
			(volatile void **)pMDMA1_D1_CURR_DESC_PTR,
			(volatile void **)pMDMA1_D1_NEXT_DESC_PTR
		},
		{
			false,
			ADI_INT_MDMA2_S0,
			ADI_INT_DMA2_ERROR,
			0,
			0,
			pMDMA2_S0_X_COUNT,
			pMDMA2_S0_X_MODIFY,
			pMDMA2_S0_Y_COUNT,
			pMDMA2_S0_Y_MODIFY,
			pMDMA2_S0_CONFIG,
			pMDMA2_S0_IRQ_STATUS,
			(volatile void **)pMDMA2_S0_START_ADDR,
			pMDMA2_D0_X_COUNT,
			pMDMA2_D0_X_MODIFY,			
			pMDMA2_D0_Y_COUNT,
			pMDMA2_D0_Y_MODIFY,			
			pMDMA2_D0_CONFIG,
			pMDMA2_D0_IRQ_STATUS,
			(volatile void **)pMDMA2_D0_START_ADDR,
			(volatile void **)pMDMA2_S0_CURR_DESC_PTR,
			(volatile void **)pMDMA2_S0_NEXT_DESC_PTR,
			(volatile void **)pMDMA2_D0_CURR_DESC_PTR,
			(volatile void **)pMDMA2_D0_NEXT_DESC_PTR
		},
		{
			false,
			ADI_INT_MDMA2_S1,
			ADI_INT_DMA2_ERROR,
			0,
			0,
			pMDMA2_S1_X_COUNT,
			pMDMA2_S1_X_MODIFY,
			pMDMA2_S1_Y_COUNT,
			pMDMA2_S1_Y_MODIFY,
			pMDMA2_S1_CONFIG,
			pMDMA2_S1_IRQ_STATUS,
			(volatile void **)pMDMA2_S1_START_ADDR,
			pMDMA2_D1_X_COUNT,
			pMDMA2_D1_X_MODIFY,			
			pMDMA2_D1_Y_COUNT,
			pMDMA2_D1_Y_MODIFY,			
			pMDMA2_D1_CONFIG,
			pMDMA2_D1_IRQ_STATUS,
			(volatile void **)pMDMA2_D1_START_ADDR,
			(volatile void **)pMDMA2_S1_CURR_DESC_PTR,
			(volatile void **)pMDMA2_S1_NEXT_DESC_PTR,
			(volatile void **)pMDMA2_D1_CURR_DESC_PTR,
			(volatile void **)pMDMA2_D1_NEXT_DESC_PTR			
		}		
	};		 

	
unsigned int g_nMDMAcount = sizeof(g_aMDMAspec) / sizeof(T_MDMA_SPEC);	//calculate the nr of elements in the array
	bool mdma_platformInit(void) {
	return true;
}

#pragma default_section(CODE)
#pragma default_section(DATA)
#pragma default_section(CONSTDATA)
#pragma default_section(BSZ)
#pragma default_section(ALLDATA)

