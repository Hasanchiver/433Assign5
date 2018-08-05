// GPIO LED demo
#include "soc_AM335x.h"
#include "beaglebone.h"
#include "gpio_v2.h"
#include "hw_types.h"      // For HWREG(...) macro
#include "watchdog.h"
#include <stdint.h>




/*****************************************************************************
 **                INTERNAL MACRO DEFINITIONS
 *****************************************************************************/
#define LED_GPIO_BASE           (SOC_GPIO_1_REGS)
#define LED0_PIN (21)
#define LED1_PIN (22)
#define LED2_PIN (23)
#define LED3_PIN (24)

#define MAX_SPEED 9
#define DEFAULT_SPEED 6
#define MIN_SPEED 0
#define BOUNCE 1
#define BAR 2
#define LED_STATES 7
#define LED_MASK ((1<<LED0_PIN) | (1<<LED1_PIN) | (1<<LED2_PIN) | (1<<LED3_PIN))

//#define DELAY_TIME 0x4000000		// Delay with MMU enabled
#define DELAY_TIME 0x40000		// Delay witouth MMU and cache

/*****************************************************************************
 **                INTERNAL FUNCTION PROTOTYPES
 *****************************************************************************/
void initializeLeds(void);

void changeMode(uint8_t newMode);
void toggleMode(void);
void flashLights(void);
static void busyWait(unsigned int count);

/*****************************************************************************
 **                EXTERNAL FUNCTION DEFINITIONS
 *****************************************************************************/


static uint8_t mode = BAR;
static int counter = 0;



void changeMode(uint8_t newMode)
{
	mode = newMode;
}


void toggleMode()
{
	if (mode == BOUNCE)
		mode = BAR;
	else
		mode = BOUNCE;
}


void initializeLeds(void)
{
	/* Enabling functional clocks for GPIO1 instance. */
	GPIO1ModuleClkConfig();


	/* Enabling the GPIO module. */
	GPIOModuleEnable(LED_GPIO_BASE);

	/* Resetting the GPIO module. */
	GPIOModuleReset(LED_GPIO_BASE);

	/* Setting the GPIO pin as an output pin. */
	GPIODirModeSet(LED_GPIO_BASE,
			LED0_PIN,
			GPIO_DIR_OUTPUT);
	GPIODirModeSet(LED_GPIO_BASE,
			LED1_PIN,
			GPIO_DIR_OUTPUT);
	GPIODirModeSet(LED_GPIO_BASE,
			LED2_PIN,
			GPIO_DIR_OUTPUT);
	GPIODirModeSet(LED_GPIO_BASE,
			LED3_PIN,
			GPIO_DIR_OUTPUT);
}


static void turnOnLED(int pin)
{
	HWREG(LED_GPIO_BASE + GPIO_SETDATAOUT) = 1<<pin;

}


static void turnOffLED(int pin)
{
	HWREG(LED_GPIO_BASE + GPIO_CLEARDATAOUT) = 1<<pin;
}


void flashLights()
{

	if (mode == BAR)
	{
		switch (counter % LED_STATES){
			case 0:
				turnOffLED(LED1_PIN);
				turnOnLED(LED0_PIN);
				break;
			case 1:
				turnOffLED(LED0_PIN);
				turnOnLED(LED1_PIN);
				break;
			case 2:
				turnOffLED(LED1_PIN);
				turnOnLED(LED2_PIN);
				break;
			case 3:
				turnOffLED(LED2_PIN);
				turnOnLED(LED3_PIN);
				break;
			case 4:
				turnOffLED(LED3_PIN);
				turnOnLED(LED2_PIN);
				break;
			case 5:
				turnOffLED(LED2_PIN);
				turnOnLED(LED1_PIN);
				break;
		}
	}
	else
	{
		switch (counter % LED_STATES){
			case 0:
				turnOffLED(LED1_PIN);
				turnOnLED(LED0_PIN);
				break;
			case 1:
				turnOnLED(LED1_PIN);
				break;
			case 2:
				turnOnLED(LED2_PIN);
				break;
			case 3:
				turnOnLED(LED3_PIN);
				break;
			case 4:
				turnOffLED(LED3_PIN);
				break;
			case 5:
				turnOffLED(LED2_PIN);
				break;
		}
	}
	counter++;
}


/*
 ** Busy-wait function
 */
static void busyWait(volatile unsigned int count)
{
	while(count--)
		;
}
