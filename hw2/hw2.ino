const int carRedLedPin = 13, 
          carYellowLedPin = 12,
          carGreenLedPin = 11,
          peopleRedLedPin = 10,
          peopleGreenLedPin = 9;
          
const int buttonPin = 4,
          buzzerPin = 3;

int currentState = 0,
    buttonState = HIGH,
    lastButtonReading = HIGH,
    timeSnapshot = 0; // used in each state for timing

// state0. default state. waiting for button press
// green for cars, red for people, no sound
void switchToState0() {
  currentState = 0;
  digitalWrite(carGreenLedPin, HIGH);
  digitalWrite(carYellowLedPin, LOW);
  digitalWrite(carRedLedPin, LOW);
  digitalWrite(peopleGreenLedPin, LOW);
  digitalWrite(peopleRedLedPin, HIGH);
}

void state0Logic() {
  static const int debounceDelay = 50;    
  static unsigned int lastDebounceTime = 0;
  
  int buttonReading = digitalRead(buttonPin);
  if (buttonReading != lastButtonReading) {
    lastDebounceTime = millis();
  }
  
  if (millis() - lastDebounceTime >= debounceDelay) {
    if (buttonReading != buttonState) {
      buttonState = buttonReading;
      if (buttonState == LOW) {
        // am apasat pe buton, deci schimb starea
        switchToState1();
      }
    }
  }

  lastButtonReading = buttonReading;
}

// state1. intermediary state between 0 and 2
// nothing happens. just wait <state1Duration>
void switchToState1() {
  currentState = 1;
  timeSnapshot = millis();
  Serial.println("State 1");
}

void state1Logic() {
  static const int state1Duration = 10 * 1000; // milliseconds
  if (millis() - timeSnapshot >= state1Duration) {
    switchToState2();
  }
}

// state2. yellow for cars, red for people, no sound
// wait <state 2 duration> seconds to switch to state 3
void switchToState2() {
  currentState = 2;
  timeSnapshot = millis();
  
  digitalWrite(carGreenLedPin, LOW);
  digitalWrite(carYellowLedPin, HIGH);
  Serial.println("State 2");
}

void state2Logic() {
  static const int state2Duration = 3 * 1000; // milliseconds
  if (millis() - timeSnapshot >= state2Duration) {
    switchToState3();
  }
}

// state3. red for cars, green for people, constant beeping
// wait <state 3 duration> seconds to switch to state 4
void switchToState3() {
  currentState = 3;
  timeSnapshot = millis();

  digitalWrite(carYellowLedPin, LOW);
  digitalWrite(carRedLedPin, HIGH);
  digitalWrite(peopleRedLedPin, LOW);
  digitalWrite(peopleGreenLedPin, HIGH);
  Serial.println("State 3");
}

void state3Logic() {

}

void setup() {
  pinMode(carGreenLedPin, OUTPUT);
  pinMode(carYellowLedPin, OUTPUT);
  pinMode(carRedLedPin, OUTPUT);
  pinMode(peopleGreenLedPin, OUTPUT);
  pinMode(peopleRedLedPin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);

  // using the internal 20.000 ohm resistor
  // thus a pullup-resistor setup for the button
  pinMode(buttonPin, INPUT_PULLUP);

  // default in state 0
  switchToState0();
  Serial.begin(9600);
}

void loop() {
  switch(currentState) {
    case 0:
      state0Logic();
      break;
    case 1:
      state1Logic();
      break;
    case 2:
      state2Logic();
      break;
    case 3:
      state3Logic();
      break;
  }
}
