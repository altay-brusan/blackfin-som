/**
	@file PPIcameraIn.c
	@ingroup camera
	@brief Interface driver to configure the PPI for capturing images from a camera
	
	
	
	BLT_DISCLAIMER(TBD)
	@author Alexander Froemel
	@version 1.1
	@date 19.11.2009
	
	@startcond Changelog
	
	@endcond
**/

#include <environment.h>
#include <cplbtab.h>
#include <stdio.h>
#include <conio.h>
#include <cycle_count_bf.h>
// local include
#include "PPIcameraIn.h"

#ifdef PPI_CAM_IN_POST_ERROR_ON_SYSLOG
    #include "../../../../../blacksheep/common/syslog/syslog.h"
#endif

#ifdef _USE_VDK_
	#include <VDK.h>
#endif

extern unsigned int g_nPPIcount;	// Platform Specific in ppi_global, PPI count
extern T_PPI_SPEC 	g_aPPIspec[];	// Platform Specific in ppi_global, PPI specification array

T_PPI_CAMERA_IN_HANDLE g_aPPIcameraInHndlCount[PPI_CAMERA_IN_MAX_NOF_DEVICES];				// array of connected camera handles

#ifdef __ADSPBF561__
section("L1_data_a") unsigned long g_aPPIcameraInDMAdescriptor[PPI_CAMERA_IN_MAX_NOF_DEVICES * FG_MAX_NOF_BUFFERED_FRAMES * PPI_CAMERA_IN_DESCR_SIZE];          // descriptor for double buffer or multi frame buffer mode
#else
section("L1_data") unsigned long g_aPPIcameraInDMAdescriptor[PPI_CAMERA_IN_MAX_NOF_DEVICES * FG_MAX_NOF_BUFFERED_FRAMES * PPI_CAMERA_IN_DESCR_SIZE];          // descriptor for double buffer or multi frame buffer mode
#endif

unsigned long g_nDMAerrorCount = 0;
unsigned long g_nPPIerrorCountOther = 0;
unsigned long g_nPPIerrorCountFIFOov = 0;
unsigned long g_nPPIframesCaptured = 0;

#pragma section("L1_code")
static ADI_INT_HANDLER_RESULT PPIcameraInInterruptHandler(void *pa_pClientArg) {
    
    T_PPI_CAMERA_IN_SPEC *pPPIcameraIn = (T_PPI_CAMERA_IN_SPEC *)pa_pClientArg;		// Provide a pointer for the Argument to the defined T_PPI_SPEC structure
    
    if (*(pPPIcameraIn->pPPIspec->pDMAirqStat) & 0x1) {
                    
        *(pPPIcameraIn->pPPIspec->pDMAirqStat) = 0x1;					// clear the interrupt
        
        // for debug purpose
        // PPIcameraIn_stop((T_PPI_CAMERA_IN_HANDLE)pa_pClientArg);
            
        if(pPPIcameraIn->fnGetTimestamp) {
            pPPIcameraIn->pFrameDescr[pPPIcameraIn->nIndexIn]->ulTS = pPPIcameraIn->fnGetTimestamp();
        }
        
        pPPIcameraIn->nFramesLoaded++;
        pPPIcameraIn->nIndexIn++;
        if(pPPIcameraIn->nIndexIn >= pPPIcameraIn->nMaxFrames) {
            pPPIcameraIn->nIndexIn = 0;
        }
        g_nPPIframesCaptured++;
        
        switch(pPPIcameraIn->nBufferMode) {
            
            case PPI_CAMERA_IN_SINGLE_BUFFER: {
                                
                if(pPPIcameraIn->nFramesLoaded >= pPPIcameraIn->nMaxFrames) {
                    PPIcameraIn_stop((T_PPI_CAMERA_IN_HANDLE)pPPIcameraIn);          // has to be started by user when ready to receive next frame
                }
                
                break;
            }
            
            case PPI_CAMERA_IN_DOUBLE_BUFFER_NO_OVERWRITE: {
                
                if(pPPIcameraIn->nFramesLoaded >= pPPIcameraIn->nMaxFrames) {
                    PPIcameraIn_stop((T_PPI_CAMERA_IN_HANDLE)pPPIcameraIn);          // will be started if user dequeues the next frame
                    *(pPPIcameraIn->pPPIspec->pDMAnextDescrPtr) = (void *)g_aPPIcameraInDMAdescriptor[pPPIcameraIn->nDescrIndOffset + (pPPIcameraIn->nIndexIn * 2)];
                }
                
                break;
            }
                        
            case PPI_CAMERA_IN_MULTI_FRAME_BUFFER_NO_OVERWRITE: {
                                
                if(pPPIcameraIn->nFramesLoaded >= pPPIcameraIn->nMaxFrames) {
                    PPIcameraIn_stop((T_PPI_CAMERA_IN_HANDLE)pPPIcameraIn);          // is started if user requests for next frame is framebuffer is empty
                    *(pPPIcameraIn->pPPIspec->pDMAnextDescrPtr) = (void *)g_aPPIcameraInDMAdescriptor[pPPIcameraIn->nDescrIndOffset + (pPPIcameraIn->nIndexIn * 2)];
                }
                
                break;
            }
            
            default: {
                break;
            }
		}
		if (pPPIcameraIn->fnCallback) {							// Check if a Callback function was hooked, (check not NULL!)
            pPPIcameraIn->fnCallback(pPPIcameraIn->pCallbackArg);
        }
		        
        if( (__cplb_ctrl & CPLB_ENABLE_DCACHE ) || (__cplb_ctrl & CPLB_ENABLE_DCACHE2)){
            dcache_invalidate_both();                                   // invalidate cache because data is written by DMA
        }
        
		return ADI_INT_RESULT_PROCESSED;			// Interrupt processed, notify ADI's ISR
		
	} else {
		return ADI_INT_RESULT_NOT_PROCESSED;	// Interrupt wasn't for us, notify ADI's ISR that we didn't process it!
		
    }
}


#pragma section("L1_code")
static ADI_INT_HANDLER_RESULT PPIcameraInErrorInterruptHandler(void *pa_pClientArg) {
  
	T_PPI_CAMERA_IN_SPEC *pPPIcameraIn = (T_PPI_CAMERA_IN_SPEC *)pa_pClientArg;		// Provide a pointer for the parameter data to the defined T_PPI_SPEC structure.
	
	if (*(pPPIcameraIn->pPPIspec->pPPIstatus)) {
	    
#ifdef PPI_CAM_IN_POST_ERROR_ON_SYSLOG
        syslog_post ("(PPIcameraIn) ppi error interrupt", 0, *(pPPIcameraIn->pPPIspec->pPPIstatus), 0);
#endif
	    
	    // for debugging, counting of different errors
        if (*(pPPIcameraIn->pPPIspec->pPPIstatus) & 0x1000) {
            g_nPPIerrorCountFIFOov ++;
        } else {
            g_nPPIerrorCountOther ++;
        }
                
        // clear all
  	    *(pPPIcameraIn->pPPIspec->pPPIstatus) = 0xfb00;
  	    
  	    // restart dma and ppi
#ifdef PPI_CAM_IN_RESTART_ON_ERROR_INT
        PPIcameraIn_stop((T_PPI_CAMERA_IN_HANDLE)pPPIcameraIn);
        PPIcameraIn_start((T_PPI_CAMERA_IN_HANDLE)pPPIcameraIn);
#endif  	    
  	            
  	    return ADI_INT_RESULT_PROCESSED;
	}
	return ADI_INT_RESULT_NOT_PROCESSED;	// Interrupt wasn't processed, so notify ADI's ISR routine.
}


#pragma section("L1_code")
static ADI_INT_HANDLER_RESULT PPIcameraInDmaErrorInterruptHandler(void *pa_pClientArg) {
    T_PPI_CAMERA_IN_SPEC *pPPIcameraIn = (T_PPI_CAMERA_IN_SPEC *)pa_pClientArg;		// Provide a pointer for the parameter data to the defined T_PPI_SPEC structure.
    
    if (*(pPPIcameraIn->pPPIspec->pDMAirqStat) & 0x0002) {
        
#ifdef PPI_CAM_IN_POST_ERROR_ON_SYSLOG
        syslog_post ("(PPIcameraIn) dma error interrupt", 0, *(pPPIcameraIn->pPPIspec->pDMAirqStat), 0);
#endif
        
        g_nDMAerrorCount ++;
   		*(pPPIcameraIn->pPPIspec->pDMAirqStat) = 0x0002;					// clear the interrupt
   		
  	    // restart dma and ppi
#ifdef PPI_CAM_IN_RESTART_ON_ERROR_INT
        PPIcameraIn_stop((T_PPI_CAMERA_IN_HANDLE)pPPIcameraIn);
        PPIcameraIn_start((T_PPI_CAMERA_IN_HANDLE)pPPIcameraIn);
#endif  	    
   		
		return ADI_INT_RESULT_PROCESSED;
    }
        
    return ADI_INT_RESULT_NOT_PROCESSED;	// Interrupt wasn't processed, so notify ADI's ISR routine.
}


