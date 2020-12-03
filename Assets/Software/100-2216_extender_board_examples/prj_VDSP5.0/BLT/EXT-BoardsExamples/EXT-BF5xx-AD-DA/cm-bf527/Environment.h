#ifndef _ENVIRONMENT_H_INCLUDED_
#define _ENVIRONMENT_H_INCLUDED_

#include <datatypes.h>

#if defined (__ADSPBF537__)
#include "../../../../../driver/src/bf537/gpio_global.h"
#elif defined (__ADSPBF533__)
#include "../../../../../driver/src/bf533/gpio_global.h"
#elif defined (__ADSPBF561__)
#include "../../../../../driver/src/bf561/gpio_global.h"
#elif defined (__ADSPBF527__)
#include "../../../../../driver/src/bf527/gpio_global.h"
#elif defined (__ADSPBF548__)
#include "../../../../../driver/src/bf548/gpio_global.h"
#else
#error "processor not yet supported!"
#endif


/**********************************************************************************************
 **                             Core Module Version                                          **
 **********************************************************************************************/
#define USE_CM_BF527_V1_1_5
//#define USE_CM_BF527_V1_1_2
/*********************************************************************************************/


#ifdef USE_CM_BF527_V1_1_5
    #define PROC_CLOCK_FREQ_IN              50      ///< Input clock source frequency in MHz
#endif
#ifdef USE_CM_BF527_V1_1_2
    #define PROC_CLOCK_FREQ_IN              25      ///< Input clock source frequency in MHz
#endif
#define CORE_CLK                                        500             // [MHz]
#define SYSTEM_CLK                                      125             // [MHz]


//select components to use
#define USE_AD5405_PARALLEL


//Global settings
#define GLOBAL_BURST_SIZE                               500
#define GLOBAL_SAMPLING_RATE                         100000
#define GLOBAL_AD5405_CHANNEL_MODE      AD5405_DUAL_CHANNEL

//AD5405 settings
#define AD5405_DEFAULT_SAMPLING_RATE   GLOBAL_SAMPLING_RATE             // default sampling rate
#define AD5405_DEFAULT_RING_BUFFER_SIZE  (GLOBAL_BURST_SIZE)        // default ring buffer size
#define AD5405_DEFAULT_TIMER                              0
#define AD5405_BASE_ADDRESS			            (0x20200000)            // base adress for writing to AD5405


#endif

