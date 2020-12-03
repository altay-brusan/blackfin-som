/**
 *	@file 		EPPIconfig.h
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
 * 	@brief Enhanced Parallel port interface driver. For abstraction of registers. 
 *	
 */

#ifndef _EPPI_CONFIG_H_INCLUDED_
#define _EPPI_CONFIG_H_INCLUDED_

#include <datatypes.h>
#include <services/services.h>

typedef void (*T_EPPI_CALLBACK)(void *pa_pClientArg);    ///< Callback function for PPI interrupt

#define ERR_PLATFORM_INIT 		    ERR_GENERIC + 10	///< Platform specific initialization error
#define ERR_SET_DMA_MAP				ERR_GENERIC + 20	///< Cannot write dma peripheral map register	
#define ERR_GET_DMA_MAP				ERR_GENERIC + 30	///< Cannot read dma peripheral map register	
#define ERR_DMA_OPEN				ERR_GENERIC + 40	///< Could not access dma channel - only with DMA manager
#define ERR_NOT_OPENED				ERR_GENERIC + 50	///< Accessing a closed (not opened) dma channel - only with DMA manager
#define ERR_DMA_CLOSE				ERR_GENERIC + 60	///< Could not close dma channel - only with DMA manager
#define ERR_PPI_INDEX				ERR_GENERIC + 70	///< Invalid PPI index
#define ERR_HOOK_INTERRUPT			ERR_GENERIC + 80	///< Error hooking the interrupt
#define ERR_ALREADY_INITIALIZED		ERR_GENERIC + 90	///< Interface was already initialized
#define ERR_TO_MUCH_BUFFERS         ERR_GENERIC + 100   ///< To much buffers for PPI DMA
	

#define EPPI_CONFIG_MAX_NOF_EPPI      4
    
/**
 *	@brief 			structure for the call of EPPI configuration functions. It represents the EPPI and according DMA registers and additional 
 *	                configuration values
 *	
 **/  
typedef struct{
    T_EPPI_CALLBACK fnCallback0;
    T_EPPI_CALLBACK fnErrPerCallback0;
    T_EPPI_CALLBACK fnErrDmaCallback0;
    T_EPPI_CALLBACK fnCallback1;
    T_EPPI_CALLBACK fnErrPerCallback1;
    T_EPPI_CALLBACK fnErrDmaCallback1;
    unsigned char cEPPI;
    unsigned long nIVG0;				    ///< Interrupt Event Level, (Interrupt Priority)
    unsigned long nPerErrorIVG0;		    ///< Peripheral error Interrupt Event Level, (Interrupt Priority)
    unsigned long nDMAerrorIVG0;		    ///< DMA error Interrupt Event Level, (Interrupt Priority)
    unsigned long nIVG1;				    ///< Interrupt Event Level, (Interrupt Priority)
    unsigned long nDMAerrorIVG1;		    ///< DMA error Interrupt Event Level, (Interrupt Priority)
    unsigned long nPerErrorIVG1;		    ///< Peripheral error Interrupt Event Level, (Interrupt Priority)
} T_EPPI_CONF_SPEC;
    
/**
 *	@brief 			structure for the call of EPPI configuration functions. It represents the EPPI and according DMA registers and additional 
 *	                configuration values
 *	
 **/
