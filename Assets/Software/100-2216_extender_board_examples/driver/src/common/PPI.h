/**
 *	@file 		PPI.h
 *	@ingroup 	PPI
 *	
 *	@brief 		Parrallel Port Interface
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
 *  @ingroup 	driver
 *
 * 	@brief Parallel port interface.
 *	
 */
 
 #ifndef __PPI_H_INCLUDED__
#define __PPI_H_INCLUDED__
#include <services/services.h>

#define PPI_DEFAULT_DMA_CHANNEL     ADI_DMA_DMA0

typedef struct {
		bool 						bInUse;							///< Flags the PPI as in use.
		ADI_DMA_CHANNEL_ID 			enDMAchannel;					///< Interrupt channel that the DMA Channel of the ppi uses.
		ADI_INT_PERIPHERAL_ID 		peripheralIntId;			    ///< Peripheral Interrupt ID, (Interrupt channel the peripheral is mapped to)
		ADI_INT_PERIPHERAL_ID 		errorIntId;						///< Peripheral Interrupt ID, (Interrupt channel the peripheral Error interrupt is mapped to)
		ADI_INT_PERIPHERAL_ID 		DMAerrorIntId;                  ///< Interrupt channel for DMA errors
		ADI_DMA_CHANNEL_HANDLE 		hDMAchannel;					///< Identifies the DMA channel.
		volatile unsigned short 	*pPPIframe;						///< Pointer to the PPI Frame register (Number of Lines per Frame)
		volatile unsigned short 	*pPPIdelay;						///< Pointer to the PPI Delay count register.
		volatile unsigned short 	*pPPIcount;						///< Pointer to the PPI Transfer count register.
		volatile unsigned short 	*pPPIcontrol;					///< Pointer to the PPI Control register.
		volatile unsigned short 	*pPPIstatus;					///< Pointer to the PPI Status register.
		volatile unsigned short 	*pDMAirqStat;					///< Pointer to the DMA channel's IRQ status.
		volatile unsigned short 	*pDMAconfig;					///< Pointer to the DMA channel's configuration register.
		volatile void 				**pDMAstartaddr;			    ///< Pointer to the DMA channel's start address register, which itself contains a pointer to a memory location.
		volatile signed short 		*pDMAxmodify;					///< Pointer to the DMA X modify (horizontal stride) register.
		volatile unsigned short 	*pDMAxcount;					///< Pointer to the DMA X count register. (horizontal width)
		volatile unsigned short 	*pDMAcurrXcount;			    ///< Pointer to the DMA Current count of the X counter register.
		volatile signed short 		*pDMAymodify;					///< Pointer to the DMA Y modify (vertical stride) register.
		volatile unsigned short 	*pDMAycount;					///< Pointer to the DMA Y count register (vertical height).
		volatile unsigned short 	*pDMAcurrYcount;			    ///< Pointer to the DMA Current count of the Y counter register.
		volatile void 				**pDMAnextDescrPtr;		        ///< Pointer to the DMA next descriptor register, which itself contains a pointer to a memory location.
		unsigned char 				cPPInr;                         ///< PPI Number (PPI instance identifier)
	} T_PPI_SPEC;

bool ppi_platformInit(unsigned char pa_cPPIindex, unsigned short pa_nPPIcontrol);	
	
#endif
