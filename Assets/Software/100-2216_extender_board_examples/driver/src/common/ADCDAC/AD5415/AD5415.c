/**
	@file AD5415.c
	@ingroup adcdac
	@brief driver for the AD5415, a serial digital to analog converter
	
	
	
	BLT_DISCLAIMER(TBD)
	@author Alexander Froemel
	@version 1.0
	@date 28.06.2009
	
	@startcond Changelog
	
	@endcond
**/
#include <cplbtab.h>
#include "AD5415.h"
#include "../../../../../driver/src/common/SPORT/sportconfig.h"

extern T_SPORT_SPEC g_aSPORTspec[];

//unsigned long long g_curAD5415[5];

#ifdef __ADSPBF561__
section("L1_data_a") unsigned long g_anTxDMAdescriptor[2][2];
#else
section("L1_data") unsigned long g_anTxDMAdescriptor[2][2];
#endif

static T_AD5415_SPEC g_aAD5415Spec;


/**     
*   @brief  SPORT Transfere Interrupt Handler, runs the user callback function
*   @param  *pa_pClientArg          void * Argument
*   @param  ADI_INT_HANDLER_RESULT  if interrupt was processed or not
**/
#pragma section("L1_code")
ADI_INT_HANDLER_RESULT AD5415txHandler(void *pa_pClientArg) {
    
    unsigned short *pBufferAccessable;
    
//    _GET_CYCLE_COUNT(g_curAD5415[0]);

	if (adi_int_SICInterruptAsserted(g_aSPORTspec[g_aAD5415Spec.nSPORT].peripheralTxIntId) == ADI_INT_RESULT_ASSERTED) {
	   	// clear the interrupt
		*(g_aSPORTspec[g_aAD5415Spec.nSPORT].pTxDMAirqStat) = 0x1;
		
		if(g_aAD5415Spec.nBufferMode == AD5415_PINGPONG_BUFFER) {
    	    if(g_aAD5415Spec.nCurrBuffer == 0) {
	            g_aAD5415Spec.nCurrBuffer = 1;
	            pBufferAccessable = (unsigned short *)g_aAD5415Spec.pBufferBase1;
	            
	            if( (__cplb_ctrl & CPLB_ENABLE_DCACHE ) || (__cplb_ctrl & CPLB_ENABLE_DCACHE2)){
	                flush_data_buffer(  (void *)g_aAD5415Spec.pBufferBase0,
	                                    (void *)((unsigned short *)g_aAD5415Spec.pBufferBase0 + g_aAD5415Spec.nMaxElements),
	                                    0);
	            }
    	    } else {
	            g_aAD5415Spec.nCurrBuffer = 0;
	            pBufferAccessable = (unsigned short *)g_aAD5415Spec.pBufferBase0;
	            
	            if( (__cplb_ctrl & CPLB_ENABLE_DCACHE ) || (__cplb_ctrl & CPLB_ENABLE_DCACHE2)){
	                flush_data_buffer(  (void *)g_aAD5415Spec.pBufferBase1,
	                                    (void *)((unsigned short *)g_aAD5415Spec.pBufferBase1 + g_aAD5415Spec.nMaxElements),
	                                    0);
	            }
	        }
	    }else if(g_aAD5415Spec.nBufferMode == AD5415_SINGLE_BUFFER) {
	        AD5415Stop();
	        pBufferAccessable = (unsigned short *)g_aAD5415Spec.pBufferBase0;
	    }
	    
	    // call user callback
	    if (g_aAD5415Spec.fCallback) {
	        g_aAD5415Spec.fCallback(pBufferAccessable);
	    }
	    
//	    _GET_CYCLE_COUNT(g_curAD5415[1]);
//    	g_curAD5415[3] = g_curAD5415[1] - g_curAD5415[0];
//    	g_curAD5415[4] = g_curAD5415[0] - g_curAD5415[2];
//    	g_curAD5415[2] = g_curAD5415[0];
	
		return ADI_INT_RESULT_PROCESSED;
	  
	} else {
		return ADI_INT_RESULT_NOT_PROCESSED;
	}
}


