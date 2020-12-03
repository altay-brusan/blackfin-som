#include "../common/SPORT/SPORT.h"

#include <cdefbf548.h>
#include <services/services.h>


T_SPORT_SPEC g_aSPORTspec[] = {
		{
			false,
			false,
	//		(ADI_DMA_CHANNEL_ID)0,
			ADI_INT_DMA0_SPORT0_RX,
			ADI_INT_DMA1_SPORT0_TX,
			ADI_INT_DMAC0_ERROR,
			ADI_INT_SPORT0_ERROR,
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
			pDMA0_IRQ_STATUS,
			pDMA0_CONFIG,
			(volatile void **)DMA0_START_ADDR,
			pDMA0_X_MODIFY,
			pDMA0_X_COUNT,
			pDMA0_CURR_X_COUNT,			
			pDMA0_Y_MODIFY,
			pDMA0_Y_COUNT,
			pDMA0_CURR_Y_COUNT,			
			(volatile void **)DMA0_NEXT_DESC_PTR,			
			pDMA1_IRQ_STATUS,
			pDMA1_CONFIG,
			(volatile void **)DMA1_START_ADDR,
			pDMA1_X_MODIFY,
			pDMA1_X_COUNT,
			pDMA1_CURR_X_COUNT,			
			pDMA1_Y_MODIFY,
			pDMA1_Y_COUNT,
			pDMA1_CURR_Y_COUNT,
			(volatile void **)DMA1_NEXT_DESC_PTR,			
		},
		{
			false,
			false,
	//		(ADI_DMA_CHANNEL_ID)0,
			ADI_INT_DMA2_SPORT1_RX,
			ADI_INT_DMA3_SPORT1_TX,
			ADI_INT_DMAC0_ERROR,
			ADI_INT_SPORT1_ERROR,
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
			pDMA2_IRQ_STATUS,
			pDMA2_CONFIG,
			(volatile void **)DMA2_START_ADDR,
			pDMA2_X_MODIFY,
			pDMA2_X_COUNT,
			pDMA2_CURR_X_COUNT,			
			pDMA2_Y_MODIFY,
			pDMA2_Y_COUNT,
			pDMA2_CURR_Y_COUNT,						
			(volatile void **)DMA2_NEXT_DESC_PTR,						
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
		},
		{
			false,
			false,
	//		(ADI_DMA_CHANNEL_ID)0,
			ADI_INT_DMA18_SPORT2_RX,
			ADI_INT_DMA19_SPORT2_TX,
			ADI_INT_DMAC1_ERROR,
			ADI_INT_SPORT2_ERROR,
			pSPORT2_TCR1,
			pSPORT2_TCR2,
			pSPORT2_RCR1,
			pSPORT2_RCR2,
			pSPORT2_TX,
			pSPORT2_RX,
			pSPORT2_STAT,
			pSPORT2_TCLKDIV,
			pSPORT2_RCLKDIV,
			pSPORT2_TFSDIV,
			pSPORT2_RFSDIV,
			pSPORT2_MCMC1,
			pSPORT2_MCMC2,
			pSPORT2_MRCS0,
			pSPORT2_MRCS1,
			pSPORT2_MRCS2,
			pSPORT2_MRCS3,
			pSPORT2_MTCS0,
			pSPORT2_MTCS1,
			pSPORT2_MTCS2,
			pSPORT2_MTCS3,
			pDMA18_IRQ_STATUS,
			pDMA18_CONFIG,
			(volatile void **)DMA18_START_ADDR,
			pDMA18_X_MODIFY,
			pDMA18_X_COUNT,
			pDMA18_CURR_X_COUNT,			
			pDMA18_Y_MODIFY,
			pDMA18_Y_COUNT,
			pDMA18_CURR_Y_COUNT,			
			(volatile void **)DMA18_NEXT_DESC_PTR,			
			pDMA19_IRQ_STATUS,
			pDMA19_CONFIG,
			(volatile void **)DMA19_START_ADDR,
			pDMA19_X_MODIFY,
			pDMA19_X_COUNT,
			pDMA19_CURR_X_COUNT,			
			pDMA19_Y_MODIFY,
			pDMA19_Y_COUNT,
			pDMA19_CURR_Y_COUNT,
			(volatile void **)DMA19_NEXT_DESC_PTR,			
		},
		{
			false,
			false,
	//		(ADI_DMA_CHANNEL_ID)0,
			ADI_INT_DMA20_SPORT3_RX,
			ADI_INT_DMA21_SPORT3_TX,
			ADI_INT_DMAC1_ERROR,
			ADI_INT_SPORT3_ERROR,
			pSPORT3_TCR1,
			pSPORT3_TCR2,
			pSPORT3_RCR1,
			pSPORT3_RCR2,
			pSPORT3_TX,
			pSPORT3_RX,
			pSPORT3_STAT,
			pSPORT3_TCLKDIV,
			pSPORT3_RCLKDIV,
			pSPORT3_TFSDIV,
			pSPORT3_RFSDIV,
			pSPORT3_MCMC1,
			pSPORT3_MCMC2,
			pSPORT3_MRCS0,
			pSPORT3_MRCS1,
			pSPORT3_MRCS2,
			pSPORT3_MRCS3,
			pSPORT3_MTCS0,
			pSPORT3_MTCS1,
			pSPORT3_MTCS2,
			pSPORT3_MTCS3,
			pDMA20_IRQ_STATUS,
			pDMA20_CONFIG,
			(volatile void **)DMA20_START_ADDR,
			pDMA20_X_MODIFY,
			pDMA20_X_COUNT,
			pDMA20_CURR_X_COUNT,			
			pDMA20_Y_MODIFY,
			pDMA20_Y_COUNT,
			pDMA20_CURR_Y_COUNT,			
			(volatile void **)DMA20_NEXT_DESC_PTR,			
			pDMA21_IRQ_STATUS,
			pDMA21_CONFIG,
			(volatile void **)DMA21_START_ADDR,
			pDMA21_X_MODIFY,
			pDMA21_X_COUNT,
			pDMA21_CURR_X_COUNT,			
			pDMA21_Y_MODIFY,
			pDMA21_Y_COUNT,
			pDMA21_CURR_Y_COUNT,
			(volatile void **)DMA21_NEXT_DESC_PTR,			
		}
	};
	
