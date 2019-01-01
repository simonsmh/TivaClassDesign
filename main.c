#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "inc/hw_gpio.h"
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/adc.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "utils/uartstdio.h"
#include "utils/uc1701.h"

uint8_t flag = 1;
uint32_t pui32ADC0Value[8] = {0}, sum = 0;
// First Line goes to ADC & LCD
// Last Three Line goes to UART
//  ADC | CHN Words
// *_______________ LCD Init Line = 1, Row = 1
// ________________ Matrix = 16 * (line - 1) + row
// ________________ Last Line becomes input box
int8_t line = 1, row = 0;
uint8_t matrix[48] = {
	32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32,
	32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32,
	32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32,
	};

void UART0Handler(void)
{
	UARTIntClear(UART0_BASE, 0); // Clear the UART interrupt flag.
	// UART Display
	/// Loop For All Chars Avaliable
	while (UARTCharsAvail(UART0_BASE))
	{
		uint8_t input = UARTCharGet(UART0_BASE);
		
		// Display Input ASCII Number
		//UC1701DisplayN(0,9,input);

		// Display Input ASCII Char
		if ((input <= 126) && (input >= 32))
		{
			UC1701Display(line, row, input);
			matrix[16 * (line - 1) + row] = input;
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
			line = 3;
			row = 0;
			for (int j = 0; j < 16; j++) {
				// Overwrite matrix
				for (int i = 1; i < 3; i++) {
						//UC1701Display(i, j, ' ');
						UC1701Display(i, j, matrix[16 * i + j]);
						matrix[16 * (i - 1) + j] = matrix[16 * i + j];
					
				}
				// Clean Up last line
				for (int i = 3; i < 4; i++) {
						UC1701Display(i, j, ' ');
						matrix[16 * (i - 1) + j] = 32;
				}
			}
		}
		
		// Current Pointer Location
		//UC1701DisplayN(0,5,row);
		//UC1701DisplayN(0,7,line);

		input = 0;
	}
}

void ADC0Sequence0Handler(void)
{
	uint16_t i;
	ADCIntClear(ADC0_BASE, 0); // Clear the ADC interrupt flag.
	ADCSequenceDataGet(ADC0_BASE, 0, pui32ADC0Value); // Read ADC Value.
	for (i = 0; i < 8; i++)
	{
		sum = sum + (pui32ADC0Value[i] * 3300 / 4096);
	}

	// ADC Display
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
	sum = 0;
}

int main(void)
{
	// Enable FPU
	FPUEnable();
	FPULazyStackingEnable();

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
	// Enable UART0 interrupt.
	UARTIntEnable(UART0_BASE, UART_INT_RT);
	IntEnable(INT_UART0);
	UARTIntClear(UART0_BASE, 0);

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
		0x00,0x20,0x30,0xDC,0xEC,0xFC,0xFC,0xB0,0xFC,0xF0,0xF8,0xFC,0xDC,0xC0,0x40,0x00,
		0x04,0x06,0x7B,0x77,0x3C,0x1A,0x0E,0x3E,0x3F,0x3B,0x1F,0x1E,0x3F,0x31,0x20,0x00,
		0x00,0x00,0x80,0xF0,0x7C,0x3C,0xE0,0xE0,0x00,0xFC,0xFC,0xC0,0xC0,0xC0,0x00,0x00,
		0x00,0x22,0x33,0x19,0x1F,0x0F,0x0F,0x0D,0x18,0x3F,0x3F,0x70,0x61,0x61,0x60,0x00,
		0x00,0x10,0x10,0xF0,0xF8,0x98,0xF8,0xF8,0xA8,0xF8,0xF8,0xAC,0xFC,0x7C,0x18,0x00,
		0x10,0x11,0x19,0x1F,0x2F,0x6C,0x64,0x25,0x2D,0x3F,0x3F,0x35,0x35,0x30,0x30,0x00,
		0x00,0x00,0x08,0xBC,0xAC,0xAC,0xFC,0xFC,0xFC,0xFC,0xEC,0x6E,0x7C,0x40,0x00,0x00,
		0x00,0x00,0x1C,0x7C,0x6E,0x2F,0x3B,0x39,0x79,0x7B,0x2E,0x2E,0x3E,0x0C,0x04,0x00,
		0x80,0x80,0xC4,0xCC,0xDC,0x48,0xF8,0xF8,0xA8,0xF8,0xAC,0xFC,0xFC,0x9C,0x08,0x00,
		0x00,0x00,0x38,0x3F,0x3F,0x18,0x1D,0x0D,0x7F,0xFF,0x7F,0x0D,0x1D,0x19,0x10,0x00,
		0x80,0x80,0x80,0xCC,0xCC,0xCC,0x80,0xFC,0x78,0x9C,0xDC,0xFC,0x74,0x60,0x00,0x00,
		0x00,0x00,0x00,0x3F,0x3F,0x18,0x2C,0x23,0x37,0x1D,0x3F,0x77,0x61,0x60,0x40,0x40,
	}; // "微处理器课设" in chn
	UC1701ChineseDisplay(0, 4, 6,CHN);
	
	int8_t count = 8;
	while (1)
	{
		// Trigger ADC0 conversion
		ADCProcessorTrigger(ADC0_BASE, 0);
		SysCtlDelay(SysCtlClockGet() / 20); // ADC Sample Delay

		/// Pointer Blink
		count--;
		if (count > 4)
		{
			UC1701Display(line, row, '_'); // Pointer on
		} else {
			UC1701Display(line, row, ' '); // Pointer off
			if (count == 0){
				count = 8;
			}
		}
	}
}
