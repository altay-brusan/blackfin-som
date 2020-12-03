/**
    @file EPPI656Out.c
    @ingroup video
    @brief 
    
    
    
    BLT_DISCLAIMER(TBD)
    @author Alexander Froemel
    @version 1.0
    @date 20.04.2010
    
    @startcond Changelog
    
    @endcond
**/

#include <cplbtab.h>
#include <stdio.h>
#include <conio.h>
#include <string.h>
#include <Environment.h>
#ifdef _USE_VDK_
    #include <vdk.h>
#endif
#include "EPPI656Out.h"
#include "../../mdma/MDMAconfig.h"
#include "../../EPPIconfig.h"
#include "../../EPPI.h"
#include "../../misc/delay.h"
#include "../../../../../BLACKSheep/common/syslog/syslog.h"
//#include "../../analog_video/analogVideo.h"


extern unsigned int g_nMDMAcount;                   ///< declared in mdma_global.c
extern T_MDMA_SPEC g_aMDMAspec[];                   ///< declared in mdma_global.c
extern T_AV_STANDARD_DESCR g_tPALstd;              ///< declared in analogVideo.c
extern T_AV_STANDARD_DESCR g_tNTSCstd;             ///< declared in analogVideo.c
extern unsigned int g_nEPPIcount;
extern T_EPPI_SPEC g_aEPPIspec[];
//extern unsigned long g_nEPPI656OutErrorCount;
extern T_MDMA_SPEC g_aMDMAspec[];


unsigned short g_nEPPI656OutActivePlane = 0;

unsigned long g_nEPPI656OutInt = 0;
unsigned long g_nEPPI656OutIntSyslogCount = 0;
unsigned long g_nEPPI656OutPlaneSwitch = 0;
unsigned long g_nEPPI656OutErrorCount = 0;
unsigned long g_nEPPI656OutDmaErrorCount = 0;
unsigned long g_nEPPI656OutErrorLtoCount = 0;
unsigned long g_nEPPI656OutFifoCount = 0;


#ifdef __ADSPBF561__
    #ifdef __COREA__
        section ("L1_data_a") 
        unsigned short g_anEPPI656ODMAdesc[EPPI656O_MAX_DEVICES][EPPI656O_DMA_DESC_SIZE * EPPI656O_DESC_PER_DEV];                                // descriptor size = 4

    #elif defined (__COREB__)
        section ("L1_data_b") 
        unsigned short g_anEPPI656ODMAdesc[EPPI656O_MAX_DEVICES][EPPI656O_DMA_DESC_SIZE * EPPI656O_DESC_PER_DEV];                                // descriptor size = 4

    #else
        #error "You must specify a core!"
    #endif    
#else
    section ("L1_data") 
    unsigned short g_anEPPI656ODMAdesc[EPPI656O_MAX_DEVICES][EPPI656O_DMA_DESC_SIZE * EPPI656O_DESC_PER_DEV];                                // descriptor size = 4
#endif


static T_EPPI656O_HANDLE g_atEPPI656Otable[EPPI656O_MAX_DEVICES];      ///< record handles
static unsigned char g_cEPPI656OnoDevices = 0;                      ///< number of initialized devices

void AVinitITU656Matrix ( unsigned char *pa_pcBuffer, T_AV_STANDARD_DESCR *pa_pStd );
T_ERROR_CODE AV_CopyYUV422toITUR656Frame(T_AV_FRAME_BUFFER_DESC* pa_tDestBuffDesc, 
                                 unsigned short pa_nXOffset,
                                 unsigned short pa_nYOffset,
                                 char* pa_pcYUV422Start, 
                                 unsigned short pa_nWidth, 
                                 unsigned short pa_nHeight, 
                                 unsigned char pa_cMdmaChannel);
                                 
                                 
