#include "../common/SPORT/SPORT.h"

#include <cdefbf518.h>
#include <services/services.h>


T_SPORT_SPEC g_aSPORTspec[] = {
		{
			false,
			false,
	//		(ADI_DMA_CHANNEL_ID)0,
			ADI_INT_DMA3_SPORT0_RX,
			ADI_INT_DMA4_SPORT0_TX_AND_RSI,
			ADI_INT_DMA_ERROR,
			ADI_INT_SPORT0_STATUS,
			pSPORT0_TCR1,
			pSPORT0_TCR2,
			pSPORT0_RCR1,
			pSPORT0_RCR2,
			pSPORT0_TX,
			pSPORT0_RX,
			pSPORT0_STAT,
			pSPORT0_TCLKDIV,
			pSPORT0_RCLKDIV,
			pSPORT0_TFSDIV,
			pSPORT0_RFSDIV,
			pSPORT0_MCMC1,
			pSPORT0_MCMC2,
			pSPORT0_MRCS0,
			pSPORT0_MRCS1,
			pSPORT0_MRCS2,
			pSPORT0_MRCS3,
			pSPORT0_MTCS0,
			pSPORT0_MTCS1,
			pSPORT0_MTCS2,
			pSPORT0_MTCS3,
			pDMA3_IRQ_STATUS,
			pDMA3_CONFIG,
			(volatile void **)DMA3_START_ADDR,
			pDMA3_X_MODIFY,
			pDMA3_X_COUNT,
			pDMA3_CURR_X_COUNT,			
			pDMA3_Y_MODIFY,
			pDMA3_Y_COUNT,
			pDMA3_CURR_Y_COUNT,			
			(volatile void **)DMA3_NEXT_DESC_PTR,			
			pDMA4_IRQ_STATUS,
			pDMA4_CONFIG,
			(volatile void **)DMA4_START_ADDR,
			pDMA4_X_MODIFY,
			pDMA4_X_COUNT,
			pDMA4_CURR_X_COUNT,			
			pDMA4_Y_MODIFY,
			pDMA4_Y_COUNT,
			pDMA4_CURR_Y_COUNT,
			(volatile void **)DMA4_NEXT_DESC_PTR,			
		},
		{
			false,
			false,
	//		(ADI_DMA_CHANNEL_ID)0,
			ADI_INT_DMA5_SPORT1_RX_AND_SPI1,
			ADI_INT_DMA6_SPORT1_TX,
			ADI_INT_DMA_ERROR,
			ADI_INT_SPORT1_STATUS,
			pSPORT1_TCR1,
			pSPORT1_TCR2,
			pSPORT1_RCR1,
			pSPORT1_RCR2,
			pSPORT1_TX,
			pSPORT1_RX,
			pSPORT1_STAT,
			pSPORT1_TCLKDIV,
			pSPORT1_RCLKDIV,
			pSPORT1_TFSDIV,
			pSPORT1_RFSDIV,
			pSPORT1_MCMC1,
			pSPORT1_MCMC2,
			pSPORT1_MRCS0,
			pSPORT1_MRCS1,
			pSPORT1_MRCS2,
			pSPORT1_MRCS3,
			pSPORT1_MTCS0,
			pSPORT1_MTCS1,
			pSPORT1_MTCS2,
			pSPORT1_MTCS3,
			pDMA5_IRQ_STATUS,
			pDMA5_CONFIG,
			(volatile void **)DMA5_START_ADDR,
			pDMA5_X_MODIFY,
			pDMA5_X_COUNT,
			pDMA5_CURR_X_COUNT,			
			pDMA5_Y_MODIFY,
			pDMA5_Y_COUNT,
			pDMA5_CURR_Y_COUNT,						
			(volatile void **)DMA5_NEXT_DESC_PTR,						
			pDMA6_IRQ_STATUS,
			pDMA6_CONFIG,
			(volatile void **)DMA6_START_ADDR,
			pDMA6_X_MODIFY,
			pDMA6_X_COUNT,
			pDMA6_CURR_X_COUNT,			
			pDMA6_Y_MODIFY,
			pDMA6_Y_COUNT,
			pDMA6_CURR_Y_COUNT,
			(volatile void **)DMA6_NEXT_DESC_PTR,						
		}
	};
	
unsigned char g_nSPORTcount = sizeof(g_aSPORTspec) / sizeof(T_SPORT_SPEC);		//calculate the nr of elements in the array

bool sport_platformInit(unsigned char pa_cSport, bool bTxNotRx, bool bSecEnable) {
    
	unsigned short nValue;
	
	switch(pa_cSport) {
	    case 0: {
	        if(bTxNotRx) {                  // enable Tx
	            nValue = *pPORTG_FER;
	            *pPORTG_FER |= 0x01c0;
	            *pPORTG_FER |= 0x01c0;
	            
	            nValue = *pPORTG_MUX;
	            *pPORTG_MUX &= ~0x03c0;
	            *pPORTG_MUX &= ~0x03c0;
	            
	            if(bSecEnable) {
	                nValue = *pPORTG_FER;
	                *pPORTG_FER |= 0x0200;
	                *pPORTG_FER |= 0x0200;
	            
	                nValue = *pPORTG_MUX;
	                *pPORTG_MUX &= ~0x0c00;
	                *pPORTG_MUX &= ~0x0c00;
	            }
	        } else {                        // enable Rx
	            nValue = *pPORTG_FER;
	            *pPORTG_FER |= 0x0038;
	            *pPORTG_FER |= 0x0038;
	            
	            nValue = *pPORTG_MUX;
	            *pPORTG_MUX &= ~0x00fc;
	            *pPORTG_MUX &= ~0x00fc;
	            
	            if(bSecEnable) {
	                nValue = *pPORTG_FER;
	                *pPORTG_FER |= 0x0400;
	                *pPORTG_FER |= 0x0400;
	            
	                nValue = *pPORTG_MUX;
	                *pPORTG_MUX &= ~0x0c00;
	                *pPORTG_MUX &= ~0x0c00;
	            }
	        }
	        break;
	    }
	    case 1: {
	        if(bTxNotRx) {                  // enable Tx
	            nValue = *pPORTH_FER;
	            *pPORTH_FER |= 0x0038;
	            *pPORTH_FER |= 0x0038;
	            
	            nValue = *pPORTH_MUX;
	            *pPORTH_MUX &= ~0x000f;
	            *pPORTH_MUX &= ~0x000f;
	            
	            if(bSecEnable) {
	                *pPORTH_FER |= 0x0040;
	                *pPORTH_FER |= 0x0040;
	            
	                nValue = *pPORTH_MUX;
	                *pPORTH_MUX &= ~0x0030;
	                *pPORTH_MUX &= ~0x0030;
	            }
	        } else {                        // enable Rx
	            nValue = *pPORTH_FER;
	            *pPORTH_FER |= 0x0007;
	            *pPORTH_FER |= 0x0007;
	            
	            nValue = *pPORTH_MUX;
	            *pPORTH_MUX &= ~0x0003;
	            *pPORTH_MUX &= ~0x0003;
	            
	            if(bSecEnable) {
	                *pPORTH_FER |= 0x0080;
	                *pPORTH_FER |= 0x0080;
	            
	                nValue = *pPORTH_MUX;
	                *pPORTH_MUX &= ~0x00c0,
	                *pPORTH_MUX &= ~0x00c0;
	            }
	        }
	        break;
	    }
	}
    return false;	//You should never be here.
    
}

