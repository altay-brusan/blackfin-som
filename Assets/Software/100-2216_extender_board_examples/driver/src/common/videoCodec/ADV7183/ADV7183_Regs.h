/**
 *	@file 		ADV7183_Regs.h
 *
 *  Register names and addresses for ADV7183
 *	@ingroup 	Video
 *	
 *	@brief 		
 *	BLT_DISCLAIMER
 *	
 *	@author 	Thomas Maier
 *	@cond svn
 *	
 *	Information of last commit
 *	$Rev::               $:  Revision of last commit
 *	$Author::            $:  Author of last commit
 *	$Date::              $:  Date of last commit
 *	
 *	@endcond
 **/

  
#ifndef _ADV7183_REG_H_
#define _ADV7183_REG_H_

 #define ADV7183_REG_INPUT_CTRL                     0x00
 #define ADV7183_REG_VIDEO_SELECTION                0x01
 #define ADV7183_REG_OUTPUT_CTRL                    0x03
 #define ADV7183_REG_EXTENDED_OUTPUT_CTRL           0x04
 #define ADV7183_REG_AUTODETECT_ENABLE              0x07
 #define ADV7183_REG_ADI_CONTROL                    0x0E
 #define ADV7183_REG_POWER_MANAGEMENT               0x0F
 #define ADV7183_REG_STATUS1                        0x10
 #define ADV7183_REG_IDENT	                        0x11
 #define ADV7183_REG_STATUS2                        0x12
 #define ADV7183_REG_STATUS3                        0x13
 #define ADV7183_REG_MISC_GAIN_CTRL                 0x2B
 #define ADV7183_REG_ADC_CONTROL                    0x3A
 
 #define ADV7183_REG_LOCKCOUNT                      0x51        
 
                  
 
#endif
