/**
	@file EPPIcameraIn.c
	@ingroup camera
	@brief Interface driver to configure the EPPI for capturing images from a camera
	
	
	
	BLT_DISCLAIMER(TBD)
	@author Alexander Froemel
	@version 1.0
	@date 13.07.2009
	
	@startcond Changelog
	
	@endcond
**/


#include <cplbtab.h>
#include <stdio.h>
#include <conio.h>
#include <cycle_count_bf.h>


// local include
#include "EPPIcameraIn.h"
#include "../../EPPI.h"
#include "../../../../../BLACKSheep/common/syslog/syslog.h"

#ifdef _USE_VDK_
	#include <VDK.h>
#endif

extern unsigned int g_nEPPIcount;	// Platform Specific in EPPI_global, EPPI count
extern T_EPPI_SPEC 	g_aEPPIspec[];	// Platform Specific in EPPI_global, EPPI specification array

unsigned long long curEPPI[20];

unsigned long g_nDMAerrorCount = 0;
unsigned long g_nEPPIerrorCountOther = 0;
unsigned long g_nEPPIerrorCountLineOV = 0;
unsigned long g_nEPPIframesCaptured = 0;

T_EPPI_CAMERA_IN_HANDLE g_aEPPIcameraInHndlCount[EPPI_CAMERA_IN_MAX_NOF_DEVICES];				// array of connected camera handles

section("L1_data") unsigned long g_aEPPIcameraInDMAdescriptor[EPPI_CAMERA_IN_MAX_NOF_DEVICES * FG_MAX_NOF_BUFFERED_FRAMES * EPPI_CAMERA_IN_DESCR_SIZE];          // descriptor for double buffer or multi frame buffer mode


#pragma section("L1_code")
static ADI_INT_HANDLER_RESULT EPPIcameraInInterruptHandler(void *pa_pClientArg) {
    T_EPPI_CAMERA_IN_SPEC *pEPPIcameraIn = (T_EPPI_CAMERA_IN_SPEC *)pa_pClientArg;		// Provide a pointer for the Argument to the defined T_EPPI_SPEC structure
    if (adi_int_SICInterruptAsserted(pEPPIcameraIn->pEPPIspec->peripheralIntId0) == ADI_INT_RESULT_ASSERTED) {
        
        T_EPPI_CAMERA_IN_CALLBACK_ARG stBufferStatus;
        unsigned long long nCycles;
        unsigned short nCurIndexIn;
        unsigned char i;
//        _GET_CYCLE_COUNT(nCycles);
        
//        _GET_CYCLE_COUNT(curEPPI[pEPPIcameraIn->cDeviceID]);
        
        *(pEPPIcameraIn->pEPPIspec->pDMA0irqStat) = 0x1;					// clear the interrupt
        
        if(pEPPIcameraIn->fnGetTimestamp){
            pEPPIcameraIn->pFrameDescr[pEPPIcameraIn->nIndexIn]->ulTS = pEPPIcameraIn->fnGetTimestamp();
        }
        nCurIndexIn = pEPPIcameraIn->nIndexIn;
        if(pEPPIcameraIn->nFramesLoaded < pEPPIcameraIn->nMaxFrames) {
            pEPPIcameraIn->nFramesLoaded++;
        }
        g_nEPPIframesCaptured++;
        
#if _EPPI_CAMERA_IN_DEBUG_LVL_ > 0
        char acMessage[100];
        sprintf(acMessage, "EPPIcameraIn: IntHandler, Frames: %u\n", g_nEPPIframesCaptured);
        syslog_post(acMessage, 0, 0, NULL);
#endif
            
        switch(pEPPIcameraIn->nBufferMode) {
            
            case EPPI_CAMERA_IN_SINGLE_BUFFER: {
                
                stBufferStatus.nIndexOut = pEPPIcameraIn->nIndexOut;
                stBufferStatus.nFramesLoaded = pEPPIcameraIn->nFramesLoaded;
                stBufferStatus.pNextFrame = pEPPIcameraIn->pFrameBuffer[pEPPIcameraIn->nIndexOut];
                
                if(pEPPIcameraIn->nFramesLoaded >= pEPPIcameraIn->nMaxFrames) {
                    EPPIcameraIn_stop((T_EPPI_CAMERA_IN_HANDLE)pEPPIcameraIn);          // has to be started by user when ready to receive next frame
                }
                
                break;
            }
            
            case EPPI_CAMERA_IN_DOUBLE_BUFFER_OVERWRITE: {
                
                if(pEPPIcameraIn->nIndexIn) {
                    pEPPIcameraIn->nIndexIn = 0;
                    pEPPIcameraIn->nIndexOut = 1;
                } else {
                    pEPPIcameraIn->nIndexIn = 1;
                    pEPPIcameraIn->nIndexOut = 0;
                }
                
                stBufferStatus.nIndexOut = pEPPIcameraIn->nIndexOut;
                stBufferStatus.nFramesLoaded = pEPPIcameraIn->nFramesLoaded;
                stBufferStatus.pNextFrame = pEPPIcameraIn->pFrameBuffer[pEPPIcameraIn->nIndexOut];
                
                break;
            }
            
            case EPPI_CAMERA_IN_DOUBLE_BUFFER_NO_OVERWRITE: {
                
                if(pEPPIcameraIn->nIndexIn) {
                    pEPPIcameraIn->nIndexIn = 0;
                    pEPPIcameraIn->nIndexOut = 1;
                } else {
                    pEPPIcameraIn->nIndexIn = 1;
                    pEPPIcameraIn->nIndexOut = 0;
                }
                
                if(pEPPIcameraIn->nFramesLoaded == pEPPIcameraIn->nMaxFrames) {
                    EPPIcameraIn_stop((T_EPPI_CAMERA_IN_HANDLE)pEPPIcameraIn);          // is started if user requests for next frame is framebuffer is empty
                    
                    *(pEPPIcameraIn->pEPPIspec->pDMA0nextDescrPtr) = &g_aEPPIcameraInDMAdescriptor[pEPPIcameraIn->nDescrIndOffset + (pEPPIcameraIn->nIndexIn * 2)];
                }
                
                stBufferStatus.nIndexOut = pEPPIcameraIn->nIndexOut;
                stBufferStatus.nFramesLoaded = pEPPIcameraIn->nFramesLoaded;
                stBufferStatus.pNextFrame = pEPPIcameraIn->pFrameBuffer[pEPPIcameraIn->nIndexOut];
                
                break;
            }
            
            case EPPI_CAMERA_IN_MULTI_FRAME_BUFFER_OVERWRITE: {
                
                pEPPIcameraIn->nIndexIn++;
                if(pEPPIcameraIn->nIndexIn >= pEPPIcameraIn->nMaxFrames) {
                    pEPPIcameraIn->nIndexIn = 0;
                } 
                
                if(pEPPIcameraIn->nIndexOut == pEPPIcameraIn->nIndexIn) {
                    pEPPIcameraIn->nIndexOut++;
                    if(pEPPIcameraIn->nIndexOut >= pEPPIcameraIn->nMaxFrames) {
                        pEPPIcameraIn->nIndexOut = 0;
                    } 
                }
                
                stBufferStatus.nIndexOut = pEPPIcameraIn->nIndexOut;
                stBufferStatus.nFramesLoaded = pEPPIcameraIn->nFramesLoaded;
                stBufferStatus.pNextFrame = pEPPIcameraIn->pFrameBuffer[pEPPIcameraIn->nIndexOut];
                
                break;
            }
            
            case EPPI_CAMERA_IN_MULTI_FRAME_BUFFER_NO_OVERWRITE: {
                
                pEPPIcameraIn->nIndexIn++;
                if(pEPPIcameraIn->nIndexIn >= pEPPIcameraIn->nMaxFrames) {
                    pEPPIcameraIn->nIndexIn = 0;
                } 
                
                if(pEPPIcameraIn->nFramesLoaded >= pEPPIcameraIn->nMaxFrames) {
                    EPPIcameraIn_stop((T_EPPI_CAMERA_IN_HANDLE)pEPPIcameraIn);          // is started if user requests for next frame is framebuffer is empty
                    *(pEPPIcameraIn->pEPPIspec->pDMA0nextDescrPtr) = &g_aEPPIcameraInDMAdescriptor[pEPPIcameraIn->nDescrIndOffset + (pEPPIcameraIn->nIndexIn * 2)];
                }
                
                stBufferStatus.nIndexOut = pEPPIcameraIn->nIndexOut;
                stBufferStatus.nFramesLoaded = pEPPIcameraIn->nFramesLoaded;
                stBufferStatus.pNextFrame = pEPPIcameraIn->pFrameBuffer[pEPPIcameraIn->nIndexOut];
                
                break;
            }
            
            default: {
                break;
            }
        }
            
        if (pEPPIcameraIn->fnCallback) {							// Check if a Callback function was hooked, (check not NULL!)
            pEPPIcameraIn->fnCallback(stBufferStatus);
		}
		
		
		if( (__cplb_ctrl & CPLB_ENABLE_DCACHE ) || (__cplb_ctrl & CPLB_ENABLE_DCACHE2)){
            dcache_invalidate_both();                                   // invalidate cash so that data written by the dma is cached again
        }
		
/*		
		for(i=0; i<2; i++) {
		    if(curEPPI[i] - curEPPI[2 + i]) {
		        curEPPI[4 + i] = curEPPI[i] - curEPPI[2 + i];
		    }
		}
		curEPPI[pEPPIcameraIn->cDeviceID + 2] = curEPPI[pEPPIcameraIn->cDeviceID];
*/
		return ADI_INT_RESULT_PROCESSED;			// Interrupt processed, notify ADI's ISR
		
	} else {
		return ADI_INT_RESULT_NOT_PROCESSED;	// Interrupt wasn't for us, notify ADI's ISR that we didn't process it!
		
    }
}


