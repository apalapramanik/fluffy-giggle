
/*******************************************************************************
     * Copyright (c) 2015 Matthijs Kooijman
     * Copyright (c) 2018 Terry Moore, MCCI Corporation
     *
     * Permission is hereby granted, free of charge, to anyone
     * obtaining a copy of this document and accompanying files,
     * to do whatever they want with them without any restriction,
     * including, but not limited to, copying, modification and redistribution.
     * NO WARRANTY OF ANY KIND IS PROVIDED.
     *
     * This example transmits data on hardcoded channel and receives data
     * when not transmitting. Running this sketch on two nodes should allow
     * them to communicate.
     *******************************************************************************/

#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>
#include <TemperatureZero.h>
#include <FlashStorage.h>
#include <math.h>  // Ensure this is included for pow() function



// we formerly would check this configuration; but now there is a flag,
// in the LMIC, LMIC.noRXIQinversion;
// if we set that during init, we get the same effect.  If
// DISABLE_INVERT_IQ_ON_RX is defined, it means that LMIC.noRXIQinversion is
// treated as always set.
//
// #if !defined(DISABLE_INVERT_IQ_ON_RX)
// #error This example requires DISABLE_INVERT_IQ_ON_RX to be set. Update \
    //        lmic_project_config.h in arduino-lmic/project_config to set it.
// #endif

// How often to send a packet. Note that this sketch bypasses the normal
// LMIC duty cycle limiting, so when you change anything in this sketch
// (payload length, frequency, spreading factor), be sure to check if
// this interval should not also be increased.
// See this spreadsheet for an easy airtime and duty cycle calculator:
// https://docs.google.com/spreadsheets/d/1voGAtQAjC1qBmaVuP1ApNKs1ekgUjavHuVQIXyYSvNc
#define TX_INTERVAL 5000  //Delay between each message in millidecond.


// Pin mapping for SAMD21
const lmic_pinmap lmic_pins = {
  .nss = 12,  //RFM Chip Select
  .rxtx = LMIC_UNUSED_PIN,
  .rst = 7,              //RFM Reset
  .dio = { 6, 10, 11 },  //RFM Interrupt, RFM LoRa pin, RFM LoRa pin
};

// These callbacks are only used in over-the-air activation, so they are
// left empty here (we cannot leave them out completely unless
// DISABLE_JOIN is set in arduino-lmoc/project_config/lmic_project_config.h,
// otherwise the linker will complain).
void os_getArtEui(u1_t* buf) {}
void os_getDevEui(u1_t* buf) {}
void os_getDevKey(u1_t* buf) {}

void onEvent(ev_t ev) {
}

osjob_t txjob;
osjob_t timeoutjob;
int p = 0;
static void tx_func(osjob_t* job);

// Temperature related variables
TemperatureZero TempZero = TemperatureZero();
FlashStorage(storedCounter, int);
int tempCounter = 0;
int timeCounter = 0;
int nodeID = 4;        //TODO: your node here
char* name = "Ani";  //TODO: your name here
float storage[5];

void floatToStr(char* buffer, float value, int precision = 2) {
  int integerPart = (int)value;                                              // Extract integer part
  int decimalPart = abs((int)((value - integerPart) * pow(10, precision)));  // Extract decimal part

  // Format the string as integerPart.decimalPart
  sprintf(buffer, "%d.%02d", integerPart, decimalPart);
}
// Transmit the given string and call the given function afterwards
void tx(const char* str, osjobcb_t func) {
  os_radio(RADIO_RST);  // Stop RX first
  delay(1);             // Wait a bit, without this os_radio below asserts, apparently because the state hasn't changed yet
  LMIC.dataLen = 0;
  SerialUSB.println("Sending message");
  // SerialUSB.println(str);

  while (*str)
    LMIC.frame[LMIC.dataLen++] = *str++;
  LMIC.osjob.func = func;


  os_radio(RADIO_TX);
  SerialUSB.println("TX");
}

// Enable rx mode and call func when a packet is received
void rx(osjobcb_t func) {
  LMIC.osjob.func = func;
  LMIC.rxtime = os_getTime();  // RX _now_
  // Enable "continuous" RX (e.g. without a timeout, still stops after
  // receiving a packet)
  os_radio(RADIO_RXON);
  SerialUSB.println("RX");
}

static void rxtimeout_func(osjob_t* job) {
  digitalWrite(LED_BUILTIN, LOW);  // off
}

static void rx_func(osjob_t* job) {
  // Blink once to confirm reception and then keep the led on
  digitalWrite(LED_BUILTIN, LOW);  // off
  delay(10);
  digitalWrite(LED_BUILTIN, HIGH);  // on

  // Timeout RX (i.e. update led status) after 3 periods without RX
  os_setTimedCallback(&timeoutjob, os_getTime() + ms2osticks(3 * TX_INTERVAL), rxtimeout_func);

  // Reschedule TX so that it should not collide with the other side's
  // next TX
  os_setTimedCallback(&txjob, os_getTime() + ms2osticks(TX_INTERVAL / 2), tx_func);

  SerialUSB.print("Got ");
  SerialUSB.print(LMIC.dataLen);
  SerialUSB.println(" bytes");
  SerialUSB.write(LMIC.frame, LMIC.dataLen);
  SerialUSB.println();

  // Restart RX
  rx(rx_func);
}

static void txdone_func(osjob_t* job) {
  //kick WDT
  WDT->CLEAR.reg = WDT_CLEAR_CLEAR_KEY;
}