/**     
*   @brief  DMA Error Interrupt Handler
*   @param  *pa_pClientArg          void * Argument
*   @param  ADI_INT_HANDLER_RESULT  if interrupt was processed or not
**/
ADI_INT_HANDLER_RESULT AD5415DMAerrorHandler(void *pa_pClientArg) {
	if (*(g_aSPORTspec[g_aAD5415Spec.nSPORT].pTxDMAirqStat) & 0x2) {
		// clear DMA error flag
		*(g_aSPORTspec[g_aAD5415Spec.nSPORT].pTxDMAirqStat) = 0x2;
		return ADI_INT_RESULT_PROCESSED;
	}
	return ADI_INT_RESULT_NOT_PROCESSED;
}


/**     
*   @brief  SPORT Error Interrupt Handler
*   @param  *pa_pClientArg          void * Argument
*   @param  ADI_INT_HANDLER_RESULT  if interrupt was processed or not
**/
ADI_INT_HANDLER_RESULT AD5415SPORTerrorHandler(void *pa_pClientArg) {
    unsigned short nSPORTstat = *(g_aSPORTspec[g_aAD5415Spec.nSPORT].pSPORTstat) & 0x000f;
    
	if (nSPORTstat) {
		// clear SPORT error flag
		*(g_aSPORTspec[g_aAD5415Spec.nSPORT].pSPORTstat) = nSPORTstat;
		return ADI_INT_RESULT_PROCESSED;
	}
	return ADI_INT_RESULT_NOT_PROCESSED;
}


