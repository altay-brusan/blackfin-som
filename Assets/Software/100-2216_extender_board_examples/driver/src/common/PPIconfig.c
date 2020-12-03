/**
 *	@file 		PPIconfig.c
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
 

#include "PPI.h"
#include "PPIconfig.h"

extern unsigned int g_nPPIcount;	// Platform Specific, PPI count
extern T_PPI_SPEC 	g_aPPIspec[];	// Platform Specific, PPI specification array

//#define PPI_FIXED_IVG						// Fixed Interrupt Event Level

#ifdef 	PPI_FIXED_IVG
#define	PPI_IVG				0x0b				// Map to EVT 11 in Event Vector Register
#endif


// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// +  Force instructions, and data to be compiled to the L1 internal memory.
// +  This is necessary because PPI requires alot of speed.
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#pragma default_section(CODE, "L1_code")
#pragma default_section(DATA, "L1_data_a")
#pragma default_section(CONSTDATA, "L1_data_a")
#pragma default_section(BSZ, "L1_data_a")
#pragma default_section(ALLDATA, "L1_data_a")


static T_PPI_CONF_SPEC g_aPPIgenericSpec[PPI_CONFIG_MAX_NOF_PPI];
unsigned long g_anGpAbDescriptor[4];	// descriptor for gp setup with alternate buffer




/**
 *	@private
 *	@brief		Interrupt Request Handler for PPI interface
 *	
 *	@param		*pa_pClientArg		A structure detailing information about the asserted interrupt. Of type T_PPI_SPEC
 *
 *	@return		ADI_INT_RESULT_PROCESSED if the interrupt was processed, else ADI_INT_NOT_PROCESSED.
 *	
 *	@see			T_PPI_SPEC definition.
 *
 **/
static ADI_INT_HANDLER_RESULT PPIinterruptHandler(void *pa_pClientArg) {
  
    T_PPI_CONF_SPEC *pPPI = (T_PPI_CONF_SPEC *)pa_pClientArg;		// Provide a pointer for the Argument to the defined T_PPI_SPEC structure
	
    if (adi_int_SICInterruptAsserted(g_aPPIspec[pPPI->cPPI].peripheralIntId) == ADI_INT_RESULT_ASSERTED) {
    
        if (pPPI->fnCallback) {							// Check if a Callback function was hooked, (check not NULL!)
            pPPI->fnCallback(pa_pClientArg);  // Pass the argument pointer (Uses the same structure as we do!)
		}
		
		*(g_aPPIspec[pPPI->cPPI].pDMAirqStat) = 0x1;					// clear the interrupt
		
		return ADI_INT_RESULT_PROCESSED;			// Interrupt processed, notify ADI's ISR
	} else {
		return ADI_INT_RESULT_NOT_PROCESSED;	// Interrupt wasn't for us, notify ADI's ISR that we didn't process it!
	}
}


/**
 *	@private
 *	@brief		Interrupt Request Handler for handling errors on the PPI interface
 *	
 *	@param		*pa_pClientArg		A structure detailing information about the asserted interrupt. Of type T_PPI_SPEC
 *
 *	@return		ADI_INT_RESULT_PROCESSED if the interrupt was processed, else ADI_INT_NOT_PROCESSED.
 *	
 *	@see			T_PPI_SPEC definition.
 *
 **/
static ADI_INT_HANDLER_RESULT PPIerrorHandler(void *pa_pClientArg) {
  
	T_PPI_CONF_SPEC *pPPI = (T_PPI_CONF_SPEC *)pa_pClientArg;		// Provide a pointer for the parameter data to the defined T_PPI_SPEC structure.
	
	if (adi_int_SICInterruptAsserted(g_aPPIspec[pPPI->cPPI].errorIntId) == ADI_INT_RESULT_ASSERTED) {
		
	    if (pPPI->fnPPIerrCallback) {										// Check if a Callback function was hooked
		    pPPI->fnPPIerrCallback(pa_pClientArg);				// If so, execute it!
		}
		
		*(g_aPPIspec[pPPI->cPPI].pPPIstatus) = 0xfb00;					// clear the interrupt
		
		//unsigned short nDummy = *(pPPI->pPPIstatus);
		
		return ADI_INT_RESULT_PROCESSED;			// Processed the interrupt, so notify ADI's ISR routine.
	} 
	
	else {
		return ADI_INT_RESULT_NOT_PROCESSED;	// Interrupt wasn't processed, so notify ADI's ISR routine.
	}
}


