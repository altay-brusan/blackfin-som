/**
 *	@file 		EPPIconfig.c
 *	@ingroup 	EPPI
 *	
 *	@brief 		Enhanced Parrallel Port Interface Driver
 *	
 *						
 *		
 *	BLT_DISCLAIMER
 *	
 *	@author 	Thomas Hartmann
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

/** @defgroup EPPI
 *  @ingroup 	driverapi
 *
 * 	@brief Enhanced Parallel port interface driver.
 *	
 */
 

#include "EPPIconfig.h"
#include "EPPI.h"

extern unsigned int g_nEPPIcount;	// Platform Specific, EPPI count
extern T_EPPI_SPEC 	g_aEPPIspec[];	// Platform Specific, EPPI specification array

//#define PPI_FIXED_IVG						// Fixed Interrupt Event Level

#ifdef 	PPI_FIXED_IVG
#define	PPI_IVG				0x0b				// Map to EVT 11 in Event Vector Register
#endif

/* Platform specific ppi initialisation function, declared in ppi_global.c */
extern bool eppi_platformInit(unsigned char pa_cEPPIindex, unsigned long pa_nEPPIcontrol);

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// +  Force instructions, and data to be compiled to the L1 internal memory.
// +  This is necessary because PPI requires alot of speed.
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#pragma default_section(CODE, "L1_code")
#pragma default_section(DATA, "L1_data_a")
#pragma default_section(CONSTDATA, "L1_data_a")
#pragma default_section(BSZ, "L1_data_a")
#pragma default_section(ALLDATA, "L1_data_a")

#define MAX_BUFFERS     8

static T_EPPI_CONF_SPEC g_aEPPIconfigSpec[EPPI_CONFIG_MAX_NOF_EPPI];

static unsigned long g_anGpAbDescriptor[MAX_BUFFERS*2];	// descriptor for gp setup with alternate buffer
static volatile unsigned long g_nCurrentBuffer;
static unsigned long g_nBuffersUsed;

/**
 *	@private
 *	@brief		Interrupt Request Handler for EPPI interface DMA Channel
 *	
 *	@param		*pa_pClientArg		A structure detailing information about the asserted interrupt. Of type T_PPI_SPEC
 *
 *	@return		ADI_INT_RESULT_PROCESSED if the interrupt was processed, else ADI_INT_NOT_PROCESSED.
 *	
 *	@see	    T_PPI_SPEC definition.
 *
 **/
static ADI_INT_HANDLER_RESULT EPPIintHandler(void *pa_pClientArg) {
  
    T_EPPI_CONF_SPEC *pEPPI = (T_EPPI_CONF_SPEC*)pa_pClientArg;		// Provide a pointer for the Argument to the defined T_PPI_SPEC structure
	
    if (adi_int_SICInterruptAsserted(g_aEPPIspec[pEPPI->cEPPI].peripheralIntId0) == ADI_INT_RESULT_ASSERTED) {
    
        if (pEPPI->fnCallback0) {				// Check if a Callback function was hooked, (check not NULL!)
            pEPPI->fnCallback0(&g_aEPPIspec[pEPPI->cEPPI]);  // Pass the argument pointer (Uses the same structure as we do!)
	    }
	    
	    if(g_nCurrentBuffer<g_nBuffersUsed) {
	        g_nCurrentBuffer++;
	    } else {
	        g_nCurrentBuffer = 0;
	    }
	    
	    *(g_aEPPIspec[pEPPI->cEPPI].pDMA0irqStat) = 0x1;			// clear the interrupt
	    return ADI_INT_RESULT_PROCESSED;		// Interrupt processed, notify ADI's ISR
	    
    } else if (adi_int_SICInterruptAsserted(g_aEPPIspec[pEPPI->cEPPI].peripheralIntId1) == ADI_INT_RESULT_ASSERTED) {
    
        if (pEPPI->fnCallback1) {							// Check if a Callback function was hooked, (check not NULL!)
          pEPPI->fnCallback1(&g_aEPPIspec[pEPPI->cEPPI]);  // Pass the argument pointer (Uses the same structure as we do!)
		}
		
		*(g_aEPPIspec[pEPPI->cEPPI].pDMA0irqStat) = 0x1;					// clear the interrupt
		
		return ADI_INT_RESULT_PROCESSED;			// Interrupt processed, notify ADI's ISR
	} else {
		return ADI_INT_RESULT_NOT_PROCESSED;	// Interrupt wasn't for us, notify ADI's ISR that we didn't process it!
	}
}


/**
 *	@private
 *	@brief		Interrupt Request Handler for handling errors on the EPPI interface DMA Channel
 *	
 *	@param		*pa_pClientArg		A structure detailing information about the asserted interrupt. Of type T_EPPI_SPEC
 *
 *	@return		ADI_INT_RESULT_PROCESSED if the interrupt was processed, else ADI_INT_NOT_PROCESSED.
 *	
 *	@see			T_PPI_SPEC definition.
 *
 **/
static ADI_INT_HANDLER_RESULT EPPIerrorDmaHandler(void *pa_pClientArg) {
  
	T_EPPI_CONF_SPEC *pEPPI = (T_EPPI_CONF_SPEC*)pa_pClientArg;		// Provide a pointer for the parameter data to the defined T_EPPI_SPEC structure.
	
	if (adi_int_SICInterruptAsserted(g_aEPPIspec[pEPPI->cEPPI].DMAerrorIntId0) == ADI_INT_RESULT_ASSERTED) {
		
	  if (pEPPI->fnErrDmaCallback0) {										// Check if a Callback function was hooked
		    pEPPI->fnErrDmaCallback0(&g_aEPPIspec[pEPPI->cEPPI]);				// If so, execute it!
		}
		
		*(g_aEPPIspec[pEPPI->cEPPI].pDMA0irqStat) = 0x0002;					// clear the interrupt
		
		
		
		return ADI_INT_RESULT_PROCESSED;			// Processed the interrupt, so notify ADI's ISR routine.
	} else if (adi_int_SICInterruptAsserted(g_aEPPIspec[pEPPI->cEPPI].DMAerrorIntId1) == ADI_INT_RESULT_ASSERTED) {
		
	  if (pEPPI->fnErrDmaCallback1) {										// Check if a Callback function was hooked
		    pEPPI->fnErrDmaCallback1(&g_aEPPIspec[pEPPI->cEPPI]);				// If so, execute it!
		}
		
		*(g_aEPPIspec[pEPPI->cEPPI].pDMA0irqStat) = 0x0002;					// clear the interrupt
		
		
		
		return ADI_INT_RESULT_PROCESSED;			// Processed the interrupt, so notify ADI's ISR routine.
	} else {
		return ADI_INT_RESULT_NOT_PROCESSED;	// Interrupt wasn't processed, so notify ADI's ISR routine.
	}

}


