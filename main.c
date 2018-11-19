#define F_CPU 2000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "lcd4bit.h"
#define degree_sysmbol 0xdf
unsigned int val=0;
unsigned int val1=0;
unsigned int val2=0;
unsigned int disp=0;
void initTimer0CTC(void);
void initExtrnalInt(void); 
void reset(void);
unsigned int r=1;
unsigned int dChanger=1;
unsigned long time=0;
unsigned int targetTemp = 15;

////FOR TEMP SENSOR////
void ADC_Init(){										
	ADMUX= 1<<REFS0 | 1<<1; //PA2 ready as ADC and making 5v
	ADCSRA= 1<<ADEN |(0x07);// auto trigger on
}
////FOR TEMP SENSOR////
int ADC_Read(char channel){
	ADCSRA = ADCSRA | 1<<ADSC;//start the conversion
	while(ADCSRA & (1<<ADSC));//wait until conversion is finished
	return ADCL | (ADCH<<8);//return the converted value              
} 
int main(void){
	
	char Temperature[10];
	float celsius;
	LCDInit(1);
	ADC_Init(); 
	DDRD = DDRD & ~(1<<6) ; // Make pin 6 of port D as a input
	DDRD = DDRD & ~(1<<7) ;
	DDRD = DDRD & ~(1<<5) ;
	DDRD = DDRD & ~(1<<4) ;
	DDRD = DDRD | 0b00000001;
	PORTD = 0x01;
	
	DDRA = DDRA & ~(1<<4) ;
	DDRA = DDRA & ~(1<<3) ;
	
	while(1){
	initExtrnalInt();
	initTimer0CTC();
	
	if(PIND & (1<<6) ){
	
		if(PINA & (1<<4) ){
			targetTemp++;
			_delay_ms(350);
		}
		if(PINA & (1<<3) ){
			if(targetTemp!=0){
				targetTemp--;
				_delay_ms(350);
			}
		}
	
	
		LCDClear();
	
		LCDWriteStringXY(1,0,"Temp");
		celsius = (ADC_Read(0)*4.88);
	   celsius = (celsius/10.00);
	   if( (int)celsius > targetTemp ){
			PORTD = 0x01;
		   sprintf(Temperature,"%d%cC  ", (int)celsius, degree_sysmbol);/* convert integer value to ASCII string */
			LCDWriteStringXY(6,0,Temperature);/* send string data for printing */
		   LCDWriteStringXY(10,0,"|");
		   LCDWriteIntXY(11,0,targetTemp,2);	//changing temp   
			_delay_ms(1000);
		}
		else{
			PORTD = 0x00;
		}
	
	}else{
		
		if(PIND & (1<<4) ){
			time++;
			_delay_ms(350);
		}
		if(PIND & (1<<5) ){
			if(time!=0){
				time--;
				_delay_ms(350);
			}
		}
		
		LCDClear();
		
		LCDWriteIntXY(0,1,(((disp+val2)%100000)/10)/100,2);
		LCDWriteStringXY(2,1,".");
		LCDWriteIntXY(3,1,(((disp+val2)%100000)/10)%100,2);
		LCDWriteStringXY(5,1,".");
		LCDWriteIntXY(6,1,(disp+val2)%10,1);
		if(time!=0){
		LCDWriteStringXY(2,0,"Timer Started");
			if(val2==time*1000){
				
				r=1;
				val2=0;
				disp=0;
				LCDWriteStringXY(4,0,"Time up!");
			}
		}
		LCDWriteStringXY(9,1,"|");
		LCDWriteIntXY(10,1,time,2);
		LCDWriteStringXY(12,1,"Min");
		_delay_ms(100);
		 memset(Temperature,0,10);
	
	}
	
	//decodeSSD(disp+val2);
   	// dispSSD(50);
	}   
}

////FOR TIMER////
ISR(INT0_vect){

	if(r==1){
		r=2;
	}
	else if(r==2){
		r=1;
	}
	
}
////FOR TIMER////
void initExtrnalInt(){
	MCUCR=(1<<ISC11)|(1<<ISC10)|(1<<ISC01)|(1<<ISC00);
	GICR=(1<<INT1)|(1<<INT0);
	sei();
}
////FOR TIMER////
ISR(INT1_vect){

	val2=0;
	disp=0;
	
}
////FOR TIMER////
void initTimer0CTC(void){
	TCCR0=(1<<CS01)|(1<<WGM01);//enable CTC mode and prescaling to 8
	TCNT0=0; // start the timer
	OCR0=250;//stop at 250. the comparing value. The maximum value TCCR0 can take
	if(r==2)TIMSK |= (1 << OCIE0);//enable the interrupt to be called. so this should be handled
	else if(r==1)TIMSK =0;
	sei();
}
////FOR TIMER////
ISR (TIMER0_COMP_vect){//ISR- interrupt service routine
	val++;
	
	if(val==100){
	disp++;
	val=0;
	}
	if(disp%600==0&&disp!=0){
	val2+=1000;
	disp=0;
	}
}