/**
 *	@private
 *	@brief		Interrupt Request Handler for handling DMA errors
 *	
 *	@param		*pa_pClientArg		A structure detailing information about the asserted interrupt. Of type T_PPI_SPEC
 *
 *	@return		ADI_INT_RESULT_PROCESSED if the interrupt was processed, else ADI_INT_NOT_PROCESSED.
 *	
 *	@see			T_PPI_SPEC definition.
 *
 **/
static ADI_INT_HANDLER_RESULT DMAerrorHandler(void *pa_pClientArg) {
  
	T_PPI_CONF_SPEC *pPPI = (T_PPI_CONF_SPEC *)pa_pClientArg;		// Provide a pointer for the parameter data to the defined T_PPI_SPEC structure.
	
	if (adi_int_SICInterruptAsserted(g_aPPIspec[pPPI->cPPI].DMAerrorIntId) == ADI_INT_RESULT_ASSERTED) {
		
	    if (pPPI->fnDMAerrCallback) {										// Check if a Callback function was hooked
		    pPPI->fnDMAerrCallback(pa_pClientArg);				// If so, execute it!
		}
		
		*(g_aPPIspec[pPPI->cPPI].pDMAirqStat) = 0x0002;					// clear the interrupt
		
		//unsigned short nDummy = *(pPPI->pPPIstatus);
		
		return ADI_INT_RESULT_PROCESSED;			// Processed the interrupt, so notify ADI's ISR routine.
	} 
	
	else {
		return ADI_INT_RESULT_NOT_PROCESSED;	// Interrupt wasn't processed, so notify ADI's ISR routine.
	}
}


// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// +  Return to default memory sections, speed is not so critical here
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#pragma default_section(ALLDATA)
#pragma default_section(BSZ)
#pragma default_section(CONSTDATA)
#pragma default_section(DATA)
#pragma default_section(CODE)
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++



