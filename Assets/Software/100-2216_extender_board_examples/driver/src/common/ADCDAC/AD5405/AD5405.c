/**
	@file AD5405.c
	@ingroup adcdac
	@brief driver for the AD5405, a parallel digital to analog converter
	
	
	
	BLT_DISCLAIMER(TBD)
	@author Alexander Froemel
	@version 1.0
	@date 28.06.2009
	
	@startcond Changelog
	
	@endcond
**/

#include "AD5405.h"
#include "../../gpTimerConfig.h"

static T_AD5405_BUFFER_SPEC *g_aAD5405BufferSpec;
static T_AD5405_SPEC g_aAD5405Spec;


extern T_GP_TIMER_SPEC g_aTIMERspec[];			///< An array describing the GP timers, defined in timer_global.h

T_AD5405_CALLBACK_ARG g_aAD5405CallbackArg;

unsigned short *g_pAD5405RingBuffer;
T_GP_TIMER_INST *g_pTimerHandle;
bool g_bRunCallback = false;

unsigned long long g_curAD5405[5];


/**     
*       @brief Timer Interrupt Handler, writes the ring buffer to AD5405 and runs the user callback function
**/
#pragma section("L1_code")
void AD5405WriteSample(void *pa_pCallbackArg) {
                                                                        // writes the ringbuffer to AD5405
  
//    _GET_CYCLE_COUNT(g_curAD5405[0]);

    unsigned short i;
    
    
    
    for(i=0;i<g_aAD5405Spec.nChannelMode; i++) {
        
        if(g_aAD5405BufferSpec[i].nElementsLoaded) {                   // if buffer is not empty
        
            if(g_aAD5405BufferSpec[i].nIndexOut >= g_aAD5405Spec.nMaxElements) {
                g_aAD5405BufferSpec[i].nIndexOut = 0;
            }
        
            *(g_aAD5405BufferSpec[i].pWriteBuffer) = *((unsigned short *)(g_aAD5405BufferSpec[i].pBufferBase) + (g_aAD5405BufferSpec[i].nIndexOut));
                
            g_aAD5405BufferSpec[i].nElementsLoaded--;
            g_aAD5405BufferSpec[i].nIndexOut++;
            
            g_aAD5405CallbackArg.nElementsLoaded = g_aAD5405BufferSpec[i].nElementsLoaded;
            g_aAD5405CallbackArg.nChannel = i;
            if( g_aAD5405BufferSpec[i].nElementsLoaded > g_aAD5405BufferSpec[i].nMaxFillStatus) {
                g_aAD5405CallbackArg.bBufferFull = true;
//                g_bRunCallback = false;
            
            }else if(g_aAD5405BufferSpec[i].nElementsLoaded <= g_aAD5405BufferSpec[i].nMinFillStatus) {
                    g_bRunCallback = true;
                    g_aAD5405CallbackArg.bBufferFull = false;
            }
            
        
        } else {                                                        // if buffer is empty
        
            switch(g_aAD5405BufferSpec[i].nBufferEmptyMode) {
            
                case AD5405_BUFFER_EMPTY_WRITE_0: {                     // zero output
                    *(g_aAD5405BufferSpec[i].pWriteBuffer) = 0x0000;
                    break;
                }
            
                case AD5405_BUFFER_EMPTY_WRITE_1: {                     // max. output
                    if(i == 0) {
                        *(g_aAD5405BufferSpec[i].pWriteBuffer) = 0x1fff;
                    } else {
                        *(g_aAD5405BufferSpec[i].pWriteBuffer) = 0x4fff;
                    }
                    
                    break;
                }
                 
                case AD5405_BUFFER_EMPTY_WRITE_LAST_SAMPLE: {           // last sample to output
/*                
                    if(g_aAD5405BufferSpec[i].nIndexOut) {
                        *(g_aAD5405BufferSpec[i].pWriteBuffer) = *(unsigned short *)(((unsigned short *)g_aAD5405BufferSpec[i].pBufferBase) + (g_aAD5405BufferSpec[i].nIndexOut - 1));
                    } else{
                        *(g_aAD5405BufferSpec[i].pWriteBuffer) = *(unsigned short *)(((unsigned short *)g_aAD5405BufferSpec[i].pBufferBase) + (g_aAD5405Spec.nMaxElements - 1));
                    }
*/                    
                    break;
                }
                 
                case AD5405_BUFFER_EMPTY_AUTOBUFFER: {                  // set autobuffer mode
                    g_aAD5405BufferSpec[i].nElementsLoaded = g_aAD5405Spec.nMaxElements;
                    break;   
                }
                 
                default: {
                    /// error handling
                    break;
                }   
            }
        }
        
        if(g_aAD5405Spec.fnCallback) {
            g_aAD5405Spec.fnCallback(&g_aAD5405CallbackArg);
        }
    }
    
    
    
//    _GET_CYCLE_COUNT(g_curAD5405[1]);
//    g_curAD5405[3] = g_curAD5405[1] - g_curAD5405[0];
//    g_curAD5405[4] = g_curAD5405[0] - g_curAD5405[2];
//    g_curAD5405[2] = g_curAD5405[0];    
    
}


