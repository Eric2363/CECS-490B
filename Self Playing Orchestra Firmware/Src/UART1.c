#include "UART1.h"

#define RB_SIZE 64

typedef struct{
  volatile uint16_t head;
  volatile uint16_t tail;
  volatile uint8_t data[RB_SIZE];
} RingBuffer;

static RingBuffer UART1_RxRB;
static bool UART1_RxInterruptEnabled = true;

// =====================
// Ring Buffer Helpers
// =====================
static void UART1_rb_Init(RingBuffer *rb){
  rb->head = 0;
  rb->tail = 0;
}

static bool UART1_rb_empty(const RingBuffer *rb){
  return (rb->head == rb->tail);
}

static bool UART1_rb_put(RingBuffer *rb, uint8_t byte){
  uint16_t next = (rb->head + 1) % RB_SIZE;

  if(next == rb->tail){
    return false; // buffer full, byte dropped
  }

  rb->data[rb->head] = byte;
  rb->head = next;
  return true;
}

static bool UART1_rb_get(RingBuffer *rb, uint8_t *byte){
  if(UART1_rb_empty(rb)){
    return false;
  }

  *byte = rb->data[rb->tail];
  rb->tail = (rb->tail + 1) % RB_SIZE;
  return true;
}

bool UART1_Available(void){
  return !UART1_rb_empty(&UART1_RxRB);
}

// =====================
// UART1 Init @ 16MHz, 115200 baud
// UART1 RX=PB0, TX=PB1
// =====================
void UART1_Init(bool RxInt, bool TxInt){
  SYSCTL_RCGCUART_R |= 0x02; // activate UART1
  SYSCTL_RCGCGPIO_R |= 0x02; // activate Port B

  while((SYSCTL_PRGPIO_R & 0x02) == 0){
  }

  UART1_rb_Init(&UART1_RxRB);
  UART1_RxInterruptEnabled = RxInt;

  UART1_CTL_R = 0;
  UART1_IBRD_R = 8;
  UART1_FBRD_R = 44;
  UART1_LCRH_R = UART_LCRH_WLEN_8; // 8-bit, no FIFO
  UART1_ICR_R = 0x7FF;

  if(RxInt || TxInt){
    NVIC_PRI1_R = (NVIC_PRI1_R & ~0x00E00000) | 0x00A00000; // priority 5
    NVIC_EN0_R |= 0x00000040; // enable UART1 interrupt in NVIC;

    if(RxInt){
      UART1_IM_R |= UART_IM_RXIM;
    }

    if(TxInt){
      UART1_IM_R |= UART_IM_TXIM;
    }
  }

  UART1_CTL_R |= UART_CTL_RXE | UART_CTL_TXE | UART_CTL_UARTEN;

  GPIO_PORTB_AFSEL_R |= 0x03;
  GPIO_PORTB_DEN_R |= 0x03;
  GPIO_PORTB_PCTL_R = (GPIO_PORTB_PCTL_R & 0xFFFFFF00) + 0x00000011;
  GPIO_PORTB_AMSEL_R &= ~0x03;
}

// =====================
// UART1 ISR
// =====================
void UART1_Handler(void){
  if(UART1_RIS_R & UART_RIS_RXRIS){
    while((UART1_FR_R & UART_FR_RXFE) == 0){
      uint8_t ch = (uint8_t)(UART1_DR_R & 0xFF);
      UART1_rb_put(&UART1_RxRB, ch);
    }
    UART1_ICR_R = UART_ICR_RXIC;
  }

  if(UART1_RIS_R & UART_RIS_TXRIS){
    UART1_ICR_R = UART_ICR_TXIC;
  }
}

void UART1_OutCRLF(void){
  UART1_OutChar(CR);
  UART1_OutChar(LF);
}

uint8_t UART1_InChar(void){
  uint8_t ch;

  if(UART1_RxInterruptEnabled){
    while(!UART1_rb_get(&UART1_RxRB, &ch)){
    }
    return ch;
  }

  while((UART1_FR_R & UART_FR_RXFE) != 0){
  }
  return (uint8_t)(UART1_DR_R & 0xFF);
}

void UART1_OutChar(uint8_t data){
  while((UART1_FR_R & UART_FR_TXFF) != 0){
  }
  UART1_DR_R = data;
}

void UART1_OutString(uint8_t *pt){
  while(*pt){
    UART1_OutChar(*pt);
    pt++;
  }
}

uint32_t UART1_InUDec(void){
  uint32_t number = 0, length = 0;
  char character;

  character = UART1_InChar();
  while(character != CR){
    if((character >= '0') && (character <= '9')){
      number = 10 * number + (character - '0');
      length++;
      UART1_OutChar(character);
    }
    else if((character == BS) && length){
      number /= 10;
      length--;
      UART1_OutChar(character);
    }
    character = UART1_InChar();
  }
  return number;
}

void UART1_OutUDec(uint32_t num){
  if(num >= 10){
    UART1_OutUDec(num / 10);
    num = num % 10;
  }
  UART1_OutChar(num + '0');
}

uint32_t UART1_InUHex(void){
  uint32_t number = 0, digit, length = 0;
  char character;

  character = UART1_InChar();
  while(character != CR){
    digit = 0x10;

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
      UART1_OutChar(character);
    }
    else if((character == BS) && length){
      number /= 0x10;
      length--;
      UART1_OutChar(character);
    }

    character = UART1_InChar();
  }
  return number;
}

void UART1_OutUHex(uint32_t number){
  if(number >= 0x10){
    UART1_OutUHex(number / 0x10);
    UART1_OutUHex(number % 0x10);
  }
  else{
    if(number < 0xA){
      UART1_OutChar(number + '0');
    }
    else{
      UART1_OutChar((number - 0x0A) + 'A');
    }
  }
}

void UART1_InString(uint8_t *bufPt, uint16_t max){
  int length = 0;
  char character;

  character = UART1_InChar();
  while(character != CR){
    if(character == BS){
      if(length){
        bufPt--;
        length--;
        UART1_OutChar(BS);
      }
    }
    else if(length < max){
      *bufPt = character;
      bufPt++;
      length++;
      UART1_OutChar(character);
    }
    character = UART1_InChar();
  }
  *bufPt = 0;
}

void UART1_OutSDec(int32_t num){
  if(num < 0){
    UART1_OutChar('-');
    num = -num;
  }
  UART1_OutUDec((uint32_t)num);
}
