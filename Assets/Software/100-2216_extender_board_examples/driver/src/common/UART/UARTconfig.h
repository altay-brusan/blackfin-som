/**
 *	@file 		UARTconfig.h
 *	@ingroup 	UART
 *	
 *	@brief 		UART Configuration and Communication routines
 *	
 *		
 *	BLT_DISCLAIMER
 *	
 *	@author 	Roland Oberhammer, Daniel Weber
 *	
 *	@cond svn
 *	
 *	Information of last commit
 *	$Rev::               $:  Revision of last commit
 *	$Author::            $:  Author of last commit
 *	$Date::              $:  Date of last commit
 *	
 *	@endcond
 **/
 
/** @defgroup UART
 *  @ingroup driverapi
 *  UART Module
 */



#ifndef _UART_CONFIG_H_INCLUDED_
#define _UART_CONFIG_H_INCLUDED_

#include <Datatypes.h>
#include <services/services.h>
#include "../global_int_id.h"
#include "uart.h"

#define UART_DEVICE_ID	0x1000

#define UART_NONE 		2
#define UART_ODD 		1
#define UART_EVEN 		0

// uart errors
#define UART_ERR_RECEIVE_OVERFLOW 	1
#define UART_ERR_HARDWARE			2

#define ERR_READ_BUFFER_OVERFLOW		(ERR_GENERIC + 10)
#define ERR_READ_BUFFER_EMPTY			(ERR_GENERIC + 20)
#define ERR_WRITE_BUFFER_OVERFLOW		(ERR_GENERIC + 30)

// handle for UART access
typedef long T_UART_HANDLE;

//Callbackfunction for the UART receivebuffer.
typedef void (*T_UART_DATA_CALLBACK)(T_UART_HANDLE, unsigned char);

// structs for multiple uart management

typedef unsigned short T_BUFFER_SIZE;
	


/**
 *	@brief 	Structure definition of a UART buffer.
 *	
 **/
typedef struct {
	unsigned char *pBuffer;		///< Pointer to a Buffer
	unsigned char *pBufferEnd;	///< Pointer to the End of the Buffer
	unsigned char *pIn;			///< 
	unsigned char *pOut;		///< 
} T_UART_BUFFER;


/**
 *	@brief 	A structure definition of a UART instance (This is what the handle points to).
 *	
 **/	
typedef struct {
	T_INT_HEADER 			stHeader;				///< Description Header of Interrupt Type
	int 					nUARTnumber;			///< The UART device number utilised.
	T_UART_SPEC 			*pSpec;					///< Pointer to the Spec @see T_UART_SPEC
	unsigned long 			nInboundIVG;			///< Rx Interrupt Handler Event Level (Priority)
	unsigned long 			nOutboundIVG;			///< Tx Interrupt Handler Event Level (Priority)
	unsigned long 			nErrorIVG;				///< Error Interrupt Handler Event Lever (Priority)
	unsigned long 			nSystemClock;			///< System Clock Frequency in Hz
	bool 					bUARTbusy;				///< UART busy flag
	T_UART_DATA_CALLBACK 	pCallbackFn;			///< Callback Function Pointer
	T_UART_BUFFER 			stInbound;				///< Pointer to a Rx Buffer
	T_UART_BUFFER 			stOutbound;				///< Pointer to a Tx Buffer
	unsigned short          nTxIvgMask;
	unsigned short          nRxIvgMask;
	
	/** The Next variables are for the File System Manager **/
	
	unsigned long 			nBlockCount;			///< Number of Blocks for the Buffer
	unsigned long 			nBlockSize;				///< Size of a Block in the buffer
	unsigned short 			nCylinderCount;			///< Number of Cylinders
	unsigned short 			nHeadCount;				///< Number of Heads
	unsigned short 			nSectorsPerTrack;		///< Sectors Per Track
} T_UART_INSTANCE;
		

// prototype declarations
void 			UARTs_init(void);
T_UART_HANDLE 	uart_open(int pa_nUARTnumber, unsigned long pa_nSystemClock, T_BUFFER_SIZE pa_nInSize, T_BUFFER_SIZE pa_nOutSize, T_UART_DATA_CALLBACK pa_fnCallback);
void 			uart_setMode(T_UART_HANDLE pa_hUART, unsigned long pa_nBaudRate, unsigned char pa_cParity, unsigned char pa_cWordLength, unsigned char pa_cStopBits);
void 			uart_close(T_UART_HANDLE pa_phUART);
T_ERROR_CODE 	uart_putChar(T_UART_HANDLE pa_hUART, unsigned char pa_cData);
unsigned char 	uart_getChar (T_UART_HANDLE pa_hUART, T_ERROR_CODE *pa_erCode);
unsigned int 	uart_writeString (T_UART_HANDLE pa_hUART, unsigned char *pa_sMessage);
T_BUFFER_SIZE 	uart_getBytesInRxBuf (T_UART_HANDLE pa_hUART);
T_BUFFER_SIZE 	uart_getBytesInTxBuf (T_UART_HANDLE pa_hUART);
unsigned int 	uart_writeMessage (T_UART_HANDLE pa_hUART, unsigned char *pa_sMessage, unsigned short pa_nMsgLength);
void 			uart_disable(T_UART_HANDLE pa_hUART);
void 			uart_enable(T_UART_HANDLE pa_hUART);


#endif

