#include "stdint.h"
#include "tm4c123gh6pm.h"
#include "bit_specific_addressing.h"

#define p volatile uint32_t*

void pinMode(volatile uint32_t *, uint32_t);
void portMode(volatile uint32_t *, uint8_t);
void portSetOutputs(volatile uint32_t *, uint8_t);
void portSetInputs(volatile uint32_t *, uint8_t);