/**
 *	@public
 *	@brief		Set's up the PPI for use in ITU 656 mode.
 *	
 *	@param		pa_nPPIindex						Index (identifier) of the PPI
 *	@param		pa_nDMAmode 						DMA mode (single shot, or auto-buffer)
 *	@param 		pa_nDMAdirection				Read or Write
 *	@param 		pa_nStartAddress				Start address of the buffer
 *	@param 		pa_nInterruptType				No interrupt, frame-wise or block-wise interrupt types.
 *	@param 		pa_fnCallback						Function pointer to a PPI interrupt Callback function.
 *	@param	 	pa_nLinesPerBlock				Number of lines per block when using 2D transfer mode
 *	@param		pa_nByteTransferWidth		Number of bytes per dma transfer
 *	@param 		pa_nDMABusWidth					Dma bus transfer width
 *	@param 		pa_nDirection						PPI input or output (has to match pa_nDMAdirection)
 *	@param 		pa_nPackingMode					Byte packing mode of PPI
 *	@param 		pa_nFieldSelect					With interleaved mode: odd, even or both
 *	@param 		pa_nSkipping						Skip odd, even or none bytes
 *	@param 		pa_nXSize								Horizontal resolution
 *	@param 		pa_nYSize								Vertical resolution
 *	@param 		pa_nNofBlankingLines		Trailing blank lines
 *
 *	@return		ERR_HOOK_INTERRUPT when hook interrupt fails, ERR_PLATFORM_INIT if platform specific init fails, ERR_PPI_INDEX if an invalid PPI identifier was given. ERR_NONE on sucess.
 *	
 *
 **/
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
		unsigned short pa_nNofBlankingLines)
{
	
	T_ERROR_CODE erResult = ERR_NONE;
	
	if (pa_nPPIindex < g_nPPIcount) {
		
		
		//void *pExitCriticalArg = adi_int_EnterCriticalRegion(NULL);

		if (g_aPPIspec[pa_nPPIindex].bInUse) {		// first check if the ppi is already in use
			erResult = ERR_ALREADY_INITIALIZED;
		}
		
		//adi_int_ExitCriticalRegion(pExitCriticalArg);
		
		if (erResult == ERR_NONE) {		// no error so far - continue	
		    
		    g_aPPIgenericSpec[pa_nPPIindex].fnCallback = pa_fnCallback;
		    g_aPPIgenericSpec[pa_nPPIindex].fnDMAerrCallback = 0;
		    g_aPPIgenericSpec[pa_nPPIindex].fnPPIerrCallback = 0;
		    g_aPPIgenericSpec[pa_nPPIindex].cPPI = pa_nPPIindex;
		    g_aPPIgenericSpec[pa_nPPIindex].nIVG = 0;
		    g_aPPIgenericSpec[pa_nPPIindex].nPPIerrIVG = 0;
		    g_aPPIgenericSpec[pa_nPPIindex].nDMAerrIVG = 0;
		    
			if ( ppi_platformInit(pa_nPPIindex, 0) ) {		// If platform specific initialisation is sucessful
							
				*(g_aPPIspec[pa_nPPIindex].pPPIframe) 	= pa_nYSize + pa_nNofBlankingLines;		// Set the Frame line count
				
				/* Set the PPI control register (Direction, Packing, Field, Skipping) */
				*(g_aPPIspec[pa_nPPIindex].pPPIcontrol) = pa_nDirection | pa_nPackingMode | pa_nFieldSelect | pa_nSkipping;
				
				/* Calculate the number of rows (lines) in each frame */
				unsigned short nRowCount = (unsigned short)((2 * pa_nXSize * pa_nLinesPerBlock) / pa_nByteTransferWidth);
				
				/* Set the DMA buffers start address */
				*(volatile unsigned long*)(g_aPPIspec[pa_nPPIindex].pDMAstartaddr) = pa_nStartAddress;								
				
				*(g_aPPIspec[pa_nPPIindex].pDMAxcount) 	= nRowCount;							// Set the DMA horizontal width
				*(g_aPPIspec[pa_nPPIindex].pDMAxmodify) = pa_nByteTransferWidth;	// Set the DMA to increment to the PPI transfer width
				
				/* Set the DMA y count, (height of DMA transfer in PPI blocks.) */
				*(g_aPPIspec[pa_nPPIindex].pDMAycount) 	= (unsigned short)(pa_nYSize / pa_nLinesPerBlock);
				
				*(g_aPPIspec[pa_nPPIindex].pDMAymodify) = pa_nByteTransferWidth;	// Set the DMA to increment to the PPI transfer width
				
				/* Set the DMA's config register, mode, Interrupt type, direction and bus width. */
				*(g_aPPIspec[pa_nPPIindex].pDMAconfig) 	= pa_nDMAmode | pa_nInterruptType | pa_nDMAdirection | pa_nDMABusWidth;
				
				ppi_platformInit(pa_nPPIindex, *(g_aPPIspec[pa_nPPIindex].pPPIcontrol));	// Platform specific initialisation
				
				if (pa_fnCallback) {		// Check for NULL pointer (i.e. was a Callback function hook provided?)

#ifdef PPI_FIXED_IVG
					g_aPPIgenericSpec[pa_nPPIindex].nIVG = PPI_IVG;			// Using a Fixed Interrupt handle level (INT priority).
#else
					/* Else use the standard IVG mapping */
					adi_int_SICGetIVG(g_aPPIspec[pa_nPPIindex].peripheralIntId, &g_aPPIgenericSpec[pa_nPPIindex].nIVG);
#endif					
					/* Hook the interrupt */
					if(adi_int_CECHook(g_aPPIgenericSpec[pa_nPPIindex].nIVG, PPIinterruptHandler, (void *)&g_aPPIgenericSpec[pa_nPPIindex], false) == ADI_INT_RESULT_SUCCESS) {
						
						adi_int_SICWakeup(g_aPPIspec[pa_nPPIindex].peripheralIntId, TRUE);	// Enable interrupt to Wake the core from an idle state
						adi_int_SICEnable(g_aPPIspec[pa_nPPIindex].peripheralIntId);				// Enable the interrupt
					
					} 
										
					else {
						erResult = ERR_HOOK_INTERRUPT;
					}			
				}
			} else {
				// error the platform init function failed
				erResult = ERR_PLATFORM_INIT;
			}
		} else {
			// ppi is already initialized
		}
	} else {
		// ppi index error
		erResult = ERR_PPI_INDEX;
	}
	return erResult;
}






/**
 *	@public
 *	@brief		Closes the designated PPI interface
 *	
 *	@param		pa_nPPIindex		Index (identifier) of the PPI
 *
 *	@return		ERR_HOOK_INTERRUPT if unhooking of interrupt failed. ERR_PPI_INDEX if an invalid PPI identifier was given. ERR_NONE on sucess.
 *	
 *
 **/
