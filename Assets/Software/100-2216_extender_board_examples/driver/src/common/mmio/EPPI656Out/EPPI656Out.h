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

#ifndef _EPPI656O_H_INCLUDED_
#define _EPPI656O_H_INCLUDED_

#include <Datatypes.h>
#ifdef _USE_VDK_
    #include <vdk.h>
#endif

#include "../../../../../blacksheep/common/vdm/VDMconfig.h"
#include "../analogVideo/AnalogVideo.h"

#define ERR_EPPI656O_OUT_OF_MEM               (ERR_GENERIC - 10) 
#define ERR_EPPI656O_UNKNWON_AV_STD           (ERR_GENERIC - 11)
#define ERR_EPPI656O_INVALID_HANDLE           (ERR_GENERIC - 12)
#define ERR_EPPI656O_EPPI_NOT_AVAILABLE       (ERR_GENERIC - 13)
#define ERR_EPPI6565O_PLATFORM_INIT           (ERR_GENERIC - 14)
#define ERR_EPPI656O_HOOK_INTERRUPT           (ERR_GENERIC - 15)
#define ERR_EPPI656O_FSYNC_TIMEOUT            (ERR_GENERIC - 16)
#define ERR_EPPI656O_NO_FREE_PLANE            (ERR_GENERIC - 17)

#define EPPI656O_MAX_DEVICES                                   2
#define EPPI656O_DESC_PER_DEV                                  2 ///< dma descriptors per device
#define EPPI656O_DMA_DESC_SIZE                                 4
//#define EPPI656O_IVG                 						0x0b ///< standard ivg for EPPI

#define EPPI656O_FSYNC_TIMEOUT                               1000; ///< time in ms we wait for frame sync

// Select driver behaviour **************************************
#define _EPPI656O_DEBUG_LVL_                                0
// **************************************************************

typedef signed long T_EPPI656O_HANDLE;
typedef void (*T_EPPI656O_CALLBACK)(void *pa_pClientArg);    ///< Callback function for EPPI interrupt
typedef T_ERROR_CODE (*T_EPPI656O_FN_CLOSE)(void *pParam);

typedef enum {
    EPPI656_DOUBLE_BUFFER,
    EPPI656_DOUBLE_BUFFER_AUTO,
    EPPI656_MULTI_BUFFER,
    EPPI656_MULTI_BUFFER_AUTO,
    EPPI656_SINGLE_BUFFER
} T_EPPI656O_BUFFER_MODE;

/**
    @brief Config struct passed to open function
**/
typedef struct {
    unsigned char   cEPPInr;             ///< Number of EPPI to use
    unsigned long   nEPPIIVG;            ///< EPPI IVG 0 = standard
    unsigned long   nErrorIVG;          ///< EPPI Error IVG 0 = standard
    unsigned long   nDMAErrorIVG;       ///< DMA Error IVG 0 = standard
    void *pHwDevHndl;                   ///< handle of underlying device
    bool bOverwriteActivePlaneAllowed;  ///< true->the active plane can be overwrite by ShowFrame. If cNumberOfPlanes is 1, this flag is always set! false->the active plane cannot be averwrite 
    bool bAutoUpdate;
    unsigned char cNumberOfPlanes;      ///< Number of planes
    T_AV_VIDEOFORMAT tAVformat;
    unsigned char  cMDMAChannel;    
    T_EPPI656O_FN_CLOSE fnDeviceClose;   ///< function pointer to close device
	bool bUseSharedBuffer;               ///< true: use external frame buffer. false: use intenal frame buffer
} T_EPPI656O_CONFIG;