T_PPI_CAMERA_IN_HANDLE PPIcameraIn_open(T_PPI_CAMERA_IN_CONF *pa_pPPIcameraInConfig) {
    
    unsigned char cPPI = pa_pPPIcameraInConfig->cPPI;
    T_PPI_CAMERA_IN_SPEC *pPPIcameraInSpec = (T_PPI_CAMERA_IN_SPEC *)malloc(sizeof(T_PPI_CAMERA_IN_SPEC));
    unsigned short i;
    
    if(!pPPIcameraInSpec) {
        pa_pPPIcameraInConfig->erError = ERR_PPI_CAMERA_IN_NOT_ENOUGH_MEMORY;
        return 0;
    }
    
    for(i=0; i<=PPI_CAMERA_IN_MAX_NOF_DEVICES; i++) {
        if(!g_aPPIcameraInHndlCount[i]) {
            g_aPPIcameraInHndlCount[i] = (T_PPI_CAMERA_IN_HANDLE) pPPIcameraInSpec;
            pPPIcameraInSpec->cDeviceID = i;
            i = PPI_CAMERA_IN_MAX_NOF_DEVICES + 1;
        }
        
        if(i == PPI_CAMERA_IN_MAX_NOF_DEVICES) {
            pa_pPPIcameraInConfig->erError = ERR_PPI_CAMERA_IN_TOO_MANY_INSTANCES;
            return 0;
        }
    }
    
    pPPIcameraInSpec->nDescrIndOffset = (pPPIcameraInSpec->cDeviceID * FG_MAX_NOF_BUFFERED_FRAMES * PPI_CAMERA_IN_DESCR_SIZE);
    
    pPPIcameraInSpec->nBufferMode = pa_pPPIcameraInConfig->nBufferMode;
    pPPIcameraInSpec->fnCallback = pa_pPPIcameraInConfig->fnCallback;
    pPPIcameraInSpec->pCallbackArg = pa_pPPIcameraInConfig->pCallbackArg;
    pPPIcameraInSpec->fnErrorCallback = pa_pPPIcameraInConfig->fnErrorCallback;
    pPPIcameraInSpec->nDataInterfaceMode = pa_pPPIcameraInConfig->nDataInterfaceMode;
    pPPIcameraInSpec->cPPI = pa_pPPIcameraInConfig->cPPI;
    pPPIcameraInSpec->nMaxFrames = pa_pPPIcameraInConfig->nMaxFrames;
    pPPIcameraInSpec->nXres = pa_pPPIcameraInConfig->nXres;
    pPPIcameraInSpec->nYres = pa_pPPIcameraInConfig->nYres;
    pPPIcameraInSpec->fFramerate = pa_pPPIcameraInConfig->fFramerate;
    pPPIcameraInSpec->pPPIspec = &g_aPPIspec[cPPI];
    pPPIcameraInSpec->nFramesLoaded = 0;
    pPPIcameraInSpec->nIndexIn = 0;
    pPPIcameraInSpec->nIndexOut = 0;
    pPPIcameraInSpec->nColorMode = pa_pPPIcameraInConfig->nColorMode;
    pPPIcameraInSpec->fnGetTimestamp = pa_pPPIcameraInConfig->fnGetTimestamp;
    
    pPPIcameraInSpec->bIsActive = false;
    pPPIcameraInSpec->bDequeueInProgress = false;
    
    pPPIcameraInSpec->pFrameGrabberInterface = (void **)malloc(sizeof(unsigned long) * FG_MAX_VIDEO_DEVICE_FUNCTIONS);
    
    //initialize FrameGrabber interface
    pPPIcameraInSpec->pFrameGrabberInterface[FG_DEVICE_RESET] 			= (void *)PPIcameraIn_resetDevice;
	pPPIcameraInSpec->pFrameGrabberInterface[FG_FREE_FRAME] 			= (void *)PPIcameraIn_freeFrame;
	pPPIcameraInSpec->pFrameGrabberInterface[FG_FRAME_AVAILABLE] 		= (void *)PPIcameraIn_framesAvailable;
	pPPIcameraInSpec->pFrameGrabberInterface[FG_DEQUEUE_FRAME] 			= (void *)PPIcameraIn_getNextFrame;
	pPPIcameraInSpec->pFrameGrabberInterface[FG_GET_FRAMERATE] 			= (void *)PPIcameraIn_getFramerate;
	pPPIcameraInSpec->pFrameGrabberInterface[FG_GET_VIDEO_DEVICE_INFO] 	= (void *)PPIcameraIn_getDeviceInfo;
    pPPIcameraInSpec->pFrameGrabberInterface[FG_START_GRABBING]         = (void *)PPIcameraIn_startGrabbing;
    pPPIcameraInSpec->pFrameGrabberInterface[FG_STOP_GRABBING]          = (void *)PPIcameraIn_stopGrabbing;
	pPPIcameraInSpec->pFrameGrabberInterface[FG_CONFIG_DEVICE] 			= (void *)PPIcameraIn_configDevice;
	
	pa_pPPIcameraInConfig->pFrameGrabberInterface = pPPIcameraInSpec->pFrameGrabberInterface;
	
	//attach specified hardware, if handle or fn is 0 it is not used
    pPPIcameraInSpec->hHwDevHndl = pa_pPPIcameraInConfig->hHwDevHndl;
    pPPIcameraInSpec->fnHwDevCloseDev = (PPI_CAMERA_IN_FN_CLOSE_DEVICE)(pa_pPPIcameraInConfig->pHwDevCloseDev);
    
    pa_pPPIcameraInConfig->erError = ERR_NONE;
    
    // check if specified PPI port is within limits
    if(pa_pPPIcameraInConfig->cPPI >= g_nPPIcount) {
        pa_pPPIcameraInConfig->erError = ERR_PPI_CAMERA_IN_PPI_INDEX;
        
        for(i=0;i<PPI_CAMERA_IN_MAX_NOF_DEVICES;i++){
            if(g_aPPIcameraInHndlCount[i] == (T_PPI_CAMERA_IN_HANDLE)pPPIcameraInSpec){
                g_aPPIcameraInHndlCount[i] = 0;
                i=PPI_CAMERA_IN_MAX_NOF_DEVICES;
            }
        }
        
        free(pPPIcameraInSpec->pFrameGrabberInterface);
        free(pPPIcameraInSpec);
        return 0;
    }
    
    // check if specified PPI port is in use
    if(g_aPPIspec[cPPI].bInUse) {
        pa_pPPIcameraInConfig->erError = ERR_PPI_CAMERA_IN_ALREADY_IN_USE;
        
        for(i=0;i<PPI_CAMERA_IN_MAX_NOF_DEVICES;i++){
            if(g_aPPIcameraInHndlCount[i] == (T_PPI_CAMERA_IN_HANDLE)pPPIcameraInSpec){
                g_aPPIcameraInHndlCount[i] = 0;
                i=PPI_CAMERA_IN_MAX_NOF_DEVICES;
            }
        }
        
        free(pPPIcameraInSpec->pFrameGrabberInterface);
        free(pPPIcameraInSpec);
        return 0;
    }
    
    g_aPPIspec[cPPI].bInUse = true;
    
    // check for Data Interface Mode error
    if(pa_pPPIcameraInConfig->nDataInterfaceMode != CAM_8BIT_HVSYNC && pa_pPPIcameraInConfig->nDataInterfaceMode != CAM_10BIT_HVSYNC) {
        pa_pPPIcameraInConfig->erError = ERR_PPI_CAMERA_IN_DATA_INTERFACE_MODE;
        PPIcameraIn_close((T_PPI_CAMERA_IN_HANDLE)pPPIcameraInSpec);
        return 0;
    }

    // reset the frame queue    
    pPPIcameraInSpec->nIndexOut = 0;
    pPPIcameraInSpec->nIndexIn = 0;
    pPPIcameraInSpec->nFramesLoaded = 0;

    // configure PPI and DMA according to BufferMode and DataInterfaceMode
    switch(pa_pPPIcameraInConfig->nBufferMode) {
        
        case PPI_CAMERA_IN_SINGLE_BUFFER: {
            
            pPPIcameraInSpec->pFrameBuffer = (unsigned short **)malloc(sizeof(unsigned long) * 1);
            pPPIcameraInSpec->pFrameBuffer[0] = (unsigned short *)calloc(pa_pPPIcameraInConfig->nXres * pa_pPPIcameraInConfig->nYres, sizeof(unsigned short));
            pPPIcameraInSpec->pFrameDescr = (ifrm_t **)malloc(sizeof(unsigned long) * 1);
            pPPIcameraInSpec->pFrameDescr[0] = (ifrm_t *)malloc(sizeof(ifrm_t));
            
            if(!(pPPIcameraInSpec->pFrameBuffer) || !(pPPIcameraInSpec->pFrameBuffer[0]) || !(pPPIcameraInSpec->pFrameDescr) || !(pPPIcameraInSpec->pFrameDescr[0])) {
                pa_pPPIcameraInConfig->erError = ERR_PPI_CAMERA_IN_NOT_ENOUGH_MEMORY;
                PPIcameraIn_close((T_PPI_CAMERA_IN_HANDLE)pPPIcameraInSpec);
                return 0;
            }
            pPPIcameraInSpec->pFrameDescr[0]->pucFramePtr = (unsigned char *)pPPIcameraInSpec->pFrameBuffer[0];
            pPPIcameraInSpec->nMaxFrames = pa_pPPIcameraInConfig->nMaxFrames = 1;
            
            if(pPPIcameraInSpec->nDataInterfaceMode == CAM_8BIT_HVSYNC) {
                
                *(g_aPPIspec[cPPI].pPPIcount) = (pa_pPPIcameraInConfig->nXres * 2) - 1;
                *(g_aPPIspec[cPPI].pPPIdelay) = 0;
                *(g_aPPIspec[cPPI].pPPIframe) = pa_pPPIcameraInConfig->nYres;
#ifdef __ADSPBF561__
                *(g_aPPIspec[cPPI].pPPIcontrol) = 0x01ac;                           // non-ITU656 mode, 2 external framesyncs, packing enabled, 32bit mode, 
#else
                *(g_aPPIspec[cPPI].pPPIcontrol) = 0x00ac;                           // non-ITU656 mode, 2 external framesyncs, packing enabled
#endif

                *(g_aPPIspec[cPPI].pDMAstartaddr) = (void *)(pPPIcameraInSpec->pFrameBuffer[0]);
#ifdef __ADSPBF561__
                *(g_aPPIspec[cPPI].pDMAxcount) = pa_pPPIcameraInConfig->nXres/2;
                *(g_aPPIspec[cPPI].pDMAxmodify) = 4;
                *(g_aPPIspec[cPPI].pDMAycount) = pa_pPPIcameraInConfig->nYres;
                *(g_aPPIspec[cPPI].pDMAymodify) = 4;
                
                *(g_aPPIspec[cPPI].pDMAconfig) = 0x00ba;                            // Stop mode, Int enable, Syncronized transition, 2D, 32bit transfere
#else
                *(g_aPPIspec[cPPI].pDMAxcount) = pa_pPPIcameraInConfig->nXres;
                *(g_aPPIspec[cPPI].pDMAxmodify) = 2;
                *(g_aPPIspec[cPPI].pDMAycount) = pa_pPPIcameraInConfig->nYres;
                *(g_aPPIspec[cPPI].pDMAymodify) = 2;
                
                *(g_aPPIspec[cPPI].pDMAconfig) = 0x00b6;                            // Stop mode, Int enable, Syncronized transition, 2D, 16bit transfere
#endif
            
            } else if(pa_pPPIcameraInConfig->nDataInterfaceMode == CAM_10BIT_HVSYNC) {
                
#ifdef __ADSPBF561__
                *(g_aPPIspec[cPPI].pPPIcontrol) = 0x09ac;                           // non-ITU656 mode, 2 external framesyncs, packing enabled, 32bit mode
#else
                *(g_aPPIspec[cPPI].pPPIcontrol) = 0x082c;                           // non-ITU656 mode, 2 external framesyncs, 
#endif
                *(g_aPPIspec[cPPI].pPPIframe) = pa_pPPIcameraInConfig->nYres;
                *(g_aPPIspec[cPPI].pPPIcount) = pa_pPPIcameraInConfig->nXres - 1;
                *(g_aPPIspec[cPPI].pPPIdelay) = 0;
                
                *(g_aPPIspec[cPPI].pDMAstartaddr) = (void *)(pPPIcameraInSpec->pFrameBuffer[0]);
                
#ifdef __ADSPBF561__
                *(g_aPPIspec[cPPI].pDMAxcount) = pa_pPPIcameraInConfig->nXres / 2;
                *(g_aPPIspec[cPPI].pDMAxmodify) = 4;
                *(g_aPPIspec[cPPI].pDMAycount) = pa_pPPIcameraInConfig->nYres;
                *(g_aPPIspec[cPPI].pDMAymodify) = 4;
                

                *(g_aPPIspec[cPPI].pDMAconfig) = 0x00ba;                            // Stop mode, Int enable, Syncronized transition, 2D, 32bit transfere
#else
                *(g_aPPIspec[cPPI].pDMAxcount) = pa_pPPIcameraInConfig->nXres;
                *(g_aPPIspec[cPPI].pDMAxmodify) = 2;
                *(g_aPPIspec[cPPI].pDMAycount) = pa_pPPIcameraInConfig->nYres;
                *(g_aPPIspec[cPPI].pDMAymodify) = 2;
                

                *(g_aPPIspec[cPPI].pDMAconfig) = 0x00b6;                            // Stop mode, Int enable, Syncronized transition, 2D, 16bit transfere
#endif
            }
            
            break;
        }
        
        case PPI_CAMERA_IN_DOUBLE_BUFFER_OVERWRITE: {
            
            pPPIcameraInSpec->pFrameBuffer = (unsigned short **)malloc(sizeof(unsigned long) * 2);
            pPPIcameraInSpec->pFrameBuffer[0] = (unsigned short *)calloc(pa_pPPIcameraInConfig->nXres * pa_pPPIcameraInConfig->nYres, sizeof(unsigned short));
            pPPIcameraInSpec->pFrameBuffer[1] = (unsigned short *)calloc(pa_pPPIcameraInConfig->nXres * pa_pPPIcameraInConfig->nYres, sizeof(unsigned short));
            pPPIcameraInSpec->pFrameDescr = (ifrm_t **)malloc(sizeof(unsigned long) * 2);
            pPPIcameraInSpec->pFrameDescr[0] = (ifrm_t *)malloc(sizeof(ifrm_t));
            pPPIcameraInSpec->pFrameDescr[1] = (ifrm_t *)malloc(sizeof(ifrm_t));
           
            if( !(pPPIcameraInSpec->pFrameBuffer) || !(pPPIcameraInSpec->pFrameBuffer[0]) || !(pPPIcameraInSpec->pFrameBuffer[1]) 
                || !(pPPIcameraInSpec->pFrameDescr) || !(pPPIcameraInSpec->pFrameDescr[0]) || !(pPPIcameraInSpec->pFrameDescr[1])) {
                pa_pPPIcameraInConfig->erError = ERR_PPI_CAMERA_IN_NOT_ENOUGH_MEMORY;
                PPIcameraIn_close((T_PPI_CAMERA_IN_HANDLE)pPPIcameraInSpec);
                return 0;
            }
            pPPIcameraInSpec->pFrameDescr[0]->pucFramePtr = (unsigned char *)pPPIcameraInSpec->pFrameBuffer[0];
            pPPIcameraInSpec->pFrameDescr[1]->pucFramePtr = (unsigned char *)pPPIcameraInSpec->pFrameBuffer[1];
            
            pPPIcameraInSpec->nMaxFrames = pa_pPPIcameraInConfig->nMaxFrames = 2;
            
            g_aPPIcameraInDMAdescriptor[pPPIcameraInSpec->nDescrIndOffset + 0] = (unsigned long)&(g_aPPIcameraInDMAdescriptor[pPPIcameraInSpec->nDescrIndOffset + 2]);	            // Next Descriptor (Second half of this array)
			g_aPPIcameraInDMAdescriptor[pPPIcameraInSpec->nDescrIndOffset + 1] = (unsigned long)pPPIcameraInSpec->pFrameBuffer[pPPIcameraInSpec->nDescrIndOffset + 0];				// Start address for 1st buffer
			g_aPPIcameraInDMAdescriptor[pPPIcameraInSpec->nDescrIndOffset + 2] = (unsigned long)&(g_aPPIcameraInDMAdescriptor[pPPIcameraInSpec->nDescrIndOffset + 0]);	            // Next Descriptor (First half of this array)
			g_aPPIcameraInDMAdescriptor[pPPIcameraInSpec->nDescrIndOffset + 3] = (unsigned long)pPPIcameraInSpec->pFrameBuffer[pPPIcameraInSpec->nDescrIndOffset + 1];				// Start address for 2nd buffer
			
			if(pa_pPPIcameraInConfig->nDataInterfaceMode == CAM_8BIT_HVSYNC) {
                
#ifdef __ADSPBF561__
                *(g_aPPIspec[cPPI].pPPIcontrol) = 0x01ac;                           // non-ITU656 mode, 2 external framesyncs, packing enabled, 32bit mode, 
#else
                *(g_aPPIspec[cPPI].pPPIcontrol) = 0x00ac;                           // non-ITU656 mode, 2 external framesyncs, packing enabled
#endif
                *(g_aPPIspec[cPPI].pPPIframe) = (pa_pPPIcameraInConfig->nYres);
                *(g_aPPIspec[cPPI].pPPIdelay) = 0;
                *(g_aPPIspec[cPPI].pPPIcount) = (pa_pPPIcameraInConfig->nXres * 2) - 1;
                
                *(g_aPPIspec[cPPI].pDMAnextDescrPtr) = (void *)&g_aPPIcameraInDMAdescriptor[pPPIcameraInSpec->nDescrIndOffset + 0];
#ifdef __ADSPBF561__
                *(g_aPPIspec[cPPI].pDMAxcount) = pa_pPPIcameraInConfig->nXres/2;
                *(g_aPPIspec[cPPI].pDMAxmodify) = 4;
                *(g_aPPIspec[cPPI].pDMAycount) = pa_pPPIcameraInConfig->nYres;
                *(g_aPPIspec[cPPI].pDMAymodify) = 4;
                
                *(g_aPPIspec[cPPI].pDMAconfig) = 0x74ba;                            // Descr list large, Descr size 4, Int enable, Syncronized transition, 2D, 32bit transfere, Memory write OP,
#else
                *(g_aPPIspec[cPPI].pDMAxcount) = pa_pPPIcameraInConfig->nXres;
                *(g_aPPIspec[cPPI].pDMAxmodify) = 2;
                *(g_aPPIspec[cPPI].pDMAycount) = pa_pPPIcameraInConfig->nYres;
                *(g_aPPIspec[cPPI].pDMAymodify) = 2;
                
                *(g_aPPIspec[cPPI].pDMAconfig) = 0x74b6;                            // Descr list large, Descr size 4, Int enable, Syncronized transition, 2D, 16bit transfere, Memory write OP,
#endif
            
            } else if(pa_pPPIcameraInConfig->nDataInterfaceMode == CAM_10BIT_HVSYNC) {
                
#ifdef __ADSPBF561__
                *(g_aPPIspec[cPPI].pPPIcontrol) = 0x09ac;                           // non-ITU656 mode, 2 external framesyncs, packing enabled, 32bit mode
#else
                *(g_aPPIspec[cPPI].pPPIcontrol) = 0x082c;                           // non-ITU656 mode, 2 external framesyncs, 
#endif
                *(g_aPPIspec[cPPI].pPPIframe) = pa_pPPIcameraInConfig->nYres;
                *(g_aPPIspec[cPPI].pPPIcount) = pa_pPPIcameraInConfig->nXres - 1;
                
                *(g_aPPIspec[cPPI].pDMAnextDescrPtr) = (void *)&g_aPPIcameraInDMAdescriptor[pPPIcameraInSpec->nDescrIndOffset + 0];
                
#ifdef __ADSPBF561__
                *(g_aPPIspec[cPPI].pDMAxcount) = pa_pPPIcameraInConfig->nXres/2;
                *(g_aPPIspec[cPPI].pDMAxmodify) = 4;
                *(g_aPPIspec[cPPI].pDMAycount) = pa_pPPIcameraInConfig->nYres;
                *(g_aPPIspec[cPPI].pDMAymodify) = 4;
                
                *(g_aPPIspec[cPPI].pDMAconfig) = 0x74ba;                            // Descr list large, Descr size 4, Int enable, Syncronized transition, 2D, 32bit transfere, Memory write OP,
#else
                *(g_aPPIspec[cPPI].pDMAxcount) = pa_pPPIcameraInConfig->nXres;
                *(g_aPPIspec[cPPI].pDMAxmodify) = 2;
                *(g_aPPIspec[cPPI].pDMAycount) = pa_pPPIcameraInConfig->nYres;
                *(g_aPPIspec[cPPI].pDMAymodify) = 2;
                
                *(g_aPPIspec[cPPI].pDMAconfig) = 0x74b6;                            // Descr list large, Descr size 4, Int enable, Syncronized transition, 2D, 16bit transfere, Memory write OP,
#endif
            }
			
            break;
        }
        
        case PPI_CAMERA_IN_DOUBLE_BUFFER_NO_OVERWRITE: {
            
            pPPIcameraInSpec->pFrameBuffer = (unsigned short **)malloc(sizeof(unsigned long) * 2);
            pPPIcameraInSpec->pFrameBuffer[0] = (unsigned short *)calloc(pa_pPPIcameraInConfig->nXres * pa_pPPIcameraInConfig->nYres, sizeof(unsigned short));
            pPPIcameraInSpec->pFrameBuffer[1] = (unsigned short *)calloc(pa_pPPIcameraInConfig->nXres * pa_pPPIcameraInConfig->nYres, sizeof(unsigned short));
            pPPIcameraInSpec->pFrameDescr = (ifrm_t **)malloc(sizeof(unsigned long) * 2);
            pPPIcameraInSpec->pFrameDescr[0] = (ifrm_t *)malloc(sizeof(ifrm_t));
            pPPIcameraInSpec->pFrameDescr[1] = (ifrm_t *)malloc(sizeof(ifrm_t));
            
            if( !(pPPIcameraInSpec->pFrameBuffer) || !(pPPIcameraInSpec->pFrameBuffer[0]) || !(pPPIcameraInSpec->pFrameBuffer[1]) 
                || !(pPPIcameraInSpec->pFrameDescr) || !(pPPIcameraInSpec->pFrameDescr[0]) || !(pPPIcameraInSpec->pFrameDescr[1])) {
                pa_pPPIcameraInConfig->erError = ERR_PPI_CAMERA_IN_NOT_ENOUGH_MEMORY;
                PPIcameraIn_close((T_PPI_CAMERA_IN_HANDLE)pPPIcameraInSpec);
                return 0;
            }
            pPPIcameraInSpec->pFrameDescr[0]->pucFramePtr = (unsigned char *)pPPIcameraInSpec->pFrameBuffer[0];
            pPPIcameraInSpec->pFrameDescr[1]->pucFramePtr = (unsigned char *)pPPIcameraInSpec->pFrameBuffer[1];
            
            pPPIcameraInSpec->nMaxFrames = pa_pPPIcameraInConfig->nMaxFrames = 2;
            
            g_aPPIcameraInDMAdescriptor[pPPIcameraInSpec->nDescrIndOffset + 0] = (unsigned long)&g_aPPIcameraInDMAdescriptor[pPPIcameraInSpec->nDescrIndOffset + 2];	            // Next Descriptor (Second half of this array)
			g_aPPIcameraInDMAdescriptor[pPPIcameraInSpec->nDescrIndOffset + 1] = (unsigned long)pPPIcameraInSpec->pFrameBuffer[0];				// Start address for 1st buffer
			g_aPPIcameraInDMAdescriptor[pPPIcameraInSpec->nDescrIndOffset + 2] = (unsigned long)&g_aPPIcameraInDMAdescriptor[pPPIcameraInSpec->nDescrIndOffset + 0];	            // Next Descriptor (First half of this array)
			g_aPPIcameraInDMAdescriptor[pPPIcameraInSpec->nDescrIndOffset + 3] = (unsigned long)pPPIcameraInSpec->pFrameBuffer[1];				// Start address for 2nd buffer
			
			if(pa_pPPIcameraInConfig->nDataInterfaceMode == CAM_8BIT_HVSYNC) {
                
#ifdef __ADSPBF561__
                *(g_aPPIspec[cPPI].pPPIcontrol) = 0x01ac;                           // non-ITU656 mode, 2 external framesyncs, packing enabled, 32bit mode, 
#else
                *(g_aPPIspec[cPPI].pPPIcontrol) = 0x00ac;                           // non-ITU656 mode, 2 external framesyncs, packing enabled
#endif
                *(g_aPPIspec[cPPI].pPPIframe) = (pa_pPPIcameraInConfig->nYres);
                *(g_aPPIspec[cPPI].pPPIdelay) = 0;
                *(g_aPPIspec[cPPI].pPPIcount) = (pa_pPPIcameraInConfig->nXres * 2) - 1;
                
                *(g_aPPIspec[cPPI].pDMAnextDescrPtr) = (void *)&g_aPPIcameraInDMAdescriptor[pPPIcameraInSpec->nDescrIndOffset + 0];
#ifdef __ADSPBF561__
                *(g_aPPIspec[cPPI].pDMAxcount) = pa_pPPIcameraInConfig->nXres/2;
                *(g_aPPIspec[cPPI].pDMAxmodify) = 4;
                *(g_aPPIspec[cPPI].pDMAycount) = pa_pPPIcameraInConfig->nYres;
                *(g_aPPIspec[cPPI].pDMAymodify) = 4;
                
                *(g_aPPIspec[cPPI].pDMAconfig) = 0x74ba;                            // Descr list large, Descr size 4, Int enable, Syncronized transition, 2D, 32bit transfere, Memory write OP,
#else
                *(g_aPPIspec[cPPI].pDMAxcount) = pa_pPPIcameraInConfig->nXres;
                *(g_aPPIspec[cPPI].pDMAxmodify) = 2;
                *(g_aPPIspec[cPPI].pDMAycount) = pa_pPPIcameraInConfig->nYres;
                *(g_aPPIspec[cPPI].pDMAymodify) = 2;
                
                *(g_aPPIspec[cPPI].pDMAconfig) = 0x74b6;                            // Descr list large, Descr size 4, Int enable, Syncronized transition, 2D, 16bit transfere, Memory write OP,
#endif
            
            } else if(pa_pPPIcameraInConfig->nDataInterfaceMode == CAM_10BIT_HVSYNC) {
                
#ifdef __ADSPBF561__
                *(g_aPPIspec[cPPI].pPPIcontrol) = 0x09ac;                           // non-ITU656 mode, 2 external framesyncs, packing enabled, 32bit mode
#else
                *(g_aPPIspec[cPPI].pPPIcontrol) = 0x082c;                           // non-ITU656 mode, 2 external framesyncs, 
#endif
                *(g_aPPIspec[cPPI].pPPIframe) = pa_pPPIcameraInConfig->nYres;
                *(g_aPPIspec[cPPI].pPPIcount) = pa_pPPIcameraInConfig->nXres - 1;
                
                *(g_aPPIspec[cPPI].pDMAnextDescrPtr) = (void *)&g_aPPIcameraInDMAdescriptor[pPPIcameraInSpec->nDescrIndOffset + 0];
                
#ifdef __ADSPBF561__
                *(g_aPPIspec[cPPI].pDMAxcount) = pa_pPPIcameraInConfig->nXres/2;
                *(g_aPPIspec[cPPI].pDMAxmodify) = 4;
                *(g_aPPIspec[cPPI].pDMAycount) = pa_pPPIcameraInConfig->nYres;
                *(g_aPPIspec[cPPI].pDMAymodify) = 4;
                
                *(g_aPPIspec[cPPI].pDMAconfig) = 0x74ba;                            // Descr list large, Descr size 4, Int enable, Syncronized transition, 2D, 32bit transfere, Memory write OP,
#else
                *(g_aPPIspec[cPPI].pDMAxcount) = pa_pPPIcameraInConfig->nXres;
                *(g_aPPIspec[cPPI].pDMAxmodify) = 2;
                *(g_aPPIspec[cPPI].pDMAycount) = pa_pPPIcameraInConfig->nYres;
                *(g_aPPIspec[cPPI].pDMAymodify) = 2;
                
                *(g_aPPIspec[cPPI].pDMAconfig) = 0x74b6;                            // Descr list large, Descr size 4, Int enable, Syncronized transition, 2D, 16bit transfere, Memory write OP,
#endif
            }
			
            break;
        }
        
        case PPI_CAMERA_IN_MULTI_FRAME_BUFFER_OVERWRITE: {
            
            unsigned short nMaxFrames = 0;                                                   // calculate the max. possible frames
            unsigned short i;
            unsigned short iMax;
            
			if(pa_pPPIcameraInConfig->cSDRAMsize){
				nMaxFrames = (pa_pPPIcameraInConfig->cSDRAMsize << 20) / (pa_pPPIcameraInConfig->nXres * pa_pPPIcameraInConfig->nYres * 2);
			}
            
            if(nMaxFrames && nMaxFrames < pa_pPPIcameraInConfig->nMaxFrames) {
                pa_pPPIcameraInConfig->nMaxFrames = nMaxFrames;
            }
            
            pPPIcameraInSpec->pFrameBuffer = (unsigned short **)malloc(sizeof(unsigned short *) * pa_pPPIcameraInConfig->nMaxFrames);
            pPPIcameraInSpec->pFrameDescr = (ifrm_t **)malloc(sizeof(ifrm_t *) * pa_pPPIcameraInConfig->nMaxFrames);
            
            if(!(pPPIcameraInSpec->pFrameBuffer) || !(pPPIcameraInSpec->pFrameDescr)) {
                pa_pPPIcameraInConfig->erError = ERR_PPI_CAMERA_IN_NOT_ENOUGH_MEMORY;
                PPIcameraIn_close((T_PPI_CAMERA_IN_HANDLE)pPPIcameraInSpec);
                return 0;
            }
            
            for(i=0; i<pa_pPPIcameraInConfig->nMaxFrames; i++) {
                pPPIcameraInSpec->pFrameBuffer[i] = (unsigned short *)calloc(pa_pPPIcameraInConfig->nXres * pa_pPPIcameraInConfig->nYres, sizeof(unsigned short));
                pPPIcameraInSpec->pFrameDescr[i] = (ifrm_t *)malloc(sizeof(ifrm_t));
                
                if(!(pPPIcameraInSpec->pFrameBuffer[i]) || !(pPPIcameraInSpec->pFrameDescr[i])) {
                    pa_pPPIcameraInConfig->erError = ERR_PPI_CAMERA_IN_NOT_ENOUGH_MEMORY;
                    PPIcameraIn_close((T_PPI_CAMERA_IN_HANDLE)pPPIcameraInSpec);
                    return 0;
                }
                pPPIcameraInSpec->pFrameDescr[i]->pucFramePtr = (unsigned char *)pPPIcameraInSpec->pFrameBuffer[i];
            }
            
            iMax = pa_pPPIcameraInConfig->nMaxFrames * 2;
            
            for(i=0; i<iMax; i+=2) {
                if((i+2) >= iMax) {
                    g_aPPIcameraInDMAdescriptor[pPPIcameraInSpec->nDescrIndOffset + i] = (unsigned long)&g_aPPIcameraInDMAdescriptor[pPPIcameraInSpec->nDescrIndOffset + 0];
                } else {
                    g_aPPIcameraInDMAdescriptor[pPPIcameraInSpec->nDescrIndOffset + i] = (unsigned long)&g_aPPIcameraInDMAdescriptor[pPPIcameraInSpec->nDescrIndOffset + i + 2];
                }
                
                g_aPPIcameraInDMAdescriptor[pPPIcameraInSpec->nDescrIndOffset + i + 1] = (unsigned long)pPPIcameraInSpec->pFrameBuffer[i>>1];
            }
            
            if(pa_pPPIcameraInConfig->nDataInterfaceMode == CAM_8BIT_HVSYNC) {
                
#ifdef __ADSPBF561__
                *(g_aPPIspec[cPPI].pPPIcontrol) = 0x01ac;                           // non-ITU656 mode, 2 external framesyncs, packing enabled, 32bit mode, 
#else
                *(g_aPPIspec[cPPI].pPPIcontrol) = 0x00ac;                           // non-ITU656 mode, 2 external framesyncs, packing enabled
#endif
                *(g_aPPIspec[cPPI].pPPIframe) = (pa_pPPIcameraInConfig->nYres);
                *(g_aPPIspec[cPPI].pPPIdelay) = 0;
                *(g_aPPIspec[cPPI].pPPIcount) = (pa_pPPIcameraInConfig->nXres << 1) - 1;
                
                *(g_aPPIspec[cPPI].pDMAnextDescrPtr) = (void *)&g_aPPIcameraInDMAdescriptor[pPPIcameraInSpec->nDescrIndOffset + 0];
#ifdef __ADSPBF561__
                *(g_aPPIspec[cPPI].pDMAxcount) = pa_pPPIcameraInConfig->nXres>>1;
                *(g_aPPIspec[cPPI].pDMAxmodify) = 4;
                *(g_aPPIspec[cPPI].pDMAycount) = pa_pPPIcameraInConfig->nYres;
                *(g_aPPIspec[cPPI].pDMAymodify) = 4;
                
                *(g_aPPIspec[cPPI].pDMAconfig) = 0x74ba;                            // Descr list large, Descr size 4, Int enable, Syncronized transition, 2D, 32bit transfere, Memory write OP,
#else
                *(g_aPPIspec[cPPI].pDMAxcount) = pa_pPPIcameraInConfig->nXres;
                *(g_aPPIspec[cPPI].pDMAxmodify) = 2;
                *(g_aPPIspec[cPPI].pDMAycount) = pa_pPPIcameraInConfig->nYres;
                *(g_aPPIspec[cPPI].pDMAymodify) = 2;
                
                *(g_aPPIspec[cPPI].pDMAconfig) = 0x74b6;                            // Descr list large, Descr size 4, Int enable, Syncronized transition, 2D, 16bit transfere, Memory write OP,
#endif
            
            } else if(pa_pPPIcameraInConfig->nDataInterfaceMode == CAM_10BIT_HVSYNC) {
                
#ifdef __ADSPBF561__
                *(g_aPPIspec[cPPI].pPPIcontrol) = 0x09ac;                           // non-ITU656 mode, 2 external framesyncs, packing enabled, 32bit mode
#else
                *(g_aPPIspec[cPPI].pPPIcontrol) = 0x082c;                           // non-ITU656 mode, 2 external framesyncs, 
#endif
                *(g_aPPIspec[cPPI].pPPIframe) = pa_pPPIcameraInConfig->nYres;
                *(g_aPPIspec[cPPI].pPPIcount) = pa_pPPIcameraInConfig->nXres - 1;
                
                *(g_aPPIspec[cPPI].pDMAnextDescrPtr) = (void *)&g_aPPIcameraInDMAdescriptor[pPPIcameraInSpec->nDescrIndOffset + 0];
                
#ifdef __ADSPBF561__
                *(g_aPPIspec[cPPI].pDMAxcount) = pa_pPPIcameraInConfig->nXres>>1;
                *(g_aPPIspec[cPPI].pDMAxmodify) = 4;
                *(g_aPPIspec[cPPI].pDMAycount) = pa_pPPIcameraInConfig->nYres;
                *(g_aPPIspec[cPPI].pDMAymodify) = 4;
                
                *(g_aPPIspec[cPPI].pDMAconfig) = 0x74ba;                            // Descr list large, Descr size 4, Int enable, Syncronized transition, 2D, 32bit transfere, Memory write OP,
#else
                *(g_aPPIspec[cPPI].pDMAxcount) = pa_pPPIcameraInConfig->nXres;
                *(g_aPPIspec[cPPI].pDMAxmodify) = 2;
                *(g_aPPIspec[cPPI].pDMAycount) = pa_pPPIcameraInConfig->nYres;
                *(g_aPPIspec[cPPI].pDMAymodify) = 2;
                
                *(g_aPPIspec[cPPI].pDMAconfig) = 0x74b6;                            // Descr list large, Descr size 4, Int enable, Syncronized transition, 2D, 16bit transfere, Memory write OP,
#endif
            }
            
            break;
        }
        
        case PPI_CAMERA_IN_MULTI_FRAME_BUFFER_NO_OVERWRITE: {
            
            unsigned short nMaxFrames = 0;                                                   // calculate the max. possible frames
            unsigned short i;
            unsigned short iMax = pa_pPPIcameraInConfig->nMaxFrames * 2;
			
            if(pa_pPPIcameraInConfig->cSDRAMsize){
				nMaxFrames = (pa_pPPIcameraInConfig->cSDRAMsize << 20) / (pa_pPPIcameraInConfig->nXres * pa_pPPIcameraInConfig->nYres * 2);
			}
            
            if(nMaxFrames && nMaxFrames < pa_pPPIcameraInConfig->nMaxFrames) {
                pa_pPPIcameraInConfig->nMaxFrames = nMaxFrames;
            }
            
            pPPIcameraInSpec->pFrameBuffer = (unsigned short **)malloc(sizeof(unsigned short *) * pa_pPPIcameraInConfig->nMaxFrames);
            pPPIcameraInSpec->pFrameDescr = (ifrm_t **)malloc(sizeof(ifrm_t *) * pa_pPPIcameraInConfig->nMaxFrames);
            
            if(!(pPPIcameraInSpec->pFrameBuffer) ||  !(pPPIcameraInSpec->pFrameDescr)) {
                pa_pPPIcameraInConfig->erError = ERR_PPI_CAMERA_IN_NOT_ENOUGH_MEMORY;
                PPIcameraIn_close((T_PPI_CAMERA_IN_HANDLE)pPPIcameraInSpec);
                return 0;
            }
            
            for(i=0; i<pa_pPPIcameraInConfig->nMaxFrames; i++) {
                pPPIcameraInSpec->pFrameBuffer[i] = (unsigned short *)calloc(pa_pPPIcameraInConfig->nXres * pa_pPPIcameraInConfig->nYres, sizeof(unsigned short));
                //pPPIcameraInSpec->pFrameBuffer[i] = (unsigned short *)malloc(pa_pPPIcameraInConfig->nXres * pa_pPPIcameraInConfig->nYres * sizeof(unsigned short));
                pPPIcameraInSpec->pFrameDescr[i] = (ifrm_t *)malloc(sizeof(ifrm_t));
                
                if(!(pPPIcameraInSpec->pFrameBuffer[i]) || !(pPPIcameraInSpec->pFrameDescr[i])) {
                    pa_pPPIcameraInConfig->erError = ERR_PPI_CAMERA_IN_NOT_ENOUGH_MEMORY;
                    PPIcameraIn_close((T_PPI_CAMERA_IN_HANDLE)pPPIcameraInSpec);
                    return 0;
                }
                pPPIcameraInSpec->pFrameDescr[i]->pucFramePtr = (unsigned char *)pPPIcameraInSpec->pFrameBuffer[i];
            }
            
            for(i=0; i<iMax; i+=2) {
                if((i+2) >= iMax) {
                    g_aPPIcameraInDMAdescriptor[pPPIcameraInSpec->nDescrIndOffset + i] = (unsigned long)&g_aPPIcameraInDMAdescriptor[pPPIcameraInSpec->nDescrIndOffset + 0];
                } else {
                    g_aPPIcameraInDMAdescriptor[pPPIcameraInSpec->nDescrIndOffset + i] = (unsigned long)&g_aPPIcameraInDMAdescriptor[pPPIcameraInSpec->nDescrIndOffset + i + 2];
                }
                
                g_aPPIcameraInDMAdescriptor[pPPIcameraInSpec->nDescrIndOffset + i + 1] = (unsigned long)pPPIcameraInSpec->pFrameBuffer[i>>1];
            }
            
            if(pa_pPPIcameraInConfig->nDataInterfaceMode == CAM_8BIT_HVSYNC) {
                
#ifdef __ADSPBF561__
                *(g_aPPIspec[cPPI].pPPIcontrol) = 0x01ac;                           // non-ITU656 mode, 2 external framesyncs, packing enabled, 32bit mode, 
#else
                *(g_aPPIspec[cPPI].pPPIcontrol) = 0x00ac;                           // non-ITU656 mode, 2 external framesyncs, packing enabled
#endif
                *(g_aPPIspec[cPPI].pPPIframe) = (pa_pPPIcameraInConfig->nYres);
                *(g_aPPIspec[cPPI].pPPIdelay) = 0;
                *(g_aPPIspec[cPPI].pPPIcount) = (pa_pPPIcameraInConfig->nXres << 1) - 1;
                
                *(g_aPPIspec[cPPI].pDMAnextDescrPtr) = (void *)&g_aPPIcameraInDMAdescriptor[pPPIcameraInSpec->nDescrIndOffset + 0];
#ifdef __ADSPBF561__
                *(g_aPPIspec[cPPI].pDMAxcount) = pa_pPPIcameraInConfig->nXres>>1;
                *(g_aPPIspec[cPPI].pDMAxmodify) = 4;
                *(g_aPPIspec[cPPI].pDMAycount) = pa_pPPIcameraInConfig->nYres;
                *(g_aPPIspec[cPPI].pDMAymodify) = 4;
                
                *(g_aPPIspec[cPPI].pDMAconfig) = 0x74ba;                            // Descr list large, Descr size 4, Int enable, Syncronized transition, 2D, 32bit transfere, Memory write OP,
#else
                *(g_aPPIspec[cPPI].pDMAxcount) = pa_pPPIcameraInConfig->nXres;
                *(g_aPPIspec[cPPI].pDMAxmodify) = 2;
                *(g_aPPIspec[cPPI].pDMAycount) = pa_pPPIcameraInConfig->nYres;
                *(g_aPPIspec[cPPI].pDMAymodify) = 2;
                
                *(g_aPPIspec[cPPI].pDMAconfig) = 0x74b6;                            // Descr list large, Descr size 4, Int enable, Syncronized transition, 2D, 16bit transfere, Memory write OP,
#endif
            
            } else if(pa_pPPIcameraInConfig->nDataInterfaceMode == CAM_10BIT_HVSYNC) {
                
#ifdef __ADSPBF561__
                *(g_aPPIspec[cPPI].pPPIcontrol) = 0x09ac;                           // non-ITU656 mode, 2 external framesyncs, packing enabled, 32bit mode
#else
                *(g_aPPIspec[cPPI].pPPIcontrol) = 0x082c;                           // non-ITU656 mode, 2 external framesyncs, 
#endif
                *(g_aPPIspec[cPPI].pPPIframe) = pa_pPPIcameraInConfig->nYres;
                *(g_aPPIspec[cPPI].pPPIcount) = pa_pPPIcameraInConfig->nXres - 1;
                
                *(g_aPPIspec[cPPI].pDMAnextDescrPtr) = (void *)&g_aPPIcameraInDMAdescriptor[pPPIcameraInSpec->nDescrIndOffset + 0];
                
#ifdef __ADSPBF561__
                *(g_aPPIspec[cPPI].pDMAxcount) = pa_pPPIcameraInConfig->nXres>>1;
                *(g_aPPIspec[cPPI].pDMAxmodify) = 4;
                *(g_aPPIspec[cPPI].pDMAycount) = pa_pPPIcameraInConfig->nYres;
                *(g_aPPIspec[cPPI].pDMAymodify) = 4;
                
                *(g_aPPIspec[cPPI].pDMAconfig) = 0x74ba;                            // Descr list large, Descr size 4, Int enable, Syncronized transition, 2D, 32bit transfere, Memory write OP,
#else
                *(g_aPPIspec[cPPI].pDMAxcount) = pa_pPPIcameraInConfig->nXres;
                *(g_aPPIspec[cPPI].pDMAxmodify) = 2;
                *(g_aPPIspec[cPPI].pDMAycount) = pa_pPPIcameraInConfig->nYres;
                *(g_aPPIspec[cPPI].pDMAymodify) = 2;
                
                *(g_aPPIspec[cPPI].pDMAconfig) = 0x74b6;                            // Descr list large, Descr size 4, Int enable, Syncronized transition, 2D, 16bit transfere, Memory write OP,
#endif
            }
            
            break;
        }
        
        default: {
            
            pa_pPPIcameraInConfig->erError = ERR_PPI_CAMERA_IN_BUFFER_MODE;
            PPIcameraIn_close((T_PPI_CAMERA_IN_HANDLE)pPPIcameraInSpec);
            return 0;
        }
    }
    
    pa_pPPIcameraInConfig->pFrameBuffer = pPPIcameraInSpec->pFrameBuffer;
    pPPIcameraInSpec->nMaxFrames = pa_pPPIcameraInConfig->nMaxFrames;
    
    if(!ppi_platformInit(cPPI, *(g_aPPIspec[cPPI].pPPIcontrol))) {
        pa_pPPIcameraInConfig->erError = ERR_PPI_CAMERA_IN_PLATFORM_INIT;
        PPIcameraIn_close((T_PPI_CAMERA_IN_HANDLE)pPPIcameraInSpec);
        return 0;
    }
    
    if (pa_pPPIcameraInConfig->fnCallback) {
       pPPIcameraInSpec->fnCallback = pa_pPPIcameraInConfig->fnCallback;
    }
    
    // Hook the Interrupt
    if(pa_pPPIcameraInConfig->nPerIVG) {
            
        pPPIcameraInSpec->nIVG = pa_pPPIcameraInConfig->nPerIVG;
        adi_int_SICSetIVG(g_aPPIspec[cPPI].peripheralIntId, pPPIcameraInSpec->nIVG);
    } else {
        adi_int_SICGetIVG(g_aPPIspec[cPPI].peripheralIntId, &(pPPIcameraInSpec->nIVG));
    }
    
    if(adi_int_CECHook(pPPIcameraInSpec->nIVG, PPIcameraInInterruptHandler, (void *)pPPIcameraInSpec, true) == ADI_INT_RESULT_SUCCESS) {
        adi_int_SICWakeup(g_aPPIspec[cPPI].peripheralIntId, TRUE);
	    adi_int_SICEnable(g_aPPIspec[cPPI].peripheralIntId);
	} else {
		pa_pPPIcameraInConfig->erError = ERR_PPI_CAMERA_IN_HOOK_INTERRUPT;
        PPIcameraIn_close((T_PPI_CAMERA_IN_HANDLE)pPPIcameraInSpec);
        return 0;
    }
    
    // Hook the Error Interrupt
    
    if (pa_pPPIcameraInConfig->fnErrorCallback) {
       pPPIcameraInSpec->fnErrorCallback = pa_pPPIcameraInConfig->fnErrorCallback;
    }
    
    if(pa_pPPIcameraInConfig->nPerErrorIVG) {
        pPPIcameraInSpec->nErrIVG = pa_pPPIcameraInConfig->nPerErrorIVG;
        adi_int_SICSetIVG(g_aPPIspec[cPPI].errorIntId, pa_pPPIcameraInConfig->nPerErrorIVG);
    } else {
        adi_int_SICGetIVG(g_aPPIspec[cPPI].errorIntId, &(pPPIcameraInSpec->nErrIVG));
    }
    
    if(adi_int_CECHook(pPPIcameraInSpec->nErrIVG, PPIcameraInErrorInterruptHandler, (void *)pPPIcameraInSpec, true) == ADI_INT_RESULT_SUCCESS) {
        adi_int_SICWakeup(g_aPPIspec[cPPI].errorIntId, TRUE);
	    adi_int_SICEnable(g_aPPIspec[cPPI].errorIntId);
	} else {
		pa_pPPIcameraInConfig->erError = ERR_PPI_CAMERA_IN_HOOK_INTERRUPT;
        PPIcameraIn_close((T_PPI_CAMERA_IN_HANDLE)pPPIcameraInSpec);
        return 0;
    }
    
    // Hook the DMA Error Interrupt
    
    if (pa_pPPIcameraInConfig->fnDmaErrorCallback) {
       pPPIcameraInSpec->fnDmaErrorCallback = pa_pPPIcameraInConfig->fnDmaErrorCallback;
    }
    
    if (pa_pPPIcameraInConfig->nDmaErrorIVG) {
        pPPIcameraInSpec->nDmaErrIVG = pa_pPPIcameraInConfig->nDmaErrorIVG;
        adi_int_SICSetIVG(g_aPPIspec[cPPI].DMAerrorIntId, pa_pPPIcameraInConfig->nDmaErrorIVG);
    } else {
        adi_int_SICGetIVG(g_aPPIspec[cPPI].DMAerrorIntId, &(pPPIcameraInSpec->nDmaErrIVG));
    }
    
    if(adi_int_CECHook(pPPIcameraInSpec->nDmaErrIVG, PPIcameraInDmaErrorInterruptHandler, (void *)pPPIcameraInSpec, true) == ADI_INT_RESULT_SUCCESS) {
        adi_int_SICWakeup(g_aPPIspec[cPPI].DMAerrorIntId, TRUE);
	    adi_int_SICEnable(g_aPPIspec[cPPI].DMAerrorIntId);
	} else {
		pa_pPPIcameraInConfig->erError = ERR_PPI_CAMERA_IN_HOOK_INTERRUPT;
        PPIcameraIn_close((T_PPI_CAMERA_IN_HANDLE)pPPIcameraInSpec);
        return 0;
    }    
    
    return (T_PPI_CAMERA_IN_HANDLE)pPPIcameraInSpec;
}



