/** *********************************************************************
 * @file HitachiTX09D.c
 * @author Roland Oberhammer
 * @date 2009-06-18
 * @version 1.5
 *
 * Initializes the HITACHI TX09D70M1CCA TFT display
 *
 *
 * If the framebuffer driver is in landscape format and the display 
 * is in portrait format. Use the HITTX09_LANDSCAPE_FORMAT macro to rotate the frame..
 *
 * If you use timer 5 on a CM-BF537E be sure to have enabled the
 * pin on the core module (soldering the right mount option)
 *
 *
 * @see HitachiTX09D.h
  ********************************************************************* **/
  
#include <stdio.h>
#include <string.h>
#include <environment.h>
#include <cycle_count_bf.h>
#include "../../../../../BLACKSheep/common/syslog/syslog.h"
#include "HitachiTX09D.h"
#include "../../pwrmngt/clockquery.h"
#include "../../gpTimerConfig.h"

#ifdef __ADSPBF548__
#include "../../EPPIconfig.h"
#include "../../EPPI.h"
#else
#include "../../PPIconfig.h"
#include "../../PPI.h"
#endif


// todo: eliminate the need of a global instance
// todo: use descriptors in portrait format as well
static T_HITTX09_INST *g_ptHITTX09globalInst = 0;
// frame complete flag
//volatile unsigned char g_TFT_FrameComplete = 0;

/*
section ("L1_data_a")
unsigned short nBitReverse_Blue []  = { 0x0000, 0x8000, 0x4000, 0xc000, 0x2000, 0xa000, 0x6000, 0xe000, 0x1000, 0x9000, 0x5000, 0xd000, 0x3000,
								 	   0xb000, 0x7000, 0xf000, 0x0800, 0x8800, 0x4800, 0xc800, 0x2800, 0xa800, 0x6800, 0xe800, 0x1800, 0x9800,
									   0x5800, 0xd800, 0x3800, 0xb800, 0x7800, 0xf800 };
section ("L1_data_a")									   
unsigned short nBitReverse_Green [] = { 0x0000, 0x0400, 0x0200, 0x0600, 0x0100, 0x0500, 0x0300, 0x0700, 0x0080, 0x0480, 0x0280, 0x0680, 0x0180,
									   0x0580, 0x0380, 0x0780, 0x0040, 0x0440, 0x0240, 0x0640, 0x0140, 0x0540, 0x0340, 0x0740, 0x00c0, 0x04c0,
									   0x02c0, 0x06c0, 0x01c0, 0x05c0, 0x03c0, 0x07c0, 0x0020, 0x0420, 0x0220, 0x0620, 0x0120, 0x0520, 0x0320, 
									   0x0720, 0x00a0, 0x04a0, 0x02a0, 0x06a0, 0x01a0, 0x05a0, 0x03a0, 0x07a0, 0x0060, 0x0460, 0x0260, 0x0660,
									   0x0160, 0x0560, 0x0360, 0x0760, 0x00e0, 0x04e0, 0x02e0, 0x06e0, 0x01e0, 0x05e0, 0x03e0, 0x07e0 };
section ("L1_data_a")									   
unsigned short nBitReverse_Red []   = { 0x0000, 0x0010, 0x0008, 0x0018, 0x0004, 0x0014, 0x000c, 0x001c, 0x0002, 0x0012, 0x000a, 0x001a, 0x0006,
									   0x0016, 0x000e, 0x001e, 0x0001, 0x0011, 0x0009, 0x0019, 0x0005, 0x0015, 0x000d, 0x001d, 0x0003, 0x0013,
									   0x000b, 0x001b, 0x0007, 0x0017, 0x000f, 0x001f };
									   
*/									   
									   
#if HITTX09_USE_LANDSCAPE_FORMAT
// descriptor list for the dma chain including descriptors for blanking lines
section ("L1_data_a") unsigned long anTFTdmaDescriptorTable[3 * (HITTX09_YSIZE + HITTX09_Y_BLANKING_LINES)];
section ("L1_data_a") unsigned long anTFTdmaDescriptorTable1[3 * (HITTX09_YSIZE + HITTX09_Y_BLANKING_LINES)];
// frame line counter
#endif


#pragma section ("L1_code")
void HITTX09setPixel (T_HITTX09_INST *pa_pInst, unsigned char pa_cPlane, unsigned short x, unsigned short y, T_VD_COLOR pa_nColor) {
    register unsigned short nColor = ((pa_nColor>> 8) & 0xf800) | ((pa_nColor >> 5) & 0x7e0) | ((pa_nColor >> 3) & 0x1f);
#if HITTX09_USE_LANDSCAPE_FORMAT
	*(unsigned short *)(((y * pa_pInst->nYsize + /*y * HITTX09_Y_BLANKING_LINES*/ + x) * 2) + (unsigned long)pa_pInst->pcPlaneBuffer[pa_pInst->cCurrActivePlane]) = nColor;//(pa_cBlue << 11) | (pa_cGreen << 5) | pa_cRed;
#else
	*(unsigned short *)(((y * pa_pInst->nXsize + x) * 2) + (unsigned long)pa_pInst->pnFrameBuffer + HITTX09_Y_BLANKING_LINES * pa_pInst->nXsize * pa_pInst->cBytesPerPixel) = nColor; //(pa_cBlue << 11) | (pa_cGreen << 5) | pa_cRed;
#endif
}

#pragma section ("L1_code")
T_VD_COLOR HITTX09getPixel(T_HITTX09_INST *pa_pInst, unsigned char pa_cPlane, unsigned short x, unsigned short y) {
#if HITTX09_USE_LANDSCAPE_FORMAT
	unsigned short nColor = *(unsigned short *)(((y * pa_pInst->nYsize + /*y * HITTX09_Y_BLANKING_LINES*/ + x) * 2) + (unsigned long)pa_pInst->pcPlaneBuffer[pa_pInst->cCurrActivePlane]);//(pa_cBlue << 11) | (pa_cGreen << 5) | pa_cRed;
#else	
	unsigned short nColor = *(unsigned short *)(((y * pa_pInst->nXsize + x) * 2) + (unsigned long)pa_pInst->pnFrameBuffer + HITTX09_Y_BLANKING_LINES * pa_pInst->nXsize * pa_pInst->cBytesPerPixel); //(pa_cBlue << 11) | (pa_cGreen << 5) | pa_cRed;
#endif
	return	(((nColor & 0xf800) << 8) | ((nColor & 0x7e0) << 5) | ((nColor & 0x1f) << 3));				
}