#pragma section ("L1_code")
ADI_INT_HANDLER_RESULT EPPI6565OinterruptHandler(void *pa_pClientArg) {
    
    T_EPPI656O_INST *pInst = (T_EPPI656O_INST *)pa_pClientArg;
    
    if(pInst) {
        
        // check if peripheral interrupt occures
        if((*g_aEPPIspec[pInst->cEPPInr].pDMA0irqStat) & DMA_DONE) {
            // clear the interrupt
            *( g_aEPPIspec[pInst->cEPPInr].pDMA0irqStat ) |= DMA_DONE;
            ssync();
            int nDummy = *(g_aEPPIspec[pInst->cEPPInr].pEPPIstatus);
            
            g_nEPPI656OutInt++;
            
            // check if a full frame was transfered
            pInst->nTransmittedLines += pInst->tAVstandard.nHeight >> 1;
            
            if(pInst->nTransmittedLines == pInst->tAVstandard.nHeight >> 1) {
                if((pInst->bPlaneSwitchRequest || pInst->bAutoUpdate) && !pInst->bPlaneSwitchField1Switched) {
                    // set start address for the new first field
                    g_anEPPI656ODMAdesc[pInst->cDeviceNr][2] = (unsigned short)((unsigned long)pInst->pcPlaneBuffer[pInst->cNextPlane] & 0xffff);    // SAL: start address frame buffer 0
                    g_anEPPI656ODMAdesc[pInst->cDeviceNr][3] = (unsigned short)((unsigned long)pInst->pcPlaneBuffer[pInst->cNextPlane] >> 16);       // SAH: start address frame buffer 0
                    pInst->bPlaneSwitchField1Switched = true;
                    pInst->bVSYNC = true;
        #ifdef _USE_VDK_
                    VDK_C_ISR_PostSemaphore(pInst->tFsyncSem);
        #endif
                    g_nEPPI656OutActivePlane = pInst->cActivePlane;                    
                }
            } 
            else {
                if(pInst->nTransmittedLines >= pInst->tAVstandard.nHeight ) {
                    pInst->nTransmittedLines = 0;
                    //pInst->bVSYNC = true;
                    if(pInst->bPlaneSwitchField1Switched) {
                        if(pInst->bPlaneSwitchRequest) {
                            g_nEPPI656OutPlaneSwitch++;
                            // set the start address for the new second field
                            g_anEPPI656ODMAdesc[pInst->cDeviceNr][6] = (unsigned short)((unsigned long)(pInst->pcPlaneBuffer[pInst->cNextPlane] + pInst->tAVstandard.nActiveVideoLineSize) & 0xffff);    // SAL: start address frame buffer 0
                            g_anEPPI656ODMAdesc[pInst->cDeviceNr][7] = (unsigned short)((unsigned long)(pInst->pcPlaneBuffer[pInst->cNextPlane] + pInst->tAVstandard.nActiveVideoLineSize) >> 16);       // SAH: start address frame buffer 0
                
                            // switch plane
                            pInst->cActivePlane = pInst->cNextPlane;
                            pInst->bPlaneSwitchRequest = false;
                        
                    
                            // update free number of planes
                            if(pInst->nFreePlanesCount < pInst->cNumberOfPlanes) {
                                pInst->nFreePlanesCount++;     
                            }
                        }
                        else {
                            if (pInst->bAutoUpdate){
                                //use buffermode strategy
                                pInst->cActivePlane++;
                                if(pInst->cActivePlane >= pInst->cNumberOfPlanes) {
                                    pInst->cActivePlane = 0;
                                }
            
                    /*            g_anEPPI656ODMAdesc[pInst->cDeviceNr][2] = (unsigned short)((unsigned long)pInst->pcPlaneBuffer[pInst->cActivePlane] & 0xffff);    // SAL: start address frame buffer 0
                                g_anEPPI656ODMAdesc[pInst->cDeviceNr][3] = (unsigned short)((unsigned long)pInst->pcPlaneBuffer[pInst->cActivePlane] >> 16);       // SAH: start address frame buffer 0
                    */
                                g_anEPPI656ODMAdesc[pInst->cDeviceNr][6] = (unsigned short)((unsigned long)(pInst->pcPlaneBuffer[pInst->cActivePlane] + pInst->tAVstandard.nActiveVideoLineSize) & 0xffff);    // SAL: start address frame buffer 0
                                g_anEPPI656ODMAdesc[pInst->cDeviceNr][7] = (unsigned short)((unsigned long)(pInst->pcPlaneBuffer[pInst->cActivePlane] + pInst->tAVstandard.nActiveVideoLineSize) >> 16);       // SAH: start address frame buffer 0
                
                                // update free number of planes                         
                                if(pInst->nFreePlanesCount < pInst->cNumberOfPlanes) {
                                    pInst->nFreePlanesCount++;     
                                }    
                            }
                        }                        
                        pInst->bPlaneSwitchField1Switched = false;
                    }
                }
            }
        
            return ADI_INT_RESULT_PROCESSED;
        }
    }
    return ADI_INT_RESULT_NOT_PROCESSED;
}


#pragma section ("L1_code")
static ADI_INT_HANDLER_RESULT EPPI656OerrorHandler(void *pa_pClientArg) {
  
	T_EPPI656O_INST *pInst = (T_EPPI656O_INST*)pa_pClientArg;
	
	if(pInst) {
    	int nDummy = 0;
		
    	// clear status register
    	nDummy = *(g_aEPPIspec[pInst->cEPPInr].pEPPIstatus);
    	*(g_aEPPIspec[pInst->cEPPInr].pEPPIstatus) = nDummy;
        // check if peripheral interrupt occures
        if(nDummy & 0x41ff) {
    	    if(nDummy & 0x0004) {
                g_nEPPI656OutErrorLtoCount++;
#if _EPPI656O_DEBUG_LVL_ > 0
                if(g_nEPPI656OutErrorLtoCount >> 8) {
                    char acMessage[50];
                    sprintf(acMessage, "(EPPI656OerrorHandler): eppi line track error: %u", g_nEPPI656OutErrorLtoCount);
                    syslog_post(acMessage, 0, 0, 0);
                }
#endif
    	    } else if(nDummy & 0x0003) {
    	        g_nEPPI656OutFifoCount++;
#if _EPPI656O_DEBUG_LVL_ > 0
                //syslog_post("(EPPI656OerrorHandler): eppi fifo under/overflow", 0, 0, 0);
#endif
    	    } else {
        
                g_nEPPI656OutErrorCount++;
#if _EPPI656O_DEBUG_LVL_ > 0
                char acMessage[50];
                sprintf(acMessage, "(EPPI656OerrorHandler): eppi error: %u, 0x%x", g_nEPPI656OutErrorCount);
                syslog_post(acMessage, 0, 0, 0);
#endif
                EPPI656Ostop((T_EPPI656O_HANDLE)pInst);

        
                if (pInst->fnErrorCallback) {                                        // Check if a Callback function was hooked
                    pInst->fnErrorCallback(pa_pClientArg);                // If so, execute it!
                }

                EPPI656Ostart((T_EPPI656O_HANDLE)pInst);  
    	    }
            return ADI_INT_RESULT_PROCESSED;            // Processed the interrupt, so notify ADI's ISR routine.
        } else {
            return ADI_INT_RESULT_NOT_PROCESSED;    // Interrupt wasn't processed, so notify ADI's ISR routine.
        }
	} else {
        return ADI_INT_RESULT_NOT_PROCESSED;    // Interrupt wasn't processed, so notify ADI's ISR routine.
    }
}



