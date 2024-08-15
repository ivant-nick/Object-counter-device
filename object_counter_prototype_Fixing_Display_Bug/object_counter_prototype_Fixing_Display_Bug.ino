#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>

// LCD1602 I2C address
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Keypad setup
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {3, 4, 5, 6}; // Connect to the row pinouts of the keypad
byte colPins[COLS] = {7, 8, 9, 10}; // Connect to the column pinouts of the keypad
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// Pins
const int laserPin = 2; // Laser sensor digital pin
const int photoResistorPin = A0; // Photoresistor analog pin
const int ledPin = 11; // LED pin

// Variables
unsigned long objectCount = 0; // Use 'unsigned long' to handle 6-digit numbers
unsigned long objectLimit = -1; // -1 means no limit set, using unsigned long to match
boolean isObjectDetected = false;
boolean settingLimit = false;
String limitInput = "";
unsigned long limitSetStartTime = 0; // To track the time spent in the Set Limit menu

void setup() {
  // Initialize LCD
  lcd.init();
  lcd.backlight();
  
  // Initialize pin modes
  pinMode(laserPin, INPUT);
  pinMode(photoResistorPin, INPUT);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  // Display initial message
  lcd.setCursor(0, 0);
  lcd.print("Object Count:");
  lcd.setCursor(0, 1);
  lcd.print(objectCount);

  Serial.begin(9600);
}

void loop() {
  // Laser beam detection (works continuously, even in Set Limit mode)
  int sensorValue = analogRead(photoResistorPin);
  Serial.println(sensorValue);

  if (sensorValue < 212 && !isObjectDetected) { // Threshold value, adjust if needed
    isObjectDetected = true;
  } else if (sensorValue >= 212 && isObjectDetected) {
    isObjectDetected = false;
    objectCount++;
    checkLimit();
    // Update display only if not setting limit
    if (!settingLimit) {
      updateDisplay();
    }
  }

  // Keypad handling
  char key = keypad.getKey();
  if (key) {
    handleKeypadInput(key);
    limitSetStartTime = millis(); // Reset the timer whenever a key is pressed
  }

  if (settingLimit) {
    // Check if 15 seconds have passed without input
    if (millis() - limitSetStartTime > 15000) {
      settingLimit = false;
      updateDisplay(); // Return to the Object Count display
    }
  }
}

void handleKeypadInput(char key) {
  if (settingLimit) {
    if (key >= '0' && key <= '9') {
      if (limitInput.length() < 6) { // Limit input to 6 digits
        limitInput += key;
        lcd.setCursor(0, 1);
        lcd.print(limitInput + "      "); // Clear trailing characters
      }
    } else if (key == 'A') {
      objectLimit = limitInput.toInt();
      settingLimit = false;
      limitInput = "";
      updateDisplay();
    } else if (key == 'C') {
      settingLimit = false;
      limitInput = "";
      updateDisplay();
    }
  } else {
    if (key == 'B') {
      settingLimit = true;
      limitInput = "";
      limitSetStartTime = millis(); // Start the timer
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Set Limit:      ");
      lcd.setCursor(0, 1);
      lcd.print(limitInput + "      "); // Clear trailing characters
    } else if (key == 'D') {
      objectLimit = -1;  // Cancel the limit
      updateDisplay();
    } else if (key == '*') {
      objectCount = 0;  // Reset the object count
      updateDisplay();
    }
  }
}

void updateDisplay() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Object Count:");
  lcd.setCursor(0, 1);
  lcd.print(objectCount);
  if (objectLimit != -1) {
    lcd.print(" / ");
    lcd.print(objectLimit);
  }
}

void checkLimit() {
  if (objectLimit != -1 && objectCount >= objectLimit) {
    digitalWrite(ledPin, HIGH);
  } else {
    digitalWrite(ledPin, LOW);
  }
}
