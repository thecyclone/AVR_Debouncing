#define F_CPU 16000000UL //16 MHz
#include <avr/io.h>
#include <util/delay.h>

// Defining Ports and Pins 
#define RS_port PORTB  //RS_bit is for register selection
#define RS_DDR DDRB
#define RS_bit PORTB0

#define E_bit PORTB1	//E_bit is the Enable bit
#define E_port PORTB
#define E_DDR DDRB

#define D4_DDR DDRD		//D4 pin connections
#define D4_port PORTD
#define D4_bit PORTD4

#define D5_DDR DDRD		//D5 pin connections
#define D5_port PORTD
#define D5_bit PORTD5

#define D6_DDR DDRD		//D6 pin connections
#define D6_port PORTD
#define D6_bit PORTD6

#define D7_DDR DDRD		//D7 pin connections
#define D7_port PORTD
#define D7_bit PORTD7

#define LED1 PORTB5
#define LED2 PORTB4

// Bit values
uint8_t line_one = 0x80;          //start of line 1
uint8_t line_two = 0xc0 ;		  //start of line 2
uint8_t Reset = 0b00110000;       //reset the lcd
uint8_t Set4_bit = 0x28;    //4 bit data, 2-line display, 5x7 font 
uint8_t Display_off = 0b00001000; //turn display off
uint8_t Display_on = 0x0C;  //turn display on 
uint8_t Entry_mode = 0b00000110;  //shift cursor from left to right
uint8_t clear = 0b00000001;		  //clear the lcd i.e replace all characters with ASCII space
uint8_t home = 0b00000010;		  //set cursor to home position i.e first position of first line
uint8_t cursor_set = 0b10000000;  //set cursor position

// Defining function prototypes
void print_string(char *);
void write_character(uint8_t);
void send_data(uint8_t);
void initialise(void);
void write_instruction(uint8_t);


int main()
{
	//4 data lines
	D7_DDR |= (1<<D7_bit); //Sets PD7 as output
	D6_DDR |= (1<<D6_bit); //Sets PD6 as output
	D5_DDR |= (1<<D5_bit); //Sets PD5 as output
	D4_DDR |= (1<<D4_bit); //Sets PD4 as output
	//For Debouncing
	DDRB |= 1 << 4; //Initialise PORTB PIn5 as output
	DDRC |= 1 << 1; //Initialise PORTC Pin1 as output
	DDRC |= 1 << 0; //Initialise PORTC Pin0 as output
	PORTC ^= 0 << 1; // Set Pin1 PORTC as 0 to give ground to LED2
	PORTC ^= 0 << 0; // Set Pin0 PORTC as 0 to give ground to LED1
	PORTB ^= 1 << LED1; //Turn on LED at PORTB Pin5
	DDRB |= 1 << 4; //Initialise PORTB PIn4 as output
	1 << PORTB3; //Set PINB3 to a high for reading button 1
	1 << PORTB2; //Set PINB2 to high for reading button 2
	int Button1 = 0;//counter for button1
	int Button2 = 0;//counter for button2
	char num1[2];//number to be printed on line 1
	char num2[2];//number to be printed on line 2
	/* Logic : When button is pressed, it connects the pin to ground i.e 0. So we will check if the pin is grounded with the help of bit_is_clear function
	   Then we will wait for 100ms so that the button can be released. After which we will turn on the led*/
	E_DDR |= (1<<E_bit);	//sets enable pin as output
	RS_DDR |= (1<<RS_bit);	//sets Register_select pin as output

	initialise();
	write_instruction(line_one);
		_delay_ms(4);
		print_string("0");		       //Prints Name1
		_delay_ms(4);
		write_instruction(line_two);
		_delay_ms(4);
		print_string("0");		       //Prints Name2
		_delay_ms(4);				   //Wait for 2s
	while(1)
	{	
		
		if (bit_is_clear(PINB,3))//check if button1 is pressed
			{
				_delay_ms(100);//wait for user to release the button
				if (!bit_is_clear(PINB,3))//check if the button1 is released
					{
						PORTB ^= 1<<LED1;//turn on the LED1
						Button1 += 1;
					}
			}
		else if (bit_is_clear(PINB,2))//check if button2 is pressed
			{
				_delay_ms(100);//wait for user to release the button
				if (!bit_is_clear(PINB,2))//check if button2 is released
					{
						PORTB ^= 1<<LED2;//turn on the LED2
						Button2 -= 1;
					}
			}
		itoa(Button1,num1,10);//convert interger "button1" to string "num1". 10 is the radix
		itoa(Button2,num2,10);//convert interger "button1" to string "num1". 10 is the radix
		write_instruction(line_one);
		_delay_ms(4);
		print_string(num1);		       //Prints Name1
		_delay_ms(4);
		write_instruction(line_two);
		_delay_ms(4);
		print_string(num2);		       //Prints Name2
		_delay_ms(4);				   //Wait for 2s

		

	}
	return 0;
}