#pragma section ("L1_code")
void HITTX09blendPixel(T_HITTX09_INST *pa_pInst, unsigned char pa_cPlane, unsigned short x, unsigned short y, T_VD_COLOR pa_nColor, unsigned char pa_cAlpha) {
	unsigned char cRed;
	unsigned char cGreen;
	unsigned char cBlue;
	// fetch the current pixel color
	T_VD_COLOR nColor = HITTX09getPixel(pa_pInst, pa_cPlane, x, y);
	// calc the blend colors
	cRed = ((pa_nColor & 0xff) * pa_cAlpha + (nColor & 0xff) * (255 - pa_cAlpha)) >> 8;
	cGreen = (((pa_nColor & 0xff00) >> 8) * pa_cAlpha + ((nColor & 0xff00) >> 8) * (255 - pa_cAlpha)) >> 8;
	cBlue = (((pa_nColor & 0xff0000) >> 16) * pa_cAlpha + ((nColor & 0xff0000) >> 16) * (255 - pa_cAlpha)) >> 8;
	// set the new pixel color
	HITTX09setPixel(pa_pInst, pa_cPlane, x, y, cRed | (cGreen << 8) | (cBlue << 16));
}

#pragma section ("L1_code")
void HITTX09clear(T_HITTX09_INST *pa_pInst, unsigned char pa_cPlane, T_VD_COLOR pa_nColor) {
    unsigned long nNofElements = (pa_pInst->nXsize * pa_pInst->nYsize * pa_pInst->cBytesPerPixel) / 4;
    unsigned long *pBuf = (unsigned long *)pa_pInst->pcPlaneBuffer[pa_pInst->cCurrActivePlane];
    unsigned long nColor;
	nColor = ((pa_nColor & 0xff) >> 3) | ((pa_nColor & 0xff00) >> 5) | ((pa_nColor & 0xff0000) >> 8);
	nColor |= nColor << 16;
    while (nNofElements--) {
        *pBuf++ = nColor;
    }
}


#pragma section ("L1_code")
void HITTX09fillSurface(T_HITTX09_INST *pa_pInst, unsigned char pa_cPlane, unsigned short pa_nX, unsigned short pa_nY, T_VD_COLOR pa_nColor) {
    register unsigned short nColor = ((pa_nColor>> 8) & 0xf800) | ((pa_nColor >> 5) & 0x7e0) | ((pa_nColor >> 3) & 0x1f);
	register unsigned short nY = pa_nY;
	
#if HITTX09_USE_LANDSCAPE_FORMAT
		register unsigned short *p = (unsigned short *)(((nY * pa_pInst->nYsize + pa_nX) * 2) + (unsigned long)pa_pInst->pcPlaneBuffer[pa_pInst->cCurrActivePlane]);
#else
		register unsigned short *p = (unsigned short *)(((nY * pa_pInst->nXsize + pa_nX) * 2) + (unsigned long)pa_pInst->pcPlaneBuffer[pa_pInst->cCurrActivePlane] + HITTX09_Y_BLANKING_LINES * pa_pInst->nXsize * pa_pInst->cBytesPerPixel);
#endif
	// go downwards
    while (*p != nColor) {
        while (*p != nColor) {
            *p-- = nColor;
        }
#if HITTX09_USE_LANDSCAPE_FORMAT
		p = (unsigned short *)(((nY * pa_pInst->nYsize + pa_nX + 1) * 2) + (unsigned long)pa_pInst->pcPlaneBuffer[pa_pInst->cCurrActivePlane]);
#else
		p = (unsigned short *)(((nY * pa_pInst->nXsize + pa_nX + 1) * 2) + (unsigned long)pa_pInst->pcPlaneBuffer[pa_pInst->cCurrActivePlane] + HITTX09_Y_BLANKING_LINES * pa_pInst->nXsize * pa_pInst->cBytesPerPixel);
#endif
        while (*p != nColor) {
            *p++ = nColor;
        }
        nY ++;
#if HITTX09_USE_LANDSCAPE_FORMAT
		p = (unsigned short *)(((nY * pa_pInst->nYsize + pa_nX) * 2) + (unsigned long)pa_pInst->pcPlaneBuffer[pa_pInst->cCurrActivePlane]);
#else
		p = (unsigned short *)(((nY * pa_pInst->nXsize + pa_nX) * 2) + (unsigned long)pa_pInst->pcPlaneBuffer[pa_pInst->cCurrActivePlane] + HITTX09_Y_BLANKING_LINES * pa_pInst->nXsize * pa_pInst->cBytesPerPixel);
#endif
    }

    // go upwards   
    nY = pa_nY - 1;
#if HITTX09_USE_LANDSCAPE_FORMAT
	p = (unsigned short *)(((nY * pa_pInst->nYsize + pa_nX) * 2) + (unsigned long)pa_pInst->pcPlaneBuffer[pa_pInst->cCurrActivePlane]);
#else
	p = (unsigned short *)(((nY * pa_pInst->nXsize + pa_nX) * 2) + (unsigned long)pa_pInst->pcPlaneBuffer[pa_pInst->cCurrActivePlane] + HITTX09_Y_BLANKING_LINES * pa_pInst->nXsize * pa_pInst->cBytesPerPixel);
#endif
    while (*p != nColor) {
        while (*p != nColor) {
            *p-- = nColor;
        }
#if HITTX09_USE_LANDSCAPE_FORMAT
		p = (unsigned short *)(((nY * pa_pInst->nYsize + pa_nX + 1) * 2) + (unsigned long)pa_pInst->pcPlaneBuffer[pa_pInst->cCurrActivePlane]);
#else
		p = (unsigned short *)(((nY * pa_pInst->nXsize + pa_nX + 1) * 2) + (unsigned long)pa_pInst->pcPlaneBuffer[pa_pInst->cCurrActivePlane] + HITTX09_Y_BLANKING_LINES * pa_pInst->nXsize * pa_pInst->cBytesPerPixel);
#endif
        while (*p != nColor) {
            *p++ = nColor;
        }
        nY --;
#if HITTX09_USE_LANDSCAPE_FORMAT
		p = (unsigned short *)(((nY * pa_pInst->nYsize + pa_nX) * 2) + (unsigned long)pa_pInst->pcPlaneBuffer[pa_pInst->cCurrActivePlane]);
#else
		p = (unsigned short *)(((nY * pa_pInst->nXsize + pa_nX) * 2) + (unsigned long)pa_pInst->pcPlaneBuffer[pa_pInst->cCurrActivePlane] + HITTX09_Y_BLANKING_LINES * pa_pInst->nXsize * pa_pInst->cBytesPerPixel);
#endif
    }
}



