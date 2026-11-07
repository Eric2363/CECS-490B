#include "PLL.h"
#include "Latch.h"
#include "SPI.h"
#include "tm4c123gh6pm.h"
#include "DelayTimer.h"
//#include "MOSFET.h"
#include "Uart.h"
#include "UART1.h"
#include <stdint.h>
#include <stdbool.h>



//Memory map of notes and where they work on the shift reg data
typedef struct{
	uint8_t midiNote;
	uint64_t bitMask;
	
}NoteMap;

const NoteMap NOTE_TABLE[40] =
{
	{0x3C, 0x0000000001},  // C4  -> bit 0
	{0x3D, 0x0000000002},  // C#4 -> bit 1
	{0x3E, 0x0000000004},  // D4  -> bit 2
	{0x3F, 0x0000000008},  // D#4 -> bit 3
	{0x40, 0x0000000010},  // E4  -> bit 4
	{0x41, 0x0000000020},  // F4  -> bit 5
	{0x42, 0x0000000040},  // F#4 -> bit 6
	{0x43, 0x0000000080},  // G4  -> bit 7

	{0x44, 0x0000000100},  // G#4 -> bit 8
	{0x45, 0x0000000200},  // A4  -> bit 9
	{0x46, 0x0000000400},  // A#4 -> bit 10
	{0x47, 0x0000000800},  // B4  -> bit 11
	{0x48, 0x0000001000},  // C5  -> bit 12
	{0x49, 0x0000002000},  // C#5 -> bit 13
	{0x4A, 0x0000004000},  // D5  -> bit 14
	{0x4B, 0x0000008000},  // D#5 -> bit 15

	{0x4C, 0x0000010000},  // E5  -> bit 16
	{0x4D, 0x0000020000},  // F5  -> bit 17
	{0x4E, 0x0000040000},  // F#5 -> bit 18
	{0x4F, 0x0000080000},  // G5  -> bit 19
	{0x50, 0x0000100000},  // G#5 -> bit 20
	{0x51, 0x0000200000},  // A5  -> bit 21
	{0x52, 0x0000400000},  // A#5 -> bit 22
	{0x53, 0x0000800000},  // B5  -> bit 23

	{0x54, 0x0001000000},  // C6  -> bit 24
	{0x55, 0x0002000000},  // C#6 -> bit 25
	{0x56, 0x0004000000},  // D6  -> bit 26
	{0x57, 0x0008000000},  // D#6 -> bit 27
	{0x58, 0x0010000000},  // E6  -> bit 28
	{0x59, 0x0020000000},  // F6  -> bit 29
	{0x5A, 0x0040000000},  // F#6 -> bit 30
	{0x5B, 0x0080000000},  // G6  -> bit 31

	{0x5C, 0x0100000000},  // G#6 -> bit 32
	{0x5D, 0x0200000000},  // A6  -> bit 33
	{0x5E, 0x0400000000},  // A#6 -> bit 34
	{0x5F, 0x0800000000},  // B6  -> bit 35
	{0x60, 0x1000000000},  // C7  -> bit 36
	{0x61, 0x2000000000},  // C#7 -> bit 37
	{0x62, 0x4000000000},  // D7  -> bit 38
	{0x63, 0x8000000000}   // D#7 -> bit 39
};

//Function Prototypes
void System_Init(void);
void PrintData(uint8_t S,uint8_t D1, uint8_t D2);
uint64_t FindNoteMask(uint8_t note);
void ProcessMIDI(uint8_t status, uint8_t note, uint8_t velocity);
void UpdateShiftRegisters(void);
uint8_t FoldNoteToRange(uint8_t note);
void SPI_AllLEDsOff(void);

void SPI_TestAll40LEDs(void);
//Hold current reg bit states
uint64_t shiftState = 0x0000000000000000;
static bool inputNoteHeld[128] = {false};
static uint8_t outputHoldCount[40] = {0};


