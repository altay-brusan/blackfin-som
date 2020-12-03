/**
 *	@file 		MDMAconfig.c
 *	@ingroup 	MDMA
 *	
 *	@brief 		Direct Memory Access, Setup and Control
 *	
 *						
 *		
 *	BLT_DISCLAIMER
 *	
 *	@author 	Roland Oberhammer, 
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

/** @defgroup DMA
 *  @ingroup 	driverapi
 *
 * 	@brief 		DMA Setup
 *	
 */
 
/** @defgroup	MDMA
 *	@ingroup	DMA
 *
 *	@brief		Sets-up and configures MDMA transfers.
 *						Allows Memory to Memory DMA transfers to create 1D, 2D, and Circular Buffers.
 */


#include "MDMAconfig.h"

extern unsigned int g_nMDMAcount;			///< Platform specific MDMA channel count
extern T_MDMA_SPEC g_aMDMAspec[];			///< Platform specific MDMA channel description

extern bool mdma_plattformInit(void);	///< Platform specific MDMA platform initialisation



// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// +  Force instructions, and data to be compiled into protected memory.
// +  This is necessary so that the BlackSHEEP loader cannot overwrite required code
// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#pragma default_section(CODE, "secure_code")
#pragma default_section(DATA, "secure_code")
#pragma default_section(CONSTDATA, "secure_code")
#pragma default_section(BSZ, "secure_code")
#pragma default_section(ALLDATA, "secure_code")

// to be sure that all variables are in section "secure code"
#pragma align 4
static T_ERROR_CODE 	erResult 	= ERR_NONE;
static unsigned long 	timeout 	= MDMA_TRANSFER_TIMEOUT;
static unsigned short nModify 	= 0;


bool mdma_platformInit(unsigned int pa_nChannel);



/**
 *	@public
 *	@brief		Set's up an MDMA transfer.
 *	
 *	@param		pa_nMDMAindex								Index Number
 *	@param		pa_nSourceStartAddress			Memory location that the source begins (where to start reading)
 *	@param		pa_nDestinationStartAddress	Memory location that the destination begins (where to start writing)
 *	@param		pa_nXelementsToTransfer			Number of X (horizontal) ELEMENTS to transfer.
 *	@param		pa_nYelementsToTransfer			Y Number of complete lines of X width to transfer. (If X=4, and we want 3 lines, 3*4 = 12).
 *	@param		pa_nTransferWidth						8,16, or 32-bit DMA transfers. MDMA_8_BIT, MDMA_16_BIT, MDMA_32_BIT
 *	@param		pa_nInterruptType						Type of interrupt
 *	@param		pa_nStrideSource						Number of ELEMENTS (8,16 or 32-bit Element Widths) offset to read the next word of data from.
 *	@param		pa_nStrideDest							Number of ELEMENTS (8,16 or 32-bit Element Widths) offset to write the next word of data to.
 *
 *	@return		ERR_MDMA_INVALID_PARAMETER, on false parameters, ERR_MDMA_PLATFORM_INIT, on platform specific initialisation failure, or ERR_NONE on sucess.
 *	
 *
 **/
