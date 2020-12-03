#include "Environment.h"
#include <stdio.h>
#include <conio.h>
#include <services/services.h>
#include <drivers/adi_dev.h>
#include <cycle_count_bf.h>
#include <stdlib.h>
#include <string.h>
#include "../../../../../driver/src/common/i2c/I2CInt.h"
#include "../../../../../driver/src/common/i2c/I2CMan.h"
#include "../../../../../driver/src/common/videoCodec/ADV7183/ADV7183.h"
#include "../../../../../driver/src/common/videoCodec/ADV7171/ADV7171.h"
#include "../../../../../driver/src/common/mmio/PPI656In/PPI656In.h"
#include "../../../../../driver/src/common/mmio/PPI656Out/PPI656Out.h"
//#include "../../../../../driver/src/common/analog_video/analogVideo.h"
#include "../../../../../driver/src/common/misc/delay.h"
#include "../../../../../driver/src/common/UART/UARTconfig.h"
#include "../../../../../blacksheep/common/systemTick/systemTick.h"
#include <gpio_global.h>



extern T_AV_SOURCE T_EXT_VIDEO_AV_SOURCES_SPEC[];     ///< declared in ext_video_av_def.c
extern unsigned cEXT_VIDEO_AVsourcesNum;              ///< declared in ext_video_av_def.c
extern char testpic_array[];
extern char testpic_array2[];

extern T_AV_STANDARD_DESCR g_tPALstd;             ///< declared in analog_video.c
extern T_AV_STANDARD_DESCR g_tNTSCstd;             ///< declared in analog_video.c

extern unsigned short g_nPPI656OutActivePlane;


T_UART_HANDLE g_hUART = 0;
unsigned short g_nFrameRate = 0;
unsigned short g_nFrameCounter = 0;
unsigned long  g_nPPI656InErrorCount = 0;
unsigned long  g_nPPI656OutErrorCount = 0;
// memory for the adi interrupt manager
#pragma align(4)
static unsigned char s_acIntMgrStorage[(ADI_INT_SECONDARY_MEMORY * 8)];



void I2CCallbackHandler( unsigned short hwid ) {
	char sMessage[100];
	printf ("I2C device found with hardware id 0x%X.\n", hwid);
}


void PPI656CallbackHandler(T_PPI656I_HANDLE hPPI656In) {
    
    char acMessage[80];
    T_PPI656I_INST *tPPI656In = (T_PPI656I_INST *)hPPI656In;

    g_nFrameCounter++;    
}





void SysTickCallback(void) {
    
    char acMessage[80];
    
	g_nFrameRate = g_nFrameCounter;
	
    // print frame rate
    sprintf(acMessage, "Frame rate: %i fps    PPI656In errors: %i   PPI656Out errors: %i\r\n",
                       g_nFrameRate,
                       g_nPPI656InErrorCount,
                       g_nPPI656OutErrorCount);
    printf(acMessage);                       
    //uart_writeString(g_hUART, (unsigned char *)acMessage);	
    
    g_nFrameCounter = 0;
    g_nPPI656InErrorCount = 0;
    g_nPPI656OutErrorCount = 0;
}


