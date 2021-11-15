
const int seg1Pin = 7,
          seg2Pin = 6,
          seg3Pin = 5,
          seg4Pin = 4;

const int joySWPin = 2,
          joyYPin = A1,
          joyXPin = A0; 

const int dataPin = 12, // DS
latchPin = 11, // STCP 
clockPin = 10; // SHCP

const int segmentsCount = 4;
const int displaySegments[] = {
  seg1Pin, seg2Pin, seg3Pin, seg4Pin
};

int digitArray[10] = {
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

void showSegment(int segmentIndex) {
  for (int i = 0; i < segmentsCount; ++ i) {
    digitalWrite(displaySegments[i], HIGH);
  }
  digitalWrite(displaySegments[segmentIndex], LOW);
}

int segmentValues[segmentsCount];

void initSegmentValues() {
  // TODO load from memory thingie
  for (int i = 0; i < segmentsCount; ++ i) {
    segmentValues[i] = i;
  }
}

const int joyLowThreshold = 200,
          joyHighThreshold = 800,
          dotBlinkDelay = 250;

int state = 0,
    segIndex = 0,
    dotState = HIGH;

bool joyMoved = false;
unsigned long timeSnapshot = 0; 

void showSegmentValues() {
  const int multiplexingDelayAmount = 2;
  for (int i = 0; i < segmentsCount; ++ i) {
    int digit = digitArray[segmentValues[i]];
    if (i == segIndex && dotState == HIGH) {
      digit ^= 1;
    }
    
    writeRegister(digit);
    showSegment(i);
    delay(multiplexingDelayAmount);
  }
}

void state0Init() {
  state = 0;
  timeSnapshot = millis();
  joyMoved = false;
  dotState = HIGH;
}

void state0Logic() {
  unsigned long timeNow = millis();
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

  if (segIndex == segmentsCount) {
    segIndex = 0;
  } else if (segIndex == -1) {
    segIndex = segmentsCount - 1;
  }
}

void state1Init() {
  state = 1;
  dotState = HIGH;
}

void state1Logic() {
  int joyX = analogRead(joyXPin);
  Serial.println(joyX);
  
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

  if (segmentValues[segIndex] == 10) {
    segmentValues[segIndex] = 0;
  } else if (segmentValues[segIndex] == -1) {
    segmentValues[segIndex] = 9;
  }
}

void switchStateISR() {
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
  attachInterrupt(digitalPinToInterrupt(joySWPin), switchStateISR, FALLING);
  
  pinMode(joyYPin, INPUT);
  pinMode(joyXPin, INPUT);
  
  pinMode(dataPin, OUTPUT);
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  for (int i = 0; i < segmentsCount; ++ i) {
    pinMode(displaySegments[i], OUTPUT);
    digitalWrite(displaySegments[i], LOW);
  }

  state0Init();  
  initSegmentValues();
  Serial.begin(9600);
}

int hm = 1;

void loop() {

  switch (state) {
    case 0:
      state0Logic();
      break;
    case 1:
      state1Logic();
      break;
  }
  
  showSegmentValues();
}
