/**
 *	@file 		MDMAconfig.h
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
 *				Allows Memory to Memory DMA transfers to create 1D, 2D, and Circular Buffers.
 */


#ifndef _MDMA_CONFIG_H_INCLUDED_
#define _MDMA_CONFIG_H_INCLUDED_

#include <Datatypes.h>
#include <services/services.h>

#define ERR_MDMA_PLATFORM_INIT 				(ERR_GENERIC - 50)	///< MDMA platform specific initialisation failed. Error return code.
#define ERR_MDMA_ALREADY_INITIALIZED        (ERR_GENERIC - 60)	///< MDMA was already initialised
#define ERR_MDMA_INVALID_PARAMETER			(ERR_GENERIC - 70)	///< MDMA invalid parameter provided
#define ERR_MDMA_TIMEOUT					(ERR_GENERIC - 80)	///< MDMA Timeout, occured.

//timeout
#define MDMA_TRANSFER_TIMEOUT               0xffffff	///< Timeout Value defined.

//transfer width definitions
#define MDMA_8_BIT                          0x0000		///< MDMA 8-bit transfer width definition
#define MDMA_16_BIT                         0x0004		///< MDMA 16-bit transfer width definition
#define MDMA_32_BIT                         0x0008		///< MDMA 32-bit transfer width definition

// base initialisation of the config registers:
// stopmode, 2D, interrupt disabled, memory read operation
#define MDMA_SOURCE_BASE_CONFIG		        0x0030
// stopmode, 2D, interrupt disabled, memory write operation
#define MDMA_DEST_BASE_CONFIG		        0x0032


/**
 *	@brief 			Structure defining an MDMA specification.
 *	
 *	
 **/
typedef struct {
		volatile bool			bInUse;						///< Flags the MDMA as in use.
		ADI_INT_PERIPHERAL_ID 	peripheralIntId;			///< Peripheral interrupt ID for the MDMA channels.
		ADI_INT_PERIPHERAL_ID 	ErrorIntId;					///< Peripheral interrupt ID for handling MDMA errors.
		unsigned long 			nIVG;						///< Interrupt event level (interrupt priority)
		void 					*pReserved;					///< Reserved pointer for future use.
		volatile unsigned short *pSourceXCount;				///< Pointer to Source X count register
		volatile signed short 	*pSourceXModify;			///< Pointer to Source Horizontal stride register
		volatile unsigned short *pSourceYCount;				///< Pointer to Source Y count register.
		volatile signed short 	*pSourceYModify;			///< Pointer to Source Vertical stride register
		volatile unsigned short *pSourceConfig;				///< Pointer to Source configuration register
		volatile unsigned short *pSourceIrqStat;		    ///< Pointer to Source IRQ status.
		volatile void 			**pSourceStartAddr;			///< Pointer to Source start address register, which contains a pointer to a memory location
		volatile unsigned short *pDestXCount;				///< Pointer to the Destination X count register.
		volatile signed short 	*pDestXModify;				///< Pointer to the Destination X stride register.
		volatile unsigned short *pDestYCount;				///< Pointer to the Destination Y count register.
		volatile signed short 	*pDestYModify;				///< Pointer to the Destination Y stride register.
		volatile unsigned short *pDestConfig;				///< Pointer to the Destination configuration register.
		volatile unsigned short *pDestIrqStat;				///< Pointer to the Destination IRQ status.
		volatile void 			**pDestStartAddr;			///< Pointer to the Destination start address register, which contains a pointer to a memory location
		volatile void 			**pSourceCurrDescPtr;		///< Pointer to the Source's current Descriptor register, which contains a pointer to a memory location 
		volatile void 			**pSourceNextDescPtr;		///< Pointer to the Source's next Descriptor register, which contains a pointer to a memory location
		volatile void 			**pDestCurrDescPtr;			///< Pointer to the Destination's current Descriptor register, which contains a pointer to a memory location
		volatile void 			**pDestNextDescPtr;			///< Pointer to the Destination's next Descriptor register, which contains a pointer to a memory location
		volatile unsigned short *pTcPeriod;                 ///< Pointer to the DMA_TC_PER register
	} T_MDMA_SPEC;
	

//prototype declarations
T_ERROR_CODE mdma_setup (	 unsigned int pa_nMDMAindex,
							 unsigned long pa_nSourceStartAddress, 
						 	 unsigned long pa_nDestinationStartAddress,  
							 unsigned short pa_nXelementsToTransfer, 
							 unsigned short pa_nYelementsToTransfer, 
							 unsigned short pa_nTransferWidth,
							 unsigned short pa_nInterruptType,
							 unsigned short pa_nStrideSource,
							 unsigned short pa_nStrideDest);	
T_ERROR_CODE mdma_close(unsigned int pa_nMDMAindex);
T_ERROR_CODE mdma_enable (unsigned int pa_nMDMAindex);
T_ERROR_CODE mdma_disable (unsigned int pa_nMDMAindex);
T_ERROR_CODE mdma_waitForRun (unsigned int pa_nMDMAindex);
T_ERROR_CODE mdma_waitForTerminate (unsigned int pa_nMDMAindex);
	
#endif