T_ERROR_CODE ppi_close(unsigned int pa_nPPIindex) {
	
	T_ERROR_CODE erResult = ERR_NONE;
	
	if (pa_nPPIindex < g_nPPIcount) {		// PPI identifier is in a valid range
		
		if (g_aPPIgenericSpec[pa_nPPIindex].fnCallback) {	// If a Callback function was hooked
			
			adi_int_SICDisable(g_aPPIspec[pa_nPPIindex].peripheralIntId);		// Disable the interrupt
			
			/* Unhook our Interrupt Handler */
			if(adi_int_CECUnhook(g_aPPIgenericSpec[pa_nPPIindex].nIVG, PPIinterruptHandler, (void *)&g_aPPIgenericSpec[pa_nPPIindex]) == ADI_INT_RESULT_SUCCESS) {													
				erResult = ERR_HOOK_INTERRUPT;
			}
		}
		if (g_aPPIgenericSpec[pa_nPPIindex].fnPPIerrCallback) {	// If a PPI Error Callback function was hooked
			
			adi_int_SICDisable(g_aPPIspec[pa_nPPIindex].errorIntId);		// Disable the interrupt
			
			/* Unhook our Interrupt Handler */
			if(adi_int_CECUnhook(g_aPPIgenericSpec[pa_nPPIindex].nPPIerrIVG, PPIinterruptHandler, (void *)&g_aPPIgenericSpec[pa_nPPIindex]) == ADI_INT_RESULT_SUCCESS) {													
				erResult = ERR_HOOK_INTERRUPT;
			}
		}
		if (g_aPPIgenericSpec[pa_nPPIindex].fnDMAerrCallback) {	// If a PPI Error Callback function was hooked
			
			adi_int_SICDisable(g_aPPIspec[pa_nPPIindex].DMAerrorIntId);		// Disable the interrupt
			
			/* Unhook our Interrupt Handler */
			if(adi_int_CECUnhook(g_aPPIgenericSpec[pa_nPPIindex].nDMAerrIVG, PPIinterruptHandler, (void *)&g_aPPIgenericSpec[pa_nPPIindex]) == ADI_INT_RESULT_SUCCESS) {													
				erResult = ERR_HOOK_INTERRUPT;
			}
		}
		
		g_aPPIspec[pa_nPPIindex].bInUse	= false;// Flag ppi as free
	    g_aPPIgenericSpec[pa_nPPIindex].fnCallback = 0;
	    g_aPPIgenericSpec[pa_nPPIindex].fnDMAerrCallback = 0;
	    g_aPPIgenericSpec[pa_nPPIindex].fnPPIerrCallback = 0;
	    g_aPPIgenericSpec[pa_nPPIindex].cPPI = 0;
	    g_aPPIgenericSpec[pa_nPPIindex].nIVG = 0;
	    g_aPPIgenericSpec[pa_nPPIindex].nPPIerrIVG = 0;
	    g_aPPIgenericSpec[pa_nPPIindex].nDMAerrIVG = 0;
	    
        ppi_disable(pa_nPPIindex);
	} 
	
	else {
		erResult = ERR_PPI_INDEX;
	}
	
	return erResult;
}



//	TODO 	The following functions duplicate twice! In the interests of code density, is it possible to maybe make the itu656 functions simply pointers to the normal ppi_enable functions?
//				Not sure how we can do that, or if its possible?
/**
 *	@public
 *	@brief		Enables ITU656 PPI (Starts the DMA controller, and then the PPI running)
 *	
 *	@param		pa_nPPIindex		Index (identifier) of the PPI
 *
 *
 **/
void ppi_enable_itu656 (unsigned int pa_nPPIindex) {
	*(g_aPPIspec[pa_nPPIindex].pDMAconfig) 	|= 0x0001; 
	*(g_aPPIspec[pa_nPPIindex].pPPIcontrol) |= 0x0001;
}





/**
 *	@public
 *	@brief		Disables ITU656 PPI (Stops the PPI, and then the DMA controller running)
 *	
 *	@param		pa_nPPIindex		Index (identifier) of the PPI
 *
 *
 **/
