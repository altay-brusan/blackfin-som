#include "Environment.h"
#include <stdio.h>
#include <conio.h>
#include <services/services.h>
#include <drivers/adi_dev.h>
#include <cycle_count_bf.h> 
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "../../../../../driver/src/common/i2c/I2CInt.h"
#include "../../../../../driver/src/common/i2c/I2CMan.h"
#include "../../../../../driver/src/common/videoCodec/ADV7183/ADV7183.h"
#include "../../../../../driver/src/common/videoCodec/ADV7171/ADV7171.h"
#include "../../../../../driver/src/common/mmio/EPPI656In/EPPI656In.h"
#include "../../../../../driver/src/common/mmio/EPPI656Out/EPPI656Out.h"
#include "../../../../../driver/src/common/pwrmngt/clockquery.h"
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


void EPPI656CallbackHandler(T_EPPI656I_HANDLE hEPPI656In) {
    
    char acMessage[80];
    T_EPPI656I_INST *tEPPI656In = (T_EPPI656I_INST *)hEPPI656In;

    g_nFrameCounter++;    
}



void SysTickCallback(void) {
    
    char acMessage[80];
    
	g_nFrameRate = g_nFrameCounter;
	
	
    // print frame rate
    printf("Frame rate: %i fps    PPI656In errors: %i   PPI656Out errors: %i\r\n",
                       g_nFrameRate,
                       g_nPPI656InErrorCount,
                       g_nPPI656OutErrorCount);
    /*sprintf(acMessage, "Frame rate: %i fps    PPI656In errors: %i   PPI656Out errors: %i\r\n",
                       g_nFrameRate,
                       g_nPPI656InErrorCount,
                       g_nPPI656OutErrorCount);
    uart_writeString(g_hUART, (unsigned char *)acMessage);	*/
    
    g_nFrameCounter = 0;
    g_nPPI656InErrorCount = 0;
    g_nPPI656OutErrorCount = 0;
}


