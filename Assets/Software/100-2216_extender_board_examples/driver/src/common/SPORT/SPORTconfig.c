#include <string.h>
#include "SPORTconfig.h"
#include "../gpTimerconfig.h"

#define SPORT_FAST_TRANSFER

extern unsigned char g_nSPORTcount;
extern T_SPORT_SPEC g_aSPORTspec[];

#define TX_BUSY_TIMEOUT                 0x1fffff

unsigned long g_testdata[10] = {89, 0x88, 0x88, 0x88, 0x88};

// prototype for the platform init function
//bool sport_platformInit(unsigned char pa_cSport, bool bRxEnable, bool bTxEnable);


// transmit interrupt handler
ADI_INT_HANDLER_RESULT SPORTtxHandler(void *pa_pClientArg) {
    T_SPORT_INST *pInst = (T_SPORT_INST *)pa_pClientArg;
    if (adi_int_SICInterruptAsserted(g_aSPORTspec[pInst->nSport].peripheralTxIntId) == ADI_INT_RESULT_ASSERTED) {
        // clear the busy flag
        pInst->bTxBusy = false;           
        
        if (pInst->fnTxCallback) {
            pInst->fnTxCallback();
        }
 
        // clear the interrupt
        *(g_aSPORTspec[pInst->nSport].pTxDMAirqStat) = 0x1;        

        return ADI_INT_RESULT_PROCESSED;
    } else {
        return ADI_INT_RESULT_NOT_PROCESSED;
    }
}


// receive interrupt handler
ADI_INT_HANDLER_RESULT SPORTrxHandler(void *pa_pClientArg) {
    T_SPORT_INST *pInst = (T_SPORT_INST *)pa_pClientArg;
    if (adi_int_SICInterruptAsserted(g_aSPORTspec[pInst->nSport].peripheralRxIntId) == ADI_INT_RESULT_ASSERTED) {
        // check for a buffer overflow
        if (pInst->pRxBuffer.nElementsInBuffer > (pInst->pRxBuffer.nMaxElements - 2)) {
            // risk of a buffer overflow, signal busy
            gpio_set (pInst->tMyBusyFlag);
        }
        pInst->pRxBuffer.nElementsInBuffer ++;
        // if exist, call user callback
        if (pInst->fnRxCallback) {
            pInst->fnRxCallback();
        }
        // clear the interrupt
        *(g_aSPORTspec[pInst->nSport].pRxDMAirqStat) = 0x1;
        return ADI_INT_RESULT_PROCESSED;
    } else {
        return ADI_INT_RESULT_NOT_PROCESSED;
    }
}



