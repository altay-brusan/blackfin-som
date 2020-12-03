#include "../common/spi/SPI.h"

#include <cdefbf537.h>
#include <services/services.h>

// global data for synchronization of SPIs

unsigned short g_nSPIslaveCount = 7;
unsigned short g_anSPIslaveFlagMasks[7] = {0xfdff, 0xfbff, 0xf7ff, 0xefff, 0xdfff, 0xbfff, 0x7fff};

T_SPI_SPEC g_SPIspec[] = {
	{
		ADI_INT_DMA7_SPI,
		ADI_INT_PERIPHERAL_ERROR,
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
		6,										// Number of slave selects signals
		false,
#ifdef _USE_VDK_
		(VDK_SemaphoreID)0,
#endif
	}
};

const unsigned char	g_nSPIinterfaces = sizeof(g_SPIspec) / sizeof(T_SPI_SPEC);

bool spi_platformInit(unsigned char pa_cSPI, unsigned long pa_cSlaveNumber) {
	unsigned short nValue;
	//WORKAROUND: read port mux registers onces and write it twices. (See anomaly list of bf537)
	
	//enable MOSI, MISO and SCK pins
	nValue = *pPORTF_FER;
	*pPORTF_FER |= 0x3800;
	*pPORTF_FER |= 0x3800;
	
	
	switch (pa_cSlaveNumber) {
		case 0: {
			//activate SPISS
			nValue = *pPORTF_FER;
			*pPORTF_FER |= 0x4000;
			*pPORTF_FER |= 0x4000;
			return true;
		}		
		case 1: {
			//activate SPISSEL1
			nValue = *pPORTF_FER;
			*pPORTF_FER |= 0x0400;
			*pPORTF_FER |= 0x0400;
			return true;
		}
		case 2: {
			//activate SPISSEL2
			nValue = *pPORT_MUX;
			*pPORT_MUX |= 0x0001;
			*pPORT_MUX |= 0x0001;
			return true;
		}
		case 3: {
			//activate SPISSEL3
			nValue = *pPORT_MUX;
			*pPORT_MUX |= 0x0001;
			*pPORT_MUX |= 0x0001;
			return true;
		}
		case 4: {
			//activate SPISSEL4
			nValue = *pPORTF_FER;
			*pPORTF_FER |= 0x0040;
			*pPORTF_FER |= 0x0040;

			nValue = *pPORT_MUX;
			*pPORT_MUX |= 0x0080;
			*pPORT_MUX |= 0x0080;
			return true;
		}
		case 5: {
			//activate SPISSEL5
			nValue = *pPORTF_FER;
			*pPORTF_FER |= 0x0020;
			*pPORTF_FER |= 0x0020;

			nValue = *pPORT_MUX;
			*pPORT_MUX |= 0x0040;
			*pPORT_MUX |= 0x0040;
			return true;
		}
		case 6: {
			//activate SPISSEL6
			nValue = *pPORTF_FER;
			*pPORTF_FER |= 0x0010;
			*pPORTF_FER |= 0x0010;

			nValue = *pPORT_MUX;
			*pPORT_MUX |= 0x0020;
			*pPORT_MUX |= 0x0020;
			return true;
		}
		case 7: {
			//activate SPISSEL7
			nValue = *pPORT_MUX;
			*pPORT_MUX |= 0x0004;
			*pPORT_MUX |= 0x0004;
			nValue = *pPORT_MUX;
			*pPORT_MUX &= ~0x0002;
			*pPORT_MUX &= ~0x0002;
			return true;
		}

		default:
			return false;	//You should never be here.
	}
}

