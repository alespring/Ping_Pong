/*

	Demo of glcd library with AVR8 microcontroller
	
	Tested on a custom made PCB (intended for another project)

	See ../README.md for connection details

*/

#include <avr/io.h>
#include "glcd/glcd.h"
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdlib.h>
//#include "glcd/fonts/Liberation_Sans15x21_Numbers.h"
#include "glcd/fonts/font5x7.h"
#include <avr/pgmspace.h>
#
#define F_CPU 16000000UL  // 1 MHz


/* Function prototypes */
static void setup(void);

static void setup(void)
{
	/* Set up glcd, also sets up SPI and relevent GPIO pins */
	glcd_init();
}

uint8_t ms, ms10,ms100,sec,min,entprell, state;

ISR (TIMER1_COMPA_vect)
{
	ms10++;
	if(entprell != 0)entprell--;
	if(ms10==10)	//10ms
	{
		ms10=0;
		ms100++;
	}
    if(ms100==10)	//100ms
	{
		ms100=0;
		sec++;
	}
	if(sec==10)	//Minute
	{
		sec=0;
		min++;
		if(state==11)state=10;
	}
}

/*void Flankentriggerung()
{
	TST_RT = (PIND & (1<<PD5))
	TST_GR = !(PIND & (1<<PD6))
	unsigned char	SIG = 0;
	unsigned char	oldSIG = 0;
	unsigned char	edgepos = 0;
	unsigned char	edgeneg =0;
	unsigned char	edge =0;
	
	//Flankentriggerung
	SIG = TST_GR || TST_RT;
	edge = SIG ^ oldSIG;
	edgepos = edge & SIG;
	edgeneg = edge & oldSIG;
	oldSIG = SIG;
}*/

unsigned char b = 0;
unsigned char z = 0;
unsigned char score = 0;
unsigned char l;
unsigned char Level;
unsigned char laenge_rect;

unsigned char x1=0;		//x Achse nullpunkt
unsigned char x2 = 83;	//x Achse endpunkt
unsigned char y1 = 0;	//y Achse nullpunkt
unsigned char y2 = 47;	//y Achse endepunkt
unsigned char y3 = 45;	//y Achse schlaeger

 struct Ball
 {
	char x, y;
	char dx, dy;
	char radius;
 };//ende struktur
struct Ball bale[4];
int a;
 
 
 void level()
 {
	unsigned char n;
	
	Level = (score / 5) + 1;
	
	if(Level <= 4){l = Level; laenge_rect = 15;}
	if(Level > 4) {n = (Level - 5); laenge_rect = ((Level - n)*2);}
 }
void balli()
{
	for(a=0;a<l;a++)
	{
		//abprallen bei erreichen nullpunkt oder endpunkt der x-Achse
		if ((bale[a].x == (x2 - bale[a].radius)) || (bale[a].x == (x1 + bale[a].radius))) bale[a].dx*=-1;//Richtungsänderung y Achse
		
		//abprallen bei erreichen nullpunkt oder endpunkt der y-Achse
		if ((bale[a].y == (y2 - bale[a].radius)) || (bale[a].y == (y1 + bale[a].radius))) bale[a].dy*=-1;//Richtungsänderung x Achse
		bale[a].x+=bale[a].dx;
		bale[a].y+=bale[a].dy;
		
		//abprallen bei erreichen des Balkens
		if(((bale[a].x >= z) && (bale[a].x <= (z +15))) && (bale[a].y == (y3 - bale[a].radius)))
		{
			bale[a].dy*=-1;//richtungsanderung y achse
			score+=1;//1 Treffer
		}
	}
} 
void schlaeger()
{
	unsigned char b;
	
	for( b= 0; b<7; b++)
	{
		if((!(PIND & (1<<PD5))) && (z < 69))z++;//Balnken nach rechts
		if((!(PIND & (1<<PD6))) && (z >  0))z--;//Balken nach links
		_delay_ms(5);
	}
}

void ausgabe()
{
	glcd_clear_buffer();//Display löschen
	char string [3];
	
	//Schläger
	glcd_draw_rect(z, 45, laenge_rect, 3, 1);
	
	//Score
	sprintf(string, "%d", score);//Score in string speichern
	glcd_draw_string_xy(20, 0, string);//String ausgeben
	
	//Level
	sprintf(string, "%d", Level);//Score in string speichern
	glcd_draw_string_xy(0, 0, string);//String ausgeben
	
	//laenge rect
	sprintf(string, "%d", laenge_rect);//Score in string speichern
	glcd_draw_string_xy(60, 0, string);//String ausgeben
	
	//Kreis
	for(a=0;a<l;a++)
	{
		glcd_draw_circle(bale[a].x, bale[a].y, bale[a].radius,1);//Kreis zeichnen
		glcd_fill_circle(bale[a].x, bale[a].y, bale[a].radius,1);//Kreis füllen
	}
	glcd_write();//Display schreiben
}

int main(void)
{
	
	/* Backlight pin PL3, set as output, set high for 100% output */
	DDRB |= (1<<PB2);
	//PORTB |= (1<<PB2);
	PORTB &= ~(1<<PB2);
	
	DDRC &= ~(1<<PC0); 	//Eingang Hallsensor
	PORTC |= (1<<PC0);	//Pullup Hallsensor einschalten
	
	DDRC |=(1<<PC1); 	//Eingang Hallsensor
	PORTC |= (1<<PC1);	//Pullup Hallsensor einschalten
	
	
	DDRD &= ~((1<<PD6) | (1<<PD2) | (1<<PD5)); 	//Taster 1-3
	PORTD |= ((1<<PD6) | (1<<PD2) | (1<<PD5)); 	//PUllups für Taster einschalten
	
	DDRD &= ~(1<<PD4); //T0 Counter Input
	TCCR0B |= (1<<CS02) | (1<<CS01) | (1<<CS00);//Counter 0 enabled clock on rising edge
	
	//Timer 1 Configuration
	OCR1A = 0x3D08;	//OCR1A = 0x3D08;==1sec
	
    TCCR1B |= (1 << WGM12);
    // Mode 4, CTC on OCR1A

    TIMSK1 |= (1 << OCIE1A);
    //Set interrupt on compare match

    TCCR1B |= (1 << CS12) | (1 << CS10);
    // set prescaler to 1024 and start the timer

    sei();
    // enable interrupts
	
	setup();
	
	for(a=0;a<4;a++)
	{
		bale[a].radius = rand()/(RAND_MAX/10);
		bale[a].x= bale[a].radius+rand()/(RAND_MAX/(x2 - (2 * bale[a].radius)));
		bale[a].y= bale[a].radius+rand()/(RAND_MAX/(y2 - (2 * bale[a].radius)));
		
		bale[a].dx=-1;
		bale[a].dy=-1;
		
	}//ende for
			
	glcd_tiny_set_font(Font5x7, 5, 7, 32, 127);
	
	while(1) 
	{
		
		
		PORTB = 0xFF;//Hintergrundlicht	
		level();
		balli();
		schlaeger();
		ausgabe();		
		//Score
		//printf("%d  ->x %d  y %d\n",ball1.x,ball1.y);
		
		//Kreis
		/*glcd_draw_circle(x,y,radius,1);//Kreis zeichnen
		glcd_fill_circle(x,y,radius,1);//Kreis füllen*/
	}//End of while
	
	return 0;
}//end of main