T_SPORT_HANDLE sportGenericCommSetup (unsigned char pa_cSport,      // sport number
                                    unsigned long pa_nCommClkFreq,  // frequency of communication clock in Hz
                                    unsigned char pa_cWordLength,   // in bit
                                    unsigned short pa_nRxBurstSize, // in byte
                                    unsigned short pa_nTxBurstSize, // in byte
                                    unsigned short pa_nRxBufSize,   // in bursts
                                    unsigned short pa_nTxBufSize,   // in bursts
                                    unsigned long pa_nRxTimeout,
                                    T_SPORT_CALLBACK pa_fRxCallback,
                                    T_SPORT_CALLBACK pa_fTxCallback,
                                    unsigned long pa_nSystemClk,    // in Hz
                                    unsigned long pa_nCoreClk,      // in Hz
                                    unsigned short pa_nClockTimer,  // timer used for the communication clock
                                    T_GPIO_MASK pa_tMyBusyFlag,
                                    T_GPIO_MASK pa_tOtherBusyFlag,                                  
                                    T_ERROR_CODE *erResult,         // error return value
                                    void *pa_pGeneric) {            // reserved for future use
                                        
    T_SPORT_HANDLE hSport = 0;
    if (pa_cSport > g_nSPORTcount) {
        *erResult = ERR_SPORT_NOT_FOUND;
        return hSport;
    }
    
    if (g_aSPORTspec[pa_cSport].bTxInUse || g_aSPORTspec[pa_cSport].bTxInUse) {
        *erResult = ERR_SPORT_IN_USE;
        return hSport;
    } else {
    
        // mark sport as in use
        g_aSPORTspec[pa_cSport].bTxInUse = true;
        g_aSPORTspec[pa_cSport].bRxInUse = true;
        
        // allocate memory for the instance
        T_SPORT_INST *pInst = (T_SPORT_INST *)malloc (sizeof (T_SPORT_INST) );
        hSport = (T_SPORT_HANDLE)pInst;
        pInst->nSport = pa_cSport;
        
        // limit wordlength
        if (pa_cWordLength < 4) {
            pa_cWordLength = 4;
        }
        if (pa_cWordLength > 32) {
            pa_cWordLength = 32;
        }
        
        // calculating the processor cycles passed during one word transfer
        // for transmit
        double fWordCycleTime = (double)(pa_cWordLength) * (1.0 / (double)pa_nCommClkFreq);
        pInst->nTxProcCyclesPerTransferCycle = (unsigned long)( fWordCycleTime * (double)pa_nCoreClk );
                            
        // set main config
        if (pa_nTxBufSize) {
            // configure transmit
            sport_platformInit(pa_cSport, true, true);
            
            *(g_aSPORTspec[pa_cSport].pSPORTtcr1) = 0x1600;//0x2600;    // external tclk
            *(g_aSPORTspec[pa_cSport].pSPORTtcr2) = 0x010f; // wordlength 15, secondary enable
            
            //*(g_aSPORTspec[pa_cSport].pSPORTtfsdiv) = (pa_cWordLength - 1);
            // *(g_aSPORTspec[pa_cSport].pSPORTtclkdiv) = (unsigned short)( (pa_nSystemClk - 2 * pa_nTxBitRate) / (2 * pa_nTxBitRate));
                    
            // get memory for the transmit buffer
            pInst->pTxBuffer.pStart = (unsigned char *)malloc (pa_nTxBurstSize);
            if (!(pInst->pTxBuffer.pStart)) {
                *erResult = ERR_SPORT_NOT_ENOUGH_MEMORY;
                // free the handle memory
                free (pInst);
                return 0;
            } else {
                pInst->pTxBuffer.pEnd = pInst->pTxBuffer.pStart + pa_nTxBufSize * pa_nTxBurstSize;
                pInst->pTxBuffer.nBurstSize = pa_nTxBurstSize;
                pInst->pTxBuffer.pIn = pInst->pTxBuffer.pStart;             
                pInst->pTxBuffer.pOut = pInst->pTxBuffer.pStart;
            }
            pInst->pTxBuffer.nElementsInBuffer = 0;
            pInst->pTxBuffer.nMaxElements = pa_nRxBufSize;

            
            // configure transmit dma
            *(g_aSPORTspec[pa_cSport].pTxDMAconfig) = 0x00a4;
            *(g_aSPORTspec[pa_cSport].pTxDMAxcount) = pa_nTxBurstSize / 2;
            *(g_aSPORTspec[pa_cSport].pTxDMAxmodify) = 2;
            *(g_aSPORTspec[pa_cSport].pTxDMAstartaddr) = (void *)(pInst->pTxBuffer.pStart);
            // configure tx interrupt       
          adi_int_SICGetIVG(g_aSPORTspec[pa_cSport].peripheralTxIntId, &(pInst->nTxIVG));
            if(adi_int_CECHook(pInst->nTxIVG, SPORTtxHandler, (void *)pInst, false) == ADI_INT_RESULT_SUCCESS) {
                adi_int_SICWakeup(g_aSPORTspec[pa_cSport].peripheralTxIntId, TRUE);
                adi_int_SICEnable(g_aSPORTspec[pa_cSport].peripheralTxIntId);
            } else {
                *erResult = ERR_SPORT_HOOK_INTERRUPT;
                free(pInst);
                return 0;
            }
            pInst->fnTxCallback = pa_fTxCallback;  
                  
        }
        
        if (pa_nRxBufSize) {
            // configure receive
            sport_platformInit(pa_cSport, false, true);
            
            *(g_aSPORTspec[pa_cSport].pSPORTrcr1) = 0x6400;//0x1400;//0x2400;
            *(g_aSPORTspec[pa_cSport].pSPORTrcr2) = 0x010f; // wordlength 15, secondary enable!
        
            //*(g_aSPORTspec[pa_cSport].pSPORTrfsdiv) = 0;
//          *(g_aSPORTspec[pa_cSport].pSPORTrclkdiv) = (unsigned short)( (pa_nSystemClk - 2 * pa_nRxBitRate) / (2 * pa_nRxBitRate));
    
            // get memory for the receive buffer
            pInst->pRxBuffer.pStart = (unsigned char *)malloc (pa_nRxBufSize * pa_nRxBurstSize);
            if (!(pInst->pRxBuffer.pStart)) {
                *erResult = ERR_SPORT_NOT_ENOUGH_MEMORY;
                free (pInst);
                return 0;
            } else {
                pInst->pRxBuffer.pEnd = pInst->pRxBuffer.pStart + pa_nRxBufSize * pa_nRxBurstSize;
                pInst->pRxBuffer.nBurstSize = pa_nRxBurstSize;
                //pInst->pRxBuffer.pIn = pInst->pRxBuffer.pStart;               
                pInst->pRxBuffer.pOut = pInst->pRxBuffer.pStart;
            }
            pInst->pRxBuffer.nElementsInBuffer = 0;
            pInst->pRxBuffer.nMaxElements = pa_nRxBufSize;

            // configure receive dma (fill the descriptor chain)
            unsigned short nElements = pa_nRxBufSize;
            unsigned char cEntriesPerElements = 2;
            // get memory for the descriptor
            pInst->pDescriptorChain = (unsigned long *)malloc (nElements * cEntriesPerElements * sizeof (unsigned long) );
            // fill the chain
            unsigned short i;
            for (i=0; i<(nElements-1); i++) {
                pInst->pDescriptorChain[i * 2] = (unsigned long)&(pInst->pDescriptorChain[i * 2 + 2]);
                pInst->pDescriptorChain[i * 2 + 1] = (unsigned long)((unsigned long)(pInst->pRxBuffer.pStart) + pa_nRxBurstSize * i);
            }
            pInst->pDescriptorChain[2 * (nElements-1)] = (unsigned long)&(pInst->pDescriptorChain[0]);
            pInst->pDescriptorChain[2 * (nElements-1) + 1] = (unsigned long)((unsigned long)(pInst->pRxBuffer.pStart) + pa_nRxBurstSize * (nElements-1));;
            
            
            //*(g_aSPORTspec[pa_cSport].pRxDMAconfig) = 0x00a6;//0x74a6;/*0x10a6;*/ // large descriptor list, descriptor size 4, 1D, interrupt enabled, 16Bit transfer
            *(g_aSPORTspec[pa_cSport].pRxDMAconfig) = 0x74a6;//0x74a6;/*0x10a6;*/   // large descriptor list, descriptor size 4, 1D, interrupt enabled, 16Bit transfer
            *(g_aSPORTspec[pa_cSport].pRxDMAxcount) = pa_nRxBurstSize / 2; //16
            *(g_aSPORTspec[pa_cSport].pRxDMAxmodify) = 2;
//          *(g_aSPORTspec[pa_cSport].pRxDMAstartaddr) = &(g_testdata[0]);// (void *)(pInst->pRxBuffer.pStart);
            *(g_aSPORTspec[pa_cSport].pRxDMAstartaddr) = (void *)(pInst->pRxBuffer.pStart);
            *(g_aSPORTspec[pa_cSport].pRxNextDescrPtr)= &(pInst->pDescriptorChain[0]);

            
            // configure interrupt
            adi_int_SICGetIVG (g_aSPORTspec[pa_cSport].peripheralRxIntId, &(pInst->nRxIVG));
            if(adi_int_CECHook (pInst->nRxIVG, SPORTrxHandler, (void *)pInst, false) == ADI_INT_RESULT_SUCCESS) {
                adi_int_SICWakeup(g_aSPORTspec[pa_cSport].peripheralRxIntId, TRUE);
                adi_int_SICEnable(g_aSPORTspec[pa_cSport].peripheralRxIntId);
            } else {
                *erResult = ERR_SPORT_HOOK_INTERRUPT;
                free (pInst);
                return 0;
            }
            pInst->fnRxCallback = pa_fRxCallback;
            
        }   
        
        // configure and enable the timer for the communication clock
    /*  float fSCKLPeriod = 1.0 / (float)(pa_nSystemClk);
        float fTFTClockPeriod = 1.0 / (float)(pa_nCommClkFreq);
        unsigned long nTimerPeriod = (unsigned long)(fTFTClockPeriod / fSCKLPeriod);
        unsigned long nTimerWidth = nTimerPeriod / 2;
        T_GP_TIMER_INST *hTimer = timer_gp_setup (pa_nClockTimer, SPORT_COMM_TIMER_CONFIG, nTimerWidth, nTimerPeriod, true, pa_nSystemClk, 0);
        timer_enable (hTimer);
    */  
        // configure the gpios for the busy flags
        pInst->tMyBusyFlag = pa_tMyBusyFlag;
        gpio_becomeOutput (pa_tMyBusyFlag);
        pInst->tOtherBusyFlag = pa_tOtherBusyFlag;
        gpio_becomeInput (pa_tOtherBusyFlag);
        // clear my busy flag
        gpio_clear(pa_tMyBusyFlag);
    
        if (pa_nRxBufSize) {
            // enable rx dma
            *(g_aSPORTspec[pa_cSport].pRxDMAconfig) |= 0x0001;
            // enable rx
            *(g_aSPORTspec[pa_cSport].pSPORTrcr1) |= 0x0001;
        }
        if (pa_nTxBufSize) {
            // enable tx
            *(g_aSPORTspec[pa_cSport].pSPORTtcr1) |= 0x0001;
        }
    }
    
    return hSport;                                                                          
}


