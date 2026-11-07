#include "SPI.h"

#include <stdint.h>
#include "tm4c123gh6pm.h"
#include "Latch.h"

//Uses A2 for clock
//USES A5 for data

void SPI_Init(void){
    
    // 1. Enable clocks
    SYSCTL_RCGCSSI_R |= 0x01;      // enable SSI0 clock
    SYSCTL_RCGCGPIO_R |= 0x01;     // enable Port A clock
    while((SYSCTL_PRGPIO_R & 0x01) == 0){};  // wait ready
    
    // 2. Configure PA2-5 for SSI
    GPIO_PORTA_AFSEL_R |= 0x3C;    // PA2-5 alternate function
    GPIO_PORTA_DEN_R   |= 0x3C;    // digital enable
    
    // Configure port control
    GPIO_PORTA_PCTL_R &= ~0x00FFFF00;
    GPIO_PORTA_PCTL_R |=  0x00222200;  // SSI0 on PA2-5
    
    GPIO_PORTA_AMSEL_R &= ~0x3C;   // disable analog
    
    // 3. Disable SSI before configuration
    SSI0_CR1_R = 0;                // disable SSI, master mode
    
    // 4. Configure clock
    SSI0_CC_R = 0x0;               // use system clock
    
    // 50 MHz system clock
    // SPI clock = SysClk / (CPSDVSR * (1 + SCR))
    // 50MHz / (50 * 1) = 1MHz
    SSI0_CPSR_R = 50;              // prescale (must be even, 2-254)
    
    SSI0_CR0_R = 0x0007;           // 8-bit data
    // SCR = 0
    // SPH = 0
    // SPO = 0
    // Freescale SPI
    
    // 5. Enable SSI
    SSI0_CR1_R |= 0x02;            // enable SSI
}
void SPI_Write(uint8_t data){
    while((SSI0_SR_R & 0x02) == 0){};  // wait until TX FIFO not full
    SSI0_DR_R = data;
    while(SSI0_SR_R & 0x10){};         // wait until not busy
}

uint8_t SPI_Read(void){
    while((SSI0_SR_R & 0x04) == 0){};  // wait until RX not empty
    return SSI0_DR_R;
}