void main (void) {
        
    char nI2C;
    T_ERROR_CODE nResult;
    unsigned long nCCLK, nSCLK, nVCO;
    T_I2C_HANDLE hI2C;
    unsigned long nResponseCount;
    T_ADV7183_HANDLE hVideoDecoder;
    T_ADV7171_HANDLE hVideoEncoder;
    unsigned char nCount;
    T_PPI656O_HANDLE hPPI656Out = 0;
    T_PPI656I_HANDLE hPPI656In = 0;
    T_PPI656I_INST *tPPI656In;

    void **pPPI656InFGFunctions = (void **)malloc(sizeof(unsigned long) * FG_MAX_VIDEO_DEVICE_FUNCTIONS);
    
    *pEBIU_AMGCTL |= 1 << 8; //Gives DMA memory access priority over CPU
    
    if (adi_int_Init (s_acIntMgrStorage, sizeof(s_acIntMgrStorage), &nResponseCount, NULL) != ADI_INT_RESULT_SUCCESS) {
        // error initialising interrupt manager, so stop executing
        printf("ERROR Cannot initialize the interrupt manager!\n");
    }
    
	ADI_PWR_COMMAND_PAIR tPowerTable [] = {	       
		{ ADI_PWR_CMD_SET_PROC_VARIANT, 	(void*)PROC_VARIANT },
		{ ADI_PWR_CMD_SET_PACKAGE, 			(void*)PROC_PACKAGE },
		{ ADI_PWR_CMD_SET_VDDEXT, 			(void*)PROC_VDDEXT }, 
		{ ADI_PWR_CMD_SET_CLKIN, 			(void*)PROC_CLOCK_FREQ_IN},	
		{ ADI_PWR_CMD_SET_VDDINT, 			(void*)PROC_VDDINT}, 
		{ ADI_PWR_CMD_END,			        NULL},
	};
	
	nResult = adi_pwr_Init(tPowerTable);
	if (nResult != ADI_DEV_RESULT_SUCCESS) {
		printf("ERROR Cannot initialize the Power Table!\n");
	}
	
#ifdef PROC_CLOCK_USE_PLL_INPUT_DIV
	nResult = adi_pwr_SetFreq(PROC_CLOCK_FREQ_CORE, PROC_CLOCK_FREQ_SYSTEM, ADI_PWR_DF_ON);
#else    
	nResult = adi_pwr_SetFreq(PROC_CLOCK_FREQ_CORE, PROC_CLOCK_FREQ_SYSTEM, ADI_PWR_DF_OFF);
#endif

	adi_pwr_GetFreq(&nCCLK, &nSCLK, &nVCO);
	

	SysTickSetup(nCCLK,
				 1000.0,								// [ms]
				 1,								// 1s
				 (T_SYSTICK_CALLBACK)SysTickCallback,
				 0);
				 
				 
				 
				 
				 
#ifdef _USE_UART_
    // setting up the uart
    UARTs_init ();
    g_hUART = uart_open(UART_NR, nSCLK, UART_RX_BUFFER_SIZE, UART_TX_BUFFER_SIZE, NULL);
    
    if(g_hUART) {
        uart_setMode(g_hUART, UART_BAUDRATE, 0, 8, 1);
        uart_writeString(g_hUART, "\r\n\r\n\r\n\r\n\r\nUART initialised\r\n");
    }
    else {
        printf("ERROR Cannot initialize the UART interface!\n");  
    }
    
#endif

	
	
#ifdef _USE_I2C_INTERFACE_	
	// Initialize the I2C interface
	T_ERROR_CODE tErrorCode = 0;
	nI2C = I2CAddInterface(I2CInt_getFunctions());
	
	if(nI2C < -1) {
		printf("ERROR Cannot initialize the I2C interface!\n");    
	}
	else {
    	T_I2C_INT_CONFIG I2CConf;
        I2CConf.mnBitRate   = I2C_BIT_RATE;
        I2CConf.mnSystemClk = getSystemClockFrequency();

    	hI2C = I2Copen( nI2C, &I2CConf, &tErrorCode, NULL);
	}
	
	I2CScanBus(hI2C, I2CCallbackHandler);
#endif	


#ifdef _USE_AV_ADV7183_        
    // Initialize video decoder ADV7183
    T_ADV7183_CONFIG tADV7183Config = {T_EXT_VIDEO_AV_SOURCES_SPEC, 
                                       cEXT_VIDEO_AVsourcesNum,
                                       AV_ADV7183_OE_PIN,
                                       hI2C,
                                       0};
    hVideoDecoder = ADV7183open(&tADV7183Config, &nResult);
    
    T_ADV7183Inst *tVideoDecoder = (T_ADV7183Inst *)hVideoDecoder;
    
    ADV7183scanAllInputs(hVideoDecoder);
    for(nCount = 0; nCount < tVideoDecoder->cAVSourcesNum ; nCount++) {
        if(tVideoDecoder->patAVsourcesList[nCount].bConnected == true) {
            ADV7183setInput(hVideoDecoder, nCount);
            tVideoDecoder->bLocked = true;
            
            //tVideoDecoder->eCurrentOperation = ADV7183_OPERATION_SCAN_SINGLE_INPUT;
            break;            
        }   
    }

#endif    


#ifdef _USE_AV_ADV7171_	
    // Initialize video encoder ADV7171 
    T_ADV7171_CONFIG tADV7171Config = {NULL,
                                       hI2C,
                                       0, // I2C default address
                                       (T_GPIO_MASK)0,
                                       (T_GPIO_MASK)0,
                                       AV_ADV7171_VIDEO_FORMAT};
    hVideoEncoder = ADV7171Open(&tADV7171Config, &nResult, NULL);

    if(nResult != ERR_NONE) {
        printf("ERROR Cannot initialize the video encoder!\n");
    }
#endif    


    
#ifdef _USE_PPI656OUT_        	
	// initialize PPI656Out
    void** pPPI656OGraficFunctions = (void **)malloc(sizeof(unsigned long) * FG_MAX_VIDEO_DEVICE_FUNCTIONS);
    T_PPI656O_INST *tPPI656Out;
	
	T_PPI656O_CONFIG tPPI656OutConfig = {PPI656OUT_PPI_NR,
	                                     PPI656OUT_PPI_IVG,
	                                     PPI656OUT_ERROR_IVG,
	                                     PPI656OUT_DMA_ERROR_IVG,
	                                     (void *)hVideoEncoder,
	                                     PPI656OUT_OVERWRITE_ACTIVE_PLANE_ALLOWED,
	                                     PPI656OUT_AUTO_UPDATE,
	                                     PPI656OUT_NR_OF_PLANES,
	                                     AV_ADV7171_VIDEO_FORMAT,
	                                     PPI656OUT_MDMA_CHANNEL};
    hPPI656Out = PPI656Oopen(&tPPI656OutConfig, 
                             &nResult, 
                             &pPPI656OGraficFunctions); 

    if(nResult == ERR_NONE) {                             
        tPPI656Out = (T_PPI656O_INST *)hPPI656Out;
        //printf("PPI656Out initialised\n");
    }
    else {
        printf("ERROR: PPI656Out not initialised!\n");
    }
#endif


#ifdef _USE_PPI656IN_

    if(hVideoDecoder) {
        
        T_PPI656I_CONFIG tPPI656InConfig = {PPI656IN_PPI_NR,
                                            PPI656IN_PPI_IVG,
                                            PPI656IN_ERROR_IVG,
                                            PPI656IN_DMA_ERROR_IVG,
                                            PPI656IN_NR_OF_FRAMES,
                                            tVideoDecoder->patAVsourcesList[tVideoDecoder->cCurrentInput].tVideoType,
                                            PPI656I_BUFFER_MODE,
                                            //NULL,
                                            (T_PPI656I_CALLBACK)PPI656CallbackHandler,
                                            NULL,
                                            NULL
                                            };
        hPPI656In = PPI656Iopen(&tPPI656InConfig,
                                &nResult,
                                pPPI656InFGFunctions);

        // check if an error occures
        if(nResult == ERR_NONE) {                             
            tPPI656In = (T_PPI656I_INST *)hPPI656In;
            printf("PPI656In initialised\n");
        }
        else {
            printf("ERROR: PPI656In not initialised!\n");
        }
    }
    else {
        printf("ERROR Cannot initialize the PPI656In!\n");       
    }
#endif


#ifdef _USE_TEST_PICTURES_

    while(1) {
        PPI656OshowFrame((void *)hPPI656Out,
                         (unsigned char *)&testpic_array[0],
                         0,
                         0,
                         720,
                         576);                         
        PPI656OsetActivePlane((void *)hPPI656Out,PPI656OgetNextPlaneNum((void *)hPPI656Out));
        
        bs_sleep(2000);                     
        PPI656OshowFrame((void *)hPPI656Out,
                         (unsigned char *)&testpic_array2[0],
                         0,
                         0,
                         720,
                         576);            
        PPI656OsetActivePlane((void *)hPPI656Out,PPI656OgetNextPlaneNum((void *)hPPI656Out));
        bs_sleep(2000);                                              
    }

#endif
    

#ifdef _USE_TAKE_PICTURE_
    if(hPPI656In) {                            

        // start grabbing
        printf("\n\nStart grabbing...");
        ((FG_FN_START_GRABBING)pPPI656InFGFunctions[FG_START_GRABBING])((void *)hPPI656In);
        
        ifrm_t *tFrame;
        
        while(1) {
            // check if there is a frame avaiable
            if(((FG_FN_FRAME_AVAILABLE)pPPI656InFGFunctions[FG_FRAME_AVAILABLE])((void *)hPPI656In)) {
                // Dequeue the next frame
                tFrame = ((FG_FN_DEQUEUE_FRAME)pPPI656InFGFunctions[FG_DEQUEUE_FRAME])((void *)hPPI656In);
                ((FG_FN_START_GRABBING)pPPI656InFGFunctions[FG_STOP_GRABBING])((void *)hPPI656In);
                break;
            }
        }
        printf("DONE\nYou can checkout the image with VisualDSP++ Image Viewer.\nUse following settings:\n  - Start address:0x%X\n  - Memory stride: 1\n  - Pixel format: UYVY (4:2:2)\n  - Width: 720\n  - Height: 576\n",
                tFrame->pucFramePtr);       
    }
#endif


#ifdef _USE_ALIVE_LED_	
    gpio_becomeOutput(ALIVE_LED_PIN);
    while(1) {
        unsigned int nWait;
        gpio_toggle(ALIVE_LED_PIN);
        for (nWait=0; nWait<0x2fffff; nWait++);
    }
#endif    
}
