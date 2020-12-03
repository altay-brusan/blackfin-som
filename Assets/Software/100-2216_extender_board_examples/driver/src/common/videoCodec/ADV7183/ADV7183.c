/**
*	@file ADV7183.c
*	
*	@brief Driver for Analog Devices ADV7183
*	
*	BLT_DISCLAIMER
*	
*	@author Thomas Maier, Walter Craffonara
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <services/services.h>
#ifdef _USE_VDK_
    #include <vdk.h>
#endif    
#include "ADV7183.h"
#include "../../../../../blacksheep/common/framegrabber/frameGrabber.h"
#include "../../GPIOconfig.h"

extern char g_acAVstrings[8][16];                ///< declared in analog_video.c
extern T_AV_STANDARD_DESCR g_tPALstd;             ///< declared in analog_video.c
extern T_AV_STANDARD_DESCR g_tNTSCstd;             ///< declared in analog_video.c


T_ADV7183_HANDLE g_atADV7183devices[ADV7183_MAX_NUM_DEVICES]; 
unsigned char g_acADV7183numDevices = 0;

#ifdef _USE_VDK_
    VDK_ThreadID g_tADV7183ThreadId;
#endif
	
//protoytpes
static T_ERROR_CODE ADV7183hardwareSetup(T_ADV7183_HANDLE pa_tHndl);
void ADV7183IntHandler(void *pa_pClientArg);


void ADV7183startGrabbing(T_ADV7183_HANDLE pa_tHndl) {
    
}


//known decoder Revisions

T_ADV7183_DECODER_TYPE g_atTypes[] = {
    {
        0x0D,
        ADV7183A_ES1,
        "ADV7183A-ES1",
    },
    {
        0x0E,
        ADV7183A_ES2,
        "ADV7183A-ES2",
    },
    {
        0x0F,
        ADV7183A_FT,
        "ADV7183A-FT",
    },
    {
        0x10,
        ADV7183A,
        "ADV7183A-FT",
    },
    {
        0x11,
        ADV7183A,
        "ADV7183A",
    },
    
    {
        0x13,
        ADV7183B,
        "ADV7183B",
    
    },
};
     

//////////////////////CONTROL//////////////////////////
/*
static I2CConfig adv7183_common [] = 
{
}; ///<Setup common to all types and modes
*/

static I2CConfig adv7183_typeA [] = 
{
    { ADV7183_REG_EXTENDED_OUTPUT_CTRL  , 0x75 }, 
    { ADV7183_REG_ADI_CONTROL           , 0x48 }, 
    { ADV7183_REG_VIDEO_SELECTION       , 0x88 },//Turn off HSync processor (SECAM only).
    { ADV7183_REG_MISC_GAIN_CTRL        , 0xE2 },//AGC tweak.
    //{ 0x3A, 0x12}, //Power down ADC 2.
    { ADV7183_REG_LOCKCOUNT             , 0x24}, //Turn off FSC detect for IN LOCK status.
    { 0xD2, 0x01}, //AGC tweak.
    { 0xD3, 0x01}, //AGC tweak.
    { 0xDB, 0x9B}, //AGC tweak.
    { ADV7183_REG_ADI_CONTROL           , 0x85}, //ADI recommended programming sequence. This sequence must be followed exactly whensetting up the decoder.
    { 0xB5, 0x8B}, //Recommended setting.
    { 0xD4, 0xFB}, //Recommended setting.
    { 0xD6, 0x6D}, //Recommended setting.
    { 0xE2, 0xAF}, //Recommended setting.
    { 0xE3, 0x00}, //Recommended setting.
    { 0xE4, 0xB5}, //Recommended setting.
    { 0xE8, 0xF3}, //Recommended setting.
    { ADV7183_REG_ADI_CONTROL           , 0x05}, //Recommended setting.
}; ///<Specific settings for type A chip

static I2CConfig adv7183_typeB [] = 
{
    { 0x1d, 0x80 }, //CLOCK three-stated
    { 0xf4, 0x3c }, //DATA and CLOCK drive strength to high
}; ///<Specific settings for type B chip