T_ERROR_CODE sportSendBurstPacket (T_SPORT_HANDLE pa_cSport, unsigned char *pa_nBuffer) {
    T_ERROR_CODE erResult = ERR_NONE;
    T_SPORT_INST *pInst = (T_SPORT_INST *)pa_cSport;
    
    // check if the device is ready to receive
    if (1/*gpio_readFlag (pInst->tOtherBusyFlag) == 0*/) {
        // the device is ready to receive, so send the packet
    
        // wait for finishing previous transfer
        unsigned long nTimeout = TX_BUSY_TIMEOUT;
    /*  while ( ( (*(g_aSPORTspec[pInst->nSport].pSPORTstat) & 0x0040) == 0) && nTimeout) {
            nTimeout--;
        }*/
        // wait until previous dma transfer has finished
        while ( (( (*(g_aSPORTspec[pInst->nSport].pTxDMAirqStat) & 0x0008) == 0x0008) && nTimeout) || (!pInst->bTxBusy)) {
            nTimeout--;
        }
        
        if (nTimeout) {
            // wait until last transfer cycle has completed
        //  _sportWaitProcCycles (pInst->nTxProcCyclesPerTransferCycle);
            
            // disable tx dma
            *(g_aSPORTspec[pInst->nSport].pTxDMAconfig) &= ~0x0001;

            // fill transmit buffer
            memcpy (pInst->pTxBuffer.pStart, pa_nBuffer, pInst->pTxBuffer.nBurstSize);      
        
            // start tx dma
            *(g_aSPORTspec[pInst->nSport].pTxDMAconfig) |= 0x0001;
    
        } else {
            erResult = ERR_SPORT_BUSY_TIMEOUT;
        }
    } else {
        erResult = ERR_SPORT_RECEIVER_IS_BUSY;
    }
    
    return erResult;                                                                            
}


