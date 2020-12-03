#include "../common/mdma/MDMAconfig.h"

#include <cdefbf548.h>

//section ("secure_code") needed to be secure that the bs_loader dont overwrites needed code
#pragma default_section(CODE, "secure_code")
#pragma default_section(DATA, "secure_code")
#pragma default_section(CONSTDATA, "secure_code")
#pragma default_section(BSZ, "secure_code")
#pragma default_section(ALLDATA, "secure_code")

#pragma align 4
T_MDMA_SPEC g_aMDMAspec[] = {
		{
			false,
			ADI_INT_MDMA_S0,
			ADI_INT_DMAC0_ERROR,
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
			(volatile void **)MDMA_D0_START_ADDR
		},
		{
			false,
			ADI_INT_MDMA_S1,
			ADI_INT_DMAC0_ERROR,
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
			(volatile void **)MDMA_D1_START_ADDR
		},
		{
			false,
			ADI_INT_MDMA_S2,
			ADI_INT_DMAC1_ERROR,
			0,
			0,
			pMDMA_S2_X_COUNT,
			pMDMA_S2_X_MODIFY,
			pMDMA_S2_Y_COUNT,
			pMDMA_S2_Y_MODIFY,
			pMDMA_S2_CONFIG,
            pMDMA_S2_IRQ_STATUS,
			(volatile void **)MDMA_S2_START_ADDR,
			pMDMA_D2_X_COUNT,
			pMDMA_D2_X_MODIFY,			
			pMDMA_D2_Y_COUNT,
			pMDMA_D2_Y_MODIFY,			
			pMDMA_D2_CONFIG,
			pMDMA_D2_IRQ_STATUS,
			(volatile void **)MDMA_D2_START_ADDR
		},
		{
			false,
			ADI_INT_MDMA_S3,
			ADI_INT_DMAC1_ERROR,
			0,
			0,
			pMDMA_S3_X_COUNT,
			pMDMA_S3_X_MODIFY,
			pMDMA_S3_Y_COUNT,
			pMDMA_S3_Y_MODIFY,
			pMDMA_S3_CONFIG,
            pMDMA_S3_IRQ_STATUS,
			(volatile void **)MDMA_S3_START_ADDR,
			pMDMA_D3_X_COUNT,
			pMDMA_D3_X_MODIFY,			
			pMDMA_D3_Y_COUNT,
			pMDMA_D3_Y_MODIFY,			
			pMDMA_D3_CONFIG,
			pMDMA_D3_IRQ_STATUS,
			(volatile void **)MDMA_D3_START_ADDR
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
