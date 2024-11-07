#define CPU_HZ 48000000
#define TIMER_PRESCALER_DIV 1024

// Timer-related functions
void startTimer(TcCount16* TC, int frequencyHz, uint32_t gclkCtrlID);
void setTimerFrequency(TcCount16* TC, int frequencyHz);

// TC3 and TC4 Interrupt Handlers
void TC3_Handler();
void TC4_Handler();

// LED pins (update to actual values from your board)
#define BLUE_LED_PIN PIN_LED_13  // Blue LED (connected to D13)
#define YELLOW_LED_PIN PIN_LED_RXL  // Yellow LED (connected to RX)

// LED state variables
bool isBlueLEDOn = false;
bool isYellowLEDOn = false;

void setup() {
  // Initialize serial for debug messages
  SerialUSB.begin(9600);
  while (!SerialUSB);

  // Set up the LED pins
  pinMode(BLUE_LED_PIN, OUTPUT);
  pinMode(YELLOW_LED_PIN, OUTPUT);

  // Start the timers with the required frequencies
  startTimer((TcCount16*)TC3, 1, GCLK_CLKCTRL_ID_TCC2_TC3);  // Blue LED toggles every 1 second
  startTimer((TcCount16*)TC4, 2, GCLK_CLKCTRL_ID_TC4_TC5);  // Yellow LED toggles with 2 seconds on, 0.5 seconds off
}

void loop() {
  // No need for logic here as everything is handled in interrupts
}

// Configure the timer with optimized setup
void startTimer(TcCount16* TC, int frequencyHz, uint32_t gclkCtrlID) {
  
  // Set the clock for the timer
  REG_GCLK_CLKCTRL = (uint16_t)(GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK0 | gclkCtrlID);
  while (GCLK->STATUS.bit.SYNCBUSY);  // Wait for synchronization

  // Disable the timer and configure it
  TC->CTRLA.reg &= ~TC_CTRLA_ENABLE;  // Disable the timer
  while (TC->STATUS.bit.SYNCBUSY);  // Wait for synchronization

  // Configure for 16-bit mode, match frequency operation, and 1024 prescaler
  TC->CTRLA.reg = TC_CTRLA_MODE_COUNT16 | TC_CTRLA_WAVEGEN_MFRQ | TC_CTRLA_PRESCALER_DIV1024;
  while (TC->STATUS.bit.SYNCBUSY);  // Wait for synchronization

  // Set the timer frequency based on the input frequency
  setTimerFrequency(TC, frequencyHz);

  // Enable overflow interrupt (OVF)
  TC->INTENSET.reg = TC_INTENSET_OVF;

  // Enable the timer interrupt in NVIC
  NVIC_EnableIRQ(TC == (TcCount16*)TC3 ? TC3_IRQn : TC4_IRQn);

  // Enable the timer
  TC->CTRLA.reg |= TC_CTRLA_ENABLE;
  while (TC->STATUS.bit.SYNCBUSY);  // Wait for synchronization
}

// Set the compare value for the timer based on desired frequency
void setTimerFrequency(TcCount16* TC, int frequencyHz) {
  int compareValue = (CPU_HZ / (TIMER_PRESCALER_DIV * frequencyHz)) - 1;
  TC->CC[0].reg = compareValue;  // Set compare register
  while (TC->STATUS.bit.SYNCBUSY);  // Wait for synchronization
}

// TC3 interrupt handler for Blue LED (1 second on, 1 second off)
void TC3_Handler() {
  TcCount16* TC = (TcCount16*) TC3;

  // Check if the overflow interrupt flag is set
  if (TC->INTFLAG.bit.OVF == 1) {
    TC->INTFLAG.bit.OVF = 1;  // Clear the interrupt flag

    // Toggle the blue LED
    isBlueLEDOn = !isBlueLEDOn;
    digitalWrite(BLUE_LED_PIN, isBlueLEDOn);

    // Print the state change
    if (isBlueLEDOn) {
      SerialUSB.println("Blue LED is on.");
    } else {
      SerialUSB.println("Blue LED is off.");
    }
  }
}

// TC4 interrupt handler for Yellow LED (2 seconds on, 0.5 seconds off)
void TC4_Handler() {
  static int yellowCounter = 0;
  TcCount16* TC = (TcCount16*) TC4;

  // Check if the overflow interrupt flag is set
  if (TC->INTFLAG.bit.OVF == 1) {
    TC->INTFLAG.bit.OVF = 1;  // Clear the interrupt flag

    // Control the yellow LED based on a counter
    if (yellowCounter < 4) {
      // LED is ON for 2 seconds (4 interrupts of 0.5 seconds each)
      if (!isYellowLEDOn) {
        isYellowLEDOn = true;
        digitalWrite(YELLOW_LED_PIN, isYellowLEDOn);
        SerialUSB.println("Yellow LED is on.");
      }
    } else if (yellowCounter >= 4 && yellowCounter < 5) {
      // LED is OFF for 0.5 seconds
      if (isYellowLEDOn) {
        isYellowLEDOn = false;
        digitalWrite(YELLOW_LED_PIN, isYellowLEDOn);
        SerialUSB.println("Yellow LED is off.");
      }
    }

    // Increment the counter and reset it after the full cycle
    yellowCounter = (yellowCounter + 1) % 5;
  }
}
