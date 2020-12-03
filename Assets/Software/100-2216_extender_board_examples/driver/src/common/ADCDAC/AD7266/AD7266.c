/**
	@file AD7266.c
	@ingroup adcdac
	@brief driver for the AD7266, a serial analog to digital converter
	
	
	
	BLT_DISCLAIMER(TBD)
	@author Alexander Froemel
	@version 1.0
	@date 28.06.2009
	
	@startcond Changelog
	
	@endcond
**/

#include <cplbtab.h>
#include "AD7266.h"

extern T_SPORT_SPEC g_aSPORTspec[];
// global stuff for managment purposes
static T_AD7266_SPEC g_aAD7266Spec;

//extern ADI_INT_HANDLER_RESULT AD7266rxHandler(void *pa_pClientArg);

//unsigned long long g_curAD7266[5];

#ifdef __ADSPBF561__
section("L1_data_a") unsigned long g_anRxDMAdescriptor[2][2];
#else
section("L1_data") unsigned long g_anRxDMAdescriptor[2][2];
#endif


/**
*       @brief  SPORT Receive Interrupt Handler, runs the user callback function
**/
#pragma section("L1_code")
ADI_INT_HANDLER_RESULT AD7266rxHandler(void *pa_pClientArg) {
    
    unsigned short *pBufferAccessable;
    
    

//    _GET_CYCLE_COUNT(g_curAD7266[0]);
	if (adi_int_SICInterruptAsserted(g_aSPORTspec[g_aAD7266Spec.nSPORT].peripheralRxIntId) == ADI_INT_RESULT_ASSERTED) {
	    
	    // clear the interrupt
	    *(g_aSPORTspec[g_aAD7266Spec.nSPORT].pRxDMAirqStat) = 0x1;
	    
	    if( (__cplb_ctrl & CPLB_ENABLE_DCACHE ) || (__cplb_ctrl & CPLB_ENABLE_DCACHE2)){
            dcache_invalidate_both(); 
        }
	  
	    if(g_aAD7266Spec.nBufferMode == AD7266_PINGPONG_BUFFER) {
            if(g_aAD7266Spec.nCurrBuffer == 0) {
	            g_aAD7266Spec.nCurrBuffer = 1;
    	        pBufferAccessable = g_aAD7266Spec.pBufferBase1;
        	} else {
	            g_aAD7266Spec.nCurrBuffer = 0;
	            pBufferAccessable = g_aAD7266Spec.pBufferBase0;
	        }
        }else if( g_aAD7266Spec.nBufferMode == AD7266_SINGLE_BUFFER) {
            AD7266Stop();
            pBufferAccessable = g_aAD7266Spec.pBufferBase0;
        }
	  
	    if (g_aAD7266Spec.fnCallback) {
	  	    g_aAD7266Spec.fnCallback(pBufferAccessable);
	    }
	    
//	    _GET_CYCLE_COUNT(g_curAD7266[1]);
//    	g_curAD7266[3] = g_curAD7266[1] - g_curAD7266[0];
//    	g_curAD7266[4] = g_curAD7266[0] - g_curAD7266[2];
//    	g_curAD7266[2] = g_curAD7266[0];
    	
	    return ADI_INT_RESULT_PROCESSED;
	  
    } else {
		
		return ADI_INT_RESULT_NOT_PROCESSED;
	}
	
}


/**
*       @brief  DMA error Interrupt Handler
**/
static ADI_INT_HANDLER_RESULT AD7266DMAerrorHandler(void *pa_pClientArg) {
	if (*(g_aSPORTspec[g_aAD7266Spec.nSPORT].pRxDMAirqStat) & 0x2) {
		// clear DMA error flag
		*(g_aSPORTspec[g_aAD7266Spec.nSPORT].pRxDMAirqStat) = 0x2;
		return ADI_INT_RESULT_PROCESSED;
	}
	return ADI_INT_RESULT_NOT_PROCESSED;
}


/**
*       @brief  SPORT error Interrupt Handler
**/
static ADI_INT_HANDLER_RESULT AD7266SPORTerrorHandler(void *pa_pClientArg) {
    unsigned short nSPORTstat = *(g_aSPORTspec[g_aAD7266Spec.nSPORT].pSPORTstat) & 0x000f;
    
	if (nSPORTstat) {
		// clear SPORT error flag
		*(g_aSPORTspec[g_aAD7266Spec.nSPORT].pSPORTstat) = nSPORTstat;
		return ADI_INT_RESULT_PROCESSED;
	}
	return ADI_INT_RESULT_NOT_PROCESSED;
}


