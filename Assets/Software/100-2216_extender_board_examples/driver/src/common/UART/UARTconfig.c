/**
 *	@file 		UARTconfig.c
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
#include <environment.h>
#include "UARTconfig.h"
#include "../ErrorHandler.h"

//#define UART_FIXED_RX_IVG			12
//#define UART_FIXED_TX_IVG			12

// global extern vars for management of multiple UARTs (and multithreading stuff)

extern bool 			g_bUARTsInitialized;
extern unsigned int 	g_nUARTcount;
extern T_UART_ENTRY 	g_UARTtable[MAX_NOF_UART];
extern T_UART_SPEC 		g_UARTspec[MAX_NOF_UART];

// prototype of the platform specific initialize function
extern bool 			uart_platformInit(int pa_nUARTnumber);


/**
 *	@public
 *	@brief		Put's a single character or byte  on the UART line.
 *
 *	@param		pa_hUART	Handle to a UART resource
 *	@param		pa_cData	A char or byte of data.
 *	
 *	@return		ERR_NONE on sucess. ERR_WRITE_BUFFER_OVERFLOW on buffer overflow.
 *	
 **/
T_ERROR_CODE uart_putChar (T_UART_HANDLE pa_hUART, unsigned char pa_cData) {
	
	T_UART_INSTANCE *pInst 	 = (T_UART_INSTANCE*)pa_hUART;
	T_ERROR_CODE 	erResult = ERR_GENERIC;

#ifdef _USE_VDK_

#ifdef _UART_NO_CRITICAL_SECTION_
    #ifdef _UART_BLOCK_IVG_
        unsigned short nImask = cli();
        sti(nImask & ~pInst->nRxIvgMask & ~pInst->nTxIvgMask);
    #else
	    adi_int_SICDisable(pInst->pSpec->nOutboundPeripheralID);			// Disable Tx interrupts
	#endif    
#else
	void *pExitCriticalArg = adi_int_EnterCriticalRegion(NULL);
#endif

#endif
	
	erResult = ERR_NONE;		// Error Code to default sucess value.
	
	if (!pInst->bUARTbusy) {	// UART is not busy, so write directly to the hardware buffer register
		
#if defined (__ADSPBF548__)
		*(pInst->pSpec->pUARTier) = 0x02;	// (on bf548 pUARTier points to UARTx_IER_SET register)
#endif	
		
		pInst->bUARTbusy = true;				// Flag UART as busy
	
		*(pInst->pSpec->pUARTthr) = pa_cData;	// Initiate a transfer by writing to the UART tx Register.
	} 
	
	
	else {	// UART was already in use, so write into a Buffer instead!
		
		if (	pInst->stOutbound.pIn + 1 != pInst->stOutbound.pOut &&
				(pInst->stOutbound.pIn != pInst->stOutbound.pBufferEnd ||
				pInst->stOutbound.pOut != pInst->stOutbound.pBuffer) ) {	// everything is fine
			
			*(pInst->stOutbound.pIn)++ = pa_cData;	// Increment the buffer pointer, and write in the UART data.
			
			// control buffer limits
			if (pInst->stOutbound.pIn > pInst->stOutbound.pBufferEnd) {
				pInst->stOutbound.pIn = pInst->stOutbound.pBuffer;
			}	
		}
				
		else {										// Not enough space in Buffer, overflow occurred!
			erResult = ERR_WRITE_BUFFER_OVERFLOW;
		}
	}

#ifdef _USE_VDK_
#ifdef _UART_NO_CRITICAL_SECTION_	
    #ifdef _UART_BLOCK_IVG_
        sti(nImask);
    #else
	    adi_int_SICEnable(pInst->pSpec->nOutboundPeripheralID);				// Re-enable tx interrupt
	#endif    
#else
	adi_int_ExitCriticalRegion(pExitCriticalArg);	// Leave critical code section
#endif
#endif
	
	return erResult;	// Return ERR_NONE on sucess, and ERR_WRITE_BUFFER_OVERFLOW on buffer full.
}






