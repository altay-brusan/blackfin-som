/**
	@file i2c_global.c
	@brief Processor specific implementation of I2C for BF548 
	
	@author Thomas Maier
**/

#include "../common/I2C/I2Cconfig.h"

#include <cdefbf548.h>
#include <services/services.h>

/*
	ADI_INT_PERIPHERAL_ID tPeripheralID;
	volatile unsigned short *clkdiv;	
	volatile unsigned short *control;
	volatile unsigned short *slave_ctl;
	volatile unsigned short *slave_stat;
	volatile unsigned short *slave_addr;	
	volatile unsigned short *master_ctl;
	volatile unsigned short *master_stat;
	volatile unsigned short *master_addr;
	volatile unsigned short *int_stat;
	volatile unsigned short *int_mask;
	volatile unsigned short *fifo_ctl;
	volatile unsigned short *fifo_stat;
	volatile unsigned short *xmt_data8;
	volatile unsigned short *xmt_data16;
	volatile unsigned short *rcv_data8;
	volatile unsigned short *rcv_data16;

*/

T_I2C_SPEC g_I2Cspec[] = {
	{
		ADI_INT_TWI0,		
		pTWI0_CLKDIV,
 		pTWI0_CONTROL,
 		pTWI0_SLAVE_CTL,		
 		pTWI0_SLAVE_STAT,	
 		pTWI0_SLAVE_ADDR,	
		pTWI0_MASTER_CTL,		
		pTWI0_MASTER_STAT,
 		pTWI0_MASTER_ADDR,	
 		pTWI0_INT_STAT,		
 		pTWI0_INT_MASK,		
 		pTWI0_FIFO_CTL,	
 		pTWI0_FIFO_STAT,	
 		pTWI0_XMT_DATA8,		
 		pTWI0_XMT_DATA16,	
 		pTWI0_RCV_DATA8,	
 		pTWI0_RCV_DATA16,
        false
	},
	{
		ADI_INT_TWI1,		
		pTWI1_CLKDIV,
 		pTWI1_CONTROL,
 		pTWI1_SLAVE_CTL,		
 		pTWI1_SLAVE_STAT,	
 		pTWI1_SLAVE_ADDR,	
		pTWI1_MASTER_CTL,		
		pTWI1_MASTER_STAT,
 		pTWI1_MASTER_ADDR,	
 		pTWI1_INT_STAT,		
 		pTWI1_INT_MASK,		
 		pTWI1_FIFO_CTL,	
 		pTWI1_FIFO_STAT,	
 		pTWI1_XMT_DATA8,		
 		pTWI1_XMT_DATA16,	
 		pTWI1_RCV_DATA8,	
 		pTWI1_RCV_DATA16,
        false
	}
};

unsigned int g_nI2Ccount = sizeof (g_I2Cspec) / sizeof (T_I2C_SPEC);

bool I2CplatformInit(int pa_nI2Cnumber) {
	bool bResult = true;
	
	switch (pa_nI2Cnumber) {
		case 0: {
			// set PE14 and PE15 to peripheral mode	
			*pPORTE_FER |= (0x1 << 14);
			*pPORTE_FER |= (0x1 << 15);
			*pPORTE_MUX &= ~0xf0000000;
			
			break;
		}
		case 1: {	
			// set PB0 and PB1 to peripheral mode
			*pPORTB_FER |= (0x1 << 0);
			*pPORTB_FER |= (0x1 << 1);
			break;
		}
	
		default: bResult = false;
	}
	return bResult;
}