static I2CConfig adv7183_pwr_CVBS [] =
{
    { ADV7183_REG_ADC_CONTROL, 0x06 },
}; ///<Power down mode for CVBS input

static I2CConfig adv7183_pwr_YC [] =
{
    { ADV7183_REG_ADC_CONTROL, 0x02 },
}; ///<Power down mode for S-Video input

static I2CConfig adv7183_pwr_YPrPb [] =
{
    { ADV7183_REG_ADC_CONTROL, 0x00 },
}; ///<All ADC's powered up

static I2CConfig adv7183_pwr_off [] =
{
    { ADV7183_REG_ADC_CONTROL, 0x0e },
}; ///<All ADC's powered down



/**
*	@brief Send an I2C config block to the ADV7183
*	
*   @param pa_tHndl Handle of driver instance
*   @param pa_tConfig Configuration to be written
*   @param pa_nConfigElements Number of elements to be configured
*	@return ERR_ADV7183_INVALID_HANDLE: Invalid handle
*           ERR_ADV7183_I2C_ERROR: if there was an error writeing the data on I2C
*           ERR_NONE: else
**/
T_ERROR_CODE ADV7183sendConfig(T_ADV7183_HANDLE pa_tHndl, I2CConfig *pa_tConfig, unsigned short pa_nConfigElements) {
    T_ERROR_CODE erResult = ERR_NONE;
    unsigned char i;
    
   
    if(pa_tHndl) {
        
        T_ADV7183Inst *pInst = (T_ADV7183Inst*) pa_tHndl;
       
           
        //send each element by issueing an I2C write 
        for(i = 0; i < pa_nConfigElements; i++ ) {
            if(I2CwriteReg(pInst->tI2CHndl, pInst->cI2Cadr, pa_tConfig->address, &pa_tConfig->value, 1, NULL) != ERR_NONE) {
                return ERR_ADV7183_I2C_ERROR;
            }   
            
            pa_tConfig++;
    
        }
        
    } else {
        erResult = ERR_ADV7183_INVALID_HANDLE;
    } 
    
    return erResult;
}



/**
*	@brief Setup for the device
*   
*	@return ERR_NONE
**/
T_ERROR_CODE ADV7183setup() {
    T_ERROR_CODE erResult = ERR_NONE;
    // nothing to do
    return erResult;
}