/**     
*       @brief  Configuration of the AD5405 parallel analog to digital converter
*       @param  pa_nSamplingRate            the sampling rate at which the AD5405 samples
*       @param  pa_nMaxElements             the ring buffer size
*       @param  pa_nTimerNr                 selects the timer that should be used
*       @param  pa_nSystemClk               System Clock
*       @param  pa_nWriteBufferAddress      Address to write to AD5405, if zero, default address is used (defined in AD5405.h)
*       @param  pa_nBufferEmptyMode         the used stategy when the buffer is empty
*       @param  pa_fnCallback               user callback function
*       @param  *__p_reserved               reserved parameter
*       @param  pa_nMaxFillStatus           Buffer fill strategy, max condition, in % of the buffersize (max. elements), callback function is called until the fill status reaches max condition, if callback should be called after each write cycle set min and max to 100
*       @param  pa_nMinFillStatus           Buffer fill strategy, min condition, in % of the buffersize (max. elements), callback function is not calles until fill status decreases under min condition
*       @return erResult                    returns an error code
**/
T_ERROR_CODE AD5405Setup(T_AD5405_CONFIG *pa_aAD5405Config) {
                                
                                
    unsigned short i;
    T_ERROR_CODE erResult = ERR_NONE;
    
    g_aAD5405Spec.nMaxElements = pa_aAD5405Config->nMaxElements;
    g_aAD5405Spec.nChannelMode = pa_aAD5405Config->nChannelMode;
    g_aAD5405BufferSpec = (T_AD5405_BUFFER_SPEC *)malloc(sizeof(T_AD5405_BUFFER_SPEC) * pa_aAD5405Config->nChannelMode);
    
    if(!g_aAD5405BufferSpec) {
        erResult = ERR_AD5405_NOT_ENOUGH_MEMORY;
        free(g_aAD5405BufferSpec);
        return erResult;
    }
    
    if(pa_aAD5405Config->fnCallback) {
        g_aAD5405Spec.fnCallback = pa_aAD5405Config->fnCallback;
    }
    
    g_pAD5405RingBuffer = (unsigned short *)malloc(2 * pa_aAD5405Config->nMaxElements * pa_aAD5405Config->nChannelMode);
    
    if(!g_pAD5405RingBuffer) {
        erResult = ERR_AD5405_NOT_ENOUGH_MEMORY;
        free(g_pAD5405RingBuffer);
        free(g_aAD5405BufferSpec);
        return erResult;
    }
    
    if(pa_aAD5405Config->nChannelMode != AD5405_SINGLE_CHANNEL && pa_aAD5405Config->nChannelMode != AD5405_DUAL_CHANNEL) {
        erResult = ERR_AD5405_CHANNEL;
        free(g_pAD5405RingBuffer);
        free(g_aAD5405BufferSpec);
        return erResult;
    }
    
    for(i=0;i<pa_aAD5405Config->nChannelMode;i++) {
        g_aAD5405BufferSpec[i].pBufferBase = (unsigned short *)((unsigned short *)g_pAD5405RingBuffer + (pa_aAD5405Config->nMaxElements * i));
        g_aAD5405BufferSpec[i].nIndexIn = 0;
        g_aAD5405BufferSpec[i].nIndexOut = 0;
        g_aAD5405BufferSpec[i].nElementsLoaded = 0;
        g_aAD5405BufferSpec[i].nBufferEmptyMode = pa_aAD5405Config->nBufferEmptyMode;
        g_aAD5405BufferSpec[i].pWriteBuffer = (unsigned short *)((unsigned short *)pa_aAD5405Config->nWriteBufferAddress + i*2);
        g_aAD5405BufferSpec[i].nMaxFillStatus = (unsigned short)(((float)pa_aAD5405Config->nMaxFillStatus / 100.0) * (float)pa_aAD5405Config->nMaxElements);
        g_aAD5405BufferSpec[i].nMinFillStatus = (unsigned short)(((float)pa_aAD5405Config->nMinFillStatus / 100.0) * (float)pa_aAD5405Config->nMaxElements);
    }
    
    T_GP_TIMER_SETUP_PARAM aTimerSetup;
    
    aTimerSetup.nTimerNr = pa_aAD5405Config->nTimerNr;
    aTimerSetup.nConfig = 0x005d;
    aTimerSetup.nWidth = (unsigned long)((pa_aAD5405Config->nSystemClk / pa_aAD5405Config->nSamplingRate) / 2);
    aTimerSetup.nPeriod = (unsigned long)(pa_aAD5405Config->nSystemClk / pa_aAD5405Config->nSamplingRate);
    aTimerSetup.nIVG = 0;
    aTimerSetup.cTMRCLKinputPin = 0;
    aTimerSetup.bPositivePulse = false;
    aTimerSetup.nSystemClk = pa_aAD5405Config->nSystemClk;
    aTimerSetup.fnCallback = AD5405WriteSample;
    aTimerSetup.pCallbackArg = 0;
    
    g_pTimerHandle = timer_gp_setup (&aTimerSetup);
    
    if(!g_pTimerHandle) {
        erResult = ERR_AD5405_NOT_ENOUGH_MEMORY;
        AD5405Cleanup();
        return erResult;
    }
    
    return erResult;
}


