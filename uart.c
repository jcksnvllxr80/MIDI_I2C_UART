/* 
 * File:   main.c
 * Author: A-A-Ron
 *
 * Created on June 24, 2017, 11:50 AM
 */

#include <stdio.h>
#include <stdlib.h>
#include <xc.h>

void sendMidi(uint8_t msg);
void midiInit(void);



void midiInit(void)
{
	TX1STAbits.TXEN = 1;   
    TX1STAbits.SYNC = 0;    
    RC1STAbits.SPEN = 1;
    TX1STAbits.TX9 = 0;
    TX1STAbits.BRGH = 2;
    ANSELB2 = 0;
    SP1BRGL = 0;
    RC1REG = 0;
    RC2PPS = 0b01001;
    UART1MD = 0;
}


void sendMidi(uint8_t msg)
{
	TX1REG = msg;
	while(!TX1IF);
}