#include <bluefruit.h>

// Pin connections & motor parameters
const int stepPin = 1;
const int dirPin = 2;
const int stepsPerRevolution = 200;
const float gearRatio = 3.0;
const float stepsPerDegree = (stepsPerRevolution * gearRatio) / 360.0;

// Non-blocking LED blink
unsigned long previousMillis = 0;
bool ledState = false;

// BLE UART Service
BLEUart bleuart;

void setup() {
  pinMode(stepPin, OUTPUT);
  pinMode(dirPin, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("Bluetooth Stepper Motor Controller");
  Serial.println("----------------------------------");
  
  // Initialize Bluefruit
  Bluefruit.begin();
  Bluefruit.setTxPower(4);    // Max power for better range
  Bluefruit.setName("Electrospinning 1");
  
  // Configure and start BLE UART service
  bleuart.begin();
  
  // Set up advertising
  startAdv();
  
  Serial.println("Please connect via Bluetooth!");
  Serial.println("Device name: StepperMotor");
}

void startAdv(void) {
  // Advertising packet
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addTxPower();
  
  // Include bleuart 128-bit uuid
  Bluefruit.Advertising.addService(bleuart);
  
  // Secondary Scan Response packet (optional)
  Bluefruit.ScanResponse.addName();
  
  // Start Advertising
  Bluefruit.Advertising.restartOnDisconnect(true);
  Bluefruit.Advertising.setInterval(32, 244);    // in unit of 0.625 ms
  Bluefruit.Advertising.setFastTimeout(30);      // number of seconds in fast mode
  Bluefruit.Advertising.start(0);                // 0 = Don't stop advertising after n seconds
}

// Send message via BLE and Serial
void sendMessage(String message) {
  Serial.println(message);
  if (Bluefruit.connected()) {
    bleuart.print(message + "\n");
  }
}

// Move motor by degrees
void moveDegrees(float degrees, bool clockwise = true) {
  digitalWrite(dirPin, clockwise ? LOW : HIGH);
  int stepsToMove = round(abs(degrees) * stepsPerDegree);

  String msg = "Moving " + String(degrees) + " degrees " + (clockwise ? "CW" : "CCW");
  sendMessage(msg);

  for (int i = 0; i < stepsToMove; i++) {
    digitalWrite(stepPin, HIGH);
    // delayMicroseconds(500);
    delay(90);
    digitalWrite(stepPin, LOW);
    // delayMicroseconds(500);
    delay(90);
  }
  
  sendMessage("Move complete!");
}

// Process commands
void processCommand(String command) {
  command.trim();
  
  if (command.length() == 0) return;
  
  sendMessage("Received: " + command);
  
  // Handle "moveDegrees(180, true)" style
  if (command.startsWith("moveDegrees(")) {
    int startIdx = command.indexOf('(') + 1;
    int commaIdx = command.indexOf(',');
    int endIdx = command.indexOf(')');
    
    if (commaIdx > 0 && endIdx > 0) {
      float degrees = command.substring(startIdx, commaIdx).toFloat();
      String dirStr = command.substring(commaIdx + 1, endIdx);
      dirStr.trim();
      bool clockwise = (dirStr == "true" || dirStr == "1" || dirStr.equalsIgnoreCase("cw"));
      
      moveDegrees(degrees, clockwise);
      return;
    }
  }
  
  // Handle "move 180 cw" or "move 90 ccw" style
  if (command.startsWith("move ")) {
    int firstSpace = command.indexOf(' ');
    int secondSpace = command.indexOf(' ', firstSpace + 1);
    
    if (secondSpace > 0) {
      float degrees = command.substring(firstSpace + 1, secondSpace).toFloat();
      String direction = command.substring(secondSpace + 1);
      direction.trim();
      direction.toLowerCase();
      
      bool clockwise = (direction == "cw" || direction == "true" || direction == "1");
      moveDegrees(degrees, clockwise);
      return;
    }
  }
  
  sendMessage("Invalid command!");
}

void loop() {
  // Check for BLE UART data
  if (bleuart.available()) {
    String command = "";
    while (bleuart.available()) {
      char c = (char)bleuart.read();
      if (c == '\n' || c == '\r') {
        if (command.length() > 0) {
          processCommand(command);
          command = "";
        }
      } else {
        command += c;
      }
    }
    // Process any remaining command
    if (command.length() > 0) {
      processCommand(command);
    }
  }
  
  // LED behavior
  if (Bluefruit.connected()) {
    // Solid on when connected
    digitalWrite(LED_BUILTIN, HIGH);
  } else {
    // Blink when not connected
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= 500) {
      previousMillis = currentMillis;
      ledState = !ledState;
      digitalWrite(LED_BUILTIN, ledState);
    }
  }
}