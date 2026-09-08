/*
TESTBENCH.c
Engineer: Eric Santana

TEST MODULES
1: SPI
Description: Will talk to 8-bit shift reg
2: PortEGPIO
Description: Will latch data into shift reg
3: DelayTimer
Description: Will provide delays in ms
4:UART
Description: Will use interupts to capture incoming
midi data, using a ring buffer.
5: PortFGPIO
Description: Will be used for debugging and testing


*/

#include "stdbool.h"
#include "stdint.h"
#include "PortF.h"
#include "UART1.h"

#define UART
//#define MIDI
//#define SPI
//#define DELAY

//Includes
#ifdef UART

	#include "Uart.h"
	//function prototypes
void System_Init(void);
void PrintMIDI(void);
void PrintData(uint8_t S,uint8_t D1, uint8_t D2);

#endif

//Main loop
int main(){

	System_Init();
	UART1_OutString((uint8_t*)"System Ready\r\n");
	UART1_OutCRLF();
	
	while(1){
		//check for data in ring buffer
		if(UART_Available()){
				//parse out MIDI message
				uint8_t Status = UART_InChar();
				uint8_t Note = UART_InChar();
				uint8_t Velocity = UART_InChar();
				//print out MIDI data
				PrintData(Status,Note,Velocity);
				UART1_OutString((uint8_t*)"=======\r\n");
				
			if(Status == 0x90 && Velocity > 0){
				GPIO_PORTF_DATA_R &= ~LEDS;
				GPIO_PORTF_DATA_R |= GREEN;
			}
			else if(Status == 0x80 || (Status == 0x90 && Velocity == 0)){
				GPIO_PORTF_DATA_R &= ~LEDS;
				GPIO_PORTF_DATA_R |= RED;
			}

			
		}
	}
}

void PrintData(uint8_t S,uint8_t D1, uint8_t D2){
	
	UART1_OutString((uint8_t*)"0x");
	UART1_OutUHex(S);
	UART1_OutCRLF();
	UART1_OutString((uint8_t*)"0x");
	UART1_OutUHex(D1);
	UART1_OutCRLF();
	UART1_OutString((uint8_t*)"0x");
	UART1_OutUHex(D2);
	UART1_OutCRLF();
	
}



void System_Init(){
	
	UART_Init(true,false);
	PortF_Init();
	UART1_Init(false,false);
	
}

