
#ifndef _SPI_H_
#define _SPI_H_

#include "../GPIOconfig.h"

#define SPI_MASTER  true
#define SPI_SLAVE   false


#ifdef _USE_VDK_
#include <VDK.h>
#endif

typedef unsigned long   T_SPI_HANDLE;   ///< Handle hides the implementation details!

typedef void (*T_SPI_DATA_CALLBACK)(T_SPI_HANDLE, unsigned short);

#define MAX_SPI_INTERFACES      5
bool spi_platformInit(unsigned char pa_cSPI, unsigned long pa_cSlaveNumber);    ///< External Platform specific Init.

 

/**
 *  @brief   Options for T_SPI_PARAM parameters.
 *
 **/
typedef enum {

    SPI_8BIT,           ///< SPI in  8 Bit Transfer Mode.
    SPI_16BIT,          ///< SPI in 16 Bit Transfer Mode.
    SPI_CPOL_HIGH,      ///< Clock Polarity set HIGH.
    SPI_CPOL_LOW,       ///< Clock Polarity set LOW.
    SPI_CPHA_HIGH,      ///< Clock Phase set HIGH.
    SPI_CPHA_LOW,       ///< Clock Phase set LOW.
    SPI_MSB_FIRST,      ///< Send the MSB first.
    SPI_LSB_FIRST,      ///< Send the LSB first.
    SPI_USE_SPISS,      ///< Use the SPI 
    SPI_NONE
} T_SPI_PARAM;



typedef struct {
    bool                mbMaSl;             ///< True if master.
    T_GPIO_MASK         mtCSEL;             ///< GPIO Chip Select Flag
    unsigned short      mnRxBufferSize;     ///< Not used in simpleio.
    unsigned short      mnTxBufferSize;     ///< Not used in simpleio.
    T_SPI_DATA_CALLBACK mfnDataCallback;    
    unsigned long       mnBaudrate;         //[Hz]
    T_SPI_PARAM         mnTransferSize;     //8 or 16 bit transfer.
    T_SPI_PARAM         mnClockPolarity;        //Sampling on rising or falling edge
    T_SPI_PARAM         mnClockPhase;
    T_SPI_PARAM         mnMsbLsbFirst;      //Sending Msb or Lsb first
    T_SPI_PARAM         mnMasterErrorFlag;  //SPISS/ as error flag in master mode activated or deactivated. (NOT PROCESSED)
    unsigned short      msDefaultWord;          // 0 for Send zero, else SPIread sends transmits the word specified here.

} T_SPI_CONFIG;


typedef struct {
        ADI_INT_PERIPHERAL_ID       nSPIPeripheralID;
        ADI_INT_PERIPHERAL_ID       nSPIErrorID;
        volatile unsigned short     *pSPIflg;
        volatile unsigned short     *pSPIbaud;
        volatile unsigned short     *pSPIctl;
        volatile unsigned short     *pSPItdbr;
        volatile unsigned short     *pSPIrdbr;
        volatile unsigned short     *pSPIstat;
        volatile unsigned short     *pSPIshadow;
        volatile unsigned long      *pSICimask;
        // DMA register
        volatile unsigned short     *pDMAirqStat;
        volatile unsigned short     *pDMAconfig;
        volatile void               **pDMAstartaddr;
        volatile signed short       *pDMAxmodify;
        volatile unsigned short     *pDMAxcount;
        volatile unsigned short     *pDMAcurrXcount;
        volatile signed short       *pDMAymodify;
        volatile unsigned short     *pDMAycount;
        volatile unsigned short     *pDMAcurrYcount;
        volatile void               **pDMAnextDescrPtr;

        unsigned char               nSlaveCount;
        bool                        bIsMaster;
#ifdef _USE_VDK_
        VDK_SemaphoreID             idAccess;
#endif
} T_SPI_SPEC;





#define SPI_BIT_XFER_SIZE   0x0100
#define SPI_BIT_BYTE_MODE   0x0200
#define SPI_BIT_CLK_PHASE   0x0400
#define SPI_BIT_CLK_POL     0x0800


