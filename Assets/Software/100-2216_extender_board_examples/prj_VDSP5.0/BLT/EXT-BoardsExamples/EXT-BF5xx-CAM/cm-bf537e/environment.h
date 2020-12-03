#ifndef _ENVIRONMENT_H_INCLUDED_
#define _ENVIRONMENT_H_INCLUDED_

#include <cdefbf537.h>
#include "gpio_global.h"

/**********************************************************************************************
 **                             Core Module Version                                          **
 **********************************************************************************************/
#define _USE_CM_BF537E_V1_X_Y_    ///< Uncomment to use version 1.X.Y of the core module
//#define _USE_CM_BF537E_V2_X_Y_    ///< Uncomment to use version 2.X.Y of the core module
//#define _USE_CM_BF537E_V3_X_Y_    ///< Uncomment to use version 3.X.Y of the core module
//#define _USE_CM_BF537E_I          ///< Uncomment to use the industrial variant of the core module
/*********************************************************************************************/



/**********************************************************************************************
 **                             Test Mode                                                    **
 **********************************************************************************************/
#define _USE_TEST_CAMERA_
//#define _USE_TEST_DISPLAY_
/*********************************************************************************************/



// DO NOT MODIFY
#ifdef _USE_TEST_CAMERA_
    #define USE_CAM1
#endif    

#ifdef _USE_TEST_DISPLAY_
    #define _USE_HITTX09D_      
#endif 


/**********************************************************************************************
 **                             Hitachi HTXX09D                                              **
 **********************************************************************************************/
#define HITTX09_DTMG_SHIFT_TIMER            1               ///< select the DTMG shift timer
#define HITTX09_PWM_TIMER                   4               ///< select the timer used for changing the brightness of the background light
#define HITTX09_PCI_PIN                     _PF14            ///< select the PCI pin
#define HITTX09_DCLK_TIMER                  3               ///< select the DCLK timer 
#define HITTX09_PWM_PIN                     _PF5            ///< PWM pin
#define HITTX09D_DEFAULT_BACKGROUND_COLOR   0            ///< default background color if no background image is selected
#define HITTX09D_USE_BACKGROUND_IMAGE       1            ///< set to 1 if you want to use the default background image stored in g_HITTX09DBackgroundImage
#define HITTX09_USE_LANDSCAPE_FORMAT        1            ///< set 1 if you want to use landscape format
#define HITTX09_NOF_PLANES                  2            ///< select number of planes
#define HITTX09_PERIPHERAL_IVG              7            ///< PPI IVG, set to 0 to use the default value
#define HITTX09_PPI_IVG                     9            ///< PPI IVG, set to 0 to use the default value
#define HITTX09_PERIPHERAL_ERROR_IVG        11           ///< PPI Error IVG, set to 0 to use the default value
#define HITTX09_DMA_ERROR_IVG               12           ///< DMA Error IVG, set to 0 to use the default value
/**********************************************************************************************/



/**********************************************************************************************
 **                             Processor Definitions                                        **
 **********************************************************************************************/
#define PROC_CLOCK_FREQ_IN                  25      ///< Input clock source frequency in MHz
#ifdef _USE_CM_BF537E_I
    #define PROC_CLOCK_FREQ_CORE            500     ///< Core frequency in MHz
    #define PROC_CLOCK_FREQ_SYSTEM          125     ///< System frequency in MHz
#else
    #define PROC_CLOCK_FREQ_CORE            600     ///< Core frequency in MHz
    #define PROC_CLOCK_FREQ_SYSTEM          133     ///< System frequency in MHz
#endif    
#define PROC_CLOCK_USE_PLL_INPUT_DIV            ///< Uncomment to use the PLL input divider
#define PROC_VARIANT    ADI_PWR_PROC_BF537SKBC600   ///< Processor variant (see adi_pwr.h)
#define PROC_PACKAGE    ADI_PWR_PACKAGE_MBGA        ///< Processor package (see adi_pwr.h)
#define PROC_VDDEXT     ADI_PWR_VDDEXT_330          ///< External Voltage supply (see adi_pwr.h)
//#define PROC_VDDINT     ADI_PWR_VLEV_140            ///< Core voltage (see adi_pwr.h)
/*********************************************************************************************/



// UART settings
#define PERIPHERAL_ERROR_IVG        0


// components to use
#define USE_CAM1
//#define USE_CAM2                      // not available for BF537