#pragma section("L1_code")
ADI_INT_HANDLER_RESULT EPPIcameraInErrorInterruptHandler(void *pa_pClientArg) {
  
	T_EPPI_CAMERA_IN_SPEC *pEPPIcameraIn = (T_EPPI_CAMERA_IN_SPEC *)pa_pClientArg;		// Provide a pointer for the parameter data to the defined T_EPPI_SPEC structure.
	T_EPPI_CAMERA_IN_CALLBACK_ARG stBufferStatus;
	
	if (adi_int_SICInterruptAsserted(pEPPIcameraIn->pEPPIspec->perErrorIntId0) == ADI_INT_RESULT_ASSERTED) {
	    
		
	    if (pEPPIcameraIn->fnErrorCallback) {										// Check if a Callback function was hooked
	        stBufferStatus.nIndexOut = pEPPIcameraIn->nIndexOut;
            stBufferStatus.nFramesLoaded = pEPPIcameraIn->nFramesLoaded;
		    pEPPIcameraIn->fnErrorCallback(stBufferStatus);				// If so, execute it!
		}
		
		// for debugging, counting of different errors
        if(*(pEPPIcameraIn->pEPPIspec->pEPPIstatus) & 0x0004) {
            g_nEPPIerrorCountLineOV ++;
#if _EPPI_CAMERA_IN_DEBUG_LVL_ > 0
        syslog_post("EPPIcameraIn: EPPI error, Line overflow\n", 0, 0, NULL);
#endif
        } else if(*(pEPPIcameraIn->pEPPIspec->pEPPIstatus)) {
            g_nEPPIerrorCountOther ++;
#if _EPPI_CAMERA_IN_DEBUG_LVL_ > 0
        char acMessage[100];
        sprintf(acMessage, "EPPIcameraIn: EPPI error, EPPIstatus: 0x%8.8x\n", (unsigned long) *(pEPPIcameraIn->pEPPIspec->pEPPIstatus));
        syslog_post(acMessage, 0, 0, NULL);
#endif
        }

		*(pEPPIcameraIn->pEPPIspec->pEPPIstatus) = *(pEPPIcameraIn->pEPPIspec->pEPPIstatus) & 0x01ff;

		/*
	    if( (__cplb_ctrl & CPLB_ENABLE_DCACHE ) || (__cplb_ctrl & CPLB_ENABLE_DCACHE2)){
            dcache_invalidate_both();                                   // invalidate cash so that data written by the dma is cached again
        }
        */
		return ADI_INT_RESULT_PROCESSED;			// Processed the interrupt, so notify ADI's ISR routine.
	} 
	
	else {
		return ADI_INT_RESULT_NOT_PROCESSED;	// Interrupt wasn't processed, so notify ADI's ISR routine.
	}
}



#pragma section("L1_code")
ADI_INT_HANDLER_RESULT EPPIcameraInDmaErrorInterruptHandler(void *pa_pClientArg) {
  
	T_EPPI_CAMERA_IN_SPEC *pEPPIcameraIn = (T_EPPI_CAMERA_IN_SPEC *)pa_pClientArg;		// Provide a pointer for the parameter data to the defined T_EPPI_SPEC structure.
	T_EPPI_CAMERA_IN_CALLBACK_ARG stBufferStatus;
	
	if (adi_int_SICInterruptAsserted(pEPPIcameraIn->pEPPIspec->DMAerrorIntId0) == ADI_INT_RESULT_ASSERTED) {
	    
		
	    if (pEPPIcameraIn->fnDmaErrorCallback) {										// Check if a Callback function was hooked
	        stBufferStatus.nIndexOut = pEPPIcameraIn->nIndexOut;
            stBufferStatus.nFramesLoaded = pEPPIcameraIn->nFramesLoaded;
		    pEPPIcameraIn->fnDmaErrorCallback(stBufferStatus);				// If so, execute it!
		}
		
		// for debugging, counting of different errors
        if(*(pEPPIcameraIn->pEPPIspec->pDMA0irqStat) & 0x0002) {
            g_nDMAerrorCount ++;
#if _EPPI_CAMERA_IN_DEBUG_LVL_ > 0
        syslog_post("EPPIcameraIn: DMA error\n", 0, 0, NULL);
#endif
        }

		*(pEPPIcameraIn->pEPPIspec->pDMA0irqStat) = *(pEPPIcameraIn->pEPPIspec->pDMA0irqStat) & 0x0002;					// clear the interrupt

		/*
	    if( (__cplb_ctrl & CPLB_ENABLE_DCACHE ) || (__cplb_ctrl & CPLB_ENABLE_DCACHE2)){
            dcache_invalidate_both();                                   // invalidate cash so that data written by the dma is cached again
        }
        */
		return ADI_INT_RESULT_PROCESSED;			// Processed the interrupt, so notify ADI's ISR routine.
	} 
	
	else {
		return ADI_INT_RESULT_NOT_PROCESSED;	// Interrupt wasn't processed, so notify ADI's ISR routine.
	}
}


