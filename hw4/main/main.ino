#include <EEPROM.h>


// 4 x seven segment display - segment pins
const int seg1Pin = 7,
          seg2Pin = 6,
          seg3Pin = 5,
          seg4Pin = 4;

// joystick pins
const int joySWPin = 2,
          joyYPin = A1,
          joyXPin = A0; 

// Shift register pins
const int dataPin = 12, // DS
latchPin = 11, // STCP 
clockPin = 10; // SHCP

const int segmentCount = 4;
const int displaySegments[] = {
  seg1Pin, seg2Pin, seg3Pin, seg4Pin
};

// corresponing bit states for 0-9 digits
// to use on 7 seven segment display
byte digitArray[10] = {
  B11111100, // 0
  B01100000, // 1
  B11011010, // 2
  B11110010, // 3
  B01100110, // 4
  B10110110, // 5
  B10111110, // 6
  B11100000, // 7
  B11111110, // 8
  B11110110, // 9
};

void writeRegister(int digit) {
  digitalWrite(latchPin, LOW);
  shiftOut(dataPin, clockPin, MSBFIRST, digit);
  digitalWrite(latchPin, HIGH);
}

// which segment to light up
void showSegment(int segmentIndex) {
  for (int i = 0; i < segmentCount; ++ i) {
    digitalWrite(displaySegments[i], HIGH);
  }
  digitalWrite(displaySegments[segmentIndex], LOW);
}

// byte because I want to store this as well in eeprom
byte segIndex = 0,
     segmentValues[segmentCount];

const int joyLowThreshold = 200,
          joyHighThreshold = 800,
          dotBlinkDelay = 250,
          stateChangeDelay = 1000;

int state = 0,
    dotState = HIGH;
    
bool joyMoved = false;

// unsigned long because we're working with time
// and we want nice behaviour in case of overflows
unsigned long timeSnapshot = 0,
              lastStateChange = 0;

void showSegmentValues() {
  const static int multiplexingDelayAmount = 2;
  
  // iterate array of values and display them one by one
  for (int i = 0; i < segmentCount; ++ i) {
    int digit = digitArray[segmentValues[i]];
    if (i == segIndex && dotState == HIGH) {
      // last bit corresponds to the DP.
      // can be activated by xoring the value with 1
      digit ^= 1;
    }
    
    writeRegister(digit);
    showSegment(i);
    delay(multiplexingDelayAmount);
  }
}

void saveState(int segIndex) {
  // the value of one of the segments was modified
  // store the new value
  EEPROM.write(segIndex, segmentValues[segIndex]);
  // also save active segment index;
  EEPROM.write(segmentCount, segIndex);
}

void loadState() {
  // iterate over each segment and load it's value
  // from the corresponding eeprom index;
  for (int i = 0; i < segmentCount; ++ i) {
    segmentValues[i] = EEPROM.read(i);
  }
  // also get active segment index;
  segIndex = EEPROM.read(segmentCount);
}

void state0Init() {
  if (state == 1) {
    // if the prvious state was 1,
    // then the value was changed
    // save the new value in eeprom
    saveState(segIndex);
  }
  state = 0;
  timeSnapshot = millis();
  joyMoved = false;
  dotState = HIGH;
}

void state0Logic() {
  unsigned long timeNow = millis();
  // alternate blinking dot state
  if (timeNow - timeSnapshot > dotBlinkDelay) {
    dotState = !dotState;
    timeSnapshot = timeNow;
  }

  int joyY = analogRead(joyYPin);
  
  if (joyMoved == false && joyY > joyHighThreshold) {
    joyMoved = true;
    segIndex += 1;
  }

  if (joyMoved == false && joyY < joyLowThreshold) {
    joyMoved = true;
    segIndex -= 1;
  }

  if (joyMoved == true && joyY >= joyLowThreshold && joyY <= joyHighThreshold) {
    joyMoved = false;
  }

  // handle overflows
  if (segIndex == segmentCount) {
    segIndex = 0;
  } else if (segIndex == 255) {
    segIndex = segmentCount - 1;
  }
}

void state1Init() {
  state = 1;
  dotState = HIGH;
}

void state1Logic() {
  int joyX = analogRead(joyXPin);
  
  if (joyMoved == false && joyX > joyHighThreshold) {
    joyMoved = true;
    segmentValues[segIndex] -= 1;
  }

  if (joyMoved == false && joyX < joyLowThreshold) {
    joyMoved = true;
    segmentValues[segIndex] += 1;
  }

  if (joyMoved == true && joyX >= joyLowThreshold && joyX <= joyHighThreshold) {
    joyMoved = false;
  }

  // handle overflows
  if (segmentValues[segIndex] == 10) {
    segmentValues[segIndex] = 0;
  } else if (segmentValues[segIndex] == -1) {
    segmentValues[segIndex] = 9;
  }
}

void switchStateISR() {
  unsigned long timeNow = millis();
  // don't switch states too often
  if (timeNow - lastStateChange < stateChangeDelay) {
    return;
  }
  lastStateChange = timeNow;
  
  switch (state) {
    case 0:
      state1Init();
      break;
    case 1:
      state0Init();
      break;
  }
}

void setup () {
  pinMode(joySWPin, INPUT_PULLUP);
  // switch state when pressing down the button
  attachInterrupt(digitalPinToInterrupt(joySWPin),
      switchStateISR, FALLING);
  
  pinMode(joyYPin, INPUT);
  pinMode(joyXPin, INPUT);
  
  pinMode(dataPin, OUTPUT);
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  for (int i = 0; i < segmentCount; ++ i) {
    pinMode(displaySegments[i], OUTPUT);
  }

  state0Init();  
  loadState();
}

void loop() {
  switch (state) {
    case 0:
      state0Logic();
      break;
    case 1:
      state1Logic();
      break;
  }
  
  // do this constantly because we're multiplexing
  showSegmentValues();
}
