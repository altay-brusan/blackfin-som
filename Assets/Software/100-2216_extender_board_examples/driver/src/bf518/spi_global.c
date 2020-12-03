#include "../common/SPI/SPI.h"

#include <cdefbf518.h>
#include <services/services.h>

// global data for synchronization of SPIs

unsigned short g_nSPIslaveCount = 7;
unsigned short g_anSPIslaveFlagMasks[7] = {0xfdff, 0xfbff, 0xf7ff, 0xefff, 0xdfff, 0xbfff, 0x7fff};

//bool g_bSPIinitialized = false;

T_SPI_SPEC g_SPIspec[] = {
    {
		ADI_INT_DMA7_SPI0,
		ADI_INT_SPI0_STATUS,
		pSPI0_FLG,
		pSPI0_BAUD,
		pSPI0_CTL,
		pSPI0_TDBR,
		pSPI0_RDBR,
		pSPI0_STAT,
		pSPI0_SHADOW,
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
    },
    {
		ADI_INT_DMA5_SPORT1_RX_AND_SPI1,
		ADI_INT_SPI1_STATUS,
		pSPI1_FLG,
		pSPI1_BAUD,
		pSPI1_CTL,
		pSPI1_TDBR,
		pSPI1_RDBR,
		pSPI1_STAT,
		pSPI1_SHADOW,
		pSIC_IMASK,
		pDMA5_IRQ_STATUS,
		pDMA5_CONFIG,			
		(volatile void **)DMA5_START_ADDR,
		pDMA5_X_MODIFY,
		pDMA5_X_COUNT,
		pDMA5_CURR_X_COUNT,			
		pDMA5_Y_MODIFY,
		pDMA5_Y_COUNT,
		pDMA5_CURR_Y_COUNT,
		(volatile void **)pDMA5_NEXT_DESC_PTR,		
		6,
		false
#ifdef _USE_VDK_		
		,
		(VDK_SemaphoreID)1
#endif		
    }
    
};
	
bool spi_platformInit(unsigned char pa_cSPI, unsigned long pa_cSlaveNumber) {
	unsigned short nValue;
	//WORKAROUND: read port mux registers onces and write it twices. (See anomaly list of bf537)
	
	switch(pa_cSPI) {
	    
	    case 0: {
	        //enable MOSI, MISO and SCK pins
	        *pPORTG_FER |= 0x7000;
	        *pPORTG_MUX &= 0x3FFF;
	
	        switch (pa_cSlaveNumber) {
		        case 0: {
			        //activate SPI0SS
			        *pPORTG_FER |= 0x0800;
			        *pPORTG_MUX &= 0xCFFF;
			        return true;
		        }		
		        case 1: {
		        	//activate SPI0SSEL1
			        *pPORTF_MUX &= 0xFFCF;
			        *pPORTF_FER |= 0x0080;
			        return true;
		        }
		        case 2: {
			        //activate SPI0SSEL2
			        *pPORTG_MUX &= 0x3FFF;
			        *pPORTG_FER |= 0x8000;
			        return true;
		        }
		        case 3: {
			        //activate SPI0SSEL3
			        *pPORTH_FER |= 0x0010;
			        *pPORTH_MUX &= 0xFFF3;
			        *pPORTH_MUX |= 0x0008;
			        return true;
		        }
		        case 4: {
			        //activate SPI0SSEL4
			        // nothing to do
			        return false;
		        }
		        case 5: {
			        //activate SPI0SSEL5
			        *pPORTG_MUX &= 0xFFF3;
			        *pPORTG_MUX |= 0x0008;
			        *pPORTG_FER |= 0x0008;
			        return true;
		        }
		        case 6: {
			        //activate SPI0SSEL6
			        // nothing to do
			        return false;
		        }
		        case 7: {
			        //activate SPI0SSEL7
			        // nothing to do
			        return false;
		        }
		        default:
			        return false;	//You should never be here.
	        }
	    }
	    
	    case 1: {
	        
	        //enable MOSI, MISO and SCK pins
	        *pPORTH_FER |= 0x000E;
	        *pPORTH_MUX &= 0xFFFC;
	        *pPORTH_MUX |= 0x0001;
	
	
	        switch (pa_cSlaveNumber) {
		        case 0: {
			        //activate SPI0SS
			        *pPORTH_FER |= 0x0001;
			        return true;
		        }		
		        case 1: {
		        	//activate SPI1SSEL1
			        *pPORTH_FER |= 0x0040;
                    *pPORTH_MUX &= 0xFFCF;
                    *pPORTH_MUX |= 0x0020;
			        return true;
		        }
		        case 2: {
			        //activate SPI1SSEL2
			        *pPORTF_FER |= 0x0001;
                    *pPORTF_MUX &= ~0x0003;
                    *pPORTF_MUX |= 0x0002;
			        return true;
		        }
		        case 3: {
			        //activate SPI1SSEL3
			        *pPORTG_FER |= 0x0001;
			        *pPORTG_MUX &= 0xFFFC;
			        *pPORTG_MUX |= 0x0002;
			        return true;
		        }
		        case 4: {
			        //activate SPI1SSEL4
			        *pPORTF_FER |= 0x0100;
                    *pPORTF_MUX &= 0xFF3F;
                    *pPORTF_MUX |= 0x0080;
			        return true;
		        }
		        case 5: {
			        //activate SPI1SSEL5
			        *pPORTG_MUX &= 0xCFFF;
			        *pPORTG_MUX |= 0x2000;
			        *pPORTG_FER |= 0x0800;
			        return true;
		        }
		        case 6: {
			        //activate SPI1SSEL6
			        // nothing to do
			        return false;
		        }
		        case 7: {
			        //activate SPI1SSEL7
			        // nothing to do
			        return false;
		        }
		        default:
    		        return false;	//You should never be here.
		        }
	        }
	}
	
	return false;
}

