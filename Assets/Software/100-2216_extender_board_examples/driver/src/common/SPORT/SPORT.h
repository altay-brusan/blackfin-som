#ifndef _SPORT_H_INCLUDED_
#define _SPORT_H_INCLUDED_

#include <services/services.h>
#include <datatypes.h>

typedef struct {
    bool bTxInUse;
    bool bRxInUse;
    ADI_INT_PERIPHERAL_ID peripheralRxIntId;
    ADI_INT_PERIPHERAL_ID peripheralTxIntId;
    ADI_INT_PERIPHERAL_ID nDMAerrIntId;
    ADI_INT_PERIPHERAL_ID nPeriheralErrIntId;
    volatile unsigned short *pSPORTtcr1;
    volatile unsigned short *pSPORTtcr2;
    volatile unsigned short *pSPORTrcr1;            
    volatile unsigned short *pSPORTrcr2;
    volatile unsigned long *pSPORTtx;
    volatile unsigned long *pSPORTrx;
    volatile unsigned short *pSPORTstat;
    volatile unsigned short *pSPORTtclkdiv;
    volatile unsigned short *pSPORTrclkdiv;
    volatile unsigned short *pSPORTtfsdiv;
    volatile unsigned short *pSPORTrfsdiv;
    volatile unsigned short *pSPORTmcmc1;
    volatile unsigned short *pSPORTmcmc2;
    volatile unsigned long *pSPORTmrcs0;
    volatile unsigned long *pSPORTmrcs1;
    volatile unsigned long *pSPORTmrcs2;
    volatile unsigned long *pSPORTmrcs3;
    volatile unsigned long *pSPORTmtcs0;
    volatile unsigned long *pSPORTmtcs1;
    volatile unsigned long *pSPORTmtcs2;
    volatile unsigned long *pSPORTmtcs3;
    volatile unsigned short *pRxDMAirqStat;
    volatile unsigned short *pRxDMAconfig;      
    volatile void **pRxDMAstartaddr;
    volatile signed short *pRxDMAxmodify;
    volatile unsigned short *pRxDMAxcount;
    volatile unsigned short *pRxDMAcurrXcount;
    volatile signed short *pRxDMAymodify;
    volatile unsigned short *pRxDMAycount;
    volatile unsigned short *pRxDMAcurrYcount;
    volatile void **pRxNextDescrPtr;
    volatile unsigned short *pTxDMAirqStat;
    volatile unsigned short *pTxDMAconfig;      
    volatile void **pTxDMAstartaddr;
    volatile signed short *pTxDMAxmodify;
    volatile unsigned short *pTxDMAxcount;
    volatile unsigned short *pTxDMAcurrXcount;
    volatile signed short *pTxDMAymodify;
    volatile unsigned short *pTxDMAycount;
    volatile unsigned short *pTxDMAcurrYcount;
    volatile void **pTxNextDescrPtr;
} T_SPORT_SPEC;

// globals from sport_global.c
extern T_SPORT_SPEC g_aSPORTspec[];             // declared in sport_global.c
extern unsigned char g_nSPORTcount;             // declared in sport_global.c

// prototype declarations
bool sport_platformInit(unsigned char pa_cSport, bool bTxNotRx, bool bSecEnable);


#endif