/**     
*       @brief  Setup Function for AD5415 serial digital to analog converter
*       @param  pa_nChannelMode                 select if one or two output channel should be used
*       @param  pa_nSamplingRate                the selected Sampling Rate of AD5415
*       @param  pa_nMaxElements                   Size of the Buffer
*       @param  pa_nStride                      Stride to use in the Buffer
*       @param  pa_nOffset                      Offset to use at begin of the Buffer
*       @param  pa_nSclk                        System Clock
*       @param  pa_nBufferMode                  select the number of Buffer to use
*       @param  **pa_ppBufferBase0              address of the first Buffer
*       @param  **pa_ppBufferBase1              address of the second Buffer
*       @param  pa_fnCallback                   user callback function
*       @param  *pa_pfRealAD5415SamplingRate    the real Sampling Rate of AD5415
*       @param  pa_nSPORTperIVG                 change the SPORT IVG, if 0 use standard
*       @param  pa_nDMAerrIVG                   change DMA error IVG, if 0 use standard
*       @param  pa_nSPORTerrIVG                 change SPORT error IVG, if 0 use standard
*       @param  *__p_reserved                   reserved parameter
*       @return erResult                        returns an error code
**/
T_ERROR_CODE AD5415Setup(T_AD5415_CONFIG *pa_aAD5415Config) {  
	
	T_ERROR_CODE erResult = ERR_NONE;
	T_ERROR_CODE erPrepBuf;
	
	g_aAD5415Spec.nBurstSize = pa_aAD5415Config->nMaxElements;
	g_aAD5415Spec.nOffset = pa_aAD5415Config->nOffset;
	g_aAD5415Spec.nStride = pa_aAD5415Config->nStride;
	g_aAD5415Spec.nChannelMode = pa_aAD5415Config->nChannelMode;
	
	if(pa_aAD5415Config->nSPORT == AD5415_USE_DEFAULT_SPORT) {
	    g_aAD5415Spec.nSPORT = 0;
	} else {
	    g_aAD5415Spec.nSPORT = pa_aAD5415Config->nSPORT;
	}
	
	// check if SPORT is already used
	if(g_aSPORTspec[g_aAD5415Spec.nSPORT].bTxInUse || g_aAD5415Spec.bInUse) {
	    erResult = ERR_AD5415_CHANNEL_IN_USE;
		return erResult;
	}
	
	g_aSPORTspec[g_aAD5415Spec.nSPORT].bTxInUse = true;
	g_aAD5415Spec.bInUse = true;
	
	if(pa_aAD5415Config->nBufferMode == AD5415_SINGLE_BUFFER) {
        pa_aAD5415Config->pBufferBase0 = (unsigned short *)calloc((pa_aAD5415Config->nChannelMode * pa_aAD5415Config->nMaxElements * pa_aAD5415Config->nStride) + (pa_aAD5415Config->nOffset * pa_aAD5415Config->nChannelMode), sizeof(unsigned short));
	    g_aAD5415Spec.nCurrBuffer = 0;
	    if(!pa_aAD5415Config->pBufferBase0) {
	        erResult = ERR_AD5415_NOT_ENOUGH_MEMORY;
	        free(pa_aAD5415Config->pBufferBase0);
	        return erResult;
	    }
	} else if (pa_aAD5415Config->nBufferMode == AD5415_PINGPONG_BUFFER) {
	    pa_aAD5415Config->pBufferBase0 = (unsigned short *)calloc((pa_aAD5415Config->nChannelMode * pa_aAD5415Config->nMaxElements * pa_aAD5415Config->nStride) + (pa_aAD5415Config->nOffset * pa_aAD5415Config->nChannelMode), sizeof(unsigned short));
        pa_aAD5415Config->pBufferBase1 = (unsigned short *)calloc((pa_aAD5415Config->nChannelMode * pa_aAD5415Config->nMaxElements * pa_aAD5415Config->nStride) + (pa_aAD5415Config->nOffset * pa_aAD5415Config->nChannelMode), sizeof(unsigned short));
	    g_aAD5415Spec.nCurrBuffer = 1;
	    if(!pa_aAD5415Config->pBufferBase0 || !pa_aAD5415Config->pBufferBase1) {
	        erResult = ERR_AD5415_NOT_ENOUGH_MEMORY;
	        free(pa_aAD5415Config->pBufferBase0);
	        free(pa_aAD5415Config->pBufferBase1);
	        return erResult;
	    }
	    
	} else {
	    erResult = ERR_AD5415_BUFFER_MODE;
	    return erResult;
	}
	
	g_aAD5415Spec.pBufferBase0 = pa_aAD5415Config->pBufferBase0;
	g_aAD5415Spec.pBufferBase1 = pa_aAD5415Config->pBufferBase1;
	
	// init I/O ports for SPORT use
    sport_platformInit(g_aAD5415Spec.nSPORT, true, false);
	
	g_aAD5415Spec.nBufferMode = pa_aAD5415Config->nBufferMode;
	
	if(!pa_aAD5415Config->nStride) {
	    pa_aAD5415Config->nStride = 1;
	}
	
	g_aAD5415Spec.nMaxElements = pa_aAD5415Config->nMaxElements;
	
	// preparing descriptors for DMA transfer
	g_anTxDMAdescriptor[0][0] = (unsigned long)&g_anTxDMAdescriptor[1][0];
	g_anTxDMAdescriptor[1][0] = (unsigned long)&g_anTxDMAdescriptor[0][0];
	g_anTxDMAdescriptor[0][1] = (unsigned long)(pa_aAD5415Config->pBufferBase0 + pa_aAD5415Config->nOffset);
	g_anTxDMAdescriptor[1][1] = (unsigned long)(pa_aAD5415Config->pBufferBase1 + pa_aAD5415Config->nOffset);
	
    
    
    erPrepBuf = PrepareAD5415Buffer(    pa_aAD5415Config->pBufferBase0,
                                        pa_aAD5415Config->nMaxElements,
                                        pa_aAD5415Config->nStride,
                                        pa_aAD5415Config->nOffset,
                                        pa_aAD5415Config->nChannelMode);       // for buffer 0
    
    if (erPrepBuf != ERR_NONE) {
        ///error handling
    }
    
    if(pa_aAD5415Config->nBufferMode == AD5415_PINGPONG_BUFFER) {
        erPrepBuf = PrepareAD5415Buffer(    pa_aAD5415Config->pBufferBase1,
                                            pa_aAD5415Config->nMaxElements,
                                            pa_aAD5415Config->nStride,
                                            pa_aAD5415Config->nOffset,
                                            pa_aAD5415Config->nChannelMode);       // for buffer 1
                                            
        if (erPrepBuf != ERR_NONE) {
            ///error handling
        }
    }
    
    // configure SPORT
	
	unsigned short nClockDiv = AD5415GenerateClkDiv(pa_aAD5415Config->nSamplingRate, pa_aAD5415Config->nChannelMode, pa_aAD5415Config->nSclk);
	unsigned long nRealTclkFreq = (unsigned long)((float)pa_aAD5415Config->nSclk / (2.0 * (float)(nClockDiv + 1)));
	
	
	*(g_aSPORTspec[g_aAD5415Spec.nSPORT].pSPORTtclkdiv) = nClockDiv;    	                                                // Speed of seriell Data Transmition is limited to max ratings
	*(g_aSPORTspec[g_aAD5415Spec.nSPORT].pSPORTtfsdiv) = AD5415GenerateTFSDIV(pa_aAD5415Config->nSamplingRate, pa_aAD5415Config->nChannelMode, nRealTclkFreq);      	    // Count of clocks for seriell Data transmition
 	
	*(g_aSPORTspec[g_aAD5415Spec.nSPORT].pSPORTtcr2) = 0x000f;    
	*(g_aSPORTspec[g_aAD5415Spec.nSPORT].pSPORTtcr1) = 0x7602;    	                                                        //Blackfin as Master, active low Framesync
	
	
	pa_aAD5415Config->fRealSamplingRate = 1.0 / ((float)(*(g_aSPORTspec[g_aAD5415Spec.nSPORT].pSPORTtfsdiv) + 1.0) / (float)nRealTclkFreq);
			
	if (pa_aAD5415Config->fnCallback) {
	    g_aAD5415Spec.fCallback = pa_aAD5415Config->fnCallback;
	}
	
	//configure interrupts
	if(!pa_aAD5415Config->nSPORTperIVG) {
	    adi_int_SICGetIVG (g_aSPORTspec[g_aAD5415Spec.nSPORT].peripheralTxIntId, &(g_aAD5415Spec.nTxIVG));
	} else {
	    g_aAD5415Spec.nTxIVG = pa_aAD5415Config->nSPORTperIVG;
	    adi_int_SICSetIVG (g_aSPORTspec[g_aAD5415Spec.nSPORT].peripheralTxIntId, g_aAD5415Spec.nTxIVG);
	}
	
	if(adi_int_CECHook (g_aAD5415Spec.nTxIVG, AD5415txHandler, (void *)0, false) == ADI_INT_RESULT_SUCCESS) {
		adi_int_SICWakeup(g_aSPORTspec[g_aAD5415Spec.nSPORT].peripheralTxIntId, TRUE);
		adi_int_SICEnable(g_aSPORTspec[g_aAD5415Spec.nSPORT].peripheralTxIntId);
		
		// hook DMA error interrupt
		if(!pa_aAD5415Config->nDMAerrIVG) {
		    adi_int_SICGetIVG (g_aSPORTspec[g_aAD5415Spec.nSPORT].nDMAerrIntId, &(g_aAD5415Spec.nDMAerrIVG));
		} else {
	        g_aAD5415Spec.nDMAerrIVG = pa_aAD5415Config->nSPORTperIVG;
	        adi_int_SICSetIVG (g_aSPORTspec[g_aAD5415Spec.nSPORT].nDMAerrIntId, g_aAD5415Spec.nDMAerrIVG);
		}
		
		if(adi_int_CECHook (g_aAD5415Spec.nDMAerrIVG, AD5415DMAerrorHandler, (void *)0, false) == ADI_INT_RESULT_SUCCESS) {
			adi_int_SICWakeup(g_aSPORTspec[g_aAD5415Spec.nSPORT].nDMAerrIntId, TRUE);
			adi_int_SICEnable(g_aSPORTspec[g_aAD5415Spec.nSPORT].nDMAerrIntId);
		}
		
		// hook SPORT error interrupt
		if(!pa_aAD5415Config->nSPORTerrIVG) {
		    adi_int_SICGetIVG (g_aSPORTspec[g_aAD5415Spec.nSPORT].nPeriheralErrIntId, &pa_aAD5415Config->nSPORTerrIVG);
		}else {
	        g_aAD5415Spec.nSPORTerrIVG = pa_aAD5415Config->nSPORTperIVG;
	        adi_int_SICSetIVG (g_aSPORTspec[g_aAD5415Spec.nSPORT].nPeriheralErrIntId, g_aAD5415Spec.nSPORTerrIVG);
		}
		
		if(adi_int_CECHook (g_aAD5415Spec.nSPORTerrIVG, AD5415SPORTerrorHandler, (void *)0, false) == ADI_INT_RESULT_SUCCESS) {
			adi_int_SICWakeup(g_aSPORTspec[g_aAD5415Spec.nSPORT].nDMAerrIntId, TRUE);
			adi_int_SICEnable(g_aSPORTspec[g_aAD5415Spec.nSPORT].nDMAerrIntId);
		}
	
		
	} else {
		erResult = ERR_AD5415_HOOK_INTERRUPT;
		AD5415Cleanup();
		return erResult;
	}


    
    // configure DMA
    if(pa_aAD5415Config->nBufferMode == AD5415_SINGLE_BUFFER) {
        
        if(pa_aAD5415Config->nChannelMode == AD5415_SINGLE_CHANNEL) {                                  // Single Buffer, Single Channel
            *(g_aSPORTspec[g_aAD5415Spec.nSPORT].pTxDMAstartaddr) = (void *)(g_anTxDMAdescriptor[0][1]);	        // data buffer
	        *(g_aSPORTspec[g_aAD5415Spec.nSPORT].pTxDMAxcount) = (pa_aAD5415Config->nMaxElements);  					            // elements to transfer
		    *(g_aSPORTspec[g_aAD5415Spec.nSPORT].pTxDMAxmodify) = 2 * pa_aAD5415Config->nStride;
    		
	    	if (pa_aAD5415Config->fnCallback) {
		    		*(g_aSPORTspec[g_aAD5415Spec.nSPORT].pTxDMAconfig) = 0x10a4;									// autobuffer, 1D, interrupt enabled, 16bit transfer size
		    		
			    } else {
				    *(g_aSPORTspec[g_aAD5415Spec.nSPORT].pTxDMAconfig) = 0x1024;									// autobuffer, 1D, interrupt disabled, 16bit transfer size
    			}
    			
        } else if(pa_aAD5415Config->nChannelMode == AD5415_DUAL_CHANNEL) {                             // Single Buffer, Dual Channel
            
            *(g_aSPORTspec[g_aAD5415Spec.nSPORT].pTxDMAstartaddr) = (void *)(g_anTxDMAdescriptor[0][1]);	        // data buffer
            *(g_aSPORTspec[g_aAD5415Spec.nSPORT].pTxDMAxcount) = 2;
            *(g_aSPORTspec[g_aAD5415Spec.nSPORT].pTxDMAxmodify) = 2 * pa_aAD5415Config->nMaxElements * pa_aAD5415Config->nStride;
            *(g_aSPORTspec[g_aAD5415Spec.nSPORT].pTxDMAycount) = pa_aAD5415Config->nMaxElements;
            *(g_aSPORTspec[g_aAD5415Spec.nSPORT].pTxDMAymodify) = (signed short)(-2 * (pa_aAD5415Config->nMaxElements * pa_aAD5415Config->nStride) + 2 * pa_aAD5415Config->nStride);
            
            if (pa_aAD5415Config->fnCallback) {
		    		*(g_aSPORTspec[g_aAD5415Spec.nSPORT].pTxDMAconfig) = 0x10b4;									// autobuffer, 2D, interrupt enabled, 16bit transfer size
		    		
			    } else {
				    *(g_aSPORTspec[g_aAD5415Spec.nSPORT].pTxDMAconfig) = 0x1034;									// autobuffer, 2D, interrupt disabled, 16bit transfer size
    			}
            
        } else {
            erResult = ERR_AD5415_CHANNEL;
            AD5415Cleanup();
		    return erResult;
        }
        
    } else if(pa_aAD5415Config->nBufferMode == AD5415_PINGPONG_BUFFER) {                                // Dual Buffer (PingPong Buffer), Single Channel
        
        if(pa_aAD5415Config->nChannelMode == AD5415_SINGLE_CHANNEL) {
            *(g_aSPORTspec[g_aAD5415Spec.nSPORT].pTxDMAstartaddr) = (void *)(g_anTxDMAdescriptor[0][1]);	        // data buffer
	        *(g_aSPORTspec[g_aAD5415Spec.nSPORT].pTxDMAxcount) = 2;              								    // transfer one element per row
		    *(g_aSPORTspec[g_aAD5415Spec.nSPORT].pTxDMAxmodify) = 2 * pa_aAD5415Config->nStride;
		    *(g_aSPORTspec[g_aAD5415Spec.nSPORT].pTxDMAycount) = pa_aAD5415Config->nMaxElements/2;                                 // elements to transfer
		    *(g_aSPORTspec[g_aAD5415Spec.nSPORT].pTxDMAymodify) = 2 * pa_aAD5415Config->nStride;
		    *(g_aSPORTspec[g_aAD5415Spec.nSPORT].pTxNextDescrPtr) = (void *)&g_anTxDMAdescriptor[0][0];
		    
		    if (pa_aAD5415Config->fnCallback) {
		    		*(g_aSPORTspec[g_aAD5415Spec.nSPORT].pTxDMAconfig) = 0x74b4;									// descriptor mode, 2D, interrupt enabled, 16bit transfer size
		    		
			    } else {
				    *(g_aSPORTspec[g_aAD5415Spec.nSPORT].pTxDMAconfig) = 0x7434;									// descriptor mode, 2D, interrupt disabled, 16bit transfer size
    			}
    			
        } else if(pa_aAD5415Config->nChannelMode == AD5415_DUAL_CHANNEL) {                             // Dual Buffer (PingPong Buffer), Dual Channel
            
            *(g_aSPORTspec[g_aAD5415Spec.nSPORT].pTxDMAstartaddr) = (void *)(g_anTxDMAdescriptor[0][1]);	        // data buffer
	        *(g_aSPORTspec[g_aAD5415Spec.nSPORT].pTxDMAxcount) = 2;              								    // transfer one element per row
		    *(g_aSPORTspec[g_aAD5415Spec.nSPORT].pTxDMAxmodify) = 2 * pa_aAD5415Config->nMaxElements * pa_aAD5415Config->nStride;
		    *(g_aSPORTspec[g_aAD5415Spec.nSPORT].pTxDMAycount) = pa_aAD5415Config->nMaxElements;                       // elements to transfer
		    *(g_aSPORTspec[g_aAD5415Spec.nSPORT].pTxDMAymodify) = (signed short)(-2 * (signed short)(pa_aAD5415Config->nMaxElements * pa_aAD5415Config->nStride) + 2 * pa_aAD5415Config->nStride);
		    *(g_aSPORTspec[g_aAD5415Spec.nSPORT].pTxNextDescrPtr) = (void *)&g_anTxDMAdescriptor[0][0];
		    
		    if (pa_aAD5415Config->fnCallback) {
		    		*(g_aSPORTspec[g_aAD5415Spec.nSPORT].pTxDMAconfig) = 0x74b4;									// descriptor mode, 2D, interrupt enabled, 16bit transfer size
		    		
			    } else {
				    *(g_aSPORTspec[g_aAD5415Spec.nSPORT].pTxDMAconfig) = 0x7434;									// descriptor mode, 2D, interrupt disabled, 16bit transfer size
		    }
        } else {
            erResult = ERR_AD5415_CHANNEL;
            AD5415Cleanup();
		    return erResult;
        }
        
    } else {
        erResult = ERR_AD5415_BUFFER_MODE;
        AD5415Cleanup();
		return erResult;
    }
    
	return erResult;
}


