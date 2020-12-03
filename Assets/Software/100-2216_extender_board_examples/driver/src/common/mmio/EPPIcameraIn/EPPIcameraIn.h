#ifndef __PPI_CAMERA_IN_INCLUDED__
#define __PPI_CAMERA_IN_INCLUDED__


#include <Datatypes.h>
#include <services/services.h>

// frame grabber
#include "../../../../../blacksheep/common/framegrabber/frameGrabber.h"

#include "../../camera/camera.h"
#include "../../EPPI.h"

#define ERR_EPPI_CAMERA_IN_PLATFORM_INIT 				    (ERR_GENERIC - 10)	///< Platform specific initialization error
#define ERR_EPPI_CAMERA_IN_DATA_INTERFACE_MODE				(ERR_GENERIC - 20)	///< data interface mode was false defined
#define ERR_EPPI_CAMERA_IN_EPPI_INDEX						(ERR_GENERIC - 30)	///< Invalid PPI index
#define ERR_EPPI_CAMERA_IN_HOOK_INTERRUPT				    (ERR_GENERIC - 40)	///< Error hooking the interrupt
#define ERR_EPPI_CAMERA_IN_ALREADY_IN_USE			        (ERR_GENERIC - 50)	///< eppi is already in use
#define ERR_EPPI_CAMERA_IN_NOT_ENOUGH_MEMORY                (ERR_GENERIC - 60)  ///< Not enough memory available
#define ERR_EPPI_CAMERA_IN_BUFFER_MODE                      (ERR_GENERIC - 70)  ///< Unsupported Buffer Mode
#define ERR_EPPI_CAMERA_IN_ACTIVITY							(ERR_GENERIC - 80)	///< Sampling is already active/inactive
#define ERR_EPPI_CAMERA_IN_TOO_MANY_INSTANCES				(ERR_GENERIC - 90)	///< the max. nr. of instances are connected
#define ERR_EPPI_CAMERA_IN_PARAMETER_ERROR                  (ERR_GENERIC - 100) ///< a parameter does not fit

#define EPPI_CAMERA_IN_DESCR_SIZE							2
#define EPPI_CAMERA_IN_MAX_NOF_DEVICES						4



// driver behaviour
#define _EPPI_CAMERA_IN_DEBUG_LVL_             0           // select the debug level
//#define _EPPI_CAMERA_IN_RESET_ON_ERROR_


//hardware device functions
typedef void (*EPPI_CAMERA_IN_FN_CLOSE_DEVICE)(unsigned long pa_pHwDevHdl);
typedef unsigned long (*EPPI_CAMERA_IN_FN_GET_TIMESTAMP)(void);


typedef unsigned long T_EPPI_CAMERA_IN_HANDLE;


typedef enum {
    EPPI_CAMERA_IN_SINGLE_BUFFER,
    EPPI_CAMERA_IN_DOUBLE_BUFFER_NO_OVERWRITE,
    EPPI_CAMERA_IN_DOUBLE_BUFFER_OVERWRITE,
    EPPI_CAMERA_IN_MULTI_FRAME_BUFFER_NO_OVERWRITE,
    EPPI_CAMERA_IN_MULTI_FRAME_BUFFER_OVERWRITE
} T_EPPI_CAMERA_IN_BUFFER_MODE;


typedef struct {
	unsigned short				nIndexOut;
    unsigned short              nFramesLoaded;
    unsigned short              nMaxFrames;
	unsigned short				*pNextFrame;
} T_EPPI_CAMERA_IN_CALLBACK_ARG;


// prototype for callback function
typedef void (*T_EPPI_CAMERA_IN_CALLBACK)(T_EPPI_CAMERA_IN_CALLBACK_ARG pa_stBufferStatus);


