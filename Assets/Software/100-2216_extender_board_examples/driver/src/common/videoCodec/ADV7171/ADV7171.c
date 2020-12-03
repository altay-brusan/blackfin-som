/**
	@file ADV7171.c
	@ingroup video
	@brief Driver for ADV7171 video Encoder
	
	
	
	BLT_DISCLAIMER(TBD)
	@author Thomas Maier
	@version 1.0
	@date 12.05.2009
	
	@startcond Changelog
	
	@endcond
**/

#include <stdlib.h>
#include <conio.h>
#include <services/services.h>
#include <Environment.h>
#include "ADV7171.h"


static unsigned char g_cADV7171DevicesOpened = 0;


static I2CConfig ADV7171PalBGHID [] = {

    { ADV7171_REG_MODE_REGISTER_0,        0x05 },
    { ADV7171_REG_SUBCARRIER_FREQUENCY_0, 0xcb },
    { ADV7171_REG_SUBCARRIER_FREQUENCY_1, 0x8a },
    { ADV7171_REG_SUBCARRIER_FREQUENCY_2, 0x09 },
    { ADV7171_REG_SUBCARRIER_FREQUENCY_3, 0x2a },
    
};

static I2CConfig ADV7171NTSC [] = {

    { ADV7171_REG_MODE_REGISTER_0, 0x00 }
	
};


static I2CConfig ADV7171Common [] = 
{
    { ADV7171_REG_MODE_REGISTER_1, 0x00 },
    { ADV7171_REG_MODE_REGISTER_2, 0x00 },
    { ADV7171_REG_MODE_REGISTER_3, 0x80 },
    { ADV7171_REG_MODE_REGISTER_4, 0x00 },
}; ///<Setup commands common to all modes.

static I2CConfig ADV717ResetTiming [] =
{
    { ADV7171_REG_TIMING_MODE_0, 0x80 }, //reset timing unit
    { ADV7171_REG_TIMING_MODE_0, 0x08 }, //set timing mode and remove reset
};


// prototype delcaration for private functions --------------------------
T_ERROR_CODE ADV7171HWsetup(T_ADV7171_HANDLE pa_tADV7171Hdl);
// ----------------------------------------------------------------------

/**
    @brief send an I2C config block to the ADV7171
**/
T_ERROR_CODE ADV7171sendConfig(T_ADV7171_HANDLE pa_tHndl, I2CConfig *pa_tConfig, unsigned short pa_nConfigElements) {
    T_ERROR_CODE erResult = ERR_NONE;
    unsigned char i;
  
    if(pa_tHndl) {
        
        T_ADV7171_INST *pInst = (T_ADV7171_INST*) pa_tHndl;  
        //send each element by issuing an I2C write 
        for(i = 0; i < pa_nConfigElements; i++ ) {  
            if(I2CwriteReg(pInst->tI2CHndl, pInst->cI2CHwAddr, pa_tConfig->address, &pa_tConfig->value, 1, NULL) <= 0) {
                return ERR_ADV7171_HW_ERROR;
            }
            pa_tConfig++;
        }    
    } else {
        erResult = ERR_ADV7171_INVALID_HANDLE;
    } 
    return erResult;
}



T_ADV7171_HANDLE ADV7171Open(T_ADV7171_CONFIG *pa_ptConfig, T_ERROR_CODE *pa_ptError, void  *pa_tGeneric) {

 
    if (g_cADV7171DevicesOpened < ADV7171_MAX_NOF_DEVICES) {
        // check if video encoder is present and operational otherwise is useless to continue
        if (pa_ptConfig->cI2CHwAddr == 0) {
            pa_ptConfig->cI2CHwAddr = ADV7171_DEFAULT_HWID;
        }
     
        T_ADV7171_INST * pInst = (T_ADV7171_INST *)malloc(sizeof(T_ADV7171_INST));
        T_ADV7171_HANDLE tHdl  = (T_ADV7171_HANDLE)pInst;

        if(pInst != NULL) {
            int i = 0;
            
            //try to setup chip
            pInst->cI2CHwAddr = pa_ptConfig->cI2CHwAddr;
            pInst->tI2CHndl =pa_ptConfig->tI2CHndl;
            pInst->tResetPin = pa_ptConfig->tResetPin;
            pInst->tBlankPin = pa_ptConfig->tBlankPin;
            pInst->tVideoFormat = pa_ptConfig->tVideoFormat;

            if (ADV7171HWsetup((T_ADV7171_HANDLE) pInst) != ERR_NONE) {
                free(pInst);
                *pa_ptError = ERR_ADV7171_HW_ERROR;
                return  (T_ADV7171_HANDLE) 0;
            } else {
               *pa_ptError = ERR_NONE;
                g_cADV7171DevicesOpened ++;
                return tHdl;
            }
        } else {
            *pa_ptError = ERR_ADV7171_OUT_OF_MEM;
            return (T_ADV7171_HANDLE) 0; 
        }
    } else {
        *pa_ptError = ERR_GENERIC;
        return (T_ADV7171_HANDLE) 0;
    }
}

                
/**
    @brief Closes the driver. Unhooks the ISR and frees all alloctated memory.
**/
T_ERROR_CODE ADV7171Close(T_ADV7171_HANDLE pa_tADV7171Hdl) {
    T_ADV7171_INST *pInst = (T_ADV7171_INST *)pa_tADV7171Hdl;
 
    // free the handle memory
    free (pInst);
    g_cADV7171DevicesOpened --;
    return (T_ADV7171_HANDLE) ERR_NONE;
}