/**     
*       @brief Starts AD5415 sampling
**/
void AD5415Start(void) {
    // enable DMA
	*(g_aSPORTspec[g_aAD5415Spec.nSPORT].pTxDMAconfig) |= 1; 
	// enable SPORT
	*(g_aSPORTspec[g_aAD5415Spec.nSPORT].pSPORTtcr1)   |= 1;
	
	g_aAD5415Spec.bIsActive = true;
}


/**     
*       @brief Stops AD5415 sampling
**/
void AD5415Stop(void) {
    
	// disable SPORT
	*(g_aSPORTspec[g_aAD5415Spec.nSPORT].pSPORTtcr1)   &= ~1;
	// disable DMA
	*(g_aSPORTspec[g_aAD5415Spec.nSPORT].pTxDMAconfig) &= ~1; 
	
	g_aAD5415Spec.bIsActive = false;
}


/**     
*       @brief prepares the buffer for sampling, masks the buffer with channel information
*       @param  *pa_pBufferBase         Address of the Buffer to prepare
*       @param  pa_nMaxElements           size of the buffer
*       @param  pa_nStride              stride used in the buffer
*       @param  pa_nOffset              offset used in the buffer
*       @param  pa_nChannelMode         number of channels used
*       @return erResult                returns an error code
**/
T_ERROR_CODE PrepareAD5415Buffer(   unsigned short *pa_pBufferBase,
                                    unsigned short pa_nMaxElements,
                                    unsigned short pa_nStride,
                                    unsigned short pa_nOffset,
                                    T_AD5415_CHANNEL_MODE pa_nChannelMode) {
                                        
    T_ERROR_CODE erResult = ERR_NONE;
   	unsigned short i;
   	unsigned short nMaxCondition = (pa_nMaxElements * pa_nStride) + pa_nOffset;
   	unsigned short *pBufferBaseCh2;
   	pBufferBaseCh2 = (unsigned short *)(pa_pBufferBase + ((pa_nMaxElements * pa_nStride) + pa_nOffset));
    
    
    if(pa_nChannelMode == AD5415_SINGLE_CHANNEL) {
        
        for(i=pa_nOffset; i<nMaxCondition; i+= pa_nStride) {                        // Single Channel
            *(pa_pBufferBase + i) &= ~0xF000;                                         // Channel A
            *(pa_pBufferBase + i) |= 0x1000;
        }
        
    } else if(pa_nChannelMode == AD5415_DUAL_CHANNEL) {

        for(i=pa_nOffset; i<nMaxCondition; i+= pa_nStride) {                        // Dual Channel
            *(pa_pBufferBase + i) &= ~0xF000;                                         // Channel A
            *(pa_pBufferBase + i) |= 0x1000;
            *(pBufferBaseCh2 + i) &= ~0xF000;                                         // Channel B
            *(pBufferBaseCh2 + i) |= 0x4000;
        }
        
    } else {
        erResult = ERR_AD5415_CHANNEL;
    }
    
    return erResult;
}