void PPIcameraIn_close(T_PPI_CAMERA_IN_HANDLE pa_tHndl) {
    if (pa_tHndl) {
        T_PPI_CAMERA_IN_SPEC *pSpec = (T_PPI_CAMERA_IN_SPEC *)pa_tHndl; 

        PPIcameraIn_stop(pa_tHndl);
        
        unsigned short i;
        // remove device from list
        for(i=0; i<PPI_CAMERA_IN_MAX_NOF_DEVICES; i++){
            if(g_aPPIcameraInHndlCount[i] == pa_tHndl){
                g_aPPIcameraInHndlCount[i] = 0;
                i = PPI_CAMERA_IN_MAX_NOF_DEVICES;
            }
        }
    
        // unhook the ISRs
        adi_int_SICDisable(g_aPPIspec[pSpec->cPPI].peripheralIntId);
        adi_int_CECUnhook(pSpec->nIVG, PPIcameraInInterruptHandler, (void *)pSpec);
        // check and unhook error interrupts
        if (pSpec->fnDmaErrorCallback) {
            adi_int_SICDisable(g_aPPIspec[pSpec->cPPI].DMAerrorIntId);
            adi_int_CECUnhook(pSpec->nDmaErrIVG, PPIcameraInDmaErrorInterruptHandler, (void *)pSpec);
        }
        if (pSpec->fnErrorCallback) {
            adi_int_SICDisable(g_aPPIspec[pSpec->cPPI].errorIntId);
            adi_int_CECUnhook(pSpec->nErrIVG, PPIcameraInErrorInterruptHandler, (void *)pSpec);           
        }
    
    
        for(i=0; i<pSpec->nMaxFrames; i++) {
            free(pSpec->pFrameBuffer[i]);
            free(pSpec->pFrameDescr[i]);
        }
    
        pSpec->pPPIspec->bInUse = false;
    
        free(pSpec->pFrameGrabberInterface);
        free(pSpec->pFrameBuffer);
        free(pSpec->pFrameDescr);
        free((void *)pSpec);
    }
}