/**
*	@brief Opens a new device
*	
*   @param pa_tConfig Config structur for the device
*   @param pa_ptError Pointer to an error instance
*   
*	@return Handle to the device or 0 if device cannot be open
**/
T_ADV7183_HANDLE ADV7183open(T_ADV7183_CONFIG *pa_tConfig,
                            T_ERROR_CODE *pa_ptError) {
                                
    T_ADV7183_HANDLE tHndl = 0;
    unsigned char cDecoder = UNKNOWN_DECODER; 
    unsigned char i;
    unsigned char cBytesPerPixel = 2;   
    
    *pa_ptError = ERR_NONE;
    
    // if not set by user use default hwid
    if(pa_tConfig->cI2Cadr == 0) {
        pa_tConfig->cI2Cadr = ADV7183_DEFAULT_HWID;
    }
 
   
    I2CreadReg(pa_tConfig->tI2CHndl, pa_tConfig->cI2Cadr, ADV7183_REG_IDENT, &cDecoder, 1, NULL) ;
    
    //query for type by parsing ID register
    for(i = 0; i < (sizeof(g_atTypes) / sizeof(T_ADV7183_DECODER_TYPE)) ; i++) {
        if(g_atTypes[i].cID == cDecoder) {
            cDecoder = g_atTypes[i].eType;
            #if ADV7183_DEBUG_LVL > 0
                printf("(ADV7183open): Detected %s at I2C address 0x%x.\n", g_atTypes[i].acName, pa_tConfig->cI2Cadr);
            #endif 
            
            break;
        }
     }
    
    if(cDecoder != UNKNOWN_DECODER) {
       
        T_ADV7183Inst *pInst = malloc ( sizeof(T_ADV7183Inst));

        if(pInst) {
            
            memset(pInst, sizeof(T_ADV7183Inst), 0); // clear instance
            
            pInst->tI2CHndl = pa_tConfig->tI2CHndl;                     
            pInst->cI2Cadr = pa_tConfig->cI2Cadr;
            pInst->eSubType = cDecoder;
           
            pInst->bLocked = false;
            
            pInst->fnHwDevCloseDev = (ADV7183_FN_CLOSE_DEVICE)ADV7183close;
            
             //reset the device
            ADV7183reset((T_ADV7183_HANDLE) pInst);
            Sleep(ADV7183_RESET_WAIT_TIME);
            
            if((pa_tConfig->patAVsourcesList != 0) && (pa_tConfig->cAVSourcesNum != 0)) {
                
                //copy sources
                pInst->cAVSourcesNum = pa_tConfig->cAVSourcesNum;
                pInst->patAVsourcesList = malloc(pa_tConfig->cAVSourcesNum * sizeof(T_AV_SOURCE));
                memcpy(pInst->patAVsourcesList, pa_tConfig->patAVsourcesList, pa_tConfig->cAVSourcesNum * sizeof(T_AV_SOURCE));
                           
                if(pInst->patAVsourcesList) {
                    //initialize decoder
                    ADV7183hardwareSetup((T_ADV7183_HANDLE) pInst);      
                    unsigned short nXsize,nYsize;
               
                    //output enable pin
                    if(pa_tConfig->tOEpin) {
                        gpio_becomeOutput( pa_tConfig->tOEpin );
                        gpio_clear( pa_tConfig->tOEpin );
                    }
                    pInst->tOEpin = pa_tConfig->tOEpin;
                    
                   
                    //save into device list
                    g_atADV7183devices[g_acADV7183numDevices] = (T_ADV7183_HANDLE) pInst;
                    
                    //fill framegrabber functionlist
                   
		
                    //start thread which scans for input and monitors lock status
#ifdef _USE_VDK_    
#if ADV7183_DEBUG_LVL > 0
                        printf("(ADV7183open): Starting Control Thread..");
#endif
                	VDK_ThreadCreationBlock tThreadDescr;
                	tThreadDescr.template_id = kADV7183controlThread;
                	tThreadDescr.thread_stack_size = 0;
                	tThreadDescr.thread_priority = (VDK_Priority)0;
                	tThreadDescr.user_data_ptr = (void *)pInst;
                	tThreadDescr.pTemplate = 0;
                	pInst->tThreadID = VDK_CreateThreadEx( &tThreadDescr );
                	//setting the io context
                	io_setThreadIO(pInst->tThreadID, 0, stdout);
                	//setting the global thread descriptor
                	char acThreadName[11];
                	sprintf(acThreadName, "ADV7183_%ud", g_acADV7183numDevices);
                
                	thread_setDesc (pInst->tThreadID, (VDK_Priority)5, acThreadName, kADV7183controlThread, 0, 0, 0, stdout);	
#if ADV7183_DEBUG_LVL > 0
                        printf("done\n");
#endif
                    g_acADV7183numDevices++;
                    
#endif // _USE_VDK_
                    tHndl =  (T_ADV7183_HANDLE) pInst;
                } else {
                    free(pInst->patAVsourcesList);
                    free(pInst);   
                    *pa_ptError = ERR_ADV7183_OUT_OF_MEM;
                    tHndl = 0;   
                }
                   
              } else {
                free(pInst); //free instance
                *pa_ptError = ERR_ADV7183_NO_SOURCES;
                tHndl = 0;
            }
     
        } else {
            *pa_ptError = ERR_ADV7183_OUT_OF_MEM;
            tHndl = 0;
        }
        
    } else {
        *pa_ptError = ERR_ADV7183_UNKNOWN_DECODER;
        tHndl = 0;
    }
    
    return tHndl;
} 