void ppi_disable_itu656 (unsigned int pa_nPPIindex) {
	*(g_aPPIspec[pa_nPPIindex].pPPIcontrol) &= ~0x0001;
	*(g_aPPIspec[pa_nPPIindex].pDMAconfig) 	&= ~0x0001; 
}






/**
 *	@public
 *	@brief		Enables PPI (Starts the DMA controller, and then the PPI running)
 *	
 *	@param		pa_nPPIindex		Index (identifier) of the PPI
 *
 *
 **/
void ppi_enable (unsigned int pa_nPPIindex) {
	*(g_aPPIspec[pa_nPPIindex].pDMAconfig) 	|= 0x0001;
	*(g_aPPIspec[pa_nPPIindex].pPPIcontrol) |= 0x0001;
}





/**
 *	@public
 *	@brief		Disables PPI (Stops the PPI, and then the DMA controller running)
 *	
 *	@param		pa_nPPIindex		Index (identifier) of the PPI
 *
 *
 **/
void ppi_disable (unsigned int pa_nPPIindex) {
	*(g_aPPIspec[pa_nPPIindex].pPPIcontrol) &= ~0x0001;
	*(g_aPPIspec[pa_nPPIindex].pDMAconfig) 	&= ~0x0001; 
}





/**
 *	@public
 *	@brief		Set's up the PPI 
 *	
 *	@param 		pa_nPPIindex			index of the PPI (0 for PPI1 1 for PPI2)
 *	@param 		pa_nStartAddress 	Startaddress of dma transfer
 *	@param 		pa_nPPIcontrol		Content of ppi_control register
 *	@param 		pa_nPPIframe			Content of ppi_frame register
 *	@param 		pa_nPPIcount			Content of ppi_count register
 *	@param 		pa_nPPIdelay			Content of ppi_delay register
 *	@param 		pa_nDMAconfig			Content of dma_config register
 *	@param 		pa_nXcount				Content of dma_x_count register
 *	@param 		pa_nXmodify				Content of dma_x_modify register
 *	@param 		pa_nYcount				Content of dma_y_count register
 *	@param 		pa_nYmodify				Content of dma_y_modify register
 *	@param 		pa_fnCallback			Callback function for completion of dma transfer
 *
 *	@return		ERR_HOOK_INTERRUPT when hook interrupt fails, ERR_PLATFORM_INIT if platform specific init fails, ERR_PPI_INDEX if an invalid PPI identifier was given. ERR_NONE on sucess.
 *	
 *
 **/
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
							T_PPI_CALLBACK pa_fnCallback) {
								
	T_ERROR_CODE erResult = ERR_NONE;
	
	if (pa_cPPIindex < g_nPPIcount) {
		
		// TODO		Should we run EnterCriticalRegion here, when in the itu656 function we don't?
//		void *pExitCriticalArg = adi_int_EnterCriticalRegion(NULL);

		if (g_aPPIspec[pa_cPPIindex].bInUse) {		// first check if the ppi is already in use
			erResult = ERR_ALREADY_INITIALIZED;
		}
		
//		adi_int_ExitCriticalRegion(pExitCriticalArg);
		
		if (erResult == ERR_NONE) {
			// no error so far - continue		

		    g_aPPIgenericSpec[pa_cPPIindex].fnCallback = pa_fnCallback;
		    g_aPPIgenericSpec[pa_cPPIindex].fnDMAerrCallback = 0;
		    g_aPPIgenericSpec[pa_cPPIindex].fnPPIerrCallback = 0;
		    g_aPPIgenericSpec[pa_cPPIindex].cPPI = pa_cPPIindex;
		    g_aPPIgenericSpec[pa_cPPIindex].nIVG = 0;
		    g_aPPIgenericSpec[pa_cPPIindex].nPPIerrIVG = 0;
		    g_aPPIgenericSpec[pa_cPPIindex].nDMAerrIVG = 0;
		    
			if ( ppi_platformInit(pa_cPPIindex, pa_nPPIcontrol) ) {
							
				// setup interrupts
				//if (pa_fnCallback) {
								
#ifdef PPI_FIXED_IVG
					g_aPPIgenericSpec[pa_cPPIindex].nIVG = PPI_IVG;
#else
					adi_int_SICGetIVG(g_aPPIspec[pa_cPPIindex].peripheralIntId, &g_aPPIgenericSpec[pa_cPPIindex].nIVG);
#endif					
					// hook the interrupt
					if(adi_int_CECHook(g_aPPIgenericSpec[pa_cPPIindex].nIVG, PPIinterruptHandler, (void *)&g_aPPIgenericSpec[pa_cPPIindex], false) == ADI_INT_RESULT_SUCCESS) {
						adi_int_SICWakeup(g_aPPIspec[pa_cPPIindex].peripheralIntId, TRUE);
						adi_int_SICEnable(g_aPPIspec[pa_cPPIindex].peripheralIntId);
					} else {
						erResult = ERR_HOOK_INTERRUPT;
					}
					
				//}
				
                // hook the ppi error interrupt
                adi_int_SICGetIVG(g_aPPIspec[pa_cPPIindex].errorIntId, &g_aPPIgenericSpec[pa_cPPIindex].nPPIerrIVG);
                if(adi_int_CECHook(g_aPPIgenericSpec[pa_cPPIindex].nPPIerrIVG, PPIerrorHandler, (void *)&g_aPPIgenericSpec[pa_cPPIindex], false) == ADI_INT_RESULT_SUCCESS) {
                	adi_int_SICWakeup(g_aPPIspec[pa_cPPIindex].errorIntId, TRUE);
                	adi_int_SICEnable(g_aPPIspec[pa_cPPIindex].errorIntId);
                } else {
                	erResult = ERR_HOOK_INTERRUPT;
                }

                // hook the dma error interrupt
                adi_int_SICGetIVG(g_aPPIspec[pa_cPPIindex].DMAerrorIntId, &g_aPPIgenericSpec[pa_cPPIindex].nDMAerrIVG);
                if(adi_int_CECHook(g_aPPIgenericSpec[pa_cPPIindex].nDMAerrIVG, DMAerrorHandler, (void *)&g_aPPIgenericSpec[pa_cPPIindex], false) == ADI_INT_RESULT_SUCCESS) {
                	adi_int_SICWakeup(g_aPPIspec[pa_cPPIindex].DMAerrorIntId, TRUE);
                	adi_int_SICEnable(g_aPPIspec[pa_cPPIindex].DMAerrorIntId);
                } else {
                	erResult = ERR_HOOK_INTERRUPT;
                }
				
				// set dma registers
				*(g_aPPIspec[pa_cPPIindex].pDMAstartaddr) = (void *)pa_nStartAddress;
				*(g_aPPIspec[pa_cPPIindex].pDMAxcount) = pa_nXcount;
				*(g_aPPIspec[pa_cPPIindex].pDMAycount) = pa_nYcount;
				*(g_aPPIspec[pa_cPPIindex].pDMAxmodify) = pa_nXmodify;
				*(g_aPPIspec[pa_cPPIindex].pDMAymodify) = pa_nYmodify;
				*(g_aPPIspec[pa_cPPIindex].pDMAnextDescrPtr) = (void *)pa_nNextDescrPtr;
				*(g_aPPIspec[pa_cPPIindex].pDMAconfig) = pa_nDMAconfig;
				
				// set ppi registers
				*(g_aPPIspec[pa_cPPIindex].pPPIcount) = pa_nPPIcount;
				*(g_aPPIspec[pa_cPPIindex].pPPIdelay) = pa_nPPIdelay;
				*(g_aPPIspec[pa_cPPIindex].pPPIframe) = pa_nPPIframe;
				*(g_aPPIspec[pa_cPPIindex].pPPIcontrol) = pa_nPPIcontrol;
				
			} 
			
			
			else {
				// error the platform init function failed
				erResult = ERR_PLATFORM_INIT;
			}
			
		} 
		
		else {
			// ppi is already initialized			
		}
	} 
	
	else {
		// ppi index error
		erResult = ERR_PPI_INDEX;		
	}
	
	return erResult;
}






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
												T_PPI_CALLBACK pa_fnCallback) {
								
	T_ERROR_CODE erResult = ERR_NONE;
	if (pa_cPPIindex < g_nPPIcount) {
		
		// TODO Again do we also need, was commented out in ITU656 function.
		void *pExitCriticalArg = adi_int_EnterCriticalRegion(NULL);	// Make sure nothing can change the InUse state at this moment!

		if (g_aPPIspec[pa_cPPIindex].bInUse) {		// first check if the ppi is already in use
			erResult = ERR_ALREADY_INITIALIZED;
		}
		
		adi_int_ExitCriticalRegion(pExitCriticalArg);
		
		if (erResult == ERR_NONE) {
			// no error so far - continue		
			
		    g_aPPIgenericSpec[pa_cPPIindex].fnCallback = pa_fnCallback;
		    g_aPPIgenericSpec[pa_cPPIindex].fnDMAerrCallback = pa_fnErrorCallback;
		    g_aPPIgenericSpec[pa_cPPIindex].fnPPIerrCallback = 0;
		    g_aPPIgenericSpec[pa_cPPIindex].cPPI = pa_cPPIindex;
		    g_aPPIgenericSpec[pa_cPPIindex].nIVG = 0;
		    g_aPPIgenericSpec[pa_cPPIindex].nPPIerrIVG = 0;
		    g_aPPIgenericSpec[pa_cPPIindex].nDMAerrIVG = 0;
		    
			if ( ppi_platformInit(pa_cPPIindex, pa_nPPIcontrol) ) {		// Continue if the Platform specific initialisation is sucessful!
				
			  // setup the descriptor table for double buffering
			  g_anGpAbDescriptor[0] = (unsigned long)&g_anGpAbDescriptor[2];	// Next Descriptor (Second half of this array)
			  g_anGpAbDescriptor[1] = (unsigned long)pa_nStartAddress0;				// Start address for 1st buffer
			  g_anGpAbDescriptor[2] = (unsigned long)&g_anGpAbDescriptor[0];	// Next Descriptor (First half of this array)
			  g_anGpAbDescriptor[3] = (unsigned long)pa_nStartAddress1;				// Start address for 2nd buffer
			    			
				// set dma registers
				*(g_aPPIspec[pa_cPPIindex].pDMAstartaddr) = (void *)pa_nStartAddress0;
				*(g_aPPIspec[pa_cPPIindex].pDMAxcount) = pa_nXcount;
				*(g_aPPIspec[pa_cPPIindex].pDMAycount) = pa_nYcount;
				*(g_aPPIspec[pa_cPPIindex].pDMAxmodify) = pa_nXmodify;
				*(g_aPPIspec[pa_cPPIindex].pDMAymodify) = pa_nYmodify;
				*(g_aPPIspec[pa_cPPIindex].pDMAnextDescrPtr) = (void *)&g_anGpAbDescriptor[0];
				pa_nDMAconfig &= 0x00ff;
				*(g_aPPIspec[pa_cPPIindex].pDMAconfig) = pa_nDMAconfig | 0x7400;	// descriptor large model, descriptor size 4
				
				// set ppi registers
				*(g_aPPIspec[pa_cPPIindex].pPPIcount) = pa_nPPIcount;
				*(g_aPPIspec[pa_cPPIindex].pPPIdelay) = pa_nPPIdelay;
				*(g_aPPIspec[pa_cPPIindex].pPPIframe) = pa_nPPIframe;
				*(g_aPPIspec[pa_cPPIindex].pPPIcontrol) = pa_nPPIcontrol;
				
				// setup interrupts
				if (pa_fnCallback) {
								
#ifdef PPI_FIXED_IVG
					g_aPPIgenericSpec[pa_cPPIindex].nIVG = PPI_IVG;
#else
					adi_int_SICGetIVG(g_aPPIspec[pa_cPPIindex].peripheralIntId, &g_aPPIgenericSpec[pa_cPPIindex].nIVG);
#endif					
					// hook the interrupt
					if(adi_int_CECHook(g_aPPIgenericSpec[pa_cPPIindex].nIVG, PPIinterruptHandler, (void *)&g_aPPIgenericSpec[pa_cPPIindex], false) == ADI_INT_RESULT_SUCCESS) {
						adi_int_SICWakeup(g_aPPIspec[pa_cPPIindex].peripheralIntId, TRUE);
						adi_int_SICEnable(g_aPPIspec[pa_cPPIindex].peripheralIntId);
					} else {
						erResult = ERR_HOOK_INTERRUPT;
					}
				}
				if (pa_fnErrorCallback) {
				    // hook the ppi error interrupt
					adi_int_SICGetIVG(g_aPPIspec[pa_cPPIindex].errorIntId, &g_aPPIgenericSpec[pa_cPPIindex].nPPIerrIVG);					
					if(adi_int_CECHook(g_aPPIgenericSpec[pa_cPPIindex].nPPIerrIVG, PPIerrorHandler, (void *)&g_aPPIgenericSpec[pa_cPPIindex], false) == ADI_INT_RESULT_SUCCESS) {
						adi_int_SICWakeup(g_aPPIspec[pa_cPPIindex].errorIntId, TRUE);
						adi_int_SICEnable(g_aPPIspec[pa_cPPIindex].errorIntId);
					} else {
						erResult = ERR_HOOK_INTERRUPT;
					}
				}							
			} else {
				// error the platform init function failed
				erResult = ERR_PLATFORM_INIT;
			}
		} else {
			// ppi is already initialized			
		}
	} else {
		// ppi index error
		erResult = ERR_PPI_INDEX;		
	}
	
	return erResult;
}