#pragma section ("L1_code")
ADI_INT_HANDLER_RESULT EPPI6565ODMAErrorInterruptHandler(void *pa_pClientArg) {
        
    T_EPPI656O_INST *pInst = (T_EPPI656O_INST *)pa_pClientArg;
    
    if (pInst) {	
        
    	if (*(g_aEPPIspec[pInst->cEPPInr].pDMA0irqStat) & DMA_ERR) {
            EPPI656Ostop((T_EPPI656O_HANDLE)pInst);
    	    
            g_nEPPI656OutDmaErrorCount++;
#if _EPPI656O_DEBUG_LVL_ > 0
                char acMessage[50];
                sprintf(acMessage, "(EPPI6565ODMAErrorInterruptHandler): eppi dma0 error: %u", g_nEPPI656OutDmaErrorCount);
                syslog_post(acMessage, 0, 0, 0);
#endif
            
    		*(g_aEPPIspec[pInst->cEPPInr].pDMA0irqStat) |= DMA_ERR;
    		
            EPPI656Ostart((T_EPPI656O_HANDLE)pInst);   
                        
    		return ADI_INT_RESULT_PROCESSED;
    	}
    }
        
    return ADI_INT_RESULT_NOT_PROCESSED;	// Interrupt wasn't for us, notify ADI's ISR that we didn't process it!
}


#pragma section ("L1_code")      
ADI_INT_HANDLER_RESULT MDMAErrorInterruptHandler(void *pa_pClientArg) { 
    T_EPPI656O_INST *pInst = (T_EPPI656O_INST *)pa_pClientArg;
    
    if(pInst) {
        if((*g_aMDMAspec[pInst->cMDMAChannel].pSourceIrqStat) & DMA_DONE) {
            *g_aMDMAspec[pInst->cMDMAChannel].pSourceIrqStat |= DMA_DONE;
            return ADI_INT_RESULT_PROCESSED;        
        }
    }
    return ADI_INT_RESULT_NOT_PROCESSED;
       
}



