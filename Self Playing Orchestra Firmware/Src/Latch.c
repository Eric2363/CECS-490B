#include "Latch.h"
#include "DelayTimer.h"   // for DelayMs()

void Latch_Init(){
  //Enable Clock for portE
  SYSCTL_RCGCGPIO_R |= PORTE;
  //wait for it finish set up (use PRGPIO, not RCGCGPIO)
  while((SYSCTL_PRGPIO_R & PORTE) == 0){}

  //GPIO config
  //Enable E0
  GPIO_PORTE_DEN_R |= LATCH_PIN;
  //E0 as output
  GPIO_PORTE_DIR_R |= LATCH_PIN;
  //Disable Analog mode
  GPIO_PORTE_AMSEL_R &= ~LATCH_PIN;
  //Disable alternate function mode
  GPIO_PORTE_AFSEL_R &= ~LATCH_PIN;

  // start low
  GPIO_PORTE_DATA_R &= ~LATCH_PIN;
}

void Latch(){
  //set pin high
  GPIO_PORTE_DATA_R |= LATCH_PIN;
}

void UnLatch(){
  //set pin low
  GPIO_PORTE_DATA_R &= ~LATCH_PIN;
}

//pulse latch
void Latch_Pulse(){
  GPIO_PORTE_DATA_R &= ~LATCH_PIN;  // low
  DelayMs(1);

  GPIO_PORTE_DATA_R |=  LATCH_PIN;  // high (rising edge updates 595 outputs)
  DelayMs(1);

  GPIO_PORTE_DATA_R &= ~LATCH_PIN;  // low
  DelayMs(1);
}