void PPIcameraIn_setup(void) {
	unsigned short i, iMax;
	
	iMax = PPI_CAMERA_IN_MAX_NOF_DEVICES * FG_MAX_NOF_BUFFERED_FRAMES * PPI_CAMERA_IN_DESCR_SIZE;
	
	for(i=0; i<iMax; i++) {
		g_aPPIcameraInDMAdescriptor[i] = 0;
	}
	
	for(i=0; i<PPI_CAMERA_IN_MAX_NOF_DEVICES; i++) {
		g_aPPIcameraInHndlCount[i] = 0;
	}
}



void PPIcameraIn_cleanup(void) {
	// nothing to do
}

/**
 *	@brief		starts the PPIcameraIn
 *
 *	@param		pa_tHndl	Handle to a PPIcameraIn Resource
 *	
 *	@return		Returns an error code
 *	
 **/
T_ERROR_CODE PPIcameraIn_start(T_PPI_CAMERA_IN_HANDLE pa_tHndl) {
    if (pa_tHndl) {
        T_PPI_CAMERA_IN_SPEC *pSpec = (T_PPI_CAMERA_IN_SPEC *)pa_tHndl;
    
        if(pSpec->bIsActive) {
            return ERR_PPI_CAMERA_IN_ACTIVITY;
        }
        
        *(pSpec->pPPIspec->pDMAconfig) 	|= 0x0001;
    	*(pSpec->pPPIspec->pPPIcontrol)  |= 0x0001;
    	pSpec->bIsActive = true;
	
    	return ERR_NONE;
    } else {
        return ERR_PPI_CAMERA_IN_INVALID_HANDLE;
    }
}

