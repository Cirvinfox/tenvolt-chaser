/*
 * =====================================================================================
 *
 *       Filename:  chaser.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  28/11/14 05:29:54
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Olivia Theze (foxiepaws), fox@foxiepa.ws
 *   Organization:  
 *
 * =====================================================================================
 */

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