typedef struct {    
    unsigned short      nEPPIhcount;        ///< EPPI horizontal transfer count register
    unsigned short      nEPPIhdelay;        ///< EPPI horizontal delay register
    unsigned short      nEPPIvcount;        ///< EPPI vertical transfer count register
    unsigned short      nEPPIvdelay;        ///< EPPI vertical delay register
    unsigned short      nEPPIframe;         ///< EPPI lines per frame register
    unsigned short      nEPPIline;          ///< EPPI samples per frame register
    unsigned short      nEPPIclkdiv;        ///< EPPI clock divide register
    unsigned long       nEPPIcontrol;       ///< EPPI control register
    unsigned long       nEPPIfs1w_hbl;      ///< EPPI FS1 width register/EPPI horziontal blanking samples per line register
    unsigned long       nEPPIfs1p_avpl;     ///< EPPI FS1 period register/ EPPI active video samples per line registers
    unsigned long       nEPPIfs2w_lvb;      ///< EPPI FS2 width register/ EPPI lines of vertical blanking registers
    unsigned long       nEPPIfs2p_lavf;     ///< EPPI FS2 period register/ EPPI lines of active video per field registers
    unsigned long       nEPPIclip;          ///< EPPI clipping registers
    
    unsigned short 	    nDMA0config;		///< DMA0 channel's configuration register.
    void 				*pDMA0startaddr;	///< Pointer to the DMA0 channel's start memory location.
    signed short 		nDMA0xmodify;		///< DMA0 X modify (horizontal stride) register.
    unsigned short 	    nDMA0xcount;		///< DMA0 X count register. (horizontal width)
    signed short 		nDMA0ymodify;		///< DMA0 Y modify (vertical stride) register.
    unsigned short 	    nDMA0ycount;		///< DMA0 Y count register (vertical height).
    void 				*pDMA0nextDescrPtr; ///< Pointer to the DMA0 next descriptor memory location.
    T_EPPI_CALLBACK 	fnCallback0;		///< Function pointer to a Callback function of DMA0.
    T_EPPI_CALLBACK 	fnErrPerCallback0;	///< Function pointer to a Callback function, when an peripheral error occurs.
    T_EPPI_CALLBACK     fnErrDmaCallback0;  ///< Function pointer to a Callback function, when an error occurs on DMA0.
    
    unsigned short 	    nDMA1config;		///< Pointer to the DMA1 channel's configuration register.
    void 				*pDMA1startaddr;	///< Pointer to the DMA1 channel's start memory location.
    signed short 		npDMA1xmodify;		///< Pointer to the DMA1 X modify (horizontal stride) register.
    unsigned short 	    nDMA1xcount;		///< Pointer to the DMA1 X count register. (horizontal width)
    signed short 		nDMA1ymodify;		///< Pointer to the DMA1 Y modify (vertical stride) register.
    unsigned short 	    nDMA1ycount;		///< Pointer to the DMA1 Y count register (vertical height).
    void 				*pDMA1nextDescrPtr; ///< Pointer to the DMA1 next descriptor memory location.
    T_EPPI_CALLBACK 	fnCallback1;		///< Function pointer to a Callback function of DMA1.
    T_EPPI_CALLBACK 	fnErrPerCallback1;	///< Function pointer to a Callback function, when an peripheral error occurs.
    T_EPPI_CALLBACK     fnErrDmaCallback1;  ///< Function pointer to a Callback function, when an error occurs on DMA1.
    
    unsigned char 		cEPPInr;			///< PPI Number (PPI instance identifier 0,1,2)
    
    unsigned long       nIVG0;  			///< Interrupt Event Level, (Interrupt Priority)
    unsigned long       nDMAerrorIVG0;	    ///< DMA error Interrupt Event Level, (Interrupt Priority)
    unsigned long       nPerErrorIVG0; 		///< Peripheral error Interrupt Event Level, (Interrupt Priority)
    unsigned long       nIVG1;  			///< Interrupt Event Level, (Interrupt Priority)
    unsigned long       nDMAerrorIVG1;	    ///< DMA error Interrupt Event Level, (Interrupt Priority)
    unsigned long       nPerErrorIVG1;    	///< Peripheral error Interrupt Event Level, (Interrupt Priority)
} T_EPPI_CONF;
	

// prototype declarations


/**
 *  @brief      Initial setup of eppi config to ensure defined starting conditions
 **/
void eppi_setup(void);

/**
 *  @brief      Cleanup if eppi config is no longer used
 **/
 void eppi_cleanup(void);
 
/**
	@param pa_tPPIconig			structure with configuration information of the EPPI
	@return on success          #ERR_NONE, appropriate errorcode otherwise
	
	sets up the EPPI module and also all configures output multiplexers, all registers are written according
	to the pa_tPPIconig parameter. 
*/
T_ERROR_CODE eppi_open (T_EPPI_CONF* pa_tEPPIconfig);
		
/**
	@param pa_nPPIindex
	
	starts the dma transfer for the specified PPI
	
*/
void eppi_enable (unsigned int pa_nEPPIindex);

/**
	@param pa_nPPIindex
	
	stops the dma transfer for the specified PPI
*/
void eppi_disable (unsigned int pa_nEPPIindex);

/**
	@param pa_nPPIindex
	@return  on success #ERR_NONE, appropriate errorcode otherwise
	
	releases resources allocated by the setup functions. closes the PPI transfer.
*/
T_ERROR_CODE eppi_close(unsigned int pa_nEPPIindex);

/**
	@param pa_nPPIindex
	@return  content of the X_COUNT register
	
*/
unsigned short eppi_dma_getXcount (unsigned char pa_cEPPIindex, unsigned char pa_cDMAnr);

/**
	@param pa_nPPIindex
	@return  content of the Y_COUNT register
	
*/
unsigned short eppi_dma_getYcount (unsigned char pa_cEPPIindex, unsigned char pa_cDMAnr);

/**
	@param pa_nPPIindex
	@return  content of the CURR_X_COUNT register
	
*/
unsigned short eppi_dma_getCurrXcount (unsigned char pa_cEPPIindex, unsigned char pa_cDMAnr);

/**
	@param pa_nPPIindex
	@return  content of the CURR_Y_COUNT register
	
*/
unsigned short eppi_dma_getCurrYcount (unsigned char pa_cEPPIindex, unsigned char pa_cDMAnr);

void eppi_setStartAddr(unsigned char pa_cEPPIindex, unsigned char pa_cDMAnr, void *pa_pStartAddr);

unsigned long eppi_get_current_buffer (unsigned int pa_nEPPIindex);

#endif

