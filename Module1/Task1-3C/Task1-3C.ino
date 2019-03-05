const uint8_t tiltSensorPin = 2;
const uint8_t photoTransistorPin = A0;
bool tiltState = LOW;
int photoTransistorAnalog = 0;

void setup() {
  // initialize digital pin LED_BUILTIN as output
  pinMode(LED_BUILTIN, OUTPUT);

  // initialise tilt sensor pin as input
  pinMode(tiltSensorPin, INPUT);

  // initialise photo transistor pin as input
  pinMode(photoTransistorPin, INPUT);
  
  // Begin serial communication on 9600 
  Serial.begin(9600);
}

void loop() {
  // Retrieve sensor data
  tiltState = digitalRead(tiltSensorPin);
  photoTransistorAnalog = analogRead(photoTransistorPin);
  
  // Print data to serial port
  Serial.print("Tilt sensor state: ");
  Serial.println(tiltState);
  Serial.print("Phototransistor current: ");
  Serial.println(photoTransistorAnalog);

  // Set the actuator based on sensor data
  if (tiltState == HIGH && photoTransistorAnalog > 500) {
    Serial.println("LED ON!");
    digitalWrite(LED_BUILTIN, HIGH);
  }
  else {
    Serial.println("LED OFF");
    digitalWrite(LED_BUILTIN, LOW);
  }
  

  //delay(1000);
}