unsigned char g_nSPORTcount = sizeof(g_aSPORTspec) / sizeof(T_SPORT_SPEC);		//calculate the nr of elements in the array

bool sport_platformInit(unsigned char pa_cSport, bool bTxNotRx, bool bSecEnable) {
	unsigned short nValue;
	//WORKAROUND: read port mux registers onces and write it twices. (See anomaly list of bf537)
	
	
	switch (pa_cSport) {
		case 0: {
			//activate SPORT0 signals
			
			if(bTxNotRx) {						// enable Tx
				nValue = *pPORTC_FER;
				*pPORTC_FER |= 0x000d;
				*pPORTC_FER |= 0x000d;
			
				nValue = *pPORTC_MUX;
				*pPORTC_MUX &= ~0x000000f3;
				*pPORTC_MUX &= ~0x000000f3;
				
				if(bSecEnable) {
					nValue = *pPORTC_FER;
					*pPORTC_FER |= 0x0002;
					*pPORTC_FER |= 0x0002;
			
					nValue = *pPORTC_MUX;
					*pPORTC_MUX &= ~0x0000000c;
					*pPORTC_MUX &= ~0x0000000c;
				}
			}else {								// enable Rx
				nValue = *pPORTC_FER;
				*pPORTC_FER |= 0x00d0;
				*pPORTC_FER |= 0x00d0;
			
				nValue = *pPORTC_MUX;
				*pPORTC_MUX &= ~0x0000f300;
				*pPORTC_MUX &= ~0x0000f300;
				
				if(bSecEnable) {
					nValue = *pPORTC_FER;
					*pPORTC_FER |= 0x0020;
					*pPORTC_FER |= 0x0020;
			
					nValue = *pPORTC_MUX;
					*pPORTC_MUX &= ~0x00000c00;
					*pPORTC_MUX &= ~0x00000c00;
				}
			}
			return true;
		}
		case 1: {
			//activate SPORT1 signals
			if(bTxNotRx) {						// enable Tx
				nValue = *pPORTD_FER;
				*pPORTD_FER |= 0x000d;
				*pPORTD_FER |= 0x000d;
			
				nValue = *pPORTD_MUX;
				*pPORTD_MUX |= 0x000000a2;
				*pPORTD_MUX |= 0x000000a2;
				
				if(bSecEnable) {
					nValue = *pPORTD_FER;
					*pPORTD_FER |= 0x0002;
					*pPORTC_FER |= 0x0002;
			
					nValue = *pPORTD_MUX;
					*pPORTD_MUX |= 0x00000008;
					*pPORTD_MUX |= 0x00000008;
				}
			}else {								// enable Rx
				nValue = *pPORTD_FER;
				*pPORTD_FER |= 0x00d0;
				*pPORTD_FER |= 0x00d0;
			
				nValue = *pPORTD_MUX;
				*pPORTD_MUX |= 0x0000a200;
				*pPORTD_MUX |= 0x0000a200;
				
				if(bSecEnable) {
					nValue = *pPORTD_FER;
					*pPORTD_FER |= 0x0020;
					*pPORTD_FER |= 0x0020;
			
					nValue = *pPORTD_MUX;
					*pPORTD_MUX |= 0x00000800;
					*pPORTD_MUX |= 0x00000800;
				}
			}
			return true;
		}
		case 2: {
			//activate SPORT2 signals
			if(bTxNotRx) {						// enable Tx
				nValue = *pPORTA_FER;
				*pPORTA_FER |= 0x000d;
				*pPORTA_FER |= 0x000d;
			
				nValue = *pPORTA_MUX;
				*pPORTA_MUX &= ~0x000000f3;
				*pPORTA_MUX &= ~0x000000f3;
				
				if(bSecEnable) {
					nValue = *pPORTA_FER;
					*pPORTA_FER |= 0x0002;
					*pPORTA_FER |= 0x0002;
			
					nValue = *pPORTA_MUX;
					*pPORTA_MUX &= ~0x0000000c;
					*pPORTA_MUX &= ~0x0000000c;
				}
			}else {								// enable Rx
				nValue = *pPORTA_FER;
				*pPORTA_FER |= 0x00d0;
				*pPORTA_FER |= 0x00d0;
			
				nValue = *pPORTA_MUX;
				*pPORTA_MUX &= ~0x0000f300;
				*pPORTA_MUX &= ~0x0000f300;
				
				if(bSecEnable) {
					nValue = *pPORTA_FER;
					*pPORTA_FER |= 0x0020;
					*pPORTA_FER |= 0x0020;
			
					nValue = *pPORTA_MUX;
					*pPORTA_MUX &= ~0x00000c00;
					*pPORTA_MUX &= ~0x00000c00;
				}
			}
			return true;
		}
		case 3: {
			//activate SPORT3 signals
			if(bTxNotRx) {						// enable Tx
				nValue = *pPORTA_FER;
				*pPORTA_FER |= 0x0d00;
				*pPORTA_FER |= 0x0d00;
			
				nValue = *pPORTA_MUX;
				*pPORTA_MUX &= ~0x00f30000;
				*pPORTA_MUX &= ~0x00f30000;
				
				if(bSecEnable) {
					nValue = *pPORTA_FER;
					*pPORTA_FER |= 0x0200;
					*pPORTA_FER |= 0x0200;
			
					nValue = *pPORTA_MUX;
					*pPORTA_MUX &= ~0x000c0000;
					*pPORTA_MUX &= ~0x000c0000;
				}
			}else {								// enable Rx
				nValue = *pPORTA_FER;
				*pPORTA_FER |= 0xd000;
				*pPORTA_FER |= 0xd000;
			
				nValue = *pPORTA_MUX;
				*pPORTA_MUX &= ~0xf3000000;
				*pPORTA_MUX &= ~0xf3000000;
				
				if(bSecEnable) {
					nValue = *pPORTA_FER;
					*pPORTA_FER |= 0x2000;
					*pPORTA_FER |= 0x2000;
			
					nValue = *pPORTA_MUX;
					*pPORTA_MUX &= ~0x0c000000;
					*pPORTA_MUX &= ~0x0c000000;
				}
			}
			return true;
		}

		default:
			return false;	//You should never be here.
	}
}

