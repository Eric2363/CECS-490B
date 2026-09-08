#ifndef UART1_H_
#define UART1_H_

#include "tm4c123gh6pm.h"
#include <stdint.h>
#include <stdbool.h>

// standard ASCII symbols
#define CR   0x0D
#define LF   0x0A
#define BS   0x08
#define ESC  0x1B
#define SP   0x20
#define DEL  0x7F
#ifndef NULL
#define NULL 0
#endif

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

// UART1 RX=PB0, TX=PB1

void UART1_Init(bool RxInt, bool TxInt);
void UART1_OutCRLF(void);

uint8_t UART1_InChar(void);
void UART1_OutChar(uint8_t data);
void UART1_OutString(uint8_t *pt);

uint32_t UART1_InUDec(void);
void UART1_OutUDec(uint32_t n);
uint32_t UART1_InUHex(void);
void UART1_OutUHex(uint32_t number);
void UART1_InString(uint8_t *bufPt, uint16_t max);

bool UART1_Available(void);

void UART1_Handler(void);
void UART1_OutSDec(int32_t n);

#endif
