

void startTimer();
void TC3_Handler();

bool isYellowLEDOn = false;
bool isBlueLEDOn = false;
uint16_t overflowCount = 0;

const uint16_t BLUE_LED_TOGGLE_INTERVAL = 32;   
const uint16_t YELLOW_LED_ON_INTERVAL = 64;     
const uint16_t YELLOW_LED_OFF_INTERVAL = 16;    
const uint16_t TOTAL_YELLOW_CYCLE = YELLOW_LED_ON_INTERVAL + YELLOW_LED_OFF_INTERVAL; // Total cycle time for yellow LED

void setup() {
  SerialUSB.begin(9600);
  pinMode(PIN_LED_13, OUTPUT);  // Blue LED
  pinMode(PIN_LED_RXL, OUTPUT); // Yellow LED

  // Configure Generic Clock Generator 2 with a 1024Hz clock
  GCLK->GENDIV.reg = GCLK_GENDIV_ID(2) | GCLK_GENDIV_DIV(46875); // 48MHz / 46875 = 1024Hz
  while (GCLK->STATUS.bit.SYNCBUSY);

  GCLK->GENCTRL.reg = GCLK_GENCTRL_ID(2) | GCLK_GENCTRL_SRC_DFLL48M | GCLK_GENCTRL_GENEN;
  while (GCLK->STATUS.bit.SYNCBUSY);

  startTimer();
}

void loop() {
  // Everything is handled in the timer interrupt
}

void startTimer() {
  // Disable the timer
  TC3->COUNT8.CTRLA.bit.ENABLE = 0;
  while (TC3->COUNT8.STATUS.bit.SYNCBUSY);

  // Configure the clock source to use Generic Clock Generator 2
  REG_GCLK_CLKCTRL = (uint16_t)(GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK2 | GCLK_CLKCTRL_ID_TCC2_TC3);
  while (GCLK->STATUS.bit.SYNCBUSY);

  // Use the 8-bit timer in Normal Frequency Operation mode with a prescaler of 256
  TC3->COUNT8.CTRLA.reg = TC_CTRLA_MODE_COUNT8 | TC_CTRLA_WAVEGEN_NFRQ | TC_CTRLA_PRESCALER_DIV256;
  while (TC3->COUNT8.STATUS.bit.SYNCBUSY);

  // Enable the overflow interrupt
  TC3->COUNT8.INTENSET.reg = TC_INTENSET_OVF;

  // Enable the IRQ for the timer
  NVIC_EnableIRQ(TC3_IRQn);

  // Enable the timer
  TC3->COUNT8.CTRLA.bit.ENABLE = 1;
  while (TC3->COUNT8.STATUS.bit.SYNCBUSY);
}

void TC3_Handler() {
  // Check for the overflow interrupt
  if (TC3->COUNT8.INTFLAG.bit.OVF) {
    TC3->COUNT8.INTFLAG.bit.OVF = 1;  // Clear the overflow interrupt flag

    overflowCount++;

    // Blue LED: Toggle every 1 second (4 overflows)
    if (overflowCount % BLUE_LED_TOGGLE_INTERVAL == 0) {
      digitalWrite(PIN_LED_13, isBlueLEDOn ? LOW : HIGH);
      SerialUSB.println(isBlueLEDOn ? "Blue LED is off" : "Blue LED is on");
      isBlueLEDOn = !isBlueLEDOn;
    }

    // Yellow LED: On for 2 seconds, off for 0.5 seconds
    if (overflowCount % TOTAL_YELLOW_CYCLE < YELLOW_LED_ON_INTERVAL) {
      if (!isYellowLEDOn) {
        digitalWrite(PIN_LED_RXL, HIGH);
        SerialUSB.println("Yellow LED is on");
        isYellowLEDOn = true;
      }
    } else {
      if (isYellowLEDOn) {
        digitalWrite(PIN_LED_RXL, LOW);
        SerialUSB.println("Yellow LED is off");
        isYellowLEDOn = false;
      }
    }
  }
}