T_EPPI_CAMERA_IN_HANDLE EPPIcameraIn_open(T_EPPI_CAMERA_IN_CONF *pa_pEPPIcameraInConfig) {
    
    unsigned char cEPPI = pa_pEPPIcameraInConfig->cEPPI;
    T_EPPI_CAMERA_IN_SPEC *pEPPIcameraInSpec = (T_EPPI_CAMERA_IN_SPEC *)malloc(sizeof(T_EPPI_CAMERA_IN_SPEC));
    unsigned short i;
    
    if(!pEPPIcameraInSpec) {
        pa_pEPPIcameraInConfig->erError = ERR_EPPI_CAMERA_IN_NOT_ENOUGH_MEMORY;
        return 0;
    }
    
    for(i=0; i<=EPPI_CAMERA_IN_MAX_NOF_DEVICES; i++) {
        if(!g_aEPPIcameraInHndlCount[i]) {
            g_aEPPIcameraInHndlCount[i] = (T_EPPI_CAMERA_IN_HANDLE) pEPPIcameraInSpec;
            pEPPIcameraInSpec->cDeviceID = i;
            i = EPPI_CAMERA_IN_MAX_NOF_DEVICES + 1;
        }
        
        if(i == EPPI_CAMERA_IN_MAX_NOF_DEVICES) {
            pa_pEPPIcameraInConfig->erError = ERR_EPPI_CAMERA_IN_TOO_MANY_INSTANCES;
            return 0;
        }
    }
    
    pEPPIcameraInSpec->nDescrIndOffset = (pEPPIcameraInSpec->cDeviceID * FG_MAX_NOF_BUFFERED_FRAMES * EPPI_CAMERA_IN_DESCR_SIZE);
    
    pEPPIcameraInSpec->nBufferMode = pa_pEPPIcameraInConfig->nBufferMode;
    pEPPIcameraInSpec->fnCallback = pa_pEPPIcameraInConfig->fnCallback;
    pEPPIcameraInSpec->fnErrorCallback = pa_pEPPIcameraInConfig->fnErrorCallback;
    pEPPIcameraInSpec->fnDmaErrorCallback = pa_pEPPIcameraInConfig->fnDmaErrorCallback;
    pEPPIcameraInSpec->nDataInterfaceMode = pa_pEPPIcameraInConfig->nDataInterfaceMode;
    pEPPIcameraInSpec->cEPPI = pa_pEPPIcameraInConfig->cEPPI;
    pEPPIcameraInSpec->nXres = pa_pEPPIcameraInConfig->nXres;
    pEPPIcameraInSpec->nYres = pa_pEPPIcameraInConfig->nYres;
    pEPPIcameraInSpec->pEPPIspec = &g_aEPPIspec[cEPPI];
    pEPPIcameraInSpec->nFramesLoaded = 0;
    pEPPIcameraInSpec->nIndexIn = 0;
    pEPPIcameraInSpec->nIndexOut = 0;
    pEPPIcameraInSpec->nNofFramesToHold = pa_pEPPIcameraInConfig->nNofFramesToHold;
    pEPPIcameraInSpec->bIsActive = false;
    pEPPIcameraInSpec->bDequeueInProgress = false;
    pEPPIcameraInSpec->fFramerate = pa_pEPPIcameraInConfig->fFramerate;
    pEPPIcameraInSpec->nColorMode = pa_pEPPIcameraInConfig->nColorMode;
    pEPPIcameraInSpec->fnHwDevCloseDev = (EPPI_CAMERA_IN_FN_CLOSE_DEVICE)pa_pEPPIcameraInConfig->pHwDevCloseDev;
    pEPPIcameraInSpec->hHwDevHndl = pa_pEPPIcameraInConfig->hHwDevHndl;
    pEPPIcameraInSpec->fnGetTimestamp = pa_pEPPIcameraInConfig->fnGetTimestamp;
    pEPPIcameraInSpec->unPerIVG = pa_pEPPIcameraInConfig->nPerIVG;;
    pEPPIcameraInSpec->unPerErrorIVG = pa_pEPPIcameraInConfig->nPerErrorIVG;
    pEPPIcameraInSpec->unDmaErrorIVG = pa_pEPPIcameraInConfig->nDmaErrorIVG;
    
    pEPPIcameraInSpec->pFrameGrabberInterface = (void **)malloc(sizeof(unsigned long) * FG_MAX_VIDEO_DEVICE_FUNCTIONS);
    
    //initialize FrameGrabber interface
    pEPPIcameraInSpec->pFrameGrabberInterface[FG_DEVICE_RESET] 			    = (void *)EPPIcameraIn_resetDevice;
	pEPPIcameraInSpec->pFrameGrabberInterface[FG_FREE_FRAME] 			    = (void *)EPPIcameraIn_freeFrame;
	pEPPIcameraInSpec->pFrameGrabberInterface[FG_FRAME_AVAILABLE] 		    = (void *)EPPIcameraIn_framesAvailable;
	pEPPIcameraInSpec->pFrameGrabberInterface[FG_DEQUEUE_FRAME] 			= (void *)EPPIcameraIn_getNextFrame;
	pEPPIcameraInSpec->pFrameGrabberInterface[FG_GET_FRAMERATE] 			= (void *)EPPIcameraIn_getFramerate;
	pEPPIcameraInSpec->pFrameGrabberInterface[FG_GET_VIDEO_DEVICE_INFO] 	= (void *)EPPIcameraIn_getDeviceInfo;
    pEPPIcameraInSpec->pFrameGrabberInterface[FG_START_GRABBING]            = (void *)EPPIcameraIn_start;
    pEPPIcameraInSpec->pFrameGrabberInterface[FG_STOP_GRABBING]             = (void *)EPPIcameraIn_stop;
	pEPPIcameraInSpec->pFrameGrabberInterface[FG_CONFIG_DEVICE] 			= (void *)EPPIcameraIn_configDevice;
	
	pa_pEPPIcameraInConfig->pFrameGrabberInterface = pEPPIcameraInSpec->pFrameGrabberInterface;
    
    pa_pEPPIcameraInConfig->erError = ERR_NONE;
    
    // check if specified EPPI port is within limits
    if(pa_pEPPIcameraInConfig->cEPPI >= g_nEPPIcount) {
        pa_pEPPIcameraInConfig->erError = ERR_EPPI_CAMERA_IN_EPPI_INDEX;
        EPPIcameraIn_close((T_EPPI_CAMERA_IN_HANDLE)pEPPIcameraInSpec);
        return 0;
    }
    
    // check if specified EPPI port is in use
    if(g_aEPPIspec[cEPPI].bInUse) {
        pa_pEPPIcameraInConfig->erError = ERR_EPPI_CAMERA_IN_ALREADY_IN_USE;
        EPPIcameraIn_close((T_EPPI_CAMERA_IN_HANDLE)pEPPIcameraInSpec);
        return 0;
    }
    
    g_aEPPIspec[cEPPI].bInUse = true;
    
    // check for Data Interface Mode error
    if(pa_pEPPIcameraInConfig->nDataInterfaceMode != CAM_8BIT_HVSYNC 
            && pa_pEPPIcameraInConfig->nDataInterfaceMode != CAM_10BIT_HVSYNC
            && pa_pEPPIcameraInConfig->nDataInterfaceMode != CAM_16BIT_HVSYNC) {
        
        pa_pEPPIcameraInConfig->erError = ERR_EPPI_CAMERA_IN_DATA_INTERFACE_MODE;
        EPPIcameraIn_close((T_EPPI_CAMERA_IN_HANDLE)pEPPIcameraInSpec);
        return 0;
    }
    
    
    // configure EPPI and DMA according to BufferMode and DataInterfaceMode
    
//    *pEBIU_AMGCTL |= (1<<8);
    switch(pa_pEPPIcameraInConfig->nBufferMode) {
        
        case EPPI_CAMERA_IN_SINGLE_BUFFER: {
            
            pEPPIcameraInSpec->pFrameBuffer = (unsigned short **)malloc(sizeof(unsigned long) * 1);
            pEPPIcameraInSpec->pFrameBuffer[0] = (void *)calloc(pa_pEPPIcameraInConfig->nXres * pa_pEPPIcameraInConfig->nYres, sizeof(unsigned short));
            pEPPIcameraInSpec->pFrameDescr = (ifrm_t **)malloc(sizeof(unsigned long) * 1);
            pEPPIcameraInSpec->pFrameDescr[0] = (ifrm_t *)malloc(sizeof(ifrm_t));
            
            if(!(pEPPIcameraInSpec->pFrameBuffer) || !(pEPPIcameraInSpec->pFrameBuffer[0]) || !pEPPIcameraInSpec->pFrameDescr) {
                pa_pEPPIcameraInConfig->erError = ERR_EPPI_CAMERA_IN_NOT_ENOUGH_MEMORY;
                EPPIcameraIn_close((T_EPPI_CAMERA_IN_HANDLE)pEPPIcameraInSpec);
                return 0;
            }
            pEPPIcameraInSpec->pFrameDescr[0]->pucFramePtr = (unsigned char *)pEPPIcameraInSpec->pFrameBuffer[0];
            pEPPIcameraInSpec->nIndexOut = 0;
            pEPPIcameraInSpec->nMaxFrames = pa_pEPPIcameraInConfig->nMaxFrames = 1;
            pEPPIcameraInSpec->nFramesLoaded = 0;
            
            if(pEPPIcameraInSpec->nDataInterfaceMode == CAM_8BIT_HVSYNC) {
                
                *(g_aEPPIspec[cEPPI].pDMA0startaddr) = (void *)(pEPPIcameraInSpec->pFrameBuffer[0]);
                *(g_aEPPIspec[cEPPI].pDMA0xcount) = (pa_pEPPIcameraInConfig->nXres * 2);
                *(g_aEPPIspec[cEPPI].pDMA0xmodify) = 1;
                *(g_aEPPIspec[cEPPI].pDMA0ycount) = pa_pEPPIcameraInConfig->nYres;
                *(g_aEPPIspec[cEPPI].pDMA0ymodify) = 1;
                
                *g_aEPPIspec[cEPPI].pDMA0config = 0x00b2;                            // Stop mode, Int enable, Syncronized transition, 2D, 8bit transfere, Memory write OP,
                
                *(g_aEPPIspec[cEPPI].pEPPIline) = (pa_pEPPIcameraInConfig->nXres * 2);
                *(g_aEPPIspec[cEPPI].pEPPIframe) = pa_pEPPIcameraInConfig->nYres;
                *(g_aEPPIspec[cEPPI].pEPPIhdelay) = 0;
                *(g_aEPPIspec[cEPPI].pEPPIvdelay) = 0;
                *(g_aEPPIspec[cEPPI].pEPPIcontrol) = 0x3020186c;                            // GP 2FS mode, DLEN 8bit, external trigger, Sample data on rising edge and sample/drive syncs on rising edge
                
            
            } else if(pa_pEPPIcameraInConfig->nDataInterfaceMode == CAM_10BIT_HVSYNC) {
                *(g_aEPPIspec[cEPPI].pDMA0startaddr) = (void *)(pEPPIcameraInSpec->pFrameBuffer[0]);
                *(g_aEPPIspec[cEPPI].pDMA0xcount) = pa_pEPPIcameraInConfig->nXres;
                *(g_aEPPIspec[cEPPI].pDMA0xmodify) = 2;
                *(g_aEPPIspec[cEPPI].pDMA0ycount) = pa_pEPPIcameraInConfig->nYres;
                *(g_aEPPIspec[cEPPI].pDMA0ymodify) = 2;
                *(g_aEPPIspec[cEPPI].pDMA0nextDescrPtr) = 0;

                *g_aEPPIspec[cEPPI].pDMA0config = 0x00b6;                            // Stop mode, Int enable, Syncronized transition, 2D, 16bit transfere, Memory write OP,
                                
                *(g_aEPPIspec[cEPPI].pEPPIline) = (pa_pEPPIcameraInConfig->nXres); // - 1;
                *(g_aEPPIspec[cEPPI].pEPPIframe) = pa_pEPPIcameraInConfig->nYres;
                *(g_aEPPIspec[cEPPI].pEPPIcontrol) = 0x3021186c;                            // GP 2FS mode, DLEN 10bit, external trigger, Sample data on rising edge and sample/drive syncs on rising edge
            }
            
            break;
        }
        
        case EPPI_CAMERA_IN_DOUBLE_BUFFER_OVERWRITE: {
            
            pEPPIcameraInSpec->pFrameBuffer = (unsigned short **)malloc(sizeof(unsigned long) * 2);
            pEPPIcameraInSpec->pFrameBuffer[0] = (unsigned short *)calloc(pa_pEPPIcameraInConfig->nXres * pa_pEPPIcameraInConfig->nYres, sizeof(unsigned short));
            pEPPIcameraInSpec->pFrameBuffer[1] = (unsigned short *)calloc(pa_pEPPIcameraInConfig->nXres * pa_pEPPIcameraInConfig->nYres, sizeof(unsigned short));
            pEPPIcameraInSpec->pFrameDescr = (ifrm_t **)malloc(sizeof(unsigned long) * 2);
            pEPPIcameraInSpec->pFrameDescr[0] = (ifrm_t *)malloc(sizeof(ifrm_t));
            pEPPIcameraInSpec->pFrameDescr[1] = (ifrm_t *)malloc(sizeof(ifrm_t));
           
            if( !(pEPPIcameraInSpec->pFrameBuffer) || !(pEPPIcameraInSpec->pFrameBuffer[0]) || !(pEPPIcameraInSpec->pFrameBuffer[1]) 
                || !pEPPIcameraInSpec->pFrameDescr || !pEPPIcameraInSpec->pFrameDescr[0] || !pEPPIcameraInSpec->pFrameDescr[1]) {
                pa_pEPPIcameraInConfig->erError = ERR_EPPI_CAMERA_IN_NOT_ENOUGH_MEMORY;
                EPPIcameraIn_close((T_EPPI_CAMERA_IN_HANDLE)pEPPIcameraInSpec);
                return 0;
            }
            pEPPIcameraInSpec->pFrameDescr[0]->pucFramePtr = (unsigned char *)pEPPIcameraInSpec->pFrameBuffer[0];
            pEPPIcameraInSpec->pFrameDescr[1]->pucFramePtr = (unsigned char *)pEPPIcameraInSpec->pFrameBuffer[1];
            
            pEPPIcameraInSpec->nIndexOut = 1;
            pEPPIcameraInSpec->nIndexIn = 0;
            pEPPIcameraInSpec->nFramesLoaded = 0;
            pEPPIcameraInSpec->nMaxFrames = pa_pEPPIcameraInConfig->nMaxFrames = 2;
            
            g_aEPPIcameraInDMAdescriptor[pEPPIcameraInSpec->nDescrIndOffset + 0] = (unsigned long)&(g_aEPPIcameraInDMAdescriptor[pEPPIcameraInSpec->nDescrIndOffset + 2]);	            // Next Descriptor (Second half of this array)
			g_aEPPIcameraInDMAdescriptor[pEPPIcameraInSpec->nDescrIndOffset + 1] = (unsigned long)pEPPIcameraInSpec->pFrameBuffer[0];				// Start address for 1st buffer
			g_aEPPIcameraInDMAdescriptor[pEPPIcameraInSpec->nDescrIndOffset + 2] = (unsigned long)&(g_aEPPIcameraInDMAdescriptor[pEPPIcameraInSpec->nDescrIndOffset + 0]);	            // Next Descriptor (First half of this array)
			g_aEPPIcameraInDMAdescriptor[pEPPIcameraInSpec->nDescrIndOffset + 3] = (unsigned long)pEPPIcameraInSpec->pFrameBuffer[1];				// Start address for 2nd buffer
			
			if(pa_pEPPIcameraInConfig->nDataInterfaceMode == CAM_8BIT_HVSYNC) {
                
                *(g_aEPPIspec[cEPPI].pDMA0nextDescrPtr) = (void *)&g_aEPPIcameraInDMAdescriptor[pEPPIcameraInSpec->nDescrIndOffset + 0];
                *(g_aEPPIspec[cEPPI].pDMA0xcount) = pa_pEPPIcameraInConfig->nXres * 2;
                *(g_aEPPIspec[cEPPI].pDMA0xmodify) = 1;
                *(g_aEPPIspec[cEPPI].pDMA0ycount) = pa_pEPPIcameraInConfig->nYres;
                *(g_aEPPIspec[cEPPI].pDMA0ymodify) = 1;
                
                *g_aEPPIspec[cEPPI].pDMA0config = 0x74b2;                            // Descr list large, Descr size 4, Int enable, Syncronized transition, 2D, 8bit transfere, Memory write OP,
                
                *(g_aEPPIspec[cEPPI].pEPPIframe) = pa_pEPPIcameraInConfig->nYres;
                *(g_aEPPIspec[cEPPI].pEPPIline) = (pa_pEPPIcameraInConfig->nXres * 2); // - 1;
                *(g_aEPPIspec[cEPPI].pEPPIcontrol) = 0x3020186c;                            // GP 2FS mode, DLEN 8bit, external trigger, Sample data on rising edge and sample/drive syncs on rising edge
                
            
            } else if(pa_pEPPIcameraInConfig->nDataInterfaceMode == CAM_10BIT_HVSYNC) {
                
                *(g_aEPPIspec[cEPPI].pDMA0nextDescrPtr) = (void *)&g_aEPPIcameraInDMAdescriptor[pEPPIcameraInSpec->nDescrIndOffset + 0];
                *(g_aEPPIspec[cEPPI].pDMA0xcount) = pa_pEPPIcameraInConfig->nXres;
                *(g_aEPPIspec[cEPPI].pDMA0xmodify) = 2;
                *(g_aEPPIspec[cEPPI].pDMA0ycount) = pa_pEPPIcameraInConfig->nYres;
                *(g_aEPPIspec[cEPPI].pDMA0ymodify) = 2;
                
                *g_aEPPIspec[cEPPI].pDMA0config = 0x74b6;                            // Descr list large, Descr size 4, Int enable, Syncronized transition, 2D, 16bit transfere, Memory write OP,
                                
                *(g_aEPPIspec[cEPPI].pEPPIframe) = pa_pEPPIcameraInConfig->nYres;
                *(g_aEPPIspec[cEPPI].pEPPIline) = (pa_pEPPIcameraInConfig->nXres); // - 1;
                *(g_aEPPIspec[cEPPI].pEPPIcontrol) = 0x3021186c;                            // GP 2FS mode, DLEN 10bit, external trigger, Sample data on rising edge and sample/drive syncs on rising edge
                
            }
			
            break;
        }
        
        case EPPI_CAMERA_IN_DOUBLE_BUFFER_NO_OVERWRITE: {
            
            pEPPIcameraInSpec->pFrameBuffer = (unsigned short **)malloc(sizeof(unsigned long) * 2);
            pEPPIcameraInSpec->pFrameBuffer[0] = (unsigned short *)calloc(pa_pEPPIcameraInConfig->nXres * pa_pEPPIcameraInConfig->nYres, sizeof(unsigned short));
            pEPPIcameraInSpec->pFrameBuffer[1] = (unsigned short *)calloc(pa_pEPPIcameraInConfig->nXres * pa_pEPPIcameraInConfig->nYres, sizeof(unsigned short));
            pEPPIcameraInSpec->pFrameDescr = (ifrm_t **)malloc(sizeof(unsigned long) * 2);
            pEPPIcameraInSpec->pFrameDescr[0] = (ifrm_t *)malloc(sizeof(ifrm_t));
            pEPPIcameraInSpec->pFrameDescr[1] = (ifrm_t *)malloc(sizeof(ifrm_t));
           
            if( !(pEPPIcameraInSpec->pFrameBuffer) || !(pEPPIcameraInSpec->pFrameBuffer[0]) || !(pEPPIcameraInSpec->pFrameBuffer[1]) 
                || !pEPPIcameraInSpec->pFrameDescr || !pEPPIcameraInSpec->pFrameDescr[0] || !pEPPIcameraInSpec->pFrameDescr[1]) {
                pa_pEPPIcameraInConfig->erError = ERR_EPPI_CAMERA_IN_NOT_ENOUGH_MEMORY;
                EPPIcameraIn_close((T_EPPI_CAMERA_IN_HANDLE)pEPPIcameraInSpec);
                return 0;
            }
            pEPPIcameraInSpec->pFrameDescr[0]->pucFramePtr = (unsigned char *)pEPPIcameraInSpec->pFrameBuffer[0];
            pEPPIcameraInSpec->pFrameDescr[1]->pucFramePtr = (unsigned char *)pEPPIcameraInSpec->pFrameBuffer[1];
            
            pEPPIcameraInSpec->nIndexOut = 0;
            pEPPIcameraInSpec->nIndexIn = 0;
            pEPPIcameraInSpec->nFramesLoaded = 0;
            pEPPIcameraInSpec->nMaxFrames = pa_pEPPIcameraInConfig->nMaxFrames = 2;
            
            g_aEPPIcameraInDMAdescriptor[pEPPIcameraInSpec->nDescrIndOffset + 0] = (unsigned long)&g_aEPPIcameraInDMAdescriptor[pEPPIcameraInSpec->nDescrIndOffset + 2];	            // Next Descriptor (Second half of this array)
			g_aEPPIcameraInDMAdescriptor[pEPPIcameraInSpec->nDescrIndOffset + 1] = (unsigned long)pEPPIcameraInSpec->pFrameBuffer[0];				// Start address for 1st buffer
			g_aEPPIcameraInDMAdescriptor[pEPPIcameraInSpec->nDescrIndOffset + 2] = (unsigned long)&g_aEPPIcameraInDMAdescriptor[pEPPIcameraInSpec->nDescrIndOffset + 0];	            // Next Descriptor (First half of this array)
			g_aEPPIcameraInDMAdescriptor[pEPPIcameraInSpec->nDescrIndOffset + 3] = (unsigned long)pEPPIcameraInSpec->pFrameBuffer[1];				// Start address for 2nd buffer
			
			if(pa_pEPPIcameraInConfig->nDataInterfaceMode == CAM_8BIT_HVSYNC) {
                
                *(g_aEPPIspec[cEPPI].pDMA0nextDescrPtr) = (void *)&g_aEPPIcameraInDMAdescriptor[pEPPIcameraInSpec->nDescrIndOffset + 0];
                *(g_aEPPIspec[cEPPI].pDMA0xcount) = pa_pEPPIcameraInConfig->nXres * 2;
                *(g_aEPPIspec[cEPPI].pDMA0xmodify) = 1;
                *(g_aEPPIspec[cEPPI].pDMA0ycount) = pa_pEPPIcameraInConfig->nYres;
                *(g_aEPPIspec[cEPPI].pDMA0ymodify) = 1;
                
                *g_aEPPIspec[cEPPI].pDMA0config = 0x74b2;                            // Descr list large, Descr size 4, Int enable, Syncronized transition, 2D, 8bit transfere, Memory write OP,
                
                *(g_aEPPIspec[cEPPI].pEPPIframe) = pa_pEPPIcameraInConfig->nYres;
                *(g_aEPPIspec[cEPPI].pEPPIline) = (pa_pEPPIcameraInConfig->nXres * 2); // - 1;
                *(g_aEPPIspec[cEPPI].pEPPIcontrol) = 0x3020186c;                            // GP 2FS mode, DLEN 8bit, external trigger, Sample data on rising edge and sample/drive syncs on rising edge
                
            
            } else if(pa_pEPPIcameraInConfig->nDataInterfaceMode == CAM_10BIT_HVSYNC) {
                
                *(g_aEPPIspec[cEPPI].pDMA0nextDescrPtr) = (void *)&g_aEPPIcameraInDMAdescriptor[pEPPIcameraInSpec->nDescrIndOffset + 0];
                *(g_aEPPIspec[cEPPI].pDMA0xcount) = pa_pEPPIcameraInConfig->nXres;
                *(g_aEPPIspec[cEPPI].pDMA0xmodify) = 2;
                *(g_aEPPIspec[cEPPI].pDMA0ycount) = pa_pEPPIcameraInConfig->nYres;
                *(g_aEPPIspec[cEPPI].pDMA0ymodify) = 2;
                
                *g_aEPPIspec[cEPPI].pDMA0config = 0x74b6;                            // Descr list large, Descr size 4, Int enable, Syncronized transition, 2D, 16bit transfere, Memory write OP,
                
                *(g_aEPPIspec[cEPPI].pEPPIframe) = pa_pEPPIcameraInConfig->nYres;
                *(g_aEPPIspec[cEPPI].pEPPIline) = (pa_pEPPIcameraInConfig->nXres); // - 1;
                *(g_aEPPIspec[cEPPI].pEPPIcontrol) = 0x3021186c;                            // GP 2FS mode, DLEN 10bit, external trigger, Sample data on rising edge and sample/drive syncs on rising edge
                
            }
			
            break;
        }
        
        case EPPI_CAMERA_IN_MULTI_FRAME_BUFFER_OVERWRITE: {
            
            unsigned short nMaxFrames;                                                   // calculate the max. possible frames
            unsigned short iMax;
            
            if(pa_pEPPIcameraInConfig->cSDRAMsize){
                nMaxFrames = (pa_pEPPIcameraInConfig->cSDRAMsize << 20) / (pa_pEPPIcameraInConfig->nXres * pa_pEPPIcameraInConfig->nYres * 2);
                
                if(nMaxFrames < pa_pEPPIcameraInConfig->nMaxFrames) {
                    pa_pEPPIcameraInConfig->nMaxFrames = nMaxFrames;
                }
            }

            pEPPIcameraInSpec->nMaxFrames = pa_pEPPIcameraInConfig->nMaxFrames;
            
            if(!pEPPIcameraInSpec->nMaxFrames){
                pa_pEPPIcameraInConfig->erError = ERR_EPPI_CAMERA_IN_PARAMETER_ERROR;
                return 0;
            }
            
            pEPPIcameraInSpec->pFrameBuffer = (unsigned short **)malloc(sizeof(unsigned long) * pa_pEPPIcameraInConfig->nMaxFrames);
            pEPPIcameraInSpec->pFrameDescr = (ifrm_t **)malloc(sizeof(unsigned long) * pa_pEPPIcameraInConfig->nMaxFrames);
            
            
            
            if(!(pEPPIcameraInSpec->pFrameBuffer) || !pEPPIcameraInSpec->pFrameDescr) {
                pa_pEPPIcameraInConfig->erError = ERR_EPPI_CAMERA_IN_NOT_ENOUGH_MEMORY;
                EPPIcameraIn_close((T_EPPI_CAMERA_IN_HANDLE)pEPPIcameraInSpec);
                return 0;
            }
            
            for(i=0; i<pa_pEPPIcameraInConfig->nMaxFrames; i++) {
                pEPPIcameraInSpec->pFrameBuffer[i] = (unsigned short *)calloc(pa_pEPPIcameraInConfig->nXres * pa_pEPPIcameraInConfig->nYres, sizeof(unsigned short));
                pEPPIcameraInSpec->pFrameDescr[i] = (ifrm_t *)malloc(sizeof(ifrm_t));
                
                if(!(pEPPIcameraInSpec->pFrameBuffer[i]) || !(pEPPIcameraInSpec->pFrameDescr[i])) {
                    pa_pEPPIcameraInConfig->erError = ERR_EPPI_CAMERA_IN_NOT_ENOUGH_MEMORY;
                    EPPIcameraIn_close((T_EPPI_CAMERA_IN_HANDLE)pEPPIcameraInSpec);
                    return 0;
                }
                pEPPIcameraInSpec->pFrameDescr[i]->pucFramePtr = (unsigned char *)pEPPIcameraInSpec->pFrameBuffer[i];
            }
            
            iMax = pa_pEPPIcameraInConfig->nMaxFrames * 2;
            
            for(i=0; i<iMax; i+=2) {
                if((i+2) >= iMax) {
                    g_aEPPIcameraInDMAdescriptor[pEPPIcameraInSpec->nDescrIndOffset + i] = (unsigned long)&g_aEPPIcameraInDMAdescriptor[pEPPIcameraInSpec->nDescrIndOffset + 0];
                } else {
                    g_aEPPIcameraInDMAdescriptor[pEPPIcameraInSpec->nDescrIndOffset + i] = (unsigned long)&g_aEPPIcameraInDMAdescriptor[pEPPIcameraInSpec->nDescrIndOffset + i + 2];
                }
                
                g_aEPPIcameraInDMAdescriptor[pEPPIcameraInSpec->nDescrIndOffset + i + 1] = (unsigned long)pEPPIcameraInSpec->pFrameBuffer[i/2];
            }
            
            if(pa_pEPPIcameraInConfig->nDataInterfaceMode == CAM_8BIT_HVSYNC) {
                
                *(g_aEPPIspec[cEPPI].pDMA0nextDescrPtr) = (void *)&g_aEPPIcameraInDMAdescriptor[pEPPIcameraInSpec->nDescrIndOffset + 0];
                *(g_aEPPIspec[cEPPI].pDMA0xcount) = pa_pEPPIcameraInConfig->nXres * 2;
                *(g_aEPPIspec[cEPPI].pDMA0xmodify) = 1;
                *(g_aEPPIspec[cEPPI].pDMA0ycount) = pa_pEPPIcameraInConfig->nYres;
                *(g_aEPPIspec[cEPPI].pDMA0ymodify) = 1;

                *g_aEPPIspec[cEPPI].pDMA0config = 0x74b2;                            // Descr list large, Descr size 4, Int enable, Syncronized transition, 2D, 8bit transfere, Memory write OP,
                                
                *(g_aEPPIspec[cEPPI].pEPPIframe) = pa_pEPPIcameraInConfig->nYres;
                *(g_aEPPIspec[cEPPI].pEPPIline) = (pa_pEPPIcameraInConfig->nXres * 2); // - 1;
                *(g_aEPPIspec[cEPPI].pEPPIcontrol) = 0x3020186c;                            // GP 2FS mode, DLEN 8bit, external trigger, Sample data on rising edge and sample/drive syncs on rising edge
                
            
            } else if(pa_pEPPIcameraInConfig->nDataInterfaceMode == CAM_10BIT_HVSYNC) {
                
                *(g_aEPPIspec[cEPPI].pDMA0nextDescrPtr) = (void *)&g_aEPPIcameraInDMAdescriptor[pEPPIcameraInSpec->nDescrIndOffset + 0];
                *(g_aEPPIspec[cEPPI].pDMA0xcount) = pa_pEPPIcameraInConfig->nXres;
                *(g_aEPPIspec[cEPPI].pDMA0xmodify) = 2;
                *(g_aEPPIspec[cEPPI].pDMA0ycount) = pa_pEPPIcameraInConfig->nYres;
                *(g_aEPPIspec[cEPPI].pDMA0ymodify) = 2;
                
                *g_aEPPIspec[cEPPI].pDMA0config = 0x74b6;                            // Descr list large, Descr size 4, Int enable, Syncronized transition, 2D, 16bit transfere, Memory write OP,
                                
                *(g_aEPPIspec[cEPPI].pEPPIframe) = pa_pEPPIcameraInConfig->nYres;
                *(g_aEPPIspec[cEPPI].pEPPIline) = (pa_pEPPIcameraInConfig->nXres); // - 1;
                *(g_aEPPIspec[cEPPI].pEPPIcontrol) = 0x3021186c;                            // GP 2FS mode, DLEN 10bit, external trigger, Sample data on rising edge and sample/drive syncs on rising edge
                
            }
            
            break;
        }
        
        case EPPI_CAMERA_IN_MULTI_FRAME_BUFFER_NO_OVERWRITE: {
            
            unsigned short nMaxFrames;                                                   // calculate the max. possible frames
            unsigned short iMax;
            
            if(pa_pEPPIcameraInConfig->cSDRAMsize){
                nMaxFrames = (pa_pEPPIcameraInConfig->cSDRAMsize << 20) / (pa_pEPPIcameraInConfig->nXres * pa_pEPPIcameraInConfig->nYres * 2);
                
                if(nMaxFrames < pa_pEPPIcameraInConfig->nMaxFrames) {
                    pa_pEPPIcameraInConfig->nMaxFrames = nMaxFrames;
                }
            }

            pEPPIcameraInSpec->nMaxFrames = pa_pEPPIcameraInConfig->nMaxFrames;
            
            if(!pEPPIcameraInSpec->nMaxFrames){
                pa_pEPPIcameraInConfig->erError = ERR_EPPI_CAMERA_IN_PARAMETER_ERROR;
                return 0;
            }
            
            iMax = pa_pEPPIcameraInConfig->nMaxFrames * 2;
            
            pEPPIcameraInSpec->pFrameBuffer = (unsigned short **)malloc(sizeof(unsigned long) * pa_pEPPIcameraInConfig->nMaxFrames);
            pEPPIcameraInSpec->pFrameDescr = (ifrm_t **)malloc(sizeof(unsigned long) * pa_pEPPIcameraInConfig->nMaxFrames);
            
            if(!(pEPPIcameraInSpec->pFrameBuffer) || !pEPPIcameraInSpec->pFrameDescr) {
                pa_pEPPIcameraInConfig->erError = ERR_EPPI_CAMERA_IN_NOT_ENOUGH_MEMORY;
                EPPIcameraIn_close((T_EPPI_CAMERA_IN_HANDLE)pEPPIcameraInSpec);
                return 0;
            }
            
            for(i=0; i<pa_pEPPIcameraInConfig->nMaxFrames; i++) {
                pEPPIcameraInSpec->pFrameBuffer[i] = (unsigned short *)calloc(pa_pEPPIcameraInConfig->nXres * pa_pEPPIcameraInConfig->nYres, sizeof(unsigned short));
                pEPPIcameraInSpec->pFrameDescr[i] = (ifrm_t *)malloc(sizeof(ifrm_t));
                
                if(!(pEPPIcameraInSpec->pFrameBuffer[i]) || !(pEPPIcameraInSpec->pFrameDescr[i])) {
                    pa_pEPPIcameraInConfig->erError = ERR_EPPI_CAMERA_IN_NOT_ENOUGH_MEMORY;
                    EPPIcameraIn_close((T_EPPI_CAMERA_IN_HANDLE)pEPPIcameraInSpec);
                    return 0;
                }
                pEPPIcameraInSpec->pFrameDescr[i]->pucFramePtr = (unsigned char *)pEPPIcameraInSpec->pFrameBuffer[i];
            }
            
            for(i=0; i<iMax; i+=2) {
                if((i+2) >= iMax) {
                    g_aEPPIcameraInDMAdescriptor[pEPPIcameraInSpec->nDescrIndOffset + i] = (unsigned long)&g_aEPPIcameraInDMAdescriptor[pEPPIcameraInSpec->nDescrIndOffset + 0];
                } else {
                    g_aEPPIcameraInDMAdescriptor[pEPPIcameraInSpec->nDescrIndOffset + i] = (unsigned long)&g_aEPPIcameraInDMAdescriptor[pEPPIcameraInSpec->nDescrIndOffset + i + 2];
                }
                
                g_aEPPIcameraInDMAdescriptor[pEPPIcameraInSpec->nDescrIndOffset + i + 1] = (unsigned long)pEPPIcameraInSpec->pFrameBuffer[i/2];
            }
            
            if(pa_pEPPIcameraInConfig->nDataInterfaceMode == CAM_8BIT_HVSYNC) {
                
                *(g_aEPPIspec[cEPPI].pDMA0nextDescrPtr) = (void *)&g_aEPPIcameraInDMAdescriptor[pEPPIcameraInSpec->nDescrIndOffset + 0];
                *(g_aEPPIspec[cEPPI].pDMA0xcount) = pa_pEPPIcameraInConfig->nXres * 2;
                *(g_aEPPIspec[cEPPI].pDMA0xmodify) = 1;
                *(g_aEPPIspec[cEPPI].pDMA0ycount) = pa_pEPPIcameraInConfig->nYres;
                *(g_aEPPIspec[cEPPI].pDMA0ymodify) = 1;

                *g_aEPPIspec[cEPPI].pDMA0config = 0x74b2;                            // Descr list large, Descr size 4, Int enable, Syncronized transition, 2D, 32bit transfere, Memory write OP,
                                
                *(g_aEPPIspec[cEPPI].pEPPIframe) = pa_pEPPIcameraInConfig->nYres;
                //*(g_aEPPIspec[cEPPI].pEPPIhdelay) = 0;
                *(g_aEPPIspec[cEPPI].pEPPIline) = (pa_pEPPIcameraInConfig->nXres * 2); // - 1;
                //*(g_aEPPIspec[cEPPI].pEPPIvdelay) = 0;
                *(g_aEPPIspec[cEPPI].pEPPIcontrol) = 0x3020186c;                            // GP 2FS mode, DLEN 8bit, external trigger, Sample data on rising edge and sample/drive syncs on rising edge
                
            
            } else if(pa_pEPPIcameraInConfig->nDataInterfaceMode == CAM_10BIT_HVSYNC) {
                
                *(g_aEPPIspec[cEPPI].pDMA0nextDescrPtr) = (void *)&g_aEPPIcameraInDMAdescriptor[pEPPIcameraInSpec->nDescrIndOffset + 0];
                *(g_aEPPIspec[cEPPI].pDMA0xcount) = pa_pEPPIcameraInConfig->nXres;
                *(g_aEPPIspec[cEPPI].pDMA0xmodify) = 2;
                *(g_aEPPIspec[cEPPI].pDMA0ycount) = pa_pEPPIcameraInConfig->nYres;
                *(g_aEPPIspec[cEPPI].pDMA0ymodify) = 2;
                
                *g_aEPPIspec[cEPPI].pDMA0config = 0x74b6;                            // Descr list large, Descr size 4, Int enable, Syncronized transition, 2D, 16bit transfere, Memory write OP,
                
                *(g_aEPPIspec[cEPPI].pEPPIframe) = pa_pEPPIcameraInConfig->nYres;
                *(g_aEPPIspec[cEPPI].pEPPIline) = (pa_pEPPIcameraInConfig->nXres); // - 1;
                *(g_aEPPIspec[cEPPI].pEPPIcontrol) = 0x3020986c;                            // GP 2FS mode, DLEN 10bit, external trigger, Sample data on rising edge and sample/drive syncs on rising edge
                
                
            } else if(pa_pEPPIcameraInConfig->nDataInterfaceMode == CAM_16BIT_HVSYNC) {
                
                *(g_aEPPIspec[cEPPI].pDMA0nextDescrPtr) = (void *)&g_aEPPIcameraInDMAdescriptor[pEPPIcameraInSpec->nDescrIndOffset + 0];
                *(g_aEPPIspec[cEPPI].pDMA0xcount) = pa_pEPPIcameraInConfig->nXres;
                *(g_aEPPIspec[cEPPI].pDMA0xmodify) = 2;
                *(g_aEPPIspec[cEPPI].pDMA0ycount) = pa_pEPPIcameraInConfig->nYres;
                *(g_aEPPIspec[cEPPI].pDMA0ymodify) = 2;

                *g_aEPPIspec[cEPPI].pDMA0config = 0x74b6;                            // Descr list large, Descr size 4, Int enable, Syncronized transition, 2D, 16bit transfere, Memory write OP,
                                
                *(g_aEPPIspec[cEPPI].pEPPIframe) = pa_pEPPIcameraInConfig->nYres;
                //*(g_aEPPIspec[cEPPI].pEPPIhdelay) = 0;
                *(g_aEPPIspec[cEPPI].pEPPIline) = (pa_pEPPIcameraInConfig->nXres); // - 1;
                //*(g_aEPPIspec[cEPPI].pEPPIvdelay) = 0;
                *(g_aEPPIspec[cEPPI].pEPPIcontrol) = 0x3022182c;                            // GP 2FS mode, DLEN 16bit, external trigger, Sample data on rising edge and sample/drive syncs on rising edge
                
                
            }
            
            break;
        }
        
        default: {
            
            pa_pEPPIcameraInConfig->erError = ERR_EPPI_CAMERA_IN_BUFFER_MODE;
            EPPIcameraIn_close((T_EPPI_CAMERA_IN_HANDLE)pEPPIcameraInSpec);
            return 0;
        }
    }
    
    pa_pEPPIcameraInConfig->pFrameBuffer = pEPPIcameraInSpec->pFrameBuffer;
    
    if(!eppi_platformInit(cEPPI, *(g_aEPPIspec[cEPPI].pEPPIcontrol))) {
        pa_pEPPIcameraInConfig->erError = ERR_EPPI_CAMERA_IN_PLATFORM_INIT;
        EPPIcameraIn_close((T_EPPI_CAMERA_IN_HANDLE)pEPPIcameraInSpec);
        return 0;
    }
    
    // Hook the Interrupt
    if(pEPPIcameraInSpec->unPerIVG) {
            
        adi_int_SICSetIVG(g_aEPPIspec[cEPPI].peripheralIntId0, pEPPIcameraInSpec->unPerIVG);
    } else {
        adi_int_SICGetIVG(g_aEPPIspec[cEPPI].peripheralIntId0, &pEPPIcameraInSpec->unPerIVG);
    }
    
    if(adi_int_CECHook(pEPPIcameraInSpec->unPerIVG, EPPIcameraInInterruptHandler, (void *)pEPPIcameraInSpec, false) == ADI_INT_RESULT_SUCCESS) {
        adi_int_SICWakeup(g_aEPPIspec[cEPPI].peripheralIntId0, TRUE);
	    adi_int_SICEnable(g_aEPPIspec[cEPPI].peripheralIntId0);
	} else {
		pa_pEPPIcameraInConfig->erError = ERR_EPPI_CAMERA_IN_HOOK_INTERRUPT;
        EPPIcameraIn_close((T_EPPI_CAMERA_IN_HANDLE)pEPPIcameraInSpec);
        return 0;
    }

#ifdef _EPPI_CAMERA_IN_HOOK_ERROR_INT_
    // Hook the eppi Error Interrupt
    if(pEPPIcameraInSpec->unPerErrorIVG) {
        adi_int_SICSetIVG(g_aEPPIspec[cEPPI].perErrorIntId0, pEPPIcameraInSpec->unPerErrorIVG);
    } else {
        adi_int_SICGetIVG(g_aEPPIspec[cEPPI].perErrorIntId0, &pEPPIcameraInSpec->unPerErrorIVG);
    }
    
    if(adi_int_CECHook(pEPPIcameraInSpec->unPerErrorIVG, EPPIcameraInErrorInterruptHandler, (void *)pEPPIcameraInSpec, false) == ADI_INT_RESULT_SUCCESS) {
        adi_int_SICWakeup(g_aEPPIspec[cEPPI].perErrorIntId0, TRUE);
	    adi_int_SICEnable(g_aEPPIspec[cEPPI].perErrorIntId0);
	} else {
		pa_pEPPIcameraInConfig->erError = ERR_EPPI_CAMERA_IN_HOOK_INTERRUPT;
        EPPIcameraIn_close((T_EPPI_CAMERA_IN_HANDLE)pEPPIcameraInSpec);
        return 0;
    }
    
    
    // Hook the dma Error Interrupt
    if(pEPPIcameraInSpec->unDmaErrorIVG) {
        adi_int_SICSetIVG(g_aEPPIspec[cEPPI].DMAerrorIntId0, pEPPIcameraInSpec->unDmaErrorIVG);
    } else {
        adi_int_SICGetIVG(g_aEPPIspec[cEPPI].DMAerrorIntId0, &pEPPIcameraInSpec->unDmaErrorIVG);
    }
    
    if(adi_int_CECHook(pEPPIcameraInSpec->unDmaErrorIVG, EPPIcameraInDmaErrorInterruptHandler, (void *)pEPPIcameraInSpec, false) == ADI_INT_RESULT_SUCCESS) {
        adi_int_SICWakeup(g_aEPPIspec[cEPPI].DMAerrorIntId0, TRUE);
	    adi_int_SICEnable(g_aEPPIspec[cEPPI].DMAerrorIntId0);
	} else {
		pa_pEPPIcameraInConfig->erError = ERR_EPPI_CAMERA_IN_HOOK_INTERRUPT;
        EPPIcameraIn_close((T_EPPI_CAMERA_IN_HANDLE)pEPPIcameraInSpec);
        return 0;
    }
#endif
    
    
    return (T_EPPI_CAMERA_IN_HANDLE)pEPPIcameraInSpec;
}



