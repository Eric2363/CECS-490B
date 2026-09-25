#include "tm4c123gh6pm.h"

 /*
 Bit postions
 
 2		1			8		4		2		1
[F] [E] | [D] [C] [B] [A]

Port F  bits
F4 - F3 - F2 - F1 - F0
SW1 - G  - B  - R  - SW2
 
 */


// PortF register Mask
#define PORTF 0x20


// Interupt Level
#define LEVEL2 0x00400000


//LEDS
#define LEDS 0x0E
#define RED 0x02
#define BLUE 0x04
#define GREEN 0x08
#define YELLOW 0x0A
#define CYAN 0x0C
#define PURPLE 0x06

//Switches
#define SWITCHES 0x11

#define SW1 0x10
#define SW2 0x01

void PortF_Init(void);

void GPIOPortF_Handler(void);