/**
 *	@public
 *	@brief		Gets a Character or Byte off the UART
 *
 *	@param		pa_hUART	Handle to a UART resource
 *	@param		*pa_erCode	A pointer to a buffer, where an error code may be written to.
 *	
 *	@return		The Character or Byte that was recieved on the UART.
 *	
 **/
unsigned char uart_getChar (T_UART_HANDLE pa_hUART, T_ERROR_CODE *pa_erCode) {
	
	T_UART_INSTANCE *pInst = (T_UART_INSTANCE*)pa_hUART;
	unsigned char cRetval = 0;
		
#ifdef _USE_VDK_
#ifdef _UART_NO_CRITICAL_SECTION_
    #ifdef _UART_BLOCK_IVG_
        unsigned short nImask = cli();
        sti(nImask & ~pInst->nRxIvgMask & ~pInst->nTxIvgMask);
    #else
	    adi_int_SICDisable(pInst->pSpec->nInboundPeripheralID);			// Disable Rx Interrupt
	#endif    
#else
	void *pExitCriticalArg = adi_int_EnterCriticalRegion(NULL);		// Enter Critical Region
#endif
#endif
		
	if (pInst->stInbound.pIn == pInst->stInbound.pOut) {			// No byte in Rx Buffer
		*pa_erCode = ERR_READ_BUFFER_EMPTY;							// Write to Error Code
	} 
	
	else {
		
		if (++pInst->stInbound.pOut > pInst->stInbound.pBufferEnd) { 
			pInst->stInbound.pOut = pInst->stInbound.pBuffer;
		}
		
		cRetval = *(pInst->stInbound.pOut);							// Set recieved char from buffer for returning.
		*pa_erCode = ERR_NONE;										// ERR_NONE Error code.
	}
		
#ifdef _USE_VDK_
#ifdef _UART_NO_CRITICAL_SECTION_
    #ifdef _UART_BLOCK_IVG_
        sti(nImask);
    #else
	    adi_int_SICEnable(pInst->pSpec->nInboundPeripheralID);			// Re-Enable RX Interrupt
	#endif
#else
	adi_int_ExitCriticalRegion(pExitCriticalArg);
#endif	
#endif
	
	return cRetval;
}


// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// force code and data from here on to internal memory

#ifdef _UART_FORCE_ISR_TO_L1_
/* Interrupt Handler in Fast L1 code. */
#pragma default_section(CODE, "L1_code")
#pragma default_section(DATA, "L1_data")
#pragma default_section(CONSTDATA, "L1_data")
#pragma default_section(BSZ, "L1_data")
#pragma default_section(ALLDATA, "L1_data")
#endif


