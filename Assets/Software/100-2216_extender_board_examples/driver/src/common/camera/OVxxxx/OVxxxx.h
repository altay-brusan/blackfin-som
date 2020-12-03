#ifndef _CAM_DEFAULT_H_INCLUDED_
#define _CAM_DEFAULT_H_INCLUDED_

#include <environment.h>
#include "../camera.h"
#include "../../GPIOconfig.h"

// SELECT DRIVER BEHAVIOUR ////////////////////
#define OVXXXX_DEV_ID_IGNORE_LAST_NIBBLE
///////////////////////////////////////////////

/// defines for some well known sccb registers
#define OVXXXX_REG_RESET            0x12
#define OVXXXX_REG_CAM_ID_HI        0x0a
#define OVXXXX_REG_CAM_ID_LO        0x0b

/// defines for some well known sccb_16bit registers
#define OVXXXX_REG_RESET_16BIT      0x3012
#define OVXXXX_REG_CAM_ID_HI_16BIT  0x300a
#define OVXXXX_REG_CAM_ID_LO_16BIT  0x300b

/// defines for some well known I2C registers
#define OVXXXX_REG_RESET_I2C_HI     0x30
#define OVXXXX_REG_RESET_I2C_LO     0x12

#define OVXXXX_REG_CAM_ID_HI_I2C_HI 0x30
#define OVXXXX_REG_CAM_ID_HI_I2C_LO 0x0a

#define OVXXXX_REG_CAM_ID_LO_I2C_HI 0x30
#define OVXXXX_REG_CAM_ID_LO_I2C_LO 0x0b

// I2C size defines
#define OVXXXX_I2C_REGSIZE          2               ///<register size in Bytes

/// bit mask defines
#define OVXXXX_MASK_RESET_BIT		(1 << 7)

#define OVXXXX_STABILIZE_TIME       2000            ///< Stabilize time of the camera in [ms]

// error macros
#define ERR_NO_CAMERA_FOUND					(ERR_GENERIC - 10)	///<camera not found
#define ERR_CONFIG_CAM						(ERR_GENERIC - 20)	///<sccb write access error
#define ERR_UNKNOWN_CAMERA					(ERR_GENERIC - 30)	///<camera not recognized or supported
#define ERR_OVXXXX_INVALID_PARAMS			(ERR_GENERIC - 40)	///<illegal combination of parameter (resolution, framerate, color mode, camera model)
#define ERR_OVXXXX_INVALID_INT_HNDL			(ERR_GENERIC - 50)	///<interface handle is not valid
#define ERR_OVXXXX_UNKNOWN_INTERFACE		(ERR_GENERIC - 60)	///<unknown camera control interface type
#define ERR_OVXXXX_COLOR_MODE_NOT_SUPPORTED	(ERR_GENERIC - 70)	///<the color mode is not supported
#define ERR_OVXXXX_RESOLUTION_NOT_SUPPORTED	(ERR_GENERIC - 80)	///<the resolution is not supported
#define ERR_OVXXXX_WRONG_CAMERA_CONNECTED	(ERR_GENERIC - 90)	///<the camera connected to the camera interface is not the same as selected
#define ERR_OVXXXX_NOT_ENOUGH_MEMORY		(ERR_GENERIC - 100)	///< not enough memory for allocating


typedef unsigned long T_OVXXXX_HANDLE;


typedef enum {
	OVT_SCCB,
	OVT_SCCB_16BIT,
	OVT_I2C_16BIT,
} T_OVXXXX_CAMERA_INTERFACE;

typedef enum {
    OV_UNKNOWN,
	OV263X,
	OV264X,
	OV265X,
} T_OVXXXX_CAMERA_MODEL;

typedef struct {
    unsigned char cDevReg;
    unsigned char cDevRegValue[OVXXXX_I2C_REGSIZE];
}T_OVXXXX_I2C_CONFIG_PARAMS;


typedef struct {
    T_CAM_MANUFACTURER          nManufacturer;
    T_OVXXXX_CAMERA_INTERFACE   nInterfaceType;
	T_OVXXXX_CAMERA_MODEL		nCameraModel;
    unsigned long               nDeviceID;
    unsigned char               cDeviceAddress;
} T_OVXXXX_CAMERA_TYPE;


typedef struct {
    T_OVXXXX_CAMERA_INTERFACE   nCameraInterface;
    T_OVXXXX_CAMERA_MODEL       nCameraModel;
    T_GPIO_MASK                 sio_cMask;                          // SCCB Masks
    T_GPIO_MASK                 sio_dMask;
    T_GPIO_MASK                 pwdnMask;
    T_GPIO_MASK                 resetMask;
    unsigned char               cDeviceAddr;
	unsigned short				nCameraID;							// is set by the open function
    unsigned long               hCamInterfaceHandle;
} T_OVXXXX_SPEC;


typedef struct {
    T_CAM_COLOR_MODE            nColorMode;
    T_CAM_DATA_INTERFACE_MODE   nDataInterfaceMode;					// is set by the open funcion
    T_OVXXXX_CAMERA_INTERFACE   nCameraInterface;
    T_CAM_RESOLUTION            nResolution;
    T_OVXXXX_CAMERA_MODEL       nCameraModel;
    T_ERROR_CODE                erError;
    unsigned short              nXres;								// is set by the open function according the resolution
    unsigned short              nYres;
	unsigned short				nCameraID;							// is set by the open function
    unsigned long               hCamInterfaceHandle;				// handle of the configuration interface (zB. SCCB)
    float                       fFramerate;							// set framerate, the open function corrects it to a supported value
    void                        *pHwDevCloseDev;                    // function for closing the camera
} T_OVXXXX_CONF;

//prototypes
void OVxxxx_setup(void);
void OVxxxx_cleanup(void);
T_OVXXXX_HANDLE OVxxxx_Open(T_OVXXXX_CONF *pa_pCameraConfig);
T_ERROR_CODE OVxxxx_getCamera(T_CAM_MANUFACTURER pa_nManufacturer, T_OVXXXX_CAMERA_MODEL pa_nCamera, T_OVXXXX_CAMERA_TYPE *pa_nCamType);
void OVxxxx_Close(T_OVXXXX_HANDLE stCameraSpec);
void OVxxxx_PWRup(T_OVXXXX_HANDLE pa_Handle);
void OVxxxx_PWRdown(T_OVXXXX_HANDLE pa_Handle);
void OVxxxx_closeDevice(T_OVXXXX_HANDLE pa_Handle);


#endif