/**
*	@brief Destroy instance and free resources
*	
    @param pa_tHndl Handle of driver instance
*	@return ERR_ADV7183_INVALID_HANDLE: Invalid handle
*           ERR_NONE: else
**/
T_ERROR_CODE ADV7183close(T_ADV7183_HANDLE pa_tHndl) {
    
    T_ERROR_CODE erResult = ERR_NONE;
    if(pa_tHndl) {
        
        T_ADV7183Inst *pInst = (T_ADV7183Inst*) pa_tHndl;
    #ifdef _USE_VDK_    
         //destroy threads
        io_removeThreadIO(pInst->tThreadID);
        thread_removeDesc(pInst->tThreadID);
        VDK_DestroyThread(pInst->tThreadID,true); 
    #endif
        
        
        if(pInst->patAVsourcesList) {
            free(pInst->patAVsourcesList);
        }
        free(pInst); // free instance
        
    } else {
        erResult = ERR_ADV7183_INVALID_HANDLE;
    } 
    return erResult;
}



/**
*	@brief Cleans up
*	
    @param pa_tHndl Handle of driver instance
*	@return none
**/
void ADV7183cleanup(T_ADV7183_HANDLE pa_tHndl) {   
   // nothing to do
}



/**
*	@brief Hardware setup for the ADV7183
*	
*	@param  pa_tHndl Handle to the device
*	@return ERR_ADV7183_INVALID_HANDLE: Invalid handle
*           ERR_NONE: else
**/
T_ERROR_CODE ADV7183hardwareSetup(T_ADV7183_HANDLE pa_tHndl) {

    T_ERROR_CODE erResult = ERR_NONE;
    
    if(pa_tHndl) {
        T_ADV7183Inst* pInst = (T_ADV7183Inst*) pa_tHndl;
        
         //common setup
         
         switch(pInst->eSubType) {
            case ADV7183A: {
                ADV7183sendConfig(pa_tHndl, adv7183_typeA, sizeof(adv7183_typeA) / sizeof(I2CConfig));
                 
                break;
            }
            case ADV7183B: { 
                ADV7183sendConfig(pa_tHndl, adv7183_typeA, sizeof(adv7183_typeB) / sizeof(I2CConfig));
                break;
            }
         }

    } else {
        erResult = ERR_ADV7183_INVALID_HANDLE;
    }
    return erResult ;
}



/**
*	@brief Check if decoder is locked in at video source
*	
*	@param  pa_tHndl Handle to the device
*	@return true: if locked
*           false: else
**/
bool ADV7183isLocked(T_ADV7183_HANDLE pa_tHndl) {
    bool bIsLocked = false;
    
    if(pa_tHndl) {
        T_ADV7183Inst* pInst = (T_ADV7183Inst*) pa_tHndl;
        
        unsigned char cData;
     
        I2CreadReg(pInst->tI2CHndl, pInst->cI2Cadr, ADV7183_REG_STATUS1, &cData, 1, NULL);
  
        #if ADV7183_DEBUG_LVL > 0
            printf("(ADV7183isLocked): Status: 0x%x\n", cData);
        #endif
        if(cData & 0x01) {            
            bIsLocked = true;
        }
    }
    
    return bIsLocked;
}



/**
*	@brief Check if current video source provides interlaced or progressive data 
*	
*	@param  pa_tHndl Handle to the device
*	@return true: if video source provides intelaced
*           false: if video source providec progressive
**/
bool ADV7183isInterlaced(T_ADV7183_HANDLE pa_tHndl) {
    bool bIsInterlaced = false;
    
    if(pa_tHndl) {
        T_ADV7183Inst* pInst = (T_ADV7183Inst*) pa_tHndl;
        
        unsigned char cData;
        
        I2CreadReg(pInst->tI2CHndl, pInst->cI2Cadr, ADV7183_REG_STATUS3, &cData, 1, NULL);

        if (cData & ( 1 << 6 )) {
            #if ADV7183_DEBUG_LVL > 0
                printf("(ADV7183isInterlaced):Interlaced Video\n");
            #endif
            
            bIsInterlaced = true;
           
        } else {
            #if ADV7183_DEBUG_LVL > 0
                printf("(ADV7183isInterlaced):Progressive Video\n");
            #endif
            bIsInterlaced = false;
        }
    }
    
    return bIsInterlaced;
}



