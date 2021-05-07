// dac.c
// This software configures DAC output
// Lab 6 requires a minimum of 4 bits for the DAC, but you could have 5 or 6 bits
// Runs on TM4C123
// Program written by: Ayan Basu & Adrian Jeyakumar
// Date Created: 3/6/17 
// Last Modified: 1/14/21 
// Lab number: 6
// Hardware connections
// TO STUDENTS "REMOVE THIS LINE AND SPECIFY YOUR HARDWARE********

#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"
// Code files contain the actual implemenation for public functions
// this file also contains an private functions and private data

// **************DAC_Init*********************
// Initialize 4-bit DAC, called once 
// Input: none
// Output: none
void DAC_Init(void){
	// write this
	// Initializes Port B as an output
	SYSCTL_RCGCGPIO_R |= 0x02; // GPIO Interrupt Enable
	
	__asm__ {
		NOP
		NOP
	}
	
	GPIO_PORTB_DIR_R |= 0x0F;		// Sets direction of PB3, PB2, PB1, PB0; these are the DAC outputs
	GPIO_PORTB_DEN_R |= 0x0F;		// Digitally enables PB3, PB2, PB1, PB0; these are the DAC outputs
}

// **************DAC_Out*********************
// output to DAC
// Input: 4-bit data, 0 to 15 
// Input=n is converted to n*3.3V/15
// Output: none
void DAC_Out(uint32_t data){
	// write this
	// Write one 4-bit value to Port B
	GPIO_PORTB_DATA_R = data;		// Data is written into Port B and resistors gets output;
															// Write data into PortB data registers
}
