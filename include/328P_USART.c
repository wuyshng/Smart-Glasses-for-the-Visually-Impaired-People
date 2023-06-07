#ifndef F_CPU
#define F_CPU 16000000
#endif

#include <io.h>

#include <stdint.h>
#include <string.h>
#include "328P_USART.h"
#include <delay.h>
#define  _BV(bit) (1<<(bit))

volatile usart * const usart0 = (void*)USART0_REG;
volatile unsigned char usart0_rx_flag = 0;

//ISR(USART_RX_vect)
interrupt [USART_RXC] void usart_rx_isr(void)
{
	/* Disable Rx Interrupt */
	//usart0 -> ucsr_b &= ~_BV(RXCIE0);
    UCSR0B &= ~_BV(RXCIE0);
	usart0_rx_flag = 1;
}

void USART_Init()
{
    uint16_t baudrate;
	/* Enable U2X0 */
	//usart0->ucsr_a |= _BV(U2X0);
    UCSR0A |= _BV(U2X0);
	
	/* 115200 Baud rate, UBRR = (fosc / (8 * BAUD)) - 1 */
	/* 16MHz / (8 * 9600) - 1 = 16 */
	baudrate = F_CPU / 8 / 9600 - 1; 
    
	//usart0->reserved = (baudrate >> 8) & 0xFF;
   // Reserved = = (baudrate >> 8) & 0xFF;
   
	//usart0->ubrr_l = baudrate & 0xFF;
    UBRR0L = baudrate & 0xFF;
	
	/* 8-N-1 */		
	//usart0->ucsr_c |= _BV(UCSZ01) | _BV(UCSZ00);
    UCSR0C |=  _BV(UCSZ01) | _BV(UCSZ00);
	
	/* Enable Transmit + Receive */
	//usart0->ucsr_b |= _BV(RXEN0) | _BV(TXEN0);
    UCSR0B |= _BV(RXEN0) | _BV(TXEN0);
}

bool USART_RxByte(unsigned char *data)
{	
	uint16_t loop = USART_TIMEOUT;
	do 
	{
		if(/*usart0->ucsr_a*/ UCSR0A & _BV(RXC0))
		{
			*data = /*usart0->udr*/ UDR0;			
			return true;
		}
		delay_ms(1);		
	} while (--loop);
	
	return false;
}

void USART_RxByte_IT()
{
	/* Enable Rx Interrupt */	
	//usart0->ucsr_b |= _BV(RXCIE0);
    UCSR0B |= _BV(RXCIE0);
}

void USART_TxByte(unsigned char data)
{
	while((/*usart0->ucsr_a*/UCSR0A & _BV(UDRE0)) == 0);	
    //usart0->udr = data;
    UDR0 = data;	
}

bool USART_RxBuffer(unsigned char *buffer, unsigned short len)
{
    int i;
	for( i=0; i < len; i++)
	{
		if(!USART_RxByte(&buffer[i]))
		{
			return false;
		}
		
		/* DFPlayer dedicated code */		
		if(i == 2 && buffer[2] != 0x06) return false;
	}	
	
	return true;
}

void USART_TxBuffer(unsigned char *buffer, unsigned short len)
{
    int i;
	for(i=0; i < len; i++)
	{
		USART_TxByte(buffer[i]);
	}
}

bool USART_Available()
{
	if(/*usart0->ucsr_a*/UCSR0A & _BV(RXC0))
	{
		return true;
	}
	
	return false;
}