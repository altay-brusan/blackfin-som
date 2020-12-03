/**
    @file ITU656Out.c
    @ingroup video
    @brief 
    
    
    
    BLT_DISCLAIMER(TBD)
    @author Thomas Maier
    @version 1.0
    @date 30.06.2009
    
    @startcond Changelog
    
    @endcond
**/

#ifndef _PPI656O_H_INCLUDED_
#define _PPI656O_H_INCLUDED_

#include <Datatypes.h>
#ifdef _USE_VDK_
    #include <vdk.h>
#endif

#include "../../../../../blacksheep/common/vdm/VDMconfig.h"
#include "../analogVideo/AnalogVideo.h"

#define ERR_PPI656O_OUT_OF_MEM               (ERR_GENERIC - 10) 
#define ERR_PPI656O_UNKNWON_AV_STD           (ERR_GENERIC - 11)
#define ERR_PPI656O_INVALID_HANDLE           (ERR_GENERIC - 12)
#define ERR_PPI656O_PPI_NOT_AVAILABLE        (ERR_GENERIC - 13)
#define ERR_PPI6565O_PLATFORM_INIT           (ERR_GENERIC - 14)
#define ERR_PPI656O_HOOK_INTERRUPT           (ERR_GENERIC - 15)
#define ERR_PPI656O_FSYNC_TIMEOUT            (ERR_GENERIC - 16)
#define ERR_PPI656O_NO_FREE_PLANE            (ERR_GENERIC - 17)

#define PPI656O_MAX_DEVICES                                   2
#define PPI656O_DESC_PER_DEV                                  2 ///< dma descriptors per device
#define PPI656O_DMA_DESC_SIZE                                 5
#define PPI656O_IVG                 						0x0b ///< standard ivg for ppi


// Select driver behaviour ****************************************
#define _PPI656O_DEBUG_LVL_                         1
// ****************************************************************

#define PPI656O_FSYNC_TIMEOUT                           1000        // ppi frame sync timeout [ms]

typedef signed long T_PPI656O_HANDLE;
typedef void (*T_PPI656O_CALLBACK)(void *pa_pClientArg);    ///< Callback function for PPI interrupt
typedef T_ERROR_CODE (*T_PPI656O_FN_CLOSE)(void *pParam);

typedef enum {
    PPI656_DOUBLE_BUFFER,
    PPI656_DOUBLE_BUFFER_AUTO,
    PPI656_MULTI_BUFFER,
    PPI656_MULTI_BUFFER_AUTO,
    PPI656_SINGLE_BUFFER
} T_PPI656O_BUFFER_MODE;

/**
    @brief Config struct passed to open function
**/
typedef struct {
    unsigned char   cPPInr;             ///< Number of PPI to use
    unsigned long   nPPIIVG;            ///< PPI IVG 0 = standard
    unsigned long   nErrorIVG;          ///< PPI Error IVG 0 = standard
    unsigned long   nDMAErrorIVG;       ///< DMA Error IVG 0 = standard
    void *pHwDevHndl;                   ///< handle of underlying device
    bool bOverwriteActivePlaneAllowed;  ///< true->the active plane can be overwrite by ShowFrame. If cNumberOfPlanes is 1, this flag is always set! false->the active plane cannot be averwrite 
    bool bAutoUpdate;
    unsigned char cNumberOfPlanes;      ///< Number of planes
    T_AV_VIDEOFORMAT tAVformat;
    unsigned char  cMDMAChannel;    
    T_PPI656O_FN_CLOSE fnDeviceClose;   ///< function pointer to close device
} T_PPI656O_CONFIG;


/**
    @brief instance of the driver 
**/
typedef struct {
    unsigned char cDeviceNr;     
    unsigned char cPPInr;      
    unsigned long nPPIIVG;                              ///< PPI IVG 0 = standard
    unsigned long nErrorIVG;                            ///< PPI Error IVG 0 = standard
    unsigned long nDMAErrorIVG;                         ///< DMA Error IVG 0 = standard     
    unsigned char *pcFrameBuffer;                       ///< pointer to current framebuffer
    bool          bOverwriteActivePlaneAllowed;         ///< true->the active plane can be overwrite by PPI656OshowFrame. If cNumberOfPlanes is 1, this flag is always set! false->the active plane cannot be overwrite.
    unsigned char cNumberOfPlanes;                      ///< Number of planes
    signed short  nFreePlanesCount;                     ///< Number of planes that can be written. -1 means that PPI656OshowFrame was never executed
	unsigned char *pcPlaneBuffer[VDM_MAX_NOF_PLANES];   ///< pointer to memory of all planebuffers
    void *pHwDevHndl;                                   ///< handle to underlying videodevice if present
    T_AV_VIDEOFORMAT tAVformat;
    T_AV_STANDARD_DESCR tAVstandard;                    ///< parameters of analog video standard
    unsigned short nCurrXsize;
	unsigned short nCurrYsize;
	unsigned char  cBytesPerPixel;  
	unsigned char  cActivePlane;                        ///< Points to the active plane at the output
	unsigned char  cNextPlane;                          ///< Points to the next plane at the output	
	bool           bPlaneSwitchRequest;
	bool           bAutoUpdate;
	bool           bVSYNC;
	unsigned char  cMDMAChannel;                        ///< channel to use for mdma
	void *pfGraficFunctions[VDM_MAX_NOF_GRAFIC_FUNCTIONS];
#ifdef _USE_VDK_
	VDK_SemaphoreID	tFsyncSem;		                    ///< Access semaphore for fsync
#endif	
   	T_PPI656O_CALLBACK fnErrorCallback;
    T_PPI656O_FN_CLOSE fnDeviceClose;                   ///< function pointer to close device
} T_PPI656O_INST;



void PPI656OsetPixel (void *pa_tHndl, unsigned char pa_cPlane, unsigned short x, unsigned short y, T_VD_COLOR pa_nColor);
T_VD_COLOR PPI656OgetPixel(void *pa_tHndl, unsigned char pa_cPlane, unsigned short x, unsigned short y);
void PPI656OblendPixel(void *pa_tHndl, unsigned char pa_cPlane, unsigned short x, unsigned short y, T_VD_COLOR pa_nColor, unsigned char pa_cAlpha);
signed char  PPI656OgetActivePlane(void *pa_tHndl);
T_ERROR_CODE PPI656OsetActivePlane(void *pa_tHndl, unsigned char pa_cPlane);
T_ERROR_CODE PPI656OwaitForFrameSync(void *pa_tHndl);
T_ERROR_CODE PPI656OshowFrame(void *pa_tHndl, 
                              unsigned char *pa_pcFrameBuffer, 
                              unsigned short pa_nXoffset, 
                              unsigned short pa_nYoffset, 
                              unsigned short pa_nXsize, 
                              unsigned short pa_nYsize);
T_PPI656O_HANDLE PPI656Oopen(T_PPI656O_CONFIG *pa_ptConfig, 
                             T_ERROR_CODE *pa_tError, 
                             void ***pfGraficFunctions);
unsigned char PPI656OgetNextPlaneNum(void *pa_tHndl);                             
T_ERROR_CODE T_PPI656Oclose(T_PPI656O_HANDLE pa_tHndl);

#endif