/**
 *	@private
 *	@brief		Interrupt Request Handler for HitachiTX09D
 *	
 *	@param		*pa_pClientArg		A structure detailing information about the asserted interrupt. Of type T_PPI_SPEC/T_EPPI_SPEC
 *	
 *	@see		T_PPI_SPEC/T_EPPI_SPEC definition.
 *
 **/
#pragma section("L1_code")
static void HITTX09intHandler(void *pa_pClientArg) {

    //plane switch request by VDM?
    if(g_ptHITTX09globalInst->bPlaneSwitchRequest) {
        // switch plane
        g_ptHITTX09globalInst->cCurrActivePlane = g_ptHITTX09globalInst->ucNextPlane;
        g_ptHITTX09globalInst->bPlaneSwitchRequest = false;
        
    } else {
        // todo: implement auto switching
    }
}



/**
 *	@private
 *	@brief		Error Interrupt Handler for HitachiTX09D
 *	
 *	@param		*pa_pClientArg		A structure detailing information about the asserted interrupt. Of type T_PPI_SPEC/T_EPPI_SPEC
 *	
 *	@see		T_PPI_SPEC/T_EPPI_SPEC definition.
 *
 **/
#pragma section("L1_code")
static void HITTX09errorIntHandler(void *pa_pClientArg) {
#ifdef __ADSPBF548__
    T_EPPI_SPEC *pInst = (T_EPPI_SPEC *) pa_pClientArg;
#else
    T_PPI_SPEC *pInst = (T_PPI_SPEC *) pa_pClientArg;
#endif
    
    //currently nothing to do

#if _HITTX09_DEBUG_LVL_ > 0
#ifdef __ADSPBF548__
    if(*(pInst->pEPPIstatus)) {
        char acMessage[100];
        sprintf(acMessage, "HITTX09 EPPI Error occured, EPPIstatus: 0x%8.8x\n", (unsigned long) *(pInst->pEPPIstatus));
        syslog_post(acMessage, 0, 0, NULL);
    }
    if(*(pInst->pDMA0irqStat) & 0x0006){
        char acMessage[100];
        sprintf(acMessage, "HITTX09 DMA Error occured, DMA0irqStat: 0x%8.8x\n", (unsigned long) *(pInst->pDMA0irqStat));
        syslog_post(acMessage, 0, 0, NULL);
    }
#endif  //#ifdef __ADSPBF548__
#endif  //#if _HITTX09_DEBUG_LVL_ > 0

#ifdef _HITTX09_RESET_ON_ERROR_
#ifdef __ADSPBF548__
    eppi_disable(pInst->cEPPInr);
    eppi_enable(pInst->cEPPInr);
#endif //#ifdef __ADSPBF548__
#endif //#ifdef _HITTX09_RESET_ON_ERROR_
    
}



// sleep function
static void HITTX09msSleep (unsigned long pa_nMs, unsigned long pa_nCoreClk) {
	unsigned long long int cur,nd;
	
	_GET_CYCLE_COUNT(cur);
	nd = cur + pa_nCoreClk / 1000 * pa_nMs;
	while (cur < nd) {
		_GET_CYCLE_COUNT(cur);
	}
}

void HITTX09setupFrameBufferUpdateProcess (T_HITTX09_INST *pa_pInst, unsigned long *pa_nWorkingBuffer0, unsigned long *pa_nWorkingBuffer1, void *pa_Generic) {
	pa_pInst->pnWorkingBuffer0 = pa_nWorkingBuffer0;
	pa_pInst->pnWorkingBuffer1 = pa_nWorkingBuffer1;
	pa_pInst->cCurrentWorkingBuffer = 0;	
}

void HITTX09setWorkingBuffer (T_HITTX09_INST *pa_pInst, unsigned char pa_cWorkingBuffer) {
	pa_pInst->cCurrentWorkingBuffer = pa_cWorkingBuffer;
}

// sets the brithness
void HITTX09setBrightness (T_HITTX09_INST *pa_pInst, unsigned short pa_cBrightness) {
	if (pa_cBrightness > HITTX09_MAX_BRIGTHNESS) {
		pa_cBrightness = HITTX09_MAX_BRIGTHNESS;
	}
}

void HITTX09setup(void) {
    // currently nothing to do
}

void HITTX09cleanup(void) {
    // currently nothing to do
}


void HITTX09close(T_HITTX09_HNDL hDev) {
    T_HITTX09_INST *pInst = (T_HITTX09_INST *)hDev;
    
    timer_close(pInst->tDclkTimerHndl);
    timer_close(pInst->tHsyncTimerHndl);
    timer_close(pInst->tDtmgTimerHndl);
    timer_close(pInst->tDtmgVsyncTimerHndl);
#ifdef __ADSPBF548__
    eppi_close(pInst->cPPI);
#else
    ppi_close(pInst->cPPI);
#endif
    free(pInst->pcPlaneBuffer[0]);
    free(pInst);
}


