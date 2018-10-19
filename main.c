#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "timers.h"
#include "stdio.h"
#include "stdint.h"
#include "tm4c123gh6pm.h"
#include "functions.h"
#include <string.h>

#define RED 0x02
#define BLUE 0x04
#define GREEN 0x08

p trig = PB2;
p echo = PB3;
p smoke = PA7;
uint32_t SystemCoreClock = 16000000;

void PORTF_init(void)
{
	uint32_t delay;
	SYSCTL_RCGCGPIO_R |= 0x20;             //clk reg
	delay = 1 + 2 ;                        //dummy variable
	//while ((SYSCTL_PRGPIO_R & 0x20)==0) {};
	GPIO_PORTF_LOCK_R  = 0x4C4F434B ;      //magic value to unlcok
	GPIO_PORTF_CR_R    = 0x1F;             //commit reg
	GPIO_PORTF_DIR_R  |= 0x0E;             //direction
	//GPIO_PORTF_AFSEL_R = 0;                //alternative fn
	//GPIO_PORTF_PCTL_R  = 0;                //port control "to select alternative fn"
	//GPIO_PORTF_AMSEL_R = 0;                //analog mode sel
	GPIO_PORTF_PUR_R  |= 0x11;             //pull up 
	//GPIO_PORTF_PDR_R  |= 0x11;           //pull down
	GPIO_PORTF_DEN_R  |= 0x1F;             //digital enable
	GPIO_PORTF_DATA_R &= 0xF1;             //turn off leds
}

void ADC_init()
{
	SYSCTL_RCGCADC_R |= 0x3;
	SYSCTL_RCGCGPIO_R |= 0x10;            //10000
	uint32_t dummy ;
  dummy=2;
	GPIO_PORTE_AFSEL_R |= 0x8;            //1000
	GPIO_PORTE_DEN_R  &= ~0x8;
	GPIO_PORTE_AMSEL_R |= 0x08;
	GPIO_PORTE_DIR_R  &= ~0x08;

	ADC0_ACTSS_R &= ~8; 
	ADC0_EMUX_R = ~0xF000;
	ADC0_SSMUX3_R = 0;
	ADC0_SSCTL3_R |= 6;
	ADC0_ACTSS_R |= 8;
	
}
void UART0_init()
{
	uint32_t delay;
	SYSCTL_RCGCUART_R |= 1;               //enable clk to UART module
	SYSCTL_RCGCGPIO_R |= 1;               //enable clk to GPIO module
	delay = 1 + 2;
	GPIO_PORTA_AFSEL_R |= 0x3;            //enable altrenativ function on PA0 and PA1
	GPIO_PORTA_PCTL_R |= 0x11;            //select UART alternative functions for PA0,PA1
	GPIO_PORTA_DEN_R  |= 0x3;             //enable digital for PA0 and PA1
	
	UART0_CTL_R &= ~ (0x1);
	//UART0_IBRD_R = 104;                   //baud rate = 16000000 / (16*9600) = 104.166667 
	//UART0_FBRD_R = 11;                    //(int) (0.166667*64+0.5) = 11
	UART0_IBRD_R = 8;
	UART0_FBRD_R = 44;


	// 11xxxxx for 8 bits, 10xxxxx for 7 bits, 01xxxxx for 6 bits, 00xxxxx for 5 bits 
	// bit7 = 0 stick parity disabled
	// bit4 = 0 fifo disabled
	// bit3 = 0 one stop bit
	// bit2 = 0 odd parity 
	// bit1 = 0 parity disabled
	// bit0 = 0 normal use
	UART0_LCRH_R = 0xE0;       

	UART0_CC_R = 0;                       // use system clk which is 16 000 000
	UART0_CTL_R = 0x301;                  //bit0 -> enable, bit8 -> enable Tx, bit9 -> enable Rx  
}

void UART1_init()
{
	uint32_t delay;
	SYSCTL_RCGCUART_R |= 2;               //enable clk to UART module
	SYSCTL_RCGCGPIO_R |= 2;               //enable clk to GPIO module
	delay = 1 + 2;
	GPIO_PORTB_AFSEL_R |= 0x3;            //enable altrenativ function on PB0 and PB1
	GPIO_PORTB_PCTL_R |= 0x11;            //select UART alternative functions for PB0,PB1
	GPIO_PORTB_DEN_R  |= 0x3;             //enable digital for PB0 and PB1
	
	UART1_CTL_R &= ~ (0x1);
	//UART1_IBRD_R = 104;                   //baud rate = 16000000 / (16*9600) = 104.166667
	//UART1_FBRD_R = 11;                    //(int) (0.166667*64+0.5) = 11
	UART1_IBRD_R = 8;
	UART1_FBRD_R = 44;	
	
	// 11xxxxx for 8 bits, 10xxxxx for 7 bits, 01xxxxx for 6 bits, 00xxxxx for 5 bits 
	// bit7 = 0 stick parity disabled
	// bit4 = 0 fifo disabled
	// bit3 = 0 one stop bit
	// bit2 = 0 odd parity 
	// bit1 = 0 parity disabled
	// bit0 = 0 normal use
	UART1_LCRH_R = 0xE0;       

	UART1_CC_R = 0;                       // use system clk which is 16 000 000
	UART1_CTL_R = 0x301;                  //bit0 -> enable, bit8 -> enable Tx, bit9 -> enable Rx  
}

