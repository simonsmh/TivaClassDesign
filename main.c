#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_ints.h"
#include "inc/hw_gpio.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/adc.h"
#include "driverlib/uart.h"
#include "utils/uc1701.h"
#include "utils/uartstdio.h"

uint8_t flag = 0;
uint32_t pui32ADC0Value[8] = {0}, sum = 0;

void ADC0Sequence0Handler(void)
{
	uint16_t i;
	ADCIntClear(ADC0_BASE, 0); // Clear the ADC interrupt flag.
	ADCSequenceDataGet(ADC0_BASE, 0, pui32ADC0Value); // Read ADC Value.
	for (i = 0; i < 8; i++)
	{
		sum = sum + (pui32ADC0Value[i] * 3300 / 4096);
	}
	flag = 1;
}

int main(void)
{
	// Set the system clock to run at 200/1=200MHz using the PLL. 
	// When using the ADC, you must either use the PLL or supply a 16 MHz clock source.
	SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);
	IntMasterEnable(); //Enable Interrupt control

	// LED init, Closes PF0 & RGB Light on the back.
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
	HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = GPIO_LOCK_KEY;
	HWREG(GPIO_PORTF_BASE + GPIO_O_CR) |= 0x01;
	HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = 0;
	GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_0);
	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, 1 << 0);
	GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1);
	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0 << 1);
	GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_2);
	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 0 << 2);
	GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_3);
	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 0 << 3);

	// UART0 init, 115200
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
	GPIOPinConfigure(GPIO_PA0_U0RX);
	GPIOPinConfigure(GPIO_PA1_U0TX);
	GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
	UARTConfigSetExpClk(UART0_BASE, SysCtlClockGet(), 115200, (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));
	UARTStdioConfig(0, 115200, SysCtlClockGet());

	// ADC0 init
	SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
	GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_0);
	// Enable sample sequence 0 with a processor signal trigger.  
	// Sequence 0 will do a single sample when the processor sends a singal to start the conversion.
	ADCSequenceConfigure(ADC0_BASE, 0, ADC_TRIGGER_PROCESSOR, 0);
	// Configure step 0--7 on sequence 0.  
	ADCSequenceStepConfigure(ADC0_BASE, 0, 0, ADC_CTL_CH3);
	ADCSequenceStepConfigure(ADC0_BASE, 0, 1, ADC_CTL_CH3);
	ADCSequenceStepConfigure(ADC0_BASE, 0, 2, ADC_CTL_CH3);
	ADCSequenceStepConfigure(ADC0_BASE, 0, 3, ADC_CTL_CH3);
	ADCSequenceStepConfigure(ADC0_BASE, 0, 4, ADC_CTL_CH3);
	ADCSequenceStepConfigure(ADC0_BASE, 0, 5, ADC_CTL_CH3);
	ADCSequenceStepConfigure(ADC0_BASE, 0, 6, ADC_CTL_CH3);
	ADCSequenceStepConfigure(ADC0_BASE, 0, 7, ADC_CTL_CH3 | ADC_CTL_IE | ADC_CTL_END);
	// Enable ADC0 sequence 0 interrupt.
	ADCIntEnable(ADC0_BASE, 0);
	IntEnable(INT_ADC0SS0);
	ADCSequenceEnable(ADC0_BASE, 0);
	ADCIntClear(ADC0_BASE, 0);

	// LED init
	UC1701Init(60000);
	UC1701Clear();
	char CHN[]={
		0x10,0x88,0xC4,0x33,0x80,0x9E,0x90,0x9F,0x90,0x9E,0x20,0xD8,0x17,0xF0,0x10,0x00,
		0x01,0x00,0xFF,0x80,0x40,0x3E,0x02,0x02,0x3E,0x10,0x88,0x67,0x18,0x67,0x80,0x00,
		0x00,0x80,0x60,0x9F,0x08,0x08,0xF8,0x00,0x00,0xFF,0x10,0x20,0x40,0x80,0x00,0x00,
		0x81,0x40,0x20,0x13,0x0C,0x13,0x20,0x20,0x40,0x5F,0x40,0x40,0x40,0x41,0x40,0x00,
		0x04,0x84,0x84,0xFC,0x84,0x84,0x00,0xFE,0x92,0x92,0xFE,0x92,0x92,0xFE,0x00,0x00,
		0x20,0x60,0x20,0x1F,0x10,0x10,0x40,0x44,0x44,0x44,0x7F,0x44,0x44,0x44,0x40,0x00,
		0x80,0x80,0x9E,0x92,0x92,0x92,0x9E,0xE0,0x80,0x9E,0xB2,0xD2,0x92,0x9E,0x80,0x00,
		0x08,0x08,0xF4,0x94,0x92,0x92,0xF1,0x00,0x01,0xF2,0x92,0x94,0x94,0xF8,0x08,0x00,
		0x40,0x42,0xCC,0x00,0x00,0x00,0xFE,0x92,0x92,0xFE,0x92,0x92,0xFE,0x00,0x00,0x00,
		0x00,0x00,0x3F,0x10,0x08,0x22,0x12,0x0A,0x06,0xFF,0x06,0x0A,0x12,0x22,0x20,0x00,
		0x40,0x40,0x42,0xCC,0x00,0x40,0xA0,0x9E,0x82,0x82,0x82,0x9E,0xA0,0x20,0x20,0x00,
		0x00,0x00,0x00,0x3F,0x90,0x88,0x40,0x43,0x2C,0x10,0x28,0x46,0x41,0x80,0x80,0x00,
	}; // "微处理器课设" in chn
	UC1701ChineseDisplay(0, 4, 6,CHN);

	// First Line goes to ADC & LCD
	// Last Three Line goes to UART
	uint8_t line = 1, row = 0, count = 8;
	while (1)
	{
		// Trigger ADC0 conversion
		ADCProcessorTrigger(ADC0_BASE, 0);
		SysCtlDelay(SysCtlClockGet() / 20); // ADC Sample Delay

		// ADC Display
		if (flag == 1)
		{
			uint32_t value = sum / 8;
			uint8_t tmp_value = value, digit = 0;
			while (tmp_value != 0)
			{
				tmp_value /= 10;
				digit++;
			}
			for (int i = digit; i < 4; i++) {
				UC1701Display(0, i, ' ');
			}
			UC1701DisplayN(0, 0, value);
			flag = 0;
			sum = 0;
		}
		
		// UART Display
		count--;
		/// Pointer Blink
		if (count > 4)
		{
			UC1701Display(line, row, '_'); // Pointer on
		} else {
			UC1701Display(line, row, ' '); // Pointer off
			if (count == 0){
				count = 8;
			}
		}
		
		/// Loop For All Chars Avaliable
		while (UARTCharsAvail(UART0_BASE))
		{
			uint8_t input = UARTCharGet(UART0_BASE);
			//UC1701DisplayN(3,0,input);
			
			// Display Input ASCII Char
			if ((input <= 126) && (input >= 32))
			{
				UC1701Display(line, row, input);
				row += 1;
			}

			// Next Line
			if ((row > 15) || (input == '\r'))
			{
				UC1701Display(line, row, ' ');
				line += 1;
				row = 0;
			}

			// Backspace
			if (input == 127)
			{
				UC1701Display(line, row, ' ');
				row -= 1;
			}

			// Last Line
			if (row < 0)
			{
				line -= 1;
				row = 15;
			}

			// Clean Screen
			if ((line > 3) || (line < 1))
			{
				line = 1;
				row = 0;
				// Clean Up
				for (int i = 1; i < 4; i++) {
					for (int j = 0; j < 16; j++) {
						UC1701Display(i, j, ' ');
					}
				}
			}

			input = 0;
		}
	}
}