T_ERROR_CODE ADV7171HWsetup(T_ADV7171_HANDLE pa_tADV7171Hdl) {
    
    T_ADV7171_INST *pInst = (T_ADV7171_INST *)pa_tADV7171Hdl;
    
    if ( pInst->tResetPin ) {
        gpio_becomeOutput( pInst->tResetPin );
        // apply reset
        gpio_clear( pInst->tResetPin );
        Sleep (500);
        gpio_set( pInst->tResetPin );
    }
    if( pInst->tBlankPin ) {
        gpio_becomeOutput( pInst->tBlankPin );
        gpio_set( pInst->tBlankPin );
    }
    
    
    ADV7171sendConfig(pa_tADV7171Hdl, ADV7171Common, sizeof(ADV7171Common) / sizeof(I2CConfig) );
   
  
        if (pInst->tVideoFormat == AV_PAL) {
            if (ADV7171sendConfig(pa_tADV7171Hdl, ADV7171PalBGHID, sizeof(ADV7171PalBGHID) / sizeof(I2CConfig) ) != ERR_NONE) {
                return ERR_ADV7171_HW_ERROR;
            }
        } else if (pInst->tVideoFormat == AV_NTSC) {
            if (ADV7171sendConfig(pa_tADV7171Hdl, ADV7171NTSC, sizeof(ADV7171NTSC) / sizeof(I2CConfig) ) != ERR_NONE) {
                return ERR_ADV7171_HW_ERROR;
            }
        } else {
            return ERR_ADV7171_VIDEO_FORMAT_NOT_SUPPORTED;
        }    
    
    
    if(ADV7171sendConfig(pa_tADV7171Hdl, ADV717ResetTiming, sizeof(ADV717ResetTiming) / sizeof(I2CConfig) ) != ERR_NONE) {
        return ERR_ADV7171_HW_ERROR;
    }
    
    return ERR_NONE;
}

/**
    @brief Write a register of the encoder chip
**/
T_ERROR_CODE ADV7171writeReg(T_ADV7171_HANDLE pa_tHndl, unsigned char pa_cRegAdr, unsigned char pa_cRegVal) {
    T_ERROR_CODE erResult = ERR_NONE;
    T_ADV7171_INST *pInst = (T_ADV7171_INST *)pa_tHndl;
    
    if(pInst) {
        if(I2CwriteReg(pInst->tI2CHndl, pInst->cI2CHwAddr, pa_cRegAdr, &pa_cRegVal, 1, NULL) != ERR_NONE) {
                erResult = ERR_ADV7171_HW_ERROR;
        }        
    } else {
        erResult = ERR_ADV7171_INVALID_HANDLE;
    }
    return ERR_NONE;
}

/**
    @brief Read a register of the encoder chip
**/
T_ERROR_CODE ADV7171readReg(T_ADV7171_HANDLE pa_tHndl, unsigned char pa_cRegAdr, unsigned char *pa_cRegVal) {
    T_ERROR_CODE erResult = ERR_NONE;
    T_ADV7171_INST *pInst = (T_ADV7171_INST *)pa_tHndl;
    
    if(pInst) {
        if(I2CreadReg(pInst->tI2CHndl, pInst->cI2CHwAddr, pa_cRegAdr, pa_cRegVal, 1, NULL) != ERR_NONE) {
            erResult = ERR_ADV7171_HW_ERROR;
        }
         
    } else {
        erResult = ERR_ADV7171_INVALID_HANDLE;
    }
    return ERR_NONE;
}
