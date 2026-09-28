#include "tm4c123gh6pm.h"
#include "stdint.h"


void SPI_Init(void);
void SPI_Write(uint8_t data);
uint8_t SPI_Read(void);
void Latch_Pulse(void);
