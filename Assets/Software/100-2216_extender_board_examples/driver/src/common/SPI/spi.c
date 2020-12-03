/**
 *  @file       spi.c
 *  @ingroup    SPI
 *  
 *  @brief      Advanced SPI Controller driver.
 *  
 *              Implements multi-plexing of a single SPI controller, including full
 *              SPI controller context-switching. Allows a single controller to be used
 *              for more than a single purpose.
 *      
 *  BLT_DISCLAIMER
 *  
 *  @author     James Walmsley
 *  
 *  @cond svn
 *  
 *  Information of last commit
 *  $Rev::               $:  Revision of last commit
 *  $Author::            $:  Author of last commit
 *  $Date::              $:  Date of last commit
 *  
 *  @endcond
 **/
 
/** @defgroup SPI
 *  @ingroup driverapi
 *  SPI
 */
 
 


#ifdef _USE_VDK_
    #include <VDK.h>
    VDK_SemaphoreID g_SPI_accessSEM;    ///< SPI Access Control
#endif
#include <math.h>
#include "spi.h"

 
T_SPI_DESCRIPTION   g_SPIcontroller;    ///< Designates the maximum number of SPI interfaces on chip.
bool    g_SPIisInitialised = false;

extern T_SPI_SPEC g_SPIspec[];
extern unsigned short g_anSPIslaveFlagMasks[];

static T_SPI_HANDLE g_hSPI =  (T_SPI_HANDLE)NULL;


T_SPI_HANDLE SPIgetGlobalHandle(void) {
    return g_hSPI;  
}


void SPIsetGlobalHandle(T_SPI_HANDLE hSPI) {
    g_hSPI = hSPI;  
}

/*
    Private Function, just initialises the global so we can make some head or tail of it.
*/
void SPIinit(void) {
    
    int i;
    
    for(i = 0; i < MAX_SPI_INTERFACES; i++) {
        
        g_SPIcontroller.m_Interfaces[i].inUse           = false;
        g_SPIcontroller.m_Interfaces[i].m_nInstances    = 0;
    }
    
    g_SPIisInitialised = true;
    
}
 


/**
*   @brief      Setup the specified SPI bus of the processor
*   
*   @param      pa_cSPI     SPI bus
*   @param      pa_nSysClk  System clock in Hz
*   @param      pa_nCoreClk Core clock in Hz
*   @param      pa_bMaSl    True if master, false if slave.
*   @return     SPI_ERR_INVALID_INTERFACE if the specified interface is not valid.
*               SPI_ERR_DEVICE_INUSE if the interface is always in use.
*               0 otherwise.
**/
T_ERROR_CODE SPIsetup(unsigned char pa_cSPI, 
                      unsigned long pa_nSysClk, 
                      unsigned long pa_nCoreClk,
                      bool pa_bMaSl) {
    
    /* Check entrant conditions! If not sane go no further! */
    if(pa_cSPI > (MAX_SPI_INTERFACES - 1)) {
        return SPI_ERR_INVALID_INTERFACE;
    }
    
    
    if(!g_SPIisInitialised) {
        SPIinit();
    }
    
    if(!g_SPIcontroller.m_Interfaces[pa_cSPI].inUse) {      // Straight-forward setup!
        
        g_SPIcontroller.m_Interfaces[pa_cSPI].inUse         = true;
        g_SPIcontroller.m_Interfaces[pa_cSPI].m_bMaSl       = pa_bMaSl;
        
        g_SPIcontroller.m_Interfaces[pa_cSPI].m_nInstances  = 0;
        
        g_SPIcontroller.m_nCoreClkFreq  = pa_nCoreClk;
        g_SPIcontroller.m_nSysClkFreq   = pa_nSysClk;
        
#ifdef _USE_VDK_
        g_SPIcontroller.m_Interfaces[pa_cSPI].m_ControlSem = VDK_CreateSemaphore(1, 1, 1, 0);
#endif
        
        return 0;

    } else {
    
        return SPI_ERR_DEVICE_INUSE;
    
    }
}

void SPIcleanup(unsigned char pa_cSPI) {
    // test inUse flag to know if its appropriate to clean it up!
} 


