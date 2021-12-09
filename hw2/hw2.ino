const int carRedLedPin = 13, 
          carYellowLedPin = 12,
          carGreenLedPin = 11,
          peopleRedLedPin = 10,
          peopleGreenLedPin = 9;
          
const int buttonPin = 4,
          buzzerPin = 3;

unsigned int currentState = 0,
             buttonState = HIGH,
             lastButtonReading = HIGH;
             
unsigned long timeSnapshot = 0, // used in each state for timing
              buzzerTimeSnapshot = 0; // used for intermitent sound
    
// state0. default state. waiting for button press
// green for cars, red for people, no sound
void switchToGreenForCars() {
  unsigned long currentTimestamp = millis();
  
  // reset values to a default state proper for further use
  currentState = 0;
  timeSnapshot = currentTimestamp;
  buzzerTimeSnapshot = currentTimestamp;
  lastButtonReading = HIGH;
  buttonState = HIGH;
  
  digitalWrite(carGreenLedPin, HIGH);
  digitalWrite(carYellowLedPin, LOW);
  digitalWrite(carRedLedPin, LOW);
  digitalWrite(peopleGreenLedPin, LOW);
  digitalWrite(peopleRedLedPin, HIGH);
}

void greenForCarsLogic() {
  static const int debounceDelay = 50;    
  static unsigned long lastDebounceTime = 0;
  
  int buttonReading = digitalRead(buttonPin);
  if (buttonReading != lastButtonReading) {
    lastDebounceTime = millis();
  }
  
  if (millis() - lastDebounceTime >= debounceDelay) {
    if (buttonReading != buttonState) {
      buttonState = buttonReading;
      if (buttonState == LOW) {
        // change state on button press
        switchToWaitingState();
        return;
      }
    }
  }

  lastButtonReading = buttonReading;
}

// state1. intermediary state between 0 and 2
// nothing happens. just wait <state1Duration>
void switchToWaitingState() {
  currentState = 1;
  timeSnapshot = millis();
}

void waitingStateLogic() {
  static const int state1Duration = 10 * 1000; // milliseconds
  if (millis() - timeSnapshot >= state1Duration) {
    switchToYellowForCars();
  }
}

// state2. yellow for cars, red for people, no sound
// wait <state 2 duration> seconds to switch to state 3
void switchToYellowForCars() {
  currentState = 2;
  timeSnapshot = millis();
  
  digitalWrite(carGreenLedPin, LOW);
  digitalWrite(carYellowLedPin, HIGH);
}

void yellowForCarsLogic() {
  static const int state2Duration = 3 * 1000; // milliseconds
  if (millis() - timeSnapshot >= state2Duration) {
    switchToGreenForPeople();
  }
}

// state3. red for cars, green for people, constant beeping
// wait <state 3 duration> seconds to switch to state 4
void switchToGreenForPeople() {
  currentState = 3;
  timeSnapshot = millis();

  digitalWrite(carYellowLedPin, LOW);
  digitalWrite(carRedLedPin, HIGH);
  digitalWrite(peopleRedLedPin, LOW);
  digitalWrite(peopleGreenLedPin, HIGH);
}

void greenForPeopleLogic() {
  static const int state3Duration = 10 * 1000, // milliseconds
                   buzzInterval = 1000, // milliseconds
                   buzzerTone = 440; // A4
  static bool buzzerState = 0;

  unsigned long currentTimestamp = millis();
  if (currentTimestamp - timeSnapshot >= state3Duration) {
    buzzerState = 0;
    switchToBlinkingGreen();
    return;
  }

  if (currentTimestamp - buzzerTimeSnapshot >= buzzInterval) {
    buzzerState = !buzzerState;
    if (buzzerState == 1) {
      tone(buzzerPin, buzzerTone);
    }
    else {
      noTone(buzzerPin);
    }
    buzzerTimeSnapshot = currentTimestamp;
  }
}

// state4. red for cars, blinking green for people, faster beeping
// wait <state 4 duration> seconds to switch back to default state 0
void switchToBlinkingGreen() {
  currentState = 4;
  timeSnapshot = millis();
}

void blinkingGreenLogic() {
  // shave and a haircut two bits (total 4.5 seconds)
                                   //    1/4       1/2       1/4,      1/8,     1/8,     1/4,      1/4,      1/4,    1/4       1/4
  static const int buzzIntervals[] = {1, 100, 400, 200, 800, 100, 400, 50, 200, 50, 200, 100, 400, 100, 400, 0, 500, 100, 400, 100, 400}, // durations
                                   //    G2        G2        C5        G5       G5       A5        G5        pause   B5        C6
                   buzzerTones[]   = {0, 98, 0,    98, 0,    523, 0,   784, 0,  784, 0,  880, 0,   784, 0,   0, 0,   988, 0,   1047, 0}, // notes
                   totalSteps = 20;
  
  static int buzzerStep = 0; 
  static bool buzzerState = 0;

  if (buzzerStep == totalSteps) {
    buzzerStep = 0;
    buzzerState = 0;
    noTone(buzzerPin);
    switchToGreenForCars();
    return;
  }

  unsigned long currentTimestamp = millis();
  if (currentTimestamp - timeSnapshot >= buzzIntervals[buzzerStep]) {
    timeSnapshot = currentTimestamp;
    buzzerStep += 1;
    if (buzzerTones[buzzerStep] == 0) {
      noTone(buzzerPin);
      digitalWrite(peopleGreenLedPin, HIGH);
    }
    else {
      tone(buzzerPin, buzzerTones[buzzerStep]);
      digitalWrite(peopleGreenLedPin, LOW);
    }
  }
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
  switchToGreenForCars();
}

void loop() {
  switch(currentState) {
    case 0:
      greenForCarsLogic();
      break;
    case 1:
      waitingStateLogic();
      break;
    case 2:
      yellowForCarsLogic();
      break;
    case 3:
      greenForPeopleLogic();
      break;
    case 4:
      blinkingGreenLogic();
      break;
  }
}