/**
 *	@private
 *	@brief		Interrupt Request Handler for handling peripheral errors on the EPPI interface
 *	
 *	@param		*pa_pClientArg		A structure detailing information about the asserted interrupt. Of type T_EPPI_SPEC
 *
 *	@return		ADI_INT_RESULT_PROCESSED if the interrupt was processed, else ADI_INT_NOT_PROCESSED.
 *	
 *	@see			T_PPI_SPEC definition.
 *
 **/
static ADI_INT_HANDLER_RESULT EPPIerrorPerHandler(void *pa_pClientArg) {
  
	T_EPPI_CONF_SPEC *pEPPI = (T_EPPI_CONF_SPEC*)pa_pClientArg;		// Provide a pointer for the parameter data to the defined T_EPPI_SPEC structure.
	
	if (adi_int_SICInterruptAsserted(g_aEPPIspec[pEPPI->cEPPI].perErrorIntId0) == ADI_INT_RESULT_ASSERTED) {
		
	  if (pEPPI->fnErrPerCallback0) {										// Check if a Callback function was hooked
		    pEPPI->fnErrPerCallback0(&g_aEPPIspec[pEPPI->cEPPI]);				// If so, execute it!
		}
		
		*(g_aEPPIspec[pEPPI->cEPPI].pEPPIstatus) = 0x003F;					// clear the interrupt
		
		//unsigned short nDummy = *(pPPI->pPPIstatus);
		
		return ADI_INT_RESULT_PROCESSED;			// Processed the interrupt, so notify ADI's ISR routine.
	} else if (adi_int_SICInterruptAsserted(g_aEPPIspec[pEPPI->cEPPI].perErrorIntId1) == ADI_INT_RESULT_ASSERTED) {
		
	  if (pEPPI->fnErrPerCallback0) {										// Check if a Callback function was hooked
		    pEPPI->fnErrPerCallback0(&g_aEPPIspec[pEPPI->cEPPI]);				// If so, execute it!
		}
		
		*(g_aEPPIspec[pEPPI->cEPPI].pEPPIstatus) = 0x003F;					// clear the interrupt
		
		//unsigned short nDummy = *(pPPI->pPPIstatus);
		
		return ADI_INT_RESULT_PROCESSED;			// Processed the interrupt, so notify ADI's ISR routine.
	} else {
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
 *  @brief      Initial setup of eppi config to ensure defined starting conditions
 **/
void eppi_setup(void) {
    
    unsigned char ucEPPInr = 0;
    
    for(ucEPPInr = 0;ucEPPInr < EPPI_CONFIG_MAX_NOF_EPPI;ucEPPInr++){
        
		g_aEPPIconfigSpec[ucEPPInr].fnCallback0 = 0;
		g_aEPPIconfigSpec[ucEPPInr].fnErrPerCallback0 = 0;
		g_aEPPIconfigSpec[ucEPPInr].fnErrDmaCallback0 = 0;
		g_aEPPIconfigSpec[ucEPPInr].fnCallback1 = 0;
		g_aEPPIconfigSpec[ucEPPInr].fnErrPerCallback1 = 0;
		g_aEPPIconfigSpec[ucEPPInr].fnErrDmaCallback1 = 0;
		g_aEPPIconfigSpec[ucEPPInr].cEPPI = 0;
		g_aEPPIconfigSpec[ucEPPInr].nIVG0 = 0;
		g_aEPPIconfigSpec[ucEPPInr].nPerErrorIVG0 = 0;
		g_aEPPIconfigSpec[ucEPPInr].nDMAerrorIVG0 = 0;
		g_aEPPIconfigSpec[ucEPPInr].nIVG1 = 0;
		g_aEPPIconfigSpec[ucEPPInr].nDMAerrorIVG1 = 0;
		g_aEPPIconfigSpec[ucEPPInr].nPerErrorIVG1 = 0;
    }
}



/**
 *  @brief      Cleanup if eppi config is no longer used
 **/
 void eppi_cleanup(void) {
     
     //currently noting to do
 }
 
 

/**
 *  @brief      set's up the EPPI module and also all configures output multiplexers, all registers are written according
 *	            to the pa_tPPIconig parameter.
 *
 *	@public
 *  @param pa_tPPIconig			structure with configuration information of the EPPI
 *	@return on success          #ERR_NONE, appropriate errorcode otherwise
 *	 
 *
 **/
T_ERROR_CODE eppi_open (T_EPPI_CONF* pa_tEPPIconfig) {
								
	T_ERROR_CODE erResult = ERR_NONE;
	unsigned char cEPPInr = pa_tEPPIconfig->cEPPInr; // to avoid typing
	
	if (cEPPInr < g_nEPPIcount) {
		
		void *pExitCriticalArg = adi_int_EnterCriticalRegion(NULL);

		if (g_aEPPIspec[cEPPInr].bInUse) {		// first check if the ppi is already in use
			erResult = ERR_ALREADY_INITIALIZED;
		}
		
		adi_int_ExitCriticalRegion(pExitCriticalArg);
		
		if (erResult == ERR_NONE) {
			// no error so far - continue
			g_aEPPIconfigSpec[cEPPInr].cEPPI = cEPPInr;
					
			if ( eppi_platformInit(pa_tEPPIconfig->cEPPInr, pa_tEPPIconfig->nEPPIcontrol) ) {
			    
				// setup interrupts for channel 0
				if (pa_tEPPIconfig->fnCallback0) {
					
					g_aEPPIconfigSpec[cEPPInr].fnCallback0 = pa_tEPPIconfig->fnCallback0;
				}
					
				if(pa_tEPPIconfig->nIVG0) {
				    g_aEPPIconfigSpec[cEPPInr].nIVG0 = pa_tEPPIconfig->nIVG0;
				    adi_int_SICSetIVG(g_aEPPIspec[cEPPInr].peripheralIntId0, g_aEPPIconfigSpec[cEPPInr].nIVG0);
				} else {
				    adi_int_SICGetIVG(g_aEPPIspec[cEPPInr].peripheralIntId0, &g_aEPPIconfigSpec[cEPPInr].nIVG0);
				}
				
				// hook the interrupt
				if(adi_int_CECHook(g_aEPPIconfigSpec[cEPPInr].nIVG0, EPPIintHandler, (void *)&g_aEPPIconfigSpec[cEPPInr], false) == ADI_INT_RESULT_SUCCESS) {
					adi_int_SICWakeup(g_aEPPIspec[cEPPInr].peripheralIntId0, TRUE);
					adi_int_SICEnable(g_aEPPIspec[cEPPInr].peripheralIntId0);
				} else {
					erResult = ERR_HOOK_INTERRUPT;
				}
#ifdef _EPPI_CONFIG_HOOK_ERROR_INT_
				if(pa_tEPPIconfig->fnErrPerCallback0) {
					g_aEPPIconfigSpec[cEPPInr].fnErrPerCallback0 = pa_tEPPIconfig->fnErrPerCallback0;
				}
					
				if(pa_tEPPIconfig->nPerErrorIVG0) {
				    g_aEPPIconfigSpec[cEPPInr].nPerErrorIVG0 = pa_tEPPIconfig->nPerErrorIVG0;
				    adi_int_SICSetIVG(g_aEPPIspec[cEPPInr].perErrorIntId0, g_aEPPIconfigSpec[cEPPInr].nPerErrorIVG0);
				} else {
				    adi_int_SICGetIVG(g_aEPPIspec[cEPPInr].perErrorIntId0, &g_aEPPIconfigSpec[cEPPInr].nPerErrorIVG0);
				}
				// hook the eppi error interrupt
				if(adi_int_CECHook(g_aEPPIconfigSpec[cEPPInr].nPerErrorIVG0, EPPIerrorPerHandler, (void *)&g_aEPPIconfigSpec[cEPPInr], false) == ADI_INT_RESULT_SUCCESS) {
					adi_int_SICWakeup(g_aEPPIspec[cEPPInr].perErrorIntId0, TRUE);
					adi_int_SICEnable(g_aEPPIspec[cEPPInr].perErrorIntId0);
				} else {
					erResult = ERR_HOOK_INTERRUPT;
				}
				
				if(pa_tEPPIconfig->fnErrDmaCallback0) {
					g_aEPPIconfigSpec[cEPPInr].fnErrDmaCallback0 = pa_tEPPIconfig->fnErrDmaCallback0;
				}
					
				if(pa_tEPPIconfig->nDMAerrorIVG0) {
				    g_aEPPIconfigSpec[cEPPInr].nDMAerrorIVG0 = pa_tEPPIconfig->nDMAerrorIVG0;
				    adi_int_SICSetIVG(g_aEPPIspec[cEPPInr].DMAerrorIntId0, g_aEPPIconfigSpec[cEPPInr].nDMAerrorIVG0);
				} else {
				    adi_int_SICGetIVG(g_aEPPIspec[cEPPInr].DMAerrorIntId0, &g_aEPPIconfigSpec[cEPPInr].nDMAerrorIVG0);
				}
				
				// hook the dma error interrupt
				if(adi_int_CECHook(g_aEPPIconfigSpec[cEPPInr].nDMAerrorIVG0, EPPIerrorDmaHandler, (void *)&g_aEPPIconfigSpec[cEPPInr], false) == ADI_INT_RESULT_SUCCESS) {
					adi_int_SICWakeup(g_aEPPIspec[cEPPInr].DMAerrorIntId0, TRUE);
					adi_int_SICEnable(g_aEPPIspec[cEPPInr].DMAerrorIntId0);
				} else {
					erResult = ERR_HOOK_INTERRUPT;
				}
#endif
				// check if second DMA channel is used
				if (pa_tEPPIconfig->nEPPIcontrol & 0x02000000) {
					
    				// setup interrupts for channel 1
    				if (pa_tEPPIconfig->fnCallback1) {
					
    					g_aEPPIconfigSpec[cEPPInr].fnCallback1 = pa_tEPPIconfig->fnCallback1;
    				}
					
					if(pa_tEPPIconfig->nIVG1) {
					    g_aEPPIconfigSpec[cEPPInr].nIVG1 = pa_tEPPIconfig->nIVG1;
					    adi_int_SICSetIVG(g_aEPPIspec[cEPPInr].peripheralIntId1, g_aEPPIconfigSpec[cEPPInr].nIVG1);
					} else {
					    adi_int_SICGetIVG(g_aEPPIspec[cEPPInr].peripheralIntId1, &g_aEPPIconfigSpec[cEPPInr].nIVG1);
					}
				
					// hook the interrupt
					if(adi_int_CECHook(g_aEPPIconfigSpec[cEPPInr].nIVG1, EPPIintHandler, (void *)&g_aEPPIconfigSpec[cEPPInr], false) == ADI_INT_RESULT_SUCCESS) {
						adi_int_SICWakeup(g_aEPPIspec[cEPPInr].peripheralIntId1, TRUE);
						adi_int_SICEnable(g_aEPPIspec[cEPPInr].peripheralIntId1);
					} else {
						erResult = ERR_HOOK_INTERRUPT;
					}
#ifdef _EPPI_CONFIG_HOOK_ERROR_INT_
    				if(pa_tEPPIconfig->fnErrPerCallback1) {
    					g_aEPPIconfigSpec[cEPPInr].fnErrPerCallback1 = pa_tEPPIconfig->fnErrPerCallback1;
    				}
					
					if(pa_tEPPIconfig->nPerErrorIVG1) {
					    g_aEPPIconfigSpec[cEPPInr].nPerErrorIVG1 = pa_tEPPIconfig->nPerErrorIVG1;
					    adi_int_SICSetIVG(g_aEPPIspec[cEPPInr].perErrorIntId1, g_aEPPIconfigSpec[cEPPInr].nPerErrorIVG1);
					} else {
					    adi_int_SICGetIVG(g_aEPPIspec[cEPPInr].perErrorIntId1, &g_aEPPIconfigSpec[cEPPInr].nPerErrorIVG1);
					}
				
					// hook the error interrupt
					if(adi_int_CECHook(g_aEPPIconfigSpec[cEPPInr].nPerErrorIVG1, EPPIerrorPerHandler, (void *)&g_aEPPIconfigSpec[cEPPInr], false) == ADI_INT_RESULT_SUCCESS) {
						adi_int_SICWakeup(g_aEPPIspec[cEPPInr].perErrorIntId1, TRUE);
						adi_int_SICEnable(g_aEPPIspec[cEPPInr].perErrorIntId1);
					} else {
						erResult = ERR_HOOK_INTERRUPT;
					}
				
    				if(pa_tEPPIconfig->fnErrDmaCallback1) {
    					g_aEPPIconfigSpec[cEPPInr].fnErrDmaCallback1 = pa_tEPPIconfig->fnErrDmaCallback1;
    				}
					
					if(pa_tEPPIconfig->nDMAerrorIVG1) {
					    g_aEPPIconfigSpec[cEPPInr].nDMAerrorIVG1 = pa_tEPPIconfig->nDMAerrorIVG1;
					    adi_int_SICSetIVG(g_aEPPIspec[cEPPInr].DMAerrorIntId1, g_aEPPIconfigSpec[cEPPInr].nDMAerrorIVG1);
					} else {
					    adi_int_SICGetIVG(g_aEPPIspec[cEPPInr].DMAerrorIntId1, &g_aEPPIconfigSpec[cEPPInr].nDMAerrorIVG1);
					}
				
					// hook the error interrupt
					if(adi_int_CECHook(g_aEPPIconfigSpec[cEPPInr].nDMAerrorIVG1, EPPIerrorDmaHandler, (void *)&g_aEPPIconfigSpec[cEPPInr], false) == ADI_INT_RESULT_SUCCESS) {
						adi_int_SICWakeup(g_aEPPIspec[cEPPInr].DMAerrorIntId1, TRUE);
						adi_int_SICEnable(g_aEPPIspec[cEPPInr].DMAerrorIntId1);
					} else {
						erResult = ERR_HOOK_INTERRUPT;
					}
#endif
				}
				
				// set first dma registers
				*(g_aEPPIspec[cEPPInr].pDMA0startaddr)      = (void *)pa_tEPPIconfig->pDMA0startaddr;
				*(g_aEPPIspec[cEPPInr].pDMA0xcount)         = pa_tEPPIconfig->nDMA0xcount;
				*(g_aEPPIspec[cEPPInr].pDMA0ycount)         = pa_tEPPIconfig->nDMA0ycount;
				*(g_aEPPIspec[cEPPInr].pDMA0xmodify)        = pa_tEPPIconfig->nDMA0xmodify;
				*(g_aEPPIspec[cEPPInr].pDMA0ymodify)        = pa_tEPPIconfig->nDMA0ymodify;
				*(g_aEPPIspec[cEPPInr].pDMA0nextDescrPtr)   = (void *)pa_tEPPIconfig->pDMA0nextDescrPtr;
				*(g_aEPPIspec[cEPPInr].pDMA0config)         = pa_tEPPIconfig->nDMA0config;
				
				// enable DMA
				//*(g_aEPPIspec[cEPPInr].pDMA0config) 	|= 0x0001;
				
				// set second dma registers if according DMACFG bit(25) is set in CONTROL register
				if((pa_tEPPIconfig->nEPPIcontrol & 0x02000000)) {
    			    *(g_aEPPIspec[cEPPInr].pDMA1startaddr)      = (void *)pa_tEPPIconfig->pDMA1startaddr;
    				*(g_aEPPIspec[cEPPInr].pDMA1xcount)         = pa_tEPPIconfig->nDMA1xcount;
    				*(g_aEPPIspec[cEPPInr].pDMA1ycount)         = pa_tEPPIconfig->nDMA1ycount;
    				*(g_aEPPIspec[cEPPInr].pDMA1xmodify)        = pa_tEPPIconfig->npDMA1xmodify;
    				*(g_aEPPIspec[cEPPInr].pDMA1ymodify)        = pa_tEPPIconfig->nDMA1ymodify;
    				*(g_aEPPIspec[cEPPInr].pDMA1nextDescrPtr)   = (void *)pa_tEPPIconfig->pDMA1nextDescrPtr;
    				*(g_aEPPIspec[cEPPInr].pDMA1config)         = pa_tEPPIconfig->nDMA1config;
				}
				
				// set eppi registers
				// write LINE and FRAME registers before HCOUNT and VCOUNT, cause LINE and FRAME also writes HCOUNT and VCOUNT
				*(g_aEPPIspec[cEPPInr].pEPPIframe)      = pa_tEPPIconfig->nEPPIframe;
				*(g_aEPPIspec[cEPPInr].pEPPIline)       = pa_tEPPIconfig->nEPPIline;
				*(g_aEPPIspec[cEPPInr].pEPPIhdelay)     = pa_tEPPIconfig->nEPPIhdelay;
				*(g_aEPPIspec[cEPPInr].pEPPIhcount)     = pa_tEPPIconfig->nEPPIhcount;
				*(g_aEPPIspec[cEPPInr].pEPPIvdelay)     = pa_tEPPIconfig->nEPPIvdelay;
				*(g_aEPPIspec[cEPPInr].pEPPIvcount)     = pa_tEPPIconfig->nEPPIvcount;
				*(g_aEPPIspec[cEPPInr].pEPPIclkdiv)     = pa_tEPPIconfig->nEPPIclkdiv;
				*(g_aEPPIspec[cEPPInr].pEPPIcontrol)    = pa_tEPPIconfig->nEPPIcontrol;
				*(g_aEPPIspec[cEPPInr].pEPPIfs1w_hbl)   = pa_tEPPIconfig->nEPPIfs1w_hbl;
				*(g_aEPPIspec[cEPPInr].pEPPIfs1p_avpl)  = pa_tEPPIconfig->nEPPIfs1p_avpl;
				*(g_aEPPIspec[cEPPInr].pEPPIfs2w_lvb)   = pa_tEPPIconfig->nEPPIfs2w_lvb;
				*(g_aEPPIspec[cEPPInr].pEPPIfs2p_lavf)  = pa_tEPPIconfig->nEPPIfs2p_lavf;
				*(g_aEPPIspec[cEPPInr].pEPPIclip)       = pa_tEPPIconfig->nEPPIclip;
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
 *  @brief      set's up the EPPI module and also all configures output multiplexers, all registers are written according
 *	            to the pa_tPPIconig parameter. Multiple buffers are used in a buffer chain
 *
 *	@public
 *  @param pa_tPPIconig			structure with configuration information of the EPPI
 *  @param pa_pBuffers          array with pointers to the buffers to be used
 *  @param pa_cBufferCount      number of buffers to be used
 *	@return on success          #ERR_NONE, appropriate errorcode otherwise
 *	 
 *
 **/
T_ERROR_CODE eppi_open_buffer (T_EPPI_CONF* pa_tEPPIconfig, 
                        void** pa_pBuffers, 
                        unsigned char pa_cBufferCount) {
								
	T_ERROR_CODE erResult = ERR_NONE;
	unsigned char cEPPInr = pa_tEPPIconfig->cEPPInr; // to avoid typing
	unsigned short i;
	
	if(pa_cBufferCount <= MAX_BUFFERS) {
    	if (cEPPInr < g_nEPPIcount) {
		
    		void *pExitCriticalArg = adi_int_EnterCriticalRegion(NULL);

    		if (g_aEPPIspec[cEPPInr].bInUse) {		// first check if the ppi is already in use
    			erResult = ERR_ALREADY_INITIALIZED;
    		}
		
    		adi_int_ExitCriticalRegion(pExitCriticalArg);
		
    		if (erResult == ERR_NONE) {
    			// no error so far - continue
			    g_aEPPIconfigSpec[cEPPInr].cEPPI = cEPPInr;
					
    			if ( eppi_platformInit(pa_tEPPIconfig->cEPPInr, pa_tEPPIconfig->nEPPIcontrol) ) {	
    			    // setup descriptors
    			    for(i=0; i<pa_cBufferCount-1; i++) {
    			        // setup the descriptor table for double buffering
            			g_anGpAbDescriptor[2*i] = (unsigned long)&g_anGpAbDescriptor[2*i+2];	// Next Descriptor (Second half of this array)
            		    g_anGpAbDescriptor[2*i+1] = (unsigned long)*(pa_pBuffers+i);		    // Start address		        
    			    }
			        g_anGpAbDescriptor[2*i] = (unsigned long)&g_anGpAbDescriptor[0];	    // Next Descriptor (First half of this array)
        			g_anGpAbDescriptor[2*i+1] = (unsigned long)*(pa_pBuffers+i);;		        // Start address of last buffer
    			    
    			    
    				// setup interrupts for channel 0
    				if (pa_tEPPIconfig->fnCallback0) {
					
    					g_aEPPIconfigSpec[cEPPInr].fnCallback0 = pa_tEPPIconfig->fnCallback0;
					
    					if(pa_tEPPIconfig->nIVG0) {
    					    g_aEPPIconfigSpec[cEPPInr].nIVG0 = pa_tEPPIconfig->nIVG0;
    					    adi_int_SICSetIVG(g_aEPPIspec[cEPPInr].peripheralIntId0, g_aEPPIconfigSpec[cEPPInr].nIVG0);
    					} else {
    					    adi_int_SICGetIVG(g_aEPPIspec[cEPPInr].peripheralIntId0, &g_aEPPIconfigSpec[cEPPInr].nIVG0);
    					}
					
    					// hook the interrupt
    					if(adi_int_CECHook(g_aEPPIconfigSpec[cEPPInr].nIVG0, EPPIintHandler, (void *)&g_aEPPIconfigSpec[cEPPInr], false) == ADI_INT_RESULT_SUCCESS) {
    						adi_int_SICWakeup(g_aEPPIspec[cEPPInr].peripheralIntId0, TRUE);
    						adi_int_SICEnable(g_aEPPIspec[cEPPInr].peripheralIntId0);
    					} else {
    						erResult = ERR_HOOK_INTERRUPT;
    					}
    				}
				
    				if(pa_tEPPIconfig->fnErrPerCallback0) {
    					g_aEPPIconfigSpec[cEPPInr].fnErrPerCallback0 = pa_tEPPIconfig->fnErrPerCallback0;
					
    					if(pa_tEPPIconfig->nPerErrorIVG0) {
    					    g_aEPPIconfigSpec[cEPPInr].nPerErrorIVG0 = pa_tEPPIconfig->nPerErrorIVG0;
    					    adi_int_SICSetIVG(g_aEPPIspec[cEPPInr].perErrorIntId0, g_aEPPIconfigSpec[cEPPInr].nPerErrorIVG0);
    					} else {
    					    adi_int_SICGetIVG(g_aEPPIspec[cEPPInr].perErrorIntId0, &g_aEPPIconfigSpec[cEPPInr].nPerErrorIVG0);
    					}
					
    					// hook the error interrupt
    					if(adi_int_CECHook(g_aEPPIconfigSpec[cEPPInr].nPerErrorIVG0, EPPIerrorPerHandler, (void *)&g_aEPPIconfigSpec[cEPPInr], false) == ADI_INT_RESULT_SUCCESS) {
    						adi_int_SICWakeup(g_aEPPIspec[cEPPInr].perErrorIntId0, TRUE);
    						adi_int_SICEnable(g_aEPPIspec[cEPPInr].perErrorIntId0);
    					} else {
    						erResult = ERR_HOOK_INTERRUPT;
    					}
    				}
				
    				if(pa_tEPPIconfig->fnErrDmaCallback0) {
    					g_aEPPIconfigSpec[cEPPInr].fnErrDmaCallback0 = pa_tEPPIconfig->fnErrDmaCallback0;
					
    					if(pa_tEPPIconfig->nDMAerrorIVG0) {
    					    g_aEPPIconfigSpec[cEPPInr].nDMAerrorIVG0 = pa_tEPPIconfig->nDMAerrorIVG0;
    					    adi_int_SICSetIVG(g_aEPPIspec[cEPPInr].DMAerrorIntId0, g_aEPPIconfigSpec[cEPPInr].nDMAerrorIVG0);
    					} else {
    					    adi_int_SICGetIVG(g_aEPPIspec[cEPPInr].DMAerrorIntId0, &g_aEPPIconfigSpec[cEPPInr].nDMAerrorIVG0);
    					}
					
    					// hook the error interrupt
    					if(adi_int_CECHook(g_aEPPIconfigSpec[cEPPInr].nDMAerrorIVG0, EPPIerrorDmaHandler, (void *)&g_aEPPIconfigSpec[cEPPInr], false) == ADI_INT_RESULT_SUCCESS) {
    						adi_int_SICWakeup(g_aEPPIspec[cEPPInr].DMAerrorIntId0, TRUE);
    						adi_int_SICEnable(g_aEPPIspec[cEPPInr].DMAerrorIntId0);
    					} else {
    						erResult = ERR_HOOK_INTERRUPT;
    					}
    				}
				
    				// set first dma registers
    				*(g_aEPPIspec[cEPPInr].pDMA0startaddr)      = (void *)*pa_pBuffers;
    				*(g_aEPPIspec[cEPPInr].pDMA0xcount)         = pa_tEPPIconfig->nDMA0xcount;
    				*(g_aEPPIspec[cEPPInr].pDMA0ycount)         = pa_tEPPIconfig->nDMA0ycount;
    				*(g_aEPPIspec[cEPPInr].pDMA0xmodify)        = pa_tEPPIconfig->nDMA0xmodify;
    				*(g_aEPPIspec[cEPPInr].pDMA0ymodify)        = pa_tEPPIconfig->nDMA0ymodify;
    				*(g_aEPPIspec[cEPPInr].pDMA0nextDescrPtr)   = (void *)&g_anGpAbDescriptor[0];;
    				*(g_aEPPIspec[cEPPInr].pDMA0config)         = pa_tEPPIconfig->nDMA0config | 0x7400;	// descriptor large model, descriptor size 4;
				
    				// set eppi registers
    				// write LINE and FRAME registers before HCOUNT and VCOUNT, cause LINE and FRAME also writes HCOUNT and VCOUNT
    				*(g_aEPPIspec[cEPPInr].pEPPIframe)      = pa_tEPPIconfig->nEPPIframe;
    				*(g_aEPPIspec[cEPPInr].pEPPIline)       = pa_tEPPIconfig->nEPPIline;
    				*(g_aEPPIspec[cEPPInr].pEPPIhdelay)     = pa_tEPPIconfig->nEPPIhdelay;
    				*(g_aEPPIspec[cEPPInr].pEPPIhcount)     = pa_tEPPIconfig->nEPPIhcount;
    				*(g_aEPPIspec[cEPPInr].pEPPIvdelay)     = pa_tEPPIconfig->nEPPIvdelay;
    				*(g_aEPPIspec[cEPPInr].pEPPIvcount)     = pa_tEPPIconfig->nEPPIvcount;
    				*(g_aEPPIspec[cEPPInr].pEPPIclkdiv)     = pa_tEPPIconfig->nEPPIclkdiv;
    				*(g_aEPPIspec[cEPPInr].pEPPIcontrol)    = pa_tEPPIconfig->nEPPIcontrol;
    				*(g_aEPPIspec[cEPPInr].pEPPIfs1w_hbl)   = pa_tEPPIconfig->nEPPIfs1w_hbl;
    				*(g_aEPPIspec[cEPPInr].pEPPIfs1p_avpl)  = pa_tEPPIconfig->nEPPIfs1p_avpl;
    				*(g_aEPPIspec[cEPPInr].pEPPIfs2w_lvb)   = pa_tEPPIconfig->nEPPIfs2w_lvb;
    				*(g_aEPPIspec[cEPPInr].pEPPIfs2p_lavf)  = pa_tEPPIconfig->nEPPIfs2p_lavf;
    				*(g_aEPPIspec[cEPPInr].pEPPIclip)       = pa_tEPPIconfig->nEPPIclip;
    				
    				g_nCurrentBuffer = 0;
    				g_nBuffersUsed   = pa_cBufferCount-1;
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
	} else {
	    erResult = ERR_TO_MUCH_BUFFERS;
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
T_ERROR_CODE eppi_close(unsigned int pa_nEPPIindex) {
	
	T_ERROR_CODE erResult = ERR_NONE;
	
	if (pa_nEPPIindex < g_nEPPIcount) {		// PPI identifier is in a valid range
		
		if (g_aEPPIconfigSpec[pa_nEPPIindex].fnCallback0) {	// If a Callback function was hooked
			
			adi_int_SICDisable(g_aEPPIspec[pa_nEPPIindex].peripheralIntId0);		// Disable the interrupt
			
			// Unhook our Interrupt Handler
			if(adi_int_CECUnhook(g_aEPPIconfigSpec[pa_nEPPIindex].nIVG0, EPPIintHandler, (void *)&g_aEPPIconfigSpec[pa_nEPPIindex]) != ADI_INT_RESULT_SUCCESS) {													
				erResult = ERR_HOOK_INTERRUPT;
			}
		}
		if (g_aEPPIconfigSpec[pa_nEPPIindex].fnErrPerCallback0) {	// If a Callback function was hooked
			
			adi_int_SICDisable(g_aEPPIspec[pa_nEPPIindex].perErrorIntId0);		// Disable the interrupt
			
			// Unhook our Interrupt Handler
			if(adi_int_CECUnhook(g_aEPPIconfigSpec[pa_nEPPIindex].nPerErrorIVG0, EPPIintHandler, (void *)&g_aEPPIconfigSpec[pa_nEPPIindex]) != ADI_INT_RESULT_SUCCESS) {													
				erResult = ERR_HOOK_INTERRUPT;
			}
		}
		if (g_aEPPIconfigSpec[pa_nEPPIindex].fnErrDmaCallback0) {	// If a Callback function was hooked
			
			adi_int_SICDisable(g_aEPPIspec[pa_nEPPIindex].DMAerrorIntId0);		// Disable the interrupt
			
			// Unhook our Interrupt Handler
			if(adi_int_CECUnhook(g_aEPPIconfigSpec[pa_nEPPIindex].nDMAerrorIVG0, EPPIintHandler, (void *)&g_aEPPIconfigSpec[pa_nEPPIindex]) != ADI_INT_RESULT_SUCCESS) {													
				erResult = ERR_HOOK_INTERRUPT;
			}
		}
		
		if(*(g_aEPPIspec[pa_nEPPIindex].pDMA0config) & 0x02000000) {
		    
    		if (g_aEPPIconfigSpec[pa_nEPPIindex].fnCallback1) {	// If a Callback function was hooked
			
    			adi_int_SICDisable(g_aEPPIspec[pa_nEPPIindex].peripheralIntId1);		// Disable the interrupt
			
    			// Unhook our Interrupt Handler
    			if(adi_int_CECUnhook(g_aEPPIconfigSpec[pa_nEPPIindex].nIVG1, EPPIintHandler, (void *)&g_aEPPIconfigSpec[pa_nEPPIindex]) != ADI_INT_RESULT_SUCCESS) {													
    				erResult = ERR_HOOK_INTERRUPT;
    			}
    		}
    		if (g_aEPPIconfigSpec[pa_nEPPIindex].fnErrPerCallback1) {	// If a Callback function was hooked
			
    			adi_int_SICDisable(g_aEPPIspec[pa_nEPPIindex].perErrorIntId1);		// Disable the interrupt
			
    			// Unhook our Interrupt Handler
    			if(adi_int_CECUnhook(g_aEPPIconfigSpec[pa_nEPPIindex].nPerErrorIVG1, EPPIintHandler, (void *)&g_aEPPIconfigSpec[pa_nEPPIindex]) != ADI_INT_RESULT_SUCCESS) {													
    				erResult = ERR_HOOK_INTERRUPT;
    			}
    		}
    		if (g_aEPPIconfigSpec[pa_nEPPIindex].fnErrDmaCallback1) {	// If a Callback function was hooked
			
    			adi_int_SICDisable(g_aEPPIspec[pa_nEPPIindex].DMAerrorIntId1);		// Disable the interrupt
			
    			// Unhook our Interrupt Handler
    			if(adi_int_CECUnhook(g_aEPPIconfigSpec[pa_nEPPIindex].nDMAerrorIVG1, EPPIintHandler, (void *)&g_aEPPIconfigSpec[pa_nEPPIindex]) != ADI_INT_RESULT_SUCCESS) {													
    				erResult = ERR_HOOK_INTERRUPT;
    			}
    		}
		}
		g_aEPPIconfigSpec[pa_nEPPIindex].fnCallback0 = 0;
		g_aEPPIconfigSpec[pa_nEPPIindex].fnErrPerCallback0 = 0;
		g_aEPPIconfigSpec[pa_nEPPIindex].fnErrDmaCallback0 = 0;
		g_aEPPIconfigSpec[pa_nEPPIindex].fnCallback1 = 0;
		g_aEPPIconfigSpec[pa_nEPPIindex].fnErrPerCallback1 = 0;
		g_aEPPIconfigSpec[pa_nEPPIindex].fnErrDmaCallback1 = 0;
		g_aEPPIconfigSpec[pa_nEPPIindex].cEPPI = 0;
		g_aEPPIconfigSpec[pa_nEPPIindex].nIVG0 = 0;
		g_aEPPIconfigSpec[pa_nEPPIindex].nPerErrorIVG0 = 0;
		g_aEPPIconfigSpec[pa_nEPPIindex].nDMAerrorIVG0 = 0;
		g_aEPPIconfigSpec[pa_nEPPIindex].nIVG1 = 0;
		g_aEPPIconfigSpec[pa_nEPPIindex].nDMAerrorIVG1 = 0;
		g_aEPPIconfigSpec[pa_nEPPIindex].nPerErrorIVG1 = 0;
		
		g_aEPPIspec[pa_nEPPIindex].bInUse	= false;// Flag ppi as free
	} else {
		erResult = ERR_PPI_INDEX;
	}
	
	return erResult;
}



/**
 *	@public
 *	@brief		Enables PPI (Starts the DMA controller, and then the PPI running)
 *	
 *	@param		pa_nEPPIindex		Index (identifier) of the PPI
 *
 *
 **/
void eppi_enable (unsigned int pa_nEPPIindex) {
    if(*(g_aEPPIspec[pa_nEPPIindex].pEPPIcontrol) & 0x02000000) {
	    *(g_aEPPIspec[pa_nEPPIindex].pDMA1config) 	|= 0x0001;
    } 
    
    *(g_aEPPIspec[pa_nEPPIindex].pDMA0config) 	|= 0x0001;
	*(g_aEPPIspec[pa_nEPPIindex].pEPPIcontrol) |= 0x00000001;
}



/**
 *	@public
 *	@brief		Disables PPI (Stops the PPI, and then the DMA controller running)
 *	
 *	@param		pa_nPPIindex		Index (identifier) of the PPI
 *
 *
 **/
void eppi_disable (unsigned int pa_nEPPIindex) {
    
    *(g_aEPPIspec[pa_nEPPIindex].pEPPIcontrol) &= ~0x00000001;
    if(*(g_aEPPIspec[pa_nEPPIindex].pEPPIcontrol) & 0x02000000) {
        *(g_aEPPIspec[pa_nEPPIindex].pDMA1config) 	&= ~0x0001; 
    }
	
	*(g_aEPPIspec[pa_nEPPIindex].pDMA0config) 	&= ~0x0001; 

}



/**
 *	@public
 *	@brief		Set's up the PPI with an alternating buffer.
 *	
 *	@param 		pa_nEPPIindex				index of the PPI (0 for PPI1 1 for PPI2)
 *  @param      pa_cEPPIdmaNr               number of the DAM (0, 1)
 *	@param 		pa_nStartAddr				Startaddress of dma transfer
 *
 *	
 **/
void eppi_setStartAddr(unsigned char pa_cEPPIindex, unsigned char pa_cDMAnr, void *pa_pStartAddr) {
    if(pa_cDMAnr==0) {
        *(g_aEPPIspec[pa_cEPPIindex].pDMA0startaddr) = pa_pStartAddr;
    } else {
        *(g_aEPPIspec[pa_cEPPIindex].pDMA1startaddr) = pa_pStartAddr;
    }
}


/**
 *	@public
 *	@brief		Get's the PPI's X count (width)
 *	
 *  @param      pa_cEPPIdmaNr               number of the DAM (0, 1)
 *	@param 		pa_nEPPIindex				index of the PPI (0 for PPI1 1 for PPI2)
 *
 *	@return 	Horizontal width of the X dimension, stored in the X count register.
 *	
 **/
unsigned short eppi_dma0_getXcount (unsigned char pa_cEPPIindex, unsigned char pa_cDMAnr) {
    if(pa_cDMAnr==0) {
	    return (*(g_aEPPIspec[pa_cEPPIindex].pDMA0xcount));
    } else {
        return (*(g_aEPPIspec[pa_cEPPIindex].pDMA1xcount));
    }
}



/**
 *	@public
 *	@brief		Get's the PPI's Y count (height)
 *	
 *  @param      pa_cEPPIdmaNr               number of the DAM (0, 1)
 *	@param 		pa_nEPPIindex				index of the PPI (0 for PPI1 1 for PPI2)
 *
 *	@return 	Vertical height of the Y dimension, stored in the Y count register.
 *	
 **/
unsigned short eppi_dma0_getYcount (unsigned char pa_cEPPIindex, unsigned char pa_cDMAnr) {
    if(pa_cDMAnr==0) {
	    return (*(g_aEPPIspec[pa_cEPPIindex].pDMA0ycount));
    } else {
        return (*(g_aEPPIspec[pa_cEPPIindex].pDMA1ycount));
    }
}

/**
 *	@public
 *	@brief		Get's the PPI's Current X count (width)
 *	
 *  @param      pa_cEPPIdmaNr               number of the DAM (0, 1)
 *	@param 		pa_nEPPIindex				index of the PPI (0 for PPI1 1 for PPI2)
 *
 *	@return 	Get's the position of the current DMA transfer on the horizontal X axis.
 *	
 **/
unsigned short eppi_dma_getCurrXcount (unsigned char pa_cEPPIindex, unsigned char pa_cDMAnr) {
    if(pa_cDMAnr==0) {
	    return (*(g_aEPPIspec[pa_cEPPIindex].pDMA0currXcount));
    } else {
        return (*(g_aEPPIspec[pa_cEPPIindex].pDMA1currXcount));
    }
}



/**
 *	@public
 *	@brief		Get's the PPI's Current Y count (height)
 *	
 *  @param      pa_cEPPIdmaNr               number of the DAM (0, 1)
 *	@param 		pa_nPPIindex				index of the PPI (0 for PPI1 1 for PPI2)
 *
 *	@return 	Get's the position of the current DMA transfer on the vertical Y axis.
 *	
 **/
unsigned short eppi_dma_getCurrYcount (unsigned char pa_cEPPIindex, unsigned char pa_cDMAnr) {
    if(pa_cDMAnr==0) {
	    return (*(g_aEPPIspec[pa_cEPPIindex].pDMA0currYcount));
    } else {
        return (*(g_aEPPIspec[pa_cEPPIindex].pDMA1currYcount));
    }
}

/**
 *	@public
 *	@brief		Returns the number of the buffer which is currently beeing processed, starting with 0 for the first buffer
 *	
 *	@param		pa_nPPIindex		Index (identifier) of the PPI
 *
 *
 **/
unsigned long eppi_get_current_buffer (unsigned int pa_nEPPIindex) {
    
    if((*(g_aEPPIspec[pa_nEPPIindex].pDMA0config)) & 0x7400) { 
        return g_nCurrentBuffer;
    } else {
        // EPPI is in single buffer mode
        return 0;
    } 
}

