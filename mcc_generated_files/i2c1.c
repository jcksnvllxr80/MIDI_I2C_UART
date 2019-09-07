
#include "i2c1.h"
#include "eusart1.h"/**
  Section: Macro Declarations
*/
#define EUSART1_TX_BUFFER_SIZE 8
#define EUSART1_RX_BUFFER_SIZE 8
#define I2C1_SLAVE_ADDRESS 0x04 
#define I2C1_SLAVE_MASK    0x7F


/**
 Section: Global Variables
*/
volatile uint8_t I2C1_slaveWriteData = 0x55;

volatile uint8_t eusart1TxHead = 0;
volatile uint8_t eusart1TxTail = 0;
volatile uint8_t eusart1TxBuffer[EUSART1_TX_BUFFER_SIZE];
volatile uint8_t eusart1TxBufferRemaining;

volatile uint8_t eusart1RxHead = 0;
volatile uint8_t eusart1RxTail = 0;
volatile uint8_t eusart1RxBuffer[EUSART1_RX_BUFFER_SIZE];
volatile uint8_t eusart1RxCount;


void I2C1_Initialize(void)
{
    ANSELC3 = 0;
    ANSELC4 = 0;
    // initialize the hardware
    SSP1STAT = 0x80;
    SSP1CON1 = 0x36;
    SSP1CON2 = 0x00;
    SSP1CON3 = 0x00;
    SSP1MSK = (I2C1_SLAVE_MASK << 1);  // adjust UI mask for R/nW bit            
    // SSPADD 4; 
    SSP1ADD = (I2C1_SLAVE_ADDRESS << 1);  // adjust UI address for R/nW bit
    
    GIE = 1;
    PEIE = 1;
    PIR3bits.SSP1IF = 0; // clear the slave interrupt flag
    PIE3bits.SSP1IE = 1; // enable the master interrupt
}


 
void I2C1_ISR ( void )
{
    uint8_t buffData = 0x00;
    
    PIR3bits.SSP1IF = 0;        // clear the slave interrupt flag
    buffData = SSP1BUF;    // read SSPBUF to clear BF
    if(1 == SSP1STATbits.D_nA){
        // this is not an I2C address
        
        EUSART1_Write(buffData); //send midi
    }
    //SSP1CON1bits.CKP = 1;    // release SCL
} // end I2C1_ISR()



/**
  Section: EUSART1 APIs
*/

void EUSART1_Initialize(void)
{
    // disable interrupts before changing states
    PIE3bits.RC1IE = 0;
    PIE3bits.TX1IE = 0;

    // Set the EUSART1 module to the options selected in the user interface.

    // ABDOVF no_overflow; SCKP Non-Inverted; BRG16 16bit_generator; WUE disabled; ABDEN disabled; 
    BAUD1CON = 0x08;

    // SPEN enabled; RX9 8-bit; CREN enabled; ADDEN disabled; SREN disabled; 
    RC1STA = 0x90;

    // TX9 8-bit; TX9D 0; SENDB sync_break_complete; TXEN enabled; SYNC asynchronous; BRGH hi_speed; CSRC slave; 
    TX1STA = 0x24;

    // Baud Rate = 31250; SP1BRGL FF; 
    SP1BRGL = 0xFF;

    // Baud Rate = 31250; SP1BRGH 01; 
    SP1BRGH = 0x01;


    // initializing the driver state
    eusart1TxHead = 0;
    eusart1TxTail = 0;
    eusart1TxBufferRemaining = sizeof(eusart1TxBuffer);

    eusart1RxHead = 0;
    eusart1RxTail = 0;
    eusart1RxCount = 0;

    // enable receive interrupt
    PIE3bits.RC1IE = 1;
}

uint8_t EUSART1_Read(void)
{
    uint8_t readValue  = 0;
    
    while(0 == eusart1RxCount)
    {
    }

    readValue = eusart1RxBuffer[eusart1RxTail++];
    if(sizeof(eusart1RxBuffer) <= eusart1RxTail)
    {
        eusart1RxTail = 0;
    }
    PIE3bits.RC1IE = 0;
    eusart1RxCount--;
    PIE3bits.RC1IE = 1;

    return readValue;
}

void EUSART1_Write(uint8_t txData)
{
    while(0 == eusart1TxBufferRemaining)
    {
    }

    if(0 == PIE3bits.TX1IE)
    {
        TX1REG = txData;
    }
    else
    {
        PIE3bits.TX1IE = 0;
        eusart1TxBuffer[eusart1TxHead++] = txData;
        if(sizeof(eusart1TxBuffer) <= eusart1TxHead)
        {
            eusart1TxHead = 0;
        }
        eusart1TxBufferRemaining--;
    }
    PIE3bits.TX1IE = 1;
}

void EUSART1_Transmit_ISR(void)
{

    // add your EUSART1 interrupt custom code
    if(sizeof(eusart1TxBuffer) > eusart1TxBufferRemaining)
    {
        TX1REG = eusart1TxBuffer[eusart1TxTail++];
        if(sizeof(eusart1TxBuffer) <= eusart1TxTail)
        {
            eusart1TxTail = 0;
        }
        eusart1TxBufferRemaining++;
    }
    else
    {
        PIE3bits.TX1IE = 0;
    }
}

void EUSART1_Receive_ISR(void)
{

    if(1 == RC1STAbits.OERR)
    {
        // EUSART1 error - restart

        RC1STAbits.CREN = 0;
        RC1STAbits.CREN = 1;
    }

    // buffer overruns are ignored
    eusart1RxBuffer[eusart1RxHead++] = RC1REG;
    if(sizeof(eusart1RxBuffer) <= eusart1RxHead)
    {
        eusart1RxHead = 0;
    }
    eusart1RxCount++;

}
/**
  End of File
*/