unsigned int SPI_getPower(unsigned int value) {

    int i;
    unsigned int test;
    test = value;
    for(i = 1; test > 1; i++) {
        test = test / 2;        
    }
    
    return i;
}


T_ERROR_CODE SPIsetReadDefaultWord(T_SPI_HANDLE pa_hSPI, unsigned short pa_usDefaultWord) {
    if(pa_hSPI) {
        T_SPI_INSTANCE *pSPIinst = (T_SPI_INSTANCE *) pa_hSPI;
    
        pSPIinst->msDefaultWord = pa_usDefaultWord;
        return(ERR_NONE);
    }
    else {
        return(SPI_ERR_INVALID_INTERFACE);   
    }
}


/**
 *  @public
 *  @brief  
 **/
T_SPI_HANDLE SPIopen(unsigned char pa_cSPI, T_SPI_CONFIG *pa_pCfg, T_ERROR_CODE *pa_pError) {
    
    if(!g_SPIisInitialised) {
        SPIinit();
    }
    
    // Check if Entrant conditions are SANE, otherwise go no further!
    
    if(pa_cSPI > (MAX_SPI_INTERFACES -1)) {
        *pa_pError = SPI_ERR_INVALID_INTERFACE;
        return (T_SPI_HANDLE) NULL;
    }
    
    if(!g_SPIcontroller.m_Interfaces[pa_cSPI].inUse) {
        *pa_pError = SPI_ERR_DEVICE_INUSE;
        return (T_SPI_HANDLE) NULL; 
    }
                            
    // Check if requested resource is available, and compatible!
    
    if(g_SPIcontroller.m_Interfaces[pa_cSPI].m_bMaSl != pa_pCfg->mbMaSl) {
        *pa_pError = SPI_ERR_DEVICE_NOT_COMPATIBLE;
        return (T_SPI_HANDLE) NULL;
    }
                              
    // Allocate resources etc!
    T_SPI_INSTANCE *pSPIinst = malloc(sizeof(T_SPI_INSTANCE));
    
    if(pSPIinst) {
        pSPIinst->cSPI = pa_cSPI;
        pSPIinst->msDefaultWord = pa_pCfg->msDefaultWord;
    
        if (pa_pCfg->mbMaSl) {      // configure as master
        
            pSPIinst->mCS = pa_pCfg->mtCSEL;
                                                    
            pSPIinst->mConfig.mSPI_BAUD = (unsigned short)ceil((double)(g_SPIcontroller.m_nSysClkFreq) / (2.0 * (double)pa_pCfg->mnBaudrate));
            pSPIinst->mConfig.mSPI_CTL  = 0x1425; // CPHA=1: CS controlled by software, TIMOD=01
            // pSPIinst->mConfig.mSPI_CTL = 0x1025; // CPHA = 0: CS controlled by hardware;
        
            /* Process PARAMS! */
        
            switch(pa_pCfg->mnTransferSize) {   // Transfer Size
                case SPI_8BIT:
                    pSPIinst->mConfig.mSPI_CTL &= (unsigned short) ~SPI_BIT_XFER_SIZE; break;
                case SPI_16BIT:
                    pSPIinst->mConfig.mSPI_CTL |= (unsigned short) SPI_BIT_XFER_SIZE; break;
                default:
                    break;
            }

            switch(pa_pCfg->mnClockPolarity) {  // Clock Polarity
                case SPI_CPOL_LOW:
                    pSPIinst->mConfig.mSPI_CTL &= (unsigned short) ~SPI_BIT_CLK_POL; break;
                case SPI_CPOL_HIGH:
                    pSPIinst->mConfig.mSPI_CTL |= (unsigned short) SPI_BIT_CLK_POL; break;
                default:
                    break;
            }
        
            switch(pa_pCfg->mnClockPhase) { // Clock Phase
                case SPI_CPHA_HIGH:
                    pSPIinst->mConfig.mSPI_CTL |= (unsigned short) SPI_BIT_CLK_PHASE; break;
                case SPI_CPHA_LOW:
                    pSPIinst->mConfig.mSPI_CTL &= (unsigned short) ~SPI_BIT_CLK_PHASE; break;
                default:
                    break;
            }
        
            switch(pa_pCfg->mnMsbLsbFirst) {    // Byte Direction
                case SPI_MSB_FIRST:
                    pSPIinst->mConfig.mSPI_CTL &= (unsigned short) ~SPI_BIT_BYTE_MODE; break;
                case SPI_LSB_FIRST:
                    pSPIinst->mConfig.mSPI_CTL |= (unsigned short) SPI_BIT_BYTE_MODE; break;
                default:
                    break;
            }
        
            spi_platformInit(pa_cSPI, pa_pCfg->mtCSEL); // Enables the SPI sel Flag
    
            if (pa_pCfg->mtCSEL & 0x80000000) {     // Enable SPISELECTS in hardware, and store flag context register.
                // a gpio is used as slave select
                gpio_becomeOutput(pa_pCfg->mtCSEL & 0x7fffffff);        // set gpio as output
                gpio_set(pa_pCfg->mtCSEL & 0x7fffffff);                 // set gpio high
            } else {
                // SPISSEL signal is used as slave select
        
                *(g_SPIspec[(pSPIinst->cSPI)].pSPIflg) |= (unsigned short)(1 << (pa_pCfg->mtCSEL & 0x000000FF));    // Enables on the SPI controller.
    
                asm ("ssync;");
            }
    
            *pa_pError = ERR_NONE;
            return (T_SPI_HANDLE) pSPIinst;
            
        
            //pSPIinst->mConfig.mSPI_CTL    |= pa_nTransferSize | pa_nClockPolarity | pa_nMsbLsbFirst | pa_nMasterErrorFlag;                                                                                                
        
        } else {
        
            // Slave configuration isn't possible for a multiplexed driver.
            // configure as slave                               
            // configure the mmrs.      
            spi_platformInit(pa_cSPI, 0); // Enables the SPI SS
            
            // TODO: implement the slave mode
            
            return (T_SPI_HANDLE) pSPIinst;
        }
    
    }
    else {
        *pa_pError = SPI_ERR_NOT_ENOUGH_MEMORY;    
        return(NULL);
    }
}



