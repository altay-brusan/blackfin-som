#include "../common/SPI/SPI.h"

#include <cdefbf548.h>
#include <services/services.h>

// global data for synchronization of SPIs

unsigned short g_nSPIslaveCount = 7;
unsigned short g_anSPIslaveFlagMasks[7] = {0xfdff, 0xfbff, 0xf7ff, 0xefff, 0xdfff, 0xbfff, 0x7fff};

//bool g_bSPIinitialized = false;

T_SPI_SPEC g_SPIspec[] = {
    
    {   // SPI0
        ADI_INT_DMA4_SPI0,
        ADI_INT_SPI0_STATUS,
        pSPI0_FLG,
        pSPI0_BAUD,
        pSPI0_CTL,
        pSPI0_TDBR,
        pSPI0_RDBR,
        pSPI0_STAT,
        pSPI0_SHADOW,
        pIMASK,
    },
    
    {   // SPI1
        ADI_INT_DMA5_SPI1,
        ADI_INT_SPI1_STATUS,
        pSPI1_FLG,
        pSPI1_BAUD,
        pSPI1_CTL,
        pSPI1_TDBR,
        pSPI1_RDBR,
        pSPI1_STAT,
        pSPI1_SHADOW,
        pIMASK,
    },
    
    {   // SPI2
        ADI_INT_DMA23_SPI2,
        ADI_INT_SPI2_STATUS,
        pSPI2_FLG,
        pSPI2_BAUD,
        pSPI2_CTL,
        pSPI2_TDBR,
        pSPI2_RDBR,
        pSPI2_STAT,
        pSPI2_SHADOW,
        pIMASK,
    }
    
};

bool spi_platformInit(unsigned char pa_cSPI, unsigned long pa_cSlaveNumber) {
    unsigned short nValue;
    
    switch(pa_cSPI) {
        case 0: {
            //enable MOSI, MISO and SCK pins
            *pPORTE_FER |= 0x0007;
            *pPORTE_MUX &= 0xFFFFFFC0;
            switch (pa_cSlaveNumber) {
                case 1: {
                    *pPORTE_FER |= 0x0010;
                    ssync();
                    *pPORTE_MUX &= 0xFFFFFCFF;
                    
                }
                break;   

                case 2: {
                    *pPORTE_FER |= 0x0020;
                    ssync();
                    *pPORTE_MUX &= 0xFFFFF3FF;
                    
                }
                break;
                
                case 3: {
                    *pPORTE_FER |= 0x0040;
                    ssync();
                    *pPORTE_MUX &= 0xFFFFCFFF;
                    
                }
                break;            
                
                default:
                    return(false);                                                    
            }
        }
        break;
        
        case 1: {
            //enable MOSI, MISO and SCK pins
            *pPORTG_FER |= 0x0700;
            switch (pa_cSlaveNumber) {
                case 1: {
                    *pPORTG_MUX &= 0xFFFFF3FF;
                    *pPORTG_FER |= 0x0020;
                }
                break;   

                case 2: {
                    *pPORTG_MUX &= 0xFFFFCFFF;
                    *pPORTG_FER |= 0x0040;
                }
                break;
                
                case 3: {
                    *pPORTG_MUX &= 0xFFFF3FFF;
                    *pPORTG_FER |= 0x0080;
                }
                break;                                                
                
                default:
                    return(false);                
            }            
        }
        break;
        
        case 2: {
            //enable MOSI, MISO and SCK pins
            *pPORTB_FER |= 0x7000;
            
            switch (pa_cSlaveNumber) {
                
                case 1: {
                    *pPORTB_MUX &= 0xFFF3FFFF;
                    *pPORTB_FER |= 0x0200;
                }
                break;   

                case 2: {
                    *pPORTB_MUX &= 0xFFCFFFFF;
                    *pPORTB_FER |= 0x0400;
                }
                break;
                
                case 3: {
                    *pPORTB_MUX &= 0xFF3FFFFF;
                    *pPORTB_FER |= 0x0800;
                }
                break;  
                
                default:
                    return(false);                                              
            }            
        }
        break;   
        
        default:
            return(false);
    }
    
    
    
    return true;
}

