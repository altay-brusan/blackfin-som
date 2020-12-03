#ifndef _ENVIRONMENT_H_INCLUDED_
#define _ENVIRONMENT_H_INCLUDED_

#include <Datatypes.h>

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

//crystal frequency
#define CLKIN		            					     25             // [MHz]
#define CORE_CLK                                        400             // [MHz]
#define SYSTEM_CLK                                      110             // [MHz]


//select components to use
#define USE_AD7266_SERIAL
#define USE_AD5415_SERIAL
//#define USE_AD5405_PARALLEL
//#define USE_PROCESS_DATA
//#define USE_SIGNAL_SAMPLES


//Global settings
#define GLOBAL_BURST_SIZE                               500
#define GLOBAL_SAMPLING_RATE                        1300000
#define GLOBAL_OFFSET                                     0
#define GLOBAL_STRIDE                                     1
#define GLOBAL_GAIN                                     0.5
#define GLOBAL_AD5415_CHANNEL_MODE      AD5415_DUAL_CHANNEL
#define GLOBAL_AD5405_CHANNEL_MODE      AD5405_DUAL_CHANNEL


//AD5415 settings
#define AD5415_DEFAULT_SAMPLING_RATE   GLOBAL_SAMPLING_RATE             // [Sps]
#define AD5415_DEFAULT_STRIDE                 GLOBAL_STRIDE             // default stride size in the AD5415-Buffer
#define AD5415_DEFAULT_OFFSET                 GLOBAL_OFFSET             // default offset in the AD5415-Buffer
#define AD5415_DEFAULT_BURST_SIZE         GLOBAL_BURST_SIZE             // default size of AD5415 buffer
#define AD5415_SPORT_NR                              0xFFFF		// SPORT0


//AD7266 settings
#define AD7266_DEFAULT_CHANNEL                            1             // default channel of AD7266, select between 1 and 6
#define AD7266_DEFAULT_SAMPLING_RATE   GLOBAL_SAMPLING_RATE             // [Sps]
#define AD7266_DEFAULT_STRIDE                 GLOBAL_STRIDE
#define AD7266_DEFAULT_OFFSET                 GLOBAL_OFFSET
#define AD7266_DEFAULT_BURST_SIZE         GLOBAL_BURST_SIZE             // default size of AD7266 buffer
#define AD7266_A0							           _PF5
#define AD7266_A1							          _PG10
#define AD7266_A2							          _PG11
#define AD7266_SPORT_NR                              0xFFFF		// SPORT0


//AD5405 settings
#define AD5405_DEFAULT_SAMPLING_RATE   GLOBAL_SAMPLING_RATE             // default sampling rate
#define AD5405_DEFAULT_RING_BUFFER_SIZE   (GLOBAL_BURST_SIZE + 500)    // default ring buffer size
#define AD5405_DEFAULT_TIMER                              0
#define AD5405_BASE_ADDRESS			            (0x20200000)


#endif