/**
*	@brief Scan all inputs and populates sources section
*
*	@param  pa_tHndl Handle to the device
*	@return ERR_ADV7183_INVALID_HANDLE: Invalid handle
*           ERR_NONE: else
**/
T_ERROR_CODE ADV7183scanAllInputs(T_ADV7183_HANDLE pa_tHndl) {
    T_ERROR_CODE erResult = ERR_NONE;
    unsigned char i;
        
    if(pa_tHndl) {

        T_ADV7183Inst* pInst = (T_ADV7183Inst*) pa_tHndl;
          //cycle trough sources
        for(i = 0 ; i < pInst->cAVSourcesNum ; i++) {
            
            unsigned char cData = pInst->patAVsourcesList[i].acOptions[0];;
            
            //switch input
            I2CwriteReg(pInst->tI2CHndl, pInst->cI2Cadr, ADV7183_REG_INPUT_CTRL, &cData, 1, NULL); 
            Sleep(ADV7183_INPUT_SETUP_TIME);
            //check for lock
            if(ADV7183isLocked(pa_tHndl)) {
                #if ADV7183_DEBUG_LVL > 0
                    printf("(ADV7183scanAllInputs): Detected Signal at %s.\n", pInst->patAVsourcesList[i].acDescriptor);
                #endif   
                pInst->patAVsourcesList[i].bConnected = true;    
                // query Input format 
                  
                switch(ADV7183queryInputVideoStd(pa_tHndl)) {
                    case ADV7183_PAL_BGHID: {
                        pInst->patAVsourcesList[i].tVideoType = AV_PAL_BGH;
                        break;
                    }
                    case ADV7183_NTSC_443: {
                        pInst->patAVsourcesList[i].tVideoType = AV_NTSC;
                        break;
                    }
                    case ADV7183_NTSC_MJ: {
                        pInst->patAVsourcesList[i].tVideoType = AV_NTSC;
                        break;
                    }
                    case ADV7183_PAL_M: {
                        pInst->patAVsourcesList[i].tVideoType = AV_PAL_N;
                        break;
                    }
                    case ADV7183_PAL_60: {
                        pInst->patAVsourcesList[i].tVideoType = AV_PAL_60;
                        break;
                    }
                    case ADV7183_SECAM: {
                        pInst->patAVsourcesList[i].tVideoType = AV_SECAM;
                        break;
                    } 
                    default: {
                        pInst->patAVsourcesList[i].tVideoType = AV_UNKNOWN;
                        break;
                    }
                } 
                
                //check interlaced/progressive video
                pInst->patAVsourcesList[i].bInterlaced = ADV7183isInterlaced(pa_tHndl);
                
            } else {
                #if ADV7183_DEBUG_LVL > 0
                    printf("(ADV7183scanAllInputs): No Signal at %s.\n", pInst->patAVsourcesList[i].acDescriptor);
                #endif   
                pInst->patAVsourcesList[i].bConnected = false;  
            }

            Sleep(250);

        }         
        
    } else {
        erResult = ERR_ADV7183_INVALID_HANDLE;
    }
    
    return erResult;
    
}