/**
*       @brief Configuration of the     AD7266 Serial Analog to Digital Converter
*       @param pa_nChannel              select the Channel to be sampled
*       @param pa_nSamplingRate         the selected sampling rate of the AD7266
*       @param pa_nMaxElements            number of samples in a buffer
*       @param pa_nStride               stride in the buffer, the space left (in samples) between two samples
*       @param pa_nOffset               offset in the buffer, the space left (in samples) before the first sample is written in the buffer
*       @param pa_fnCallback            unser callback function
*       @param pa_adr0                  address pin of AD7266 multiplexer
*       @param pa_adr1                  address pin of AD7266 multiplexer
*       @param pa_adr2                  address pin of AD7266 multiplexer
*       @param pa_nSclk                 System Clock
*       @param **pa_ppBufferBase0       address of the first buffer
*       @param **pa_ppBufferBase1       address of the second buffer
*       @param **pa_fRealSamplingRate   the real sampling rate of the AD7266
*       @param pa_nBufferMode           selects if one or two buffers are used
*       @param pa_nSPORTperIVG          change SPORT IVG, if 0 use standard
*       @param pa_nDMAerrIVG            change DMA error IVG, if 0 use standard
*       @param pa_nSPORTerrIVG          change SPORT error IVG, if 0 use standard
*       @param pa_nFlags                flags for AD7266 control, see AD7266.h for more details
*       @param pa_nSPORT                change the used SPORT, if 0xffff use standard
*       @param *__p_reserved            reserved parameter
*       @return erResult                returns an error code
**/
T_ERROR_CODE AD7266Setup(T_AD7266_CONFIG *pa_aAD7266Config) {
    
    T_ERROR_CODE erResult = ERR_NONE;
    unsigned short nClockDiv;
    
    g_aAD7266Spec.nMaxElements = pa_aAD7266Config->nMaxElements;
	g_aAD7266Spec.nCurrElement = 0;
	g_aAD7266Spec.fnCallback = pa_aAD7266Config->fnCallback;
	g_aAD7266Spec.nBufferMode = pa_aAD7266Config->nBufferMode;
	g_aAD7266Spec.nStride = pa_aAD7266Config->nStride;
	g_aAD7266Spec.nOffset = pa_aAD7266Config->nOffset;
	if(pa_aAD7266Config->nSPORT == AD7266_USE_DEFAULT_SPORT) {
	    g_aAD7266Spec.nSPORT = 0;
	} else {
	    g_aAD7266Spec.nSPORT = pa_aAD7266Config->nSPORT;
	}
	
	// check if SPORT is already used
	if(g_aSPORTspec[g_aAD7266Spec.nSPORT].bRxInUse || g_aAD7266Spec.bInUse) {
	    erResult = ERR_AD7266_CHANNEL_IN_USE;
		return erResult;
	}
	
	g_aSPORTspec[g_aAD7266Spec.nSPORT].bRxInUse = true;
	g_aAD7266Spec.bInUse = true;
	
    // init I/O ports for sport use
    sport_platformInit(g_aAD7266Spec.nSPORT, false, true);
    
    if(pa_aAD7266Config->nBufferMode == AD7266_SINGLE_BUFFER) {
	    pa_aAD7266Config->pBufferBase0 = (unsigned short *)calloc((2 * pa_aAD7266Config->nMaxElements * pa_aAD7266Config->nStride) + (pa_aAD7266Config->nOffset * 2), sizeof(unsigned short));
	    g_aAD7266Spec.nCurrBuffer = 0;
	    if(!pa_aAD7266Config->pBufferBase0) {
	        erResult = ERR_AD7266_NOT_ENOUGH_MEMORY;
	        free(pa_aAD7266Config->pBufferBase0);
	        return erResult;
	    }
	    
	} else if(pa_aAD7266Config->nBufferMode == AD7266_PINGPONG_BUFFER) {
	    pa_aAD7266Config->pBufferBase0 = (unsigned short *)calloc((2 * pa_aAD7266Config->nMaxElements * pa_aAD7266Config->nStride) + (pa_aAD7266Config->nOffset * 2), sizeof(unsigned short));
        pa_aAD7266Config->pBufferBase1 = (unsigned short *)calloc((2 * pa_aAD7266Config->nMaxElements * pa_aAD7266Config->nStride) + (pa_aAD7266Config->nOffset * 2), sizeof(unsigned short));
	    g_aAD7266Spec.nCurrBuffer = 1;
        
        if(!pa_aAD7266Config->pBufferBase0 || !pa_aAD7266Config->pBufferBase1) {
            erResult = ERR_AD7266_NOT_ENOUGH_MEMORY;
            free(pa_aAD7266Config->pBufferBase0);
            free(pa_aAD7266Config->pBufferBase1);
            return erResult;
        }
        
	}else {
	    erResult = ERR_AD7266_BUFFER_MODE;
	    return erResult;
	}
	
	g_aAD7266Spec.pBufferBase0 = pa_aAD7266Config->pBufferBase0;
	g_aAD7266Spec.pBufferBase1 = pa_aAD7266Config->pBufferBase1;

	// set AD7266 to correct channel
	gpio_becomeOutput(pa_aAD7266Config->adr0);
	gpio_becomeOutput(pa_aAD7266Config->adr1);
	gpio_becomeOutput(pa_aAD7266Config->adr2);
	
	if(!pa_aAD7266Config->nChannel) {
	    erResult = ERR_AD7266_CHANNEL;
	    AD7266Cleanup();
	    return erResult;
	}
	
	pa_aAD7266Config->nChannel--;   // user selects AD7266 from 1 to 6

	if(pa_aAD7266Config->nChannel & 0x01)	{
	  gpio_set(pa_aAD7266Config->adr0); 
	}	else {
	  gpio_clear(pa_aAD7266Config->adr0);
	}
	if(pa_aAD7266Config->nChannel & 0x02)	{
	  gpio_set(pa_aAD7266Config->adr1); 
	}	else {
	  gpio_clear(pa_aAD7266Config->adr1);
	}
	if(pa_aAD7266Config->nChannel & 0x04)	{
	  gpio_set(pa_aAD7266Config->adr2); 
	}	else {
	  gpio_clear(pa_aAD7266Config->adr2);
	}
	
	// configuration of descriptors
	g_anRxDMAdescriptor[0][0] = (unsigned long)&g_anRxDMAdescriptor[1][0];
	g_anRxDMAdescriptor[1][0] = (unsigned long)&g_anRxDMAdescriptor[0][0];
	g_anRxDMAdescriptor[0][1] = (unsigned long)(pa_aAD7266Config->pBufferBase0 + pa_aAD7266Config->nOffset);
	g_anRxDMAdescriptor[1][1] = (unsigned long)(pa_aAD7266Config->pBufferBase1 + pa_aAD7266Config->nOffset);
	
	//configure interrupts
	if(!pa_aAD7266Config->nSPORTperIVG) {
	    adi_int_SICGetIVG (g_aSPORTspec[g_aAD7266Spec.nSPORT].peripheralRxIntId, &(g_aAD7266Spec.nRxIVG));
	} else {
	    g_aAD7266Spec.nRxIVG = pa_aAD7266Config->nSPORTperIVG;
	    adi_int_SICSetIVG (g_aSPORTspec[g_aAD7266Spec.nSPORT].peripheralRxIntId, g_aAD7266Spec.nRxIVG);
	}
	
	if(adi_int_CECHook (g_aAD7266Spec.nRxIVG, AD7266rxHandler, (void *)0, false) == ADI_INT_RESULT_SUCCESS) {
		adi_int_SICWakeup(g_aSPORTspec[g_aAD7266Spec.nSPORT].peripheralRxIntId, TRUE);
		adi_int_SICEnable(g_aSPORTspec[g_aAD7266Spec.nSPORT].peripheralRxIntId);
		
		// hook DMA error interrupt
		if(!pa_aAD7266Config->nDMAerrIVG) {
		    adi_int_SICGetIVG (g_aSPORTspec[g_aAD7266Spec.nSPORT].nDMAerrIntId, &(g_aAD7266Spec.nDMAerrIVG));
		} else {
	        g_aAD7266Spec.nDMAerrIVG = pa_aAD7266Config->nSPORTperIVG;
	        adi_int_SICSetIVG (g_aSPORTspec[g_aAD7266Spec.nSPORT].nDMAerrIntId, g_aAD7266Spec.nDMAerrIVG);
		}
		
		if(adi_int_CECHook (g_aAD7266Spec.nDMAerrIVG, AD7266DMAerrorHandler, (void *)0, false) == ADI_INT_RESULT_SUCCESS) {
			adi_int_SICWakeup(g_aSPORTspec[g_aAD7266Spec.nSPORT].nDMAerrIntId, TRUE);
			adi_int_SICEnable(g_aSPORTspec[g_aAD7266Spec.nSPORT].nDMAerrIntId);
		}
		
		// hook SPORT error interrupt
		if(!pa_aAD7266Config->nSPORTerrIVG) {
		    adi_int_SICGetIVG (g_aSPORTspec[g_aAD7266Spec.nSPORT].nPeriheralErrIntId, &pa_aAD7266Config->nSPORTerrIVG);
		}else {
	        g_aAD7266Spec.nSPORTerrIVG = pa_aAD7266Config->nSPORTperIVG;
	        adi_int_SICSetIVG (g_aSPORTspec[g_aAD7266Spec.nSPORT].nPeriheralErrIntId, g_aAD7266Spec.nSPORTerrIVG);
		}
		
		if(adi_int_CECHook (g_aAD7266Spec.nSPORTerrIVG, AD7266SPORTerrorHandler, (void *)0, false) == ADI_INT_RESULT_SUCCESS) {
			adi_int_SICWakeup(g_aSPORTspec[g_aAD7266Spec.nSPORT].nDMAerrIntId, TRUE);
			adi_int_SICEnable(g_aSPORTspec[g_aAD7266Spec.nSPORT].nDMAerrIntId);
		}
	
		
	} else {
		erResult = ERR_AD7266_HOOK_INTERRUPT;
		AD7266Cleanup();
		return erResult;
	}
	
	// SPORT config
	
	nClockDiv = AD7266GenerateClkDiv(pa_aAD7266Config->nSamplingRate, pa_aAD7266Config->nSclk);

	*(g_aSPORTspec[g_aAD7266Spec.nSPORT].pSPORTrclkdiv) = nClockDiv;
	unsigned long nRealRclkFreq = (unsigned long)((float)pa_aAD7266Config->nSclk / (2.0 * ((float)(nClockDiv + 1.0))));
	
	*(g_aSPORTspec[g_aAD7266Spec.nSPORT].pSPORTrfsdiv) = AD7266GenerateRFSDIV(pa_aAD7266Config->nSamplingRate, nRealRclkFreq);
	
	*(g_aSPORTspec[g_aAD7266Spec.nSPORT].pSPORTrcr2) = 0x010d;     // secondary side enabled, 14 bit data word
	*(g_aSPORTspec[g_aAD7266Spec.nSPORT].pSPORTrcr1) = 0x7602;     // 
		
	// calculate the real sampling frequency
	
	pa_aAD7266Config->fRealSamplingRate = 1.0 / ((float)(*(g_aSPORTspec[g_aAD7266Spec.nSPORT].pSPORTrfsdiv) + 1.0) / (float)nRealRclkFreq);
	
	// configure DMA
	if(g_aAD7266Spec.nBufferMode == AD7266_SINGLE_BUFFER) {
	    
	    *(g_aSPORTspec[g_aAD7266Spec.nSPORT].pRxDMAstartaddr) = (void *)(g_anRxDMAdescriptor[0][1]);
	    *(g_aSPORTspec[g_aAD7266Spec.nSPORT].pRxDMAxcount) = 2;
    	*(g_aSPORTspec[g_aAD7266Spec.nSPORT].pRxDMAxmodify) = 2 * pa_aAD7266Config->nMaxElements * pa_aAD7266Config->nStride;
    	*(g_aSPORTspec[g_aAD7266Spec.nSPORT].pRxDMAycount) = pa_aAD7266Config->nMaxElements; // outer Loop
    	*(g_aSPORTspec[g_aAD7266Spec.nSPORT].pRxDMAymodify) = (signed short)( -2 * (signed short)(pa_aAD7266Config->nMaxElements * pa_aAD7266Config->nStride) + 2 * pa_aAD7266Config->nStride);
	    
	    if(g_aAD7266Spec.fnCallback) {
    	    *(g_aSPORTspec[g_aAD7266Spec.nSPORT].pRxDMAconfig) = 0x10a6; //0x10b6;	// autobuffer, 2D, interrupt enabled
    	}else {
    	    *(g_aSPORTspec[g_aAD7266Spec.nSPORT].pRxDMAconfig) = 0x1036;	// autobuffer, 2D, interrupt disabled
    	}
	    
	}else if(g_aAD7266Spec.nBufferMode == AD7266_PINGPONG_BUFFER) {
	    
    	*(g_aSPORTspec[g_aAD7266Spec.nSPORT].pRxDMAstartaddr) = (void *)(g_anRxDMAdescriptor[0][1]);
	    *(g_aSPORTspec[g_aAD7266Spec.nSPORT].pRxDMAxcount) = 2;
    	*(g_aSPORTspec[g_aAD7266Spec.nSPORT].pRxDMAxmodify) = 2 * pa_aAD7266Config->nMaxElements * pa_aAD7266Config->nStride;
    	*(g_aSPORTspec[g_aAD7266Spec.nSPORT].pRxDMAycount) = pa_aAD7266Config->nMaxElements; // outer Loop
    	*(g_aSPORTspec[g_aAD7266Spec.nSPORT].pRxDMAymodify) = (signed short)( -2 * (signed short)(pa_aAD7266Config->nMaxElements * pa_aAD7266Config->nStride) + 2 * pa_aAD7266Config->nStride);
	    *(g_aSPORTspec[g_aAD7266Spec.nSPORT].pRxNextDescrPtr) = (void *)&g_anRxDMAdescriptor[0][0];
	    
	    if(g_aAD7266Spec.fnCallback) {
    	    *(g_aSPORTspec[g_aAD7266Spec.nSPORT].pRxDMAconfig) = 0x74b6;	// descriptor mode large, 2D, interrupt enabled
    	}else {
    	    *(g_aSPORTspec[g_aAD7266Spec.nSPORT].pRxDMAconfig) = 0x7436;	// descriptor mode large, 2D, interrupt disabled
    	}

	} else {
	    erResult = ERR_AD7266_BUFFER_MODE;
	    AD7266Cleanup();
	}
	
  return erResult;
}