/**     
*       @brief  function to write to the ring buffer
*       @param  pa_nValue       value to write to teh buffer
*       @param  pa_nChannel     corresponding channel of the value
*       @return erResult        returns an error code
**/
T_ERROR_CODE AD5405Write(unsigned short pa_nValue, unsigned short pa_nChannel) {
                                                                // writes value into the ringbuffer
    T_ERROR_CODE erResult = ERR_NONE;
    unsigned short *pActualBufferAddress;
    
    if(g_aAD5405BufferSpec[pa_nChannel].nElementsLoaded < g_aAD5405Spec.nMaxElements) {
        
        pActualBufferAddress = (unsigned short *)((unsigned short *)(g_aAD5405BufferSpec[pa_nChannel].pBufferBase) + (g_aAD5405BufferSpec[pa_nChannel].nIndexIn));
        
        *(g_aTIMERspec[g_pTimerHandle->nSystemTimerNr].pTIMERconfig) &= ~0x0010;
        
        *pActualBufferAddress = pa_nValue;
        g_aAD5405BufferSpec[pa_nChannel].nElementsLoaded++;
        g_aAD5405BufferSpec[pa_nChannel].nIndexIn++;
        
        if(g_aAD5405BufferSpec[pa_nChannel].nIndexIn >= g_aAD5405Spec.nMaxElements) {
            g_aAD5405BufferSpec[pa_nChannel].nIndexIn = 0;
        }
        *(g_aTIMERspec[g_pTimerHandle->nSystemTimerNr].pTIMERconfig) |= 0x0010;
    } else {
        erResult = ERR_AD5405_RINGBUFFER_FULL;
    }
    
    return erResult;
}


/**     
*       @brief  Starts AD5405 sampling
**/
void AD5405Start(void) {
    timer_enable(g_pTimerHandle);
}


/**     
*       @brief  Stops AD5405 sampling
**/
void AD5405Stop(void) {
    timer_disable(g_pTimerHandle);
}


/**     
*       @brief  Closes AD5405, if it should be used again, run AD5405Setup() first
**/
void AD5405Cleanup(void) {
    timer_close(g_pTimerHandle);
    free(g_pAD5405RingBuffer);
    free(g_aAD5405BufferSpec);
}
