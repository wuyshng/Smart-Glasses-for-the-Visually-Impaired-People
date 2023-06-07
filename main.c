#ifndef F_CPU
#define F_CPU 16000000
#endif

#include <io.h>
#include <delay.h>
#include <stdio.h>
#include <string.h>
#include <mega328p.h>

#include "include/DFPlayer.h"
#include "include/328P_USART.h"


#define trigLeft  PORTB.3  // D11
#define echoLeft  PINB.0   // D8
#define trigMid   PORTB.4  // D12
#define echoMid   PINB.1   // D9
#define trigRight PORTB.5  // D13
#define echoRight PINB.2   // D10
#define led1      PORTD.4
#define led2      PORTD.5

unsigned int leftSensor, midSensor, rightSensor;
unsigned int count, countH, countL;
unsigned int distance, time;
unsigned int echo;


// Pin change 0-7 interrupt service routine
interrupt [PC_INT0] void pin_change_isr0(void)
{
   echo = echoMid + echoLeft + echoRight;
   if(echo == 1)
   {
      TCNT1H = TCNT1L = 0;
      TCCR1B = 0x03;
   }
   else
   {      
      countL = TCNT1L;
      countH = TCNT1H;
      count = countH*256 + countL;      //tinh so Timer dem duoc      
      time = count * 4;    //prescaler = 64 => Thoi gian thuc hien 1 xung la 4us
      distance = time / 58;      //cm 
      TCNT1H = TCNT1L = 0;
      TCCR1B = 0;    
   } 
}

void initPort()
{
   // Input/Output Ports initialization
   // Port B initialization
   // Function: Bit7=In Bit6=In Bit5=Out Bit4=Out Bit3=Out Bit2=In Bit1=In Bit0=In 
   DDRB=(0<<DDB7) | (0<<DDB6) | (1<<DDB5) | (1<<DDB4) | (1<<DDB3) | (0<<DDB2) | (0<<DDB1) | (0<<DDB0);
   // State: Bit7=T Bit6=T Bit5=0 Bit4=0 Bit3=0 Bit2=T Bit1=T Bit0=T 
   PORTB=(0<<PORTB7) | (0<<PORTB6) | (0<<PORTB5) | (0<<PORTB4) | (0<<PORTB3) | (0<<PORTB2) | (0<<PORTB1) | (0<<PORTB0);

   // Port C initialization
   // Function: Bit6=In Bit5=In Bit4=In Bit3=In Bit2=In Bit1=In Bit0=In 
   DDRC=(0<<DDC6) | (0<<DDC5) | (0<<DDC4) | (0<<DDC3) | (0<<DDC2) | (0<<DDC1) | (0<<DDC0);
   // State: Bit6=T Bit5=T Bit4=T Bit3=T Bit2=T Bit1=T Bit0=T 
   PORTC=(0<<PORTC6) | (0<<PORTC5) | (0<<PORTC4) | (0<<PORTC3) | (0<<PORTC2) | (0<<PORTC1) | (0<<PORTC0);

   // Port D initialization
   // Function: Bit7=In Bit6=In Bit5=In Bit4=In Bit3=In Bit2=In Bit1=In Bit0=In 
   DDRD=(0<<DDD7) | (0<<DDD6) | (1<<DDD5) | (1<<DDD4) | (0<<DDD3) | (0<<DDD2) | (0<<DDD1) | (0<<DDD0);
   // State: Bit7=T Bit6=T Bit5=T Bit4=T Bit3=T Bit2=T Bit1=T Bit0=T 
   PORTD=(0<<PORTD7) | (0<<PORTD6) | (0<<PORTD5) | (0<<PORTD4) | (0<<PORTD3) | (0<<PORTD2) | (0<<PORTD1) | (0<<PORTD0);
}

