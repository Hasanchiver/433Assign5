// leds.h:
// Hardware  module: led controller

#ifndef LEDS_H_
#define LEDS_H_

void initializeLeds(void);
void changeMode(uint8_t newMode);
void toggleMode(void);
void flashLights(void);

#endif /* LEDS_H_ */
