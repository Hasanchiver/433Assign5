#include "soc_AM335x.h"
#include "beaglebone.h"
#include "consoleUtils.h"
#include "hw_types.h"
#include <stdint.h>
#include "gpio_v2.h"
#include "dmtimer.h"
#include "error.h"


#include "leds.h"
#include "button.h"
#include "watchdog.h"
#include "timer.h"
#include "serial.h"
#include "wdtimer.h"

#define EXTERNAL_RESET      5
#define WATCHDOG_RESET      4
#define COLD_RESET   	    0

#define MAX_SPEED 9
#define DEFAULT_SPEED 6
#define MIN_SPEED 0
#define BOUNCE 1
#define BAR 2
#define MIN_NUM_ASCII 48
#define MAX_NUM_ASCII 57



#define PRM_DEV 0x44E00F00
#define PRM_RSTST_OFFSET 0x8

static uint8_t speed = DEFAULT_SPEED;


static _Bool stopWatchDog = false;

static _Bool isButtonPressed = false;
static _Bool lastButtonState = false;
static int counter = 0;


static volatile _Bool s_doThings = false;


static void listCommands(void);
static int getSpeedDivider(void);
static void printResetScources(void);
static void listCommands(void);


/******************************************************************************
 **              SERIAL PORT HANDLING
 ******************************************************************************/
static volatile uint8_t s_rxByte = 0;
static void serialRxIsrCallback(uint8_t rxByte) {
	s_rxByte = rxByte;
}
static void changeSpeed(uint8_t newSpeed)
{
	speed = newSpeed;
}
static void doBackgroundSerialWork(void)
{

	if (s_rxByte != 0) {
		if (s_rxByte == '?') {
			listCommands();
		}
		else if(s_rxByte >= MIN_NUM_ASCII && s_rxByte <= MAX_NUM_ASCII) {
			ConsoleUtilsPrintf("\nSetting LED speed to %c\n", s_rxByte);
			changeSpeed(s_rxByte - MIN_NUM_ASCII);
		}
		else if(s_rxByte == 'a' || s_rxByte == 'A') {
			ConsoleUtilsPrintf("\nChanging to bounce mode\n");
			changeMode(BOUNCE);
		}
		else if(s_rxByte == 'b' || s_rxByte == 'B') {
			ConsoleUtilsPrintf("\nChanging to bar mode\n");
			changeMode(BAR);
		}
		else if(s_rxByte == 'x' || s_rxByte == 'X') {
			ConsoleUtilsPrintf("\nNo longer hitting the watchdog.\n");
			stopWatchDog = true;
		}
		else {
			ConsoleUtilsPrintf("\nInvalid command\n");
		}

		s_rxByte = 0;
	}
}

void notifyOnTimeIsr(void)
{
	s_doThings = true;
}
void doBackgroundWork(void)
{

	if (s_doThings) {

		s_doThings = false;
		isButtonPressed = readButtonWithBitTwiddling();

		if (counter % getSpeedDivider() == 0){
			flashLights();
		}
		if (lastButtonState != isButtonPressed) {
				if (isButtonPressed == false)
				{
					toggleMode();
				}
				lastButtonState = isButtonPressed;
		}
		counter++;
	}
}

/******************************************************************************
 **              MAIN FUNCTION
 ******************************************************************************/

int main()
{


	Serial_init(serialRxIsrCallback);
	Timer_init();
	Watchdog_init();
	initializeLeds();
	initializeButtonPin();

	Serial_setRxIsrCallback(serialRxIsrCallback);
	Timer_setTimerIsrCallback(notifyOnTimeIsr);

	ConsoleUtilsPrintf("\nThis is my Light Bouncer Program\n");
	ConsoleUtilsPrintf("    Written by Duc Lai\n");
	ConsoleUtilsPrintf("------------------------------------------------\n");
	printResetScources();
	listCommands();


	while (1)
	{
		if(Timer_isIsrFlagSet()) {



			doBackgroundSerialWork();
			doBackgroundWork();

			Timer_clearIsrFlag();
			if (!stopWatchDog){
				Watchdog_hit();
			}
		}

	}


}


/******************************************************************************
 **              STRING HANDLING
 ******************************************************************************/

static void printResetScources(void){

	uint32_t resetSourceRegister = HWREG(PRM_DEV + PRM_RSTST_OFFSET);

	ConsoleUtilsPrintf("Reset source (0x%x) = ", resetSourceRegister);

	if((resetSourceRegister & (1 << EXTERNAL_RESET)) != 0) {
		ConsoleUtilsPrintf("External reset, \n");
	}
	else if((resetSourceRegister & (1 << WATCHDOG_RESET)) != 0) {
		ConsoleUtilsPrintf("Watchdog reset, \n");
	}
	else if((resetSourceRegister & (1 << COLD_RESET)) != 0) {
		ConsoleUtilsPrintf("Cold reset, \n");
	}

	HWREG(PRM_DEV + PRM_RSTST_OFFSET) |= resetSourceRegister;
}

static void listCommands(){
	ConsoleUtilsPrintf(" \n");
	ConsoleUtilsPrintf("Commands:\n");
	ConsoleUtilsPrintf(" '?' : Display help message.\n");
	ConsoleUtilsPrintf(" '0-9' : Set speed 0 (slow) to 9 (fast).\n");
	ConsoleUtilsPrintf(" 'a':  Select pattern A (bounce).\n");
	ConsoleUtilsPrintf(" 'b':  Select pattern B (bar).\n");
	ConsoleUtilsPrintf(" 'x':  Stop hitting the watchdog.\n");
	ConsoleUtilsPrintf(" 'BTN':  Push-button to toggle mode.\n");
}

static int getSpeedDivider(){
	int speedDividerFactor = 1;
	for(int i = 0; i < MAX_SPEED - speed; i++) {
		speedDividerFactor = speedDividerFactor * 2;
	}
	return speedDividerFactor;
}