/**     
*       @brief Starts AD7266 sampling
**/
void AD7266Start(void) {
  	// Start DMA and SPORT
	*(g_aSPORTspec[g_aAD7266Spec.nSPORT].pRxDMAconfig) |= 1;
	*(g_aSPORTspec[g_aAD7266Spec.nSPORT].pSPORTrcr1)   |= 1;
	g_aAD7266Spec.bIsActive = true;
}


/**     
*       @brief Stops AD7266 sampling
**/
void AD7266Stop(void) {
  	// stop DMA and SPORT
	*(g_aSPORTspec[g_aAD7266Spec.nSPORT].pSPORTrcr1)   &= ~0x1;
	*(g_aSPORTspec[g_aAD7266Spec.nSPORT].pRxDMAconfig) &= ~0x1;
	g_aAD7266Spec.bIsActive = false;
}


/**     
*       @brief Closes AD7266, if it should be used again, run AD7266Setup() first
**/
void AD7266Cleanup(void) {
    free ((void *)(g_aAD7266Spec.pBufferBase0));
    free ((void *)(g_aAD7266Spec.pBufferBase1));
    g_aSPORTspec[g_aAD7266Spec.nSPORT].bRxInUse = false;
    g_aAD7266Spec.bInUse = true;
}


/**     
*       @brief Generates the Clock Divider for AD7266Setup
*       @param pa_nSamplingRate     the Sampling Rate fpr wich the Clock divider should be calculated
*       @param pa_nSclk             System Clock
*       @return nClockDiv           Clock Divider
**/
unsigned short AD7266GenerateClkDiv(    unsigned long pa_nSamplingRate,
                                        unsigned long pa_nSclk) {
    
    unsigned long nTransferClock = pa_nSamplingRate * 16;
	if(nTransferClock > AD7266_MAX_TRANSFER_CLOCK) {
	    nTransferClock = AD7266_MAX_TRANSFER_CLOCK;
	}
	unsigned short nClockDiv = (unsigned short)((float)pa_nSclk / (2.0 * (float)(nTransferClock)) - 1.0);
	
	// limit clock divider
	if (nClockDiv < 1) {
	    nClockDiv = 1;
	}
	
	nClockDiv = ((nClockDiv + 1) / 2) * 2 - 1;
	
	return nClockDiv;
}


unsigned short AD7266GenerateRFSDIV(    unsigned long pa_nSamplingRate,
                                        unsigned long pa_nRealRclkFreq) {
    unsigned short nRFSdiv;
    
    
	nRFSdiv = (pa_nRealRclkFreq / pa_nSamplingRate) - 1;
	
	if(nRFSdiv < AD7266_MIN_RFS_DIV) {
	    nRFSdiv = AD7266_MIN_RFS_DIV;
	}
	
	return nRFSdiv;
}

