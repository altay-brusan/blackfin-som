#ifndef _HITTX09_CONFIG_H_INCLUDED_
#define _HITTX09_CONFIG_H_INCLUDED_

#include <datatypes.h>
#include <services/services.h>
#include "../../GPIOconfig.h"
#include "../../gpTimerConfig.h"
#include "../../../../../blacksheep/common/vdm/VDMconfig.h"

// error codes
#define ERR_HITTX09_ILLEGAL_PPI		    	(ERR_GENERIC - 10)
#define ERR_HITTX09_NOT_ENOUGH_MEM          (ERR_GENERIC - 21)
#define ERR_T_HITTX09_INST_INVALID_HANDLE   (ERR_GENERIC - 25)

// driver behaviour
#define _HITTX09_DEBUG_LVL_             0           // select the debug level
//#define _HITTX09_RESET_ON_ERROR_


typedef unsigned long T_HITTX09_HNDL;


#define HITTX09_XSIZE                       240
#define HITTX09_YSIZE                       320
#define HITTX09_DEFAULT_BYTE_PER_PIXEL      2
#define HITTX09_CLOCK_FREQ				    4000000//5330000//2208330//2660000	// [Hz]
#define HITTX09_DCLK_TIMER_CONFIG		    0x000d	// pwm mode
#define HITTX09_PWM_FREQ				    100000	// [hz]
#define HITTX09_PWM_TIMER_CONFIG		    0x000d	// pwm mode
#define HITTX09_HSYNC_TIMER_CONFIG		    0x00a9
#define HITTX09_DTMG_TIMER_CONFIG		    0x00a9
#ifdef __ADSPBF548__
#define HITTX09_Y_BLANKING_LINES            0       // no blanking lines when using EPPI (bf548)
#else
#define HITTX09_Y_BLANKING_LINES		    7		// number of vertikal blanking lines
#endif
#define HITTX09_DMA_CONFIG                  0x7505
// max value for the brightness
#define HITTX09_MAX_BRIGTHNESS			    255
#define HITTX09_DEVICE_NAME                 "Hitachi TX09Dx0VM1CCA"







typedef struct {
	unsigned char cPPI;
	bool bBitReverse;
	T_GP_TIMER_INST *tDclkTimerHndl;
	T_GP_TIMER_INST *tPwmTimerHndl;
	T_GP_TIMER_INST *tHsyncTimerHndl;
	T_GP_TIMER_INST *tDtmgTimerHndl;
//	T_GP_TIMER_INST *tDtmgPhaseTimerHndl;
	T_GP_TIMER_INST *tDtmgVsyncTimerHndl;
	unsigned short nXsize;
	unsigned short nYsize;
	unsigned char cBytesPerPixel;
	unsigned long *pnWorkingBuffer0;
	unsigned long *pnWorkingBuffer1;
	unsigned char cCurrentWorkingBuffer;
	void *pfGraficFunctions[VDM_MAX_NOF_GRAFIC_FUNCTIONS];
	bool bEnableInterrupt;
	unsigned char *pcNextOverlayBuffer;
	bool bUpdateOverlayBuffer;
	bool bOverlayCapability;
	unsigned char *pcPlaneBuffer[VDM_MAX_NOF_PLANES];
	unsigned char cCurrActivePlane;
	unsigned long nCclk;
	unsigned long nSclk;
	T_GPIO_MASK cPCIflag;
	bool bPlaneSwitchRequest;
	unsigned char ucNextPlane;
	unsigned char ucNofPlanes;
} T_HITTX09_INST;

typedef struct {
    unsigned char cPPI;
	unsigned long nCoreClk;
	unsigned long nSystemClk;
	//unsigned short nXsize;
	//unsigned short nYsize;
	unsigned char cBytesPerPixel;
	unsigned char cPWMtimer;
	unsigned char cDTMGshiftTimer;
	bool bCreateDCLK;
	unsigned char cDCLKtimer;
	T_GPIO_MASK cPCIflag;
	unsigned char cNofPlanes;
	unsigned short nDefColor;
	unsigned short nIVG;
	unsigned short nErrIVG;
	unsigned short nDmaErrIVG;
	unsigned short *nBckGrndImage;
	bool bOverlayCapability;
	bool bAutoUpdate;
} T_HITTX09_CONFIG;

// prototype declarations
void HITTX09setup(void);
void HITTX09cleanup(void);

T_HITTX09_HNDL HITTX09open (	T_HITTX09_CONFIG *pa_tConf,
                            	unsigned char ***pa_pcPlaneBuffer,
                            	void ***pa_pGraficFunctions,
                                T_ERROR_CODE *pa_tErr,
        						void *pa_pReserved);

T_ERROR_CODE HITTX09updateOverlayBuffer (T_HITTX09_INST *pa_pInst, unsigned char *pa_pcOverlayBuffer);
unsigned long HITTX09getCurrPlanebuffer (T_HITTX09_INST *pa_pInst);
void HITTX09setPixel (T_HITTX09_INST *pa_pInst, unsigned char pa_cPlane, unsigned short x, unsigned short y, T_VD_COLOR pa_nColor);
T_VD_COLOR HITTX09getPixel(T_HITTX09_INST *pa_pInst, unsigned char pa_cPlane, unsigned short x, unsigned short y);
void HITTX09blendPixel(T_HITTX09_INST *pa_pInst, unsigned char pa_cPlane, unsigned short x, unsigned short y, T_VD_COLOR pa_nColor, unsigned char pa_cAlpha);
void HITTX09clear(T_HITTX09_INST *pa_pInst, unsigned char pa_cPlane, T_VD_COLOR pa_nColor);
void HITTX09fillSurface(T_HITTX09_INST *pa_pInst, unsigned char pa_cPlane, unsigned short pa_nX, unsigned short pa_nY, T_VD_COLOR pa_nColor);
void HITTX09setupFrameBufferUpdateProcess (T_HITTX09_INST *pa_pInst, unsigned long *pa_nWorkingBuffer0, unsigned long *pa_nWorkingBuffer1, void *pa_Generic);
void HITTX09setWorkingBuffer (T_HITTX09_INST *pa_pInst, unsigned char pa_cWorkingBuffer);
void HITTX09setBrightness (T_HITTX09_INST *pa_pInst, unsigned short pa_cBrightness);
T_ERROR_CODE HITTX09showFrame(void *pa_tHndl, 
                              unsigned char *pa_pcFrameBuffer, 
                              unsigned short pa_nXoffset, 
                              unsigned short pa_nYoffset, 
                              unsigned short pa_nXsize, 
                              unsigned short pa_nYsize);
unsigned char HITTX09getNextPlaneNum(void *pa_tHndl);
T_ERROR_CODE HITTX09setActivePlane(void *pa_tHndl, unsigned char pa_ucPlane);
void HITTX09close(T_HITTX09_HNDL hDev);

#endif

