#ifndef __PPI_CAMERA_IN_INCLUDED__
#define __PPI_CAMERA_IN_INCLUDED__

#include <Datatypes.h>
#include <services/services.h>
// frame grabber
#include "../../../../../blacksheep/common/framegrabber/frameGrabber.h"
#include "../../camera/camera.h"
#include "../../PPI.h"

#define ERR_PPI_CAMERA_IN_PLATFORM_INIT 				    (ERR_GENERIC - 10)	///< Platform specific initialization error
#define ERR_PPI_CAMERA_IN_DATA_INTERFACE_MODE				(ERR_GENERIC - 20)	///< data interface mode was false defined
#define ERR_PPI_CAMERA_IN_PPI_INDEX						    (ERR_GENERIC - 30)	///< Invalid PPI index
#define ERR_PPI_CAMERA_IN_HOOK_INTERRUPT				    (ERR_GENERIC - 40)	///< Error hooking the interrupt
#define ERR_PPI_CAMERA_IN_ALREADY_IN_USE			        (ERR_GENERIC - 50)	///< ppi is already in use
#define ERR_PPI_CAMERA_IN_NOT_ENOUGH_MEMORY                 (ERR_GENERIC - 60)  ///< Not enough memory available
#define ERR_PPI_CAMERA_IN_BUFFER_MODE                       (ERR_GENERIC - 70)  ///< Unsupported Buffer Mode
#define ERR_PPI_CAMERA_IN_ACTIVITY							(ERR_GENERIC - 80)	///< Sampling is already active/inactive
#define ERR_PPI_CAMERA_IN_TOO_MANY_INSTANCES				(ERR_GENERIC - 90)	///< the max. nr. of instances are connected
#define ERR_PPI_CAMERA_IN_INVALID_HANDLE                    (ERR_GENERIC - 95)  ///< No valid handle passed to the function

#define PPI_CAMERA_IN_DESCR_SIZE							2
#define PPI_CAMERA_IN_MAX_NOF_DEVICES						4


//hardware device functions
typedef void (*PPI_CAMERA_IN_FN_CLOSE_DEVICE)(unsigned long pa_pHwDevHdl);
typedef unsigned long (*PPI_CAMERA_IN_FN_GET_TIMESTAMP)(void);


typedef long T_PPI_CAMERA_IN_HANDLE;


typedef enum {
    PPI_CAMERA_IN_SINGLE_BUFFER,
    PPI_CAMERA_IN_DOUBLE_BUFFER_NO_OVERWRITE,
    PPI_CAMERA_IN_DOUBLE_BUFFER_OVERWRITE,
    PPI_CAMERA_IN_MULTI_FRAME_BUFFER_NO_OVERWRITE,
    PPI_CAMERA_IN_MULTI_FRAME_BUFFER_OVERWRITE
} T_PPI_CAMERA_IN_BUFFER_MODE;

/*
typedef struct {
	unsigned short				nIndexOut;
    unsigned short              nFramesLoaded;
    unsigned short              nMaxFrames;
	unsigned short				*pNextFrame;
} T_PPI_CAMERA_IN_CALLBACK_ARG;
*/

// prototype for callback function
typedef void (*T_PPI_CAMERA_IN_CALLBACK)(void *);


typedef struct {
	bool						bIsActive;
	bool						bDequeueInProgress;
    T_PPI_CAMERA_IN_BUFFER_MODE nBufferMode;
    T_PPI_CAMERA_IN_CALLBACK    fnCallback;
    void                        *pCallbackArg;
    T_PPI_CAMERA_IN_CALLBACK    fnErrorCallback;
    T_PPI_CAMERA_IN_CALLBACK    fnDmaErrorCallback;
    T_CAM_DATA_INTERFACE_MODE   nDataInterfaceMode;
	T_CAM_COLOR_MODE			nColorMode;
	T_PPI_SPEC					*pPPIspec;
    void                        **pFrameGrabberInterface;
	unsigned char				cDeviceID;
    unsigned char               cPPI;
	unsigned short				nDescrIndOffset;
    unsigned short              nXres;
    unsigned short              nYres;
	unsigned short				nIndexOut;
	unsigned short				nIndexIn;
    unsigned short              nMaxFrames;
//	unsigned short				nNofFramesToHold;
    unsigned short              nFramesLoaded;
    unsigned short              nFrameBufferIndex;
    unsigned short              **pFrameBuffer;							///< Frame buffer, size is allocated in open function
	ifrm_t 						**pFrameDescr;								///< Frame Description
	float						fFramerate;
	PPI_CAMERA_IN_FN_CLOSE_DEVICE	fnHwDevCloseDev;				///< attach specified hardware function for closing the device to PPIcameraIn driver, if 0 no hardware attached
	unsigned long               hHwDevHndl;                         ///< handle of the specified hardware attached to PPIcameraIn, if 0 no hardware attached
	unsigned long               nIVG;
	unsigned long               nErrIVG;
	unsigned long               nDmaErrIVG;
    PPI_CAMERA_IN_FN_GET_TIMESTAMP fnGetTimestamp;
} T_PPI_CAMERA_IN_SPEC;


