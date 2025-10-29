#include "fsl_device_registers.h"
void software_delay(unsigned long delay)
{
	while (delay > 0) delay--;
}
int main(void){
	SIM_SCGC5 |= SIM_SCGC5_PORTB_MASK;		//Configure Clock Gating for Ports B,C, and D;
	SIM_SCGC5 |= SIM_SCGC5_PORTC_MASK;
	SIM_SCGC5 |= SIM_SCGC5_PORTD_MASK;
	PORTD_GPCLR = 0x00FF0100;				//Configure Port D Pins 0-7 for GPIO;
	GPIOD_PDDR = 0x000000FF;				//Configure Port D Pins 0-7 for Output;
	PORTC_GPCLR = 0x01BF0100;				//Configure Port C Pins 0-5 and 7-8 for GPIO;
	GPIOC_PDDR = 0x000001BF;				//Configure Port C Pins 0-5 and 7-8 for Output;
	PORTB_GPCLR = 0x000C0100;				//Configure Port B Pin 2-3 for GPIO;
	GPIOB_PDDR = 0x00000000;				//Configure Port B Pin 2-3 for Input;

	GPIOC_PDOR = 0x00000000;				//Initialize Port C to 0;
	GPIOD_PDOR = 0x00000001;				//Initialize Port D such that only 1 bit is ON;

	unsigned long Delay = 0x100000; 		//Delay Value
	uint32_t Input;
	unsigned long Shifter = 0x01;
	unsigned long Counter = 0x00000000;
	uint32_t CNT_DIR;			//0 for upward, 1 for downward - switch 2
	uint32_t ROT_DIR;			//0 for left, 1 for right - switch 3

	while (1) {
		software_delay(Delay);				//delay for the effect of soft clock;
		Input = GPIOB_PDIR & 0x0C;					//read Port B and bits 2-3;
		CNT_DIR = Input & 0x4;
		ROT_DIR = Input & 0x8;

		if (CNT_DIR == 0){					//switch ON
			//increment Port C;
			Counter += 1;
			if (Counter >= 255){			//8bit -> can have 256 elements 0-255
				Counter = 0x00;
				GPIOC_PDOR = Counter;		//Output on Port C
			}
			else{
				GPIOC_PDOR = Counter;		//Output on Port C
			}
		}
		else{
			//decrement Port C;
			Counter -= 1;
			if (Counter <= 0){
				Counter = 0xFF;
				GPIOC_PDOR = Counter;	//Output on Port C
			}
			else{
				GPIOC_PDOR = Counter;		//Output on Port C
			}
		}


		if (ROT_DIR == 0){						//switch ON
			//left-rotate Port D;
			if(Shifter == 0x80){				//if all left, come back to all right
				Shifter = 0x01;
				GPIOD_PDOR = Shifter;
			}
			else{
				Shifter = Shifter << 1;
				GPIOD_PDOR = Shifter;			//Shift left by 1 bit, in port D
			}
		}
		else{
			//right-rotate Port D;
			if(Shifter == 0x01){				//if all right, come back to all left
				Shifter = 0x80;
				GPIOD_PDOR = Shifter;
			} 								//shift right by 1 bit, in port D
			else{
				Shifter = Shifter >> 1;
				GPIOD_PDOR = Shifter;
			}
		}

	}

	return 0;
}
