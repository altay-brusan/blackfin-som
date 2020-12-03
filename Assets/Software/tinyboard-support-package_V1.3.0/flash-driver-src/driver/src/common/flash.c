/**
 *	@file 		flash.c
 *	@ingroup 	FLASH
 *	
 *	@brief 		Intel Strata (+ compatible) Flash Driver
 *
 *  @attention  If data caching is enabled. Be sure to use a cplbtab
 *              which has disabled caching for async bank 0 and 1!
 *		
 *	BLT_DISCLAIMER
 *	
 *	@author 	Roland Oberhammer, 
 *	
 *	@cond svn
 *	
 *	Information of last commit
*   $Rev::               $:  Revision of last commit
*   $Author::            $:  Author of last commit
*   $Date::              $:  Date of last commit
 *	
 *	@endcond
 **/
 
/** @defgroup FLASH
 *  @ingroup driverapi
 *  Flash Memory Controller
 */

/* 
    Revision History
    
    15.02.2018 ROB: Support for CFI flash added. Tested on CM-BF537E V3.4.0, Flash S29GL256S90GHI020 from Cypress
    26.06.2018 ROB: - Makro FLASH_CFI renamed to FLASH_CYPRESS
                    - Support for ISSI flash added (Makro FLASH_ISSI)
                    In environment.h set macro FLASH_INTEL_STRATA for intel strata compatibility, FLASH_CYPRESS for CYPRESS compatibility
                    or FLASH_ISSI for ISSI compatibility
                    If the makro FLASH_ISSI_DO_CHIP_ERASE_WITH_SECTOR_ERASE is invoked the chip erase is done by multiple sector erase calls.
                    If the makro is not invoked the dedicated chip erase command sequence is applied. Sector erase may be faster because only 
                    the real addressable sectors are erased not the entire flash.
    10.07.2018 ROB: - Support for multiple flash types added.
                    - Can be enabled with makro FLASH_MULTIPLE_FLASH_TYPE_SUPPORT
                    - Flash_Setup then probes for different known flash types and accordingly the appropriate access functions will be used. 
                      If the manufacturer code is not known, the Issi access functions will be used, which seems to be the most common for currently available flash types.
    29.04.2019 ROB: ProgramWord and EraseSector function for ISSI flash modified to support flash internal wait and status functions
    09.05.2019 ROB:	ISSIwaitForEndOfEraseExecution and ISSIwaitForEndOfProgramExecution functions has been gated with '#pragma optimize_off' and #pragma 'optimize_as_cmd_line'.
    				The read1, read2, and read3 variables have been decalred as volatile.
    23.07.2019 ROB: - ssync added after each read in ISSIwaitForEndOfEraseExecution and ISSIwaitForEndOfProgramExecution
                    - timeout added in FlashISSI_ProgramWord (including µs delay and flash reset command to force back to read array)
                      The timeout is 5 seconds  
                    - timeout added in FlashISSI_EraseSector (including µs delay and flash reset command to force back to read array)
                      The timeout is 5 seconds  
                    - FlashReset removed within FlashISSI_EraseSector
*/ 
//////////////////////////////////////////////////////////////
/*
#ifdef _USE_DATA_CACHING_
#include <cplbtab.h> 
#endif
*/
 
#include <conio.h>
#include <math.h>
#include <stdio.h>
#include <environment.h>
#include <cycle_count_bf.h>
#include "flash.h"


//////////////////////////////////////////////////////////////
// LOCAL PROTOTYPES
//////////////////////////////////////////////////////////////
#ifdef FLASH_ISSI
int ISSIwaitForEndOfEraseExecution(unsigned long wordAddressAMI);
int ISSIwaitForEndOfProgramExecution(unsigned long wordAddressAMI);
#endif
#ifdef FLASH_MULTIPLE_FLASH_TYPE_SUPPORT
unsigned short FlashJ3_EraseSector(unsigned long startAddress, unsigned long sectorAddress);
unsigned short FlashJ3_EraseChip(void);
unsigned short FlashJ3_ProgramWord(unsigned long startAddress, unsigned long wordAddress, unsigned short data);
void FlashJ3_ReadIdentifier (unsigned short *cDevCode, unsigned short *cManuCode);
void FlashJ3_Read64BitUniqueFactoryID(unsigned long long *pa_unID);
unsigned short FlashJ3_UnlockBlock (unsigned long pa_nSectorAddr);
unsigned short FlashJ3_LockBlock (unsigned long pa_nSectorAddr);
unsigned short FlashJ3_Reset (void);
unsigned short FlashIssi_EraseSector(unsigned long startAddress, unsigned long sectorAddress);
unsigned short FlashIssi_EraseChip(void);
unsigned short FlashIssi_ProgramWord(unsigned long startAddress, unsigned long wordAddress, unsigned short data);
void FlashIssi_ReadIdentifier (unsigned short *cDevCode, unsigned short *cManuCode);
void FlashIssi_Read64BitUniqueFactoryID(unsigned long long *pa_unID);
unsigned short FlashIssi_UnlockBlock (unsigned long pa_nSectorAddr);
unsigned short FlashIssi_LockBlock (unsigned long pa_nSectorAddr);
unsigned short FlashIssi_Reset (void);
unsigned short FlashCypress_EraseSector(unsigned long startAddress, unsigned long sectorAddress);
unsigned short FlashCypress_EraseChip(void);
unsigned short FlashCypress_ProgramWord(unsigned long startAddress, unsigned long wordAddress, unsigned short data);
void Flash_UsDelay(unsigned long pa_ulUsec);
#endif //FLASH_MULTIPLE_FLASH_TYPE_SUPPORT

//////////////////////////////////////////////////////////////

// check makros for plausibility /////////////////////////////
#if ((FLASH_BANK_SIZE * FLASH_NOF_BANKS) != FLASH_SIZE)
#error "Illegal bank- flash- size combination!"
#endif

#if !defined (FLASH_INTEL_STRATA) && !defined (FLASH_CYPRESS) && !defined (FLASH_ISSI)
    #error "No parallel NOR flash type selected!"
#endif

#if defined (FLASH_INTEL_STRATA) && defined (FLASH_CYPRESS) && defined (FLASH_ISSI) && !defined (FLASH_MULTIPLE_FLASH_TYPE_SUPPORT)
    #error "Selecting more than one NOR flash type is only allowed if makro FLASH_MULTIPLE_FLASH_TYPE_SUPPORT is set!"
#endif
//////////////////////////////////////////////////////////////

T_FLASH_SPEC g_tFlash;	///< Global for management purposes!


#ifdef _USE_VDK_

#include <VDK.h>
#include <services/services.h>


char Flash_LockAccess (VDK_ThreadID *ThreadId) {
	if (g_tFlash.bLocked) {
		*ThreadId = g_tFlash.tThreadId;
		return -1;
	} else {
		g_tFlash.tThreadId = VDK_GetThreadID();
		g_tFlash.bLocked = true;
		return 0;
	}
}

char Flash_UnlockAccess (void) {
	g_tFlash.bLocked = false;
	return 0;	
}

#endif


/// wait 1µs * pa_ulUsec using the cycle counters
void Flash_UsDelay(unsigned long pa_ulUsec) {
	
    unsigned long cclk = getCoreClockFrequency();
	
	unsigned long long int cur = 0;
	unsigned long long int nd = 0;
	
	_GET_CYCLE_COUNT(cur);
	
	nd = cur + pa_ulUsec * 1000000 * cclk;
	
	while (cur < nd) {
		_GET_CYCLE_COUNT(cur);
	}
}


static void Flash_BankSelectReset (void) {
	unsigned char i = 0;
	for (i=0; i<g_tFlash.cNofBankSelect; i++) {
		gpio_clear (g_tFlash.nBankSelect[i]);
	}
	g_tFlash.cCurrentBank = 0;
}

void Flash_BankSelectSetup (void) {
	unsigned char i = 0;
	for (i=0; i<g_tFlash.cNofBankSelect; i++) {
		gpio_becomeOutput (g_tFlash.nBankSelect[i]);
		gpio_clear (g_tFlash.nBankSelect[i]);
	}
	g_tFlash.cCurrentBank = 0;
}






