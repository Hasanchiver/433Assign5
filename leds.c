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

#define LED_MASK ((1<<LED0_PIN) | (1<<LED1_PIN) | (1<<LED2_PIN) | (1<<LED3_PIN))

//#define DELAY_TIME 0x4000000		// Delay with MMU enabled
#define DELAY_TIME 0x40000		// Delay witouth MMU and cache

/*****************************************************************************
 **                INTERNAL FUNCTION PROTOTYPES
 *****************************************************************************/
void initializeLeds(void);
void changeSpeed(uint8_t newSpeed);
void changeMode(uint8_t newMode);
void toggleMode(void);
void flashLights(void);
void driveLedsWithSWFunction(void);
void driveLedsWithSetAndClear(void);
void driveLedsWithBitTwiddling(void);
static void busyWait(unsigned int count);

/*****************************************************************************
 **                INTERNAL FUNCTION DEFINITIONS
 *****************************************************************************/

static uint8_t speed = DEFAULT_SPEED;
static uint8_t mode = BAR;

void changeSpeed(uint8_t newSpeed)
{
	speed = newSpeed;
}
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

	/* Selecting GPIO1[23] pin for use. */
	//GPIO1Pin23PinMuxSetup();

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

void flashLights()
{
	if (mode == BAR)
	{
		for (int pin = LED0_PIN; pin <= LED3_PIN; pin++) {
			HWREG(LED_GPIO_BASE + GPIO_SETDATAOUT) = 1<<pin;
			busyWait(DELAY_TIME);
		}
		for (int pin = LED3_PIN; pin >= LED0_PIN; pin--) {
			HWREG(LED_GPIO_BASE + GPIO_CLEARDATAOUT) = 1<<pin;
			busyWait(DELAY_TIME);
		}
	}
	else
	{
		for (int pin = LED0_PIN; pin <= LED3_PIN; pin++) {
			/* Driving a logic HIGH on the GPIO pin. */
			GPIOPinWrite(LED_GPIO_BASE,
					pin,
					GPIO_PIN_HIGH);

			busyWait(DELAY_TIME);

			/* Driving a logic LOW on the GPIO pin. */
			GPIOPinWrite(LED_GPIO_BASE,
					pin,
					GPIO_PIN_LOW);

			busyWait(DELAY_TIME);
		}
		for (int pin = LED3_PIN; pin >= LED0_PIN; pin--) {
			/* Driving a logic HIGH on the GPIO pin. */
			GPIOPinWrite(LED_GPIO_BASE,
					pin,
					GPIO_PIN_HIGH);

			busyWait(DELAY_TIME);

			/* Driving a logic LOW on the GPIO pin. */
			GPIOPinWrite(LED_GPIO_BASE,
					pin,
					GPIO_PIN_LOW);

			busyWait(DELAY_TIME);
		}
	}
}
/*

 ** The main function. Application starts here.
 */

void driveLedsWithSWFunction(void)
{

		// Flash each LED individually
		for (int pin = LED0_PIN; pin <= LED3_PIN; pin++) {
			/* Driving a logic HIGH on the GPIO pin. */
			GPIOPinWrite(LED_GPIO_BASE,
					pin,
					GPIO_PIN_HIGH);

			busyWait(DELAY_TIME);

			/* Driving a logic LOW on the GPIO pin. */
			GPIOPinWrite(LED_GPIO_BASE,
					pin,
					GPIO_PIN_LOW);

			busyWait(DELAY_TIME);
		}

		// Hit the watchdog (must #include "watchdog.h"
		// Each time you hit the WD, must pass it a different number
		// than the last time you hit it.

}

void driveLedsWithSetAndClear(void)
{
//	int wdCounter = 0;

		// Set all the LEDs:
		HWREG(LED_GPIO_BASE + GPIO_SETDATAOUT) = LED_MASK;
		busyWait(DELAY_TIME);

		// Clear all the LEDs:
		HWREG(LED_GPIO_BASE + GPIO_CLEARDATAOUT) = LED_MASK;
		busyWait(DELAY_TIME);

		// Hit the watchdog (must #include "watchdog.h"
		// Each time you hit the WD, must pass it a different number
		// than the last time you hit it.
//		wdCounter++;
//		WatchdogTimerTriggerSet(SOC_WDT_1_REGS, wdCounter);

}

void driveLedsWithBitTwiddling(void)
{
	int wdCounter = 0;
	while (1) {
		// Set all the LEDs:
		HWREG(LED_GPIO_BASE + GPIO_DATAOUT) |= LED_MASK;
		busyWait(DELAY_TIME);

		// Clear all the LEDs:
		HWREG(LED_GPIO_BASE + GPIO_DATAOUT) &= ~LED_MASK;
		busyWait(DELAY_TIME);

		// Hit the watchdog (must #include "watchdog.h"
		// Each time you hit the WD, must pass it a different number
		// than the last time you hit it.
		wdCounter++;
		WatchdogTimerTriggerSet(SOC_WDT_1_REGS, wdCounter);
	}
}

/*
 ** Busy-wait function
 */
static void busyWait(volatile unsigned int count)
{	
	uint8_t i;
	uint32_t speedDividerFactor = 1;
	for(i = 0; i < MAX_SPEED - speed; i++) {
		speedDividerFactor = speedDividerFactor * 2;
	}
	uint32_t newCount = count/speedDividerFactor;
	while((newCount--));
}


