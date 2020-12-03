#ifndef _ADV7183_H_INCLUDED_
#define _ADV7183_H_INCLUDED_

#ifdef _USE_VDK_
    #include <vdk.h>
#endif
    
#include <Datatypes.h>
#include "../../GPIOconfig.h"
#include "../../mmio/AnalogVideo/AnalogVideo.h"
#include "../../i2c/I2CMan.h"
#include "../../../../../blacksheep/common/framegrabber/frameGrabber.h"
#include "ADV7183_Regs.h"

#define ERR_ADV7183_I2C_ERROR           (ERR_GENERIC - 10)
#define ERR_ADV7183_OUT_OF_MEM          (ERR_GENERIC - 20)
#define ERR_ADV7183_INVALID_HANDLE      (ERR_GENERIC - 30)
#define ERR_ADV7183_UNKNOWN_DECODER     (ERR_GENERIC - 40)
#define ERR_ADV7183_NO_SOURCES          (ERR_GENERIC - 50)

// select driver behaviour *******************************
#define ADV7183_DEBUG_LVL                               0
// *******************************************************

#define ADV7183_DEFAULT_HWID                            0x20        ///< Default I²C ID 
#define ADV7183_MAX_NOF_FRAME_BUFFERS                   16          ///< max number of framebuffers
#define ADV7183_INPUT_SETUP_TIME                        250         ///< time in ms to wait after input switch to check for lock
#define ADV7183_RESET_WAIT_TIME                         100         ///< delay in ms after reset
#define ADV7183_THREAD_UPDATE_TIME                      500         ///< sleep time for control threads in ms
#define ADV1783_X_MAX                                   720         ///< maximum pixels in x direction of all supported formats
#define ADV1783_Y_MAX                                   576         ///< maximum pixels in y direction of all supported formats
#define ADV7183_MAX_NUM_DEVICES                         4           ///< maximum of parallel devices supported

#define ADV7183_AUTOSCAN                                255



typedef  signed long T_ADV7183_HANDLE;
typedef void (*ADV7183_FN_CLOSE_DEVICE)(unsigned long pa_pHwDevHdl);


typedef enum {
    /// Composite input at AIN1
    ADV7183_CVBS_AIN1 = 0x0001,
    /// Composite input at AIN2
    ADV7183_CVBS_AIN2 = 0x0002,
    /// Composite input at AIN3
    ADV7183_CVBS_AIN3 = 0x0004,
    /// Composite input at AIN4
    ADV7183_CVBS_AIN4 = 0x0008,
    /// Composite input at AIN5
    ADV7183_CVBS_AIN5 = 0x0010,
    /// Composite input at AIN6
    ADV7183_CVBS_AIN6 = 0x0020,
    /// YC input at AIN1 and AIN4
    ADV7183_YC_AIN14  = 0x0040,
    /// YC input at AIN2 and AIN5
    ADV7183_YC_AIN25  = 0x0080,
    /// YC input at AIN3 and AIN6
    ADV7183_YC_AIN36   = 0x0100,
    /// YPrPb input at AIN1, AIN4 and AIN5
    ADV7183_YPrPb_AIN145 =   0x0200,
    /// YPrPb input at AIN2, AIN3 and AIN6
    ADV7183_YPrPb_AIN236 =   0x0400,
    /// Composite input at AIN7
    ADV7183_CVBS_AIN7    =   0x0800,
    /// Composite input at AIN8
    ADV7183_CVBS_AIN8    =   0x1000,
    /// Composite input at AIN9
    ADV7183_CVBS_AIN9    =   0x2000,
    /// Composite input at AIN10
    ADV7183_CVBS_AIN10   =   0x4000,
    /// Composite input at AIN11
    ADV7183_CVBS_AIN11   =   0x8000,
    /// Mask of all composite inputs
    ADV7183_CVBS_ALL     =   0xf83f,
    /// Mask of all YC inputs
    ADV7183_YC_ALL       =   0x01c0,
    /// Mask of all YPrPb inputs
    ADV7183_YPrPb_ALL    =   0x0600,
    
    
} E_ADV7183_INPUT;

/**
    @brief Values for video standard autodetection
**/
typedef enum
{
    ADV7183_NTSC_MJ = 0x00,
    ADV7183_NTSC_443 = 0x10,
    ADV7183_PAL_M = 0x20,
    ADV7183_PAL_60 = 0x30,
    ADV7183_PAL_BGHID = 0x40,
    ADV7183_SECAM = 0x50,
    ADV7183_PAL_N = 0x60,
    ADV7183_SECAM_525 = 0x70,
    ADV7183_AUTODETECT = 0xff,
} E_ADV7183_INPUT_TYPE;


