// Pin connections & motor parameters
const int stepPin = 1;
const int dirPin = 2;
const int stepsPerRevolution = 200;  // motor itself
const float gearRatio = 3.0;         // 5:1 (motor:output)

// Calculate steps per degree of big gear
const float stepsPerDegree = (stepsPerRevolution * gearRatio) / 360.0;

void setup() {
  pinMode(stepPin, OUTPUT);
  pinMode(dirPin, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
}

// Function to move motor by a certain number of degrees (big gear degrees)
void moveDegrees(float degrees, bool clockwise = true) {
  // Determine direction
  digitalWrite(dirPin, clockwise ? LOW : HIGH);

  // Calculate number of steps
  int stepsToMove = round(abs(degrees) * stepsPerDegree);

  // Step through each pulse
  for (int i = 0; i < stepsToMove; i++) {
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(500); // adjust for speed
    digitalWrite(stepPin, LOW);
    delayMicroseconds(500);
  }
}

void loop() {
  digitalWrite(LED_BUILTIN, HIGH);
  delay(500);
  digitalWrite(LED_BUILTIN, LOW);
  delay(500);

  // Example: move big gear by +90° then back -90°
  moveDegrees(90, true);   // CW 90 degrees
  delay(1000);
  moveDegrees(90, false);  // CCW 90 degrees
  delay(1000);
}