/**
    @brief Open an ITU656 over EPPI output device
**/
T_EPPI656O_HANDLE EPPI656Oopen(T_EPPI656O_CONFIG *pa_ptConfig, T_ERROR_CODE *pa_tError, void ***pfGraficFunctions) {
    T_EPPI656O_HANDLE tHndl = 0;
    
    T_EPPI656O_INST *pInst = (T_EPPI656O_INST*) malloc (sizeof(T_EPPI656O_INST));
    *pa_tError = ERR_NONE;
    
    if(pInst) {
        
        memset(pInst, 0, sizeof(T_EPPI656O_INST));
        
        //get video standard fill standard descriptor
        switch(pa_ptConfig->tAVformat) {
            case AV_PAL: {
                pInst->tAVstandard = g_tPALstd;
                break;
            }
            case AV_NTSC: {
                pInst->tAVstandard = g_tNTSCstd;
                break;
            }
            
            default: {
                *pa_tError = ERR_EPPI656O_UNKNWON_AV_STD;
                tHndl = 0;
                break;
            }
        }
        
        if(*pa_tError == ERR_NONE) {
        
            if(pa_ptConfig->cNumberOfPlanes == 0) {
                pa_ptConfig->cNumberOfPlanes = 1;
            }
        
            if(pa_ptConfig->cNumberOfPlanes > VDM_MAX_NOF_PLANES) {
                pa_ptConfig->cNumberOfPlanes = VDM_MAX_NOF_PLANES;
            }
        
            //malloc space for planebuffers
            if(!pa_ptConfig->bUseSharedBuffer) {
                pInst->pcPlaneBuffer[0] = (unsigned char*) calloc(pa_ptConfig->cNumberOfPlanes * pInst->tAVstandard.nWidth * pInst->tAVstandard.nHeight, pInst->tAVstandard.nPixelStride);
            }
            pInst->bUseSharedBuffer = pa_ptConfig->bUseSharedBuffer;

                    
            if(pInst->pcPlaneBuffer[0] || pInst->bUseSharedBuffer) {
            
                unsigned short i;
            
                pInst->cDeviceNr = g_cEPPI656OnoDevices;
                g_cEPPI656OnoDevices++;
                
                pInst->pcFrameBuffer = pInst->pcPlaneBuffer[0];
        
#if _EPPI656O_DEBUG_LVL_ > 0
                    printf("(T_EPPI656Oopen): Planebuffer 0 @ 0x%x\n", pInst->pcFrameBuffer);
#endif
        
                //init planebuffers
                for(i = 1; i < pa_ptConfig->cNumberOfPlanes; i++) {
                    pInst->pcPlaneBuffer[i] = pInst->pcPlaneBuffer[i-1] + pInst->tAVstandard.nWidth * pInst->tAVstandard.nHeight * pInst->tAVstandard.nPixelStride;
#if _EPPI656O_DEBUG_LVL_ > 0
                        printf("(T_EPPI656Oopen): Planebuffer %d @ 0x%x\n", i, pInst->pcPlaneBuffer[i]);
#endif
                }
        
                /*
                for(i = 0; i < pa_ptConfig->cNumberOfPlanes; i++) {
                    AVinitITU656Matrix(pInst->pcPlaneBuffer[i], &pInst->tAVstandard);
                }
            */
                pInst->pHwDevHndl = pa_ptConfig->pHwDevHndl;
                pInst->cNumberOfPlanes = pa_ptConfig->cNumberOfPlanes;
                pInst->bOverwriteActivePlaneAllowed = (pInst->cNumberOfPlanes == 1) ? true : 
                                                      pa_ptConfig->bOverwriteActivePlaneAllowed;
                pInst->cActivePlane = 0;
                
                if(pInst->cNumberOfPlanes > 1) {
                    pInst->cNextPlane = 1;
                } else {
                    pInst->cNextPlane = 0;
                }
                pInst->nTransmittedLines = 0;
                pInst->nFreePlanesCount = (pInst->cNumberOfPlanes == 1) ? 1 : -1;//  (pInst->cNumberOfPlanes - 1 + ((pInst->bOverwriteActivePlaneAllowed) ? 1 : 0));
                pInst->tAVformat = pa_ptConfig->tAVformat;
                pInst->cEPPInr = pa_ptConfig->cEPPInr;
                pInst->nEPPIIVG = pa_ptConfig->nEPPIIVG;            ///< EPPI IVG 0 = standard
                pInst->nErrorIVG =  pa_ptConfig->nErrorIVG;          ///< EPPI Error IVG 0 = standard
                pInst->nDMAErrorIVG =  pa_ptConfig->nDMAErrorIVG;
                pInst->bAutoUpdate =  pa_ptConfig->bAutoUpdate;
                pInst->cMDMAChannel = pa_ptConfig->cMDMAChannel;
                pInst->bPlaneSwitchField1Switched = false;
                pInst->bPlaneSwitchRequest = false;
                if(pa_ptConfig->fnDeviceClose) {
                    pInst->fnDeviceClose = pa_ptConfig->fnDeviceClose;
                }
                else {
                    pInst->fnDeviceClose = 0;
                }
                //fill functions for VDM
                pInst->pfGraficFunctions[VD_SET_PIXEL] = (void *)EPPI656OsetPixel;
                pInst->pfGraficFunctions[VD_GET_PIXEL] = (void *)EPPI656OgetPixel;
                pInst->pfGraficFunctions[VD_BLEND_PIXEL] = (void *)EPPI656OblendPixel;
                pInst->pfGraficFunctions[VD_GET_ACTIVE_PLANE] = (void *)EPPI656OgetActivePlane;
                pInst->pfGraficFunctions[VD_SET_ACTIVE_PLANE] = (void *)EPPI656OsetActivePlane;
                pInst->pfGraficFunctions[VD_FSYNC_WAIT] = (void*)EPPI656OwaitForFrameSync;
                pInst->pfGraficFunctions[VD_SHOW_FRAME] = (void *)EPPI656OshowFrame;
                pInst->pfGraficFunctions[VD_GET_NEXT_PLANE] = (void *)EPPI656OgetNextPlaneNum;
                *pfGraficFunctions = pInst->pfGraficFunctions;
#ifdef _USE_VDK_
                //create semaphore for fsync
                pInst->tFsyncSem = VDK_CreateSemaphore(1, 1, 1, 0);
#endif

                //setup DMA descriptor
                g_anEPPI656ODMAdesc[pInst->cDeviceNr][0] = (unsigned short)((unsigned long)&g_anEPPI656ODMAdesc[pInst->cDeviceNr][4] & 0xffff);            // NDPL
                g_anEPPI656ODMAdesc[pInst->cDeviceNr][1] = (unsigned short)((unsigned long)&g_anEPPI656ODMAdesc[pInst->cDeviceNr][4] >> 16);            // NDPH
                g_anEPPI656ODMAdesc[pInst->cDeviceNr][2] = (unsigned short)((unsigned long)pInst->pcPlaneBuffer[pInst->cActivePlane] & 0xffff);    // SAL: start address frame buffer 0
                g_anEPPI656ODMAdesc[pInst->cDeviceNr][3] = (unsigned short)((unsigned long)pInst->pcPlaneBuffer[pInst->cActivePlane] >> 16);       // SAH: start address frame buffer 0
                
                g_anEPPI656ODMAdesc[pInst->cDeviceNr][4] = (unsigned short)((unsigned long)&g_anEPPI656ODMAdesc[pInst->cDeviceNr][0] & 0xffff);            // NDPL
                g_anEPPI656ODMAdesc[pInst->cDeviceNr][5] = (unsigned short)((unsigned long)&g_anEPPI656ODMAdesc[pInst->cDeviceNr][0] >> 16);            // NDPH
                g_anEPPI656ODMAdesc[pInst->cDeviceNr][6] = (unsigned short)((unsigned long)(pInst->pcPlaneBuffer[pInst->cActivePlane] + pInst->tAVstandard.nActiveVideoLineSize) & 0xffff);    // SAL: start address frame buffer 0
                g_anEPPI656ODMAdesc[pInst->cDeviceNr][7] = (unsigned short)((unsigned long)(pInst->pcPlaneBuffer[pInst->cActivePlane] + pInst->tAVstandard.nActiveVideoLineSize) >> 16);       // SAH: start address frame buffer 0
                
                //setup EPPI 
                //void *pExitCriticalArg = adi_int_EnterCriticalRegion(NULL);
            
                if (g_aEPPIspec[pa_ptConfig->cEPPInr].bInUse) {
                    *pa_tError = ERR_ALREADY_INITIALIZED;
                }
                //adi_int_ExitCriticalRegion(pExitCriticalArg);
        
                if (*pa_tError == ERR_NONE) {
                    // no error so far - continue        
                    if ( eppi_platformInit(pa_ptConfig->cEPPInr, 0x00101572) ) {
                        
                        /*
                        // set dma registers
                        *(g_aEPPIspec[pInst->cEPPInr].pDMA0xcount)    = pInst->tAVstandard.nLineStride >> 2;
                        *(g_aEPPIspec[pInst->cEPPInr].pDMA0xmodify)   = pInst->tAVstandard.nPixelStride << 1;
                        
                        *(g_aEPPIspec[pInst->cEPPInr].pDMA0ycount)    = pInst->tAVstandard.nTotalLines;
                        *(g_aEPPIspec[pInst->cEPPInr].pDMA0ymodify)   = (pInst->tAVstandard.nPixelStride << 1);
                        */
                        
                        *(g_aEPPIspec[pInst->cEPPInr].pDMA0xcount)    = (pInst->tAVstandard.nActiveVideoLineSize) >> 2;
                        *(g_aEPPIspec[pInst->cEPPInr].pDMA0xmodify)   = 4;
                    
                        *(g_aEPPIspec[pInst->cEPPInr].pDMA0ycount)    = pInst->tAVstandard.nHeight >> 1;
                        *(g_aEPPIspec[pInst->cEPPInr].pDMA0ymodify)   = pInst->tAVstandard.nActiveVideoLineSize + 4;
                    
                        *(g_aEPPIspec[pInst->cEPPInr].pDMA0nextDescrPtr) = (void *)&g_anEPPI656ODMAdesc[pInst->cDeviceNr][0];
                        *(g_aEPPIspec[pInst->cEPPInr].pDMA0config)    = 0x7498;
                        
                    // set eppi registers
                        *(g_aEPPIspec[pInst->cEPPInr].pEPPIhdelay)    = 0;
                        *(g_aEPPIspec[pInst->cEPPInr].pEPPIvdelay)    = 0;
                        *(g_aEPPIspec[pInst->cEPPInr].pEPPIclkdiv)    = 0;
                        *(g_aEPPIspec[pInst->cEPPInr].pEPPIfs1w_hbl)  = pInst->tAVstandard.nBlankLineSize;
                        *(g_aEPPIspec[pInst->cEPPInr].pEPPIfs1p_avpl) = pInst->tAVstandard.nActiveVideoLineSize;
                        *(g_aEPPIspec[pInst->cEPPInr].pEPPIfs2w_lvb)  = 0x02170216;//pInst->tAVstandard.nTotalLines - (pInst->tAVstandard.nHeight);
                        *(g_aEPPIspec[pInst->cEPPInr].pEPPIfs2p_lavf) = 0x01200120;//(pInst->tAVstandard.nHeight >> 1) << 16 | (pInst->tAVstandard.nHeight >> 1);
                        *(g_aEPPIspec[pInst->cEPPInr].pEPPIclip)      = 0xff00ff00;
                        *(g_aEPPIspec[pInst->cEPPInr].pEPPIcontrol)   = 0x00101572;
                        *(g_aEPPIspec[pInst->cEPPInr].pEPPIframe)     = 0;//pInst->tAVstandard.nTotalLines;
                        *(g_aEPPIspec[pInst->cEPPInr].pEPPIline)      = 0;//pInst->tAVstandard.nLineStride;
                        
   
                        if(!pa_ptConfig->nEPPIIVG) {
                        
                            adi_int_SICGetIVG(g_aEPPIspec[pa_ptConfig->cEPPInr].peripheralIntId0, &pInst->nEPPIIVG);
                        } else {
                            adi_int_SICSetIVG(g_aEPPIspec[pa_ptConfig->cEPPInr].peripheralIntId0, pa_ptConfig->nEPPIIVG);
                            pInst->nEPPIIVG = pa_ptConfig->nEPPIIVG; //use custom ivg
                        }
                       
                        // hook the interrupt
                        if(adi_int_CECHook(pInst->nEPPIIVG, EPPI6565OinterruptHandler, (void *)pInst, false) == ADI_INT_RESULT_SUCCESS) {
                            adi_int_SICWakeup(g_aEPPIspec[pa_ptConfig->cEPPInr].peripheralIntId0, FALSE);
                            adi_int_SICEnable(g_aEPPIspec[pa_ptConfig->cEPPInr].peripheralIntId0);
                        } else {
                            *pa_tError = ERR_EPPI656O_HOOK_INTERRUPT;
                        }
                        
                        if(!pa_ptConfig->nErrorIVG) {
                            adi_int_SICGetIVG(g_aEPPIspec[pa_ptConfig->cEPPInr].perErrorIntId0, &pInst->nErrorIVG);
                        } else {
                            adi_int_SICSetIVG(g_aEPPIspec[pa_ptConfig->cEPPInr].perErrorIntId0, pa_ptConfig->nErrorIVG);
                            pInst->nErrorIVG = pa_ptConfig->nErrorIVG; //use custom ivg
                        }                        
                        //hook error interupt                             
                        if(adi_int_CECHook(pInst->nErrorIVG, EPPI656OerrorHandler, (void *)pInst, false) == ADI_INT_RESULT_SUCCESS) {
                            adi_int_SICWakeup(g_aEPPIspec[pa_ptConfig->cEPPInr].perErrorIntId0, FALSE);
                            adi_int_SICEnable(g_aEPPIspec[pa_ptConfig->cEPPInr].perErrorIntId0);
                        } else {
                            *pa_tError = ERR_EPPI656O_HOOK_INTERRUPT;
                        }    
                        
                        if(!pa_ptConfig->nDMAErrorIVG) {
                            adi_int_SICGetIVG(g_aEPPIspec[pa_ptConfig->cEPPInr].DMAerrorIntId0, &pInst->nDMAErrorIVG);
                        } else {
                            adi_int_SICSetIVG(g_aEPPIspec[pa_ptConfig->cEPPInr].DMAerrorIntId0, pa_ptConfig->nDMAErrorIVG);
                            pInst->nDMAErrorIVG = pa_ptConfig->nDMAErrorIVG; //use custom ivg
                        }       
                        // hook DMA error interrupt
                        if(adi_int_CECHook(pInst->nDMAErrorIVG, EPPI6565ODMAErrorInterruptHandler, (void *)pInst, false) == ADI_INT_RESULT_SUCCESS) {
                            adi_int_SICWakeup(g_aEPPIspec[pa_ptConfig->cEPPInr].DMAerrorIntId0, FALSE);
                            adi_int_SICEnable(g_aEPPIspec[pa_ptConfig->cEPPInr].DMAerrorIntId0);
                        } else {
                            *pa_tError = ERR_EPPI656O_HOOK_INTERRUPT;
                        }
                    
                        tHndl = (T_EPPI656O_HANDLE) pInst; 
                        //enable EPPI
                        EPPI656Ostart(tHndl);
                   } else {
                        // error the platform init function failed
                        *pa_tError = ERR_EPPI6565O_PLATFORM_INIT;
                        if(!pInst->bUseSharedBuffer) {
                            free(pInst->pcPlaneBuffer[0]);
                        }
                        free(pInst);
                        tHndl = 0;
                    }
                } else {
                    // EPPI is already initialized    
                       // free plane buffer and instances
                       if(!pInst->bUseSharedBuffer) {
                           free(pInst->pcPlaneBuffer[0]);
                       }                       
                       free(pInst);
                       *pa_tError =ERR_EPPI656O_EPPI_NOT_AVAILABLE;
                       tHndl = 0;
                       }      
                   
            } else {
                free(pInst);
                tHndl = 0;
                *pa_tError = ERR_EPPI656O_OUT_OF_MEM;
            }
        } else {
            free(pInst);
            tHndl = 0;
            *pa_tError = ERR_EPPI656O_UNKNWON_AV_STD;
        }
    } else {
        tHndl = 0;
        *pa_tError = ERR_EPPI656O_OUT_OF_MEM;
    }    
    return tHndl;
}