#define BS_IVG_PERIPHERAL_ERROR 0

// ********************************************************************************************
// PLEASE TAKE IN CONSIDER THAT IN RAW MODE THE EFFECTIVE RESOLUTION IS DIVIDED BY 2 IN X AND Y.
// THIS IS CAUSED BY THE DEMOSAICING PROCESS DURING CONVERTION TO YUV422.
// SO THE EFFECTIVE RESOLUTION BY UXGA SELECTING THE RAW FORMAT IS 800 X 600.
// ********************************************************************************************

/* ********** PLEASE CONFIGURE ************************ */

// SELECT THE CAMERA CONNECTED -------------------------
// OV7660
// OV2630
// OV2640

#define DEFAULT_CAMERA_CAM1             OV264X
//#define DEFAULT_CAMERA_CAM2             OV264X
#define DEFAULT_MANUFACTURER_CAM1       OMNIVISION
//#define DEFAULT_MANUFACTURER_CAM2       OMNIVISION

// -----------------------------------------------------

// SELECT THE RESOLUTION -------------------------------
// CAM_VGA		// for OV2640 only
// CAM_QVGA		// for OV2640 only
// CAM_SVGA		// for OV2640 only
// CAM_UXGA		// for OV2640 and OV2630 only

#define DEFAULT_RESOLUTION_CAM1     CAM_QVGA
#define DEFAULT_RESOLUTION_CAM2     CAM_SVGA
#define DEFAULT_FRAMERATE_CAM1      12.5
#define DEFAULT_FRAMERATE_CAM2      28

// -----------------------------------------------------

// FOR OV2640 ONLY: SELECT THE OUTPUT MODE -------------
// CAM_RAW
// CAM_YUV_422
// CAM_RGB_565

#define DEFAULT_COLOR_MODE_CAM1     CAM_YUV_422
#define DEFAULT_COLOR_MODE_CAM2     CAM_YUV_422

// -----------------------------------------------------

// THE USED FILE FORMAT IS BMP -------------------------
// -----------------------------------------------------

/* *************************************************** */

// PPIcameraIn settings
// Buffer Modes:
// PPI_CAMERA_IN_SINGLE_BUFFER
// PPI_CAMERA_IN_DOUBLE_BUFFER_NO_OVERWRITE
// PPI_CAMERA_IN_DOUBLE_BUFFER_OVERWRITE
// PPI_CAMERA_IN_MULTI_FRAME_BUFFER_NO_OVERWRITE
// PPI_CAMERA_IN_MULTI_FRAME_BUFFER_OVERWRITE


// default settings CAM1
#define DEFAULT_SDRAM_MEMORY_CAM1               16                    // [MB]
#define CAM_SIO_C1_PIN                          _PG15
#define CAM_SIO_D1_PIN                          _PG12
#define CAM_PWDN1_PIN                           0                     // not connected
#define DEFAULT_BUFFER_MODE_CAM1                PPI_CAMERA_IN_MULTI_FRAME_BUFFER_OVERWRITE
#define DEFAULT_PPI_CAM1                        0
#define RESIZE_X_VALUE_CAM1                     400
#define DEFAULT_FRAMES_TO_HOLD_CAM1             1
#define DEFAULT_MAX_FRAMES_CAM1                 2
#define OVXXXX_IVG_PERIPHERAL              8           ///< Peripheral IVG
#define OVXXXX_IVG_PERIPHERAL_ERROR        0  ///< Peripheral error IVG
#define OVXXXX_IVG_DMA_ERROR               0         ///< DMA error iVG

/*                                                                                                      // not available for BF537
// default settings CAM2
#define DEFAULT_SDRAM_MEMORY_CAM2               16                    // [MB]
#define CAM_SIO_C2_PIN			                _PF10
#define CAM_SIO_D2_PIN			                _PF11
#define CAM_PWDN2_PIN			                0				    // not connected
#define DEFAULT_BUFFER_MODE_CAM2                PPI_CAMERA_IN_MULTI_FRAME_BUFFER_OVERWRITE
#define DEFAULT_PPI_CAM2                       1
#define RESIZE_X_VALUE_CAM2                     400
#define DEFAULT_FRAMES_TO_HOLD_CAM2             1
#define DEFAULT_MAX_FRAMES_CAM2                 5
*/

// baudrate for the uart
#define UART_BAUD_RATE                      115200

// 2000 ms capture timeout
#define CAPTURE_TIMEOUT                       2000


#endif