// sets up the timer and ppi/eppi for the HITTX09-display
T_HITTX09_HNDL HITTX09open (	T_HITTX09_CONFIG *pa_tConf,
                            	unsigned char ***pa_pcPlaneBuffer,
                            	void ***pa_pGraficFunctions,
                                T_ERROR_CODE *pa_tErr,
        						void *pa_pReserved) {
							
	*pa_tErr = ERR_NONE;
	T_HITTX09_HNDL tHndl = 0;
	int i = 0;
	
	T_HITTX09_INST *pInst = malloc (sizeof (T_HITTX09_INST) );
	
	if (pInst) {
		
		for (i=0; i<VDM_MAX_NOF_GRAFIC_FUNCTIONS; i++){
			pInst->pfGraficFunctions[i] = 0;
		}
	
	    tHndl = (T_HITTX09_HNDL)pInst;
	    // set default values if not set by user
/*	    if (pa_tConf->nXsize == 0) {
	        pa_tConf->nXsize = HITTX09_DEFAULT_XSIZE;
	    }
	    if (pa_tConf->nYsize == 0) {
	        pa_tConf->nYsize = HITTX09_DEFAULT_YSIZE;
	    }*/
	    
	    pa_tConf->cBytesPerPixel = HITTX09_DEFAULT_BYTE_PER_PIXEL;
	    
	    if (pa_tConf->cNofPlanes > VDM_MAX_NOF_PLANES) {
	        pa_tConf->cNofPlanes = VDM_MAX_NOF_PLANES;
	    }
	    
       	pInst->cBytesPerPixel = HITTX09_DEFAULT_BYTE_PER_PIXEL;
        pInst->nXsize = HITTX09_XSIZE;
        pInst->nYsize = HITTX09_YSIZE;
        pInst->ucNofPlanes = pa_tConf->cNofPlanes;
        pInst->cCurrActivePlane = 0;
        pInst->bPlaneSwitchRequest = false;
        g_ptHITTX09globalInst = pInst;
        	
	    // allocate memory for the planes
	    pInst->pcPlaneBuffer[0] = (unsigned char *)malloc (pa_tConf->cNofPlanes * pa_tConf->cBytesPerPixel * pInst->nXsize * pInst->nYsize);
	    
	    if(pa_tConf->nBckGrndImage!=NULL){  //Image available
	        memcpy(pInst->pcPlaneBuffer[0],pa_tConf->nBckGrndImage,HITTX09_YSIZE * HITTX09_XSIZE * HITTX09_DEFAULT_BYTE_PER_PIXEL);
	    } else {    //draw colored screen (set color in environment.h)
	        memset(pInst->pcPlaneBuffer[0],pa_tConf->nDefColor,HITTX09_YSIZE * HITTX09_XSIZE * HITTX09_DEFAULT_BYTE_PER_PIXEL);
	    }
	    
	    if (pInst->pcPlaneBuffer[0]) {
    	    int i = 0;
    	    for (i=0; i<pa_tConf->cNofPlanes; i++) {
    	        pInst->pcPlaneBuffer[i] = (unsigned char *)(pInst->pcPlaneBuffer[0] + i * pInst->nXsize * pInst->nYsize * pInst->cBytesPerPixel);
//    			*pa_pcPlaneBuffer[i] = pInst->pcPlaneBuffer[i];
    	    }
    	    *pa_pcPlaneBuffer = pInst->pcPlaneBuffer;
	    
        	// filling the instance
        	pInst->bOverlayCapability = pa_tConf->bOverlayCapability;
        	pInst->bUpdateOverlayBuffer = false;
	        pInst->cCurrActivePlane = 0;
	        pInst->nSclk = pa_tConf->nSystemClk;
	        pInst->nCclk = pa_tConf->nCoreClk;
	        pInst->cPCIflag = pa_tConf->cPCIflag;
	        if (pInst->nSclk == 0) {
	            //pInst->nSclk = 
	        }
	        if (pInst->nCclk == 0) {
	            //pInst->nSclk = 
	        }
#ifndef __ADSPBF548__
        	if (pa_tConf->bCreateDCLK) {
        		// create the dclk clock output (usually 5.33Mhz)
        		float fSCKLPeriod = 1.0 / (float)(pInst->nSclk);
        		float fTFTClockPeriod = 1.0 / (float)(HITTX09_CLOCK_FREQ);
        		unsigned long nTimerPeriod = (unsigned long)(fTFTClockPeriod / fSCKLPeriod);
        		unsigned long nTimerWidth = nTimerPeriod / 2;
        		
            	T_GP_TIMER_SETUP_PARAM st_TimerSetup;
            	
            	st_TimerSetup.nTimerNr        = pa_tConf->cDCLKtimer;
            	st_TimerSetup.nConfig         = HITTX09_DCLK_TIMER_CONFIG;
            	st_TimerSetup.nWidth          = nTimerWidth;
            	st_TimerSetup.nPeriod         = nTimerPeriod;
            	st_TimerSetup.nIVG            = 0;
                st_TimerSetup.cTMRCLKinputPin = 0;
            	st_TimerSetup.bPositivePulse  = true;
            	st_TimerSetup.nSystemClk      = pInst->nSclk;
            	st_TimerSetup.erResult        = 0;
            	st_TimerSetup.fnCallback      = 0;
            	st_TimerSetup.pCallbackArg    = 0;       		
        		pInst->tDclkTimerHndl = timer_gp_setup (&st_TimerSetup);
        	}
#endif
        	// configure the timer for the pwm output ( max brigthness)
        	/*float fSCKLPeriod = 1.0 / (float)(pInst->nSclk);
        	float fTFTClockPeriod = 1.0 / (float)(HITTX09_PWM_FREQ);
        	unsigned long nTimerPeriod = (unsigned long)(fTFTClockPeriod / fSCKLPeriod);
        	unsigned long nTimerWidth = nTimerPeriod;	// 100% duty cycle
        	
        	T_GP_TIMER_SETUP_PARAM st_TimerSetup;
            	
        	st_TimerSetup.nTimerNr        = pa_tConf->cPWMtimer;
        	st_TimerSetup.nConfig         = HITTX09_PWM_TIMER_CONFIG;
        	st_TimerSetup.nWidth          = nTimerWidth;
        	st_TimerSetup.nPeriod         = nTimerPeriod;
        	st_TimerSetup.nIVG            = 0;
            st_TimerSetup.cTMRCLKinputPin = 0;
        	st_TimerSetup.bPositivePulse  = true;
        	st_TimerSetup.nSystemClk      = pInst->nSclk;
        	st_TimerSetup.erResult        = 0;
        	st_TimerSetup.fnCallback      = 0;
        	st_TimerSetup.pCallbackArg    = 0;        	
        	
        	//T_GP_TIMER_SETUP_PARAM st_TimerSetup = {pa_tConf->cPWMtimer, HITTX09_PWM_TIMER_CONFIG, nTimerWidth, nTimerPeriod, 0, 0, true, pInst->nSclk,0, 0, 0};
        	pInst->tPwmTimerHndl = timer_gp_setup(&st_TimerSetup);
			*/
			// configure gpio for PWM pin	
        	gpio_becomeOutput (HITTX09_PWM_PIN);
			gpio_set(HITTX09_PWM_PIN);
        	// configure gpio for pci pin	
        	gpio_becomeOutput (pa_tConf->cPCIflag);
 
        	// define the timer used for hsync and dtmg depending on the ppi whitch the display is connected to
        	unsigned char cHSYNCtimer;
        	unsigned char cDTMGtimer;

#if defined (__ADSPBF548__)
        		//cHSYNCtimer = 0;
        		//cDTMGtimer = 1;
        		unsigned char cEPPI = 1;
#else   //#if defined (__ADSPBF548__)

        	if (pa_tConf->cPPI == 0) {
        #if defined (__ADSPBF561__)
        		cHSYNCtimer = 8;
        		cDTMGtimer = 9;
        #elif defined (__ADSPBF537__)
        		cHSYNCtimer = 0;
        		cDTMGtimer = 1;
        #elif defined (__ADSPBF527__)
        		cHSYNCtimer = 0;
        		cDTMGtimer = 1;
        #elif defined (__ADSPBF518__)
        		cHSYNCtimer = 0;
        		cDTMGtimer = 1;       	
        #else
        #error "processor not yet supported"
        #endif
        	} else if (pa_tConf->cPPI == 1) {
        		cHSYNCtimer = 10;
        		cDTMGtimer = 11;
        	} else {
        		*pa_tErr = ERR_HITTX09_ILLEGAL_PPI;
        		return 0;
        	}
       	
            pInst->cPPI = pa_tConf->cPPI;
#endif   //#if defined (__ADSPBF548__)
       		
        	// setup dma and ppi
#if HITTX09_USE_LANDSCAPE_FORMAT

        	// fill the descriptor table
        	//if (pa_Reserved) {
        		//unsigned short i = 0;
////////////////////////////////////////////////  ORIGINAL /////////////////////////////////////////////////////////////

        		for (i=0; i<pInst->nYsize + HITTX09_Y_BLANKING_LINES - 1; i++) {
        			// next descriptor pointer		
        			anTFTdmaDescriptorTable[3 * i] = (unsigned long)&anTFTdmaDescriptorTable[3 * i + 3];
        			// next startaddres
        			if (i<HITTX09_Y_BLANKING_LINES) {
        				// blanking lines
        				anTFTdmaDescriptorTable[3 * i + 1] = (unsigned long)pInst->pcPlaneBuffer[0];
        			} else {
        				// visible lines
        				anTFTdmaDescriptorTable[3 * i + 1] = (unsigned long)pInst->pcPlaneBuffer[0] + 2 * (pInst->nYsize +  - 1 - (i - HITTX09_Y_BLANKING_LINES));
        			}
        			// dma config register
        			anTFTdmaDescriptorTable[3 * i + 2] = (unsigned long)(0x7505 | ((unsigned short)pInst->nXsize << 15));	// config with no interrupt
        		}
        		// last descriptor points to first
        		anTFTdmaDescriptorTable[3 * (pInst->nYsize + HITTX09_Y_BLANKING_LINES - 1)] = (unsigned long)&anTFTdmaDescriptorTable[0];
        		anTFTdmaDescriptorTable[3 * (pInst->nYsize + HITTX09_Y_BLANKING_LINES - 1) + 1] = (unsigned long)pInst->pcPlaneBuffer[0];
        		anTFTdmaDescriptorTable[3 * (pInst->nYsize + HITTX09_Y_BLANKING_LINES - 1) + 2] = (unsigned long)(0x7585 | ((unsigned short)pInst->nXsize << 15));		// config with interrupt 

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        		for (i=0; i<pInst->nYsize + HITTX09_Y_BLANKING_LINES - 1; i++) {
        			// next descriptor pointer		
        			anTFTdmaDescriptorTable1[3 * i] = (unsigned long)&anTFTdmaDescriptorTable1[3 * i + 3];
        			// next startaddres
        			if (i<HITTX09_Y_BLANKING_LINES) {
        				// blanking lines
        				anTFTdmaDescriptorTable1[3 * i + 1] = (unsigned long)pInst->pcPlaneBuffer[1];
        			} else {
        				// visible lines
        				anTFTdmaDescriptorTable1[3 * i + 1] = (unsigned long)pInst->pcPlaneBuffer[1] + 2 * (pInst->nYsize +  - 1 - (i - HITTX09_Y_BLANKING_LINES));
        			}
        			// dma config register
        			anTFTdmaDescriptorTable1[3 * i + 2] = (unsigned long)(0x7505 | ((unsigned short)pInst->nXsize << 15));	// config with no interrupt
        		}
        		// last descriptor points to first
        		anTFTdmaDescriptorTable1[3 * (pInst->nYsize + HITTX09_Y_BLANKING_LINES - 1)] = (unsigned long)&anTFTdmaDescriptorTable1[0];
        		anTFTdmaDescriptorTable1[3 * (pInst->nYsize + HITTX09_Y_BLANKING_LINES - 1) + 1] = (unsigned long)pInst->pcPlaneBuffer[1];
        		anTFTdmaDescriptorTable1[3 * (pInst->nYsize + HITTX09_Y_BLANKING_LINES - 1) + 2] = (unsigned long)(0x7585 | ((unsigned short)pInst->nXsize << 15));		// config with interrupt 

/*
        		for (i=0; i<pInst->nYsize - 1; i++) {
        			// next descriptor pointer		
        			anTFTdmaDescriptorTable[3 * i] = (unsigned long)&anTFTdmaDescriptorTable[3 * i + 3];
        			// next startaddres
        			// visible lines
        			anTFTdmaDescriptorTable[3 * i + 1] = (unsigned long)(pInst->pcPlaneBuffer[pInst->cCurrActivePlane] + HITTX09_DEFAULT_BYTE_PER_PIXEL * (pInst->nYsize - i - 1));

        			// dma config register
        			anTFTdmaDescriptorTable[3 * i + 2] = (unsigned long)(0x7505 | ((unsigned short)pInst->nXsize << 15));	// config with no interrupt
        		}
        		// last descriptor points to first
        		anTFTdmaDescriptorTable[3 * (pInst->nYsize - 1)] = (unsigned long)&anTFTdmaDescriptorTable[0];
        		anTFTdmaDescriptorTable[3 * (pInst->nYsize - 1) + 1] = (unsigned long)pInst->pcPlaneBuffer[pInst->cCurrActivePlane];
        		anTFTdmaDescriptorTable[3 * (pInst->nYsize - 1) + 2] = (unsigned long)(0x7585 | ((unsigned short)pInst->nXsize << 15));		// config with interrupt 
*/

        	/*} else {
        		unsigned short i = 0;
        		for (i=0; i<pInst->nYsize + HITTX09_Y_BLANKING_LINES - 1; i++) {
        			// next descriptor pointer		
        			anTFTdmaDescriptorTable[2 * i] = (unsigned long)&anTFTdmaDescriptorTable[2 * i + 2];
        			// next startaddres
        			if (i<HITTX09_Y_BLANKING_LINES) {
        				// blanking lines
        				anTFTdmaDescriptorTable[2 * i + 1] = (unsigned long)pInst->pnFrameBuffer;			
        			} else {
        				// visible lines
        				anTFTdmaDescriptorTable[2 * i + 1] = (unsigned long)pInst->pnFrameBuffer + 2 * (pa_nYsize - 1 - (i - HITTX09_Y_BLANKING_LINES));
        			}
        		}
        		// last descriptor points to first
        		anTFTdmaDescriptorTable[2 * (pInst->nYsize + HITTX09_Y_BLANKING_LINES - 1)] = (unsigned long)&anTFTdmaDescriptorTable[0];
        		anTFTdmaDescriptorTable[2 * (pInst->nYsize + HITTX09_Y_BLANKING_LINES - 1) + 1] = (unsigned long)pInst->pnFrameBuffer;
        	}	*/
        	// setup ppi and dma

        	/*if (pInst->bOverlayCapability) {
        			erResult = ppi_setup_gp (	pInst->cPPI,
        										(unsigned long)pInst->pcPlaneBuffer[pInst->cCurrActivePlane],
        										0x381f,
        										0,
        										pInst->nXsize - 1,
        										27,
        										//0x7405,			// interrupt disable
        										0x7505,				// interrupt enable, continous transition
        										pInst->nXsize,
        										2 * (pInst->nYsize),
        										0,
        										0,
        										(unsigned long)&anTFTdmaDescriptorTable[0],
        										//0);
        										(T_PPI_CALLBACK)HITTX09overlayUpdateHandler);	// call overlay update callback
        	} else {*/
        		//if (pa_Reserved) {
#ifdef __ADSPBF548__
                    T_EPPI_CONF tConf = {   pInst->nXsize,
                                            22,
                                            pInst->nYsize,
                                            5,
                                            pInst->nYsize,
                                            pInst->nXsize,
                                            (unsigned short)(((pInst->nSclk / HITTX09_CLOCK_FREQ) - 1) / 2),
                                            0x0002762e,                     // 
                                            5,
                                            273,
                                            0x01 * 273,
                                            327 * 273, //327,
                                            0xff00ff00,
                                            HITTX09_DMA_CONFIG & 0xfffe,                         // int disabled, continous transition, 16 bit
                                            0,
                                            (signed short) (pInst->nYsize * HITTX09_DEFAULT_BYTE_PER_PIXEL),
                                            pInst->nXsize,
                                            0,
                                            0,
                                            (void *)&anTFTdmaDescriptorTable[0],
                                            (T_EPPI_CALLBACK)HITTX09intHandler,
                                            (T_EPPI_CALLBACK)HITTX09errorIntHandler,
                                            (T_EPPI_CALLBACK)HITTX09errorIntHandler,
                                            0,
                                            0,
                                            0,
                                            0,
                                            0,
                                            0,
                                            0,
                                            0,
                                            0,
                                            0,
                                            cEPPI,
                                            0,
                                            0,
                                            0,
                                            0,
                                            0,
                                            0};
                    
                    *pa_tErr = eppi_open(&tConf);
#else   // #ifdef __ADSPBF548__
        			*pa_tErr = ppi_setup_gp (	pInst->cPPI,
        										0,
        										0x381f,
        										0,
        										pInst->nXsize - 1,
        										22,
        										//0x7405,			// interrupt disable
        										0x7505,				// interrupt enable, continous transition
        										pInst->nXsize,
        										2 * (pInst->nYsize/* + HITTX09_Y_BLANKING_LINES*/),
        										0,
        										0,
        										(unsigned long)&anTFTdmaDescriptorTable[0],
        										//0);
        										(T_PPI_CALLBACK)HITTX09intHandler);	// call user callback
        		/*} else {
        			erResult = ppi_setup_gp (	pInst->cPPI,
        										(unsigned long)pInst->pcPlaneBuffer[pInst->cCurrActivePlane],
        										0x381f,
        										0,
        										pInst->nXsize - 1,
        										27,
        										0x7405,			// interrupt disable
        										pInst->nXsize,
        										2 * (pInst->nYsize),
        										0,
        										0,
        										(unsigned long)&anTFTdmaDescriptorTable[0],
        										0);
        		}*/
        //	}
#endif   // #ifdef __ADSPBF548__

#else	// #if HITTX09_USE_LANDSCAPE_FORMAT	

#ifdef __ADSPBF548__
                    T_EPPI_CONF tConf = {   pInst->nXsize,
                                            22,
                                            pInst->nYsize,
                                            5,
                                            pInst->nYsize,
                                            pInst->nXsize,
                                            (unsigned short)(((pInst->nSclk / HITTX09_CLOCK_FREQ) - 1) / 2),
                                            0x0002762e,                     // 
                                            5,
                                            273,
                                            0x01 * 273,
                                            327 * 273, //327,
                                            0xff00ff00,
                                            0x1014,                         // int disabled, continous transition, 16 bit
                                            (void *)(pInst->pcPlaneBuffer[pInst->cCurrActivePlane]),
                                            HITTX09_DEFAULT_BYTE_PER_PIXEL,
                                            pInst->nXsize,
                                            HITTX09_DEFAULT_BYTE_PER_PIXEL,
                                            pInst->nYsize,
                                            0,
                                            (T_EPPI_CALLBACK)HITTX09intHandler,
                                            (T_EPPI_CALLBACK)HITTX09errorIntHandler,
                                            (T_EPPI_CALLBACK)HITTX09errorIntHandler,
                                            0,
                                            0,
                                            0,
                                            0,
                                            0,
                                            0,
                                            0,
                                            0,
                                            0,
                                            0,
                                            cEPPI,
                                            0,
                                            0,
                                            0,
                                            0,
                                            0,
                                            0};
                    
                    *pa_tErr = eppi_open(&tConf);
#else   // #ifdef __ADSPBF548__
        	erResult = ppi_setup_gp (	pInst->cPPI,
        								(unsigned long)(pInst->pcPlaneBuffer[pInst->cCurrActivePlane]),
        								0x381f,
        								0,
        								pInst->nXsize - 1,
        								27,
        								0x1055,		// interrupts disable
        								pInst->nXsize,
        								2,
        								pInst->nYsize,
        								2,
        								0,
        								0);
#endif   // #ifdef __ADSPBF548__
#endif	//#if HITTX09_USE_LANDSCAPE_FORMAT								

        	if (*pa_tErr != ERR_NONE) {
        	    //*pa_tErr = erResult;
        		return 0;
        	}

        /*	// hook dma error interrupt
        	unsigned long nPPIerrorIVG = 0;
        	if (pa_nErrorIVG != 0) {
        	    nPPIerrorIVG = pa_nErrorIVG;
        	    adi_int_SICSetIVG(g_aPPIspec[pa_cPPI].errorIntId, nPPIerrorIVG);
            } else {
                adi_int_SICGetIVG(g_aPPIspec[pa_cPPI].errorIntId, &nPPIerrorIVG); 
            }
            if( adi_int_CECHook( nPPIerrorIVG, tftPPIerrorHandler, 0, TRUE ) != ADI_INT_RESULT_SUCCESS) {
                return ERR_GENERIC;
            }*/

#ifndef __ADSPBF548__
        	// setup hsync timing signal
        	T_GP_TIMER_SETUP_PARAM st_hsyncTimerSetup;
            	
        	st_hsyncTimerSetup.nTimerNr        = cHSYNCtimer;
        	st_hsyncTimerSetup.nConfig         = HITTX09_HSYNC_TIMER_CONFIG;
        	st_hsyncTimerSetup.nWidth          = 5;
        	st_hsyncTimerSetup.nPeriod         = 273;
        	st_hsyncTimerSetup.nIVG            = 0;
            st_hsyncTimerSetup.cTMRCLKinputPin = 0;
        	st_hsyncTimerSetup.bPositivePulse  = false;
        	st_hsyncTimerSetup.nSystemClk      = pInst->nSclk;
        	st_hsyncTimerSetup.erResult        = 0;
        	st_hsyncTimerSetup.fnCallback      = 0;
        	st_hsyncTimerSetup.pCallbackArg    = 0;
         	pInst->tHsyncTimerHndl = timer_gp_setup(&st_hsyncTimerSetup);//{cHSYNCtimer, HITTX09_HSYNC_TIMER_CONFIG, 5,273, 0,0, false, pInst->nSclk,0, 0, 0});
        	
         	// setup dtmg timing signal
        	T_GP_TIMER_SETUP_PARAM st_dtmgTimerSetup;
            	
        	st_dtmgTimerSetup.nTimerNr        = cDTMGtimer;
        	st_dtmgTimerSetup.nConfig         = HITTX09_DTMG_TIMER_CONFIG;
        	st_dtmgTimerSetup.nWidth          = 33;
        	st_dtmgTimerSetup.nPeriod         = 273;
        	st_dtmgTimerSetup.nIVG            = 0;
            st_dtmgTimerSetup.cTMRCLKinputPin = 0;
        	st_dtmgTimerSetup.bPositivePulse  = false;
        	st_dtmgTimerSetup.nSystemClk      = pInst->nSclk;
        	st_dtmgTimerSetup.erResult        = 0;
        	st_dtmgTimerSetup.fnCallback      = 0;
        	st_dtmgTimerSetup.pCallbackArg    = 0;
        	pInst->tDtmgTimerHndl = timer_gp_setup(&st_dtmgTimerSetup);//{cDTMGtimer, HITTX09_DTMG_TIMER_CONFIG, 33, 273, 0,0, false, pInst->nSclk,0, 0, 0});

        	// setup help timer for vsync period in dtmg signal
        	T_GP_TIMER_SETUP_PARAM st_dtmgVsyncTimerSetup;
        	st_dtmgVsyncTimerSetup.nTimerNr        = pa_tConf->cDTMGshiftTimer;
        	st_dtmgVsyncTimerSetup.nConfig         = HITTX09_DTMG_TIMER_CONFIG;
        	st_dtmgVsyncTimerSetup.nWidth          = 1911;
        	st_dtmgVsyncTimerSetup.nPeriod         = 89271;
        	st_dtmgVsyncTimerSetup.nIVG            = 0;
            st_dtmgVsyncTimerSetup.cTMRCLKinputPin = 0;
        	st_dtmgVsyncTimerSetup.bPositivePulse  = false;
        	st_dtmgVsyncTimerSetup.nSystemClk      = pInst->nSclk;
        	st_dtmgVsyncTimerSetup.erResult        = 0;
        	st_dtmgVsyncTimerSetup.fnCallback      = 0;
        	st_dtmgVsyncTimerSetup.pCallbackArg    = 0;
        	pInst->tDtmgVsyncTimerHndl = timer_gp_setup(&st_dtmgVsyncTimerSetup);//{pa_tConf->cDTMGshiftTimer, HITTX09_DTMG_TIMER_CONFIG, 1911, 89271, 0, 0, false, pInst->nSclk,0, 0, 0});

           	// enable pci
        	gpio_set (pInst->cPCIflag);
        	asm ("ssync;");
        	// enable dtmg timer
        	timer_enable (pInst->tDtmgTimerHndl);
        	// enable help timer for vsync period in dtmg signal
        	timer_enable (pInst->tDtmgVsyncTimerHndl);
        	// enable hsync timer
        	timer_enable (pInst->tHsyncTimerHndl);
	
        	if (pa_tConf->bCreateDCLK) {	
        		// enable 5.33 Mhz clock
        		timer_enable (pInst->tDclkTimerHndl);
        	}
#endif

#ifdef __ADSPBF548__
            eppi_enable(cEPPI);
#else
            //ppi_enable(pInst->cPPI);
#endif
        	// wait at least one frame
        	HITTX09msSleep (50, pInst->nCclk);
	
        	// enable the pwm timer
        	//timer_enable (pInst->tPwmTimerHndl);

        	// set the grafic functions for the vdm
        	pInst->pfGraficFunctions[VD_SET_PIXEL] = (void *)HITTX09setPixel;
        	pInst->pfGraficFunctions[VD_GET_PIXEL] = (void *)HITTX09getPixel;
        	pInst->pfGraficFunctions[VD_BLEND_PIXEL] = (void *)HITTX09blendPixel;
        	pInst->pfGraficFunctions[VD_CLEAR_SCREEN] = (void *)HITTX09clear;
        	pInst->pfGraficFunctions[VD_FILL_SURFACE] = (void *)HITTX09fillSurface;
        	pInst->pfGraficFunctions[VD_OVERLAY_BUFFER] = (void *)HITTX09updateOverlayBuffer;
        	pInst->pfGraficFunctions[VD_SHOW_FRAME] = (void *)HITTX09showFrame;
        	pInst->pfGraficFunctions[VD_GET_NEXT_PLANE] = (void *)HITTX09getNextPlaneNum;
        	pInst->pfGraficFunctions[VD_SET_ACTIVE_PLANE] = (void *)HITTX09setActivePlane;
	
        	*pa_pGraficFunctions = &(pInst->pfGraficFunctions[0]);
	
        	return tHndl;

	    } else {
    	    *pa_tErr = ERR_HITTX09_NOT_ENOUGH_MEM;
    	    return 0;
	    }    	
	} else {
	    *pa_tErr = ERR_HITTX09_NOT_ENOUGH_MEM;
	    return 0;
	}
}