/**

**/
T_ERROR_CODE T_EPPI656Oclose(T_EPPI656O_HANDLE pa_tHndl) {
    T_ERROR_CODE erResult = ERR_NONE;
    
    if(pa_tHndl) {
        T_EPPI656O_INST *pInst = (T_EPPI656O_INST *)pa_tHndl;
        unsigned short i;
        
        // disable
        EPPI656Ostop(pa_tHndl);
        
        //Sleep(1);
        
        // unhook the interrupts
        adi_int_SICDisable(g_aEPPIspec[pInst->cEPPInr].peripheralIntId0);
        adi_int_CECUnhook(pInst->nEPPIIVG, EPPI6565OinterruptHandler, (void *)pInst);
        
        adi_int_SICDisable(g_aEPPIspec[pInst->cEPPInr].perErrorIntId0);
        adi_int_CECUnhook(pInst->nErrorIVG, EPPI656OerrorHandler, (void *)pInst);
        
        adi_int_SICDisable(g_aEPPIspec[pInst->cEPPInr].DMAerrorIntId0);
        adi_int_CECUnhook(pInst->nDMAErrorIVG, EPPI6565ODMAErrorInterruptHandler, (void *)pInst);
        

        // close device
        if(pInst->fnDeviceClose && pInst->pHwDevHndl) {
            pInst->fnDeviceClose(pInst->pHwDevHndl);
        }
        
        if(!pInst->bUseSharedBuffer) {
           free(pInst->pcPlaneBuffer[0]);
        }                       
        
        #ifdef _USE_VDK_
            //destroy semaphore for fsync
            VDK_DestroySemaphore(pInst->tFsyncSem);
        #endif
        free(pInst);
        g_cEPPI656OnoDevices--;
    } else {
        erResult = ERR_EPPI656O_INVALID_HANDLE;
    }
    
    return erResult;
}