T_ERROR_CODE sportReceiveBurstPacket (T_SPORT_HANDLE pa_cSport, unsigned char *pa_nBuffer, unsigned short *pa_nPacketLength) {
    T_ERROR_CODE erResult = ERR_NONE;
    T_SPORT_INST *pInst = (T_SPORT_INST *)pa_cSport;
        
    if (pInst->pRxBuffer.nElementsInBuffer > 0) {
        // packet(s) ready to read
        pInst->pRxBuffer.nElementsInBuffer --;
        memcpy(pa_nBuffer, (unsigned char *)(pInst->pRxBuffer.pOut), pInst->pRxBuffer.nBurstSize);
        // increment buffer pointer
        pInst->pRxBuffer.pOut += pInst->pRxBuffer.nBurstSize;
        // control limits
        if (pInst->pRxBuffer.pOut > pInst->pRxBuffer.pEnd) {
            pInst->pRxBuffer.pOut = pInst->pRxBuffer.pStart;
        }
        // one packet was read, so release busy signal
        gpio_clear (pInst->tMyBusyFlag);
    } else {
        erResult = ERR_SPORT_NO_PACKET_READY;
    }
    
    *pa_nPacketLength = pInst->pRxBuffer.nBurstSize;
    
    return erResult;                                                                            
}

bool sportBufferFree (T_SPORT_HANDLE pa_cSport) {
    return true;
}

T_ERROR_CODE sportClose (T_SPORT_HANDLE pa_cSport) {
    T_ERROR_CODE erResult = ERR_NONE;
    T_SPORT_INST *pInst = (T_SPORT_INST *)pa_cSport;
    
    // wait until all packets are processed
    while (!sportBufferFree (pa_cSport));
    
    // disable tx
    *(g_aSPORTspec[pInst->nSport].pSPORTtcr1) &= ~0x0001;
    *(g_aSPORTspec[pInst->nSport].pTxDMAconfig) &= ~0x0001;
    
    // disable rx
    *(g_aSPORTspec[pInst->nSport].pSPORTrcr1) &= ~0x0001;
    *(g_aSPORTspec[pInst->nSport].pRxDMAconfig) &= ~0x0001;
    
    // TODO: unhook the interrupts
    
    // free the buffer memory
    free (pInst->pTxBuffer.pStart);
    free (pInst->pRxBuffer.pStart);
    
    // free the handle memory
    free (pInst);
    
    return erResult;                                                                            
}
