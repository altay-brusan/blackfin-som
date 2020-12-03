#ifndef _ENVIRONMENT_H_INCLUDED_
#define _ENVIRONMENT_H_INCLUDED_

/**********************************************************************************************
 **                             Processor Definitions                                        **
 **********************************************************************************************/
#define PROC_CLOCK_FREQ_IN               25         ///< Input clock source frequency in MHz
#define PROC_CLOCK_FREQ_CORE            525         ///< Core frequency in MHz
#define PROC_CLOCK_FREQ_SYSTEM          133         ///< System frequency in MHz
#define PROC_CLOCK_USE_PLL_INPUT_DIV                ///< Uncomment to use the PLL input divider
#define PROC_VARIANT    ADI_PWR_PROC_BF561SBB600    ///< Processor variant (see adi_pwr.h)
#define PROC_PACKAGE    ADI_PWR_PACKAGE_MBGA        ///< Processor package (see adi_pwr.h)
#define PROC_VDDEXT     ADI_PWR_VDDEXT_330          ///< External Voltage supply (see adi_pwr.h)
//#define PROC_USE_EXTERNAL_VCCINT                    ///< Uncomment to use an external VccINT
//#define PROC_VDDINT     ADI_PWR_VLEV_140            ///< Core voltage (see adi_pwr.h)
#define BS_INT_MNG_NOF_SECONDARY_HANDLER    25          ///< Number of secondary Interrupt handlers
#define BS_NOF_USER_IVGS                    7           ///< Number of user IVGs
/*********************************************************************************************/



#endif