/**
 *	@public
 *	@brief		Initialises the Flash driver (Sets some global configuration variables for management purposes).
 *
 *	@attention 	This function must be called prior to flash use.
 *
 *	Sets some global configuration variables for managment purposes.
 *	Has to called prior to use the flash.
 **/
void Flash_Setup (void) {
	g_tFlash.cNofBanks = FLASH_NOF_BANKS;
	g_tFlash.nBankSize = FLASH_BANK_SIZE;
	g_tFlash.cNofBankSelect = (unsigned char)(log ( (double)g_tFlash.cNofBanks) / log (2.0));
	g_tFlash.nBankSelect[0] = FLASH_BANK_SELECT0;
	g_tFlash.nBankSelect[1] = FLASH_BANK_SELECT1;
	g_tFlash.nBankSelect[2] = FLASH_BANK_SELECT2;
	g_tFlash.nBankSelect[3] = FLASH_BANK_SELECT3;
	g_tFlash.nBankSelect[4] = FLASH_BANK_SELECT4;
	g_tFlash.nBankSelect[5] = FLASH_BANK_SELECT5;
#ifdef _USE_VDK_
	g_tFlash.bLocked = false;
#endif
	Flash_BankSelectSetup ();
	
	g_tFlash.bMultiThreading = true;
	
#ifdef FLASH_MULTIPLE_FLASH_TYPE_SUPPORT	
	// changed 10.07.2018 ROB
	// check which flash is connected
	unsigned short cDevCode;
	unsigned short cManuCode;
	FlashIssi_ReadIdentifier (&cDevCode, &cManuCode);
	if (cManuCode == MANUFACTURER_CODE_INTEL) {
	    g_tFlash.tFlashType = FLASH_TYPE_J3;
	} else if (cManuCode == MANUFACTURER_CODE_CYPRESS) {
	    g_tFlash.tFlashType = FLASH_TYPE_CYPRESS;
	} else if (cManuCode == MANUFACTURER_CODE_ISSI) {
        g_tFlash.tFlashType = FLASH_TYPE_ISSI;
	} else {
	    // not a known flash type; try with issi access functions
	    g_tFlash.tFlashType = FLASH_TYPE_ISSI;
	}
#endif  //FLASH_MULTIPLE_FLASH_TYPE_SUPPORT	
}

void Flash_SelectBank (unsigned char pa_cBank) {
	if (g_tFlash.cCurrentBank != pa_cBank) {
		// bank must be changed
		if (pa_cBank < g_tFlash.cNofBanks) {
			// valid bank selected
			register unsigned char i;
			register unsigned char cBankMask = 0x01;
			for (i=0; i<g_tFlash.cNofBankSelect; i++) {
				if (pa_cBank & cBankMask) {
					gpio_set (g_tFlash.nBankSelect[i]);
				} else {
					gpio_clear (g_tFlash.nBankSelect[i]);		
				}
				cBankMask <<= 1;
			}
			g_tFlash.cCurrentBank = pa_cBank;
			// wait for stabilizing gpio pins
			//Sleep (1);	// not really necessary
			asm("ssync;");
		}
	}
}

inline unsigned char Flash_GetBank (unsigned long nOffset) {
	return (nOffset / g_tFlash.nBankSize);
}

inline unsigned long Flash_GetAddress (unsigned char cBank, unsigned long nOffset) {
	return (nOffset - cBank * g_tFlash.nBankSize);	
}


/**
    @brief Disable Multithreading
**/
int Flash_DisableMultiThreading(void) {
    g_tFlash.bMultiThreading = false;
    
    return 0;
}

int Flash_EnableMultiThreading(void) {
    g_tFlash.bMultiThreading = true;    
    
    return 0;
}






/**
 *	@public	
 *	@brief		Erases the Specified Sector
 *
 *	@param 		startAddress 	start address of the flash device
 *	@param 		sectorAddress	address of the sector to be erased
 *
 *	@return 	Contents of the flash status register (on success 0x0080)
 *	
 **/
#ifdef FLASH_MULTIPLE_FLASH_TYPE_SUPPORT
unsigned short Flash_EraseSector(unsigned long startAddress, unsigned long sectorAddress) {
    switch (g_tFlash.tFlashType) {
        case FLASH_TYPE_J3:
            return FlashJ3_EraseSector(startAddress, sectorAddress);
        case FLASH_TYPE_CYPRESS:
            return FlashCypress_EraseSector(startAddress, sectorAddress);
        case FLASH_TYPE_ISSI:
            return FlashIssi_EraseSector(startAddress, sectorAddress);
        default:
            return 0xff00;        
    }
}
#endif //FLASH_MULTIPLE_FLASH_TYPE_SUPPORT
 
 
#ifdef FLASH_INTEL_STRATA