void EPPIcameraIn_close(T_EPPI_CAMERA_IN_HANDLE pa_hEPPIcameraInHndl) {
    T_EPPI_CAMERA_IN_SPEC *stEPPIcamInSpec = (T_EPPI_CAMERA_IN_SPEC *)pa_hEPPIcameraInHndl; 

    unsigned short i;
    
    adi_int_CECUnhook(stEPPIcamInSpec->unPerIVG, EPPIcameraInInterruptHandler, (void *)stEPPIcamInSpec);

#ifdef _EPPI_CAMERA_IN_HOOK_ERROR_INT_
    adi_int_CECUnhook(stEPPIcamInSpec->unPerErrorIVG, EPPIcameraInErrorInterruptHandler, (void *)stEPPIcamInSpec);
    adi_int_CECUnhook(stEPPIcamInSpec->unPerIVG, EPPIcameraInInterruptHandler, (void *)stEPPIcamInSpec);
#endif
    
    for(i=0; i<stEPPIcamInSpec->nMaxFrames; i++) {
        if(stEPPIcamInSpec->pFrameBuffer[i]) {
            free(stEPPIcamInSpec->pFrameBuffer[i]);
        }
        if(stEPPIcamInSpec->pFrameDescr[i]) {
            free(stEPPIcamInSpec->pFrameDescr[i]);
        }
    }
    
    if(stEPPIcamInSpec->pFrameGrabberInterface) {
        free(stEPPIcamInSpec->pFrameGrabberInterface);
    }
    
    if(stEPPIcamInSpec->pFrameBuffer) {
        free(stEPPIcamInSpec->pFrameBuffer);
    }
    
    if(stEPPIcamInSpec) {
        stEPPIcamInSpec->pEPPIspec->bInUse = false;
        free((void *)stEPPIcamInSpec);
    }
}



