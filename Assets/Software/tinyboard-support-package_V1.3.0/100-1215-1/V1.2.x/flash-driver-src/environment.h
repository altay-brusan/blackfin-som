#ifndef _ENVIRONMENT_H_INCLUDED_
#define _ENVIRONMENT_H_INCLUDED_

#include "../../../flash-driver-src/driver/src/bf561/gpio_global.h"

//flash configuration
//V1.2.x, 32MB
#define FLASH_MULTIPLE_FLASH_TYPE_SUPPORT
#define FLASH_ISSI
#define FLASH_CYPRESS	        					
#define FLASH_INTEL_STRATA
#define FLASH_ISSI_DO_CHIP_ERASE_WITH_SECTOR_ERASE
#define FLASH_START_ADDRESS         0x20000000  ///< Start address of the flash memory
#define FLASH_NUM_SECTORS			256         ///< Number of flash sectors
#define FLASH_SECTOR_SIZE			0x20000      ///< Size of flash sectors
#define FLASH_NOF_BANKS				1           ///< Number of flash banks
#define FLASH_BANK_SIZE				0x2000000    ///< Size of a bank
#define FLASH_BANK_SELECT0			0        ///< Bank select pin for bank 0 (set 0 if not support)
#define FLASH_BANK_SELECT1			0           ///< Bank select pin for bank 1 (set 0 if not support)
#define FLASH_BANK_SELECT2			0           ///< Bank select pin for bank 2 (set 0 if not support)
#define FLASH_BANK_SELECT3			0           ///< Bank select pin for bank 3 (set 0 if not support)
#define FLASH_BANK_SELECT4			0           ///< Bank select pin for bank 4 (set 0 if not support)
#define FLASH_BANK_SELECT5			0           ///< Bank select pin for bank 5 (set 0 if not support)

#endif