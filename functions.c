#include "functions.h"


#define fn1(X,NUM)                                 \
{                                                  \
	SYSCTL_RCGCGPIO_R |= NUM;                      \
	while ((SYSCTL_PRGPIO_R & NUM)==0) {};         \
	GPIO_PORT ## X ## _DIR_R   = mode;             \
	GPIO_PORT ## X ## _DEN_R   = 0xFF;             \
	break;                                         \
}                                                  \

#define fn2(X,NUM)                                 \
{                                                  \
	SYSCTL_RCGCGPIO_R |= NUM;                      \
	while ((SYSCTL_PRGPIO_R & NUM)==0) {};         \
	GPIO_PORT ## X ## _DIR_R  |= mode;             \
	GPIO_PORT ## X ## _DEN_R  |= mode;             \
	break;                                         \
}                                                  \

#define fn3(X,NUM)                                 \
{                                                  \
	SYSCTL_RCGCGPIO_R |= NUM;                      \
	while ((SYSCTL_PRGPIO_R & NUM)==0) {};         \
	GPIO_PORT ## X ## _DIR_R  &= (~mode);          \
	GPIO_PORT ## X ## _DEN_R  |= mode;             \
	break;                                         \
}                                                  \

#define create(NAME,X)                             \
void NAME(volatile uint32_t * px, uint8_t mode)    \
{                                                  \
	switch((uint32_t)px)                           \
	{                                              \
		case 0x40004000 : fn ## X(A , 0x1)         \
		case 0x40005000 : fn ## X(B , 0x2)         \
		case 0x40006000 : fn ## X(C , 0x4)         \
		case 0x40007000 : fn ## X(D , 0x8)         \
		case 0x40024000 : fn ## X(E , 0x10)        \
		case 0x40025000 : fn ## X(F , 0x20)        \
	}                                              \
}                                                  \

create(portMode,1)
create(portSetOutputs,2)
create(portSetInputs,3)





#define fn(X,NUM)                                \
{                                                \
	SYSCTL_RCGCGPIO_R |= NUM;                      \
	while ((SYSCTL_PRGPIO_R & NUM)==0) {};         \
	if(mode)GPIO_PORT ## X ## _DIR_R |= offset;    \
	else    GPIO_PORT ## X ## _DIR_R &= (~offset); \
	GPIO_PORT ## X ## _DEN_R  |= offset;           \
	break;                                         \
}                                                \


void pinMode(volatile uint32_t * px, uint32_t mode)       //for input  mode=0   for output  mode=1
{
	uint32_t port   = (uint32_t)px  & 0xFF000 ; 
	uint32_t offset = ((uint32_t)px & 0xFFF )/4;  
	switch(port)
	{
		case 0x4000  : fn(A , 0x1)      // port A
		case 0x5000  : fn(B , 0x2)      // port B
		case 0x6000  : fn(C , 0x4)      // port C
		case 0x7000  : fn(D , 0x8)      // port D
		case 0x24000 : fn(E , 0x10)     // port E
		case 0x25000 : fn(F , 0x20)     // port F
	}
}
