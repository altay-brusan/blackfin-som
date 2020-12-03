#ifndef _ENVIRONMENT_H_INCLUDED_
#define _ENVIRONMENT_H_INCLUDED_

#include "../../../flash-driver-src/driver/src/bf548/gpio_global.h"

//flash configuration
//V1.2.x, 8MB
#define _USE_EBIU_MODE_0_
#define FLASH_MULTIPLE_FLASH_TYPE_SUPPORT
#define FLASH_ISSI						///< Uncomment if ISSI compatible flash
#define FLASH_CYPRESS
#define FLASH_INTEL_STRATA
#define FLASH_START_ADDRESS         0x20000000  ///< Start address of the flash memory
#define FLASH_NUM_SECTORS			64         ///< Number of flash sectors
#define FLASH_SECTOR_SIZE			0x20000      ///< Size of flash sectors
#define FLASH_NOF_BANKS				1           ///< Number of flash banks
#define FLASH_BANK_SIZE				0x800000    ///< Size of a bank
#define FLASH_BANK_SELECT0			0        ///< Bank select pin for bank 0 (set 0 if not support)
#define FLASH_BANK_SELECT1			0           ///< Bank select pin for bank 1 (set 0 if not support)
#define FLASH_BANK_SELECT2			0           ///< Bank select pin for bank 2 (set 0 if not support)
#define FLASH_BANK_SELECT3			0           ///< Bank select pin for bank 3 (set 0 if not support)
#define FLASH_BANK_SELECT4			0           ///< Bank select pin for bank 4 (set 0 if not support)
#define FLASH_BANK_SELECT5			0           ///< Bank select pin for bank 5 (set 0 if not support)


#endif