ADI_INT_HANDLER_RESULT UARTintHandler(void *pa_pClientArg) {
		
	T_UART_INSTANCE *pInst = (T_UART_INSTANCE*)pa_pClientArg;
	
	/* Check the asserted interrupt was for this Handler ( A UART Interrupt Occurred! ) */
	if(adi_int_SICInterruptAsserted(pInst->pSpec->nInboundPeripheralID) == ADI_INT_RESULT_ASSERTED ||
		adi_int_SICInterruptAsserted(pInst->pSpec->nOutboundPeripheralID) == ADI_INT_RESULT_ASSERTED  ||
		adi_int_SICInterruptAsserted(pInst->pSpec->nUARTErrorID ) == ADI_INT_RESULT_ASSERTED) {
	
#if defined (__ADSPBF548__)
		unsigned short iValue = *(pInst->pSpec->pUARTlsr);	// a read-operation on UART_IIR clears the tx interrupt,
#else														// if no other interrupt was set
		unsigned short iValue = *(pInst->pSpec->pUARTiir);								
#endif		
																					
	//	if ((iValue & 0x0001) == 0x0000) {
			//unsigned char cNoErrors = 1;
			
#if defined (__ADSPBF548__)
			if (iValue & 0x1E) {
				// errors occur
				// clear errors
				*(pInst->pSpec->pUARTlsr) |= 0x1E;
				// call error handling function
	  			errorHandler(pInst->stHeader.nDeviceID, iValue, ERR_LEV_CRITICAL);
				//cNoErrors = 0;
				return ADI_INT_RESULT_PROCESSED;	// Inform ADI's Handler that we processed the interrupt
			}
#else		
		  	if ((iValue & 0x0006) == 0x0006) {										// error occurs.	  	
		  		unsigned short iErrorValue = *(pInst->pSpec->pUARTlsr) & 0x001E;  	// reading the UART_LSR clears the interrupt.	
		  		if (iErrorValue != 0x0) {											// errors detected!	  		
		  			//cNoErrors = 0;
		  			errorHandler(pInst->stHeader.nDeviceID, iErrorValue, ERR_LEV_CRITICAL);
					return ADI_INT_RESULT_PROCESSED;	// Inform ADI's Handler that we processed the interrupt
		  		}
		  	}
#endif		  	
		  	
		  	//if (cNoErrors) {
#if defined (__ADSPBF548__)
				if ((iValue & 0x01) == 0x01) {		/* TODO: for these BF548 lines, mostly it's just a difference in a bit-mask. Could we not define these bit MASKS somewhere? */
					// data received
#else		  		
				if ((iValue & 0x0004) == 0x0004) {
					// data received					
#endif					
					// reading the UART_RBR clears the receive interrupt
					unsigned char cGarbageValue = (unsigned char)*(pInst->pSpec->pUARTrbr);

					if (pInst->pCallbackFn) {	// Check for NULL pointer! A callback method is specified
						pInst->pCallbackFn((T_UART_HANDLE)pa_pClientArg, cGarbageValue);
					} 
					
					else {	// no callback - write to buffer
						
						T_UART_BUFFER *pInbound = &(pInst->stInbound);
						
						if ( pInbound->pIn + 1 != pInbound->pOut &&
	  						(pInbound->pIn != pInbound->pBufferEnd || 
	  						 pInbound->pOut != pInbound->pBuffer)	) {	// increase buffer pointer - check ring buffer constraints
	  						 
	  						if (++pInbound->pIn > pInbound->pBufferEnd) {
	  							pInbound->pIn = pInbound->pBuffer;
	  						}
	  						
	  						*(pInbound->pIn) = cGarbageValue;			// write to the inbound buffer
	  					} 
	  					
	  					else {
	  						//Receivebuffer overflow!
	    					errorHandler(pInst->stHeader.nDeviceID, UART_ERR_RECEIVE_OVERFLOW, ERR_LEV_WARNING);
	  					}
					}
					
					return ADI_INT_RESULT_PROCESSED;	// Inform ADI's Handler that we processed the interrupt

	  			} // end of data received
	  			
#if defined (__ADSPBF548__)
				if ((iValue & 0x20) == 0x20) {
#else	  			
				if ((iValue & 0x0002) == 0x0002) {
#endif					
					
					T_UART_BUFFER *pOutbound = &(pInst->stOutbound);	//Data transmitted								
					
					if (pOutbound->pIn != pOutbound->pOut) {			// still data in the buffer - transmit it

						*(pInst->pSpec->pUARTthr) = *(pOutbound->pOut);
					
						if (++pOutbound->pOut > pOutbound->pBufferEnd) {
							pOutbound->pOut = pOutbound->pBuffer;
						}
					} 
					
					else {
						pInst->bUARTbusy = false; // Data has been transmitted, Flag UART as not in use.
#if defined (__ADSPBF548__)
						// disable tx interrupt
						*(pInst->pSpec->pUARTiir) = 0x02;	// (on bf548 pUARTiir points to UARTx_IER_CLEAR register)
#endif					
	  				}
	  				
					return ADI_INT_RESULT_PROCESSED;	// Inform ADI's Handler that we processed the interrupt

	  			} // end of data transmitted
	  	//	} // if (cNoErrors)
			//return ADI_INT_RESULT_PROCESSED;	// Inform ADI's Handler that we processed the interrupt
//		} else {
//			return ADI_INT_RESULT_NOT_PROCESSED;
//		}
	} //else {
		return ADI_INT_RESULT_NOT_PROCESSED;	// Inform ADI's Handler that we didnt process the interrupt.
	//}
}


#ifdef _UART_FORCE_ISR_TO_L1_
//return to default sections
#pragma default_section(ALLDATA)
#pragma default_section(BSZ)
#pragma default_section(CONSTDATA)
#pragma default_section(DATA)
#pragma default_section(CODE)
#endif
// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++




/**
 *	@public
 *	@brief		Writes a standard C String out on the UART
 *				Warning: Using a buffer as a string, that has no End Of String char (\0), can
 *				result in buffer over-runs.
 *
 *	@param		pa_hUART		Handle to a UART resource
 *	@param		*pa_sMessage	Pointer to a String, (Character Array) with End of String terminator.	
 *	
 *	@return		The number of characters transmitted.
 *	
 **/
unsigned int uart_writeString (T_UART_HANDLE pa_hUART, unsigned char *pa_sMessage) {
	
    if (pa_hUART) {
    	unsigned int 	nCharsWritten = 0;
    	unsigned char 	*pData = pa_sMessage; // Copy pointer, so we don't modify the passed pointer
	
    	/* 	This means that in the host application, no unexpected behaviour would result when using
    		the original passed pointer, as it will still point to the beggining of the string.	
    	 */
		
    	while (*pData) {	// Put chars until the \0 End of String char is reached.
    		if (uart_putChar(pa_hUART, *pData) == ERR_NONE) { // Put the char out on the line
    			// No Error occured, so increment the buffer pointer and put out another char!
    			pData ++;
    			nCharsWritten ++;
    		}
    	}
	
    	return nCharsWritten;
    } else {
        return 0;
    }
}



/**
 *	@public
 *	@brief		Writes a buffer of Bytes out to the UART
 *				Similar to writeString, except the length of the buffer must be given so that a 
 *				buffer overrun cannot occur.
 *
 *	@param		pa_hUART		Handle to a UART resource
 *	@param		*pa_sMessage	Pointer to a buffer, (Byte Array).
 *	@param		pa_nMsgLength	Length of the buffer in bytes.	
 *	
 *	@return		The number of bytes (Chars) transmitted.
 *	
 **/
unsigned int uart_writeMessage (T_UART_HANDLE pa_hUART, unsigned char *pa_sMessage, unsigned short pa_nMsgLength) {
	
	unsigned int nCharsWritten = 0;
	unsigned char *pData = pa_sMessage;		// Copy pointer to prevent unexpected behaviour in host application.
		
	while (nCharsWritten < pa_nMsgLength) {	// Put chars until we reach the message length.
		if (uart_putChar(pa_hUART, *pData) == ERR_NONE) {	// Put the char out on the line
			// no error occured
			pData ++;
			nCharsWritten ++;
		}
	}
	
	return nCharsWritten;
}



/**
 *	@public
 *	@brief		Configures the UARTs Mode of operation.
 *
 *	@param		pa_hUART		Handle to a UART resource
 *	@param		pa_nBaudRate	Baudrate of UART to run at in (BPs)
 *	@param		pa_cParity		Parity bits #(UART_NONE / UART_ODD / UART_EVEN)
 *	@param		pa_cWordLength	Length of a Word (8 / 7 / 6 / 5)
 *	@param		pa_cStopBits	Number of stop bits (2 or 0)
 *	
 **/
void uart_setMode(T_UART_HANDLE pa_hUART, unsigned long pa_nBaudRate, unsigned char pa_cParity, unsigned char pa_cWordLength, unsigned char pa_cStopBits) {
	
	T_UART_INSTANCE *pInst = (T_UART_INSTANCE*)pa_hUART;
	// get the right uart spec struct
	T_UART_SPEC *pSpec = g_UARTspec + pInst->nUARTnumber;
	
	volatile unsigned short *line_control_reg;
	volatile unsigned short *global_control_reg;
	volatile unsigned short *divisor_latch_reg_l;
	volatile unsigned short *divisor_latch_reg_h;
	unsigned short 			divisor, lcrMask = 0;
	unsigned short 			divisorHighbyte, divisorLowbyte;

	line_control_reg 	= pSpec->pUARTlcr;
	global_control_reg 	= pSpec->pUARTgctl;
	divisor_latch_reg_l = pSpec->pUARTdll;
	divisor_latch_reg_h = pSpec->pUARTdlh;
	
	*global_control_reg &= 0xfffe ; 					// disable UART-Clock
	*line_control_reg 	|= 0x0080; 						// setting DLAB-Flag to reach the divisor-latches
	
	asm("ssync;");
	
	divisor 			 = (unsigned short)((float)(pInst->nSystemClock) /* [Hz]*/ / ((float)pa_nBaudRate * 16.0));
	divisorHighbyte 	 = divisor;
	divisorHighbyte    >>= 8;
	divisorLowbyte 		 = (unsigned short)(divisor & 0xff);
	*divisor_latch_reg_l = divisorLowbyte;	
	
	asm("ssync;");
	
	*divisor_latch_reg_h = divisorHighbyte;				// setting the values for the baudrate.
	
	asm("ssync;");
	
	*line_control_reg 	&= (unsigned short)0x007f; 		// reseting the DLAB-Flag.
	
	asm("ssync;");
	
	switch(pa_cWordLength) {
		case 8:
	    	lcrMask |= 0x0003;
	    	break;
		case 7:
	    	lcrMask |= 0x0002;
	    	break;
		case 6:
	    	lcrMask |= 0x0001;
	    	break;
		case 5: break;	// 0x0000
	}
	
	switch(pa_cParity) {
		case UART_NONE:
			lcrMask &= 0xFFF7;
			break;
		case UART_ODD:
			lcrMask |= 0x8;
			break;
		case UART_EVEN:
			lcrMask |= 0x18;
			break;
	}
	
	if (pa_cStopBits == 2) {
		lcrMask |= 0x0004;
	}
	
	*line_control_reg = lcrMask; 		// Setting the correct values for the choosen UART-parameter
	*global_control_reg |= 0x0001;		// Enable the UART-Clock
	
	asm("ssync;");
}


/**
 *	@public
 *	@brief		Opens a UART resource, and returns a Handle.
 *
 *	@param		pa_nUARTnumber	Number of the UART device to utilise
 *	@param		pa_nSystemClock	System Clock frequency in Hz (@see getSystemClockFrequency())
 *	@param		pa_nInSize		Size of the Rx Buffer (Bytes)
 *	@param		pa_nOutSize		Size of the Tx Buffer (Bytes)
 *	@param		pa_fnCallback	Pointer to a Callback function.
 *
 *	@return		Handle to the Opened UART resource, 
 *	
 **/
T_UART_HANDLE uart_open(int pa_nUARTnumber, unsigned long pa_nSystemClock, T_BUFFER_SIZE pa_nInSize, T_BUFFER_SIZE pa_nOutSize, T_UART_DATA_CALLBACK pa_fnCallback) {
	
	if (pa_nUARTnumber >= g_nUARTcount) {	// Invalid UART number provided // TODO: What would happen if a number less than 0 was provided?
		return 0;
	}
	
	T_UART_HANDLE hUART = 0;	// Create a Handle, and initialise it to null
	
	if (!g_bUARTsInitialized) {	
		UARTs_init();			// Initialise the global UART Table
	}
	
	bool bUARTfree = false;		// Create an IN-USE Flag (default in use!)
	
	uart_platformInit(pa_nUARTnumber);		// call the platform specific init function

#ifdef _USE_VDK_
	void *pExitCriticalArg = adi_int_EnterCriticalRegion(NULL);	
#endif	
		// check the appropriate uart entry
		if (!g_UARTtable[pa_nUARTnumber].nUARTstatus) {
			g_UARTtable[pa_nUARTnumber].nUARTstatus = 1;
			bUARTfree = true;
		}
		
#ifdef _USE_VDK_
	adi_int_ExitCriticalRegion(pExitCriticalArg);	// Exit the critical region
#endif	
	
	if (bUARTfree) {	// UART is not in use
		
		// create an instance for the Handle
		T_UART_INSTANCE *pUART 			= (T_UART_INSTANCE*)malloc(sizeof(T_UART_INSTANCE));
		pUART->nUARTnumber 				= pa_nUARTnumber;
		pUART->pCallbackFn 				= pa_fnCallback;
		pUART->nSystemClock 			= pa_nSystemClock;
		pUART->bUARTbusy 				= false;
		pUART->pSpec 					= &g_UARTspec[pa_nUARTnumber];
		
		// get memory for the buffers (and set them up)
		pUART->stInbound.pBuffer 		= (unsigned char*)malloc(pa_nInSize);
		pUART->stInbound.pBufferEnd 	= pUART->stInbound.pBuffer + pa_nInSize - 1;		
		pUART->stInbound.pIn 			= pUART->stInbound.pBuffer;
		pUART->stInbound.pOut 			= pUART->stInbound.pBuffer;
		pUART->stOutbound.pBuffer 		= (unsigned char*)malloc(pa_nOutSize);
		pUART->stOutbound.pBufferEnd 	= pUART->stOutbound.pBuffer + pa_nOutSize - 1;		
		pUART->stOutbound.pIn 			= pUART->stOutbound.pBuffer;
		pUART->stOutbound.pOut 			= pUART->stOutbound.pBuffer;
		
		// setup the header to identifier this device (mostly for interrupts)
		pUART->stHeader.nIntType 		= INT_TYPE_UART;
		pUART->stHeader.nDeviceID 		= pa_nUARTnumber + UART_DEVICE_ID;
		pUART->stHeader.pErrorReg 		= pUART->pSpec->pUARTiir;

		// these parameters are used by the msd manager and fs manager
		pUART->nBlockSize 				= 1;
		pUART->nBlockCount 				= (unsigned long)pa_nInSize;
		pUART->nCylinderCount 			= 0;
		pUART->nHeadCount 				= 0;
		pUART->nSectorsPerTrack 		= (unsigned long)pa_nOutSize;
	
		// enable interrupt tx, rx and error for uart
#if defined (__ADSPBF548__)
		*(pUART->pSpec->pUARTier) |= 0x5;	// tx interrupt will be enabled within the putchar function
#else		
		*(pUART->pSpec->pUARTier) |= 0x0007;
#endif		
				
		// get the inbound IVG
		//u32 nCurrentIVG;
		
#ifdef _UART_USE_FIXED_IVG_
		pUART->nInboundIVG = UART_FIXED_RX_IVG;
		adi_int_SICSetIVG(pUART->pSpec->nInboundPeripheralID, pUART->nInboundIVG);
#else
		adi_int_SICGetIVG(pUART->pSpec->nInboundPeripheralID, &(pUART->nInboundIVG));
#endif		

        // set the IVG mask
        pUART->nRxIvgMask = 1;
        unsigned char i = 0;
        for (i=0; i<pUART->nInboundIVG; i++) {
            pUART->nRxIvgMask <<= 1;    
        }
        
		// hook the Rx interrupt handler 
		if(adi_int_CECHook(pUART->nInboundIVG, UARTintHandler, pUART, TRUE) != ADI_INT_RESULT_SUCCESS) {						
				
			free(pUART->stInbound.pBuffer);		// Free Buffers and Handles (Error)
			free(pUART->stOutbound.pBuffer);
			free(pUART);
			
			return 0;
		}		
		adi_int_SICWakeup(pUART->pSpec->nInboundPeripheralID, TRUE);
		adi_int_SICEnable(pUART->pSpec->nInboundPeripheralID);

#ifdef _UART_USE_FIXED_IVG_
		pUART->nOutboundIVG = UART_FIXED_TX_IVG;
		adi_int_SICSetIVG(pUART->pSpec->nOutboundPeripheralID, pUART->nOutboundIVG);
#else				
		// get the default outbound IVG		
		adi_int_SICGetIVG(pUART->pSpec->nOutboundPeripheralID, &(pUART->nOutboundIVG));
#endif		

        // set the IVG mask
        pUART->nTxIvgMask = 1;
        for (i=0; i<pUART->nOutboundIVG; i++) {
            pUART->nTxIvgMask <<= 1;    
        }

		if (pUART->nInboundIVG != pUART->nOutboundIVG) {
			// hook the Tx interrupt handler 
			if(adi_int_CECHook(pUART->nOutboundIVG, UARTintHandler, pUART, TRUE) != ADI_INT_RESULT_SUCCESS) {						
				
				free(pUART->stInbound.pBuffer);
				free(pUART->stOutbound.pBuffer);
				free(pUART);
				
				return 0;
			}
		}
		adi_int_SICWakeup(pUART->pSpec->nOutboundPeripheralID, TRUE);
		adi_int_SICEnable(pUART->pSpec->nOutboundPeripheralID);
				
		// get the error IVG
		if (BS_IVG_PERIPHERAL_ERROR == 0) {
		    // use default mapping
		    adi_int_SICGetIVG(pUART->pSpec->nUARTErrorID, &(pUART->nErrorIVG));
		} else {
		    pUART->nErrorIVG = BS_IVG_PERIPHERAL_ERROR;
		}
		adi_int_SICSetIVG(pUART->pSpec->nUARTErrorID, pUART->nErrorIVG);
		
		if ((pUART->nErrorIVG != pUART->nInboundIVG) && (pUART->nErrorIVG != pUART->nOutboundIVG)) {

			// hook the error interrupt handler 
			if(adi_int_CECHook(pUART->nErrorIVG, UARTintHandler, pUART, TRUE) != ADI_INT_RESULT_SUCCESS) {						
				return 0;
			}	
		}
		adi_int_SICWakeup(pUART->pSpec->nUARTErrorID, TRUE);
		adi_int_SICEnable(pUART->pSpec->nUARTErrorID);
		
		hUART = (T_UART_HANDLE)pUART;
	}
	return hUART;
}






/**
 *	@public
 *	@brief		Closes the specified 
 *
 *	@param		pa_phUART	Handle to a UART resource to be closed.
 *
 **/
void uart_close(T_UART_HANDLE pa_phUART) {
	
	if (pa_phUART) {	//TODO: This isn't really necessary, because the host application should never pass a null handle.
	
		T_UART_INSTANCE *pInst = (T_UART_INSTANCE*)pa_phUART;
		// busy - wait for the buffers to be processed
		while (pInst->stInbound.pIn != pInst->stInbound.pOut);
		while (pInst->stOutbound.pIn != pInst->stOutbound.pOut);
		// the uart has finished its work - free/unhook everything
		
		adi_int_SICDisable(pInst->pSpec->nInboundPeripheralID);				
		adi_int_SICWakeup(pInst->pSpec->nInboundPeripheralID, FALSE);				
		if (adi_int_CECUnhook(pInst->nInboundIVG, UARTintHandler, pInst) != ADI_INT_RESULT_SUCCESS) {
			// error dont know what to do
			return;
		}
		
		adi_int_SICDisable(pInst->pSpec->nOutboundPeripheralID);
		adi_int_SICWakeup(pInst->pSpec->nOutboundPeripheralID, FALSE);
		
		// now if the inbound and outbound ivg are not equal - unhook the outbound
		if (pInst->nInboundIVG != pInst->nOutboundIVG) {											
			if (adi_int_CECUnhook(pInst->nInboundIVG, UARTintHandler, pInst) != ADI_INT_RESULT_SUCCESS) {
				// error - cant unhook
				return;
			}
		}
							
/*		adi_int_SICDisable(pInst->pSpec->nUARTErrorID);
		adi_int_SICWakeup(pInst->pSpec->nUARTErrorID, FALSE);
					
		if (pInst->nErrorIVG != pInst->nInboundIVG && pInst->nErrorIVG != pInst->nOutboundIVG) {
			if (adi_int_CECUnhook(pInst->nErrorIVG, UARTintHandler, pInst) != ADI_INT_RESULT_SUCCESS) {
				// error - cant unhook
				return;
			}
		}
*/				
		// release the global uart handle
		void *pExitCriticalArg = adi_int_EnterCriticalRegion(NULL);
			// reset the status for the global uart entry
			g_UARTtable[pInst->nUARTnumber].nUARTstatus = 0;
		adi_int_ExitCriticalRegion(pExitCriticalArg);
		
		// now, clean up the instance structure (incl. buffers)		
		free(pInst->stInbound.pBuffer);
		free(pInst->stOutbound.pBuffer);
		free(pInst);
		
		pa_phUART = 0; 	// TODO: 	This modifies nothing! Because this parameter will be scrubbed once the function is left
	}					//			Because the HANDLE is not a pointer, its actually a long (32-bit addr), that gets converted to a pointer.
}





/**
 *	@public
 *	@brief		Gets the number of bytes available in the Receive Buffer
 *
 *	@param		pa_hUART	Handle to the UART resource.
 *
 *	@return		Number of bytes available in the Rx buffer.
 *	
 **/
T_BUFFER_SIZE uart_getBytesInRxBuf (T_UART_HANDLE pa_hUART) {
	
	T_UART_INSTANCE *pInst = (T_UART_INSTANCE*)pa_hUART;
	
	T_BUFFER_SIZE nBytesInBuffer = 0;
	
	if (pInst->stInbound.pIn >= pInst->stInbound.pOut) {
		nBytesInBuffer = pInst->stInbound.pIn - pInst->stInbound.pOut;
	} 
	
	else {
		nBytesInBuffer = pInst->stInbound.pBufferEnd - pInst->stInbound.pOut
						+ pInst->stInbound.pIn - pInst->stInbound.pBuffer;
	}
	
	return nBytesInBuffer;
}





/**
 *	@public
 *	@brief		Gets the number of bytes available in the Transmit Buffer
 *
 *	@param		pa_hUART	Handle to the UART resource. 
 *
 *	@return		Number of bytes available in the Rx buffer.
 *	
 **/
T_BUFFER_SIZE uart_getBytesInTxBuf (T_UART_HANDLE pa_hUART) {
	
	T_UART_INSTANCE *pInst = (T_UART_INSTANCE*)pa_hUART;
	
	T_BUFFER_SIZE nBytesInBuffer = 0;
	
	if (pInst->stOutbound.pIn >= pInst->stOutbound.pOut) {
		nBytesInBuffer = pInst->stOutbound.pIn - pInst->stOutbound.pOut;
	}
	
	else {
		nBytesInBuffer = pInst->stOutbound.pBufferEnd - pInst->stOutbound.pOut
						+ pInst->stOutbound.pIn - pInst->stOutbound.pBuffer;
	}
	
	return nBytesInBuffer;
}



/**
 *	@public
 *	@brief		Disables the UART
 *
 *	@param		pa_hUART	Handle to the UART resource. 
 *
 *	
 **/
void uart_disable(T_UART_HANDLE pa_hUART) {
	T_UART_INSTANCE *pInst = (T_UART_INSTANCE*)pa_hUART;
	// get the right uart spec struct
	T_UART_SPEC *pSpec = g_UARTspec + pInst->nUARTnumber;
	*(pSpec->pUARTgctl) &= ~0x0001;
}




/**
 *	@public
 *	@brief		Enables the UART; Enable UART not before uart_open, uart_setMode
 *
 *	@param		pa_hUART	Handle to the UART resource. 
 *
 *	
 **/
void uart_enable(T_UART_HANDLE pa_hUART) {
	T_UART_INSTANCE *pInst = (T_UART_INSTANCE*)pa_hUART;
	// get the right uart spec struct
	T_UART_SPEC *pSpec = g_UARTspec + pInst->nUARTnumber;
	*(pSpec->pUARTgctl) |= 0x0001;
}
