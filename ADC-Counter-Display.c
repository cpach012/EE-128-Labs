#include "fsl_device_registers.h"

unsigned short ADC_read16b(void);

const unsigned int decoder[10] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x67};			//hex for 0-9  

void software_delay(unsigned long delay)
{
	while (delay > 0) delay--;
}

int main (void)
{
// Enable Port A, B, C, D, and ADC0 clock gating.
	SIM_SCGC5 |= SIM_SCGC5_PORTA_MASK;
	SIM_SCGC5 |= SIM_SCGC5_PORTB_MASK;
	SIM_SCGC5 |= SIM_SCGC5_PORTC_MASK;
	SIM_SCGC5 |= SIM_SCGC5_PORTD_MASK;
	SIM_SCGC6 |= SIM_SCGC6_ADC0_MASK;

	// Configure PA1, PB[10, 3:2], PC[8:7, 5:0], PD[7:0] for GPIO.
	PORTB_GPCLR = 0x00C0100;
	PORTC_GPCLR = 0x01BF0100;
	PORTD_GPCLR = 0x00FF0100;

									// Configure PA1 to trigger interrupts on falling edge input.
	PORTA_PCR1 = 0xA0100;			// Configure Port A  pin 1 for GPIO and interrupt on falling edge

	PORTB_PCR10 = 0x100;			//Configure PORTB[10] for GPIO and to generate clock


	GPIOA_PDDR |= (0 << 1); 		// PortA[1] input mode
	GPIOB_PDDR |= (1 << 10);		//PortB[10] as an output
	GPIOC_PDDR = 0x000001BF;		//Configure Port C Pins 0-5, 7-8 for Output;
	GPIOD_PDDR = 0x000000FF;		//Configure Port D Pins 0-7 for Output;

	ADC0_CFG1 = 0x0C;				// Configure ADC for 16 bits, and to use bus clock.
	ADC0_SC1A = 0x1F;				// Disable the ADC module;


	// Clear PORTA ISFR
	PORTA_ISFR = (1<<1);			// Clear ISFR for Port A, pin 1


	NVIC_EnableIRQ(PORTA_IRQn);		// Enable Port A IRQ interrupts;




	unsigned long Delay = 0x50000; 								//Delay Value
	for (;;){
		GPIOB_PTOR |= (1<<10);									//Clock generator
		software_delay(Delay);                					//delay for the effect of soft clock;
	}

	return 0;
}



void PORTA_IRQHandler(void)		// Count & ADC Operations
{
	uint32_t MODE_SW;						// 0 for ADC -> read voltage from ADC and display  			ON		(switch 2)
											// 1 for counter mode -> update a counter and display		OFF

	uint32_t CNT_DIR;						// 0 for upward counting -> count up until 99 and roll over to 0 after 99)		ON	(switch 3)   switch is down
											// 1 for downward counting -> count down until 0 and roll over 99 after 0)		OFF				 switch is up
											//CNT_DIR is effective only when MODE_SW == 1 (counter mode)

	MODE_SW = GPIOB_PDIR & 0x04;
	CNT_DIR = GPIOB_PDIR & 0x08;
	static int counter = 0;
	static int voltage = 0;
	static int value = 0;


	if(MODE_SW == 0){
		value = ADC_read16b();
		voltage = value* ((3.3)/65535);    //(3.3 - 0)/ (2^16) -1)
		GPIOC_PDOR = (decoder[voltage%10] & 0x3F) | ((decoder[voltage % 10] & 0x40) << 1); 
		GPIOD_PDOR = decoder[voltage/10];												   

	}
	else{
		if(CNT_DIR == 0){
			if(counter >= 99){				//count up to 99 and roll over to 0
				counter = 0;
			}
			else{
				counter++;
			}
		}
		else{
			if(counter <= 0){				//count down to 0 and roll over to 99
				counter = 99;
			}
			else{
				counter --;
			}
		}
		GPIOC_PDOR = (decoder[counter%10] & 0x3F) | ((decoder[counter % 10] & 0x40) << 1);  
		GPIOD_PDOR = decoder[counter/10];													
	}

	PORTA_ISFR = (1 << 1);		//Clear ISFR
}

unsigned short ADC_read16b(void)
{
	ADC0_SC1A = 0x00; 								// Write to SC1A to start  conversion from ADC_0
	while(ADC0_SC2 & ADC_SC2_ADACT_MASK);			// Conversion in progress
	while(!(ADC0_SC1A & ADC_SC1_COCO_MASK)); 		// UNtil conversion complete
	return ADC0_RA;
}
