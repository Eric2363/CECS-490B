#include "tm4c123gh6pm.h"
#include <stdint.h>
#include <stdbool.h>  // for C boolean data type

// standard ASCII symbols
#define CR   0x0D
#define LF   0x0A
#define BS   0x08
#define ESC  0x1B
#define SP   0x20
#define DEL  0x7F
#define NULL 0


// Basic Colors
#define COLOR_BLACK        "\033[30m"
#define COLOR_RED          "\033[31m"
#define COLOR_GREEN        "\033[32m"
#define COLOR_YELLOW       "\033[33m"
#define COLOR_BLUE         "\033[34m"
#define COLOR_MAGENTA      "\033[35m"
#define COLOR_CYAN         "\033[36m"
#define COLOR_WHITE        "\033[37m"

// Bright Colors
#define COLOR_BRIGHT_BLACK   "\033[90m"
#define COLOR_BRIGHT_RED     "\033[91m"
#define COLOR_BRIGHT_GREEN   "\033[92m"
#define COLOR_BRIGHT_YELLOW  "\033[93m"
#define COLOR_BRIGHT_BLUE    "\033[94m"
#define COLOR_BRIGHT_MAGENTA "\033[95m"
#define COLOR_BRIGHT_CYAN    "\033[96m"
#define COLOR_BRIGHT_WHITE   "\033[97m"


#define NVIC_EN0_UART0 0x20     // UART0 IRQ number 5

// UART Functions
// --------------------

void UART_Init(bool RxInt, bool TxInt);
void OutCRLF(void);

uint8_t UART_InChar(void);
void UART_OutChar(uint8_t data);
void UART_OutString(uint8_t *pt);

uint32_t UART_InUDec(void);
void UART_OutUDec(uint32_t n);
uint32_t UART_InUHex(void);
void UART_OutUHex(uint32_t number);
void UART_InString(uint8_t *bufPt, uint16_t max);

// ring buffer helper API
bool UART_Available(void);

// ISR
void UART0_Handler(void);
void UART_OutSDec(int32_t n);