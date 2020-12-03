/**
 *	@file 		flash.h
 *	@ingroup 	FLASH
 *	
 *	@brief 		Intel Strata (+ compatible) Flash Driver
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
 
/** @defgroup FLASH
 *  @ingroup driverapi
 *  Flash Memory Controller
 */
 
 
#ifndef _FLASH_H_INCLUDED_
#define _FLASH_H_INCLUDED_

#include "GPIOconfig.h"

#define FLASH_PROGRAM_TIMEOUT 			0x0ffffff
#define FLASH_ERASE_TIMEOUT				0xfffffff
#define FLASH_UNLOCK_TIMEOUT			0x0ffffff

#define FLASH_DEBUG_LEVEL				0
#define FLASH_MAX_NOF_BANK_SELECT		6

#define FLASH_LAST_SECTOR				(FLASH_SECTOR_SIZE * (FLASH_NUM_SECTORS - 1))
#define FLASH_SIZE						(FLASH_SECTOR_SIZE * FLASH_NUM_SECTORS)


#ifdef _USE_VDK_
#include <VDK.h>
#endif


#ifdef FLASH_MULTIPLE_FLASH_TYPE_SUPPORT

#define MANUFACTURER_CODE_ISSI              0x9d
#define MANUFACTURER_CODE_CYPRESS           0x01
#define MANUFACTURER_CODE_INTEL             0x89

typedef enum {
    FLASH_TYPE_J3,
    FLASH_TYPE_CYPRESS,
    FLASH_TYPE_ISSI
} T_FLASH_TYPE;

#endif //FLASH_MULTIPLE_FLASH_TYPE_SUPPORT


typedef struct {
	
	unsigned char 	cNofBanks;
	unsigned long 	nBankSize;
	unsigned char 	cNofBankSelect;
	T_GPIO_MASK 	nBankSelect[FLASH_MAX_NOF_BANK_SELECT];
	bool bMultiThreading;

#ifdef _USE_VDK_
	bool 			bLocked;
	VDK_ThreadID 	tThreadId;
#endif

	unsigned char 	cCurrentBank;

#ifdef FLASH_MULTIPLE_FLASH_TYPE_SUPPORT
    T_FLASH_TYPE    tFlashType;
#endif
	
} T_FLASH_SPEC;

//prototype declarations
//management/informational
void            Flash_Setup             (void);
unsigned short  Flash_Reset             (void);
unsigned short  Flash_LockBlock         (unsigned long pa_nSectorAddr);
unsigned short  Flash_UnlockBlock       (unsigned long pa_nSectorAddr);
unsigned short  Flash_LockAll           (void);
unsigned short  Flash_UnlockAll         (void);
unsigned long   Flash_GetSectorSize     (unsigned long pa_unSectorAddr);
unsigned long   Flash_GetSectorAddress  (unsigned long pa_unSectorNumber);
int             Flash_GetSectorNumber   (unsigned long ulOffset);
// read/write operations
unsigned short  Flash_ReadWord          (unsigned long pa_nOffset);
unsigned short  Flash_ProgramWord       (unsigned long startAddress, unsigned long wordAddress, unsigned short data);
void            Flash_ReadIdentifier    (unsigned short *cDevCode, unsigned short *cManuCode);
bool            Flash_CheckIfEmpty      (unsigned long pa_nStartAddress, unsigned long pa_nEndAddress);
unsigned short  Flash_EraseSector       (unsigned long startAddress, unsigned long sectorAddress);
unsigned short  Flash_EraseChip         (void);
void Flash_Read64BitUniqueFactoryID(unsigned long long *pa_unID);
int Flash_DisableMultiThreading(void);
int Flash_EnableMultiThreading(void);

//multithreading
#ifdef _USE_VDK_
    char            Flash_LockAccess        (VDK_ThreadID *ThreadId);
#endif    
char            Flash_UnlockAccess      (void);

#endif