#ifndef FLASH_MULTIPLE_FLASH_TYPE_SUPPORT
  unsigned short Flash_EraseSector(unsigned long startAddress, unsigned long sectorAddress) {
#else
  unsigned short FlashJ3_EraseSector(unsigned long startAddress, unsigned long sectorAddress) {
#endif      
	if (g_tFlash.cNofBanks > 1) {
		// we have more than one bank, so check witch bank to select
		// and correct the offset
		unsigned char cBank = Flash_GetBank (sectorAddress);
		Flash_SelectBank (cBank);
		sectorAddress = Flash_GetAddress (cBank, sectorAddress);
	}
	
	volatile unsigned short *memIndex = (unsigned short *)(startAddress + sectorAddress);
/*	
#ifdef _USE_DATA_CACHING_		
	int nOldVal = __cplb_ctrl;
	dcache_invalidate_both(); 
	disable_data_cache();
#endif	
*/
	*memIndex = 0x0050;
	asm("ssync;");
	*memIndex = 0x0020;
	asm("ssync;");
	*memIndex = 0x00d0;
	asm("ssync;");

	unsigned short status;
	unsigned long nTimeout = 0;
	do {
		status = *memIndex;
		asm("ssync;");
	#ifdef _USE_VDK_
	    if (g_tFlash.bMultiThreading) {
	        VDK_Yield();
	    }
	#endif    
		nTimeout ++;
	}
	while (((status & 0x0080) == 0x00) && (nTimeout < FLASH_ERASE_TIMEOUT));
		
	if (nTimeout >= FLASH_ERASE_TIMEOUT) {
		status = 0xff00;
	}
	
	nTimeout = FLASH_ERASE_TIMEOUT;
	do {
		*memIndex = 0xff; 		//back to "read array mode"
		asm("ssync;");
	#ifdef _USE_VDK_
	     if (g_tFlash.bMultiThreading) {
	        VDK_Yield();
	    }
	#endif    
		nTimeout--;
	}
	while ((*memIndex != 0xffff) && nTimeout);
	
	if (!nTimeout) {
		status = 0xff00;
	}
/*	
#ifdef _USE_DATA_CACHING_		
	enable_data_cache(nOldVal);
#endif	
*/	
	return (status);
}

#endif //FLASH_INTEL_STRATA


#ifdef FLASH_CYPRESS

#ifndef FLASH_MULTIPLE_FLASH_TYPE_SUPPORT
  unsigned short Flash_EraseSector(unsigned long startAddress, unsigned long sectorAddress) {
#else
  unsigned short FlashCypress_EraseSector(unsigned long startAddress, unsigned long sectorAddress) {
#endif      
	unsigned char cBank = 0;
	unsigned long sectorAddressOffset = sectorAddress;
	
	// select bank 0
	if (g_tFlash.cNofBanks > 1) {
		// we have more than one bank, so check witch bank to select
		// and correct the offset
		Flash_SelectBank (0);
		cBank = Flash_GetBank (sectorAddress);
		sectorAddressOffset = Flash_GetAddress (cBank, sectorAddress);
	}

	// wait until device is ready
	unsigned long nTimeout = 0;
	unsigned short status;	
	do {
		*(unsigned short *)(startAddress + (0x555 << 1)) = 0x70;
		status = *(unsigned short *)(startAddress);
		asm("ssync;");
#ifdef _USE_VDK_	
		if (g_tFlash.bMultiThreading) {
        	VDK_Yield();
    	}
#endif	
		nTimeout++;
	}
	while (((status & 0x0080) == 0x00) && (nTimeout < FLASH_PROGRAM_TIMEOUT));
	if (nTimeout >= FLASH_PROGRAM_TIMEOUT) {
		return 0xff00;
	}
	
	// clear the status register
	*(unsigned short *)(startAddress + (0x555 << 1)) = 0x71;
	asm("ssync;");
	
	// prepare sector erase command
	*(unsigned short *)(startAddress + (0x555 << 1)) = 0xaa;
	asm("ssync;");
	*(unsigned short *)(startAddress + (0x2aa << 1)) = 0x55;
	asm("ssync;");
	*(unsigned short *)(startAddress + (0x555 << 1)) = 0x80;
	asm("ssync;");
	*(unsigned short *)(startAddress + (0x555 << 1)) = 0xaa;
	asm("ssync;");
	*(unsigned short *)(startAddress + (0x2aa << 1)) = 0x55;
	asm("ssync;");
	
	if (g_tFlash.cNofBanks > 1) {
		Flash_SelectBank (cBank);
	}	
	
	// write the command to the right sector address
	volatile unsigned short *memIndex = (unsigned short *)(startAddress + sectorAddressOffset);
	*memIndex = 0x30;
	asm("ssync;");
	
/*		
#ifdef _USE_DATA_CACHING_		
	int nOldVal = __cplb_ctrl;
	dcache_invalidate_both(); 
	disable_data_cache();
#endif	
*/
	// return to bank 0
	if (g_tFlash.cNofBanks > 1) {
		Flash_SelectBank (0);
	}
	
	// poll the status register
	status = 0;
	nTimeout = 0;
	do {
		*(unsigned short *)(startAddress + (0x555 << 1)) = 0x70;
		asm("ssync;");
		
		status = *memIndex & 0xa2;
		asm("ssync;");
		
	#ifdef _USE_VDK_
	    if (g_tFlash.bMultiThreading) {
	        VDK_Yield();
	    }
	#endif    
		nTimeout ++;
	}
	while (((status != 0x80) && (nTimeout < FLASH_ERASE_TIMEOUT)) || (status & 0x20 != 0x20));
		
	if (nTimeout >= FLASH_ERASE_TIMEOUT) {
		return 0xff00;
	}
	if (status & 0x20 == 0x20) {
	    // sector is locked
	    return 0xfe00;
	}
	
	// return to read array mode ////////////////////
	// select the right bank
	if (g_tFlash.cNofBanks > 1) {
		Flash_SelectBank (cBank);
	}
	
	nTimeout = FLASH_ERASE_TIMEOUT;
	memIndex = (unsigned short *)(startAddress + sectorAddressOffset);
	do {
		*memIndex = 0xf0; 		//back to "read array mode"
		asm("ssync;");
	#ifdef _USE_VDK_
	     if (g_tFlash.bMultiThreading) {
	        VDK_Yield();
	    }
	#endif    
		nTimeout--;
	}
	while ((*memIndex != 0xffff) && nTimeout);
	
	if (!nTimeout) {
		status = 0xff00;
	}
/*
#ifdef _USE_DATA_CACHING_		
	enable_data_cache(nOldVal);
#endif	
*/	
	return (status);
}

#endif //FLASH_CYPRESS

#ifdef FLASH_ISSI

#ifndef FLASH_MULTIPLE_FLASH_TYPE_SUPPORT
  unsigned short Flash_EraseSector(unsigned long startAddress, unsigned long sectorAddress) {
#else
  unsigned short FlashIssi_EraseSector(unsigned long startAddress, unsigned long sectorAddress) {
#endif      
    unsigned short usRetVal = 0;
	unsigned char cBank = 0;
	unsigned long sectorAddressOffset = sectorAddress;
	
	// select bank 0
	if (g_tFlash.cNofBanks > 1) {
		// we have more than one bank, so check witch bank to select
		// and correct the offset
		Flash_SelectBank (0);
		cBank = Flash_GetBank (sectorAddress);
		sectorAddressOffset = Flash_GetAddress (cBank, sectorAddress);
	}
	Flash_Reset();

	// prepare sector erase command
	*(unsigned short *)(startAddress + (0x555 << 1)) = 0xaa;
	asm("ssync;");
	*(unsigned short *)(startAddress + (0x2aa << 1)) = 0x55;
	asm("ssync;");
	*(unsigned short *)(startAddress + (0x555 << 1)) = 0x80;
	asm("ssync;");
	*(unsigned short *)(startAddress + (0x555 << 1)) = 0xaa;
	asm("ssync;");
	*(unsigned short *)(startAddress + (0x2aa << 1)) = 0x55;
	asm("ssync;");
	
	// write the command to the right sector address
	volatile unsigned short *memIndex = (unsigned short *)(startAddress + sectorAddressOffset);
	if (g_tFlash.cNofBanks > 1) {
		Flash_SelectBank (cBank);
	}	
	*memIndex = 0x30;
	asm("ssync;");
	
	if ((ISSIwaitForEndOfEraseExecution(startAddress + sectorAddressOffset) == 0)) {
	    // check if first sectoraddress is empty
	    unsigned long ulTimeout = 5000000 / 100;
	    while (ulTimeout) {
	        ulTimeout--;
    	    // reset to read array
      	    *memIndex = 0xf0;
      	    asm("ssync;");
        	usRetVal = *memIndex;
    	    if (usRetVal == 0xffff) {
    	        ulTimeout = 0;
    	    } else {
    	        Flash_UsDelay(100);
    	    }
	    }
	    if (usRetVal == 0xffff) {
	        usRetVal = 0x80;
	    } else {
	        usRetVal = 0xee80;
	    }
	} else {
	    usRetVal = 0xee80;
	}
	
	
/*	unsigned short usResult;
	unsigned long nTimeout = 0;
	do {
	    usResult = *(unsigned short *)(startAddress + sectorAddressOffset);
	    asm ("ssync;");
	#ifdef _USE_VDK_
	    if (g_tFlash.bMultiThreading) {
	        VDK_Yield();
	    }
	#endif    
		nTimeout ++;	    
	} while ((usResult != 0xffff) && (nTimeout < FLASH_ERASE_TIMEOUT));
	
	if (nTimeout >= FLASH_ERASE_TIMEOUT) {
	    Flash_Reset();
		return 0xff00;
	}	
*/
	 // wait and read status
/*	if ((ISSIcheckFlashStatus(startAddress + sectorAddressOffset) == 0)) {
	    usRetVal = 0x80;
	} else {
	    usRetVal = 0xee80;
	}*/
	
	
	return usRetVal;
	
}

#endif //FLASH_CYPRESS





/**
 *	@public
 *	@brief		Erases the Entire Flash memory!
 *
 *	@return 	Contents of the flash status register (on success 0x0080 for Intel Strata Compatible Devices)
 *	
 **/
#ifdef FLASH_MULTIPLE_FLASH_TYPE_SUPPORT
unsigned short Flash_EraseChip(void) {
    switch (g_tFlash.tFlashType) {
        case FLASH_TYPE_J3:
            return FlashJ3_EraseChip();
        case FLASH_TYPE_CYPRESS:
            return FlashCypress_EraseChip();
        case FLASH_TYPE_ISSI:
            return FlashIssi_EraseChip();
        default:
            return 0xff00;        
    }
}
#endif //FLASH_MULTIPLE_FLASH_TYPE_SUPPORT
 
#ifdef FLASH_INTEL_STRATA

#ifndef FLASH_MULTIPLE_FLASH_TYPE_SUPPORT
  unsigned short Flash_EraseChip(void) {
#else
  unsigned short FlashJ3_EraseChip(void) {
#endif      
	
	volatile unsigned long sectorAddress;
	unsigned short status;
	
	sectorAddress = 0;
	Flash_Reset ();
	do {
		status = Flash_EraseSector(FLASH_START_ADDRESS, sectorAddress);
		#if FLASH_DEBUG_LEVEL > 1
			printf("  Status of block %xh: %xh\n", sectorAddress, status);
		#endif
		sectorAddress += FLASH_SECTOR_SIZE;
	}
	while ((sectorAddress <= FLASH_LAST_SECTOR) && (status != 0xff00));

	return (status);
}

#endif //FLASH_INTEL_STRATA

#ifdef FLASH_CYPRESS

#ifndef FLASH_MULTIPLE_FLASH_TYPE_SUPPORT
  unsigned short Flash_EraseChip(void) {
#else
  unsigned short FlashCypress_EraseChip(void) {      
#endif      
	
	// select bank 0
	if (g_tFlash.cNofBanks > 1) {
		Flash_SelectBank (0);
	}

	// wait until device is ready
	unsigned long nTimeout = 0;
	unsigned short status;	
	do {
		*(unsigned short *)(FLASH_START_ADDRESS + (0x555 << 1)) = 0x70;
		status = *(unsigned short *)(FLASH_START_ADDRESS);
		asm("ssync;");
#ifdef _USE_VDK_	
		if (g_tFlash.bMultiThreading) {
        	VDK_Yield();
    	}
#endif	
		nTimeout++;
	}
	while (((status & 0x0080) == 0x00) && (nTimeout < FLASH_PROGRAM_TIMEOUT));
	if (nTimeout >= FLASH_PROGRAM_TIMEOUT) {
		return 0xff00;
	}
	
	// clear the status register
	*(unsigned short *)(FLASH_START_ADDRESS + (0x555 << 1)) = 0x71;
	asm("ssync;");
	
	// send chip erase command sequence
	*(unsigned short *)(FLASH_START_ADDRESS + (0x555 << 1)) = 0xaa;
	asm("ssync;");
	*(unsigned short *)(FLASH_START_ADDRESS + (0x2aa << 1)) = 0x55;
	asm("ssync;");
	*(unsigned short *)(FLASH_START_ADDRESS + (0x555 << 1)) = 0x80;
	asm("ssync;");
	*(unsigned short *)(FLASH_START_ADDRESS + (0x555 << 1)) = 0xaa;
	asm("ssync;");
	*(unsigned short *)(FLASH_START_ADDRESS + (0x2aa << 1)) = 0x55;
	asm("ssync;");
	*(unsigned short *)(FLASH_START_ADDRESS + (0x555 << 1)) = 0x10;
	asm("ssync;");
		
	// poll the status register
	status = 0;
	nTimeout = 0;
	do {
		*(unsigned short *)(FLASH_START_ADDRESS + (0x555 << 1)) = 0x70;
		asm("ssync;");
		
		status = *(unsigned short *)(FLASH_START_ADDRESS) & 0xa2;
		asm("ssync;");
		
	#ifdef _USE_VDK_
	    if (g_tFlash.bMultiThreading) {
	        VDK_Yield();
	    }
	#endif    
		nTimeout ++;
	}
	while (((status != 0x80) && (nTimeout < FLASH_ERASE_TIMEOUT)) || (status & 0x20 != 0x20));
		
	if (nTimeout >= FLASH_ERASE_TIMEOUT) {
		return 0xff00;
	}
	if (status & 0x20 == 0x20) {
	    // sector is locked
	    return 0xfe00;
	}
	
	// return to read array mode ////////////////////
	nTimeout = FLASH_ERASE_TIMEOUT;
	do {
		*(unsigned short *)(FLASH_START_ADDRESS) = 0xf0; 		//back to "read array mode"
		asm("ssync;");
	#ifdef _USE_VDK_
	     if (g_tFlash.bMultiThreading) {
	        VDK_Yield();
	    }
	#endif    
		nTimeout--;
	}
	while ((*(unsigned short *)(FLASH_START_ADDRESS) != 0xffff) && nTimeout);
	
	if (!nTimeout) {
		status = 0xff00;
	}

	return (status);
}

#endif //FLASH_CYPRESS

#ifdef FLASH_ISSI

#ifdef FLASH_ISSI_DO_CHIP_ERASE_WITH_SECTOR_ERASE

#ifndef FLASH_MULTIPLE_FLASH_TYPE_SUPPORT
  unsigned short Flash_EraseChip(void) {
#else
  unsigned short FlashIssi_EraseChip(void) {
#endif
	
	volatile unsigned long sectorAddress;
	unsigned short status;
	
	sectorAddress = 0;
	Flash_Reset ();
	do {
		status = Flash_EraseSector(FLASH_START_ADDRESS, sectorAddress);
		#if FLASH_DEBUG_LEVEL > 1
			printf("  Status of block %xh: %xh\n", sectorAddress, status);
		#endif
		sectorAddress += FLASH_SECTOR_SIZE;
	}
	while ((sectorAddress <= FLASH_LAST_SECTOR) && (status != 0xff00));

	return (status);
}

#else   //FLASH_ISSI_DO_CHIP_ERASE_WITH_SECTOR_ERASE

#ifndef FLASH_MULTIPLE_FLASH_TYPE_SUPPORT
  unsigned short Flash_EraseChip(void) {
#else
  unsigned short FlashIssi_EraseChip(void) {
#endif      
/*	// select bank 0
	if (g_tFlash.cNofBanks > 1) {
		Flash_SelectBank (0);
	}    
    // send chip erase command sequence
	*(unsigned short *)(FLASH_START_ADDRESS + (0x555 << 1)) = 0xaa;
	asm("ssync;");
	*(unsigned short *)(FLASH_START_ADDRESS + (0x2aa << 1)) = 0x55;
	asm("ssync;");
	*(unsigned short *)(FLASH_START_ADDRESS + (0x555 << 1)) = 0x80;
	asm("ssync;");
	*(unsigned short *)(FLASH_START_ADDRESS + (0x555 << 1)) = 0xaa;
	asm("ssync;");
	*(unsigned short *)(FLASH_START_ADDRESS + (0x2aa << 1)) = 0x55;
	asm("ssync;");
	*(unsigned short *)(FLASH_START_ADDRESS + (0x555 << 1)) = 0x10;
	asm("ssync;");

    // wait and read status
	unsigned short usResult;
	unsigned long nTimeout = 0;
	do {
	    usResult = *(unsigned short *)(FLASH_START_ADDRESS);
	    asm ("ssync;");
	#ifdef _USE_VDK_
	    if (g_tFlash.bMultiThreading) {
	        VDK_Yield();
	    }
	#endif    
		nTimeout ++;
	} while ((usResult != 0xffff) && (nTimeout < FLASH_ERASE_TIMEOUT));
	
	if (nTimeout >= FLASH_ERASE_TIMEOUT) {
		return 0xff00;
	}
	return 0x0080;
	*/
	return 0xff00;
}
#endif  //FLASH_ISSI_DO_CHIP_ERASE_WITH_SECTOR_ERASE

#endif //FLASH_ISSI






/**
 *	@public
 *	@brief 		Programs a Word at the specified address. (startAddress + wordAddress)
 *
 *	@param 		startAddress 	Start address of the flash device
 *	@param 		wordAddress		Address of the word to be programmed
 *	@param 		data			Value to be written
 *
 *	@return 	0x0080 All data written sucessfully, 0xff00 Programming sequence timeout error, 0xee00 Value not programmed correctly.
 *
 **/
#ifdef FLASH_MULTIPLE_FLASH_TYPE_SUPPORT
unsigned short Flash_ProgramWord(unsigned long startAddress, unsigned long wordAddress, unsigned short data) {
    switch (g_tFlash.tFlashType) {
        case FLASH_TYPE_J3:
            return FlashJ3_ProgramWord(startAddress, wordAddress, data);
        case FLASH_TYPE_CYPRESS:
            return FlashCypress_ProgramWord(startAddress, wordAddress, data);
        case FLASH_TYPE_ISSI:
            return FlashIssi_ProgramWord(startAddress, wordAddress, data);
        default:
            return 0xff00;        
    }
}
#endif //FLASH_MULTIPLE_FLASH_TYPE_SUPPORT

#ifdef FLASH_INTEL_STRATA

// Programmingsequence for intel flash devices.
// Offset: Wordaddress, relativ to startAddress
// Return values:
// 0x0080:	everything ok.
// 0xff00:	programming sequence timeout error.
// 0xee00:	value not programmed correctly.

#ifndef FLASH_MULTIPLE_FLASH_TYPE_SUPPORT
  unsigned short Flash_ProgramWord(unsigned long startAddress, unsigned long wordAddress, unsigned short data) {
#else
  unsigned short FlashJ3_ProgramWord(unsigned long startAddress, unsigned long wordAddress, unsigned short data) {
#endif      
/*
#ifdef _USE_DATA_CACHING_		
	int nOldVal = __cplb_ctrl;
	dcache_invalidate_both(); 
	disable_data_cache();
#endif
*/
	if (g_tFlash.cNofBanks > 1) {
		// we have more than one bank, so check witch bank to select
		// and correct the offset
		unsigned char cBank = Flash_GetBank (wordAddress);
		Flash_SelectBank (cBank);
		wordAddress = Flash_GetAddress (cBank, wordAddress);
	}
	
	volatile unsigned short *memIndex;
	unsigned long nTimeout;
	unsigned short status;
	
	memIndex = (unsigned short *)(startAddress + wordAddress);

	*memIndex = 0x00ff;
	asm("ssync;");
	*memIndex = 0x0050; 	//reset statusregister
	asm("ssync;");
	*memIndex = 0x0040;
	asm("ssync;");
	*memIndex = data;
	asm("ssync;");
	
	nTimeout=0;
	do {
		status = *memIndex;
		asm("ssync;");
	#ifdef _USE_VDK_	
		 if (g_tFlash.bMultiThreading) {
	        VDK_Yield();
	    }
	#endif	
		nTimeout++;
	}
	while (((status & 0x0080) == 0x00) && (nTimeout < FLASH_PROGRAM_TIMEOUT));
	if (nTimeout >= FLASH_PROGRAM_TIMEOUT) {
		status = 0xff00;
	}
	
	nTimeout = FLASH_PROGRAM_TIMEOUT;
	unsigned short nValue;
	if (status != 0xff00) {
		do {
			*memIndex = 0xffff; 		//back to "read array mode".
			asm("ssync;");
			nValue = *memIndex;
			asm("ssync;");
		#ifdef _USE_VDK_
			 if (g_tFlash.bMultiThreading) {
	        VDK_Yield();
	    }
		#endif
            nTimeout --;
		} while (nTimeout && (nValue != data));
		if (nTimeout == 0) {
			status = 0xee00;		
		}
	}
/*	
#ifdef _USE_DATA_CACHING_		
	enable_data_cache(nOldVal);
#endif	
*/	
	return status;	
}

#endif //FLASH_INTEL_STRATA

#ifdef FLASH_CYPRESS

#ifndef FLASH_MULTIPLE_FLASH_TYPE_SUPPORT
  unsigned short Flash_ProgramWord(unsigned long startAddress, unsigned long wordAddress, unsigned short data) {
#else
  unsigned short FlashCypress_ProgramWord(unsigned long startAddress, unsigned long wordAddress, unsigned short data) {      
#endif      
/*
#ifdef _USE_DATA_CACHING_		
	int nOldVal = __cplb_ctrl;
	dcache_invalidate_both(); 
	disable_data_cache();
#endif
*/
	unsigned char cWordAddressBank = 0;
	unsigned long wordAddressAMI = wordAddress + startAddress;
	// select bank 0
	if (g_tFlash.cNofBanks > 1) {
		// we have more than one bank, so check witch bank to select
		// and correct the offset
		cWordAddressBank = Flash_GetBank (wordAddress);
		wordAddressAMI = Flash_GetAddress (cWordAddressBank, wordAddress) + startAddress;
		Flash_SelectBank (0);
	}
	
	// wait until device is ready
	unsigned long nTimeout = 0;
	unsigned short status;	
	do {
		*(unsigned short *)(startAddress + (0x555 << 1)) = 0x70;
		status = *(unsigned short *)(startAddress);
		asm("ssync;");
#ifdef _USE_VDK_	
		if (g_tFlash.bMultiThreading) {
        	VDK_Yield();
    	}
#endif	
		nTimeout++;
	}
	while (((status & 0x0080) == 0x00) && (nTimeout < FLASH_PROGRAM_TIMEOUT));
	if (nTimeout >= FLASH_PROGRAM_TIMEOUT) {
		return 0xff00;
	}
	

	// clear the status register
	*(unsigned short *)(startAddress + (0x555 << 1)) = 0x71;
	asm("ssync;");
	
	// prepare program word command
	*(unsigned short *)(startAddress + (0x555 << 1)) = 0xaa;
	asm("ssync;");
	*(unsigned short *)(startAddress + (0x2aa << 1)) = 0x55;
	asm("ssync;");
	*(unsigned short *)(startAddress + (0x555 << 1)) = 0xa0;
	asm("ssync;");
	
	// select the right bank
	if (g_tFlash.cNofBanks > 1) {
		Flash_SelectBank (cWordAddressBank);		
	}
	// program word
	*(unsigned short *)(wordAddressAMI) = data;
	
	// read status register ////////////////////////////////////////
	nTimeout = 0;
	status = 0;
	// select bank 0
	if (g_tFlash.cNofBanks > 1) {
		Flash_SelectBank (0);
	}	

	do {
		*(unsigned short *)(startAddress + (0x555 << 1)) = 0x70;
		status = *(unsigned short *)(startAddress) & 0x92;
		asm("ssync;");
#ifdef _USE_VDK_	
	 if (g_tFlash.bMultiThreading) {
        VDK_Yield();
    }
#endif	
		nTimeout++;
	}
	while (((status != 0x80) && (nTimeout < FLASH_PROGRAM_TIMEOUT)) || (status & 0x02 != 0x02));
	if (nTimeout >= FLASH_PROGRAM_TIMEOUT) {
		return 0xff00;
	}
	if (status & 0x02 == 0x02) {
	    // sector is locked
	    return 0xfe00;
	}
	
	// verify ///////////////////////////////////////////////////////		
	nTimeout = FLASH_PROGRAM_TIMEOUT;
	unsigned short nValue;
	// select the right bank
	if (g_tFlash.cNofBanks > 1) {
		// we have more than one bank, so check witch bank to select
		// and correct the offset
		Flash_SelectBank (cWordAddressBank);
	}	
	do {
		*(unsigned short *)(startAddress) = 0xf0; 		//back to "read array mode".
		asm("ssync;");
		nValue = *(unsigned short *)(wordAddressAMI);
		asm("ssync;");
#ifdef _USE_VDK_
 		if (g_tFlash.bMultiThreading) {
			VDK_Yield();
		}
#endif
		nTimeout --;
	} while (nTimeout && (nValue != data));
	if (nTimeout == 0) {
		return 0xee00;		
	}

		/*	
#ifdef _USE_DATA_CACHING_		
	enable_data_cache(nOldVal);
#endif	
*/	
	return 0x80;	
}

#endif //FLASH_CYPRESS

#ifdef FLASH_ISSI

#pragma optimize_off
int ISSIwaitForEndOfEraseExecution(unsigned long wordAddressAMI) {
	int nRetVal = 0;
	//unsigned long usNofWaitCycles = 0;
	bool bToggling = true;	
	volatile unsigned short read1;
	volatile unsigned short read2;
	volatile unsigned short read3;
	
	do {
	    bToggling = true;
    	read1 = *(unsigned short *)(wordAddressAMI);
    	asm("ssync;");
    	read2 = *(unsigned short *)(wordAddressAMI);
    	asm("ssync;");
    	read3 = *(unsigned short *)(wordAddressAMI);
    	asm("ssync;");
	    if (((read1 & 0x40) == (read2 & 0x40)) && ((read1 & 0x40) == (read3 & 0x40)) && ((read2 & 0x40) == (read3 & 0x40))) {
	        // we are done
	        bToggling = false;
	    } else {
	        // still toggling check DQ5 for timeout
	        if (read1 & 0x20) {
	            // timeout occured
	            bToggling = false;
	            nRetVal = -1;
	        }
	    }
	    //usNofWaitCycles++;
	} while (bToggling);

	read1 = *(unsigned short *)(wordAddressAMI);
	asm("ssync;");
	read2 = *(unsigned short *)(wordAddressAMI);
	asm("ssync;");
	
	if ((read1 & 0x02) != (read2 & 0x02)) {
	    // return suspend
	    nRetVal = -1;
	}

    return nRetVal;
}
#pragma optimize_as_cmd_line

#pragma optimize_off
int ISSIwaitForEndOfProgramExecution(unsigned long wordAddressAMI) {
	int nRetVal = 0;
	bool bToggling = true;	
	volatile unsigned short read1;
	volatile unsigned short read2;
	volatile unsigned short read3;
	
	do {
	    bToggling = true;
    	read1 = *(unsigned short *)(wordAddressAMI);
    	asm("ssync;");
    	read2 = *(unsigned short *)(wordAddressAMI);
    	asm("ssync;");
    	read3 = *(unsigned short *)(wordAddressAMI);
    	asm("ssync;");
	    if (((read1 & 0x40) == (read2 & 0x40)) && ((read1 & 0x40) == (read3 & 0x40)) && ((read2 & 0x40) == (read3 & 0x40))) {
	        // we are done
	        bToggling = false;
	    }
	} while (bToggling);

	// do additional reads to be on save side
	read1 = *(unsigned short *)(wordAddressAMI);
	asm("ssync;");
	read2 = *(unsigned short *)(wordAddressAMI);
	asm("ssync;");
    
    return nRetVal;
}
#pragma optimize_as_cmd_line

#ifndef FLASH_MULTIPLE_FLASH_TYPE_SUPPORT
  unsigned short Flash_ProgramWord(unsigned long startAddress, unsigned long wordAddress, unsigned short data) {
#else
  unsigned short FlashIssi_ProgramWord(unsigned long startAddress, unsigned long wordAddress, unsigned short data) {
#endif      

    unsigned short usRetVal = 0;
    unsigned short usResult;
	unsigned char cWordAddressBank = 0;
	unsigned long wordAddressAMI = wordAddress + startAddress;
	// select bank 0
	if (g_tFlash.cNofBanks > 1) {
		// we have more than one bank, so check witch bank to select
		// and correct the offset
		cWordAddressBank = Flash_GetBank (wordAddress);
		wordAddressAMI = Flash_GetAddress (cWordAddressBank, wordAddress) + startAddress;
		Flash_SelectBank (0);
	}
	
	// prepare program word command
	*(unsigned short *)(startAddress + (0x555 << 1)) = 0xaa;
	asm("ssync;");
	*(unsigned short *)(startAddress + (0x2aa << 1)) = 0x55;
	asm("ssync;");
	*(unsigned short *)(startAddress + (0x555 << 1)) = 0xa0;
	asm("ssync;");
	
	// select the right bank
	if (g_tFlash.cNofBanks > 1) {
		Flash_SelectBank (cWordAddressBank);		
	}
	// program word
	*(unsigned short *)(wordAddressAMI) = data;
	asm("ssync;");
	
	if ((ISSIwaitForEndOfProgramExecution(wordAddressAMI) == 0)) {
	    unsigned long ulTimeout = 5000000 / 100;
	    while (ulTimeout) {
    	    ulTimeout--;
    	    // reset to read array
    	    *(unsigned short *)(wordAddressAMI) = 0xf0;
    	    asm("ssync;");
    	    usResult = *(unsigned short *)(wordAddressAMI);
    	    if (usResult == data) {
    	        ulTimeout = 0;
    	    } else {
    	        Flash_UsDelay(100);
    	    }
	    }
	    if (usResult == data) {
	        usRetVal = 0x80;
	    } else {
	        usRetVal = 0xee00;
	    }
    } else {
	    usRetVal = 0xee00;
	}
	
	return usRetVal;
	    
	/*
	unsigned short usResult;
	unsigned long nTimeout = 0;
	do {
	    usResult = *(unsigned short *)(wordAddressAMI);
	    asm ("ssync;");
	#ifdef _USE_VDK_
	    if (g_tFlash.bMultiThreading) {
	        VDK_Yield();
	    }
	#endif    
		nTimeout ++;	    
	} while ((usResult != data) && (nTimeout < FLASH_ERASE_TIMEOUT));
	
	if (nTimeout >= FLASH_ERASE_TIMEOUT) {
	    Flash_Reset();
		return 0xff00;
	}
	*/
	// read status and verify
/*	if ((ISSIcheckFlashStatus(wordAddressAMI) == 0) && (*(unsigned short *)(wordAddressAMI) == data)) {
	    return 0x80;
	} else {
	    return 0xee00;
	}*/
//	return 0x80;

}

#endif //FLASH_ISSI





/**
 *	@public
 *	@brief		Reads the Flash Device identifier, and Manufacture Codes
 *
 *	@param 		cDevCode	Holds the device code after the function call
 *	@param 		cManuCode	Holds the manufacturer code after the function call	
 *
 **/
#ifdef FLASH_MULTIPLE_FLASH_TYPE_SUPPORT
void Flash_ReadIdentifier (unsigned short *cDevCode, unsigned short *cManuCode) {
    switch (g_tFlash.tFlashType) {
        case FLASH_TYPE_J3:
            FlashJ3_ReadIdentifier (cDevCode, cManuCode);
        break;
        case FLASH_TYPE_CYPRESS:
            FlashIssi_ReadIdentifier (cDevCode, cManuCode);
        break;
        case FLASH_TYPE_ISSI:
            FlashIssi_ReadIdentifier (cDevCode, cManuCode);
        break;
        default:
            *cDevCode = 0xffff;
            *cManuCode = 0xffff;
    }
}
#endif //FLASH_MULTIPLE_FLASH_TYPE_SUPPORT

#ifdef FLASH_INTEL_STRATA

#ifndef FLASH_MULTIPLE_FLASH_TYPE_SUPPORT
  void Flash_ReadIdentifier (unsigned short *cDevCode, unsigned short *cManuCode) {
#else
  void FlashJ3_ReadIdentifier (unsigned short *cDevCode, unsigned short *cManuCode) {
#endif      
	volatile unsigned short *nFlashAddr;

	Flash_BankSelectReset ();
	nFlashAddr = (unsigned short *)FLASH_START_ADDRESS;
	
	*nFlashAddr = 0x0090;	//read identifier command.
	asm("ssync;");
/*
#ifdef _USE_DATA_CACHING_		
		flush_data_cache(); 
#endif
*/		
	*cManuCode = *nFlashAddr;
	asm("ssync;");
	nFlashAddr++;
/*	
#ifdef _USE_DATA_CACHING_		
		flush_data_cache(); 
#endif	
*/
	*cDevCode  = *nFlashAddr;
	asm("ssync;");
	
	*nFlashAddr = 0x00ff;	//return to read array mode.
	asm("ssync;");
}

#endif //FLASH_INTEL_STRATA

#if defined (FLASH_ISSI) || defined (FLASH_CYPRESS)

#ifndef FLASH_MULTIPLE_FLASH_TYPE_SUPPORT
  void Flash_ReadIdentifier (unsigned short *cDevCode, unsigned short *cManuCode) {
#else
  void FlashIssi_ReadIdentifier (unsigned short *cDevCode, unsigned short *cManuCode) {
#endif      

	Flash_BankSelectReset ();
	// prepare read ID command
	*(unsigned short *)(FLASH_START_ADDRESS + (0x555 << 1)) = 0xaa;
	asm("ssync;");
	*(unsigned short *)(FLASH_START_ADDRESS + (0x2aa << 1)) = 0x55;
	asm("ssync;");
	*(unsigned short *)(FLASH_START_ADDRESS + (0x555 << 1)) = 0x90;
	asm("ssync;");
	
	*cManuCode = *(unsigned short *)(FLASH_START_ADDRESS);
		
	*cDevCode = *(unsigned short *)(FLASH_START_ADDRESS + (0x1 << 1));
	
	// reset flash
	*(unsigned short *)(FLASH_START_ADDRESS) = 0x00f0;
	asm("ssync;");	
	
}

#endif //FLASH_ISSI





/**
 *	@public
 *	@brief		Reads the Flash Device identifier, and Manufacture Codes
 *
 *	@param 		cDevCode	Holds the device code after the function call
 *	@param 		cManuCode	Holds the manufacturer code after the function call	
 *
 **/
#ifdef FLASH_MULTIPLE_FLASH_TYPE_SUPPORT
void Flash_Read64BitUniqueFactoryID(unsigned long long *pa_unID) {
    switch (g_tFlash.tFlashType) {
        case FLASH_TYPE_J3:
            FlashJ3_Read64BitUniqueFactoryID(pa_unID);
        break;
        case FLASH_TYPE_CYPRESS:
            FlashJ3_Read64BitUniqueFactoryID(pa_unID);
        break;
        case FLASH_TYPE_ISSI:
            FlashIssi_Read64BitUniqueFactoryID(pa_unID);
        break;
        default:
            *pa_unID = (unsigned long long)0xffffffffffffffff;
    }
}
#endif //FLASH_MULTIPLE_FLASH_TYPE_SUPPORT

#if defined (FLASH_INTEL_STRATA) || defined (FLASH_CYPRESS)

#ifndef FLASH_MULTIPLE_FLASH_TYPE_SUPPORT
  void Flash_Read64BitUniqueFactoryID(unsigned long long *pa_unID) {
#else
  void FlashJ3_Read64BitUniqueFactoryID(unsigned long long *pa_unID) {
#endif      
	volatile unsigned short *nFlashAddr;
	
	Flash_BankSelectReset ();
	nFlashAddr = (unsigned short *)FLASH_START_ADDRESS;
	
	*nFlashAddr = 0x0090;	//read identifier command.
	asm("ssync;");
/*
#ifdef _USE_DATA_CACHING_		
		flush_data_cache(); 
#endif
*/		
    nFlashAddr += 0x81;
	*pa_unID = *nFlashAddr;
	asm("ssync;");
	
	nFlashAddr++;
	*pa_unID += ((unsigned long long)*nFlashAddr) << 16;
	asm("ssync;");

	nFlashAddr++;
	*pa_unID += ((unsigned long long)*nFlashAddr) << 32;
	asm("ssync;");

	nFlashAddr++;
	*pa_unID += ((unsigned long long)*nFlashAddr) << 48;
	asm("ssync;");
		
	
/*	
#ifdef _USE_DATA_CACHING_		
		flush_data_cache(); 
#endif	
*/
	
	*nFlashAddr = 0x00ff;	//return to read array mode.
	asm("ssync;");
	
}

#endif //defined (FLASH_INTEL_STRATA) || defined (FLASH_CYPRESS)

#ifdef FLASH_ISSI

#ifndef FLASH_MULTIPLE_FLASH_TYPE_SUPPORT
  void Flash_Read64BitUniqueFactoryID(unsigned long long *pa_unID) {
#else
  void FlashIssi_Read64BitUniqueFactoryID(unsigned long long *pa_unID) {
#endif      
	Flash_BankSelectReset ();
	// prepare read ID command
	*(unsigned short *)(FLASH_START_ADDRESS + (0x555 << 1)) = 0xaa;
	asm("ssync;");
	*(unsigned short *)(FLASH_START_ADDRESS + (0x2aa << 1)) = 0x55;
	asm("ssync;");
	*(unsigned short *)(FLASH_START_ADDRESS + (0x555 << 1)) = 0x90;
	asm("ssync;");
	
	*pa_unID = 0;
	*pa_unID = (unsigned long long)*(unsigned short *)(FLASH_START_ADDRESS) 
	            | (unsigned long long)*(unsigned short *)(FLASH_START_ADDRESS + (0x1 << 1)) << 16
	            | (unsigned long long)*(unsigned short *)(FLASH_START_ADDRESS + (0xe << 1)) << 32
	            | (unsigned long long)*(unsigned short *)(FLASH_START_ADDRESS + (0xf << 1)) << 48;
	
	// reset flash
	*(unsigned short *)(FLASH_START_ADDRESS) = 0x00f0;
	asm("ssync;");	
		
}

#endif //FLASH_ISSI




/**
 *	@public
 *	@brief		Unlocks a specified sector of the flash
 *
 *	@param 		pa_nSectorAddr		Sector address to be unlocked
 *
 *	@return		Contents of the flash status register (on success 0x0080 for Intel Strata Compatible Devices)
 *
 **/
#ifdef FLASH_MULTIPLE_FLASH_TYPE_SUPPORT
unsigned short Flash_UnlockBlock (unsigned long pa_nSectorAddr) {
    switch (g_tFlash.tFlashType) {
        case FLASH_TYPE_J3:
            return FlashJ3_UnlockBlock (pa_nSectorAddr);
        case FLASH_TYPE_CYPRESS:
            return FlashIssi_UnlockBlock (pa_nSectorAddr);
        case FLASH_TYPE_ISSI:
            return FlashIssi_UnlockBlock (pa_nSectorAddr);
        default:
            return 0xff00;
    }
}
#endif //FLASH_MULTIPLE_FLASH_TYPE_SUPPORT

#ifdef FLASH_INTEL_STRATA

#ifndef FLASH_MULTIPLE_FLASH_TYPE_SUPPORT
  unsigned short Flash_UnlockBlock (unsigned long pa_nSectorAddr) {
#else
  unsigned short FlashJ3_UnlockBlock (unsigned long pa_nSectorAddr) {
#endif      
      
	if (g_tFlash.cNofBanks > 1) {
		// we have more than one bank, so check witch bank to select
		// and correct the offset
		unsigned char cBank = Flash_GetBank (pa_nSectorAddr);
		Flash_SelectBank (cBank);
		pa_nSectorAddr = Flash_GetAddress (cBank, pa_nSectorAddr);
	}
	
	volatile unsigned short *nFlashAddr;
	unsigned short nStatus = 0;

	nFlashAddr = (unsigned short *)(FLASH_START_ADDRESS + pa_nSectorAddr);

	*nFlashAddr = 0x00ff;	
	asm("ssync;");
	*nFlashAddr = 0x0050; 	//reset statusregister
	asm("ssync;");
	*nFlashAddr = 0x0060;	
	asm("ssync;");
	*nFlashAddr = 0x00d0;	
	asm("ssync;");
	
/*
#ifdef _USE_DATA_CACHING_		
		flush_data_cache(); 
#endif	
*/
	unsigned long nTimeout = FLASH_UNLOCK_TIMEOUT;
	do {
		nStatus = *nFlashAddr;
	#ifdef _USE_VDK_
	     if (g_tFlash.bMultiThreading) {
	        VDK_Yield();
	    }
	#endif    	
		nTimeout --;
	} while (((nStatus & 0x0080) != 0x0080) && nTimeout);

	*nFlashAddr = 0x00ff;		//return to read array mode.	
	
	if (!nTimeout) {
		nStatus = 0xff00;
	}
		
	return (nStatus);
}

#endif //FLASH_INTEL_STRATA

#if defined (FLASH_CYPRESS) || defined (FLASH_ISSI)

#ifndef FLASH_MULTIPLE_FLASH_TYPE_SUPPORT
  unsigned short Flash_UnlockBlock (unsigned long pa_nSectorAddr) {
#else      
  unsigned short FlashIssi_UnlockBlock (unsigned long pa_nSectorAddr) {
#endif
    return 0x80;
}

#endif //defined (FLASH_CYPRESS) || defined (FLASH_ISSI)





/**
 *	@public
 *	@brief		Locks a specified sector of the Flash
 *
 *	@param 		pa_nSectorAddr	Address of sector to be locked
 *
 *	@return		Contents of the flash status register (on success 0x0080 for Intel Strata Compatible Devices)
 *	
 **/
 #ifdef FLASH_MULTIPLE_FLASH_TYPE_SUPPORT
unsigned short Flash_LockBlock (unsigned long pa_nSectorAddr) {
    switch (g_tFlash.tFlashType) {
        case FLASH_TYPE_J3:
            return FlashJ3_LockBlock (pa_nSectorAddr);
        case FLASH_TYPE_CYPRESS:
            return FlashIssi_LockBlock (pa_nSectorAddr);
        case FLASH_TYPE_ISSI:
            return FlashIssi_LockBlock (pa_nSectorAddr);
        default:
            return 0xff00;
    }
}
#endif //FLASH_MULTIPLE_FLASH_TYPE_SUPPORT

#ifdef FLASH_INTEL_STRATA

#ifndef FLASH_MULTIPLE_FLASH_TYPE_SUPPORT
  unsigned short Flash_LockBlock (unsigned long pa_nSectorAddr) {
#else      
  unsigned short FlashJ3_LockBlock (unsigned long pa_nSectorAddr) {
#endif      
	if (g_tFlash.cNofBanks > 1) {
		// we have more than one bank, so check witch bank to select
		// and correct the offset
		unsigned char cBank = Flash_GetBank (pa_nSectorAddr);
		Flash_SelectBank (cBank);
		pa_nSectorAddr = Flash_GetAddress (cBank, pa_nSectorAddr);
	}
		
	volatile unsigned short *nFlashAddr;
	unsigned short nStatus = 0x0080;
	//unsigned long nTimeout = 0;

	nFlashAddr = (unsigned short *)(FLASH_START_ADDRESS + pa_nSectorAddr);

	//nTimeout=0;
	asm("ssync;");
	*nFlashAddr = 0x0060;	
	asm("ssync;");
	*nFlashAddr = 0x0001;	
	asm("ssync;");
	*nFlashAddr = 0x00ff;		//return to read array mode.
	asm("ssync;");
/*
#ifdef _USE_DATA_CACHING_		
		flush_data_cache(); 
#endif
*/	
	return (nStatus);
}

#endif //FLASH_INTEL_STRATA

#if defined (FLASH_CYPRESS) || defined (FLASH_ISSI)

#ifndef FLASH_MULTIPLE_FLASH_TYPE_SUPPORT
  unsigned short Flash_LockBlock (unsigned long pa_nSectorAddr) {
#else      
  unsigned short FlashIssi_LockBlock (unsigned long pa_nSectorAddr) {
#endif      
    return 0x80;
}

#endif //defined (FLASH_CYPRESS) || defined (FLASH_ISSI)






/**
 *	@public
 *	@brief		Locks all sectors of the Flash
 *
 *	@return		Contents of the flash status register (on success 0x0080 for Intel Strata Compatible Devices)
 *	
 **/
unsigned short Flash_LockAll (void) {
	volatile unsigned long sectorAddress = 0;
	unsigned short status = 0;
	
	Flash_Reset ();
	do {
		status = Flash_LockBlock (sectorAddress);
		sectorAddress += FLASH_SECTOR_SIZE;
	}
	while ((sectorAddress <= FLASH_LAST_SECTOR) && (status != 0xff00));

	return status;
}





/**
 *	@public
 *	@brief		Unlocks all sectors of the Flash
 *
 *	@return		Contents of the flash status register (on success 0x0080 for Intel Strata Compatible Devices)
 *	
 **/
unsigned short Flash_UnlockAll (void) {
	volatile unsigned long sectorAddress = 0;
	unsigned short status = 0;
	
	Flash_Reset ();
	do {
		status = Flash_UnlockBlock (sectorAddress);
		sectorAddress += FLASH_SECTOR_SIZE;
	}
	while ((sectorAddress <= FLASH_LAST_SECTOR) && (status != 0xff00));

	return status;
}





/**
 *	@public
 *	@brief		Resets the Flash's State Machine to default values.
 *
 *	@return 	Returns 0 on sucess!
 *
 **/
 
 /* TODO: It seems the return value here is unecessary, or maybe we could handle it better! */
 #ifdef FLASH_MULTIPLE_FLASH_TYPE_SUPPORT
unsigned short Flash_Reset (void) {
    switch (g_tFlash.tFlashType) {
        case FLASH_TYPE_J3:
            return FlashJ3_Reset();
        case FLASH_TYPE_CYPRESS:
            return FlashJ3_Reset();
        case FLASH_TYPE_ISSI:
            return FlashIssi_Reset();;
        default:
            return 0xff00;
    }
}
#endif //FLASH_MULTIPLE_FLASH_TYPE_SUPPORT
 
#if defined (FLASH_INTEL_STRATA) || defined (FLASH_CYPRESS)

#ifndef FLASH_MULTIPLE_FLASH_TYPE_SUPPORT
  unsigned short Flash_Reset (void) {
#else
  unsigned short FlashJ3_Reset (void) {
#endif      
      
	// reset the bank selectors
	Flash_BankSelectReset ();
	// return to read array mode
	*(unsigned short *)FLASH_START_ADDRESS = 0xffff;
	asm("ssync;");
	
	return 0;
}

#endif //defined (FLASH_INTEL_STRATA) || defined (FLASH_CYPRESS)

#ifdef FLASH_ISSI

#ifndef FLASH_MULTIPLE_FLASH_TYPE_SUPPORT
  unsigned short Flash_Reset (void) {
#else
  unsigned short FlashIssi_Reset (void) {
#endif            
	// reset the bank selectors
	Flash_BankSelectReset ();
	// reset flash
	*(unsigned short *)(FLASH_START_ADDRESS) = 0x00f0;
	asm("ssync;");	
	
	return 0;
}

#endif





/**
 *	@public
 *	@brief		Reads a word from the specified address
 *
 *	@param 		pa_nOffset 	Address offset to the Word
 *
 *	@return 	Contents of the flash status register (on success 0x0080 for Intel Strata Compatible Devices)
 *	
 **/
unsigned short Flash_ReadWord (unsigned long pa_nOffset) {
/*
#ifdef _USE_DATA_CACHING_		
	int nOldVal = __cplb_ctrl;
	dcache_invalidate_both(); 
	disable_data_cache();
#endif
*/	
	if (g_tFlash.cNofBanks > 1) {
		// we have more than one bank, so check which bank to select
		// and correct the offset
		unsigned char cBank = Flash_GetBank (pa_nOffset);
		Flash_SelectBank (cBank);
		pa_nOffset = Flash_GetAddress (cBank, pa_nOffset);
	}
	
	volatile unsigned short *nFlashAddr;
	unsigned short nValue;
	
	nFlashAddr = (unsigned short *)(FLASH_START_ADDRESS + pa_nOffset);

	// put the value at the location passed in
	nValue = (*nFlashAddr);
	asm ("ssync;");
/*	
#ifdef _USE_DATA_CACHING_		
	enable_data_cache(nOldVal);
#endif	
*/
	return (nValue);
}





/**
 *	@public
 *	@brief		Gets the sector number of the specified Address offset.
 *
 *	@param 		ulOffset		Address offset
 *
 *	@return 	Sector number containing the memory location specified by the Address offset.
 *	
 **/
int Flash_GetSectorNumber( unsigned long ulOffset) {
    /*
	if (g_tFlash.cNofBanks > 1) {
		// we have more than one bank, so check witch bank to select
		// and correct the offset
		unsigned char cBank = Flash_GetBank (ulOffset);
		Flash_SelectBank (cBank);
		ulOffset = Flash_GetAddress (cBank, ulOffset);
	}	
	*/
	return ((int)(ulOffset / FLASH_SECTOR_SIZE));		//calculates the sectornumber.
}






/**
 *	@public
 *	@brief		Checks if the Flash at the specified address is Empty.
 *
 *	@attention	Empty means a value of 0xffff (Erased Value!)
 *
 *	@param 		pa_nStartAddress	Start address of the check
 *	@param		pa_nEndAddress		End address of the check
 *
 *	@return 	true if empty, otherwise false
 **/
bool Flash_CheckIfEmpty (unsigned long pa_nStartAddress, unsigned long pa_nEndAddress) {
	unsigned long nWordIndex;

	// calculate relative offset to FLASH_START_ADDRESS
	pa_nStartAddress -= FLASH_START_ADDRESS;
	for (nWordIndex=pa_nStartAddress; nWordIndex < pa_nEndAddress/*FLASH_SIZE*/; nWordIndex+=2) {
		if (Flash_ReadWord(nWordIndex) != 0xffff) {
			return false;
		}
	}
		
	return true;
}






/**
 *	@public
 *	@brief		Get the size of the sector.
 *
 *	@param 		pa_unSectorAddr	Sector of interrest
 *
 *	@return 	the size of the given sector
 **/
unsigned long Flash_GetSectorSize(unsigned long pa_unSectorAddr) {
	return FLASH_SECTOR_SIZE;
}






/**
 *	@public
 *	@brief		Get the sector address.
 *
 *	@param 		pa_unSectorNumber	Sector number of interrest
 *
 *	@return 	the address of the given sector
 **/
unsigned long Flash_GetSectorAddress(unsigned long pa_unSectorNumber) {
	return (pa_unSectorNumber * FLASH_SECTOR_SIZE);
}