/**
 *	@public
 *	@brief		Set's up the PPI with an alternating buffer.
 *	
 *	@param 		pa_nPPIindex				index of the PPI (0 for PPI1 1 for PPI2)
 *	@param 		pa_nStartAddr				Startaddress of dma transfer
 *
 *	
 **/
void ppi_setStartAddr(unsigned char pa_cPPIindex, void *pa_pStartAddr) {
    *(g_aPPIspec[pa_cPPIindex].pDMAstartaddr) = pa_pStartAddr;
}





/**
 *	@public
 *	@brief		Get's the PPI's X count (width)
 *	
 *	@param 		pa_nPPIindex				index of the PPI (0 for PPI1 1 for PPI2)
 *
 *	@return 	Horizontal width of the X dimension, stored in the X count register.
 *	
 **/
unsigned short ppi_dma_getXcount (unsigned char pa_cPPIindex) {
	return (*(g_aPPIspec[pa_cPPIindex].pDMAxcount));
}






/**
 *	@public
 *	@brief		Get's the PPI's Y count (height)
 *	
 *	@param 		pa_nPPIindex				index of the PPI (0 for PPI1 1 for PPI2)
 *
 *	@return 	Vertical height of the Y dimension, stored in the Y count register.
 *	
 **/
unsigned short ppi_dma_getYcount (unsigned char pa_cPPIindex) {
	return (*(g_aPPIspec[pa_cPPIindex].pDMAycount));
}