char UART0_read(void)
{
	while((UART0_FR_R & 0x10) != 0);      //Rx FIFO empty
	return (char) (UART0_DR_R & 0xFF);
}

void UART0_print(char c)
{
	while((UART0_FR_R & 0x20) != 0);      //Tx FIFO full
	UART0_DR_R = (char)c;
}

void UART0_print_string(char* string)
{
	while(*string)
	{
		UART0_print((char)*(string++));
	}
}

char UART1_read(void)
{
	while((UART1_FR_R & 0x10) != 0);      //Rx FIFO empty
	return (char) (UART1_DR_R & 0xFF);
}

void UART1_print(char c)
{
	while((UART1_FR_R & 0x20) != 0);      //Tx FIFO full
	UART1_DR_R = (char)c;
}

void UART1_print_string(char* string)
{
	while(*string)
	{
		UART1_print((char)*(string++));
	}
}

void AT_READ()
{
		char c;
		for(c=UART1_read(); c!='\n'; c=UART1_read())
		{	
			UART0_print(c);
		}
		UART0_print(c);
		
		for(c=UART1_read(); c!='\n'; c=UART1_read())
		{	
			UART0_print(c);
		}
		UART0_print(c);
		
		for(c=UART1_read(); c!='\n'; c=UART1_read())
		{	
			UART0_print(c);
		}
		UART0_print(c);
}

void delay(uint32_t x)
{
		uint32_t i;
		for(i=0; i<x; i++);
}

void AT_RUNSERVER()
{
	delay(10000);
	
	UART1_print_string("AT+CIPMUX=1");
	UART1_print_string("\r\n");
	AT_READ();
	
	delay(10000);

	UART1_print_string("AT+CIPSERVER=1,80");
	UART1_print_string("\r\n");
	AT_READ();
	
	delay(10000);
}

void AT_SEND(int num, char* string)
{
	char s[50];
	sprintf(s,"AT+CIPSEND=0,%d",num);
	UART1_print_string(s);
	UART1_print_string("\r\n");
	delay(10000);
	UART1_print_string(string);
}


void Task1(void)
{
	for(;;)
	{
		volatile TickType_t current = xTaskGetTickCount();
		*trig = 0;
		while(xTaskGetTickCount()-current < 1);
		current = xTaskGetTickCount();
		*trig = 0xFF;
		while(xTaskGetTickCount()-current < 1);
		*trig = 0;
		while(*echo != 0);
		while(*echo == 0);
		current = xTaskGetTickCount();
		while(*echo != 0);
		current = xTaskGetTickCount() - current;
		current /= 5.77;	
		char str[5];
		sprintf(str, "%d", (int)current);
		UART0_print_string(str);
		UART0_print_string("\r\n");
		
		if(current < 10)
		{
			str[1]='\r';
			str[2]='\n';
			AT_SEND(3,str);
		}
		else if(current <100)
		{
			str[2]='\r';
			str[3]='\n';
			AT_SEND(4,str);
		}
		else
		{
			str[3]='\r';
			str[4]='\n';
			AT_SEND(5,str);
		}
		//AT_SEND(flbkmfb   2,"\r\n");
		//current = xTaskGetTickCount();
		vTaskDelay(100000);
		//while(xTaskGetTickCount()-current < 100000);
	}
}

void Task2(void)
{
	char str[12];
	for(;;)
	{
		ADC0_PSSI_R	|= 8;
		while((ADC0_RIS_R) == 0);
		sprintf(str, "%d", (int)(ADC0_SSFIFO3_R));
		ADC0_ISC_R = 8;
		UART0_print_string(str);
		UART0_print_string("\r\n");
		//UART0_print_string("11111");
		//AT_SEND(strlen(str),str);
	}
}

void Task3(void)
{
	for(;;)
	{
		
		if(*smoke == 0)
		{
			UART0_print_string("7are2aaaaaaaaa ");
		  UART0_print_string("\r\n");}
		else
		{
			UART0_print_string("tmam el7  ");
		  UART0_print_string("\r\n");
		}
		
		//AT_SEND(flbkmfb   2,"\r\n");
		//current = xTaskGetTickCount();
		//vTaskDelay(100000);
		//while(xTaskGetTickCount()-current < 100000);
	}
}

int main()
{
	UART0_init();
	UART1_init();
	pinMode(trig,1);
	pinMode(echo,0);
	pinMode(smoke,0);
	AT_RUNSERVER();
	ADC_init();
	
	//xTaskCreate(Task1, "My Task1", 256, NULL, 3, NULL);
	xTaskCreate(Task1, "My Task1", 256, NULL, 3, NULL);
	//xTaskCreate(vPeriodicTask2, "My Task2", 256, NULL, 2, NULL);
	// Startup of the FreeRTOS scheduler.  The program should block here.
	vTaskStartScheduler();
	
	// The following line should never be reached.  Failure to allocate enough
	//	memory from the heap would be one reason.
	for (;;);
}