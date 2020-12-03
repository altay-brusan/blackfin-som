// ************************************************
//
// Bitrmasks for ppi initialisation
//
// ************************************************
#ifndef _PPI_GLOBAL_H_INCLUDED_
#define _PPI_GLOBAL_H_INCLUDED_

//Parameter: pa_nDMAmode
#define PPI_AUTOBUFFER_MODE				0x1010		//PPI restarts transfering after completing each frame.
#define PPI_SINGLE_SHOT					0x0030		//PPI transfers one frame and stops.

//Parameter: pa_nDMAdirection,
#define PPI_MEMORY_READ					0x0000		//Necessary if PPI is configured in transmit mode.
#define PPI_MEMORY_WRITE				0x0002		//Necessary if PPI is configured in receive mode.

//Parameter: pa_nInterruptType
#define PPI_NO_INT						0x0000		//PPI does not generate any interrupt.
#define PPI_BLOCKWISE_INT				0x00c0		//PPI generates an interrupt after each transfered block.
#define PPI_FRAMEWISE_INT				0x0080		//PPI generates an interrupt after each transfered frame.

//Parameter: pa_nDMABusWith
#define PPI_DMA_8_BIT					0x0000		//PPI-DMA transfer configured as 8 bit transfer.
#define PPI_DMA_16_BIT					0x0004		//PPI-DMA transfer configured as 16 bit transfer.
#define PPI_DMA_32_BIT					0x0008		//PPI-DMA transfer configured as 32 bit transfer.

//Parameter: pa_nSkipping
#define PPI_SKIP_EVEN					0x0600		//PPI skips all even bytes.
#define PPI_SKIP_ODD					0x0200		//PPI skips all odd bytes.
#define PPI_SKIP_NONE					0x0000		//PPI reads in all bytes.

//Parameter: pa_nDirection
#define PPI_RECEIVE_ACTIVE_ONLY		 	0x0000		//PPI receives only the active video information.
#define PPI_RECEIVE_BLANK_ONLY			0x0008		//PPI receives only the blanking intervall information.
#define PPI_RECEIVE_ALL					0x0004		//PPI receives the hole frame information. (including blanking)
#define PPI_TRANSMIT					0x0002		//PPI is configured as output (transmit).

//Parameter: pa_nPackingMode
#define PPI_32BIT_PACKING				0x0180		//Packing 4 input bytes to one 32 bit transfer.
#define PPI_16BIT_PACKING				0x0080		//Packing 2 input bytes to one 16 bit transfer.
#define PPI_NO_PACKING					0x0000		//Packing is disabled. Each byte is transfered to the DMA.

//Parameter: pa_nFieldSelect
#define PPI_FIELD1_ONLY					0x0000		//On an interleaced video input only field 1 is transfered.
#define PPI_FIELD_BOTH					0x0040		//Both video fields are transfered.

//Parameter: pa_nSkipping
#define PPI_SKIP_EVEN					0x0600		//PPI skips all even bytes.
#define PPI_SKIP_ODD					0x0200		//PPI skips all odd bytes.
#define PPI_SKIP_NONE					0x0000		//PPI reads in all bytes.

#endif