/**
*	@brief Set input to specific channel
*	
*	@param  pa_tHndl Handle to the device
*   @param  pa_cInputNumber offset in patAVsourcesList use 255 to enable autoscan
*	@return ERR_ADV7183_INVALID_HANDLE: Invalid handle
*           ERR_NONE: else
**/
T_ERROR_CODE ADV7183setInput(T_ADV7183_HANDLE pa_tHndl, unsigned char pa_cInputNumber) {
    
    T_ERROR_CODE erResult = ERR_NONE;
    
    if(pa_tHndl) {
        T_ADV7183Inst* pInst = (T_ADV7183Inst*) pa_tHndl;   
        
        if(pa_cInputNumber != ADV7183_AUTOSCAN) {

            unsigned char cData = pInst->patAVsourcesList[pa_cInputNumber].acOptions[0]; 
            
            I2CwriteReg(pInst->tI2CHndl, pInst->cI2Cadr, ADV7183_REG_INPUT_CTRL, &cData, 1, NULL);
            Sleep(ADV7183_INPUT_SETUP_TIME);
            if(ADV7183isLocked(pa_tHndl)) {
            
                pInst->bLocked = true;
            #if ADV7183_DEBUG_LVL > 0
                printf("(ADV7183setInput):Source %s -> Locked\n", pInst->patAVsourcesList[pa_cInputNumber].acDescriptor);
            #endif             
            } else {
                pInst->bLocked = false;
            #if ADV7183_DEBUG_LVL > 0
                printf("(ADV7183setInput):Source %s -> No Lock!\n", pInst->patAVsourcesList[pa_cInputNumber].acDescriptor);
            #endif 
            }
            
            //set the current input
            pInst->cCurrentInput = pa_cInputNumber;
            //pInst->eCurrentOperation = ADV7183_OPERATION_SCAN_SINGLE_INPUT;
        } else {
            //enable autoscan via control thread
            pInst->eCurrentOperation = ADV7183_OPERATION_SCAN_ALL_INPUTS;
        }
        
        erResult = ERR_NONE;
    } else {
        erResult = ERR_ADV7183_INVALID_HANDLE;
    }
    
    
    return erResult;
}



/**
*	@brief Sets the operation mode
*	
*	@param  pa_tHndl Handle to the device
*   @param  pa_eMode New operation mode
*	@return ERR_ADV7183_INVALID_HANDLE: Invalid handle
*           ERR_NONE: else
**/
T_ERROR_CODE ADV7183setOperationMode(T_ADV7183_HANDLE pa_tHndl, E_ADV7183_OPERATION pa_eMode) {
    T_ERROR_CODE erResult = ERR_NONE;
    
    if(pa_tHndl) {
        T_ADV7183Inst* pInst = (T_ADV7183Inst*) pa_tHndl;   
        pInst->eCurrentOperation = pa_eMode;
        erResult = ERR_NONE;
    } else {
        erResult = ERR_ADV7183_INVALID_HANDLE;
    }
    
    return erResult;
}



/**
*	@brief Returs the detected video format from autodetect mode
*
*	@param  pa_tHndl Handle to the device
*	@return the bits [6:4] from the STATUS1 register
**/
unsigned char ADV7183queryInputVideoStd(T_ADV7183_HANDLE pa_tHndl) {
    unsigned char cInput = ADV7183_AUTODETECT;
    
    if(pa_tHndl) {
        T_ADV7183Inst* pInst = (T_ADV7183Inst*) pa_tHndl;
        unsigned char cData; 
        I2CreadReg(pInst->tI2CHndl, pInst->cI2Cadr, ADV7183_REG_STATUS1, &cData, 1, NULL);   
        cInput = (cData & 0x70 );
    } else {
        
    }
    
    return cInput;
}



/**
*	@brief Reset decoder trough an I²C command
*	
*	@param  pa_tHndl Handle to the device
*	@return ERR_ADV7183_INVALID_HANDLE: Invalid handle
*           ERR_NONE: else
**/
T_ERROR_CODE ADV7183reset(T_ADV7183_HANDLE pa_tHndl) {
    T_ERROR_CODE erResult = ERR_NONE;
    
    if(pa_tHndl) {
        T_ADV7183Inst* pInst = (T_ADV7183Inst*) pa_tHndl;
        unsigned char cValue = 0x01;
        I2CwriteReg(pInst->tI2CHndl, pInst->cI2Cadr, ADV7183_REG_POWER_MANAGEMENT, &cValue, 1, NULL);
            
    } else {
        erResult = ERR_ADV7183_INVALID_HANDLE;
    }
    return erResult;
}



