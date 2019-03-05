int tiltSensorPin = 6;
bool tiltState = false;

void setup() {
  // initialize digital pin LED_BUILTIN as output
  pinMode(LED_BUILTIN, OUTPUT);

  // initialise tilt sensor pin as input
  pinMode(tiltSensorPin, INPUT);

  // Begin serial communications on 9600 
  Serial.begin(9600);
}

void loop() {
  // Get the tilt state from the tilt sensor
  tiltState = digitalRead(tiltSensorPin);

  // Print tilt state to serial
  Serial.println(tiltState);

  // Set the actuator (built in LED) to the tilt state
  digitalWrite(LED_BUILTIN, tiltState);
}