/**
 *	@brief		stops the PPIcameraIn
 *
 *	@param		pa_tHndl	Handle to a PPIcameraIn Resource
 *	
 *	@return		Returns an error code
 *	
 **/
T_ERROR_CODE PPIcameraIn_stop(T_PPI_CAMERA_IN_HANDLE pa_tHndl) {
    if (pa_tHndl) {
        T_PPI_CAMERA_IN_SPEC *pSpec = (T_PPI_CAMERA_IN_SPEC *)pa_tHndl;
    
        if(!pSpec->bIsActive) {
            // camera not active
            return ERR_GENERIC;
        }
    
        *(pSpec->pPPIspec->pPPIcontrol)  &= ~0x0001;
    	*(pSpec->pPPIspec->pDMAconfig) 	&= ~0x0001; 
    	pSpec->bIsActive = false;
	
    	return ERR_NONE;
    } else {
        return ERR_PPI_CAMERA_IN_INVALID_HANDLE;
    }
}

// interface functions to the video grabber ------------------------------

/**
 *	@public
 *	@brief		starts the PPIcameraIn
 *
 *	@param		pa_tHndl	Handle to a PPIcameraIn Resource
 *	
 *	@return		Returns an error code
 *	
 **/
T_ERROR_CODE PPIcameraIn_startGrabbing(T_PPI_CAMERA_IN_HANDLE pa_tHndl) {
    if (pa_tHndl) {
        T_PPI_CAMERA_IN_SPEC *pSpec = (T_PPI_CAMERA_IN_SPEC *)pa_tHndl;
    
        if(pSpec->bIsActive) {
            return ERR_PPI_CAMERA_IN_ACTIVITY;
        }
        
        // reset all necessary
        pSpec->bDequeueInProgress = false;
        // reset the queue
        pSpec->nIndexOut = 0;
        pSpec->nIndexIn = 0;
        pSpec->nFramesLoaded = 0;
        
        // reset next descriptor pointer register
        *(g_aPPIspec[pSpec->cPPI].pDMAnextDescrPtr) = (void *)&g_aPPIcameraInDMAdescriptor[pSpec->nDescrIndOffset + 0];
        
        // enable PPI/DMA
        *(pSpec->pPPIspec->pDMAconfig) 	|= 0x0001;
    	*(pSpec->pPPIspec->pPPIcontrol)  |= 0x0001;
    	pSpec->bIsActive = true;
	
    	return ERR_NONE;
    } else {
        return ERR_PPI_CAMERA_IN_INVALID_HANDLE;
    }
}