typedef struct {
	bool						bIsActive;
	bool						bDequeueInProgress;
    T_EPPI_CAMERA_IN_BUFFER_MODE nBufferMode;
    T_EPPI_CAMERA_IN_CALLBACK    fnCallback;
    T_EPPI_CAMERA_IN_CALLBACK    fnErrorCallback;
    T_EPPI_CAMERA_IN_CALLBACK    fnDmaErrorCallback;
    T_CAM_DATA_INTERFACE_MODE   nDataInterfaceMode;
	T_CAM_COLOR_MODE			nColorMode;
	T_EPPI_SPEC					*pEPPIspec;
    void                        **pFrameGrabberInterface;
	unsigned char				cDeviceID;
    unsigned char               cEPPI;
	unsigned short				nDescrIndOffset;
    unsigned short              nXres;
    unsigned short              nYres;
	unsigned short				nIndexOut;
	unsigned short				nIndexIn;
    unsigned short              nMaxFrames;
	unsigned short				nNofFramesToHold;
    unsigned short              nFramesLoaded;
    unsigned short              **pFrameBuffer;							///< Frame buffer, size is allocated in open function
	ifrm_t 						**pFrameDescr;								///< Frame Description
	float						fFramerate;
	EPPI_CAMERA_IN_FN_CLOSE_DEVICE	fnHwDevCloseDev;				///< attach specified hardware function for closing the device to PPIcameraIn driver, if 0 no hardware attached
	unsigned long               hHwDevHndl;                         ///< handle of the specified hardware attached to PPIcameraIn, if 0 no hardware attached
	unsigned long               unPerIVG;
	unsigned long               unPerErrorIVG;
	unsigned long               unDmaErrorIVG;
	EPPI_CAMERA_IN_FN_GET_TIMESTAMP fnGetTimestamp;
} T_EPPI_CAMERA_IN_SPEC;


typedef struct {
    T_ERROR_CODE                erError;
    T_EPPI_CAMERA_IN_BUFFER_MODE nBufferMode;
    T_EPPI_CAMERA_IN_CALLBACK    fnCallback;
    T_EPPI_CAMERA_IN_CALLBACK    fnErrorCallback;
    T_EPPI_CAMERA_IN_CALLBACK    fnDmaErrorCallback;
    T_CAM_DATA_INTERFACE_MODE   nDataInterfaceMode;
	T_CAM_COLOR_MODE			nColorMode;
    void                        **pFrameGrabberInterface;
    unsigned char               cEPPI;
	unsigned char				cSDRAMsize;							//< size for the buffer in the SDRAM, [MB], optional, if not used set 0
    unsigned short              nMaxFrames;
	unsigned short				nNofFramesToHold;
    unsigned short              nXres;
    unsigned short              nYres;
    unsigned short               **pFrameBuffer;
    unsigned long               nPerIVG;
    unsigned long               nPerErrorIVG;
    unsigned long               nDmaErrorIVG;
	float						fFramerate;
	void        	            *pHwDevCloseDev;					///< attach specified hardware function for closing the device to EPPIcameraIn driver, if 0 no hardware attached
	unsigned long               hHwDevHndl;                         ///< handle of the specified hardware attached to EPPIcameraIn, if 0 no hardware attached
	EPPI_CAMERA_IN_FN_GET_TIMESTAMP fnGetTimestamp;
    void                        *pCallbackArg;
} T_EPPI_CAMERA_IN_CONF;


// prototypes
T_EPPI_CAMERA_IN_HANDLE EPPIcameraIn_open(T_EPPI_CAMERA_IN_CONF *pa_pEPPIcameraInConfig);
void EPPIcameraIn_close(T_EPPI_CAMERA_IN_HANDLE pa_hEPPIcameraInHndl);
void EPPIcameraIn_setup(void);
void EPPIcameraIn_cleanup(void);

// interface functions to the video grabber ------------------------------
T_ERROR_CODE EPPIcameraIn_start(T_EPPI_CAMERA_IN_HANDLE pa_EPPIcameraInHandle);
T_ERROR_CODE EPPIcameraIn_stop(T_EPPI_CAMERA_IN_HANDLE pa_EPPIcameraInHandle);
float EPPIcameraIn_getFramerate(T_EPPI_CAMERA_IN_HANDLE pa_tHndl);
T_ERROR_CODE EPPIcameraIn_resetDevice(T_EPPI_CAMERA_IN_HANDLE pa_tHndl);
int EPPIcameraIn_framesAvailable(T_EPPI_CAMERA_IN_HANDLE pa_tHndl);
ifrm_t *EPPIcameraIn_getNextFrame(T_EPPI_CAMERA_IN_HANDLE pa_tHndl);
T_ERROR_CODE EPPIcameraIn_freeFrame(T_EPPI_CAMERA_IN_HANDLE pa_tHndl);
void EPPIcameraIn_getDeviceInfo(T_EPPI_CAMERA_IN_HANDLE pa_tHndl, T_FG_VIDEO_DEVICE_INFO *pa_tInfoBlock);
T_ERROR_CODE EPPIcameraIn_configDevice(T_EPPI_CAMERA_IN_HANDLE pa_tHndl, T_FG_CMD pa_tCmd, T_FG_ARG *pa_tArg);
// -----------------------------------------------------------------------

#endif