T_ERROR_CODE EPPI656Ostart(T_EPPI656O_HANDLE pa_tHndl) {
    T_ERROR_CODE tErr = ERR_NONE;
    T_EPPI656O_INST *pInst = (T_EPPI656O_INST *)pa_tHndl;
    
    if(pInst) {
        *(g_aEPPIspec[pInst->cEPPInr].pDMA0config) |= 0x0001;
        *(g_aEPPIspec[pInst->cEPPInr].pEPPIcontrol) |= 0x0001;
        //ssync();
    }
    
    return tErr;
}


T_ERROR_CODE EPPI656Ostop(T_EPPI656O_HANDLE pa_tHndl) {
    T_ERROR_CODE tErr = ERR_NONE;
    T_EPPI656O_INST *pInst = (T_EPPI656O_INST *)pa_tHndl;
    
    if(pInst) {
        *(g_aEPPIspec[pInst->cEPPInr].pDMA0config) &= ~0x0001;
        *(g_aEPPIspec[pInst->cEPPInr].pEPPIcontrol) &= ~0x0001;
        //ssync();
    }
    
    return tErr;
}

/**
    @brief set a pixel in itu 656 frame
**/
//#pragma section ("L1_code")
void EPPI656OsetPixel (void* pa_tHndl, unsigned char pa_cPlane, unsigned short x, unsigned short y, T_VD_COLOR pa_nColor) {
    register T_EPPI656O_INST *pInst;
    register unsigned long nIndex;
    unsigned long nYUVcolor, nFlagValue;
    unsigned short i, nStartLine;
    unsigned char cU, cV, cY, cRed, cBlue, cGreen, cYnew, cUnew, cVnew;
    
    pInst = (T_EPPI656O_INST*) pa_tHndl;
    
    //calculate position in frame
    //even or odd line
    if(y & 0x1 ) {
        nFlagValue = AV_FIELD_2;

    } else {
        nFlagValue = AV_FIELD_1;
    }
    
    //cylce trough fields to get start of active video
    for(i = 0; i < AV_FLAG_NUM ; i++) {
        if(pInst->tAVstandard.tFlags[i].nFlags == nFlagValue) {
            nStartLine = pInst->tAVstandard.tFlags[i].nLine;
            break;
        }
    } 
    
    //start of Active video
    nIndex = (unsigned long)pInst->pcPlaneBuffer[pa_cPlane] + nStartLine * pInst->tAVstandard.nLineStride; 
    nIndex += (y/2) * pInst->tAVstandard.nLineStride;   // move to correct line
    nIndex += pInst->tAVstandard.nBlankLineSize + x * pInst->tAVstandard.nPixelStride + 8 ;  //move to xpos
   
    if ((nIndex & 0x3) == 0) {
        // we are on a 4 byte boundary
        nYUVcolor = *(unsigned long *)nIndex;
        cU = nYUVcolor & 0xff;
        cV = (nYUVcolor >> 16) & 0xff;
        cY = (nYUVcolor >> 24) & 0xff;
        cRed = pa_nColor & 0xff;
        cGreen = (pa_nColor >> 8) & 0xff; 
        cBlue = (pa_nColor >> 16) & 0xff;
        cYnew = (unsigned char)((2449 * cRed + 4809 * cGreen + 943 * cBlue) >> 13);
        cUnew = cBlue - cYnew + 128;
        cVnew = cRed - cYnew + 128;
        // calc average of both crominanz values
        //cUnew = (cUnew + cU) >> 1;
        //cVnew = (cVnew + cV) >> 1;        
        // set new value
        *(unsigned long *)nIndex = (cY << 24) | (cVnew << 16) | (cYnew << 8) | cUnew;
    } else {
        // we are on a 2 byte boundary    
        nYUVcolor = *(unsigned long *)(nIndex - 2);
        cU = nYUVcolor & 0xff;
        cV = (nYUVcolor >> 16) & 0xff; 
        cY = (nYUVcolor >> 8) & 0xff;
        cRed = pa_nColor & 0xff;
        cGreen = (pa_nColor >> 8) & 0xff; 
        cBlue = (pa_nColor >> 16) & 0xff;
        cYnew = (unsigned char)((2449 * cRed + 4809 * cGreen + 943 * cBlue) >> 13);
        cUnew = cBlue - cYnew + 128;
        cVnew = cRed - cYnew + 128;
        // calc average of both crominanz values
        //cUnew = (cUnew + cU) >> 1;
        //cVnew = (cVnew + cV) >> 1;        
        // set new value
        *(unsigned long *)(nIndex - 2) = (cYnew << 24) | (cVnew << 16) | (cY << 8) | cUnew;        
    }
}