void initTimer()
{
   TCCR1A=(0<<COM1A1) | (0<<COM1A0) | (0<<COM1B1) | (0<<COM1B0) | (0<<WGM11) | (0<<WGM10);
   TCCR1B=(0<<ICNC1) | (0<<ICES1) | (0<<WGM13) | (0<<WGM12) | (0<<CS12) | (0<<CS11) | (0<<CS10);
   TCNT1H=0x00;
   TCNT1L=0x00;
   ICR1H=0x00;
   ICR1L=0x00;
   OCR1AH=0x00;
   OCR1AL=0x00;
   OCR1BH=0x00;
   OCR1BL=0x00;
}

void initExtInt()
{
   EICRA=(0<<ISC11) | (0<<ISC10) | (0<<ISC01) | (0<<ISC00);
   EIMSK=(0<<INT1) | (0<<INT0);
   PCICR=(0<<PCIE2) | (0<<PCIE1) | (1<<PCIE0);
   PCMSK0=(0<<PCINT7) | (0<<PCINT6) | (0<<PCINT5) | (0<<PCINT4) | (0<<PCINT3) | (1<<PCINT2) | (1<<PCINT1) | (1<<PCINT0);
   PCIFR=(0<<PCIF2) | (0<<PCIF1) | (1<<PCIF0);
}

void delayMs(unsigned int ms)
{  
   while(ms--)
   {
      TCNT0 = 6;
      TCCR0B = 0x03;
      while(!(TIFR0&(1<<TOV0)));
      TIFR0 |= (1 << TOV0);   
      TCCR0B = 0;
   }
}
void delayUs(unsigned int us)
{
   while(us--)
   {
      TCNT0 = 240;
      TCCR0B = 0x01;
      while(!(TIFR0&(1<<TOV0)));
      TIFR0 |= (1 << TOV0);   
      TCCR0B = 0;
   }
}

void WarningLeft(unsigned int distance)
{
   if ( (distance > 475) && (distance <= 500) ){
     DFP_PlayTrackInRoot(1);
   } else if ( (distance > 450) && (distance <= 475) ){
     DFP_PlayTrackInRoot(2);
   } else if ( (distance > 425) && (distance <= 450) ){
     DFP_PlayTrackInRoot(3);
   } else if ( (distance > 4.0*100) && (distance <= 4.25*100) ){
     DFP_PlayTrackInRoot(4);
   } else if ( (distance > 3.75*100) && (distance <= 4.0*100) ){
     DFP_PlayTrackInRoot(5);
   } else if ( (distance > 3.5*100) && (distance <= 3.75*100) ){
     DFP_PlayTrackInRoot(6);
   } else if ( (distance > 3.25*100) && (distance <= 3.5*100) ){
     DFP_PlayTrackInRoot(7);
   } else if ( (distance > 3.0*100) && (distance <= 3.25*100) ){
     DFP_PlayTrackInRoot(8);
   } else if ( (distance > 2.75*100) && (distance <= 3.0*100) ){
     DFP_PlayTrackInRoot(9);
   } else if ( (distance > 2.5*100) && (distance <= 2.75*100) ){
     DFP_PlayTrackInRoot(10);
   } else if ( (distance > 2.25*100) && (distance <= 2.5*100) ){
     DFP_PlayTrackInRoot(11);
   } else if ( (distance > 2.0*100) && (distance <= 2.25*100) ){
     DFP_PlayTrackInRoot(12);
   } else if ( (distance > 1.75*100) && (distance <= 2.0*100) ){
     DFP_PlayTrackInRoot(13);
   } else if ( (distance > 1.5*100) && (distance <= 1.75*100) ){
     DFP_PlayTrackInRoot(14);
   } else if ( (distance > 1.25*100) && (distance <= 1.5*100) ){
     DFP_PlayTrackInRoot(15);
   } else if ( (distance > 1.0*100) && (distance <= 1.25*100) ){
     DFP_PlayTrackInRoot(16);
   } else if ( (distance > 0.75*100) && (distance <= 1.0*100) ){
     DFP_PlayTrackInRoot(17);
   } else if ( (distance > 0.5*100) && (distance <= 0.75*100) ){
     DFP_PlayTrackInRoot(18);
   } else if ( (distance > 0.25*100) && (distance <= 0.5*100) ){
     DFP_PlayTrackInRoot(19);
   } else if ( (distance >= 0) && (distance <= 25) ){ 
     DFP_PlayTrackInRoot(20);
   }   
   
   delay_ms(3000);
}