/**
 *	@public
 *	@brief		Get's the PPI's Current X count (width)
 *	
 *	@param 		pa_nPPIindex				index of the PPI (0 for PPI1 1 for PPI2)
 *
 *	@return 	Get's the position of the current DMA transfer on the horizontal X axis.
 *	
 **/
unsigned short ppi_dma_getCurrXcount (unsigned char pa_cPPIindex) {
	return (*(g_aPPIspec[pa_cPPIindex].pDMAcurrXcount));
}





/**
 *	@public
 *	@brief		Get's the PPI's Current Y count (height)
 *	
 *	@param 		pa_nPPIindex				index of the PPI (0 for PPI1 1 for PPI2)
 *
 *	@return 	Get's the position of the current DMA transfer on the vertical Y axis.
 *	
 **/
unsigned short ppi_dma_getCurrYcount (unsigned char pa_cPPIindex) {
	return (*(g_aPPIspec[pa_cPPIindex].pDMAcurrYcount));
}




/**
 *	@public
 *	@brief		Hooks a Callback function to the PPI Error Interrupt handler
 *	
 *	@param 		pa_nPPIindex				index of the PPI (0 for PPI1 1 for PPI2)
 *	@param		pa_fnErrorCallback	Function pointer, to call should an error interrupt be asserted.
 *
 *	
 **/
void ppi_setPPIerrorCallback (unsigned char pa_cPPIindex, T_PPI_CALLBACK pa_fnErrorCallback) {
	g_aPPIgenericSpec[pa_cPPIindex].fnPPIerrCallback = pa_fnErrorCallback;
}




/**
 *	@public
 *	@brief		Hooks a Callback function to the DMA Error Interrupt handler
 *	
 *	@param 		pa_nPPIindex				index of the PPI (0 for PPI1 1 for PPI2)
 *	@param		pa_fnErrorCallback	Function pointer, to call should an error interrupt be asserted.
 *
 *	
 **/
void ppi_setDMAerrorCallback (unsigned char pa_cPPIindex, T_PPI_CALLBACK pa_fnErrorCallback) {
	g_aPPIgenericSpec[pa_cPPIindex].fnDMAerrCallback = pa_fnErrorCallback;
}