void EPPIcameraIn_setup(void) {
	unsigned short i, iMax;
	
	iMax = EPPI_CAMERA_IN_MAX_NOF_DEVICES * FG_MAX_NOF_BUFFERED_FRAMES * EPPI_CAMERA_IN_DESCR_SIZE;
	
	for(i=0; i<iMax; i++) {
		g_aEPPIcameraInDMAdescriptor[i] = 0;
	}
	
	for(i=0; i<EPPI_CAMERA_IN_MAX_NOF_DEVICES; i++) {
		g_aEPPIcameraInHndlCount[i] = 0;
	}
}



void EPPIcameraIn_cleanup(void) {
	// nothing to do
}



// interface functions to the video grabber ------------------------------


/**
 *	@public
 *	@brief		Starts the EPPIcameraIn to capture frames
 *
 *	@param		pa_EPPIcameraInHandle	Handle to a EPPIcameraIn Resource
 *	
 *	@return		Returns an error code
 *	
 **/
T_ERROR_CODE EPPIcameraIn_start(T_EPPI_CAMERA_IN_HANDLE pa_EPPIcameraInHandle) {
    T_EPPI_CAMERA_IN_SPEC *pEPPIcameraInSpec = (T_EPPI_CAMERA_IN_SPEC *)pa_EPPIcameraInHandle;
    
    if(pEPPIcameraInSpec->bIsActive) {
        return ERR_EPPI_CAMERA_IN_ACTIVITY;
    }
    
#if _EPPI_CAMERA_IN_DEBUG_LVL_ > 0
        syslog_post("EPPIcameraIn started\n", 0, 0, NULL);
#endif

    *(pEPPIcameraInSpec->pEPPIspec->pDMA0config)    |= 0x0001;
	*(pEPPIcameraInSpec->pEPPIspec->pEPPIcontrol)   |= 0x0001;
	pEPPIcameraInSpec->bIsActive = true;
	
	return ERR_NONE;
}