//#pragma section ("L1_code")
T_VD_COLOR EPPI656OgetPixel(void *pa_tHndl, unsigned char pa_cPlane, unsigned short x, unsigned short y) {
    register T_EPPI656O_INST *pInst;
    register unsigned long nIndex ;
    unsigned long nYUVcolor, nFlagValue;
    unsigned short i, nStartLine;
    unsigned char cU, cV, cY;
    signed short nRed, nBlue, nGreen;
    unsigned long nVDcolor;
    
    
    pInst = (T_EPPI656O_INST*) pa_tHndl;
    
    //calculate position in frame
    //even or odd line
    if(y & 0x1 ) {
        nFlagValue = AV_FIELD_2;

    } else {
        nFlagValue = AV_FIELD_1;
    }
    
    //cylce trough fields to get start of active video
    for(i = 0; i < AV_FLAG_NUM ; i++) {
        if(pInst->tAVstandard.tFlags[i].nFlags == nFlagValue) {
            nStartLine = pInst->tAVstandard.tFlags[i].nLine;
            break;
        }
    } 
    
    //start of Active video
    nIndex = (unsigned long)pInst->pcPlaneBuffer[pa_cPlane] + nStartLine * pInst->tAVstandard.nLineStride; 
    nIndex += (y/2) * pInst->tAVstandard.nLineStride;   // move to correct line
    nIndex += pInst->tAVstandard.nBlankLineSize + x * pInst->tAVstandard.nPixelStride + 6 ;  //move to xpos
    
    if ((nIndex & 0x3) == 0) {
        // we are on a 4 byte boundary
        nYUVcolor = *(unsigned long *)nIndex;
        cU = nYUVcolor & 0xff;
        cV = (nYUVcolor >> 16) & 0xff;
        cY = (nYUVcolor >> 24) & 0xff;
        nRed = cV - 128 + cY;
        if (nRed > 255)
            nRed = 255;
        if (nRed < 0  )
            nRed = 0;
        nBlue = cU - 128 + cY;
        if (nBlue > 255)
            nBlue = 255;
        if (nBlue < 0 ) 
            nBlue = 0;
        nGreen = (6922 * cY - 2130 * nRed - 778 * nBlue) >> 12;
        if (nGreen > 255) 
            nGreen = 255;
        if (nGreen < 0 ) 
            nGreen = 0; 
        nVDcolor = (nBlue << 16) | (nGreen << 8) | nRed;
    } else {
        // we are on a 2 byte boundary    
        nYUVcolor = *(unsigned long *)(nIndex - 2);
        cU = nYUVcolor & 0xff;
        cV = (nYUVcolor >> 16) & 0xff; 
        cY = (nYUVcolor >> 8) & 0xff;
        nRed = cV - 128 + cY;
        if (nRed > 255)
            nRed = 255;
        if (nRed < 0  )
            nRed = 0;
        nBlue = cU - 128 + cY;
        if (nBlue > 255)
            nBlue = 255;
        if (nBlue < 0 ) 
            nBlue = 0;
        nGreen = (6922 * cY - 2130 * nRed - 778 * nBlue) >> 12;
        if (nGreen > 255) 
            nGreen = 255;
        if (nGreen < 0 ) 
            nGreen = 0;
        nVDcolor = (nBlue << 16) | (nGreen << 8) | nRed;
    }
    
    return nVDcolor;
}


//#pragma section ("L1_code")
void EPPI656OblendPixel(void *pa_tHndl, unsigned char pa_cPlane, unsigned short x, unsigned short y, T_VD_COLOR pa_nColor, unsigned char pa_cAlpha) {
    unsigned char cRed;
    unsigned char cGreen;
    unsigned char cBlue;
    // fetch the current pixel color
    T_VD_COLOR nColor = EPPI656OgetPixel(pa_tHndl, pa_cPlane, x, y);
    // calc the blend colors
    cRed = ((pa_nColor & 0xff) * pa_cAlpha + (nColor & 0xff) * (255 - pa_cAlpha)) >> 8;
    cGreen = (((pa_nColor & 0xff00) >> 8) * pa_cAlpha + ((nColor & 0xff00) >> 8) * (255 - pa_cAlpha)) >> 8;
    cBlue = (((pa_nColor & 0xff0000) >> 16) * pa_cAlpha + ((nColor & 0xff0000) >> 16) * (255 - pa_cAlpha)) >> 8;
    // set the new pixel color
    EPPI656OsetPixel(pa_tHndl, pa_cPlane, x, y, cRed | (cGreen << 8) | (cBlue << 16));
}



signed char  EPPI656OgetActivePlane(void *pa_tHndl){
    T_EPPI656O_INST *pInst;
    
    if(pa_tHndl) {
        pInst = (T_EPPI656O_INST*) pa_tHndl;
        return pInst->cActivePlane;
    } else {
        return (signed char)ERR_EPPI656O_INVALID_HANDLE;
    }
    
    
}





