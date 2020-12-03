/**
 *	@file 		PPIconfig.h
 *	@ingroup 	PPI
 *	
 *	@brief 		Parrallel Port Interface Driver
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

/** @defgroup PPI
 *  @ingroup 	driverapi
 *
 * 	@brief Parallel port interface driver.
 *	
 */


#ifndef _PPI_CONFIG_H_INCLUDED_
#define _PPI_CONFIG_H_INCLUDED_

#include <datatypes.h>
#include <services/services.h>
#include "PPIbitmask.h"
#include "PPI.h"

#define PPI_DEFAULT_DMA_CHANNEL     ADI_DMA_DMA0
#define PPI_CONFIG_MAX_NOF_PPI      4

typedef void (*T_PPI_CALLBACK)(void *pa_pClientArg);///< Callback function for PPI interrupt

#define ERR_PLATFORM_INIT 			(ERR_GENERIC - 10)	///< Platform specific initialization error
#define ERR_SET_DMA_MAP				(ERR_GENERIC - 15)	///< Cannot write dma peripheral map register	
#define ERR_GET_DMA_MAP				(ERR_GENERIC - 20)	///< Cannot read dma peripheral map register	
#define ERR_DMA_OPEN				(ERR_GENERIC - 25)	///< Could not access dma channel - only with DMA manager
#define ERR_NOT_OPENED			    (ERR_GENERIC - 30)	///< Accessing a closed (not opened) dma channel - only with DMA manager
#define ERR_DMA_CLOSE				(ERR_GENERIC - 35)	///< Could not close dma channel - only with DMA manager
#define ERR_PPI_INDEX				(ERR_GENERIC - 40)	///< Invalid PPI index
#define ERR_HOOK_INTERRUPT			(ERR_GENERIC - 45)	///< Error hooking the interrupt
#define ERR_ALREADY_INITIALIZED		(ERR_GENERIC - 50)	///< Interface was already initialized


typedef struct{
    T_PPI_CALLBACK fnCallback;
    T_PPI_CALLBACK fnDMAerrCallback;
    T_PPI_CALLBACK fnPPIerrCallback;
    unsigned char cPPI;
    unsigned long nIVG;
    unsigned long nPPIerrIVG;
    unsigned long nDMAerrIVG;
} T_PPI_CONF_SPEC;

// prototype declarations
void ppi_setup(void);
void ppi_cleanup(void);

/**
	@param pa_nPPIindex			index of the PPI (0 for PPI1 1 for PPI2)
	@param pa_nStartAddress 	startaddress of dma transfer
	@param pa_nPPIcontrol		content of ppi_control register
	@param pa_nPPIframe			content of ppi_frame register
	@param pa_nPPIcount			content of ppi_count register
	@param pa_nPPIdelay			content of ppi_delay register
	@param pa_nDMAconfig		content of dma_config register
	@param pa_nXcount			content of dma_x_count register
	@param pa_nXmodify			content of dma_x_modify register
	@param pa_nYcount			content of dma_y_count register
	@param pa_nYmodify			content of dma_y_modify register
	@param T_PPI_CALLBACK pa_fnCallback		callback function for completition of dma transfer
	@return on success #ERR_NONE, appropriate errorcode otherwise
	
	sets up the PPI in general purpose mode, all registers can be written by the parameter
	values
*/
T_ERROR_CODE ppi_setup_gp (	unsigned char pa_cPPIindex,
							unsigned long pa_nStartAddress,
							unsigned short pa_nPPIcontrol,
							unsigned short pa_nPPIframe,
							unsigned short pa_nPPIcount,
							unsigned short pa_nPPIdelay,
							unsigned short pa_nDMAconfig,
							unsigned short pa_nXcount,
							signed short pa_nXmodify,
							unsigned short pa_nYcount,
							signed short pa_nYmodify,
							unsigned long pa_nNextDescrPtr,
							T_PPI_CALLBACK pa_fnCallback);
		
/**
	@param pa_nPPIindex			index of the PPI
	@param pa_nDMAmode 			single shot or auto buffer
	@param pa_nDMAdirection		read or write
	@param pa_nStartAddress		buffer start address
	@param pa_nInterruptType	no interrupt, framewise or blockwise interrupt
	@param pa_fnCallback		callback at PPI interrupt
	@param pa_nLinesPerBlock	lines per block with 2D transfer mode
	@param pa_nByteTransferWidth	number of bytes per dma transfer
	@param pa_nDMABusWidth		dma bus transfer width
	@param pa_nDirection		PPI input or output (has to match pa_nDMAdirection)
	@param pa_nPackingMode		byte packing mode of PPI
	@param pa_nFieldSelect		with interleaved mode: odd, even or both
	@param pa_nSkipping			skip odd, even or none bytes
	@param pa_nXSize			horizontal resolution
	@param pa_nYSize			vertical resolution
	@param pa_nNofBlankingLines	trailing blank lines
	@return on success #ERR_NONE, appropriate errorcode otherwise
	
	sets up the PPI in ITU656 mode, only active video is transmitted
*/
T_ERROR_CODE ppi_setup_itu656 (
		unsigned int pa_nPPIindex,
		unsigned short pa_nDMAmode,		
		unsigned short pa_nDMAdirection,
		unsigned long pa_nStartAddress,
		unsigned short pa_nInterruptType,
		T_PPI_CALLBACK pa_fnCallback,
		unsigned short pa_nLinesPerBlock,
		unsigned short pa_nByteTransferWidth,
		unsigned short pa_nDMABusWidth,
		unsigned short pa_nDirection,
		unsigned short pa_nPackingMode,
		unsigned short pa_nFieldSelect,
		unsigned short pa_nSkipping,
		unsigned short pa_nXSize,
		unsigned short pa_nYSize,
		unsigned short pa_nNofBlankingLines
		);
		