unsigned long HITTX09getCurrPlanebuffer (T_HITTX09_INST *pa_pInst) {
	return (unsigned long)(pa_pInst->pcPlaneBuffer[pa_pInst->cCurrActivePlane]);
}


T_ERROR_CODE HITTX09updateOverlayBuffer (T_HITTX09_INST *pa_pInst, unsigned char *pa_pcOverlayBuffer) {

	pa_pInst->pcNextOverlayBuffer = pa_pcOverlayBuffer;
	pa_pInst->bUpdateOverlayBuffer = true;

	return ERR_NONE;
	
}



T_ERROR_CODE HITTX09setActivePlane(void *pa_tHndl, unsigned char pa_ucPlane){
    T_HITTX09_INST *pInst;
    
    if(pa_tHndl) {
        pInst = (T_HITTX09_INST*) pa_tHndl;
        
//        anTFTdmaDescriptorTable[pInst->cDeviceNr *  VDM_MAX_NOF_PLANES * PPI656O_DMA_DESC_SIZE + 2] = (unsigned short)((unsigned long)pInst->pcPlaneBuffer[pa_cPlane] & 0xffff);    // SAL: start address frame buffer 0
//        anTFTdmaDescriptorTable[pInst->cDeviceNr *  VDM_MAX_NOF_PLANES * PPI656O_DMA_DESC_SIZE + 3] = (unsigned short)((unsigned long)pInst->pcPlaneBuffer[pa_cPlane] >> 16);        // SAH: start address frame buffer 0
        if (pa_ucPlane == 0) {
            anTFTdmaDescriptorTable[3 * (pInst->nYsize + HITTX09_Y_BLANKING_LINES - 1)] = (unsigned long)&anTFTdmaDescriptorTable[0];
            anTFTdmaDescriptorTable1[3 * (pInst->nYsize + HITTX09_Y_BLANKING_LINES - 1)] = (unsigned long)&anTFTdmaDescriptorTable[0];
        } else {
            anTFTdmaDescriptorTable1[3 * (pInst->nYsize + HITTX09_Y_BLANKING_LINES - 1)] = (unsigned long)&anTFTdmaDescriptorTable1[0];
            anTFTdmaDescriptorTable[3 * (pInst->nYsize + HITTX09_Y_BLANKING_LINES - 1)] = (unsigned long)&anTFTdmaDescriptorTable1[0];
        }
           
        pInst->ucNextPlane = pa_ucPlane;
        pInst->bPlaneSwitchRequest = true;
        
        return ERR_NONE;
    } else {
        return ERR_T_HITTX09_INST_INVALID_HANDLE;
    }
    
    
}



