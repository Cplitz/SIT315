/**
 * Created by Cameron Pleissnitzer 06/03/2019
 * 
 * This sketch demonstrates the use of different interrupts on the Arduino Uno board.
 * There are only 2 possible external interrupts that can be utilised (INT0 on digital pin 2, and INT1 on digital pin 3)
 * so we must find a way to enable more interrupts using pin change interrupts (PCINT)
 * 
 * I have used exclusively register manipulations to achieve this task with the exception of the analogRead and Serial.println functions.
 * I understand this is generally considered as bad practice for small tasks, as the code is not very maintainable
 * and will also break if used on a different arduino model or processor.
 * However, I have chose to do it this way to increase my knowledge as I have never seen this type of programming before or had to use it.
 * Personally, I think I have done a good job as I completed every other interrupt on my own after watching the timer interrupt tutorial.
 * 
 * In this sketch: 
 *  - A delay of 1000ms is used on the loop function to simulate long processing times and prove the interrupts are working
 *  - PCINT1 is triggered by a phototransistor, which toggles the green LED.
 *  - PCINT2 is trigerred by a push button, which toggles the built in LED
 *  - INT0 is triggered by another push button, which toggles the blue LED
 *  - TIMER1 is a 2 second timer that toggles the red LED, using a prescaler value of 1024 and an output compare match value of 31250
 *  
 *  Youtube link of video demonstration: https://www.youtube.com/watch?v=k6uT7tq0X1s
 */

// LED pins
const int red_led_pin = PD3;
const int green_led_pin = PD6;
const int blue_led_pin = PD7;
const int builtin_led = PB5;

// INPUT pins
const int btn_pcint_pin = PD5;
const int btn_int0_pin = PD2;
const int pt_pin = PC0;

// Timer constants
const uint16_t t1_load = 0;
const uint16_t t1_comp = 31250;

void setup() {
  Serial.begin(9600); 
  
  /* INT0 SETUP */
  // Initialse button as input pullup and blue LED as output
  DDRD &= ~(1 << btn_int0_pin);
  PORTD |= (1 << btn_int0_pin);
  DDRD |= (1 << blue_led_pin);
  PORTD &= ~(1 << blue_led_pin);

  // Set falling edge sense control for INT0 interrupt
  EICRA |= (1 << ISC01);
  EICRA &= ~(1 << ISC00);

  // Enable mask register bit for INT0 and clear flag register
  EIMSK |= (1 << INT0);
  EIFR &= ~(1 << INTF0);

  /* Pin change interrupt (PCINT1) for phototransistor SETUP */
  // Set phototransistor analog pin as input pullup and green LED as output */
  DDRC &= ~(1 << pt_pin);
  PORTC |= (1 << pt_pin);
  DDRD |= (1 << green_led_pin);
  PORTD |= (1 << green_led_pin);
  
  // Enable mask register bit for PCINT8 (A0 pin) and clear flag register
  PCIFR &= ~(1 << PCIF1);
  PCICR |= (1 << PCIE1);
  PCMSK1 |= (1 << PCINT8);
  
  /* Pin change interrupt (PCINT2) for button SETUP */
  // Set buton pin as input pullup and builtin LED as output
  DDRD &= ~(1 << btn_pcint_pin);
  PORTD |= (1 << btn_pcint_pin);
  DDRB |= (1 << builtin_led);
  PORTB &= ~(1 << builtin_led);
  
  // Enable mask register bit for PCINT21 (digital pin 5) and clear flag register
  PCIFR &= ~(1 << PCIF2);
  PCICR |= (1 << PCIE2);
  PCMSK2 |= (1 << PCINT21);

  /* Timer Interrupt (Output Compare Match) SETUP */
  // Set LED pin to be output
  DDRD |= (1 << red_led_pin);

  // Reset Timer1 Control Reg A
  TCCR1A = 0;

  // Set CTC mode
  TCCR1B &= ~(1 << WGM13);
  TCCR1B |= (1 << WGM12);

  // Set to prescaler of 1024
  TCCR1B |= (1 << CS12);
  TCCR1B &= ~(1 << CS11);
  TCCR1B |= (1 << CS10);

  // Reset Timer1 and set compare value
  TCNT1 = t1_load;
  OCR1A = t1_comp;

  // Enable Timer1 compare interrupt
  TIMSK1 = (1 << OCIE1A);

  //Enable global interrupts
  sei();

  //digitalWrite(13, LOW);
  
}

void loop() {
  //Serial.println(analogRead(A0));
  delay(1000);
}

// Timer 1 interrupt 
ISR(TIMER1_COMPA_vect) {
  Serial.println("Interrupt fired: TIMER1_COMPA");

  // Toggle red LED
  PORTD ^= (1 << red_led_pin);
}

// Pin change 2 interrupt
ISR(PCINT2_vect) {
  Serial.println("Interrupt fired: PCINT2 (Button)");

  // Toggle built in LED
  PORTB ^= (1 << builtin_led);
}

// Pin change 1 interrupt
ISR(PCINT1_vect) {
  Serial.println("Interrupt fired: PCINT1 (Phototransistor)");

  // Toggle green LED
  PORTD ^= (1 << green_led_pin);
}

// External interrupt 0
ISR(INT0_vect) {
  Serial.println("Interrupt fired: INT0 (Button)");

  // Toggle blue LED
  PORTD ^= (1 << blue_led_pin);
}