/**
	@param pa_nPPIindex
	
	starts the dma transfer for the specified PPI
	
*/
void ppi_enable (unsigned int pa_nPPIindex);

/**
	@param pa_nPPIindex
	
	stops the dma transfer for the specified PPI
*/
void ppi_disable (unsigned int pa_nPPIindex);
		
/**
	@param pa_nPPIindex
	
	starts the dma transfer for the specified PPI
	
*/
void ppi_enable_itu656 (unsigned int pa_nPPIindex);

/**
	@param pa_nPPIindex
	
	stops the dma transfer for the specified PPI
*/
void ppi_disable_itu656 (unsigned int pa_nPPIindex);

/**
	@param pa_nPPIindex
	@return  on success #ERR_NONE, appropriate errorcode otherwise
	
	releases resources allocated by the setup functions. closes the PPI transfer.
*/
T_ERROR_CODE ppi_close(unsigned int pa_nPPIindex);

/**
	@param pa_nPPIindex
	@return  content of the X_COUNT register
	
*/
unsigned short ppi_dma_getXcount (unsigned char pa_cPPIindex);

/**
	@param pa_nPPIindex
	@return  content of the Y_COUNT register
	
*/
unsigned short ppi_dma_getYcount (unsigned char pa_cPPIindex);

/**
	@param pa_nPPIindex
	@return  content of the CURR_X_COUNT register
	
*/
unsigned short ppi_dma_getCurrXcount (unsigned char pa_cPPIindex);

/**
	@param pa_nPPIindex
	@return  content of the CURR_Y_COUNT register
	
*/
unsigned short ppi_dma_getCurrYcount (unsigned char pa_cPPIindex);

void ppi_setStartAddr(unsigned char pa_cPPIindex, void *pa_pStartAddr);
void ppi_setErrorCallback (unsigned char pa_cPPIindex, T_PPI_CALLBACK pa_fnErrorCallback);

/**
 *	@public
 *	@brief		Set's up the PPI with an alternating buffer.
 *	
 *	@param 		pa_nPPIindex				index of the PPI (0 for PPI1 1 for PPI2)
 *	@param 		pa_nStartAddress0		Startaddress of first buffer for dma transfer
 *	@param		pa_nStartAddress1		Startaddress of second buffer for dma transfer
 *	@param 		pa_nPPIcontrol			Content of ppi_control register
 *	@param 		pa_nPPIframe				Content of ppi_frame register
 *	@param 		pa_nPPIcount				Content of ppi_count register
 *	@param 		pa_nPPIdelay				Content of ppi_delay register
 *	@param 		pa_nDMAconfig				Content of dma_config register
 *	@param 		pa_nXcount					Content of dma_x_count register
 *	@param 		pa_nXmodify					Content of dma_x_modify register
 *	@param 		pa_nYcount					Content of dma_y_count register
 *	@param 		pa_nYmodify					Content of dma_y_modify register
 *	@param		pa_fnErrorCallback	Callback function to handle errors.
 *	@param 		pa_fnCallback				Callback function for completion of dma transfer
 *
 *	@return		ERR_HOOK_INTERRUPT when hook interrupt fails, ERR_PLATFORM_INIT if platform specific init fails, ERR_PPI_INDEX if an invalid PPI identifier was given. ERR_NONE on sucess.
 *	
 *
 **/
T_ERROR_CODE ppi_setup_gp_alternate_buffer (	unsigned char pa_cPPIindex,
												void *pa_nStartAddress0,
												void *pa_nStartAddress1,
												unsigned short pa_nPPIcontrol,
												unsigned short pa_nPPIframe,
												unsigned short pa_nPPIcount,
												unsigned short pa_nPPIdelay,
												unsigned short pa_nDMAconfig,
												unsigned short pa_nXcount,
												signed short pa_nXmodify,
												unsigned short pa_nYcount,
												signed short pa_nYmodify,
												T_PPI_CALLBACK pa_fnErrorCallback,
												T_PPI_CALLBACK pa_fnCallback);


//@}


//@}

/**
 *	@public
 *	@brief		Hooks a Callback function to the PPI Error Interrupt handler
 *	
 *	@param 		pa_nPPIindex				index of the PPI (0 for PPI1 1 for PPI2)
 *	@param		pa_fnErrorCallback	Function pointer, to call should an error interrupt be asserted.
 *
 *	
 **/
void ppi_setPPIerrorCallback (unsigned char pa_cPPIindex, T_PPI_CALLBACK pa_fnErrorCallback);


/**
 *	@public
 *	@brief		Hooks a Callback function to the DMA Error Interrupt handler
 *	
 *	@param 		pa_nPPIindex				index of the PPI (0 for PPI1 1 for PPI2)
 *	@param		pa_fnErrorCallback	Function pointer, to call should an error interrupt be asserted.
 *
 *	
 **/
void ppi_setDMAerrorCallback (unsigned char pa_cPPIindex, T_PPI_CALLBACK pa_fnErrorCallback);

#endif

