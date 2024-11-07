
// Define LED pins
#define BLUE_LED_PIN PIN_LED_13  // Blue LED connected to D13
#define YELLOW_LED_PIN PIN_LED_RXL // Yellow LED connected to RX pin

// Time intervals in milliseconds
#define BLUE_LED_ON_TIME 1000   // 1 second on
#define BLUE_LED_OFF_TIME 1000  // 1 second off
#define YELLOW_LED_ON_TIME 2000 // 2 seconds on
#define YELLOW_LED_OFF_TIME 500 // 0.5 second off

// Variables to track LED states
bool isBlueLedOn = false;
bool isYellowLedOn = false;

// Variables to track timing
unsigned long previousBlueLedTime = 0;
unsigned long previousYellowLedTime = 0;

void setup() {
  // Initialize serial communication for printing
  SerialUSB.begin(9600);
  while (!SerialUSB); // Wait for serial monitor to start

  // Initialize LED pins
  pinMode(BLUE_LED_PIN, OUTPUT);
  pinMode(YELLOW_LED_PIN, OUTPUT);

  // Start with both LEDs off
  digitalWrite(BLUE_LED_PIN, LOW);
  digitalWrite(YELLOW_LED_PIN, LOW);
}

void loop() {
  unsigned long currentMillis = millis();

  // Handle Blue LED timing
  if (isBlueLedOn) {
    // Check if it's time to turn off the blue LED
    if (currentMillis - previousBlueLedTime >= BLUE_LED_ON_TIME) {
      digitalWrite(BLUE_LED_PIN, LOW);
      SerialUSB.println("Blue LED is off.");
      isBlueLedOn = false;
      previousBlueLedTime = currentMillis; // Reset the timer
    }
  } else {
    // Check if it's time to turn on the blue LED
    if (currentMillis - previousBlueLedTime >= BLUE_LED_OFF_TIME) {
      digitalWrite(BLUE_LED_PIN, HIGH);
      SerialUSB.println("Blue LED is on.");
      isBlueLedOn = true;
      previousBlueLedTime = currentMillis; // Reset the timer
    }
  }

  // Handle Yellow LED timing
  if (isYellowLedOn) {
    // Check if it's time to turn off the yellow LED
    if (currentMillis - previousYellowLedTime >= YELLOW_LED_ON_TIME) {
      digitalWrite(YELLOW_LED_PIN, LOW);
      SerialUSB.println("Yellow LED is off.");
      isYellowLedOn = false;
      previousYellowLedTime = currentMillis; // Reset the timer
    }
  } else {
    // Check if it's time to turn on the yellow LED
    if (currentMillis - previousYellowLedTime >= YELLOW_LED_OFF_TIME) {
      digitalWrite(YELLOW_LED_PIN, HIGH);
      SerialUSB.println("Yellow LED is on.");
      isYellowLedOn = true;
      previousYellowLedTime = currentMillis; // Reset the timer
    }
  }
}