/**
 *	@public
 *	@brief		stops grabbing (Interface function for the framegrabber)
 *
 *	@param		pa_tHndl	Handle to a PPIcameraIn Resource
 *	
 *	@return		Returns an error code
 *	
 **/
T_ERROR_CODE PPIcameraIn_stopGrabbing(T_PPI_CAMERA_IN_HANDLE pa_tHndl) {
    if (pa_tHndl) {
        T_PPI_CAMERA_IN_SPEC *pSpec = (T_PPI_CAMERA_IN_SPEC *)pa_tHndl;
    
        if(!pSpec->bIsActive) {
            // camera not active
            return ERR_GENERIC;
        }
    
        *(pSpec->pPPIspec->pPPIcontrol)  &= ~0x0001;
    	*(pSpec->pPPIspec->pDMAconfig) 	&= ~0x0001; 
    	pSpec->bIsActive = false;
	
    	return ERR_NONE;
    } else {
        return ERR_PPI_CAMERA_IN_INVALID_HANDLE;
    }
}

/**
 *	@public
 *	@brief		Gets the current FrameRate of the PPIcameraIn
 *
 *	@param		pa_tHndl	Handle to a PPIcameraIn Resource
 *	
 *	@return		Returns the FrameRate
 *	
 **/
float PPIcameraIn_getFramerate(T_PPI_CAMERA_IN_HANDLE pa_tHndl) {
	T_PPI_CAMERA_IN_SPEC *pSpec = (T_PPI_CAMERA_IN_SPEC *)pa_tHndl;
    return pSpec->fFramerate;
}


