/**
 *	@file 		EPPIconfig.h
 *	@ingroup 	EPPI
 *	
 *	@brief 		Enhanced Parrallel Port Interface
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
 * 	@brief Enhanced Parallel port interface.
 *	
 */

#ifndef _EPPI_H_INCLUDED_
#define _EPPI_H_INCLUDED_
#include <services/services.h>

/**
 *	@brief 			structure to abstract the registers of the EPPI unit. Structure is filled up for according processor. Also 2 DMAs are 
 *                  present even if they are not available in all modes. 
 *	
 *	
 **/
typedef struct {
    bool 						bInUse;				///< Flags the PPI as in use.
    
    ADI_DMA_CHANNEL_ID 			enDMAchannel0;		///< Interrupt channel that the DMA Channel of the ppi uses.
    ADI_INT_PERIPHERAL_ID 		peripheralIntId0;   ///< Peripheral Interrupt ID, (Interrupt channel the peripheral is mapped to)
    ADI_INT_PERIPHERAL_ID 		perErrorIntId0;		///< Error Interrupt ID, (Interrupt channel the DMA Error interrupt is mapped to)
    ADI_INT_PERIPHERAL_ID 		DMAerrorIntId0;	///< Error Interrupt ID, (Interrupt channel the peripheral Error interrupt is mapped to)
    
    ADI_DMA_CHANNEL_ID 			enDMAchannel1;		///< Interrupt channel that the DMA Channel of the ppi uses.
    ADI_INT_PERIPHERAL_ID 		peripheralIntId1;   ///< Peripheral Interrupt ID, (Interrupt channel the peripheral is mapped to)
    ADI_INT_PERIPHERAL_ID 		perErrorIntId1;		///< Error Interrupt ID, (Interrupt channel the DMA Error interrupt is mapped to)
    ADI_INT_PERIPHERAL_ID 		DMAerrorIntId1;	///< Error Interrupt ID, (Interrupt channel the peripheral Error interrupt is mapped to)
    
    volatile unsigned short     *pEPPIstatus;       ///< Pointer to the EPPI status register
    volatile unsigned short     *pEPPIhcount;       ///< Pointer to the EPPI horizontal transfer count register
    volatile unsigned short     *pEPPIhdelay;       ///< Pointer to the EPPI horizontal delay register
    volatile unsigned short     *pEPPIvcount;       ///< Pointer to the EPPI vertical transfer count register
    volatile unsigned short     *pEPPIvdelay;       ///< Pointer to the EPPI vertical delay register
    volatile unsigned short     *pEPPIframe;        ///< Pointer to the EPPI lines per frame register
    volatile unsigned short     *pEPPIline;         ///< Pointer to the EPPI samples per frame register
    volatile unsigned short     *pEPPIclkdiv;       ///< Pointer to the EPPI clock divide register
    volatile unsigned long      *pEPPIcontrol;      ///< Pointer to the EPPI control register
    volatile unsigned long      *pEPPIfs1w_hbl;     ///< Pointer to the EPPI FS1 width register/EPPI horziontal blanking samples per line register
    volatile unsigned long      *pEPPIfs1p_avpl;    ///< Pointer to the EPPI FS1 period register/ EPPI active video samples per line registers
    volatile unsigned long      *pEPPIfs2w_lvb;     ///< Pointer to the EPPI FS2 width register/ EPPI lines of vertical blanking registers
    volatile unsigned long      *pEPPIfs2p_lavf;    ///< Pointer to the EPPI FS2 period register/ EPPI lines of active video per field registers
    volatile unsigned long      *pEPPIclip;         ///< Pointer to the EPPI clipping registers
    
    volatile unsigned short 	*pDMA0irqStat;		///< Pointer to the DMA0 channel's IRQ status.
    volatile unsigned short 	*pDMA0config;		///< Pointer to the DMA0 channel's configuration register.
    volatile void 				**pDMA0startaddr;	///< Pointer to the DMA0 channel's start address register, which itself contains a pointer to a memory location.
    volatile signed short 		*pDMA0xmodify;		///< Pointer to the DMA0 X modify (horizontal stride) register.
    volatile unsigned short 	*pDMA0xcount;		///< Pointer to the DMA0 X count register. (horizontal width)
    volatile unsigned short 	*pDMA0currXcount;	///< Pointer to the DMA0 Current count of the X counter register.
    volatile signed short 		*pDMA0ymodify;		///< Pointer to the DMA0 Y modify (vertical stride) register.
    volatile unsigned short 	*pDMA0ycount;		///< Pointer to the DMA0 Y count register (vertical height).
    volatile unsigned short 	*pDMA0currYcount;   ///< Pointer to the DMA0 Current count of the Y counter register.
    volatile void 				**pDMA0nextDescrPtr;///< Pointer to the DMA0 next descriptor register, which itself contains a pointer to a memory location.
    
    volatile unsigned short 	*pDMA1irqStat;		///< Pointer to the DMA1 channel's IRQ status.
    volatile unsigned short 	*pDMA1config;		///< Pointer to the DMA1 channel's configuration register.
    volatile void 				**pDMA1startaddr;	///< Pointer to the DMA1 channel's start address register, which itself contains a pointer to a memory location.
    volatile signed short 		*pDMA1xmodify;		///< Pointer to the DMA1 X modify (horizontal stride) register.
    volatile unsigned short 	*pDMA1xcount;		///< Pointer to the DMA1 X count register. (horizontal width)
    volatile unsigned short 	*pDMA1currXcount;	///< Pointer to the DMA1 Current count of the X counter register.
    volatile signed short 		*pDMA1ymodify;		///< Pointer to the DMA1 Y modify (vertical stride) register.
    volatile unsigned short 	*pDMA1ycount;		///< Pointer to the DMA1 Y count register (vertical height).
    volatile unsigned short 	*pDMA1currYcount;   ///< Pointer to the DMA1 Current count of the Y counter register.
    volatile void 				**pDMA1nextDescrPtr;///< Pointer to the DMA1 next descriptor register, which itself contains a pointer to a memory location.
    
    unsigned char 				cEPPInr;			///< PPI Number (PPI instance identifier)
} T_EPPI_SPEC;
	

// prototype declarations
/**
 *  @brief      set's up the the port multiplexers according to the configuration word of the EPPI
 *
 *	@public
 *  @param pa_cEPPIindex        index of the EPPI (0,1,2)
 *  @param pa_nEPPIcontrol		value of the configuration register for the EPPI
 *	@return on success          #ERR_NONE, appropriate errorcode otherwise
 *	 
 *
 **/
bool eppi_platformInit(unsigned char pa_cEPPIindex, unsigned long pa_nEPPIcontrol);

#endif

