const int potCount = 3, ledCount = 3;
//small maxLedBrigtness because the led is very bright
// at higher coresponding voltages
const int minPotValue = 0, maxPotValue = 1023,
          minLedBrightness = 0, maxLedBrightness = 32;

const int potPins[potCount] = {A0, A1, A2};
const int ledPins[ledCount] = {11, 10, 9};


void setArrayPinMode(int *arr, int len, int mode) {
  for (int i = 0; i < len; ++ i) {
    pinMode(arr[i], mode);
  }
}


void setup() {
  Serial.begin(9600);
  
  setArrayPinMode(potPins, potCount, INPUT);
  setArrayPinMode(ledPins, ledCount, OUTPUT);
}

void loop() {
  for (int i = 0; i < potCount; ++ i) {
    int potValue = analogRead(potPins[i]);
    int ledBrightness = map(potValue, minPotValue, maxPotValue,
                            minLedBrightness, maxLedBrightness);
    analogWrite(ledPins[i], ledBrightness);
  }
}