void WarningMid(unsigned int distance)
{
   if ( (distance > 475) && (distance <= 500) ){
     DFP_PlayTrackInRoot(21);
   } else if ( (distance > 450) && (distance <= 475) ){
     DFP_PlayTrackInRoot(22);
   } else if ( (distance > 425) && (distance <= 450) ){
     DFP_PlayTrackInRoot(23);
   } else if ( (distance > 4.0*100) && (distance <= 4.25*100) ){
     DFP_PlayTrackInRoot(24);
   } else if ( (distance > 3.75*100) && (distance <= 4.0*100) ){
     DFP_PlayTrackInRoot(25);
   } else if ( (distance > 3.5*100) && (distance <= 3.75*100) ){
     DFP_PlayTrackInRoot(26);
   } else if ( (distance > 3.25*100) && (distance <= 3.5*100) ){
     DFP_PlayTrackInRoot(27);
   } else if ( (distance > 3.0*100) && (distance <= 3.25*100) ){
     DFP_PlayTrackInRoot(28);
   } else if ( (distance > 2.75*100) && (distance <= 3.0*100) ){
     DFP_PlayTrackInRoot(29);
   } else if ( (distance > 2.5*100) && (distance <= 2.75*100) ){
     DFP_PlayTrackInRoot(30);
   } else if ( (distance > 2.25*100) && (distance <= 2.5*100) ){
     DFP_PlayTrackInRoot(31);
   } else if ( (distance > 2.0*100) && (distance <= 2.25*100) ){
     DFP_PlayTrackInRoot(32);
   } else if ( (distance > 1.75*100) && (distance <= 2.0*100) ){
     DFP_PlayTrackInRoot(33);
   } else if ( (distance > 1.5*100) && (distance <= 1.75*100) ){
     DFP_PlayTrackInRoot(34);
   } else if ( (distance > 1.25*100) && (distance <= 1.5*100) ){
     DFP_PlayTrackInRoot(35);
   } else if ( (distance > 1.0*100) && (distance <= 1.25*100) ){
     DFP_PlayTrackInRoot(36);
   } else if ( (distance > 0.75*100) && (distance <= 1.0*100) ){
     DFP_PlayTrackInRoot(37);
   } else if ( (distance > 0.5*100) && (distance <= 0.75*100) ){
     DFP_PlayTrackInRoot(38);
   } else if ( (distance > 0.25*100) && (distance <= 0.5*100) ){
     DFP_PlayTrackInRoot(39);
   } else if ( (distance >= 0) && (distance <= 25) ){ 
     DFP_PlayTrackInRoot(40);
   }   
   
   delay_ms(3000);
}

void WarningRight(unsigned int distance)
{
   if ( (distance > 475) && (distance <= 500) ){
     DFP_PlayTrackInRoot(41);
   } else if ( (distance > 450) && (distance <= 475) ){
     DFP_PlayTrackInRoot(42);
   } else if ( (distance > 425) && (distance <= 450) ){
     DFP_PlayTrackInRoot(43);
   } else if ( (distance > 4.0*100) && (distance <= 4.25*100) ){
     DFP_PlayTrackInRoot(44);
   } else if ( (distance > 3.75*100) && (distance <= 4.0*100) ){
     DFP_PlayTrackInRoot(45);
   } else if ( (distance > 3.5*100) && (distance <= 3.75*100) ){
     DFP_PlayTrackInRoot(46);
   } else if ( (distance > 3.25*100) && (distance <= 3.5*100) ){
     DFP_PlayTrackInRoot(47);
   } else if ( (distance > 3.0*100) && (distance <= 3.25*100) ){
     DFP_PlayTrackInRoot(48);
   } else if ( (distance > 2.75*100) && (distance <= 3.0*100) ){
     DFP_PlayTrackInRoot(49);
   } else if ( (distance > 2.5*100) && (distance <= 2.75*100) ){
     DFP_PlayTrackInRoot(50);
   } else if ( (distance > 2.25*100) && (distance <= 2.5*100) ){
     DFP_PlayTrackInRoot(51);
   } else if ( (distance > 2.0*100) && (distance <= 2.25*100) ){
     DFP_PlayTrackInRoot(52);
   } else if ( (distance > 1.75*100) && (distance <= 2.0*100) ){
     DFP_PlayTrackInRoot(53);
   } else if ( (distance > 1.5*100) && (distance <= 1.75*100) ){
     DFP_PlayTrackInRoot(54);
   } else if ( (distance > 1.25*100) && (distance <= 1.5*100) ){
     DFP_PlayTrackInRoot(55);
   } else if ( (distance > 1.0*100) && (distance <= 1.25*100) ){
     DFP_PlayTrackInRoot(56);
   } else if ( (distance > 0.75*100) && (distance <= 1.0*100) ){
     DFP_PlayTrackInRoot(57);
   } else if ( (distance > 0.5*100) && (distance <= 0.75*100) ){
     DFP_PlayTrackInRoot(58);
   } else if ( (distance > 0.25*100) && (distance <= 0.5*100) ){
     DFP_PlayTrackInRoot(59);
   } else if ( (distance >= 0) && (distance <= 25) ){ 
     DFP_PlayTrackInRoot(60);
   }   
   
   delay_ms(3000);
}