/**
 *	@public
 *	@brief		Stops the EPPIcameraIn
 *
 *	@param		pa_EPPIcameraInHandle	Handle to a EPPIcameraIn Resource
 *	
 *	@return		Returns an error code
 *	
 **/
T_ERROR_CODE EPPIcameraIn_stop(T_EPPI_CAMERA_IN_HANDLE pa_EPPIcameraInHandle) {
    T_EPPI_CAMERA_IN_SPEC *pEPPIcameraInSpec = (T_EPPI_CAMERA_IN_SPEC *)pa_EPPIcameraInHandle;
    
    if(!pEPPIcameraInSpec->bIsActive) {
        return ERR_EPPI_CAMERA_IN_ACTIVITY;
    }
    
#if _EPPI_CAMERA_IN_DEBUG_LVL_ > 0
        syslog_post("EPPIcameraIn stopped\n", 0, 0, NULL);
#endif

    *(pEPPIcameraInSpec->pEPPIspec->pEPPIcontrol)  &= ~0x0001;
	*(pEPPIcameraInSpec->pEPPIspec->pDMA0config) 	&= ~0x0001; 
	pEPPIcameraInSpec->bIsActive = false;
	
	return ERR_NONE;
}


/**
 *	@public
 *	@brief		Gets the current FrameRate of the Camera
 *
 *	@param		pa_tHndl	Handle to a Camera Resource
 *	
 *	@return		Returns the FrameRate
 *	
 **/