unsigned char HITTX09getNextPlaneNum(void *pa_tHndl) {
    
    T_HITTX09_INST *pInst = (T_HITTX09_INST*) pa_tHndl;
    unsigned char cNewValue = pInst->cCurrActivePlane + 1;
    
    return((cNewValue >= pInst->ucNofPlanes) ? 0 : cNewValue);
}




T_ERROR_CODE HITTX09showFrame(void *pa_tHndl, 
                              unsigned char *pa_pcFrameBuffer, 
                              unsigned short pa_nXoffset, 
                              unsigned short pa_nYoffset, 
                              unsigned short pa_nXsize, 
                              unsigned short pa_nYsize) {
                                  
    T_ERROR_CODE erResult = ERR_NONE;
    
    if(pa_tHndl) {
        T_HITTX09_INST *pInst = (T_HITTX09_INST*) pa_tHndl;
        
        if (pInst->cCurrActivePlane == 0) {
            memcpy(pInst->pcPlaneBuffer[1], pa_pcFrameBuffer, pa_nXsize * pa_nYsize * pInst->cBytesPerPixel);
        } else {
            memcpy(pInst->pcPlaneBuffer[0], pa_pcFrameBuffer, pa_nXsize * pa_nYsize * pInst->cBytesPerPixel);
        }
        
    }
    return erResult;
}