void initialise(void) 				//make the LCD ready for 4-bit data interface
{
	_delay_ms(20);   				//Power-up delay
	RS_port &= ~(1<<RS_bit);		//RS-bit low i.e selecting the Instruction Register
	E_port &= ~(1<<E_bit);			//Enable low

	//3-step reset sequence as mentioned in the Instruction manual

	send_data(Reset);				//Step-1
	_delay_ms(5);

	send_data(Reset);				//Step-2
	_delay_us(200);

	send_data(Reset);				//Step-3
	_delay_us(200);

	send_data(Set4_bit);			//4-bit interface set-up
	_delay_ms(5);					//Data up-time

	write_instruction(Set4_bit);	//Set lines,font and mode
	_delay_ms(5);					//Data up-time

	write_instruction(Display_off);	//Turn off the display
	_delay_ms(5);					//Delay

	write_instruction(clear);		//Clear everyhthing from screen i.e empty the display RAM
	_delay_ms(4);					//Delay

	write_instruction(Entry_mode);	//Instruct the chip how you want to enter the data i.e left-to-right / right-to-left
	_delay_us(80);					//We have chosen left-to-right mode i.e the cursor moves from left-to-right

	write_instruction(Display_on);	//Finally, turn the display-on so that we can start printing the data
	_delay_us(80);					//Delay
}

void print_string(char String[])	//Print a string on the LCD 
{
	volatile int a = 0;				//create a volatile integer a and initialise it with 0
	while(String[a] != 0)			// Check if the string is not empty or we have reached the end of the string
	{
		write_character(String[a]); //Write the character at the index a of the string
		a += 1;						//increase the value of a by 1 i.e move to the next character(if any) in the string
		_delay_ms(5);				//Data set-up time
	}
}

void write_character(uint8_t character)	//Sends a byte of information to the LCD data register
{
	RS_port |= (1<<RS_bit);				//RS-bit high i.e selecting the Data Register
	E_port |= (1<<E_bit);				//Enable low
	send_data(character);				//Write upper-4 digits of data
	//Since we have only 4-bit interface, we can only send 4-bits at a time but the data is 8-bits. So we right shift the data and send the lower 4-bits
	send_data(character << 4);			//Right shift by 4 bits and write the lower 4 bits of data
}

void write_instruction(uint8_t Instruction) //Sends a byte of information to LCD instruction register
{
	RS_port &= (0 << RS_bit);				//RS-bit low i.e selecting the Instruction Register
	E_port &= (0 << E_bit);				//Enable low
	send_data(Instruction);					//Write upper-4 digits of data
	//Since we have only 4-bit interface, we can only send 4-bits at a time but the data is 8-bits. So we right shift the data and send the lower 4-bits
	send_data(Instruction << 4);			//Right Shift by 4 bits and write the lower 4 bits of data
}

void send_data(uint8_t Byte)				//Sends a byte of information to the LCD
{
	PORTD = Byte;

	E_port |= (1<<E_bit);					//Enable pin-high
	_delay_us(1);							//Data set-up time
	E_port &= ~(1<<E_bit);					//Enable pin-low
	_delay_us(1);							//Data set-up time
}