/* NONE OF THIS MUST OCCUR UNLESS THE HANDLE REALLY HAS CONTROL!! */

T_ERROR_CODE SPIswitchContext(T_SPI_HANDLE pa_hSPI) {
    T_SPI_INSTANCE *pSPIinst = (T_SPI_INSTANCE *) pa_hSPI;
    
    
    if(pSPIinst->bIsInControl) {
        
        // Ensure the SPI is disabled.
        *(g_SPIspec[(pSPIinst->cSPI)].pSPIctl) &= ~0x4000;

        if (g_SPIcontroller.m_Interfaces[pSPIinst->cSPI].m_bMaSl) {                                         
            // Get SPI Controller back into Context!
            *(g_SPIspec[(pSPIinst->cSPI)].pSPIbaud) = pSPIinst->mConfig.mSPI_BAUD;
            *(g_SPIspec[(pSPIinst->cSPI)].pSPIctl)  = pSPIinst->mConfig.mSPI_CTL;
        
        } else {

                // Context switching is not possible for Slave mode device.
        
        }

        // enable the spi
        *(g_SPIspec[(pSPIinst->cSPI)].pSPIctl) |= 0x4000;   
    
        ssync ();
    
        return ERR_NONE;
        
    }
    
    return SPI_ERR_ILLEGAL_CONTEXT_SWITCH;
}






 
T_ERROR_CODE SPIcontrol (T_SPI_HANDLE   pa_hSPI) {
                            
        T_SPI_INSTANCE *pSPIinst = (T_SPI_INSTANCE *) pa_hSPI;
                            
                            
        /*  Re-configure the SPI instance*/ 
            return 0;
}
                        
 
 
T_ERROR_CODE SPIclose(T_SPI_HANDLE pa_hSPI) {
    T_SPI_INSTANCE *pSPIinst = (T_SPI_INSTANCE *) pa_hSPI;
    
    *(g_SPIspec[(pSPIinst->cSPI)].pSPIflg) &= ~(unsigned short)((pSPIinst->mCS & 0x000000FF) << 1);
    g_SPIcontroller.m_Interfaces[pSPIinst->cSPI].m_nInstances --;
    
    free(pSPIinst);
    // Cleanup all resources allocated to that handle
    return 0;
}
 