float EPPIcameraIn_getFramerate(T_EPPI_CAMERA_IN_HANDLE pa_tHndl) {
	T_EPPI_CAMERA_IN_SPEC *pSpec = (T_EPPI_CAMERA_IN_SPEC *)pa_tHndl;
    return pSpec->fFramerate;
}


/**
 *	@public
 *	@brief		Resets the specified Camera Device
 *
 *	@param		pa_tHndl	Handle to a Camera Resource
 *	
 *	@return		ERR_NONE on sucess
 **/
T_ERROR_CODE EPPIcameraIn_resetDevice(T_EPPI_CAMERA_IN_HANDLE pa_tHndl) {
	T_EPPI_CAMERA_IN_SPEC *pSpec = (T_EPPI_CAMERA_IN_SPEC *)pa_tHndl;
    // reset buffer pointer for multi-buffering
    pSpec->nIndexIn = 0;
    pSpec->nIndexOut = 0;
    pSpec->nFramesLoaded = 0;
    pSpec->bDequeueInProgress = false;

    return ERR_NONE;
}


/**
 *	@public
 *	@brief		Configures the specified Camera resource
 *
 *				At the moment this configuration function isn't completely implemented, only the number of Frames
 *				to Hold argument from the T_FG_ARG pa_tARG is used to control the number of frames held.
 *
 *	@param		pa_tHndl	Handle to a Camera Resource
 *	@param		pa_tCmd		Camera Command (currently not implemented)
 *	@param		pa_tArg		Argument for command @see T_FG_ARG (can only use the pa_tArg.nNofFramesToHold element).
 *	
 *	@return		ERR_NONE on sucess.
 *	
 **/
 
 /* TODO: This function is incomplete, cannot configure the on-chip camera control registers */	