T_ERROR_CODE mdma_setup (	 unsigned int 	pa_nMDMAindex,
							 unsigned long 	pa_nSourceStartAddress, 
						 	 unsigned long 	pa_nDestinationStartAddress,  
							 unsigned short pa_nXelementsToTransfer, 
							 unsigned short pa_nYelementsToTransfer, 
							 unsigned short pa_nTransferWidth,
							 unsigned short pa_nInterruptType,
							 unsigned short pa_nStrideSource,
							 unsigned short pa_nStrideDest) {
	
	erResult = ERR_NONE;
	
	if (g_nMDMAcount > pa_nMDMAindex) {
		
		g_aMDMAspec[pa_nMDMAindex].bInUse = true;
		
		nModify = 0;
		
		switch (pa_nTransferWidth) {				/* Switch case the Transfer Width */
			case MDMA_32_BIT : 	nModify = 4;
							  	break;
			case MDMA_16_BIT : 	nModify = 2;
							  	break;
			case MDMA_8_BIT : 	nModify = 1;
							 	break;
			default :
				return ERR_MDMA_INVALID_PARAMETER;
		}
		
		/* Write source channel config registers */
		*(g_aMDMAspec[pa_nMDMAindex].pSourceStartAddr) 	= (unsigned long *)pa_nSourceStartAddress;		// Set Start Addr
		*(g_aMDMAspec[pa_nMDMAindex].pSourceXCount) 		= pa_nXelementsToTransfer;										// Set num X elements
		*(g_aMDMAspec[pa_nMDMAindex].pSourceXModify) 		= nModify * pa_nStrideSource;									// Set Source Stride X length (horizontal stride)
		*(g_aMDMAspec[pa_nMDMAindex].pSourceYCount) 		= pa_nYelementsToTransfer;										// Set num Y elements
		*(g_aMDMAspec[pa_nMDMAindex].pSourceYModify) 		= nModify * pa_nStrideSource;									// Set Source Stride Y length (vertical stride)
		*(g_aMDMAspec[pa_nMDMAindex].pSourceConfig) 		= MDMA_SOURCE_BASE_CONFIG | pa_nTransferWidth;// Set the Transfer Width.
		asm("ssync;");	// Sync the register writes
	
		/* Write destination channel config registers */
		*(g_aMDMAspec[pa_nMDMAindex].pDestStartAddr) 		= (unsigned long *)pa_nDestinationStartAddress;	// Set Start ADDR	
		*(g_aMDMAspec[pa_nMDMAindex].pDestXCount) 			= pa_nXelementsToTransfer;											// Set num of X elements
		*(g_aMDMAspec[pa_nMDMAindex].pDestXModify) 			= nModify * pa_nStrideDest;											// Set destination Stride X length (horizontal stride)
		*(g_aMDMAspec[pa_nMDMAindex].pDestYCount) 			= pa_nYelementsToTransfer;											// Set num of Y elements
		*(g_aMDMAspec[pa_nMDMAindex].pDestYModify) 			= nModify * pa_nStrideDest;											// Set destination Stride X length (vertical stride)
		*(g_aMDMAspec[pa_nMDMAindex].pDestConfig) 			= MDMA_DEST_BASE_CONFIG | pa_nTransferWidth;		// Set transfer width
		asm("ssync;");	// Sync the register writes
	} 
	
	else {
		erResult = ERR_MDMA_INVALID_PARAMETER;	
	}
	
	if (!mdma_platformInit(pa_nMDMAindex)) {							// Execute the platform specific initialisation, and check for success.
		erResult = ERR_MDMA_PLATFORM_INIT;
	}
	
	return erResult;		// Return the error code.
}



/**
 *	@public
 *	@brief		Closes an MDMA transfer.
 *	
 *	@param		pa_nMDMAindex			Index Number (MDMA identifier)
 *
 *	@return		ERR_MDMA_INVALID_PARAMETER, when an invalid MDMA identifier is used. ERR_NONE on sucess.
 *	
 *
 **/
T_ERROR_CODE mdma_close(unsigned int pa_nMDMAindex) {
	
	erResult = ERR_NONE;
	
	if (!g_aMDMAspec[pa_nMDMAindex].bInUse) {				// Check the MDMA is in use
		erResult = ERR_MDMA_INVALID_PARAMETER;
	} 
	
	else {
		g_aMDMAspec[pa_nMDMAindex].bInUse = false;		// If it was, then mark it as not in-use
	}																								// We can now say, the MDMA is closed
	
	return erResult;
}




/**
 *	@public
 *	@brief		Enables (Starts or Opens) an MDMA transfer.
 *	
 *	@param		pa_nMDMAindex			Index Number (MDMA identifier)
 *
 *	@return		ERR_MDMA_INVALID_PARAMETER, when an invalid MDMA identifier is used. ERR_NONE on sucess.
 *	
 *
 **/
T_ERROR_CODE mdma_enable (unsigned int pa_nMDMAindex) {
	
	erResult = ERR_NONE;
	
	if (!g_aMDMAspec[pa_nMDMAindex].bInUse) {		// Check MDMA identifier is in use
		erResult = ERR_MDMA_INVALID_PARAMETER;
	} 
	
	else {	//enable MDMA
		
		*(g_aMDMAspec[pa_nMDMAindex].pSourceConfig) |= 0x0001;	// Set the transfer enable bit
		asm("ssync;");
		*(g_aMDMAspec[pa_nMDMAindex].pDestConfig) |= 0x0001;		// Set the transfer enable bit
		asm("ssync;");
	}
	
	return erResult;	// Return Error Code
}