void main(void)
{
   // Crystal Oscillator division factor: 1
   #pragma optsize-
   CLKPR=(1<<CLKPCE);
   CLKPR=(0<<CLKPCE) | (0<<CLKPS3) | (0<<CLKPS2) | (0<<CLKPS1) | (0<<CLKPS0);
   #ifdef _OPTIMIZE_SIZE_
   #pragma optsize+
   #endif  
   
   initPort();
   initTimer();
   initExtInt(); 
   DFP_Initialize();
   //waiting for initing DFPlayer module
   delay_ms(2000);
   //set volume
   DFP_SetVolume(30);
   //set EQ
   DFP_SetEQ(DFP_EQ_NORMAL);
   //initLCD();
   // Globally enable interrupts
   #asm("sei")
   TCNT1H = TCNT1L = 0;
   
   while (1)
   {   

      trigLeft = 1;
      trigMid = 0;
      trigRight = 0;
      delayUs(15);
      trigLeft = 0; 
      delayMs(50);
      leftSensor = distance;      
      WarningLeft(leftSensor); 
      delay_ms(100); 
      //distance = 0; 
       
      trigMid = 1;
      trigLeft = 0;
      trigRight = 0;
      delayUs(15);
      trigMid = 0;
      delayMs(50);
      midSensor = distance;
      WarningMid(midSensor); 
      //WarningMid(10);
      delay_ms(100);
      //distance = 0;
      
      trigRight = 1;
      trigLeft = 0;
      trigMid = 0;
      delayUs(15);
      trigRight = 0;
      delayMs(50);
      rightSensor = distance;
      WarningRight(rightSensor);
      delay_ms(100);
     // distance = 0; 
      /*
      if(leftSensor <= 10)
      {
         led1 = 0;
         led2 = 0;  
         //DFP_PlayTrackInRoot(20);
         //delay_ms(2000) ;
      }      
      else if( (leftSensor > 10) && (leftSensor <= 30) )
      {
         led1 = 0;
         led2 = 1;  
         //DFP_PlayTrackInRoot(19);
         //delay_ms(2000);
      }
      else if( (leftSensor > 30) && (leftSensor <= 50) )
      {
         led1 = 1;
         led2 = 0;   
         //DFP_PlayTrackInRoot(19);
         //delay_ms(2000) ;
      }
      else if( (leftSensor > 50) && (leftSensor <= 100) )
      {
         led1 = 1;
         led2 = 1; 
         //DFP_PlayTrackInRoot(17);
         //delay_ms(2000)  ;
      }
      else
      {
         led1 = ~led1;
         delayMs(200);
         led2 = 0;
         //DFP_PlayTrackInRoot(16);
         //delay_ms(2000)   ;
      }   */  
      //delay_ms(100);
      
   } 
     
}