/**
 *	@public
 *	@brief		Resets the specified PPIcameraIn Device
 *
 *	@param		pa_tHndl	Handle to a PPIcameraIn Resource
 *	
 *	@return		ERR_NONE on sucess
 **/
T_ERROR_CODE PPIcameraIn_resetDevice(T_PPI_CAMERA_IN_HANDLE pa_tHndl) {
	T_PPI_CAMERA_IN_SPEC *pSpec = (T_PPI_CAMERA_IN_SPEC *)pa_tHndl;
    // reset buffer pointer for multi-buffering
    pSpec->nIndexIn = 0;
    pSpec->nIndexOut = 0;
    pSpec->nFrameBufferIndex = 0;
    pSpec->nFramesLoaded = 0;
    pSpec->bDequeueInProgress = false;

    return ERR_NONE;
}


/**
 *	@public
 *	@brief		Configures the specified PPIcameraIn resource
 *				
 *
 *	@param		pa_tHndl	Handle to a PPIcameraIn Resource
 *	@param		pa_tCmd		PPIcameraIn Commands @see frameGrabber.h
 *	@param		pa_tArg		Argument for command @see T_FG_ARG (can only use the pa_tArg.nNofFramesToHold element).
 *	
 *	@return		ERR_NONE on sucess.
 *	
 **/