static void tx_func(osjob_t* job) {
  // Read temperature from sensor
  float temperature = TempZero.readInternalTemperature();

  // Debug: Print the current temperature reading
  // SerialUSB.print("Current Temperature: ");
  // SerialUSB.println(temperature);

  // Store temperature in the array
  storage[tempCounter++] = temperature;

  // Reset tempCounter to 0 when it reaches 5, creating a rolling window of 5 values
  if (tempCounter >= 5) {
    tempCounter = 0;
  }

  // Calculate the sum and average of the current temperatures in the storage array
  float sum = 0;
  int count = 0;

  // Debug: Print the stored values in the array
  // SerialUSB.print("Stored Temperatures: ");
  for (int i = 0; i < 5; i++) {
    // SerialUSB.print(storage[i]);
    // SerialUSB.print(" ");
    if (storage[i] != 0) {  // Only sum up valid temperatures
      sum += storage[i];
      count++;
    }
  }
  SerialUSB.println();  // Newline for readability

  // Calculate average only if we have valid values
  float average = count > 0 ? sum / count : 0;

  // Debug: Print the calculated average
  // SerialUSB.print("Calculated Average: ");
  // SerialUSB.println(average);

  // Ensure the buffer is large enough
  char packetToSend[256];  // Increased size to prevent overflow

  // Convert the average float value to a string manually
  char avgStr[10];              // Buffer to hold the formatted average as a string
  floatToStr(avgStr, average);  // Convert float to string

  // Increment the packet counter
  p++;

  // Concatenate the packet counter value to the message
  sprintf(packetToSend, "Sender=%s,NodeID=%d,p=%d,Time=%lu,TempAvg=%s", name, nodeID, p, millis(), avgStr);

  // Debug: Print the packet to be sent
  // SerialUSB.print("Packet to send: ");
  // SerialUSB.println(packetToSend);

  // Send the packet
  tx(packetToSend, txdone_func);

  // Reschedule job every TX_INTERVAL (plus a bit of random to prevent systematic collisions)
  os_setTimedCallback(job, os_getTime() + ms2osticks(TX_INTERVAL + random(500)), tx_func);
}


void startWDT() {
  GCLK->GENDIV.reg = GCLK_GENDIV_ID(2) | GCLK_GENDIV_DIV(4);
  GCLK->GENCTRL.reg = GCLK_GENCTRL_ID(2) | GCLK_GENCTRL_GENEN | GCLK_GENCTRL_SRC_OSCULP32K | GCLK_GENCTRL_DIVSEL;
  while (GCLK->STATUS.bit.SYNCBUSY)
    ;
  GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID_WDT | GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK2;


  WDT->CTRL.reg = 0;
  WDT->CONFIG.bit.PER = 0xB;
  WDT->EWCTRL.bit.EWOFFSET = 0xA;
  WDT->INTENSET.bit.EW = 1;
  WDT->CTRL.bit.WEN = 0;
  NVIC_EnableIRQ(WDT_IRQn);
  WDT->CTRL.bit.ENABLE = 1;
  while (WDT->STATUS.bit.SYNCBUSY)
    ;
}

void WDT_Handler() {
  if (WDT->INTFLAG.bit.EW) {
    WDT->INTFLAG.bit.EW = 1;
    SerialUSB.println("WDT triggered");
    // logErrorInFlash(ERROR_CODE_WDT);
  }
}


// application entry point
void setup() {
  SerialUSB.begin(115200);
  //while(!SerialUSB);
  delay(5000);
  SerialUSB.println("Starting");
  //  #ifdef VCC_ENABLE
  //  // For Pinoccio Scout boards
  //  pinMode(VCC_ENABLE, OUTPUT);
  //  digitalWrite(VCC_ENABLE, HIGH);
  //  delay(1000);
  //  #endif

  pinMode(LED_BUILTIN, OUTPUT);

  // Initialize temp sensor
  TempZero.init();
  //setup WDT
  startWDT();
  // initialize runtime env
  os_init();
  // lmic reset values
  LMIC_reset();

  // this is automatically set to the proper bandwidth in kHz,
  // based on the selected channel.
  uint32_t uBandwidth;
  //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  LMIC.freq = 902900000;  //change this for assigned frequencies, match with int freq in loraModem.h
  //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  uBandwidth = 125;
  LMIC.datarate = US915_DR_SF7;  // DR4
  LMIC.txpow = 21;


  // disable RX IQ inversion
  LMIC.noRXIQinversion = true;

  // This sets CR 4/5, BW125 (except for EU/AS923 DR_SF7B, which uses BW250)
  LMIC.rps = updr2rps(LMIC.datarate);

  SerialUSB.print("Frequency: ");
  SerialUSB.print(LMIC.freq / 1000000);
  SerialUSB.print(".");
  SerialUSB.print((LMIC.freq / 100000) % 10);
  SerialUSB.print("MHz");
  SerialUSB.print("  LMIC.datarate: ");
  SerialUSB.print(LMIC.datarate);
  SerialUSB.print("  LMIC.txpow: ");
  SerialUSB.println(LMIC.txpow);

  // This sets CR 4/5, BW125 (except for DR_SF7B, which uses BW250)
  LMIC.rps = updr2rps(LMIC.datarate);

  // disable RX IQ inversion
  LMIC.noRXIQinversion = true;

  SerialUSB.println("Started");
  SerialUSB.flush();

  // setup initial job
  os_setCallback(&txjob, tx_func);
}

void loop() {
  // execute scheduled jobs and events
  os_runloop_once();
}