/**
*   @brief      Read the given amount of bytes from SPI interface without changing the CS-signal.
*   @attention  The CS-signal must be driven accordigly by the functions SPIselect and SPIdeselect
*   
*   @param      pa_hSPI Handle to the SPI interface
*   @param      pa_pnData Pointer to a buffer The function will perform a cast to char if the interface
*               is configured with 8bit transfer.
*   @param      pa_nLength Length in bytes of the data to be readed
*   @param      pa_pError Poiter to the return error value
*   @return     SPI_ERR_NOT_IN_CONTROL if the SPI interface is not in control
*               0 otherwise.
*   @pre        SPIselect
*   @post       SPIdeselect
**/
T_ERROR_CODE SPIread(T_SPI_HANDLE pa_hSPI, 
                     unsigned short *pa_pnData, 
                     unsigned long pa_nLength, 
                     T_ERROR_CODE *pa_pError) {
    
    int i;
    T_SPI_INSTANCE *pSPIinst = (T_SPI_INSTANCE *) pa_hSPI;
    
        
    if(pSPIinst->bIsInControl) {
    
        // Read out on the SPI
        if(pSPIinst->mConfig.mSPI_CTL & SPI_BIT_XFER_SIZE) { // 16-bit Transfer mode!
    
            for(i = 0; i < (pa_nLength >> 1); i++) {
        
                //simple io mode just write a dummy value and read in the received data
                while ((*(g_SPIspec[(pSPIinst->cSPI)].pSPIstat) & 0x0008) == 0x0008) {
            #ifdef _USE_VDK_
                    VDK_Yield();
            #endif
                };  //wait until transmit buffer register is empty
                *(g_SPIspec[(pSPIinst->cSPI)].pSPItdbr) = 0xffff; //write dummy data to transmit register
                while ((*(g_SPIspec[(pSPIinst->cSPI)].pSPIstat) & 0x0020) == 0x0000) {
            #ifdef _USE_VDK_
                    VDK_Yield();
            #endif
                };  //wait until receive buffer register is full
        
                pa_pnData[i] = *(g_SPIspec[(pSPIinst->cSPI)].pSPIrdbr);                     //read data from receive register       
        
            }
    
        } else {                // 8-bit transfer mode!
        
            unsigned char *pnData = (unsigned char *) pa_pnData;
            for(i = 0; i < pa_nLength; i++) {
        
                //simple io mode just write a dummy value and read in the received data
                while ((*(g_SPIspec[(pSPIinst->cSPI)].pSPIstat) & 0x0008) == 0x0008) {
            #ifdef _USE_VDK_
                    VDK_Yield();
            #endif
                };  //wait until transmit buffer register is empty
                *(g_SPIspec[(pSPIinst->cSPI)].pSPItdbr) = 0xffff;                       //write dummy data to transmit register
                while ((*(g_SPIspec[(pSPIinst->cSPI)].pSPIstat) & 0x0020) == 0x0000) {
            #ifdef _USE_VDK_
                    VDK_Yield();
            #endif
                };  //wait until receive buffer register is full
        
                pnData[i] = *(g_SPIspec[(pSPIinst->cSPI)].pSPIrdbr);                        //read data from receive register       
        
            }
    
        }
     
        return 0;   
    }
    

    return SPI_ERR_NOT_IN_CONTROL;
}
 


