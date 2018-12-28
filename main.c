//*****************************************************************************
//
// 
// 
//
//*****************************************************************************
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_ints.h"
#include "inc/hw_gpio.h"
#include "inc/hw_sysctl.h"
#include "inc/hw_adc.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/adc.h"
#include "driverlib/uart.h"
#include "driverlib/uc1701.h"
#include "utils/uartstdio.h"

//*****************************************************************************
//
// The error routine that is called if the driver library encounters an error.
//
//*****************************************************************************
#ifdef DEBUG
void
__error__(char *pcFilename, unsigned long ulLine)
{
}
#endif

//*****************************************************************************
//
// Global variables
// 
// 
//
//*****************************************************************************
unsigned char flag;
int value;

		//
    // This array is used for storing the data read from the ADC FIFO. It
    // must be as large as the FIFO for the sequencer in use.  This example
    // uses sequence 3 which has a FIFO depth of 1.  If another sequence
    // was used with a deeper FIFO, then the array size must be changed.
    //
 uint32_t pui32ADC0Value[8],sum;

//*****************************************************************************
//
// Configure the UART and its pins.  This must be called before UARTprintf().
//
//*****************************************************************************
void
ConfigureUART(void)
{
    //
    // Enable the GPIO Peripheral used by the UART.
    //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    //
    // Enable UART0
    //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);

    //
    // Configure GPIO Pins for UART mode.
    //
    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    UARTConfigSetExpClk(UART0_BASE, SysCtlClockGet(), 115200,(UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));

    //
    // Initialize the UART for console I/O.
    //
    UARTStdioConfig(0, 115200, SysCtlClockGet());
}

void ADC0Sequence0Handler(void)
{

uint16_t i;
	
        //
        // Clear the ADC interrupt flag.
        //
        ADCIntClear(ADC0_BASE, 0);

        //
        // Read ADC Value.
        //
        ADCSequenceDataGet(ADC0_BASE, 0, pui32ADC0Value);

   for(i = 0;i < 8;i ++)
    {
     sum= sum+(pui32ADC0Value[i]*3300/4096);
    }

		flag=1;
		
}

//*****************************************************************************
//
// The main application. 
//
//*****************************************************************************
int
main(void)
{
    //
    // Set the system clock to run at 200/5=40MHz using the PLL.  When
    // using the ADC, you must either use the PLL or supply a 16 MHz clock
    // source.
    SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN |
                   SYSCTL_XTAL_16MHZ);
	
    //
    // LED init
    //
		SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
	  HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = GPIO_LOCK_KEY;
    HWREG(GPIO_PORTF_BASE + GPIO_O_CR) |= 0x01;
    HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = 0;		
		
	  GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_0);
    GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_0,1<<0);

		GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1);
    GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_1,0<<1);
		
		GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_2);
    GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_2,0<<2);
		
		GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_3);
    GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_3,0<<3);

    //
    // enable processer interrupt
    //		
    IntMasterEnable();
			
    //
    // Set up the serial console to use for displaying messages.  This is just
    // for this example program and is not needed for ADC operation.
    //
    ConfigureUART();	

    //
    // The ADC0 peripheral must be enabled for use.
    //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);

    //
    // Display the setup on the console.
    //
    //UARTprintf("ADC ->\n");
    //UARTprintf("  Input Pin: AIN3/PE0\n\n");

    //
    // example ADC0 is used with AIN3 on port E0.
    //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);

    //
    // Select the analog ADC function for these pins.
    //
    GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_0);


    //
    // Enable sample sequence 0 with a processor signal trigger.  Sequence 0
    // will do a single sample when the processor sends a singal to start the
    // conversion.  Each ADC module has 4 programmable sequences, sequence 0
    // to sequence 3.  This example is arbitrarily using sequence 0.
    //
    ADCSequenceConfigure(ADC0_BASE, 0, ADC_TRIGGER_PROCESSOR, 0);

    //
    // Configure step 0--7 on sequence 0.  
    //
		ADCSequenceStepConfigure(ADC0_BASE, 0, 0, ADC_CTL_CH3 );
		ADCSequenceStepConfigure(ADC0_BASE, 0, 1, ADC_CTL_CH3 );
		ADCSequenceStepConfigure(ADC0_BASE, 0, 2, ADC_CTL_CH3 );
		ADCSequenceStepConfigure(ADC0_BASE, 0, 3, ADC_CTL_CH3 );
		ADCSequenceStepConfigure(ADC0_BASE, 0, 4, ADC_CTL_CH3 );
		ADCSequenceStepConfigure(ADC0_BASE, 0, 5, ADC_CTL_CH3 );
		ADCSequenceStepConfigure(ADC0_BASE, 0, 6, ADC_CTL_CH3 );
    ADCSequenceStepConfigure(ADC0_BASE, 0, 7, ADC_CTL_CH3 | ADC_CTL_IE | ADC_CTL_END);

    //
    // enable ADC0 sequence 0 interrupt
    //
    ADCIntEnable(ADC0_BASE, 0);
    IntEnable(INT_ADC0SS0);

    //
    // Since sample sequence 0 is now configured, it must be enabled.
    //
    ADCSequenceEnable(ADC0_BASE, 0);
		
    //
    // Clear the interrupt status flag.  This is done to make sure the
    // interrupt flag is cleared before we sample.
    //
    ADCIntClear(ADC0_BASE, 0);

    //
    // Sample the temperature sensor forever.  Display the value on the
    // console.
    //
		
		UC1701Init(60000);
    UC1701Clear();
		
		//First Line goes to ADC
		//Last Three Line goes to UART
		unsigned int line=1,row=0;
		unsigned char input=0;
		
    while(1)
    {
        //
        // Trigger the ADC conversion.
        //
        ADCProcessorTrigger(ADC0_BASE, 0);

				
        if(flag==1)
				{
					value=sum/8;
					for(int i=1;i<4;i++){
						UC1701Display(0,i,' ');
					}
					UC1701DisplayN(0, 0,value);
					UC1701Display(line,row,'_');
					SysCtlDelay(SysCtlClockGet() / 40);	
				  flag=0;
					sum=0;
					
					while (UARTCharsAvail(UART0_BASE))
					{
						input = UARTCharGet(UART0_BASE);
						if((input <= 126) && (input >= 32 ))
							{
								UC1701Display(line,row,input);
								row+=1;
							}
						}
						if((row >= 16) || (input == '\r')) {
							UC1701Display(line,row,' ');
							line+=1;
							row=0;
						}
						if(line==4){
							line=1;
							row=0;
							//Clean Up
							for(int i=1;i<4;i++){
								for(int j=0;j<16;j++){
									UC1701Display(i,j,' ');
								}
							}
							SysCtlDelay(200);
							UC1701Display(line,row,'_');
						}
						input = 0;
					}

				//
				//sample delay
				//
        SysCtlDelay(1000);
				
    }
}