void main (void) {
        
    char nI2C;
    T_ERROR_CODE nResult;
    T_I2C_HANDLE hI2C;
    unsigned long nResponseCount;
    T_ADV7183_HANDLE hVideoDecoder;
    T_ADV7171_HANDLE hVideoEncoder;
    unsigned char nCount;
    T_EPPI656O_HANDLE hEPPI656Out = 0;
    T_EPPI656I_HANDLE hEPPI656In = 0;
    T_EPPI656I_INST *tEPPI656In;
    unsigned short msel, ssel, imask;
    
    void **pEPPI656InFGFunctions = (void **)malloc(sizeof(unsigned long) * FG_MAX_VIDEO_DEVICE_FUNCTIONS);
       
    if (adi_int_Init (s_acIntMgrStorage, sizeof(s_acIntMgrStorage), &nResponseCount, NULL) != ADI_INT_RESULT_SUCCESS) {
        // error initialising interrupt manager, so stop executing
        printf("ERROR Cannot initialize the interrupt manager!\n");
    }
    
	ADI_PWR_COMMAND_PAIR tPowerTable [] = {	       
		{ ADI_PWR_CMD_SET_PROC_VARIANT, 	(void*)PROC_VARIANT },
		{ ADI_PWR_CMD_SET_PACKAGE, 			(void*)PROC_PACKAGE },
		{ ADI_PWR_CMD_SET_VDDEXT, 			(void*)PROC_VDDEXT }, 
		{ ADI_PWR_CMD_SET_CLKIN, 			(void*)PROC_CLOCK_FREQ_IN},	
		{ ADI_PWR_CMD_END,			        NULL},
	};
	
	// initialize power management
	adi_pwr_Init(tPowerTable);
	
    
	setClockIn(PROC_CLOCK_FREQ_IN*1000000);
	
	msel = (unsigned short)ceil(((float)(PROC_CLOCK_FREQ_CORE) / (float)(PROC_CLOCK_FREQ_IN)));
	ssel = (unsigned short)ceil(((float)(PROC_CLOCK_FREQ_CORE) / (float)(PROC_CLOCK_FREQ_SYSTEM)));
	
	asm("cli r0; \
		%0 = r0;"
		:"=d"(imask)
		:
		:"r0"); 			//IMASK bitpattern saved in "imask".
		
//	*pPLL_LOCKCNT = 0x400;
	
	*pSIC_IWR0 |= 0x1;		//Enable PLL-interrupt to wakeup processor from idle-mode.

	*pPLL_DIV = ssel;
	
	msel = msel << 9;
	*pPLL_CTL = msel;
	
							//PLL programming sequence.
	asm ("r0 = %0; \
		idle; \
		sti r0;"
		:
		:"d"(imask)
		:"r0"); 

		
	*pEBIU_MODE |= AMI_EBIU_MODE_VALUE;
	*pEBIU_AMBCTL0 = AMI_EBIU_AMBCTL0_VALUE;
	*pEBIU_AMBCTL1 = AMI_EBIU_AMBCTL1_VALUE;
	*pEBIU_AMGCTL = AMI_EBIU_AMGCTL_VALUE;  

	// set port H MUX to configure PH8-PH13 as 1st Function (MUX = 00)
	// (bits 16-27 == 0) - Address signals A4-A9
	*pPORTH_FER |= 0x3F00;
	*pPORTH_MUX &= 0xF000FFFF;
	//*pPORTH_DIR_SET |= 0x3F00;

	// configure PI0-PI15 as A10-A21 respectively
	*pPORTI_FER = 0xFFFF;
	*pPORTI_MUX = 0x0;
	//*pPORTI_DIR_SET = 0xFFFF;
	
	*pPORTJ_FER |= 0x0001;
	*pPORTJ_MUX &= 0xFFFFFFFC;
	//*pPORTJ_DIR_SET |= 0x0001;    

		/*	
	SysTickSetup(getCoreClockFrequency(),
				 1000.0,								// [ms]
				 1,								// 1s
				 (T_SYSTICK_CALLBACK)SysTickCallback,
				 0);
		*/		 
				 
				 
				 
/*				 
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

	*/
	
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


#ifdef _USE_EPPI656IN_

    if(hVideoDecoder) {
        
        T_EPPI656I_CONFIG tEPPI656InConfig = {EPPI656IN_EPPI_NR,
                                            EPPI656IN_EPPI_IVG,
                                            EPPI656IN_ERROR_IVG,
                                            EPPI656IN_DMA_ERROR_IVG,
                                            EPPI656IN_NR_OF_FRAMES,
                                            tVideoDecoder->patAVsourcesList[tVideoDecoder->cCurrentInput].tVideoType,
                                            EPPI656I_BUFFER_MODE,
                                            //NULL,
                                            (T_EPPI656I_CALLBACK)EPPI656CallbackHandler,
                                            NULL,
                                            NULL,
                                            NULL};
        hEPPI656In = EPPI656Iopen(&tEPPI656InConfig,
                                &nResult,
                                pEPPI656InFGFunctions);

        // check if an error occures
        if(nResult == ERR_NONE) {                             
            tEPPI656In = (T_EPPI656I_INST *)hEPPI656In;
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

    
#ifdef _USE_EPPI656OUT_        	
	// initialize PPI656Out
    void** pEPPI656OGraficFunctions;
    T_EPPI656O_INST *tEPPI656Out;
	
	T_EPPI656O_CONFIG tEPPI656OutConfig;
	
    tEPPI656OutConfig.cEPPInr = EPPI656OUT_EPPI_NR;
    tEPPI656OutConfig.nEPPIIVG = EPPI656OUT_EPPI_IVG;
    tEPPI656OutConfig.nErrorIVG = EPPI656OUT_ERROR_IVG;
    tEPPI656OutConfig.nDMAErrorIVG = EPPI656OUT_DMA_ERROR_IVG;
    tEPPI656OutConfig.pHwDevHndl = (void *)hVideoEncoder;
    tEPPI656OutConfig.bOverwriteActivePlaneAllowed = EPPI656OUT_OVERWRITE_ACTIVE_PLANE_ALLOWED;
    tEPPI656OutConfig.bAutoUpdate = EPPI656OUT_AUTO_UPDATE;
    tEPPI656OutConfig.cNumberOfPlanes = EPPI656OUT_NR_OF_PLANES;
    tEPPI656OutConfig.tAVformat = AV_ADV7171_VIDEO_FORMAT;
    tEPPI656OutConfig.cMDMAChannel = EPPI656OUT_MDMA_CHANNEL;
    tEPPI656OutConfig.fnDeviceClose = NULL;
#ifdef _USE_PROCESSED_LOOP_THROUGH_ 
    // set shared buffer
    tEPPI656OutConfig.bUseSharedBuffer = true;
#else
    tEPPI656OutConfig.bUseSharedBuffer = false;
#endif        
    
    hEPPI656Out = EPPI656Oopen(&tEPPI656OutConfig, 
                             &nResult, 
                             &pEPPI656OGraficFunctions); 

    if(nResult == ERR_NONE) {                             
        tEPPI656Out = (T_EPPI656O_INST *)hEPPI656Out;
        printf("PPI656Out initialised\n");
    }
    else {
        printf("ERROR: PPI656Out not initialised!\n");
    }
#endif


#ifdef _USE_TEST_PICTURES_
    gpio_becomeOutput(ALIVE_LED_PIN);
    gpio_becomeOutput(_PH6);
    gpio_set(_PH6);
    gpio_clear(ALIVE_LED_PIN);

    while(1)  {  
        EPPI656OshowFrame((void *)hEPPI656Out,
                         (unsigned char *)&testpic_array[0],
                         0,
                         0,
                         720,
                         576);   
        EPPI656OsetActivePlane((void *)hEPPI656Out,EPPI656OgetNextPlaneNum((void *)hEPPI656Out));
        gpio_toggle(_PH6);
        gpio_toggle(ALIVE_LED_PIN);                 
        Sleep(1000);                 
        //EPPI656OwaitForFrameSync(hEPPI656Out);
        EPPI656OshowFrame((void *)hEPPI656Out,
                         (unsigned char *)&testpic_array2[0],
                         0,
                         0,
                         720,
                         576); 
        EPPI656OsetActivePlane((void *)hEPPI656Out,EPPI656OgetNextPlaneNum((void *)hEPPI656Out));   
        gpio_toggle(ALIVE_LED_PIN);
        gpio_toggle(_PH6);
        //EPPI656OwaitForFrameSync(hEPPI656Out);
        Sleep(1000);                      
    }

#endif
    

#ifdef _USE_TAKE_PICTURE_
    if(hEPPI656In) {                            

        // start grabbing
        printf("\n\nStart grabbing...");
        ((FG_FN_START_GRABBING)pEPPI656InFGFunctions[FG_START_GRABBING])((void *)hEPPI656In);
        ifrm_t *tFrame;
        
        while(1) {
            // check if there is a frame avaiable
            if(((FG_FN_FRAME_AVAILABLE)pEPPI656InFGFunctions[FG_FRAME_AVAILABLE])((void *)hEPPI656In)) {
                // Dequeue the next frame
                tFrame = ((FG_FN_DEQUEUE_FRAME)pEPPI656InFGFunctions[FG_DEQUEUE_FRAME])((void *)hEPPI656In);
                ((FG_FN_START_GRABBING)pEPPI656InFGFunctions[FG_STOP_GRABBING])((void *)hEPPI656In);
                break;
            }
        }
        printf("DONE\nYou can checkout the image with VisualDSP++ Image Viewer.\nUse following settings:\n  - Start address:0x%X\n  - Memory stride: 1\n  - Pixel format: UYVY (4:2:2)\n  - Width: 720\n  - Height: 576\n",
                tFrame->pucFramePtr);       
    }
#endif


#ifdef _USE_PROCESSED_LOOP_THROUGH_
              
    if(hEPPI656In && hEPPI656Out) {                            

        // start grabbing
        ((FG_FN_START_GRABBING)pEPPI656InFGFunctions[FG_START_GRABBING])((void *)hEPPI656In);
        while(1) {
        
            // check if there is a frame avaiable
           
            if(((FG_FN_FRAME_AVAILABLE)pEPPI656InFGFunctions[FG_FRAME_AVAILABLE])((void *)hEPPI656In)) {
                // Dequeue the next frame
                ifrm_t *tFrame = ((FG_FN_DEQUEUE_FRAME)pEPPI656InFGFunctions[FG_DEQUEUE_FRAME])((void *)hEPPI656In);
            
                // copy frame
                if(tFrame) {
                    EPPI656OshowFrame((void *)hEPPI656Out,
                                     (unsigned char *)tFrame->pucFramePtr,
                                     0,
                                     0,
                                     720,
                                     576);  
                    EPPI656OsetActivePlane((void *)hEPPI656Out,0);
                    //EPPI656OwaitForFrameSync(hEPPI656Out);
                    // free frame
                    ((FG_FN_FREE_FRAME)pEPPI656InFGFunctions[FG_FREE_FRAME])((void *)hEPPI656In);
                    //bs_sleep(10);
                    //EPPI656OgetNextPlaneNum((void *)hEPPI656Out));
                    //             
                }
            }
        }
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

    while(1);
}