/**
    @brief instance of the driver 
**/
typedef struct {
    unsigned char cDeviceNr;     
    unsigned char cEPPInr;      
    unsigned long nEPPIIVG;                              ///< EPPI IVG 0 = standard
    unsigned long nErrorIVG;                            ///< EPPI Error IVG 0 = standard
    unsigned long nDMAErrorIVG;                         ///< DMA Error IVG 0 = standard     
    unsigned char *pcFrameBuffer;                       ///< pointer to current framebuffer
    bool          bOverwriteActivePlaneAllowed;         ///< true->the active plane can be overwrite by EPPI656OshowFrame. If cNumberOfPlanes is 1, this flag is always set! false->the active plane cannot be overwrite.
    unsigned char cNumberOfPlanes;                      ///< Number of planes
    signed short  nFreePlanesCount;                     ///< Number of planes that can be written. -1 means that EPPI656OshowFrame was never executed
	unsigned char *pcPlaneBuffer[VDM_MAX_NOF_PLANES];   ///< pointer to memory of all planebuffers
    void *pHwDevHndl;                                   ///< handle to underlying videodevice if present
    T_AV_VIDEOFORMAT tAVformat;
    T_AV_STANDARD_DESCR tAVstandard;                    ///< parameters of analog video standard
    unsigned short nCurrXsize;
	unsigned short nCurrYsize;
	unsigned char  cBytesPerPixel;  
	unsigned char  cActivePlane;                        ///< Points to the active plane at the output
	unsigned char  cNextPlane;                          ///< Points to the next plane at the output	
	bool           bPlaneSwitchRequest;                 ///< Signals a plane switch request
	bool           bPlaneSwitchField1Switched;          ///< Indicates if the start address of the first field was switched
	bool           bAutoUpdate;
	bool           bVSYNC;
	unsigned char  cMDMAChannel;                        ///< channel to use for mdma
	void           *pfGraficFunctions[VDM_MAX_NOF_GRAFIC_FUNCTIONS];
#ifdef _USE_VDK_
	VDK_SemaphoreID tFsyncSem;		                    ///< Access semaphore for fsync
#endif	
   	T_EPPI656O_CALLBACK fnErrorCallback;
    T_EPPI656O_FN_CLOSE fnDeviceClose;                   ///< function pointer to close device 	
   	unsigned long  nTransmittedLines;         ///< Number of transmitted lines for the active frame
   	bool           bUseSharedBuffer;
} T_EPPI656O_INST;



void EPPI656OsetPixel (void *pa_tHndl, unsigned char pa_cPlane, unsigned short x, unsigned short y, T_VD_COLOR pa_nColor);
T_VD_COLOR EPPI656OgetPixel(void *pa_tHndl, unsigned char pa_cPlane, unsigned short x, unsigned short y);
void EPPI656OblendPixel(void *pa_tHndl, unsigned char pa_cPlane, unsigned short x, unsigned short y, T_VD_COLOR pa_nColor, unsigned char pa_cAlpha);
signed char  EPPI656OgetActivePlane(void *pa_tHndl);
T_ERROR_CODE EPPI656OsetActivePlane(void *pa_tHndl, unsigned char pa_cPlane);
T_ERROR_CODE EPPI656OwaitForFrameSync(void *pa_tHndl);
T_ERROR_CODE EPPI656OshowFrame(void *pa_tHndl, 
                              unsigned char *pa_pcFrameBuffer, 
                              unsigned short pa_nXoffset, 
                              unsigned short pa_nYoffset, 
                              unsigned short pa_nXsize, 
                              unsigned short pa_nYsize);
unsigned char EPPI656OgetNextPlaneNum(void *pa_tHndl);

T_EPPI656O_HANDLE EPPI656Oopen(T_EPPI656O_CONFIG *pa_ptConfig, 
                             T_ERROR_CODE *pa_tError, 
                             void ***pfGraficFunctions);                          
T_ERROR_CODE T_EPPI656Oclose(T_EPPI656O_HANDLE pa_tHndl);

T_ERROR_CODE EPPI656Ostart(T_EPPI656O_HANDLE pa_tHndl);
T_ERROR_CODE EPPI656Ostop(T_EPPI656O_HANDLE pa_tHndl);

#endif

