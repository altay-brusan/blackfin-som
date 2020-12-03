#ifndef _ADV7171_H_INCLUDED_
#define _ADV7171_H_INCLUDED_

#include <datatypes.h>
#include "ADV7171_Regs.h"
#include "../../i2c/I2CMan.h"
#include "../../gpioconfig.h"
#include "../../mmio/AnalogVideo/AnalogVideo.h"

#define ADV7171_DEVICE_NAME             "ADV7171"
#define ADV7171_DEFAULT_HWID            0x2A

#define ERR_ADV7171_INVALID_HANDLE                  (ERR_GENERIC - 10)
#define ERR_ADV7171_OUT_OF_MEM                      (ERR_GENERIC - 11)
#define ERR_ADV7171_HW_ERROR                        (ERR_GENERIC - 12)
#define ERR_ADV7171_VIDEO_FORMAT_NOT_SUPPORTED      (ERR_GENERIC - 13)

#define ADV7171_MAX_NOF_DEVICES                                        4


typedef struct {
     T_AV_STANDARD_DESCR     *tVideoStd;
     T_I2C_HANDLE            tI2CHndl;  
     unsigned char           cI2CHwAddr;
     T_GPIO_MASK             tResetPin;
     T_GPIO_MASK             tBlankPin;   
     T_AV_VIDEOFORMAT        tVideoFormat;  
} T_ADV7171_CONFIG;


/**
 *	@brief 			Structure, defining a HANDLE for the ADV7171 driver
 *	
 *	
 **/
typedef struct {
    T_AV_VIDEOFORMAT        tVideoFormat;
    T_AV_STANDARD_DESCR     *tVideoStd;
    T_I2C_HANDLE            tI2CHndl;
    unsigned char           cI2CHwAddr;
    T_GPIO_MASK             tResetPin;
    T_GPIO_MASK             tBlankPin;

} T_ADV7171_INST;

typedef signed long     T_ADV7171_HANDLE;
typedef unsigned short  T_ADV7171_CONTROL_CMD;

// prototype declarations for public functions ----------------
T_ADV7171_HANDLE ADV7171Open(T_ADV7171_CONFIG *pa_ptConfig, T_ERROR_CODE *pa_ptError, void  *pa_tGeneric);
T_ERROR_CODE ADV7171Close(T_ADV7171_HANDLE pa_tADV7171Hdl);
// ------------------------------------------------------------

#endif
