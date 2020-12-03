#ifndef _SPORT_CONFIG_H_INCLUDED_
#define _SPORT_CONFIG_H_INCLUDED_

#include <datatypes.h>
#include <services/services.h>
#include "../GPIOconfig.h"
#include "SPORT.h"


// errors
#define ERR_SPORT_NOT_ENOUGH_MEMORY             (ERR_GENERIC - 10)
#define ERR_SPORT_HOOK_INTERRUPT                (ERR_GENERIC - 11)
#define ERR_SPORT_BUSY_TIMEOUT                  (ERR_GENERIC - 12)
#define ERR_SPORT_NOT_FOUND                     (ERR_GENERIC - 13)
#define ERR_SPORT_IN_USE                        (ERR_GENERIC - 14)
#define ERR_SPORT_NO_PACKET_READY               (ERR_GENERIC - 15)
#define ERR_SPORT_RECEIVER_IS_BUSY              (ERR_GENERIC - 16)

#define SPORT_COMM_TIMER_CONFIG                 0x000d  // pwm mode

typedef void (*T_SPORT_CALLBACK)(void); ///<callback function for SPORT interrupt
typedef unsigned long T_SPORT_HANDLE;


typedef struct {
    unsigned char *pStart;
    unsigned char *pEnd;
    unsigned char *pIn;
    unsigned char *pOut;
    unsigned short nBurstSize;
    unsigned long nElementsInBuffer;
    unsigned long nMaxElements;
} T_SPORT_BUFFER;

    
typedef struct {
    bool bTxBusy;
    bool bRxFinished;
    unsigned long nTxIVG;
    unsigned long nRxIVG;
    T_SPORT_CALLBACK fnRxCallback;
    T_SPORT_CALLBACK fnTxCallback;
    unsigned long nTxProcCyclesPerTransferCycle;            // processor cycles passed during one word transfer
    unsigned long nRxProcCyclesPerTransferCycle;            // processor cycles passed during one word transfer 
    ADI_DMA_CHANNEL_ID enDMAchannel;
    T_SPORT_BUFFER pTxBuffer;
    T_SPORT_BUFFER pRxBuffer;
    unsigned long *pDescriptorChain;
    unsigned short nSport;
    T_GPIO_MASK tMyBusyFlag;
    T_GPIO_MASK tOtherBusyFlag;
} T_SPORT_INST;


// prototype declarations
bool sport_platformInit(unsigned char pa_cSport, bool bTxNotRx, bool bSecEnable);


#endif

