#ifndef _328P_USART_H_
#define _328P_USART_H_

#include <io.h>
#include <interrupt.h>

#define USART0_REG		0xC0
#define USART_TIMEOUT	100	/* loop */

typedef enum
{
	false = 0,
	true = 1	
}bool;

typedef struct _usart
{
	unsigned char ucsr_a;		/* USART Control & Status Register A */
	unsigned char ucsr_b;		/* USART Control & Status Register B */
	unsigned char ucsr_c;		/* USART Control & Status Register C */
	unsigned char reserved;
	unsigned char ubrr_l;		/* USART Baud Rate Register Low */
	unsigned char ubrr_h;		/* USART Baud Rate Register High */
	unsigned char udr;		/* USART Data Register */
} usart;

void USART_Init(void);
bool USART_RxByte (unsigned char *data);
void USART_TxByte(unsigned char data);
void USART_RxByte_IT();
bool USART_RxBuffer(unsigned char *buffer, unsigned short len);
void USART_TxBuffer(unsigned char *buffer, unsigned short len);
bool USART_Available(void);

extern volatile usart * const usart0;
extern volatile unsigned char usart0_rx_flag;

#endif /* _328P_USART_H_ */