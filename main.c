/* Corbin Irvin 2011
 
 Changelog:
 10/26/2012 - CA Irvin
 Removed sequence properties
 Added serial routines
 */
//#include <msp430g2221.h>  
#include <msp430.h>
#include <signal.h>
#include <inttypes.h>

/* Function Prototypes */
uint8_t scanKeys(void);
void initIO(void);
static void __inline__ brief_pause(register unsigned int n);
void delayMs(uint32_t);

/* Definitions */
#define MODESW_MASK 0xC0
//port 2 definitions
#define LED_HEARTBEAT BIT6
#define BTN_PROGRA	BIT8
// port 1 definitions
#define EN_CH1	BIT0
#define EN_CH2	BIT1
#define EN_CH3	BIT2
#define EN_CH4	BIT3
#define BTNR1	BIT4
#define BTNR2	BIT5
#define BTNR3	BIT6
#define BTNC1	0x80;
//const int BTNC1 = BIT7;

/* macro to help us toggle bits on ports */
#define _togglePort(port,bit) P##port##OUT^= bit
#define _setPort(port,bit) P##port##OUT |= bit
#define _clearPort(port,bit) P##port##OUT &= ~bit

/* general bit manipulations */
#define _toggleBit(reg,bit) reg ^= bit  
#define _setBit(reg,bit) reg |= bit
#define _clearBit(reg,bit) reg &= ~##bit

/* global vars */
volatile uint32_t regTimer = 0;	// timer register, starts at 0 on startup, 500uS steps
volatile uint8_t	keyRegOld;
volatile uint8_t	keyReg;		// register that holds key state
volatile uint8_t	keyPulse;

volatile int		tapState =0;
volatile uint32_t delayTime = 500; // delay time in half milliseconds

void main(void)
{
	uint8_t keyreg = 0;
	int i =0;
	
	/* watchdog timer is pwm timebase */
	WDTCTL = WDTPW | WDT_MDLY_0_5;			// Set up watchdog to exp, 0.5ms
	IE1 |= WDTIE;							// enable wtd interrupt
  	
	/* configure the internal oscillator */
	BCSCTL1 = CALBC1_1MHZ_;	// set up for 1mhz stable operation without a crystal
	DCOCTL = CALDCO_1MHZ_;
  
	initIO();
	/* enable interrupts */
	//dint(); // opposite is 
	eint();
	
	/* switch to low power mode, act on interrupts */
	//LPM0;			
	
	while(1)
	{
		delayMs(500);
		_togglePort(2, LED_HEARTBEAT);
	
	}
}


/* 	
	this interrupt fires at 2KHz and does the pwm routine
*/
interrupt(WDT_VECTOR) WTD_interrupt(void)
{   
	uint32_t oldTime;
	// increment the time counter   	
	regTimer++;
	// update the keys
	keyRegOld = keyReg;
	keyReg = scanKeys();
	keyPulse = (keyReg ^ keyRegOld) & keyReg;

	
	// state machine for setting the LED tempo
	switch(tapState)
	{
		case 0:
		if(keyPulse & BIT5)
		{
			tapState = 1;
			oldTime = regTimer;
			_setPort(2, LED_HEARTBEAT);
		}
		break;
		
		case 1:
		if(keyPulse & BIT5)
		{
			tapState = 0;
			delayTime = regTimer - oldTime;
			_clearPort(2, LED_HEARTBEAT);
		}
		break;

		default:
		tapState = 0;
		break;
	}

	// Control the output channels, if a button is pressed, 
	// the channel must be bumped.
	if(keyReg & BIT3)
	{
		_setPort(1, EN_CH1);
	}
	else
	{
		_clearPort(1, EN_CH1);
	}
	if(keyReg & BIT0)
	{
		_setPort(1, EN_CH2);
	}
	else
	{
		_clearPort(1, EN_CH2);
	}
	if(keyReg & BIT4)
	{
		_setPort(1, EN_CH3);
	}
	else
	{
		_clearPort(1, EN_CH3);
	}
	if(keyReg & BIT1)
	{
		_setPort(1, EN_CH4);
	}
	else
	{
		_clearPort(1, EN_CH4);
	}


} 



/* initalizes the io ports */
void initIO(void)
{
	
	/* configure IO ports */
			
	P1SEL = 0x00;		// disable any special functions
	P2SEL = 0x00;
	
	P1DIR = 0x8F;		// set up the port directions
	P1REN = 0x70;		// enable internal resistors
	P1OUT = 0x70;		// set resistors to pullup
	
	P2DIR = BIT6 ;		// set up port directions
	P2REN = BIT7;		// enable internal resistors
	P2OUT = BIT7;		// set resistors to pullup
	
	return;
}

/*
	Scans the 6 input keys and updates a 8 bit value containing the button statuses
	BIT0	-	BTN2	-	CH2
	BIT1	-	BTN4	-	CH4
	BIT2	-	BTN6	-	STARTSTOP
	BIT3	-	BTN1	-	CH1
	BIT4	-	BTN3	-	CH3
	BIT5	-	BTN5	-	TAP
	BIT6	-	Reserved
	BIT7	-	Reserved
*/
uint8_t scanKeys(void)
{
	uint8_t keys=0;

	_clearPort(1, BTNC1);

	brief_pause(10000);

	keys = (P1IN & 0x70);

	keys = keys >> 3;

	_setPort(1, BTNC1);

	brief_pause(10000);

	keys |= (P1IN & 0x70);

	keys = keys >> 1;

	return  ((~keys)& 0x3f);
}

static void __inline__ brief_pause(register unsigned int n)
{
    __asm__ __volatile__ (
	"1: \n"
	" dec	%[n] \n"
	" jne	1b \n"
    : [n] "+r"(n));
}
	

void delayMs(uint32_t delay)
{
	uint32_t i ;
	i = regTimer + delay;
	while( regTimer < i ) nop();
	return;
}