int main(){
	
	System_Init();

	SPI_AllLEDsOff();
	UART1_OutString((uint8_t*)"System Ready!\r\n");

	SPI_TestAll40LEDs();
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
	__asm volatile ("cpsie i" ::: "memory");
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

//Uses Binary Search to find note mask
uint64_t FindNoteMask(uint8_t note)
{
	uint8_t low = 0;
	uint8_t high = 40;  // One past the last table entry

	while (low < high)
	{
		uint8_t mid = low + (high - low) / 2;

		if (NOTE_TABLE[mid].midiNote == note)
		{
			return NOTE_TABLE[mid].bitMask;
		}
		else if (note < NOTE_TABLE[mid].midiNote)
		{
			high = mid;
		}
		else
		{
			low = mid + 1;
		}
	}

	return 0;  // Note is not mapped
}

/*
//process the recieved MIDI data and determin what note to play
void ProcessMIDI(uint8_t status, uint8_t note, uint8_t velocity)
{
    uint64_t mask = FindNoteMask(note);

    if (mask == 0)
    {
        return;
    }

    uint8_t command = status & 0xF0;

	//set bit if command is on
    if (command == 0x90 && velocity != 0)
    {
        shiftState |= mask;
    }
	//clear the coresponding bit
    else if (command == 0x80 ||(command == 0x90 && velocity == 0))
    {
        shiftState &= ~mask;
    }

	//update shift regs
    UpdateShiftRegisters();
}
*/
void ProcessMIDI(uint8_t status, uint8_t note, uint8_t velocity)
{
	uint8_t command = status & 0xF0;
	uint8_t mappedNote = FoldNoteToRange(note);
	uint8_t outputIndex = mappedNote - 0x3C;
	uint64_t mask = FindNoteMask(mappedNote);

	if (command == 0x90 && velocity != 0)  // Note On
	{
		if (!inputNoteHeld[note])
		{
			inputNoteHeld[note] = true;
			outputHoldCount[outputIndex]++;
			shiftState |= mask;
			UpdateShiftRegisters();
		}
	}
	else if (command == 0x80 || (command == 0x90 && velocity == 0))  // Note Off
	{
		if (inputNoteHeld[note])
		{
			inputNoteHeld[note] = false;
			outputHoldCount[outputIndex]--;

			if (outputHoldCount[outputIndex] == 0)
			{
				shiftState &= ~mask;
			}

			UpdateShiftRegisters();
		}
	}
}
// Move notes into the mapped range, C4 (60) through D#7 (99),
// preserving the note's pitch class.
uint8_t FoldNoteToRange(uint8_t note)
{
	while (note < 0x3C)  // Below C4
	{
		note += 12;
	}

	while (note > 0x63)  // Above D#7
	{
		note -= 12;
	}

	return note;
}
//Update Shift regs
void UpdateShiftRegisters(void)
{
	UnLatch();

	for (int8_t byte = 4; byte >= 0; byte--)
	{
		SPI_Write((uint8_t)(shiftState >> (byte * 8)));
	}

	Latch_Pulse();
}

//Test all LEDS
void SPI_TestAll40LEDs(void)
{
	uint64_t pattern = 1ULL;

	for (uint8_t led = 0; led < 40; led++)
	{
		SPI_Write((uint8_t)(pattern >> 32));
		SPI_Write((uint8_t)(pattern >> 24));
		SPI_Write((uint8_t)(pattern >> 16));
		SPI_Write((uint8_t)(pattern >> 8));
		SPI_Write((uint8_t)pattern);

		Latch_Pulse();
		DelayMs(50);

		pattern <<= 1;
	}

	// Clear all five shift registers
	for (uint8_t i = 0; i < 5; i++)
		SPI_Write(0x00);

	Latch_Pulse();
}
//Turn all LEDS off
void SPI_AllLEDsOff(void)
{
	for (uint8_t i = 0; i < 5; i++)
	{
		SPI_Write(0x00);
	}

	Latch_Pulse();
}