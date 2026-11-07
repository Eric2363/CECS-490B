#include <stdint.h>
#include "tm4c123gh6pm.h"

#define TIMER0_CLK 0x01  // Timer0

// Initialize Timer0A as a 1ms  periodic down-counter
void DelayTimer_Init(void){
  SYSCTL_RCGCTIMER_R |= TIMER0_CLK;
  while((SYSCTL_RCGCTIMER_R & TIMER0_CLK) == 0){}

  TIMER0_CTL_R = 0;            // disable Timer0A during setup
  TIMER0_CFG_R = 0x00;         // 32-bit mode
  TIMER0_TAMR_R = 0x02;        // periodic mode, down-count
  TIMER0_TAPR_R = 0;           // prescale = 0

  // For 50 MHz clock: 50,000 ticks = 1 ms
  TIMER0_TAILR_R = 50000 - 1;  // 1ms
  TIMER0_ICR_R = 0x01;         // clear timeout flag
  TIMER0_CTL_R = 0x01;         // enable Timer0A
}

// Blocking delay in milliseconds
void DelayMs(uint32_t ms){
  while(ms){
    TIMER0_ICR_R = 0x01;                 // clear TATO flag
    while((TIMER0_RIS_R & 0x01) == 0){}  // wait for timeout
    ms--;
  }
}

//  Delay(): 0.1 seconds
void Delay(void){
  DelayMs(100);
}
