#ifndef _ENVIRONMENT_H_INCLUDED_
#define _ENVIRONMENT_H_INCLUDED_

#include <cdefbf561.h>
#include <gpio_global.h>


/**********************************************************************************************
 **                             Project specific definitions                                 **
 **********************************************************************************************/
//---------------------------------------
// Test Pictures settings for CM-BF561 and CVBS0-OUT socket:
//        S1: 01100000
//        S2: all ON
//        S3: all OFF
//        S4: 1
//        S5: 1                     
//---------------------------------------        
#define _USE_TEST_PICTURES_


//---------------------------------------
// Take Picture settings:
//        S1: 00000000
//        S2: all OFF
//        S3: all OFF
//        S4: 1
//        S5: 1                     
//---------------------------------------        
//#define _USE_TAKE_PICTURE_


//---------------------------------------
// Direct Loop Through settings:
//        S1: 10000000
//        S2: all OFF
//        S3: all ON
//        S4: 1
//        S5: 1                     
//---------------------------------------
//#define _USE_DIRECT_LOOP_THROUGH_


//---------------------------------------
// Processed Loop Through settings:
//        S1: 01100000
//        S2: all ON
//        S3: all ONFF
//        S4: 1
//        S5: 1                     
//---------------------------------------
//#define _USE_PROCESSED_LOOP_THROUGH_



// DO NOT MODIFY
#ifdef _USE_TEST_PICTURES_
    #define _USE_PPI656OUT_    
    #define _USE_AV_ADV7171_    
#endif    

#ifdef _USE_TAKE_PICTURE_
    #define _USE_AV_ADV7183_
    #define _USE_PPI656IN_
#endif 

#ifdef _USE_DIRECT_LOOP_THROUGH_
    #define _USE_AV_ADV7171_
    #define _USE_AV_ADV7183_
#endif
    
#ifdef _USE_PROCESSED_LOOP_THROUGH_
    #define _USE_AV_ADV7171_
    #define _USE_AV_ADV7183_
    #define _USE_PPI656OUT_
    #define _USE_PPI656IN_
#endif

/**********************************************************************************************/


/**********************************************************************************************
 **                             UART                                                         **
 **********************************************************************************************/
#define _USE_UART_               ///< Uncomment to use Uart
#define _UART_USE_FIXED_IVG_
#define UART_FIXED_RX_IVG 15
#define UART_FIXED_TX_IVG 14
/**********************************************************************************************/
#define UART_NR                  0       ///< UART number
#define UART_BAUDRATE            115200  ///< Baudrate of the terminal output
#define UART_RX_BUFFER_SIZE      2048    ///< Receive buffer size
#define UART_TX_BUFFER_SIZE      20000   ///< Transmit buffer size
#define UART_MESSAGE_MAX_LENGTH  256     ///< Maximal length of the message buffer
#define BS_IVG_PERIPHERAL_ERROR  0
/*********************************************************************************************/



/**********************************************************************************************
 **                             Alive LED                                                    **
 **********************************************************************************************/
#define _USE_ALIVE_LED_     ///< Uncomment to use a flashing LED for debug purposes
/**********************************************************************************************/
#define ALIVE_LED_PIN           _PF43       ///< Alive LED pin
#define ALIVE_LED_INTERVAL      100         ///< Interval for flashing LED in ms
/**********************************************************************************************/



/**********************************************************************************************
 **                             Processor Definitions                                        **
 **********************************************************************************************/
#define PROC_CLOCK_FREQ_IN              25      ///< Input clock source frequency in MHz
#define PROC_CLOCK_FREQ_CORE            525     ///< Core frequency in MHz
#define PROC_CLOCK_FREQ_SYSTEM          132     ///< System frequency in MHz
#define PROC_CLOCK_USE_PLL_INPUT_DIV            ///< Uncomment to use the PLL input divider

#define PROC_VARIANT    ADI_PWR_PROC_BF561SBB600   ///< Processor variant (see adi_pwr.h)
#define PROC_PACKAGE    ADI_PWR_PACKAGE_MBGA        ///< Processor package (see adi_pwr.h)
#define PROC_VDDEXT     ADI_PWR_VDDEXT_330          ///< External Voltage supply (see adi_pwr.h)
/*********************************************************************************************/



/**********************************************************************************************
 **                             JTAG IO                                                      **
 **********************************************************************************************/
#define _USE_JTAG_IO_       ///< Uncomment to use the JTAG IO
/*********************************************************************************************/



/**********************************************************************************************
 **                             Asyncronous Memory                                       **
 **********************************************************************************************/
#define AMI_EBIU_AMBCTL0_VALUE      0x7bb07bb0  ///< Value of the EBIU_AMBCTL0 register
#define AMI_EBIU_AMBCTL1_VALUE      0x7bb07bb0  ///< Value of the EBIU_AMBCTL1 register
#define AMI_EBIU_AMGCTL_VALUE       0x01f8      ///< Value of the EBIU_AMGCTL  register
/*********************************************************************************************/



/**********************************************************************************************
 **                             FLASH Memory                                                 **
 **********************************************************************************************/
