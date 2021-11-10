const int pinA = 2,
          pinB = 3,
          pinC = 4,
          pinD = 5,
          pinE = 6,
          pinF = 7,
          pinG = 8,
          pinDP = 9,
          buzzerPin = 10,
          antennaPin = A5;

// constraining limits used to avoid fluctuations
// and stabilize the readings from the antenna
const float avgLowerConstraint = 20.0,
            avgUpperConstraint = 60.0,
            readingUpperConstraint = 200,
            buzzerNoToneThreshold = 2,
            buzzerMaxTone = 1500;
            
const int segSize = 8;
int segments[segSize] = {
  pinA, pinB, pinC, pinD, pinE, pinF, pinG, pinDP
};

const int noOfDigits = 10;
byte digitMatrix[noOfDigits][segSize - 1] = {
// a  b  c  d  e  f  g
  {1, 1, 1, 1, 1, 1, 0}, // 0
  {0, 1, 1, 0, 0, 0, 0}, // 1
  {1, 1, 0, 1, 1, 0, 1}, // 2
  {1, 1, 1, 1, 0, 0, 1}, // 3
  {0, 1, 1, 0, 0, 1, 1}, // 4
  {1, 0, 1, 1, 0, 1, 1}, // 5
  {1, 0, 1, 1, 1, 1, 1}, // 6
  {1, 1, 1, 0, 0, 0, 0}, // 7
  {1, 1, 1, 1, 1, 1, 1}, // 8
  {1, 1, 1, 1, 0, 1, 1}  // 9
};

// uses the above matrix to form a digit
// on the 7 seven segment display
void displayDigit(int digit) {
  // -1 to avoid the dot
  for (int i = 0; i < segSize - 1; ++ i) {
    digitalWrite(segments[i], digitMatrix[digit][i]);
  }
}

void setup() {
  // initialize all segments as off
  for (int i = 0; i < segSize; ++ i) {
    pinMode(segments[i], OUTPUT);
    digitalWrite(segments[i], LOW);
  }

  pinMode(buzzerPin, OUTPUT);
  pinMode(antennaPin, INPUT);
}

// represents an approximation of the antenna readings
// with greater meaning given to the most recent readings
float runningAvg = 0;
// 1 / runningAvgSensitivity is the coefficient used when
// updating the running avg of the signal
const float runningAvgSensitivity = 1000;

// make the most recent values count more and the oldest count less
// to the value. more and less are defined by the sensitivity constant above
void updateRunningAverage(float newValue) {
  runningAvg = ((runningAvgSensitivity - 1.0f) * runningAvg + newValue) / runningAvgSensitivity;  
}

void loop() {
  int reading = analogRead(antennaPin);
  // avoid values that are too big
  int constrainedReading = constrain(reading, 0, readingUpperConstraint);
  updateRunningAverage(constrainedReading);

  // normalized the runningAvg
  float constrainedAvg = constrain(runningAvg, avgLowerConstraint, avgUpperConstraint);
  
  // get the correspoding digit to display
  int displayValue = map(constrainedAvg, avgLowerConstraint, avgUpperConstraint, 0, 9);
  displayDigit(displayValue);

  // allow it to be silent when reading is near minimum
  if (constrainedAvg <= avgLowerConstraint + buzzerNoToneThreshold) {
    noTone(buzzerPin);
  }
  else {
    // powerful increase in volume
    int buzzerTone = constrain(constrainedAvg * constrainedAvg, 0, buzzerMaxTone);
    tone(buzzerPin, buzzerTone);
  }
}
