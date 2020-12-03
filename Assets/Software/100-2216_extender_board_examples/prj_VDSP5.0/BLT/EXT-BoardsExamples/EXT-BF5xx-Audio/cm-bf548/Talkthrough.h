#ifndef  __Talkthrough_DEFINED
	#define __Talkthrough_DEFINED

//--------------------------------------------------------------------------//
// Header files																//
//--------------------------------------------------------------------------//
#include <sys\exception.h>
#include <services/services.h>
#include <drivers/adi_dev.h>
#include <ccblkfn.h>
#include <sysreg.h>
#include <gpio_global.h>


/**********************************************************************************************
 **                             Processor Definitions                                        **
 **********************************************************************************************/
#define PROC_CLOCK_FREQ_IN                  25      ///< Input clock source frequency in MHz
#define PROC_CLOCK_FREQ_CORE                500     ///< Core frequency in MHz
#define PROC_CLOCK_FREQ_SYSTEM              125     ///< System frequency in MHz
#define PROC_CLOCK_USE_PLL_INPUT_DIV                ///< Uncomment to use the PLL input divider
#define PROC_VARIANT    ADI_PWR_PROC_BF548SBBC1533   ///< Processor variant (see adi_pwr.h)
#define PROC_PACKAGE    ADI_PWR_PACKAGE_MBGA        ///< Processor package (see adi_pwr.h)
#define PROC_VDDEXT     ADI_PWR_VDDEXT_330          ///< External Voltage supply (see adi_pwr.h)
//#define PROC_VDDINT     ADI_PWR_VLEV_140            ///< Core voltage (see adi_pwr.h)
/*********************************************************************************************/

#define BS_INT_MNG_NOF_SECONDARY_HANDLER    25          ///< Number of secondary Interrupt handlers
#define BS_NOF_USER_IVGS                    7           ///< Number of user IVGs


#define BUTTON_FLAG				_PH7
#define LED_FLAG				_PH6
    

//--------------------------------------------------------------------------//
// Symbolic constants														//
//--------------------------------------------------------------------------//

// names for codec registers, used for iCodec1836TxRegs[]
#define DAC_CONTROL_1		0x0000
#define DAC_CONTROL_2		0x1000
#define DAC_VOLUME_0		0x2000
#define DAC_VOLUME_1		0x3000
#define DAC_VOLUME_2		0x4000
#define DAC_VOLUME_3		0x5000
#define DAC_VOLUME_4		0x6000
#define DAC_VOLUME_5		0x7000
#define ADC_0_PEAK_LEVEL	0x8000
#define ADC_1_PEAK_LEVEL	0x9000
#define ADC_2_PEAK_LEVEL	0xA000
#define ADC_3_PEAK_LEVEL	0xB000
#define ADC_CONTROL_1		0xC000
#define ADC_CONTROL_2		0xD000
#define ADC_CONTROL_3		0xE000

// names for slots in ad1836 audio frame
#define INTERNAL_ADC_L0			0
#define INTERNAL_ADC_L1			1
#define INTERNAL_ADC_R0			4
#define INTERNAL_ADC_R1			5
#define INTERNAL_DAC_L0			0
#define INTERNAL_DAC_L1			1
#define INTERNAL_DAC_L2			2
#define INTERNAL_DAC_R0			4
#define INTERNAL_DAC_R1			5
#define INTERNAL_DAC_R2			6

// size of array iCodec1836TxRegs and iCodec1836RxRegs
#define CODEC_1836_REGS_LENGTH	11

// SPORT0 word length
#define SLEN_32	0x001f

// DMA flow mode
#define FLOW_1	0x1000

//--------------------------------------------------------------------------//
// Global variables															//
//--------------------------------------------------------------------------//
extern int iChannel0LeftIn;
extern int iChannel0RightIn;
extern int iChannel0LeftOut;
extern int iChannel0RightOut;
extern int iChannel1LeftIn;
extern int iChannel1RightIn;
extern int iChannel1LeftOut;
extern int iChannel1RightOut;
extern int iChannel2LeftOut;
extern int iChannel2RightOut;
extern unsigned short sCodec1836TxRegs[];
extern volatile int iRxBuffer1[];
extern volatile int iTxBuffer1[];
//extern bool btoggleinput;


//--------------------------------------------------------------------------//
// Prototypes																//
//--------------------------------------------------------------------------//
// in file Initialisation.c
void Init_EBIU(void);
void Init_Flash(void);
void Init1836(void);
void Init_Sport0(void);
void Init_DMA(void);
void Init_Sport_Interrupts(void);
void Enable_DMA_Sport(void);
void Enable_DMA_Sport0(void);
ADI_INT_HANDLER_RESULT SPORT0RxHandler(void *pa_pClientArg);

// in file Process_data.c
void Process_Data(void);

// in file ISRs.c
EX_INTERRUPT_HANDLER(Sport0_RX_ISR);

#endif //__Talkthrough_DEFINED