/**
 *	@public
 *	@brief		Disables (Finishes or Closes) an MDMA transfer.
 *	
 *	@param		pa_nMDMAindex			Index Number (MDMA identifier)
 *
 *	@return		ERR_MDMA_INVALID_PARAMETER, when an invalid MDMA identifier is used. ERR_NONE on sucess.
 *	
 *
 **/
T_ERROR_CODE mdma_disable (unsigned int pa_nMDMAindex) {
	
	erResult = ERR_NONE;
	
	if (!g_aMDMAspec[pa_nMDMAindex].bInUse) {		// Check MDMA identifier is in use
		erResult = ERR_MDMA_INVALID_PARAMETER;
	} 
	
	else {	//disable MDMA
		
		*(g_aMDMAspec[pa_nMDMAindex].pSourceConfig) &= 0xfffe;	// Clear the transfer enable bit
		asm("ssync;");
		*(g_aMDMAspec[pa_nMDMAindex].pDestConfig) &= 0xfffe;		// Clear the transfer enable bit
		asm("ssync;");
	}
	
	return erResult;
}





/**
 *	@public
 *	@brief		Waits for a run of DMA transfer to begin,
 *	
 *	@param		pa_nMDMAindex			Index Number (MDMA identifier)
 *
 *	@return		ERR_MDMA_INVALID_PARAMETER, when an invalid MDMA identifier is used, or ERR_MDMA_TIMEOUT when a timeout occurs. ERR_NONE on sucess.
 *	
 *
 **/
T_ERROR_CODE mdma_waitForRun (unsigned int pa_nMDMAindex) {
	
	erResult = ERR_NONE;
	
	timeout = MDMA_TRANSFER_TIMEOUT;
	
	/* wait until dma run bit is set	*/
	while ((timeout) && ((*(g_aMDMAspec[pa_nMDMAindex].pDestIrqStat) & 0x0008) == 0x0000)) {
		timeout--;
	}
	
	if (!timeout) {
		erResult = ERR_MDMA_TIMEOUT;
	}
	
	return erResult;
}





/**
 *	@public
 *	@brief		Waits for a run of DMA transfer to complete,
 *	
 *	@param		pa_nMDMAindex			Index Number (MDMA identifier)
 *
 *	@return		ERR_MDMA_INVALID_PARAMETER, when an invalid MDMA identifier is used, or ERR_MDMA_TIMEOUT when a timeout occurs. ERR_NONE on sucess.
 *	
 *
 **/
T_ERROR_CODE mdma_waitForTerminate (unsigned int pa_nMDMAindex) {
	
	erResult = ERR_NONE;
	
	timeout = MDMA_TRANSFER_TIMEOUT;
	
	/* wait until dma run bit is cleared */
	while ((timeout) && ((*(g_aMDMAspec[pa_nMDMAindex].pDestIrqStat) & 0x0008) == 0x0008)) {
		timeout--;
	}
	
	if (!timeout) {
		erResult = ERR_MDMA_TIMEOUT;
	}
	return erResult;
}


// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// +  Compile code to the default memory sections!
// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#pragma default_section(CODE)
#pragma default_section(DATA)
#pragma default_section(CONSTDATA)
#pragma default_section(BSZ)
#pragma default_section(ALLDATA)

/**
 *	@public
 *	@brief		Sets the MDMA_ROUND_ROBIN_PERIOD field in the DMA_TC_PER regsiter,
 *	
 *	@param		pa_nMDMArrPeriod			Value of the MDMA_ROUND_ROBIN_PERIOD field.
 *
 *	@return		void
 *	
 *
 **/
void mdma_setMDMAroundRobinPeriod (unsigned short pa_nMDMArrPeriod) {
    *(g_aMDMAspec[0].pTcPeriod) = (pa_nMDMArrPeriod & 0x1f) << 11;
}
