#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
/* Timer count = (required delay/Clock Time Period) - 1
Here we have delay = 200ms and CTP = 625*(10^(-10)) 
So timer count = (16*(10^6)/5) -1
= 3199999
but even with a timer1 that is 16 bit, we can only count to 65535.
So if we use a prescaler of 1024, the clock time period increases by 1024.
now timer count = 3125 -1 = 3124.
*/
ISR (TIMER1_COMPA_vect)
{
    0<<PB3;
}
int main(void)
{
	DDRB |= 1 << 5; //Initialise PORTB PIn5 as output
	DDRC |= 1 << 1; //Initialise PORTC Pin1 as output
	DDRC |= 1 << 0; //Initialise PORTC Pin0 as output
	PORTC ^= 0 << 1; // Set Pin1 PORTC as 0 to give ground to LED2
	PORTC ^= 0 << 0; // Set Pin0 PORTC as 0 to give ground to LED1
	PORTB ^= 1 << 5; //Turn on LED at PORTB Pin5
	DDRB |= 1 << 4; //Initialise PORTB PIn4 as output
	PORTB |= 1 << 3; //Set PINB3 to a high for reading button 1
	PORTB |= 1 << 2; //Set PINB2 to high for reading button 2
	int Button1 = 0;//counter for button1
	int Button2 = 0;//counter for button2
	char num1[2];//number to be printed on line 1
	char num2[2];//number to be printed on line 2
	/* Logic : When button is pressed, it connects the pin to ground i.e 0. So we will check if the pin is grounded with the help of bit_is_clear function
	   Then we will wait for 100ms so that the button can be released. After which we will turn on the led*/
	cli(); //disable global interrupt

    OCR1A = 0xC34; //OCR1A is an output compare register. It is constantly compared with a timer, which generates a PWM pulse
    // 0xC34 = 3124
    TIMSK1 |= _BV(OCIE1A); //Set interrupt on compare match
    TCCR1B |= _BV(WGM12); // Mode 4, CTC on OCR1A
    TCCR1B |= (1<<CS12) | (1<<CS10);
    /*We need a prescaler of 1024
    Therefore, CS10 = 1, CS11 = 0 and CS12 = 0*/

    sei(); // enable global interrupt

	while (1)
	{
		itoa(Button1,num1,10);//convert interger "button1" to string "num1". 10 is the radix
		itoa(Button2,num2,10);//convert interger "button1" to string "num1". 10 is the radix
		if (bit_is_clear(PINB,3))//check if button1 is pressed
			{
				_delay_ms(100);//wait for user to release the button
				if (!bit_is_clear(PINB,3))//check if the button1 is released
					{
						PORTB ^= 1<<5;//turn on the LED1
						Button1 += 1;
					}
			}
		else if (bit_is_clear(PINB,2))//check if button2 is pressed
			{
				_delay_ms(100);//wait for user to release the button
				if (!bit_is_clear(PINB,2))//check if button2 is released
					{
						PORTB ^= 1<<4;//turn on the LED2
						Button2 -= 1;
					}
			}
	}
}
