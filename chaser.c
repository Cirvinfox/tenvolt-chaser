/*
 * =====================================================================================
 *
 *       Filename:  chaser.c
 *
 *    Description: 
 *
 *        Changed: 2014-11-28
 *		
 *      Author(s):  Corbin Irvin (cirvinfox)
 *                  Olivia Theze (foxiepaws), fox@foxiepa.ws
 *
 * =====================================================================================
 */

/* 
 * (c) 2011 Corbin Irvin 
 *
 * Please see LICENSE for License details.
 *
 */

#include <msp430.h>
#include <signal.h>
#include <inttypes.h>
#include "chaser.h"

/* global vars */
volatile uint32_t   regTimer = 0;	 /* timer register, starts at 0 on startup, 500uS steps */
volatile uint8_t	keyRegOld;
volatile uint8_t	keyReg;          /* register that holds key state */
volatile uint8_t	keyPulse;

volatile int		tapState =0;
volatile int		heartbeatState;
volatile uint32_t   delayTime = 500; /* delay time in  milliseconds */
volatile uint32_t	heartbeatStartTime;
volatile uint32_t	oldTime;
volatile int		tapFlag = 0;

volatile uint8_t	programArray[MAXSTEPS] = { 0x01, 0x02, 0x04, 0x08};
volatile int		programStep = 0;
volatile int		programLength = 4;
volatile uint8_t	programState = 1;
volatile uint32_t	programCycleStartTime;

void main(void)
{
	uint8_t keyreg = 0;
	int i =0;
	
	/* watchdog timer */
	WDTCTL = WDTPW | WDTHOLD;	/* WDT disabled */			
  	
	/* configure the internal oscillator */
	BCSCTL1 = RSEL3 + RSEL2 + RSEL1 + RSEL0;	/* set up for 15.25MHz internal oscillator */
	DCOCTL = DCO2 + DCO1 + DCO0;

	/* set up inputs and outputs */
	initIO();

	/* configure timerA0 for 1ms interrupt rate */
	TA0CTL = TASSEL_2 + MC_1 + TACLR ; /* SMCLK source, no division, up to TACCR0, interrupts on */
	TACCTL0 = CCIE;	/* enable the interrupt on compare */
	CCR0 = 15250 - 1;	/* count requried for a 1ms interrupt rate */


	/* enable interrupts */
	/* dint(); */ /* opposite is  */
	eint();
	
	/* switch to low power mode, act on interrupts */
	/* LPM0; */			
	
	while(1)
	{
		delayMs(500);
	
	}
}

/* 	
	this interrupt fires at 1KHz and does the pwm routine
*/
interrupt(TIMERA0_VECTOR) TimerA0_interrupt(void)
{   
	uint8_t temp;
	/* increment the time counter */   	
	regTimer++;
	/* update the keys */
	keyRegOld = keyReg;
	keyReg = scanKeys();
	keyPulse = (keyReg ^ keyRegOld) & keyReg;

	
	/* state machine for setting the LED tempo */
	switch(tapState)
	{
		case 0:
		if(keyPulse & BIT5)
		{
			tapState = 1;
			oldTime = regTimer;
		}
		break;
		
		case 1:
		if(keyPulse & BIT5)
		{
			tapState = 0;
			delayTime = (regTimer - oldTime);
			heartbeatState = 0;
		}
		break;

		default:
		tapState = 0;
		break;
	}
	
	/* light sequence program state machine */
	switch ( programState ) 
	{
		case 0:	/* program not running */
			if(keyPulse & BIT2) /* if the startstop button is pressed, run prog */
			{
				programState = 1;
			}
			break;

		case 1:	/* program running, cycle start */
			if ( keyPulse & BIT2 ) /* turn off if we push the startstop button again */
			{
				programState = 0;
			}
			else
			{
				programCycleStartTime = regTimer;
				programState = 2;
			}
			break;

		case 2: /* program running, cycle step	 */
			if(keyPulse & BIT2) /* turn off if we push the startstop button again */
			{
				programState = 0;
			}
			else if(tapFlag)
			{
				/* time to go to the next step */
				/* write the program to the port */
				++programStep;
				if(programStep > programLength - 1)
				{
					programStep = 0;
				}
				programState = 1;
				tapFlag =0;
			}


			break;

		default:
			programState = 0;			
			break;
	}			
	

	/* Control the heartbeat */
	switch(heartbeatState)
	{
		case 0:
		_setPort(2, LED_HEARTBEAT);
		heartbeatState = 1;
		heartbeatStartTime = regTimer;
		tapFlag = 1;
		break;

		case 1:
		if(regTimer == heartbeatStartTime + 50)
		{
			_clearPort(2, LED_HEARTBEAT);
			heartbeatState = 2;
		}
		break;

		case 2:
		if(regTimer == heartbeatStartTime + delayTime)
		{
			heartbeatState = 0;
		}
		break;
		
		default:
			heartbeatState=0;

	}



	/* Control the output channels, if a button is pressed,
	 * the channel must be bumped. 
    *  |)}># */
	temp = (P1OUT & 0xF0);
	temp |= programArray[programStep];

	if(keyReg & BIT3)
	{
		temp |= EN_CH1;
	}
	if(keyReg & BIT0)
	{
		temp |= EN_CH2;
	}
	if(keyReg & BIT4)
	{
		temp|= EN_CH3;
	}
	if(keyReg & BIT1)
	{
		temp |= EN_CH4;
	}
	P1OUT = temp;

} 



/* initalizes the io ports */
void initIO(void)
{
	
	/* configure IO ports */
			
	P1SEL = 0x00;		/* disable any special functions */
	P2SEL = 0x00;
	
	P1DIR = 0x8F;		/* set up the port directions */
	P1REN = 0x70;		/* enable internal resistors */
	P1OUT = 0x70;		/* set resistors to pullup */
	
	P2DIR = BIT6 ;		/* set up port directions */
	P2REN = BIT7;		/* enable internal resistors */
	P2OUT = BIT7;		/* set resistors to pullup */
	
	return;
}

/*
 *  Scans the 6 input keys and updates a 8 bit value containing the button statuses
 *  BIT0	-	BTN2	-	CH2
 *  BIT1	-	BTN4	-	CH4
 *  BIT2	-	BTN6	-	STARTSTOP
 *  BIT3	-	BTN1	-	CH1
 *  BIT4	-	BTN3	-	CH3
 *  BIT5	-	BTN5	-	TAP
 *  BIT6	-	Reserved
 *  BIT7	-	Reserved
*/
uint8_t scanKeys(void)
{
	uint8_t keys=0;
	_clearPort(1, BTNC1);
	brief_pause(50);
	keys = (P1IN & 0x70);
	keys = keys >> 3;
	_setPort(1, BTNC1);
	brief_pause(50);
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

/* vim: set noet :*/
