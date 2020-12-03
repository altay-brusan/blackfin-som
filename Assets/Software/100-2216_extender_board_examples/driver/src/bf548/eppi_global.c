/**
 *	@file 		eppi_global.c
 *	@ingroup 	EPPI
 *	
 *	@brief 		Enhanced Parrallel Port Interface Driver global configuration
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
 * 	@brief Enhanced Parallel port interface driver global configuration. 
 *	
 */


#include "../common/EPPI.h"

#include <cdefbf548.h>
#include <datatypes.h>
#include <services/services.h>

T_EPPI_SPEC g_aEPPIspec[] = {
	{
	    false, 
	    ADI_DMA_DMA12, 
	    ADI_INT_DMA12_EPPI0, 
	    ADI_INT_EPPI0_ERROR, 
	    ADI_INT_DMAC0_ERROR, 
	    ADI_DMA_DMA12, 
	    ADI_INT_DMA12_EPPI0, 
	    ADI_INT_EPPI0_ERROR,
	    ADI_INT_DMAC0_ERROR, 
	    
	    pEPPI0_STATUS, 
	    pEPPI0_HCOUNT, 
	    pEPPI0_HDELAY, 
	    pEPPI0_VCOUNT, 
	    pEPPI0_VDELAY, 
	    pEPPI0_FRAME, 
	    pEPPI0_LINE,
	    pEPPI0_CLKDIV, 
	    pEPPI0_CONTROL, 
	    pEPPI0_FS1W_HBL, 
	    pEPPI0_FS1P_AVPL, 
	    pEPPI0_FS2W_LVB, 
	    pEPPI0_FS2P_LAVF, 
	    pEPPI0_CLIP,
	    
	    pDMA12_IRQ_STATUS, 
	    pDMA12_CONFIG, 
	    (volatile void **)pDMA12_START_ADDR, 
	    pDMA12_X_MODIFY, 
	    pDMA12_X_COUNT,
	    pDMA12_CURR_X_COUNT, 
	    pDMA12_Y_MODIFY, 
	    pDMA12_Y_COUNT, 
	    pDMA12_CURR_Y_COUNT, 
	    (volatile void **)pDMA12_NEXT_DESC_PTR,

	    pDMA12_IRQ_STATUS, 
	    pDMA12_CONFIG, 
	    (volatile void **)pDMA12_START_ADDR, 
	    pDMA12_X_MODIFY, 
	    pDMA12_X_COUNT,
	    pDMA12_CURR_X_COUNT, 
	    pDMA12_Y_MODIFY, 
	    pDMA12_Y_COUNT, 
	    pDMA12_CURR_Y_COUNT, 
	    (volatile void **)pDMA12_NEXT_DESC_PTR,
	    0
	},
	{
	    false, 
	    ADI_DMA_DMA13, 
	    ADI_INT_DMA13_EPPI1, 
	    ADI_INT_EPPI1_ERROR,
	    ADI_INT_DMAC1_ERROR, 
	    ADI_DMA_DMA14, 
	    ADI_INT_DMA14_EPPI2_HOST, 
	    ADI_INT_EPPI2_ERROR,
	    ADI_INT_DMAC1_ERROR,  
	    
	    pEPPI1_STATUS, 
	    pEPPI1_HCOUNT, 
	    pEPPI1_HDELAY, 
	    pEPPI1_VCOUNT, 
	    pEPPI1_VDELAY, 
	    pEPPI1_FRAME, 
	    pEPPI1_LINE,
	    pEPPI1_CLKDIV, 
	    pEPPI1_CONTROL, 
	    pEPPI1_FS1W_HBL, 
	    pEPPI1_FS1P_AVPL, 
	    pEPPI1_FS2W_LVB, 
	    pEPPI1_FS2P_LAVF, 
	    pEPPI1_CLIP, 
	    
	    pDMA13_IRQ_STATUS, 
	    pDMA13_CONFIG, 
	    (volatile void **)pDMA13_START_ADDR, 
	    pDMA13_X_MODIFY, 
	    pDMA13_X_COUNT,
	    pDMA13_CURR_X_COUNT, 
	    pDMA13_Y_MODIFY, 
	    pDMA13_Y_COUNT, 
	    pDMA13_CURR_Y_COUNT, 
	    (volatile void **)pDMA13_NEXT_DESC_PTR,
	    
	    pDMA14_IRQ_STATUS, 
	    pDMA14_CONFIG, 
	    (volatile void **)pDMA14_START_ADDR, 
	    pDMA14_X_MODIFY, 
	    pDMA14_X_COUNT,
	    pDMA14_CURR_X_COUNT, 
	    pDMA14_Y_MODIFY, 
	    pDMA14_Y_COUNT, 
	    pDMA14_CURR_Y_COUNT, 
	    (volatile void **)pDMA14_NEXT_DESC_PTR,
	    1
    },	
	{
	    false, 
	    ADI_DMA_DMA14, 
	    ADI_INT_DMA14_EPPI2_HOST, 
	    ADI_INT_EPPI2_ERROR, 
	    ADI_INT_DMAC1_ERROR, 
	    ADI_DMA_DMA13, 
	    ADI_INT_DMA13_EPPI1, 
	    ADI_INT_EPPI1_ERROR, 
	    ADI_INT_DMAC1_ERROR, 
	    
	    pEPPI2_STATUS, 
	    pEPPI2_HCOUNT, 
	    pEPPI2_HDELAY, 
	    pEPPI2_VCOUNT, 
	    pEPPI2_VDELAY, 
	    pEPPI2_FRAME, 
	    pEPPI2_LINE,
	    pEPPI2_CLKDIV, 
	    pEPPI2_CONTROL, 
	    pEPPI2_FS1W_HBL, 
	    pEPPI2_FS1P_AVPL, 
	    pEPPI2_FS2W_LVB, 
	    pEPPI2_FS2P_LAVF, 
	    pEPPI2_CLIP, 
	    
	    pDMA14_IRQ_STATUS, 
	    pDMA14_CONFIG, 
	    (volatile void **)pDMA14_START_ADDR, 
	    pDMA14_X_MODIFY, 
	    pDMA14_X_COUNT,
	    pDMA14_CURR_X_COUNT, 
	    pDMA14_Y_MODIFY, 
	    pDMA14_Y_COUNT, 
	    pDMA14_CURR_Y_COUNT, 
	    (volatile void **)pDMA14_NEXT_DESC_PTR,
	    
	    pDMA13_IRQ_STATUS, 
	    pDMA13_CONFIG, 
	    (volatile void **)pDMA13_START_ADDR, 
	    pDMA13_X_MODIFY, 
	    pDMA13_X_COUNT,
	    pDMA13_CURR_X_COUNT, 
	    pDMA13_Y_MODIFY, 
	    pDMA13_Y_COUNT, 
	    pDMA13_CURR_Y_COUNT, 
	    (volatile void **)pDMA13_NEXT_DESC_PTR,
	    2
    }			
};
	
