// leds.h:
// Hardware  module: led controller

#ifndef BUTTON_H_
#define BUTTON_H_

void initializeButtonPin(void);
_Bool readButtonWithStarteWare(void);
_Bool readButtonWithBitTwiddling(void);

void anotherUartInitialize(void);

#endif /* BUTTON_H_ */