/**     
*       @brief  Closes AD5415, if it should be used again, run AD5415Setup() first
**/
void AD5415Cleanup(void) {
    free ((void *)g_aAD5415Spec.pBufferBase0);
    free ((void *)g_aAD5415Spec.pBufferBase1);
    g_aSPORTspec[g_aAD5415Spec.nSPORT].bTxInUse = false;
    g_aAD5415Spec.bInUse = false;
    
}


/**     
*       @brief Generates the Clock Divider for AD5415Setup
*       @param  pa_nSamplingRate        the Sampling Rate fpr wich the Clock divider should be calculated
*       @param  pa_nChannelMode         how many channels are used
*       @param  pa_nSclk                System Clock
*       @return nClockDiv               Clock Divider
**/
unsigned short AD5415GenerateClkDiv(    unsigned long pa_nSamplingRate,
                                        T_AD5415_CHANNEL_MODE pa_nChannelMode,
                                        unsigned long pa_nSclk) {
    
    unsigned long nTransferClock;
    
    nTransferClock = pa_nSamplingRate * 16 * pa_nChannelMode;

    if(nTransferClock > AD5415_MAX_TRANSFER_CLOCK) {
	    nTransferClock = AD5415_MAX_TRANSFER_CLOCK;
	}
	
	unsigned short nClockDiv = (unsigned short)((float)pa_nSclk / (2.0 * (float)(nTransferClock)) - 1.0);
	
	return (unsigned short)nClockDiv;
}


/**     
*       @brief Generates the Clock Divider for AD5415Setup
*       @param  pa_nSamplingRate        the Sampling Rate fpr wich the Clock divider should be calculated
*       @param  pa_nChannelMode         how many channels are used
*       @param  pa_nRealTclkFreq        Serial transfer Frequency
*       @return nClockDiv               Clock Divider
**/
unsigned short AD5415GenerateTFSDIV(    unsigned long pa_nSamplingRate,
                                        T_AD5415_CHANNEL_MODE pa_nChannelMode,
                                        unsigned long pa_nRealTclkFreq) {
    unsigned short nTFSdiv;
    
    
	nTFSdiv = (pa_nRealTclkFreq / (pa_nSamplingRate * pa_nChannelMode)) - 1;
	
	if(nTFSdiv < AD5415_MIN_TFS_DIV) {
	    nTFSdiv = AD5415_MIN_TFS_DIV;
	}
	
	return nTFSdiv;
}