/**
*   @brief      Read the given amount of bytes from SPI interface without changing the CS-signal. You can also specify the data send to MOSI when reading.
*   @attention  The CS-signal must be driven accordigly by the functions SPIselect and SPIdeselect
*   
*   @param      pa_hSPI Handle to the SPI interface
*   @param      pa_pnInputData Pointer to a buffer The function will perform a cast to char if the interface
*                              is configured with 8bit transfer.
*   @param      pa_pnOutputData Array of data that is send on MOSI when reading is performed.
*   @param      pa_nLength Length in bytes of the data to be readed
*   @param      pa_pError Poiter to the return error value
*   @return     SPI_ERR_NOT_IN_CONTROL if the SPI interface is not in control
*               0 otherwise.
*   @pre        SPIselect
*   @post       SPIdeselect
**/
T_ERROR_CODE SPIreadwrite(T_SPI_HANDLE pa_hSPI, 
                          unsigned short *pa_pnInputData, 
                          unsigned short *pa_pnOutputData, 
                          unsigned long pa_nLength, 
                          T_ERROR_CODE *pa_pError) {
    
    int i;
    T_SPI_INSTANCE *pSPIinst = (T_SPI_INSTANCE *) pa_hSPI;
    
        
    if(pSPIinst->bIsInControl) {
    
        // Read out on the SPI
        if(pSPIinst->mConfig.mSPI_CTL & SPI_BIT_XFER_SIZE) { // 16-bit Transfer mode!
    
            for(i = 0; i < (pa_nLength >> 1); i++) {
        
                //simple io mode just write a dummy value and read in the received data
                while ((*(g_SPIspec[(pSPIinst->cSPI)].pSPIstat) & 0x0008) == 0x0008) {
            #ifdef _USE_VDK_
                    VDK_Yield();
            #endif
                };  //wait until transmit buffer register is empty
                *(g_SPIspec[(pSPIinst->cSPI)].pSPItdbr) = pa_pnOutputData[i];
                while ((*(g_SPIspec[(pSPIinst->cSPI)].pSPIstat) & 0x0020) == 0x0000) {
            #ifdef _USE_VDK_
                    VDK_Yield();
            #endif
                };  //wait until receive buffer register is full
        
                pa_pnInputData[i] = *(g_SPIspec[(pSPIinst->cSPI)].pSPIrdbr);                        //read data from receive register       
        
            }
    
        } else {                // 8-bit transfer mode!
        
            unsigned char *pnData = (unsigned char *) pa_pnInputData;
            for(i = 0; i < pa_nLength; i++) {
        
                //simple io mode just write a dummy value and read in the received data
                while ((*(g_SPIspec[(pSPIinst->cSPI)].pSPIstat) & 0x0008) == 0x0008) {
            #ifdef _USE_VDK_
                    VDK_Yield();
            #endif
                };  //wait until transmit buffer register is empty
                *(g_SPIspec[(pSPIinst->cSPI)].pSPItdbr) = pa_pnOutputData[i];
                while ((*(g_SPIspec[(pSPIinst->cSPI)].pSPIstat) & 0x0020) == 0x0000) {
            #ifdef _USE_VDK_
                    VDK_Yield();
            #endif
                };  //wait until receive buffer register is full
        
                pnData[i] = *(g_SPIspec[(pSPIinst->cSPI)].pSPIrdbr);                        //read data from receive register       
        
            }
    
        }
     
        return 0;   
    }
    

    return SPI_ERR_NOT_IN_CONTROL;
}



