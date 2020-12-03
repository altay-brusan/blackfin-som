#ifndef _ENVIRONMENT_H_INCLUDED_
#define _ENVIRONMENT_H_INCLUDED_

#include <cdefbf533.h>
#include "gpio_global.h"

/**********************************************************************************************
 **                             Processor Definitions                                        **
 **********************************************************************************************/
#define PROC_CLOCK_FREQ_IN               25         ///< Input clock source frequency in MHz
#define PROC_CLOCK_FREQ_CORE            500         ///< Core frequency in MHz
#define PROC_CLOCK_FREQ_SYSTEM          125         ///< System frequency in MHz
#define PROC_CLOCK_USE_PLL_INPUT_DIV                ///< Uncomment to use the PLL input divider
#define PROC_VARIANT    ADI_PWR_PROC_BF533SKBC600   ///< Processor variant (see adi_pwr.h)
#define PROC_PACKAGE    ADI_PWR_PACKAGE_MBGA        ///< Processor package (see adi_pwr.h)
#define PROC_VDDEXT     ADI_PWR_VDDEXT_330          ///< External Voltage supply (see adi_pwr.h)
//#define PROC_VDDINT     ADI_PWR_VLEV_140            ///< Core voltage (see adi_pwr.h)
/*********************************************************************************************/



// UART settings
#define PERIPHERAL_ERROR_IVG        0


// components to use
#define USE_CAM1


#define BS_IVG_PERIPHERAL_ERROR 0
#define BS_INT_MNG_NOF_SECONDARY_HANDLER    25          ///< Number of secondary Interrupt handlers
#define BS_NOF_USER_IVGS                    7           ///< Number of user IVGs

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
#define DEFAULT_MANUFACTURER_CAM1       OMNIVISION

// -----------------------------------------------------

// SELECT THE RESOLUTION -------------------------------
// CAM_VGA		// for OV2640 only
// CAM_QVGA		// for OV2640 only
// CAM_SVGA		// for OV2640 only
// CAM_UXGA		// for OV2640 and OV2630 only

#define DEFAULT_RESOLUTION_CAM1     CAM_QVGA
#define DEFAULT_FRAMERATE_CAM1      12.5

// -----------------------------------------------------

// FOR OV2640 ONLY: SELECT THE OUTPUT MODE -------------
// CAM_RAW
// CAM_YUV_422
// CAM_RGB_565

#define DEFAULT_COLOR_MODE_CAM1     CAM_YUV_422

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
#define CAM_SIO_C1_PIN			                _PF4
#define CAM_SIO_D1_PIN			                _PF7
#define CAM_PWDN1_PIN			                0				    // not connected
#define DEFAULT_BUFFER_MODE_CAM1                PPI_CAMERA_IN_MULTI_FRAME_BUFFER_OVERWRITE
#define DEFAULT_PPI_CAM1                        0
#define RESIZE_X_VALUE_CAM1                     400
#define DEFAULT_FRAMES_TO_HOLD_CAM1             2
#define DEFAULT_MAX_FRAMES_CAM1                 2
#define OVXXXX_IVG_PERIPHERAL              8           ///< Peripheral IVG
#define OVXXXX_IVG_PERIPHERAL_ERROR        0  ///< Peripheral error IVG
#define OVXXXX_IVG_DMA_ERROR               0         ///< DMA error iVG


// baudrate for the uart
#define UART_BAUD_RATE			  115200

// 2000 ms capture timeout
#define CAPTURE_TIMEOUT			    2000				


#endif