T_ERROR_CODE EPPIcameraIn_configDevice(T_EPPI_CAMERA_IN_HANDLE pa_tHndl, T_FG_CMD pa_tCmd, T_FG_ARG *pa_tArg) {
	T_EPPI_CAMERA_IN_SPEC *pSpec = (T_EPPI_CAMERA_IN_SPEC *)pa_tHndl;

	switch(pa_tCmd){
	    case FG_CMD_CLOSE:{
	        if(pSpec->fnHwDevCloseDev && pSpec->hHwDevHndl){
	            pSpec->fnHwDevCloseDev(pSpec->hHwDevHndl);
	        }
	        EPPIcameraIn_stop(pa_tHndl);
	        EPPIcameraIn_close(pa_tHndl);
	        break;
	    }
	    case FG_CMD_GET_DEVICE_NAME:{
	        sprintf(pa_tArg->cDeviceName, "EPPIcameraIn%d", pSpec->cEPPI);
	        break;
	    }
	    default:{
	        return ERR_GENERIC;
	    }
	}
	
    return ERR_NONE;
}


/**
 *	@public
 *	@brief		Returns the number of Frames waiting in the Buffer
 *
 *	@param		pa_tHndl	Handle to a Camera Resource
 *	
 *	@return		The number of frames waiting to be grabbed.
 *	
 **/
int EPPIcameraIn_framesAvailable(T_EPPI_CAMERA_IN_HANDLE pa_tHndl) {
	T_EPPI_CAMERA_IN_SPEC *pSpec = (T_EPPI_CAMERA_IN_SPEC *)pa_tHndl;
    return (int)pSpec->nFramesLoaded;
}


/**
 *	@public
 *	@brief		Returns a pointer to the next Frame.
 *
 *				Provides a pointer to a ifrm_t frame. @see ifrm_t.
 *
 *	@param		pa_tHndl	Handle to a Camera Resource
 *	
 *	@return		A valid frame pointer or a NULL pointer on error.
 *	
 **/
ifrm_t *EPPIcameraIn_getNextFrame(T_EPPI_CAMERA_IN_HANDLE pa_tHndl) {
	T_EPPI_CAMERA_IN_SPEC *pSpec = (T_EPPI_CAMERA_IN_SPEC *)pa_tHndl;
/*	
	if(!pSpec->nFramesLoaded && !pSpec->bIsActive) {
	    EPPIcameraInStart(pa_tHndl);	
	}
	
    while(!pSpec->nFramesLoaded) {
	    ;
	}
	
	
	tNextFramePtr.pucFramePtr = (unsigned char *)pSpec->pFrameBuffer[pSpec->nIndexOut];
	pSpec->nIndexOut++;
	
	if(pSpec->nIndexOut >= pSpec->nMaxFrames) {
	    pSpec->nIndexOut = 0;
	}
	
	pSpec->nFramesLoaded--;
	
	
	
	if(pSpec->nFramesLoaded < pSpec->nMaxFrames && !pSpec->bIsActive) {
	    EPPIcameraInStart(pa_tHndl);	
	}
	
	return stBufferStatus;*/
	
	
	if(pSpec->nFramesLoaded >= pSpec->nNofFramesToHold && !pSpec->bDequeueInProgress) {
	    pSpec->bDequeueInProgress = true;
	    return (pSpec->pFrameDescr[pSpec->nIndexOut]);
	} else {
	    return NULL;
	}
}


/**
 *	@public
 *	@brief		Frees a Frame from the Camera Frame Buffer
 *
 *	@param		pa_tHndl	Handle to a Camera Resource
 *	
 *	@return		ERR_NONE on sucess, ERR_GENERIC on failure.
 *	
 **/
T_ERROR_CODE EPPIcameraIn_freeFrame(T_EPPI_CAMERA_IN_HANDLE pa_tHndl) {
	T_EPPI_CAMERA_IN_SPEC *pSpec = (T_EPPI_CAMERA_IN_SPEC *)pa_tHndl;
	if (pSpec->bDequeueInProgress) {
		pSpec->nIndexOut ++;
		
		if (pSpec->nIndexOut == pSpec->nMaxFrames) {
		    pSpec->nIndexOut = 0;
		}
		
		pSpec->nFramesLoaded --;
		// enable EPPI because maybe it was blocked due a buffer queue overrun
	
		if (!pSpec->bIsActive) {
			EPPIcameraIn_start(pa_tHndl);
		}
		
		pSpec->bDequeueInProgress = false;
		
		return ERR_NONE;
	} else {
	    return ERR_GENERIC;
	}
}


/**
 *	@public
 *	@brief		Writes back a T_FG_VIDEO_DEVICE_INFO stucture to a provided buffer. @see T_FG_VIDEO_DEVICE_INFO 
 *
 *				
 *
 *	@param		pa_tHndl		Handle to a Camera Resource
 *	@param		pa_tInfoBlock	Provide the address of a T_FG_VIDEO_DEVICE_INFO structure.
 *	
 **/
void EPPIcameraIn_getDeviceInfo(T_EPPI_CAMERA_IN_HANDLE pa_tHndl, T_FG_VIDEO_DEVICE_INFO *pa_tInfoBlock) {
	T_EPPI_CAMERA_IN_SPEC *pSpec = (T_EPPI_CAMERA_IN_SPEC *)pa_tHndl;
    pa_tInfoBlock->tFramerate = (T_FG_FRAMERATE)pSpec->fFramerate;
    pa_tInfoBlock->nXres = pSpec->nXres;
    pa_tInfoBlock->nYres = pSpec->nYres;
//    pa_tInfoBlock->nFrameCounter = pSpec->nFrameCounter;
//    pa_tInfoBlock->nFrameDropCounter = pSpec->nFrameDropCounter;
    switch (pSpec->nColorMode) {
        case CAM_YUV_422:{
            pa_tInfoBlock->tColorMode = FG_YUV422;
        	break;
        }
        case CAM_RGB_565:{
            pa_tInfoBlock->tColorMode = FG_RGB565;
        	break;
        }
		default :{
		    pa_tInfoBlock->tColorMode = FG_COLOR_MODE_NOT_SUPPORTED;
		    
		}
    }
}
// -----------------------------------------------------------------------