T_ERROR_CODE EPPI656OsetActivePlane(void *pa_tHndl, unsigned char pa_cPlane){
    T_EPPI656O_INST *pInst;
    
    if(pa_tHndl) {
        pInst = (T_EPPI656O_INST*) pa_tHndl;
        pInst->cNextPlane = pa_cPlane;
        pInst->bPlaneSwitchRequest = true;
        
        return ERR_NONE;
    } else {
        return ERR_EPPI656O_INVALID_HANDLE;
    }
    
    
}



T_ERROR_CODE EPPI656OwaitForFrameSync(void *pa_tHndl) {
    T_EPPI656O_INST *pInst;
    T_ERROR_CODE erResult = ERR_NONE;
    
    if(pa_tHndl) {
        pInst = (T_EPPI656O_INST*) pa_tHndl;
        
        unsigned short nTimeout = EPPI656O_FSYNC_TIMEOUT;
        #ifdef _USE_VDK_         
            VDK_Ticks tSemTimeout = nTimeout / VDK_GetTickPeriod (); 
            VDK_PendSemaphore(pInst->tFsyncSem,tSemTimeout | VDK_kNoTimeoutError);

        #else
            pInst->bVSYNC = false;
            while ((!pInst->bVSYNC) && nTimeout) {
                bs_sleep(1);
                nTimeout --;
            }
            
            if(!nTimeout) {
                erResult = ERR_EPPI656O_FSYNC_TIMEOUT;
            }
            
        #endif
      
    } else {
        erResult = ERR_EPPI656O_INVALID_HANDLE;
    }
    
    return erResult;
}


unsigned char EPPI656OgetNextPlaneNum(void *pa_tHndl) {
    
    T_EPPI656O_INST *pInst = (T_EPPI656O_INST*) pa_tHndl;
    unsigned char cNewValue = pInst->cActivePlane + 1;
    
    return((cNewValue >= pInst->cNumberOfPlanes) ? 0 : cNewValue);
}



T_ERROR_CODE EPPI656OshowFrame(void *pa_tHndl, 
                              unsigned char *pa_pcFrameBuffer, 
                              unsigned short pa_nXoffset, 
                              unsigned short pa_nYoffset, 
                              unsigned short pa_nXsize, 
                              unsigned short pa_nYsize) {
                                  
    T_ERROR_CODE erResult = ERR_NONE;
    
    if(pa_tHndl) {
        T_EPPI656O_INST *pInst = (T_EPPI656O_INST*) pa_tHndl;
        
        // check if shared frames are used
        if(pInst->bUseSharedBuffer) {
            pInst->pcPlaneBuffer[0] = pa_pcFrameBuffer;
            pInst->cActivePlane = 0;
            
            return(erResult);
        }
        
        if((pInst->nFreePlanesCount || pInst->bOverwriteActivePlaneAllowed) && (pa_nXsize == pInst->tAVstandard.nWidth)) {
            unsigned short nNextInputPlane;
            T_AV_FRAME_BUFFER_DESC tFb;
                
            // build a frame buffer with the used video standard
            tFb.width = pInst->tAVstandard.nWidth; 
            tFb.height = pInst->tAVstandard.nHeight;
            tFb.offset = pInst->tAVstandard.nActiveVideo1Offset;
            tFb.pixel_stride = pInst->tAVstandard.nPixelStride;
            tFb.line_stride = pInst->tAVstandard.nLineStride;
            tFb.field_stride = pInst->tAVstandard.nFieldStride;
            tFb.interlaced = 1;
        
            if(pInst->nFreePlanesCount < 0) {
                if(pInst->cNumberOfPlanes > 1) {
                    nNextInputPlane = 1;
                } else {
                    nNextInputPlane = 0;
                }
                pInst->nFreePlanesCount = pInst->cNumberOfPlanes;
            }
            else {
                if(pInst->nFreePlanesCount == 0) {
                    if(pInst->bOverwriteActivePlaneAllowed) {
                        nNextInputPlane = pInst->cActivePlane;
                    }
                    else {
                        return(ERR_EPPI656O_NO_FREE_PLANE);
                    }
                }
                else {
                    nNextInputPlane = pInst->cActivePlane + pInst->cNumberOfPlanes - pInst->nFreePlanesCount;
                    if(nNextInputPlane >= pInst->cNumberOfPlanes) {
                        nNextInputPlane -= pInst->cNumberOfPlanes;
                    }
            
                    // all planes are free
                    if((pInst->nFreePlanesCount == pInst->cNumberOfPlanes) && (pInst->cNumberOfPlanes != 1)) {
                        // get the first plane after the active plane
                        if((++nNextInputPlane) >= pInst->cNumberOfPlanes) {
                            nNextInputPlane -= pInst->cNumberOfPlanes;
                        }
                    }
                }
            }
            
            tFb.start_addr = pInst->pcPlaneBuffer[nNextInputPlane];
                    
            //copy frame
            /*
            erResult = AV_CopyYUV422toITUR656Frame(&tFb, 
                                     pa_nXoffset,
                                     pa_nYoffset,
                                     (char *)pa_pcFrameBuffer, 
                                     pa_nXsize, 
                                     pa_nYsize, 
                                     pInst->cMDMAChannel);
            */
            // check Ysize of the frame to be show
            if(pa_nYsize > tFb.height) {
                pa_nYsize = tFb.height;
            }
            memcpy(pInst->pcPlaneBuffer[nNextInputPlane], pa_pcFrameBuffer, pa_nXsize * pa_nYsize * pInst->tAVstandard.nPixelStride);
            if(pInst->nFreePlanesCount) {                                     
                if((pInst->cNumberOfPlanes == 1) && pInst->bOverwriteActivePlaneAllowed) {
                    pInst->nFreePlanesCount = 1;
                }
                else {
                    pInst->nFreePlanesCount--;
                }
            }
            
            //pInst->cNextInputPlane = EPPI656OgetNewNextInputPlane(pa_tHndl);
        }  
        else {
            erResult = ERR_EPPI656O_NO_FREE_PLANE;
        }      
    } 
    else {
        erResult = ERR_EPPI656O_INVALID_HANDLE;
    }
    
    return erResult;
}
