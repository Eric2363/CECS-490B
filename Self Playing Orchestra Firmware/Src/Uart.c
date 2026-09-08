
#include "UART.h"
#define RB_SIZE 64

typedef struct{
  volatile uint16_t head;
  volatile uint16_t tail;
  volatile uint8_t data[RB_SIZE];
} RingBuffer;

 RingBuffer RxRB;
 bool RxInterruptEnabled = true;

// =====================
// Ring Buffer Helpers
// =====================
void rb_Init(RingBuffer *rb){
  rb->head = 0;
  rb->tail = 0;
}

bool rb_empty(const RingBuffer *rb){
  return (rb->head == rb->tail);
}

bool rb_full(const RingBuffer *rb){
  return (uint16_t)((rb->head + 1) % RB_SIZE) == rb->tail;
}

bool rb_put(RingBuffer *rb, uint8_t byte){
  uint16_t next = (rb->head + 1) % RB_SIZE;

  if(next == rb->tail){
    return false; // buffer full, byte dropped
  }

  rb->data[rb->head] = byte;
  rb->head = next;
  return true;
}

bool rb_get(RingBuffer *rb, uint8_t *byte){
  if(rb_empty(rb)){
    return false;
  }

  *byte = rb->data[rb->tail];
  rb->tail = (rb->tail + 1) % RB_SIZE;
  return true;
}

bool UART_Available(void){
  return !rb_empty(&RxRB);
}

// =====================
// UART Init @ 16Mhz 115,200 baud
// =====================
void UART_Init(bool RxInt, bool TxInt){
  SYSCTL_RCGC1_R |= SYSCTL_RCGC1_UART0; // activate UART0
  SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOA; // activate port A

  rb_Init(&RxRB);
  RxInterruptEnabled = RxInt;

  UART0_CTL_R = 0;                      // disable UART
  UART0_IBRD_R = 8;                    // IBRD = int(16,000,000 / (16 * 115,200)) = int(8.680)
  UART0_FBRD_R = 44;                     // FBRD = int(0.680 * 64 + 0.5) = 44
  UART0_LCRH_R = UART_LCRH_WLEN_8;      // 8-bit, no FIFO
  UART0_ICR_R = 0x7FF;                  // clear all UART interrupt flags

  // take care of interrupt setup
  if(RxInt || TxInt){
    NVIC_PRI1_R = (NVIC_PRI1_R & ~0x0000E000) | 0x0000A000; // priority 5
    NVIC_EN0_R |= NVIC_EN0_UART0;                           // enable UART0 interrupt in NVIC

    if(RxInt){
      UART0_IM_R |= UART_IM_RXIM;         // Enable RX interrupt
    }

    if(TxInt){
      UART0_IM_R |= UART_IM_TXIM;         // Enable TX interrupt
    }
  }

  UART0_CTL_R |= UART_CTL_RXE | UART_CTL_TXE | UART_CTL_UARTEN; // enable Tx, RX and UART

  GPIO_PORTA_AFSEL_R |= 0x03;           // enable alt funct on PA1-0
  GPIO_PORTA_DEN_R |= 0x03;             // enable digital I/O on PA1-0
  GPIO_PORTA_PCTL_R = (GPIO_PORTA_PCTL_R & 0xFFFFFF00) + 0x00000011; // configure PA1-0 as UART
  GPIO_PORTA_AMSEL_R &= ~0x03;          // disable analog functionality on PA
}

// =====================
// UART ISR
// =====================
void UART0_Handler(void){
  // RX interrupt
  if(UART0_RIS_R & UART_RIS_RXRIS){
    while((UART0_FR_R & UART_FR_RXFE) == 0){
      uint8_t ch = (uint8_t)(UART0_DR_R & 0xFF);
      rb_put(&RxRB, ch);
    }
    UART0_ICR_R = UART_ICR_RXIC; // clear RX interrupt
		
  }

  // TX interrupt
  if(UART0_RIS_R & UART_RIS_TXRIS){
    UART0_ICR_R = UART_ICR_TXIC; // clear TX interrupt
  }
}

void OutCRLF(void){
  UART_OutChar(CR);
  UART_OutChar(LF);
}

uint8_t UART_InChar(void){
  uint8_t ch;

  // if RX interrupt is enabled, read from ring buffer
  if(RxInterruptEnabled){
    while(!rb_get(&RxRB, &ch)){
    }
    return ch;
  }

  // otherwise use normal polling
  while((UART0_FR_R & UART_FR_RXFE) != 0){
  }
  return (uint8_t)(UART0_DR_R & 0xFF);
}

void UART_OutChar(uint8_t data){
  while((UART0_FR_R & UART_FR_TXFF) != 0){
  }
  UART0_DR_R = data;
}

void UART_OutString(uint8_t *pt){
  while(*pt){
    UART_OutChar(*pt);
    pt++;
  }
}

uint32_t UART_InUDec(void){
  uint32_t number = 0, length = 0;
  char character;

  character = UART_InChar();
  while(character != CR){
    if((character >= '0') && (character <= '9')){
      number = 10 * number + (character - '0');
      length++;
      UART_OutChar(character);
    }
    else if((character == BS) && length){
      number /= 10;
      length--;
      UART_OutChar(character);
    }
    character = UART_InChar();
  }
  return number;
}

void UART_OutUDec(uint32_t n){
  if(n >= 10){
    UART_OutUDec(n / 10);
    n = n % 10;
  }
  UART_OutChar(n + '0');
}

uint32_t UART_InUHex(void){
  uint32_t number = 0, digit, length = 0;
  char character;

  character = UART_InChar();
  while(character != CR){
    digit = 0x10; // assume bad

    if((character >= '0') && (character <= '9')){
      digit = character - '0';
    }
    else if((character >= 'A') && (character <= 'F')){
      digit = (character - 'A') + 0xA;
    }
    else if((character >= 'a') && (character <= 'f')){
      digit = (character - 'a') + 0xA;
    }

    if(digit <= 0xF){
      number = number * 0x10 + digit;
      length++;
      UART_OutChar(character);
    }
    else if((character == BS) && length){
      number /= 0x10;
      length--;
      UART_OutChar(character);
    }

    character = UART_InChar();
  }
  return number;
}

void UART_OutUHex(uint32_t number){
  if(number >= 0x10){
    UART_OutUHex(number / 0x10);
    UART_OutUHex(number % 0x10);
  }
  else{
    if(number < 0xA){
      UART_OutChar(number + '0');
    }
    else{
      UART_OutChar((number - 0x0A) + 'A');
    }
  }
}

void UART_InString(uint8_t *bufPt, uint16_t max){
  int length = 0;
  char character;

  character = UART_InChar();
  while(character != CR){
    if(character == BS){
      if(length){
        bufPt--;
        length--;
        UART_OutChar(BS);
      }
    }
    else if(length < max){
      *bufPt = character;
      bufPt++;
      length++;
      UART_OutChar(character);
    }
    character = UART_InChar();
  }
  *bufPt = 0; // null terminator

}

void UART_OutSDec(int32_t n) {
    if (n < 0) {
        UART_OutChar('-');
        n = -n;
    }
    UART_OutUDec((uint32_t)n);
}