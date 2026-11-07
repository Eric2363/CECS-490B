#include "PLL.h"
#include "Latch.h"
#include "SPI.h"
#include "tm4c123gh6pm.h"
#include "DelayTimer.h"
#include "MOSFET.h"
#include "Uart.h"
#include "UART1.h"
#include <stdint.h>
#include <stdbool.h>

//Make a struct to contain the memory map of notes and their where they work on the shift reg data
typedef struct{
	uint8_t midiNote;
	uint8_t bitMask;
	
}NoteMap;

const NoteMap NOTE_TABLE[8] =
{
    {0x3C, 0x01},  // Middle C -> bit 0
    {0x3E, 0x02},  // D        -> bit 1
    {0x40, 0x04},  // E        -> bit 2
    {0x41, 0x08},  // F        -> bit 3
    {0x43, 0x10},  // G        -> bit 4
    {0x45, 0x20},  // A        -> bit 5
    {0x47, 0x40},  // B        -> bit 6
    {0x48, 0x80}   // Next C   -> bit 7
};

//Function Prototypes
void System_Init(void);
void PrintData(uint8_t S,uint8_t D1, uint8_t D2);
uint8_t FindNoteMask(uint8_t note);
void ProcessMIDI(uint8_t status, uint8_t note, uint8_t velocity);

//Hold current reg bit states
uint8_t shiftState = 0x00;



int main(){
	
	System_Init();
	UART1_OutString((uint8_t*)"System Ready!\r\n");
	UART_OutString((uint8_t*)"System Ready!\r\n");
	uint8_t data = 0x01;
	
	while(1){
 
		//Read incoming data then update reg
		
		if(UART_Available()){
			
			//Parse out MIDI message
			uint8_t Status = UART_InChar();
			uint8_t Note = UART_InChar();
			uint8_t Velocity = UART_InChar();
			
			//process the shift reg state
			ProcessMIDI(Status, Note,Velocity);
			
			//print out MIDI data
			PrintData(Status,Note,Velocity);
			UART1_OutString((uint8_t*)"=======\r\n");
		
		}
		
		
//		UnLatch();
//		SPI_Write(data);
//		Latch_Pulse();
//		DelayMs(1000);
//		data = data <<  1;
		
		
	
	}
	
	return 0;
}

void System_Init(){
	
	PLL_Init();
	
  UART_Init(true,false);
	SPI_Init();
	Latch_Init();
	DelayTimer_Init();
	UART1_Init(false,false);
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

uint8_t FindNoteMask(uint8_t note)
{
    for (uint8_t i = 0; i < 8; i++)
    {
        if (NOTE_TABLE[i].midiNote == note)
        {
            return NOTE_TABLE[i].bitMask;
        }
    }

    return 0;  // Note is not mapped
}


void ProcessMIDI(uint8_t status, uint8_t note, uint8_t velocity)
{
    uint8_t mask = FindNoteMask(note);

    if (mask == 0)
    {
        return;
    }

    uint8_t command = status & 0xF0;

    if (command == 0x90 && velocity != 0)
    {
        shiftState |= mask;
    }
    else if (command == 0x80 ||
             (command == 0x90 && velocity == 0))
    {
        shiftState &= ~mask;
    }

    UnLatch();
    SPI_Write(shiftState);
    Latch_Pulse();
}