typedef enum {
    UNKNOWN_DECODER,
    ADV7183A_ES1,
    ADV7183A_ES2,
    ADV7183A_FT,
    ADV7183A,
    ADV7183B,

} E_ADV7183_TYPE;

/**
    @brief Sampling mode of decoder
**/
typedef enum {
    ADV7183_MODE_SINGLE_SHOT,                               ///< capture one frame 
    ADV7183_CONT_CAPTURE_AUTO,                                
    ADV7183_CONT_CAPTURE_MULTI_FRAME_BUFFER_NO_OVERWRITE,
    ADV7183_CONT_CAPTURE_MULTI_FRAME_BUFFER_OVERWRITE,

} E_ADV7183_MODE;


/**
    @brief mapping of ID register values to Decoder types
**/
typedef struct {
    unsigned char cID;
    E_ADV7183_TYPE eType;
    char acName[16];
    //I2CConfig atSetupValues[18];
} T_ADV7183_DECODER_TYPE;

typedef enum {
    ADV7183_OPERATION_IDLE,                ///< do nothing
    ADV7183_OPERATION_SCAN_ALL_INPUTS,     ///< perform scan on all if lock is lost and switch to first available
    ADV7183_OPERATION_SCAN_SINGLE_INPUT,   ///< always wait for lock
    
} E_ADV7183_OPERATION;


/**
    @brief config struct passed to open function
**/
typedef struct {
  
    T_AV_SOURCE *patAVsourcesList;
    unsigned char cAVSourcesNum;
    T_GPIO_MASK  tOEpin; 
    T_I2C_HANDLE tI2CHndl;                
    unsigned char cI2Cadr;  
    
} T_ADV7183_CONFIG;

/**
    @brief Instance of ADV7183 Decoder
**/
typedef struct {
    T_I2C_HANDLE tI2CHndl;                   ///< handle for I²C communication
    unsigned char cI2Cadr;                  ///< I²C device address
    T_GPIO_MASK  tOEpin;                    ///< output enable pin
    unsigned char  eSubType;               ///< Type of decoder
    E_ADV7183_OPERATION eCurrentOperation;   ///<
    unsigned char cCurrentInput;            ///< current input source
    bool          bLocked;                  ///< currently locked in
    T_AV_SOURCE *patAVsourcesList;          ///< List of AV sources
    unsigned char cAVSourcesNum;            ///< number of elements in list
    T_AV_STANDARD_DESCR     *tVideoStd;     ///< Descriptor for video standards
#ifdef _USE_VDK_
    VDK_ThreadID tThreadID;                 ///< ID of control thread
#endif    
    ADV7183_FN_CLOSE_DEVICE fnHwDevCloseDev;///< pointer to the close funtion
} T_ADV7183Inst;



//-----------------------------Prototypes------------------------------------------------//
T_ERROR_CODE ADV7183setup();
T_ERROR_CODE ADV7183scanAllInputs(T_ADV7183_HANDLE pa_tHndl);
unsigned char ADV7183queryInputVideoStd(T_ADV7183_HANDLE pa_tHndl);
bool ADV7183isInterlaced(T_ADV7183_HANDLE);
T_ADV7183_HANDLE ADV7183open(T_ADV7183_CONFIG *pa_tConfig,
                            T_ERROR_CODE *pa_ptError);
T_ERROR_CODE ADV7183reset(T_ADV7183_HANDLE pa_tHndl);
//ifrm_t *ADV7183getNextFrame(T_ADV7183_HANDLE pa_tHndl);
T_ERROR_CODE ADV7183freeFrame(T_ADV7183_HANDLE pa_tHndl);
void ADV7183startGrabbing(T_ADV7183_HANDLE pa_tHndl);
void ADV7183stopGrabbing(T_ADV7183_HANDLE pa_tHndl);     
T_ERROR_CODE ADV7183config(T_ADV7183_HANDLE pa_tHndl);
int ADV7183framesAvailable(T_ADV7183_HANDLE pa_tHndl);   
bool ADV7183isLocked(T_ADV7183_HANDLE pa_tHndl);
T_ERROR_CODE ADV7183setInput(T_ADV7183_HANDLE pa_tHndl, unsigned char pa_cInputNumber);
T_ERROR_CODE ADV7183setOperationMode(T_ADV7183_HANDLE pa_tHndl, E_ADV7183_OPERATION pa_eMode);
T_ERROR_CODE ADV7183close(T_ADV7183_HANDLE pa_tHndl);
           
#endif
