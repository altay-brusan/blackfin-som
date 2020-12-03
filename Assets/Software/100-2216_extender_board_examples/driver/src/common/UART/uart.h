#ifndef _UART_GLOBAL_INCLUDED_
#define _UART_GLOBAL_INCLUDED_

#include <services/services.h>

#define MAX_NOF_UART 	4

/**
 *	@brief 	Structure definition of a UART status
 *	
 **/
typedef struct {
		int nUARTstatus;		
} T_UART_ENTRY;

/**
 *	@brief 	Structure definition of a UART specification
 *	
 **/
typedef struct {
	ADI_INT_PERIPHERAL_ID 	nInboundPeripheralID;	///< Interrupt Peripheral ID of Rx
	ADI_INT_PERIPHERAL_ID 	nOutboundPeripheralID;	///< Interrupt Peripheral ID of Tx
	ADI_INT_PERIPHERAL_ID 	nUARTErrorID;			///< Interrupt Peripheral ID of UART Error
	ADI_INT_PERIPHERAL_ID   nDMArxErrorID;          ///< Interrupt Peripheral ID of DMA Rx Error
	ADI_INT_PERIPHERAL_ID   nDMAtxErrorID;          ///< Interrupt Peripheral ID of DMA Tx Error
	volatile unsigned short *pUARTlcr;				///< Pointer to Line Control Register
	volatile unsigned short *pUARTgctl;				///< Pointer to Global Control Register
	volatile unsigned short *pUARTdll;				///< Pointer to Devisor Latch Low Byte Register
	volatile unsigned short *pUARTdlh;				///< Pointer to Devisor Latch High Byte Register
	volatile unsigned short *pUARTiir;				///< Pointer to Interrupt Identification Register
	volatile unsigned short *pUARTlsr;				///< Pointer to Line Status Register
	volatile unsigned short *pUARTthr;				///< Pointer to Transmit Holding Register
	volatile unsigned short *pUARTrbr;				///< Pointer to Receiver Buffer Register
	volatile unsigned short *pUARTier;				///< Pointer to Interrupt Enable Register
	volatile unsigned short *pTxDMAconfig;          ///< DMA config register
	volatile void **pTxDMAstartAddr;
	volatile unsigned short *pTxDMAxCount;          ///< DMA x count register
	volatile signed short *pTxDMAxModify;
	volatile unsigned short *pTxDMAyCount;
	volatile signed short *pTxDMAyModify;
	volatile void **pTxDMAnextDescPtr;
	volatile unsigned short *pTxDMAirqStat;
	volatile unsigned short *pRxDMAconfig;          ///< DMA config register
	volatile void **pRxDMAstartAddr;
	volatile unsigned short *pRxDMAxCount;          ///< DMA x count register
	volatile signed short *pRxDMAxModify;
	volatile unsigned short *pRxDMAyCount;
	volatile signed short *pRxDMAyModify;
	volatile void **pRxDMAnextDescPtr;
	volatile unsigned short *pRxDMAirqStat;
} T_UART_SPEC;


bool uart_platformInit(int pa_nUARTnumber);

#endif