typedef struct {

    //unsigned char m_nID;              ///< Physical Number of the interface. //??? Just use array Indexor as an array???
    bool            inUse;              ///< Has this interface been initialised??
    bool            m_bMaSl;            ///< If this interface is a master or a slave.
    unsigned char   m_nInstances;       ///< The number of driver instances attached to this interface.
#ifdef _USE_VDK_
    VDK_SemaphoreID m_ControlSem;       ///< Queue Access to this interface via Semaphore!
#endif


} T_SPI_INTERFACE;


typedef struct {

    T_SPI_INTERFACE m_Interfaces[MAX_SPI_INTERFACES];

    unsigned long   m_nCoreClkFreq;
    unsigned long   m_nSysClkFreq;

} T_SPI_DESCRIPTION;


typedef struct {

    unsigned char   mMaSl;              ///< Master / Slave (0 for Slave, 1 for Master).
    unsigned short  mSPI_BAUD;          ///< Context Saved BAUD rate register.  (This is set during SPI open).
    //unsigned short    mSPI_FLG;           ///< Context Saved FLAG register.       (This is set during SPI open).

    // For Transfer Size, Clock polarity, MSB/LSB first, Mastor Error Flag
    unsigned short  mSPI_CTL;           ///< Configuration Bit flags.       (This is set during SPI open).



} T_SPI_CONTEXT;


typedef struct {        // All SPI configuration etc goes in the instance

    T_SPI_CONTEXT   mConfig;
    bool            bIsInControl;
    unsigned char   cSPI;               ///< Number of SPI interface that this Context relates to.
    T_GPIO_MASK     mCS;                ///< GPIO Chip Select of the
    unsigned short  msDefaultWord;      ///< 0 for Send zero, else SPIread sends transmits the word specified here.
} T_SPI_INSTANCE;


//----------- PROTOTYPES
void SPIinit(void);
T_ERROR_CODE SPIsetup(unsigned char pa_cSPI, unsigned long pa_nSysClk, unsigned long pa_nCoreClk,bool pa_bMaSl);
void SPIcleanup(unsigned char pa_cSPI);
unsigned int SPI_getPower(unsigned int value);
T_SPI_HANDLE SPIopen(unsigned char pa_cSPI, T_SPI_CONFIG *pa_pCfg, T_ERROR_CODE *pa_pError);
T_ERROR_CODE SPIsetCsLevel(T_SPI_HANDLE pa_hSPI, bool pa_bHigh);

T_ERROR_CODE    SPIswitchContext    (T_SPI_HANDLE pa_hSPI);
T_ERROR_CODE    SPIcontrol          (T_SPI_HANDLE   pa_hSPI);
T_ERROR_CODE    SPIclose            (T_SPI_HANDLE pa_hSPI);
T_ERROR_CODE    SPIread             (T_SPI_HANDLE pa_hSPI, unsigned short *pa_pnData, unsigned long pa_nLength, T_ERROR_CODE *pa_pError);
unsigned short  SPIwrite            (T_SPI_HANDLE pa_hSPI, unsigned short *pa_pnData, unsigned long pa_nLength, T_ERROR_CODE *pa_pError);
T_ERROR_CODE    SPIselect           (T_SPI_HANDLE pa_hSPI);
T_ERROR_CODE    SPIdeselect         (T_SPI_HANDLE pa_hSPI);
T_ERROR_CODE    SPItakeControl      (T_SPI_HANDLE pa_hSPI);
T_ERROR_CODE    SPIreleaseControl   (T_SPI_HANDLE pa_hSPI);

T_ERROR_CODE    SPIreadwrite(T_SPI_HANDLE pa_hSPI, 
                             unsigned short *pa_pnInputData, 
                             unsigned short *pa_pnOutputData, 
                             unsigned long pa_nLength, 
                             T_ERROR_CODE *pa_pError);



//----------- ERROR CODES

#define SPI_ERR_GENERIC                 -1
#define SPI_ERR_INVALID_INTERFACE       -2
#define SPI_ERR_DEVICE_INUSE            -3
#define SPI_ERR_ILLEGAL_CONTEXT_SWITCH  -4
#define SPI_ERR_NOT_IN_CONTROL          -5
#define SPI_ERR_DEVICE_NOT_COMPATIBLE   -6
#define SPI_ERR_NOT_ENOUGH_MEMORY       -7

#endif
