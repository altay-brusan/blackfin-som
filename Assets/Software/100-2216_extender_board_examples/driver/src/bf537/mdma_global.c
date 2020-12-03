#include "../common/mdma/MDMAconfig.h"

#include <cdefbf537.h>

//section ("secure_code") needed to be secure that the bs_loader dont overwrites needed code
//used in blacksheep
#pragma default_section(CODE, "secure_code")
#pragma default_section(DATA, "secure_code")
#pragma default_section(CONSTDATA, "secure_code")
#pragma default_section(BSZ, "secure_code")
#pragma default_section(ALLDATA, "secure_code")

#pragma align 4
T_MDMA_SPEC g_aMDMAspec[] = {
		{
			false,
			ADI_INT_MDMAS0,
			ADI_INT_DMA_ERROR,
			0,
			0,
			pMDMA_S0_X_COUNT,
			pMDMA_S0_X_MODIFY,
			pMDMA_S0_Y_COUNT,
			pMDMA_S0_Y_MODIFY,
			pMDMA_S0_CONFIG,
			pMDMA_S0_IRQ_STATUS,
			(volatile void **)MDMA_S0_START_ADDR,
			pMDMA_D0_X_COUNT,
			pMDMA_D0_X_MODIFY,			
			pMDMA_D0_Y_COUNT,
			pMDMA_D0_Y_MODIFY,			
			pMDMA_D0_CONFIG,
			pMDMA_D0_IRQ_STATUS,
			(volatile void **)MDMA_D0_START_ADDR,
			(volatile void **)pMDMA_S0_CURR_DESC_PTR,
			(volatile void **)pMDMA_S0_NEXT_DESC_PTR,
			(volatile void **)pMDMA_D0_CURR_DESC_PTR,
			(volatile void **)pMDMA_D0_NEXT_DESC_PTR,
			pDMA_TC_PER
		},
		{
			false,
			ADI_INT_MDMAS1,
			ADI_INT_DMA_ERROR,
			0,
			0,
			pMDMA_S1_X_COUNT,
			pMDMA_S1_X_MODIFY,
			pMDMA_S1_Y_COUNT,
			pMDMA_S1_Y_MODIFY,
			pMDMA_S1_CONFIG,
			pMDMA_S1_IRQ_STATUS,
			(volatile void **)MDMA_S1_START_ADDR,
			pMDMA_D1_X_COUNT,
			pMDMA_D1_X_MODIFY,			
			pMDMA_D1_Y_COUNT,
			pMDMA_D1_Y_MODIFY,			
			pMDMA_D1_CONFIG,
			pMDMA_D1_IRQ_STATUS,
			(volatile void **)MDMA_D1_START_ADDR,
			(volatile void **)pMDMA_S1_CURR_DESC_PTR,
			(volatile void **)pMDMA_S1_NEXT_DESC_PTR,
			(volatile void **)pMDMA_D1_CURR_DESC_PTR,
			(volatile void **)pMDMA_D1_NEXT_DESC_PTR,
			pDMA_TC_PER
		}		

		
	};		

unsigned int g_nMDMAcount = sizeof(g_aMDMAspec) / sizeof(T_MDMA_SPEC);	//calculate the nr of elements in the array

bool mdma_platformInit(void) {
	return true;
}

//return to default sections
#pragma default_section(CODE)
#pragma default_section(DATA)
#pragma default_section(CONSTDATA)
#pragma default_section(BSZ)
#pragma default_section(ALLDATA)