T_ERROR_CODE PPIcameraIn_configDevice(T_PPI_CAMERA_IN_HANDLE pa_tHndl, T_FG_CMD pa_tCmd, T_FG_ARG *pa_tArg) {
	T_PPI_CAMERA_IN_SPEC *pSpec = (T_PPI_CAMERA_IN_SPEC *)pa_tHndl;

	switch(pa_tCmd){
	    case FG_CMD_CLOSE:{
	        if(pSpec->fnHwDevCloseDev && pSpec->hHwDevHndl){
	            pSpec->fnHwDevCloseDev(pSpec->hHwDevHndl);
	        }
	        PPIcameraIn_stop(pa_tHndl);
	        PPIcameraIn_close(pa_tHndl);
	        break;
	    }
	    case FG_CMD_GET_DEVICE_NAME:{
	        sprintf(pa_tArg->cDeviceName, "PPIcameraIn%d", pSpec->cPPI);
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
 *	@param		pa_tHndl	Handle to a PPIcameraIn Resource
 *	
 *	@return		The number of frames waiting to be grabbed.
 *	
 **/
int PPIcameraIn_framesAvailable(T_PPI_CAMERA_IN_HANDLE pa_tHndl) {
	T_PPI_CAMERA_IN_SPEC *pSpec = (T_PPI_CAMERA_IN_SPEC *)pa_tHndl;
    return (int)pSpec->nFramesLoaded;
}


/**
 *	@public
 *	@brief		Returns a pointer to the next Frame.
 *
 *				Provides a pointer to a ifrm_t frame. @see ifrm_t.
 *
 *	@param		pa_tHndl	Handle to a PPIcameraIn Resource
 *	
 *	@return		A valid frame pointer or a NULL pointer on error.
 *	
 **/
ifrm_t *PPIcameraIn_getNextFrame(T_PPI_CAMERA_IN_HANDLE pa_tHndl) {
    if (pa_tHndl) {
    	T_PPI_CAMERA_IN_SPEC *pSpec = (T_PPI_CAMERA_IN_SPEC *)pa_tHndl;
	
    	if(pSpec->nFramesLoaded > 0 && !pSpec->bDequeueInProgress) {
    	    pSpec->bDequeueInProgress = true;
    	    return (pSpec->pFrameDescr[pSpec->nIndexOut]);
    	} else {
    	    return NULL;
    	}
    } else {
        return NULL;
    }
}


/**
 *	@public
 *	@brief		Frees a Frame from the PPIcameraIn Frame Buffer
 *
 *	@param		pa_tHndl	Handle to a PPIcameraIn Resource
 *	
 *	@return		ERR_NONE on sucess, ERR_GENERIC on failure.
 *	
 **/
T_ERROR_CODE PPIcameraIn_freeFrame(T_PPI_CAMERA_IN_HANDLE pa_tHndl) {
    if (pa_tHndl) {
    	T_PPI_CAMERA_IN_SPEC *pSpec = (T_PPI_CAMERA_IN_SPEC *)pa_tHndl;
    	if (pSpec->bDequeueInProgress) {
    		pSpec->nIndexOut ++;
		
    		if (pSpec->nIndexOut >= pSpec->nMaxFrames) {
    		    pSpec->nIndexOut = 0;
    		}
		
    		pSpec->nFramesLoaded --;
	
    		if (!pSpec->bIsActive && ((pSpec->nBufferMode == PPI_CAMERA_IN_DOUBLE_BUFFER_NO_OVERWRITE) || 
    		    (pSpec->nBufferMode ==  PPI_CAMERA_IN_MULTI_FRAME_BUFFER_NO_OVERWRITE))) {
    			PPIcameraIn_start(pa_tHndl);
    		}
		
    		pSpec->bDequeueInProgress = false;
		
    		return ERR_NONE;
    	} else {
    	    return ERR_GENERIC;
    	}
    } else {
        return ERR_PPI_CAMERA_IN_INVALID_HANDLE;
    }
}


/**
 *	@public
 *	@brief		Writes back a T_FG_VIDEO_DEVICE_INFO stucture to a provided buffer. @see T_FG_VIDEO_DEVICE_INFO 
 *
 *				
 *
 *	@param		pa_tHndl		Handle to a PPIcameraIn Resource
 *	@param		pa_tInfoBlock	Provide the address of a T_FG_VIDEO_DEVICE_INFO structure.
 *	
 *	@return		ERR_NONE on sucess, ERR_GENERIC on timeout or failure.
 *	
 **/
void PPIcameraIn_getDeviceInfo(T_PPI_CAMERA_IN_HANDLE pa_tHndl, T_FG_VIDEO_DEVICE_INFO *pa_tInfoBlock) {
	T_PPI_CAMERA_IN_SPEC *pSpec = (T_PPI_CAMERA_IN_SPEC *)pa_tHndl;
    pa_tInfoBlock->tFramerate = (T_FG_FRAMERATE)pSpec->fFramerate;
    pa_tInfoBlock->nXres = pSpec->nXres;
    pa_tInfoBlock->nYres = pSpec->nYres;
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

