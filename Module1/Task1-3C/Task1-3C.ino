// Set pins
const uint8_t tiltSensorPin = PD2;
const uint8_t photoTransistorPin = PC0;
const uint8_t ledPin = PD3;

void setup() {
  // Begin serial communication on 9600 
  Serial.begin(9600);

  /* PIN input/output SETUP */
  // led: OUTPUT LOW
  DDRD |= (1 << ledPin);
  PORTD &= ~(1 << ledPin);

  // Tilt sensor pin: INPUT PULLUP
  DDRD &= ~(1 << tiltSensorPin);
  PORTD |= (1 << tiltSensorPin);

  // Phototransistor pin: INPUT PULLUP
  DDRC &= ~(1 << photoTransistorPin);
  PORTC |= (1 << photoTransistorPin);

  
  /* INT0 SETUP */
  // Set falling edge sense control for INT0 interrupt
  EICRA |= (1 << ISC01);
  EICRA &= ~(1 << ISC00);

  // Enable mask register bit for INT0 and clear flag register
  EIMSK |= (1 << INT0);
  EIFR &= ~(1 << INTF0);
  
  /* Pin change interrupt (PCINT1) for phototransistor A0 pin SETUP */
  // Enable mask register bit for PCINT8 (A0 pin) and clear flag register
  PCIFR &= ~(1 << PCIF1);
  PCICR |= (1 << PCIE1);
  PCMSK1 |= (1 << PCINT8);
}

void loop() {
  // Simulate processing delay
  delay(1000);
}

// Toggles the current led state
void toggleLED() {
  PORTD ^= (1 << ledPin);
}

// PCINT1 interrupt to toggle the LED state
ISR(PCINT1_vect) {
  toggleLED();

  Serial.println("Toggled LED via PCINT1 - Phototransistor change");
}

// External INT0 interrupt to toggle the led state
ISR(INT0_vect) {
  toggleLED();

  Serial.println("Toggled LED via INT0 - Tilt sensor change");
}