/**
*   @brief      Writes the given data to the SPI interface without changing the CS-signal
*   @attention  The CS-signal must be driven accordigly by the functions SPIselect and SPIdeselect
*   
*   @param      pa_hSPI Handle to the SPI interface
*   @param      pa_pnData Pointer to the data to be written. The function will perform a cast to char if the interface
*               is configured with 8bit transfer.
*   @param      pa_nLength Length in bytes of the data to be written
*   @param      pa_pError Poiter to the return error value
*   @return     The readed value from SPI of the last SPI transfer.
*   @pre        SPIselect
*   @post       SPIdeselect
**/
unsigned short SPIwrite(T_SPI_HANDLE pa_hSPI, 
                        unsigned short *pa_pnData, 
                        unsigned long pa_nLength, 
                        T_ERROR_CODE *pa_pError) {
    
    T_SPI_INSTANCE *pSPIinst = (T_SPI_INSTANCE *) pa_hSPI;
    int i;  
    unsigned short nDummyValue;
    
    if(pSPIinst->bIsInControl) {
        
        // Write out on the SPI
    
        if(pSPIinst->mConfig.mSPI_CTL & SPI_BIT_XFER_SIZE) { // 16-bit Transfer mode!
    
            for(i = 0; i < (pa_nLength >> 1); i++) {
                //simple io mode, just write data and discard received data.
                while ((*(g_SPIspec[(pSPIinst->cSPI)].pSPIstat) & 0x0008) == 0x0008) {  // This has potential for deadlock!! (SHOULD BE CHECKED!!)
            #ifdef _USE_VDK_
                    VDK_Yield();
            #endif
                };  //wait until transmit buffer register is empty
                *(g_SPIspec[(pSPIinst->cSPI)].pSPItdbr) = pa_pnData[i];                 //write data to transmit register
                while ((*(g_SPIspec[(pSPIinst->cSPI)].pSPIstat) & 0x0020) == 0x0000) {
            #ifdef _USE_VDK_
                    VDK_Yield();
            #endif
                };  //wait until receive buffer register is full
                nDummyValue = *(g_SPIspec[(pSPIinst->cSPI)].pSPIrdbr);  //read dummy data from receive register
        
            }
        
        } else {    // 8-bit Transfer mode!
            unsigned char *pnData = (unsigned char *) pa_pnData;
            for(i = 0; i < pa_nLength; i++) {
                //simple io mode, just write data and discard received data.
                while ((*(g_SPIspec[(pSPIinst->cSPI)].pSPIstat) & 0x0008) == 0x0008) {  // This has potential for deadlock!! (SHOULD BE CHECKED!!)
            #ifdef _USE_VDK_
                    VDK_Yield();
            #endif
                };  //wait until transmit buffer register is empty
                *(g_SPIspec[(pSPIinst->cSPI)].pSPItdbr) = pnData[i];                    //write data to transmit register
                while ((*(g_SPIspec[(pSPIinst->cSPI)].pSPIstat) & 0x0020) == 0x0000) {
            #ifdef _USE_VDK_
                    VDK_Yield();
            #endif
                };  //wait until receive buffer register is full
                nDummyValue = *(g_SPIspec[(pSPIinst->cSPI)].pSPIrdbr);  //read dummy data from receive register
        
            }       
        }
        
        return nDummyValue; 
    }
    
    if(pa_pError) {
        *pa_pError = SPI_ERR_NOT_IN_CONTROL;
    }
    return 0;   
}
 
 

/*
    Clocks bytes of FF on the SPI line with no devices selected.
*/
T_ERROR_CODE SPIclock(T_SPI_HANDLE pa_hSPI, unsigned long pa_nBytes) {
    
    unsigned short dummyData = 0xFFFF;
    int i;
    T_SPI_INSTANCE *pSPIinst = (T_SPI_INSTANCE *) pa_hSPI;
    
    // Take Control of Device
    SPItakeControl(pa_hSPI);
    // Switch the Context
    SPIswitchContext(pa_hSPI);  // so clock rate is good for device to by reset via clocking
    
    if(pSPIinst->mConfig.mSPI_CTL & SPI_BIT_XFER_SIZE) {    // 16-bit mode
        for(i = 0; i < (pa_nBytes / 2); i++) {
            SPIwrite(pa_hSPI, &dummyData, 2, NULL); 
        }
    } else {
        for(i = 0; i < (pa_nBytes); i++) {
            SPIwrite(pa_hSPI, &dummyData, 1, NULL); 
        }
    }
    
    SPIreleaseControl(pa_hSPI);
    
    return ERR_NONE;
}
 


/**
*   @brief      Select device with CS-signal.
*   
*   @param      pa_hSPI Handle to the SPI interface
*   @return     0
**/
T_ERROR_CODE SPIselect(T_SPI_HANDLE pa_hSPI) {
    T_SPI_INSTANCE *pSPIinst = (T_SPI_INSTANCE *) pa_hSPI;
    
    // Take Control of Device
    SPItakeControl(pa_hSPI);
    // Switch the Context
    SPIswitchContext(pa_hSPI);
    
    if (pSPIinst->mCS & 0x80000000) {
        // gpio is used as slave select signal
        // *(unsigned short *)0x203ffffa &= ~0x0004;
        gpio_clear(pSPIinst->mCS & 0x7fffffff);
    } else {
        // SPISSEL is used as slave select signal
        // Enable it
        //*(g_SPIspec[(pSPIinst->cSPI)].pSPIflg) |= (pSPIinst->mCS & 0x0000FFFF);
        //unsigned short nMask = 0xFFFF;
        //nMask &= (unsigned short) ~(pSPIinst->mCS << 8);
        //*(g_SPIspec[(pSPIinst->cSPI)].pSPIflg) &= nMask;
        unsigned short nMask = 1 << ((pSPIinst->mCS & 0x000000FF) + 8);
        *(g_SPIspec[(pSPIinst->cSPI)].pSPIflg) &= ~nMask;
        
    }
    
    return 0;
}
 


