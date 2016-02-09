#define F_CPU 16000000UL
#include <avr/io.h>
#include<avr/wdt.h>
#include<avr/sleep.h>
#include <util/delay.h>
#include<stdio.h>
#include<string.h>

#define    clear        0b00000001
#define    home        0b00000010
#define    entry        0b00000110
#define    dispoff        0b00001000
#define    dispon        0b00001100
#define    reset    0b00110000
#define    fourbit    0b00101000
#define cursor        0b10000000
#define lineone        0b00000000
#define linetwo        0b01000000

void writetolcd (char theByte)
{
    PORTD &= theByte & (0b11110000);        // Setting the PD7, PD6, PD5, PD4 
    PORTD |= theByte & (0b11110000);        // Setting the PD7, PD6, PD5, PD4 
    PORTB |= 0b00000010;
    _delay_us (1);
    PORTB &= 0b11111101;
    _delay_us (1);
}

void writeinstruction (char theInstruction)
{
    PORTB &= 0b11111110;                        // Rs Low
    PORTB &= 0b11111101;                        // Enable Low
    writetolcd (theInstruction);
    writetolcd (theInstruction << 4);
}

void lcd_init (void )
{
    _delay_ms (100);                               
    
    PORTB &= 0b11111110;                        // RS = low
    PORTB &= 0b11111101;                        // EN = 0

    writeinstruction (reset);
    _delay_ms (10);                            // First part of reset sequence

    writeinstruction (reset);
    _delay_us (200);                        // Second part of reset sequence

    writeinstruction (reset);
    _delay_us (200);                        // Third part of reset sequence
    
    writeinstruction (fourbit);            //4-bit mode
    _delay_us (80);                            

    writeinstruction (fourbit);            // Mode Set
    _delay_us (80);                           

    writeinstruction (dispoff);               
    _delay_ms (80);                           

    writeinstruction (clear);              
    _delay_ms (4);                            

    writeinstruction (entry);                
    _delay_us (80);                            

    writeinstruction (dispon);                
    _delay_us (80);                           
}

void characterwrite (char theData)
{
    PORTB |= 0b00000001;                        // RS = 1
    PORTB &= 0b11111101;                        // EN = 0
    writetolcd (theData);
    writetolcd (theData << 4);
}

void writetolcd_string (char theString[])
{
    volatile int i = 0;
    while (theString[i] != '\0')
    {
        characterwrite (theString[i]);            
        i ++;
        _delay_us (80);                        
    }
}

void wdt()
{
  MCUSR &= 0xF7;
  WDTCSR |= 0x18;// WatchDog Current Status Register  for entering kind of setup mode
  WDTCSR = 0x48;// for 16ms time - out
  sei(); // enabling interrupts
  sleep_cpu();
  WDTCSR = 0x00;
}

void Interrupt_setup() // for INT1 & INT0
{
  cli();
  EIMSK |= (1 << INT0); // Enable externaal interrupt INT0 
  EIMSK |= (1 << INT1);  // Enable externaal interrupt INT1
  EICRA |= (0 << ISC00);    // Trigger INT0 on falling edge
  EICRA |= (1 << ISC01);
  EICRA |= (0 << ISC10);    // Trigger INT1 on falling edge
  EICRA |= (1 << ISC11);
  sei();
}
void pincsetup() // for counter increment
{
  
  DDRB &= ~(1 << PB4); // 12 pin
  PORTB |= (1 << PB4); // for pulling up
  PCICR |= (1 << PCIE0);    // set PCIE0 to enable PCMSK0 scan
  PCMSK0 |= (1 << PCINT4);  // set PCINT0 to trigger an interrupt on state change 
  
  DDRC &= ~(1 << PC1); // ADC1 pin
  PORTC |= (1 << PC1); // for pulling up
  PCICR |= (1 << PCIE1);    // set PCIE1 to enable PCMSK0 scan
  PCMSK1 |= (1 << PCINT9);  // set PCINT9 to trigger an interrupt on state change 
  sei();
}
int counter1 = 0;
int counter2 = 0;
int check = 0;
char str[5];
int a=1;
int main(void)
{
    DDRD |= 0b11110000;    // 7,6,5,4 - 1 for lcd     3,2 - 0 for INT0,INT1
    DDRB |= 0b00001111;    // 8,9 - 1 for lcd    12 for  PC Interrupts      10 ,11 pin for LED
    PORTB |= 0b00001100;   // making 10,11 intially high  
    DDRC &= (0<<PC1);      // ADC1(PCINT9) for PC interrupt         
    lcd_init();          //initializing lcd as 4-bit mode
    Interrupt_setup();
    pincsetup();
    while(1)
    {
      writeinstruction(home);  
      _delay_ms(4);
      writeinstruction(clear);
      _delay_ms(4);
      sprintf(str,"%d",counter1);
      writetolcd_string(str);
      _delay_ms(4);
      writeinstruction(cursor|0x40);
      _delay_ms(4);
      sprintf(str,"%d",counter2);
      writetolcd_string(str);
      _delay_ms(4);
      sei();
    }
    return 0;
}
/////////////////////////////////////////////////////
ISR(INT0_vect) // connected to 2
{
  cli();
  check = 1;
  WDTCSR |= 0x18;// WatchDog Current Status Register  for entering kind of setup mode
  WDTCSR = 0x40;// for 16ms time - out
  sei(); // enabling interrupts
  sleep_cpu();
  sei();
}
ISR(INT1_vect) // connected to 3
{
  cli();
  check = 2;
  WDTCSR |= 0x18;// WatchDog Current Status Register  for entering kind of setup mode
  WDTCSR = 0x40;// for 16ms time - out
  sei(); // enabling interrupts
  sleep_cpu();
  sei();
}
/////////////////////////////////////////////////////////
ISR(PCINT0_vect) // connected to 12
{
  PORTB ^= 0b00001000; // pin 11 toggling
  cli();
  check = 3;
  WDTCSR |= 0x18;// WatchDog Current Status Register  for entering kind of setup mode
  WDTCSR = 0x40;// for 16ms time - out
  sei(); // enabling interrupts
  sleep_cpu();
  sei();
}
ISR(PCINT1_vect) // connected to ADC1
{
  cli();
  check = 4;
  WDTCSR |= 0x18;// WatchDog Current Status Register  for entering kind of setup mode
  WDTCSR = 0x40;// for 16ms time - out
  sei(); // enabling interrupts
  sleep_cpu();
  //PORTB ^= 0b00001000; // pin 11 toggling
  sei();
}
///////////////////////////////////////////////////////////
ISR(WDT_vect) // watchdog interrupt
{
  cli();
  wdt_reset(); // to stop watchdog from resetting the MCU (because in Interrupt & System Reset Mode)
  if(check == 1) // for Toggling led on 10 pin
  {
    uint8_t val = PIND;
    val &= 0b00000100;
    if(val == 0)
    {
     PORTB ^= 0b00000100; // pin 10 toggling 
    }
  }
  
  else if(check == 2) // for Toggling led on 11 pin
  {
    int val = PIND;
    val &= 0b00001000;
    if(val == 0)
    {
     PORTB ^= 0b00001000; // pin 11 toggling 
    }
  }
  else if(check == 3) // for incrementing counter1
  {
    int val = PINB;
    val &= 0b00010000; // choosing 12 pin  
    if(val == 0)
    {
     counter1++; // incrementing counter
    }
  }
  else if(check == 4)
  {
    int val = PINC;
    val &= 0b00000010;
    if(val == 0)
    {
     counter2--; // decrementing counter  
    }
  }
  WDTCSR = 0x00;
  sei();
  check = 0;
}

