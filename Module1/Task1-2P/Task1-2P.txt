// Pins
const uint8_t btn_pin = 2;
const uint8_t led_pin = 5;

// Globals
uint8_t led_state = LOW;

void setup() {
  pinMode(btn_pin, INPUT_PULLUP);
  pinMode(led_pin, OUTPUT); 

  // Connect to Serial port
  Serial.begin(9600);

  // Attach the interrupt function on pin 2 (INT0 for Arduino Uno)
  attachInterrupt(digitalPinToInterrupt(btn_pin), toggleLED, FALLING);
}

void loop() {
  // Simulate processor delay
  Serial.println("START BLOCKING DELAY");
  delay(1000);
  Serial.println("END BLOCKING DELAY");
  Serial.println("-------------------");
}

// Interrupt function to toggle LED state
void toggleLED() {
  led_state = !led_state;
  digitalWrite(led_pin, led_state);
  Serial.println("Button pressed!");
}
