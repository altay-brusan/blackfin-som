#include "../common/SPI/SPI.h"

#include <services/services.h>

// global data for synchronization of SPIs

unsigned short g_nSPIslaveCount = 7;
unsigned short g_anSPIslaveFlagMasks[7] = {0xfdff, 0xfbff, 0xf7ff, 0xefff, 0xdfff, 0xbfff, 0x7fff};

//bool g_bSPIinitialized = false;

T_SPI_SPEC g_SPIspec = {
		ADI_INT_DMA5_SPI,
		ADI_INT_SPI_ERROR,
		pSPI_FLG,
		pSPI_BAUD,
		pSPI_CTL,
		pSPI_TDBR,
		pSPI_RDBR,
		pSPI_STAT,
		pSPI_SHADOW,
		pSIC_IMASK	
	};
	
bool spi_platformInit(unsigned char pa_cSPI, unsigned long pa_cSlaveNumber) {
	//no specific initialisation needed for cm-bf533
	return true;
}