/**
*	@brief Prints sources list and data
*	
*	@param  pa_tHndl Handle to the device
*	@return none
**/
void ADV7183printSources(T_ADV7183_HANDLE pa_tHndl) {
    if(pa_tHndl) {
        unsigned char i,cDecoderType;
        T_ADV7183Inst* pInst = (T_ADV7183Inst*) pa_tHndl;
        
        for(cDecoderType = 0; cDecoderType < sizeof(g_atTypes) / sizeof(T_ADV7183_DECODER_TYPE) ; cDecoderType++) {
            if(pInst->eSubType == g_atTypes[cDecoderType].eType) {
                break;
            }
        }
        
        printf("ADV7183: Type:%s %d Sources defined:\n", g_atTypes[cDecoderType].acName, pInst->cAVSourcesNum);
        printf("------------------------------------\n");
        printf("[Number] [Connected?] [Name] [Type] [ Format]\n");
        for(i = 0; i < pInst->cAVSourcesNum; i++) {
            if(pInst->patAVsourcesList[i].bConnected) {
                printf("[%d]: [Connected] [%s] [%s ", i, 
                                                           pInst->patAVsourcesList[i].acDescriptor, 
                                                           g_acAVstrings[pInst->patAVsourcesList[i].tVideoType]);
                if( pInst->patAVsourcesList[i].bInterlaced) {
                    printf("Interlaced]");
                } else {
                    printf("progressive]");
                }
            } else {
                printf("[%d]: [NotConnected] [%s]", i, pInst->patAVsourcesList[i].acDescriptor);
            }
            
            printf("\n");
        }
        
    }
}



/**
*	@brief Writes a register on the device
*	
*	@param  pa_tHndl Handle to the device
*   @param  pa_cRegAdr Register address
*   @param  pa_cRegVal New value of the register
*
*	@return ERR_ADV7183_I2C_ERROR: Error in reading value from I2C
*           ERR_ADV7183_INVALID_HANDLE: Invalid handle
*           ERR_NONE: else
**/
T_ERROR_CODE ADV7183writeReg(T_ADV7183_HANDLE pa_tHndl, unsigned char pa_cRegAdr, unsigned char pa_cRegVal) {
    T_ERROR_CODE erResult = ERR_NONE;
    T_ADV7183Inst *pInst = (T_ADV7183Inst *)pa_tHndl;
    
    if(pInst) {
        if(I2CwriteReg(pInst->tI2CHndl, pInst->cI2Cadr, pa_cRegAdr, &pa_cRegVal, 1, NULL) != ERR_NONE) {
                erResult = ERR_ADV7183_I2C_ERROR;
        }        
    } else {
        erResult = ERR_ADV7183_INVALID_HANDLE;
    }
    return ERR_NONE;
}



/**
*	@brief Reads a register from the device
*	
*	@param  pa_tHndl Handle to the device
*   @param  pa_cRegAdr Register address
*   @param  pa_cRegVal Poiter to a variable to be written
*
*	@return ERR_ADV7183_I2C_ERROR: Error in reading value from I2C
*           ERR_ADV7183_INVALID_HANDLE: Invalid handle
*           ERR_NONE: else
*	@pre <text>
*	@post  <text>
**/T_ERROR_CODE ADV7183readReg(T_ADV7183_HANDLE pa_tHndl, unsigned char pa_cRegAdr, unsigned char *pa_cRegVal) {
    T_ERROR_CODE erResult = ERR_NONE;
    T_ADV7183Inst *pInst = (T_ADV7183Inst *)pa_tHndl;
    
    if(pInst) {
        if(I2CreadReg(pInst->tI2CHndl, pInst->cI2Cadr, pa_cRegAdr, pa_cRegVal, 1, NULL) != ERR_NONE) {
            erResult = ERR_ADV7183_I2C_ERROR;
        }
         
    } else {
        erResult = ERR_ADV7183_INVALID_HANDLE;
    }
    return ERR_NONE;
}
