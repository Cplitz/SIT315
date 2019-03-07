#include "arduino.hpp"
#include "board.h"
// Pins
const int btn_pin = PD2;
const int led_pin = PD5;

void setup() {
  // Connect to Serial port
  Serial.begin(9600);

  /* INT0 SETUP */
  // Initialse button as input pullup and blue LED as output
  DDRD &= ~(1 << btn_pin);
  PORTD |= (1 << btn_pin);
  DDRD |= (1 << led_pin);
  PORTD &= ~(1 << led_pin);

  // Set falling edge sense control for INT0 interrupt
  EICRA |= (1 << ISC01);
  EICRA &= ~(1 << ISC00);

  // Enable mask register bit for INT0 and clear flag register
  EIMSK |= (1 << INT0);
  EIFR &= ~(1 << INTF0);

  //attachInterrupt(digitalPinToInterrupt(btn_pin), toggleLED, FALLING);
}

void loop() {
  // Simulate processor delay
  Serial.println("START BLOCKING DELAY");
  delay(5000);
  Serial.println("END BLOCKING DELAY");
}

// Interrupt function to toggle LED state
ISR(INT0_vect) {
  PORTD ^= (1 << led_pin);
  Serial.println("Button pressed!");
}

int main(void)
{
    /* run the Arduino setup */
    setup();
    /* and the event loop */
    while (1) {
        loop();
    }
    /* never reached */
    return 0;
}