typedef struct {
    T_ERROR_CODE                erError;
    T_PPI_CAMERA_IN_BUFFER_MODE nBufferMode;
    T_PPI_CAMERA_IN_CALLBACK    fnCallback;
    T_PPI_CAMERA_IN_CALLBACK    fnErrorCallback;
    T_PPI_CAMERA_IN_CALLBACK    fnDmaErrorCallback;
    T_CAM_DATA_INTERFACE_MODE   nDataInterfaceMode;
	T_CAM_COLOR_MODE			nColorMode;
    void                        **pFrameGrabberInterface;
    unsigned char               cPPI;
	unsigned char				cSDRAMsize;							///< size for the buffer in the SDRAM, [MB]
    unsigned short              nMaxFrames;
//	unsigned short				nNofFramesToHold;
    unsigned short              nXres;
    unsigned short              nYres;
    unsigned short               **pFrameBuffer;
    unsigned long               nPerIVG;
    unsigned long               nPerErrorIVG;
    unsigned long               nDmaErrorIVG;
	float						fFramerate;
	void        	            *pHwDevCloseDev;					///< attach specified hardware function for closing the device to PPIcameraIn driver, if 0 no hardware attached
	unsigned long               hHwDevHndl;                         ///< handle of the specified hardware attached to PPIcameraIn, if 0 no hardware attached
    PPI_CAMERA_IN_FN_GET_TIMESTAMP fnGetTimestamp;
    void                        *pCallbackArg;
} T_PPI_CAMERA_IN_CONF;


// prototypes
T_PPI_CAMERA_IN_HANDLE PPIcameraIn_open(T_PPI_CAMERA_IN_CONF *pa_pPPIcameraInConfig);
void PPIcameraIn_close(T_PPI_CAMERA_IN_HANDLE pa_hPPIcameraInHndl);
void PPIcameraIn_setup(void);
void PPIcameraIn_cleanup(void);
T_ERROR_CODE PPIcameraIn_start(T_PPI_CAMERA_IN_HANDLE pa_PPIcameraInHandle);
T_ERROR_CODE PPIcameraIn_stop(T_PPI_CAMERA_IN_HANDLE pa_PPIcameraInHandle);

// interface functions to the video grabber ------------------------------
T_ERROR_CODE PPIcameraIn_startGrabbing(T_PPI_CAMERA_IN_HANDLE pa_tHndl);
T_ERROR_CODE PPIcameraIn_stopGrabbing(T_PPI_CAMERA_IN_HANDLE pa_PPIcameraInHandle);
float PPIcameraIn_getFramerate(T_PPI_CAMERA_IN_HANDLE pa_tHndl);
T_ERROR_CODE PPIcameraIn_resetDevice(T_PPI_CAMERA_IN_HANDLE pa_tHndl);
int PPIcameraIn_framesAvailable(T_PPI_CAMERA_IN_HANDLE pa_tHndl);
ifrm_t *PPIcameraIn_getNextFrame(T_PPI_CAMERA_IN_HANDLE pa_tHndl);
T_ERROR_CODE PPIcameraIn_freeFrame(T_PPI_CAMERA_IN_HANDLE pa_tHndl);
void PPIcameraIn_getDeviceInfo(T_PPI_CAMERA_IN_HANDLE pa_tHndl, T_FG_VIDEO_DEVICE_INFO *pa_tInfoBlock);
T_ERROR_CODE PPIcameraIn_configDevice(T_PPI_CAMERA_IN_HANDLE pa_tHndl, T_FG_CMD pa_tCmd, T_FG_ARG *pa_tArg);
// -----------------------------------------------------------------------

#endif
