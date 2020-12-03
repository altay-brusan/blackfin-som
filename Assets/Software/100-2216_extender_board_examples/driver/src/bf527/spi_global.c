#include "../common/SPI/SPI.h"

#include <cdefbf527.h>
#include <services/services.h>

// global data for synchronization of SPIs

unsigned short g_nSPIslaveCount = 7;
unsigned short g_anSPIslaveFlagMasks[7] = {0xfdff, 0xfbff, 0xf7ff, 0xefff, 0xdfff, 0xbfff, 0x7fff};

//bool g_bSPIinitialized = false;

T_SPI_SPEC g_SPIspec = {
		ADI_INT_DMA7_SPI,
		ADI_INT_SPI_STATUS,
		pSPI_FLG,
		pSPI_BAUD,
		pSPI_CTL,
		pSPI_TDBR,
		pSPI_RDBR,
		pSPI_STAT,
		pSPI_SHADOW,
		pSIC_IMASK,
		pDMA7_IRQ_STATUS,
		pDMA7_CONFIG,			
		(volatile void **)DMA7_START_ADDR,
		pDMA7_X_MODIFY,
		pDMA7_X_COUNT,
		pDMA7_CURR_X_COUNT,			
		pDMA7_Y_MODIFY,
		pDMA7_Y_COUNT,
		pDMA7_CURR_Y_COUNT,
		(volatile void **)pDMA7_NEXT_DESC_PTR,		
		6,
		false
#ifdef _USE_VDK_		
		,
		(VDK_SemaphoreID)0
#endif		
};
	
bool spi_platformInit(unsigned char pa_cSPI, unsigned long pa_nSlaveNumber) {
	unsigned short nValue;
	//WORKAROUND: read port mux registers onces and write it twices. (See anomaly list of bf537)
	
	//enable MOSI, MISO and SCK pins
	*pPORTG_MUX &= ~0x0003;
	*pPORTG_FER |= 0x001c;
	
	
	if (pa_nSlaveNumber <= 7) {
		switch (pa_nSlaveNumber) {
			case 0: {
				//activate SPISS
				*pPORTG_FER |= 0x0002;
				return true;
			}		
			case 1: {
				//activate SPISSEL1
				*pPORTG_MUX |= 0x0002;
				*pPORTG_FER |= 0x0002;
				return true;
			}
			case 2: {
				//activate SPISSEL2
				*pPORTF_FER |= 0x1000;
				*pPORTF_MUX &= ~0x0300;
				*pPORTF_MUX |= 0x0200;
				return true;
			}
			case 3: {
				//activate SPISSEL3
				*pPORTF_FER |= 0x2000;
				*pPORTF_MUX &= ~0x0300;
				*pPORTF_MUX |= 0x0200;
				return true;
			}
			case 4: {
				//activate SPISSEL4
				*pPORTH_MUX &= ~0x000c;
				*pPORTH_FER |= 0x0100;
				return true;
			}
			case 5: {
				//activate SPISSEL5
				*pPORTH_MUX &= ~0x0030;
				*pPORTH_FER |= 0x0200;
				return true;
			}
			case 6: {
				//activate SPISSEL6
				*pPORTF_FER |= 0x0200;
				*pPORTF_MUX &= ~0x000c;
				*pPORTF_MUX |= 0x0008;
				return true;
			}
			case 7: {
				//activate SPISSEL7
				*pPORTF_FER |= 0x0400;
				*pPORTF_MUX &= ~0x0030;
				*pPORTF_MUX |= 0x0020;
				return true;
			}

			default:
				return false;	//You should never be here.
		}
	} else {
		return true;
	}
}