/**
*   @brief      Deselect device with CS-signal.
*   
*   @param      pa_hSPI Handle to the SPI interface
*   @return     0
**/ 
T_ERROR_CODE SPIdeselect(T_SPI_HANDLE pa_hSPI) {
    T_SPI_INSTANCE *pSPIinst = (T_SPI_INSTANCE *) pa_hSPI;
    
    if (pSPIinst->mCS & 0x80000000) {
        // *(unsigned short *)0x203ffffa |= 0x0004;
        gpio_set(pSPIinst->mCS & 0x7fffffff);       
    } else {
        // Disable it!
        //*(g_SPIspec[(pSPIinst->cSPI)].pSPIflg) &= ~(pSPIinst->mCS & 0x0000FFFF);
        //unsigned short nMask = 0x00000000;
        //nMask |= (unsigned short) (pSPIinst->mCS << 8);
        //*(g_SPIspec[(pSPIinst->cSPI)].pSPIflg) |= nMask;
        unsigned short nMask = 1 << ((pSPIinst->mCS & 0x000000FF) + 8);
        *(g_SPIspec[(pSPIinst->cSPI)].pSPIflg) |= nMask;
    }
    
    // Release Control
    SPIreleaseControl(pa_hSPI);
    
    return 0;
}
 
T_ERROR_CODE SPItakeControl(T_SPI_HANDLE pa_hSPI) {
    // Grabs access to the SPI via Semaphore

        T_SPI_INSTANCE *pSPIinst = (T_SPI_INSTANCE *) pa_hSPI;
    
#ifdef _USE_VDK_    
    if(VDK_PendSemaphore(g_SPIcontroller.m_Interfaces[pSPIinst->cSPI].m_ControlSem, 0) == true) {
        pSPIinst->bIsInControl = true;
        return 0;   
    } else {
        return -1;  
    }
#else
    pSPIinst->bIsInControl = true;
    return 0;
#endif
}
 
T_ERROR_CODE SPIreleaseControl(T_SPI_HANDLE pa_hSPI) {
    // Releases access to the SPI via Semaphore
    T_SPI_INSTANCE *pSPIinst = (T_SPI_INSTANCE *) pa_hSPI;
    pSPIinst->bIsInControl = false;
    
#ifdef _USE_VDK_    
    VDK_PostSemaphore(g_SPIcontroller.m_Interfaces[pSPIinst->cSPI].m_ControlSem);
    return 0;
#else
    pSPIinst->bIsInControl = false;
    return 0;
#endif  
}

T_ERROR_CODE SPIsetCsLevel(T_SPI_HANDLE pa_hSPI, bool pa_bHigh) {
    T_SPI_INSTANCE *pSPIinst = (T_SPI_INSTANCE *) pa_hSPI;
    
    if(pSPIinst->bIsInControl) {
        
        if (pa_bHigh) {
            if (pSPIinst->mCS & 0x80000000) {
                // gpio is used as CS
                gpio_set(pSPIinst->mCS & 0x7fffffff);
            } else {
                // dedicated flag is used                   
                unsigned short nMask = 1 << ((pSPIinst->mCS & 0x000000FF) + 8);
                *(g_SPIspec[(pSPIinst->cSPI)].pSPIflg) |= nMask;
            }
        } else {
            if (pSPIinst->mCS & 0x80000000) {
                // gpio is used as CS
                gpio_clear(pSPIinst->mCS & 0x7fffffff);
                
            } else {
                // dedicated flag is used
                unsigned short nMask = 1 << ((pSPIinst->mCS & 0x000000FF) + 8);
                *(g_SPIspec[(pSPIinst->cSPI)].pSPIflg) &= ~nMask;               
            }
        }
    
        return ERR_NONE;
    } else {
        return SPI_ERR_NOT_IN_CONTROL;
    }

}