#define FLASH_INTEL_STRATA						///< Uncomment if Intel Strata compatible flash
#define FLASH_START_ADDRESS         0x20000000  ///< Start address of the flash memory
#define FLASH_NUM_SECTORS			128         ///< Number of flash sectors
#define FLASH_SECTOR_SIZE			0x8000      ///< Size of flash sectors
#define FLASH_NOF_BANKS				2           ///< Number of flash banks
#define FLASH_BANK_SIZE				0x200000    ///< Size of a bank
#define FLASH_BANK_SELECT0			_PF4        ///< Bank select pin for bank 0 (set 0 if not support)
#define FLASH_BANK_SELECT1			0           ///< Bank select pin for bank 1 (set 0 if not support)
#define FLASH_BANK_SELECT2			0           ///< Bank select pin for bank 2 (set 0 if not support)
#define FLASH_BANK_SELECT3			0           ///< Bank select pin for bank 3 (set 0 if not support)
#define FLASH_BANK_SELECT4			0           ///< Bank select pin for bank 4 (set 0 if not support)
#define FLASH_BANK_SELECT5			0           ///< Bank select pin for bank 5 (set 0 if not support)
#define FLASH_APP_ERASE_END_ADDR	FLASH_START_ADDRESS + ((FLASH_NUM_SECTORS - 5) * FLASH_SECTOR_SIZE)    ///< End address for erasing applications in flash
#define FLASH_RSDEV_PASS_PROTECT                ///< Uncomment to protect the erase device function by password
#define FLASH_USE_WORKAROUND                    ///< Uncomment to use the flash workaround
/*********************************************************************************************/



/**********************************************************************************************
 **                             I2C interface                                                **
 **********************************************************************************************/
#define _USE_I2C_INTERFACE_               ///< Uncomment to use the I2C interface
//#define I2C_DEBUG                       ///< Uncomment to get I2C debug information
/**********************************************************************************************/
//#define I2C_GLOBAL_HANDLER_NR   0         ///< Number of the I2C interface that is used as global handler
#define I2C_BIT_RATE            100000    ///< Bitrate fot the I2C interface
#define I2C_SCL_PIN             _PF10     ///< Pin used to emulate SCL signal
#define I2C_SDA_PIN             _PF9      ///< Pin used to emulate SDA signal            
#define I2C_TIMER               1         ///< Timer used to emulate I2C
/*********************************************************************************************/



/**********************************************************************************************
 **                             PPI656Out                                                    **
 **********************************************************************************************/
//#define _USE_PPI656OUT_                     ///< Uncomment to use PPI656Out
/**********************************************************************************************/
#define PPI656OUT_PPI_NR            1           ///< Number of PPI to use (0->PPI1, 1->PPI2,...)
#define PPI656OUT_PPI_IVG           9           ///< PPI IVG, set to 0 to use the default value
#define PPI656OUT_ERROR_IVG         10           ///< PPI Error IVG, set to 0 to use the default value
#define PPI656OUT_DMA_ERROR_IVG     12           ///< DMA Error IVG, set to 0 to use the default value
#define PPI656OUT_AUTO_UPDATE       0           ///< Enable Auto Update of the buffer (0->disable; 1->enable)
#define PPI656OUT_NR_OF_PLANES      2           ///< Number of planes  
#define PPI656OUT_OVERWRITE_ACTIVE_PLANE_ALLOWED    0   ///< allowed to overwrite the active plane by PPI656OshowFrame
#define PPI656OUT_MDMA_CHANNEL      3           ///< MDMA Channel used to copy a frame
/**********************************************************************************************/



/**********************************************************************************************
 **                             Analog Video                                                 **
 **********************************************************************************************/
//#define _USE_AV_      ///< Uncomment to use the analog video device
/**********************************************************************************************/
#define AV_AD7171_I2C_ADDR     		0x2A	///< I2C address of the AD7171 device
#define AV_OUT_PPI      			PPI656OUT_PPI_NR		///< number of PPI used for analog video
#define AV_VISIBLE_X        		720		///< Resolution in x
#define AV_VISIBLE_Y        		576		///< Resolution in y
//#define AV_USE_TESTPIC						///< Uncomment to use a sample picture for debugging
#define AV_FAST_MEM_BUFFER_SIZE		5760   	///< Buffer size for fast memory. At least xWidth*2, only complete multiples of xWidth*2 makes sense 
#define AV_ADV7183_OE_PIN           _PF36   ///<  /OE pin of the video decoder ADV7183, output pixels enable
#define AV_ADV7183_IRQ_PIN          _PF35   ///< /IRQ pin of the video decoder ADV7183
#define AV_ADV7171_VIDEO_FORMAT     AV_PAL  ///< defined in AnalogVideo.h
/**********************************************************************************************/



/**********************************************************************************************
 **                             PPI656In                                                     **
 **********************************************************************************************/
//#define _USE_PPI656IN_                      ///< Uncomment to use PPI656In
/**********************************************************************************************/
#define PPI656IN_PPI_NR            0           ///< Number of PPI to use (0->PPI1, 1->PPI2,...)
#define PPI656IN_PPI_IVG           7           ///< PPI IVG, set to 0 to use the default value
#define PPI656IN_ERROR_IVG         11           ///< PPI Error IVG, set to 0 to use the default value
#define PPI656IN_DMA_ERROR_IVG     12           ///< DMA Error IVG, set to 0 to use the default value

#define PPI656IN_NR_OF_FRAMES      2          ///< Number of frames  
#define PPI656I_BUFFER_MODE        PPI656I_BUFFER_OVERWRITE   ///< allowed to overwrite the active plane by PPI656OshowFrame
/**********************************************************************************************/



/**********************************************************************************************
 **                             JPEG Encoder                                                 **
 **********************************************************************************************/
#define JPEG_ENC_HEAP_NR  1   ///< Which heap is used for encoding. 0...external SD RAM 1...internal L1 cache
/**********************************************************************************************/
 
 
#endif