unsigned int g_nEPPIcount = sizeof(g_aEPPIspec) / sizeof(T_EPPI_SPEC);		//calculate the nr of elements in the array


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
bool eppi_platformInit(unsigned char pa_cEPPIindex, unsigned long pa_nEPPIcontrol) {
	// switch according to config register the appropriate output pins for PPI
	
	unsigned char cDLEN;
	unsigned char cFS_CFG;
	
	cDLEN = (unsigned char) ((pa_nEPPIcontrol>>15) & 0x7);
	cFS_CFG = (unsigned char) ((pa_nEPPIcontrol>>4) & 0x3);
	// switch EPPI number
	switch(pa_cEPPIindex) {
	    case 0:
	        // EPPI0
	        if(!(pa_nEPPIcontrol & 0x4) || !(pa_nEPPIcontrol & 0x8)) {
	            // bt656 mode
                /*
	            *pPORTG_FER |= 0x0002;	// enabling FS1 signal. PG1
	            *pPORTG_FER |= 0x0004;	// enabling FS2 signal. PG2
	            *pPORTH_FER |= 0x0002;	// enabling FS3 signal. PH1
                */
	            *pPORTG_FER |= 0x0001;	// enabling CLK signal. PG0
	            
	            *pPORTG_MUX &= ~(0x00000003);  // MUX 00 CLK
	            //*pPORTG_MUX &= ~(0x0000003F);  // MUX 00 for FS1, FS2, CLK
	            *pPORTH_MUX = ((*pPORTH_MUX & ~0x0000000C) | 0x00000004);  // MUX 01 for FS3
	        
	            // enable data bits, only 10 or 8 bits width should used according to datasheet
	            if(cDLEN == 0) {
	                // 000 8Bit
	                *pPORTF_FER |= 0x00FF;	    // enabling D0-D7 signal. PF0-PF7
	                *pPORTF_MUX &= ~(0x0000FFFF);  // MUX 00
	                
	            } else if(cDLEN == 1) {
	                // 001 10Bit
	                *pPORTF_FER |= 0x03FF;	// enabling D0-D9 signal. PF0-PF9
	                *pPORTF_MUX &= ~(0x000FFFFF);  // MUX 00
	            } else if(cDLEN == 4) {
	                // 100 16 bit
                    *pPORTF_FER |= 0xFFFF;	        // enabling D0-D15 signal. PF0-PF15
                    *pPORTF_MUX &= ~(0xFFFFFFFF);   // MUX 00
	            } else {
	                return false;
	            }
	            
    	    } else {
    	        // GP frame sync mode
	            // set data lines
	            switch (cDLEN) {
	                case 0:
	                    // 8Bit
	                    *pPORTF_FER |= 0x00FF;	        // enabling D0-D7 signal. PF0-PF7
	                    *pPORTF_MUX &= ~(0x0000FFFF);   // MUX 00
	                break;    
	                case 1:
	                    // 10 Bit
	                    *pPORTF_FER |= 0x03FF;	        // enabling D0-D9 signal. PF0-PF9
	                    *pPORTF_MUX &= ~(0x000FFFFF);   // MUX 00
	                break; 
	                case 2:
	                    // 12 Bit
	                    *pPORTF_FER |= 0x0FFF;	        // enabling D0-D11 signal. PF0-PF11
	                    *pPORTF_MUX &= ~(0x00FFFFFF);   // MUX 00
	                break; 
	                case 3:
	                    // 14 Bit
	                    *pPORTF_FER |= 0x3FFF;	        // enabling D0-D13 signal. PF0-PF13
	                    *pPORTF_MUX &= ~(0x0FFFFFFF);   // MUX 00
	                break; 
	                case 4:
	                    // 16 Bit
	                    *pPORTF_FER |= 0xFFFF;	        // enabling D0-D15 signal. PF0-PF15
	                    *pPORTF_MUX &= ~(0xFFFFFFFF);   // MUX 00
	                break; 
	                case 5:
	                    // 18 Bit
	                    *pPORTF_FER |= 0xFFFF;	        // enabling D0-D15 signal. PF0-PF15
	                    *pPORTF_MUX &= ~(0xFFFFFFFF);   // MUX 00
	                    *pPORTG_FER |= 0x0018;	        // enabling D16-D17 signal. PG3-PG4
	                    *pPORTG_MUX &= ~(0x000003C0);   // MUX 00
	                    
	                break; 
	                case 6:
	                    // 24 Bit
	                    *pPORTF_FER |= 0xFFFF;	        // enabling D0-D15 signal. PF0-PF15
	                    *pPORTF_MUX &= ~(0xFFFFFFFF);   // MUX 00
	                    *pPORTG_FER |= 0x0018;	        // enabling D16-D17 signal. PG3-PG4
	                    *pPORTG_MUX &= ~(0x000003C0);   // MUX 00
	                    *pPORTD_FER |= 0x003F;	        // enabling D18-D23 signal. PD0-PD5
	                    *pPORTD_MUX |= (0x00000FFF);    // MUX 11
	                break; 
	                default: 
	                    return false;   
	            }
	            
	            // set frame sync and CLK lines
	            switch(cFS_CFG) {
	                case 2:
	                    *pPORTH_FER |= 0x0002;	        // enabling FS3 signal. PH1	
	                    *pPORTH_MUX = ((*pPORTH_MUX & ~0x0000000C) | 0x00000004);  // MUX 01 for FS3                   
//	                case 2:
    	                *pPORTG_FER |= 0x0004;	        // enabling FS2 signal. PG2 
    	                *pPORTG_MUX &= ~(0x00000030);   // MUX 00 for FS2	                
	                case 1:
	                    *pPORTG_FER |= 0x0002;	        // enabling FS1 signal. PG1	
	                    *pPORTG_MUX &= ~(0x0000000C);   // MUX 00 for FS1            
	                break;
	            }
	            
	            // enable Clock
	            *pPORTG_FER |= 0x0001;	        // enabling CLK signal. PG0
	            *pPORTG_MUX &= ~(0x00000003);   // MUX 00 for CLK  
    	    }
	    break;
	    
	    case 1:
	        // EPPI1
	        if(!(pa_nEPPIcontrol & 0x4) || !(pa_nEPPIcontrol & 0x8)) {
	            // bt656 mode
                /*
	            *pPORTE_FER |= 0x1000;	// enabling FS1 signal. PE12
	            *pPORTE_FER |= 0x2000;	// enabling FS2 signal. PE13
	            *pPORTH_FER |= 0x0001;	// enabling FS3 signal. PH0
                */
	            *pPORTE_FER |= 0x0800;	// enabling CLK signal. PE11
	            
	            *pPORTE_MUX &= ~(0x00C00000);  // MUX 00 CLK
	            //*pPORTE_MUX &= ~(0x0FC00000);  // MUX 00 for FS1, FS2, CLK
	            //*pPORTH_MUX = ((*pPORTH_MUX & ~0x00000003) | 0x00000001);  // MUX 01 for FS3
	        
	            // enable data bits, only 10 or 8 bits width should used according to datasheet
	            if(cDLEN == 0) {
	                // 000 8Bit
	                *pPORTD_FER |= 0x00FF;	    // enabling D0-D7 signal. PD0-PD7
	                *pPORTD_MUX &= ~(0x0000FFFF);  // MUX 00
	                
	            } else if(cDLEN == 1) {
	                // 001 10Bit
	                *pPORTD_FER |= 0x03FF;	// enabling D0-D9 signal. PD0-PD9
	                *pPORTD_MUX &= ~(0x000FFFFF);  // MUX 00
	            } else if(cDLEN == 4) {
	                // 100 16 bit
                    *pPORTD_FER |= 0xFFFF;	        // enabling D0-D15 signal. PD0-PD15
                    *pPORTD_MUX &= ~(0xFFFFFFFF);   // MUX 00
	            } else {
	                return false;
	            }
	            
    	    } else {
    	        // GP frame sync mode
	            // set data lines
	            switch (cDLEN) {
	                case 0:
	                    // 8Bit
    	                *pPORTD_FER |= 0x00FF;	        // enabling D0-D7 signal. PD0-PD7
    	                *pPORTD_MUX &= ~(0x0000FFFF);   // MUX 00
	                break;    
	                case 1:
	                    // 10 Bit
    	                *pPORTD_FER |= 0x03FF;	        // enabling D0-D9 signal. PD0-PD9
    	                *pPORTD_MUX &= ~(0x000FFFFF);   // MUX 00
	                break; 
	                case 2:
	                    // 12 Bit
	                    *pPORTD_FER |= 0x0FFF;	        // enabling D0-D11 signal. PD0-PD11
	                    *pPORTD_MUX &= ~(0x00FFFFFF);   // MUX 00
	                break; 
	                case 3:
	                    // 14 Bit
	                    *pPORTD_FER |= 0x3FFF;	        // enabling D0-D13 signal. PD0-PD13
	                    *pPORTD_MUX &= ~(0x0FFFFFFF);   // MUX 00
	                break; 
	                case 4:
	                    // 16 Bit
	                    *pPORTD_FER |= 0xFFFF;	        // enabling D0-D15 signal. PD0-PD15
	                    *pPORTD_MUX &= ~(0xFFFFFFFF);   // MUX 00
	                break; 
	                default: 
	                    return false;
	            }
	            
	            // set frame sync and CLK lines
	            switch(cFS_CFG) {
	                case 2:
	                    *pPORTH_FER |= 0x0001;	                                    // enabling FS3 signal. PH0
	                    *pPORTH_MUX = ((*pPORTH_MUX & ~0x00000003) | 0x00000001);  // MUX 01 for FS3                  
//	                case 2:
    	                *pPORTE_FER |= 0x2000;	// enabling FS2 signal. PE13 
    	                *pPORTE_MUX &= ~(0x0C000000);  // MUX 00 for FS1, FS2, CLK	                
	                case 1:
	                    *pPORTE_FER |= 0x1000;	// enabling FS1 signal. PE12
	                    *pPORTE_MUX &= ~(0x03000000);  // MUX 00 for FS1, FS2, CLK            
	                break;
	            }
	            
	            // enable Clock
	            *pPORTE_FER |= 0x0800;	// enabling CLK signal. PE11
	            *pPORTE_MUX &= ~(0x00C00000);  // MUX 00 for FS1, FS2, CLK  
    	    }
	    break;
	    
	    case 2: 
	        // EPPI2
	        if(!(pa_nEPPIcontrol & 0x4) || !(pa_nEPPIcontrol & 0x8)) {
	            // bt656 mode
                /*
	            *pPORTE_FER |= 0x0040;	// enabling FS1 signal. PG6
	            *pPORTE_FER |= 0x0020;	// enabling FS2 signal. PG5
	            *pPORTH_FER |= 0x0004;	// enabling FS3 signal. PH2
	            */
	            *pPORTE_FER |= 0x0080;	// enabling CLK signal. PG7
	            
	            *pPORTG_MUX = ((*pPORTG_MUX & ~0x000000E0) | 0x0000A800);  // MUX 10 for FS1, FS2, CLK
	            *pPORTH_MUX = ((*pPORTH_MUX & ~0x00000030) | 0x00000020);  // MUX 10 for FS3
	        
	            // enable data bits, only 8 bits width
	            if(cDLEN == 0) {
	                // 000 8Bit
	                *pPORTD_FER |= 0x00FF;	    // enabling D0-D7 signal. PD0-PD7
	                *pPORTD_MUX &= ~(0x0000FFFF);  // MUX 00
	                
	            } else {
	                return false;
	            }
	            
    	    } else {
    	        // GP frame sync mode
	            // set data lines
	            switch (cDLEN) {
	                case 0:
	                    // 8Bit
    	                *pPORTD_FER |= 0x00FF;	        // enabling D0-D7 signal. PD0-PD7
    	                *pPORTD_MUX &= ~(0x0000FFFF);   // MUX 00
	                break;    
	                default: 
	                    return false; 
	            }
	            
	            // set frame sync and CLK lines
	            switch(cFS_CFG) {
	                case 2:
	                    *pPORTH_FER |= 0x0004;	// enabling FS3 signal. PH2
	                    *pPORTH_MUX = ((*pPORTH_MUX & ~0x00000030) | 0x00000020);  // MUX 10 for FS3                 
//	                case 2:
    	                *pPORTE_FER |= 0x0020;	// enabling FS2 signal. PG5 
    	                *pPORTE_MUX = ((*pPORTE_MUX & ~0x00000C00) | 0x00000800);  // MUX 10 for FS2	                
	                case 1:
	                    *pPORTE_FER |= 0x0040;	// enabling FS1 signal. PG6
	                    *pPORTE_MUX = ((*pPORTE_MUX & ~0x00003000) | 0x00002000);  // MUX 10 for FS1           
	                break;
	            }
	            
	            // enable Clock
	            *pPORTE_FER |= 0x0080;	// enabling CLK signal. PG7
	            *pPORTE_MUX = ((*pPORTE_MUX & ~0x0000C000) | 0x00008000);  // MUX 10 for CLK 
    	    }
	    break;      
	}
	
	return true;
}

