#include <Arduino.h>

#define CPU_HZ 48000000
#define TIMER_PRESCALER_DIV 1024

void startTimer(int frequencyHz);
void setTimerFrequency(int frequencyHz);
void TC3_Handler();

bool isBlueLEDOn = false;
bool isYellowLEDOn = false;

unsigned long blueLEDTime = 0;
unsigned long yellowLEDTime = 0;
unsigned long lastYellowToggleTime = 0;
unsigned long currentTime = 0;

void setup() {
  SerialUSB.begin(9600);
  pinMode(PIN_LED_13, OUTPUT);  // Blue LED
  pinMode(PIN_LED_RXL, OUTPUT); // Yellow LED
  startTimer(4); // Set timer to 4 Hz for the desired interval
}

void loop() {
  // Everything is handled in the timer interrupt
}

void setTimerFrequency(int frequencyHz) {
  int compareValue = (CPU_HZ / (TIMER_PRESCALER_DIV * frequencyHz)) - 1;
  TcCount16* TC = (TcCount16*) TC3;

  // Adjust count to prevent jitter when changing the compare value
  TC->COUNT.reg = map(TC->COUNT.reg, 0, TC->CC[0].reg, 0, compareValue);
  TC->CC[0].reg = compareValue;
  while (TC->STATUS.bit.SYNCBUSY == 1);
}

void startTimer(int frequencyHz) {
  REG_GCLK_CLKCTRL = (uint16_t) (GCLK_CLKCTRL_CLKEN |
                                  GCLK_CLKCTRL_GEN_GCLK0 |
                                  GCLK_CLKCTRL_ID_TCC2_TC3); // Set clock 0 to TC2 and TC3

  while (GCLK->STATUS.bit.SYNCBUSY == 1); // Wait for sync

  TcCount16* TC = (TcCount16*) TC3;  // Efficient way to do things, cast TC3 to TC.

  // Disable timer
  TC->CTRLA.reg &= ~TC_CTRLA_ENABLE;
  while (TC->STATUS.bit.SYNCBUSY == 1); // Wait for sync

  // Use the 16-bit timer
  TC->CTRLA.reg |= TC_CTRLA_MODE_COUNT16;
  while (TC->STATUS.bit.SYNCBUSY == 1); // Wait for sync

  // Use match mode so the timer counter resets when the count matches the compare register
  TC->CTRLA.reg |= TC_CTRLA_WAVEGEN_MFRQ;
  while (TC->STATUS.bit.SYNCBUSY == 1); // Wait for sync

  // Set prescaler to 1024
  TC->CTRLA.reg |= TC_CTRLA_PRESCALER_DIV1024;
  while (TC->STATUS.bit.SYNCBUSY == 1); // Wait for sync

  setTimerFrequency(frequencyHz);

  // Enable the compare interrupt
  TC->INTENSET.reg = TC_INTENSET_MC0;
  NVIC_EnableIRQ(TC3_IRQn);

  // Enable the timer
  TC->CTRLA.reg |= TC_CTRLA_ENABLE;
  while (TC->STATUS.bit.SYNCBUSY == 1); // Wait for sync
}

void TC3_Handler() {
  TcCount16* TC = (TcCount16*) TC3;
  if (TC->INTFLAG.bit.MC0 == 1) {
    TC->INTFLAG.bit.MC0 = 1; // Clear the match compare interrupt flag

    currentTime += 250; // 250 ms has passed

    // Handle Blue LED
    if (currentTime - blueLEDTime >= 1000) { // Toggle every 1 second
      isBlueLEDOn = !isBlueLEDOn;
      digitalWrite(PIN_LED_13, isBlueLEDOn ? HIGH : LOW);
      blueLEDTime = currentTime;
      SerialUSB.println(isBlueLEDOn ? "Blue LED is on" : "Blue LED is off");
    }

    // Handle Yellow LED
    if (currentTime - lastYellowToggleTime >= 2000) { // 2 seconds for on period
      if (isYellowLEDOn) {
        digitalWrite(PIN_LED_RXL, LOW);
        SerialUSB.println("Yellow LED is off");
        isYellowLEDOn = false;
        lastYellowToggleTime = currentTime; // Set last toggle time for off period
      }
    } else if (currentTime - lastYellowToggleTime >= 500) { // 0.5 seconds for off period
      if (!isYellowLEDOn) {
        digitalWrite(PIN_LED_RXL, HIGH);
        SerialUSB.println("Yellow LED is on");
        isYellowLEDOn = true;
        lastYellowToggleTime = currentTime; // Set last toggle time for on period
      }
    }
  }
}
