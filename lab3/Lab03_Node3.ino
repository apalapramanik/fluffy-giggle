/*
      Both the TX and RX ProRF boards will need a wire antenna. We recommend a 3" piece of wire.
      This example is a modified version of the example provided by the Radio Head
      Library which can be found here:
      www.github.com/PaulStoffregen/RadioHeadd
    */
/*
WDT every 6 sconds, kick every time message recieved or sent, if long enough send error and node one restarts the chain
*/


#include <SPI.h>
#include <RH_RF95.h>
#include <TemperatureZero.h>
#include <FlashStorage.h>

#define CPU_HZ 48000000
#define TIMER_PRESCALER_DIV 1024
#define nodeID 3

//error codes definition
#define ERROR_CODE_NONE 0
#define ERROR_CODE_RESET 1
#define ERROR_CODE_PACKET_RECEPTION 2
#define ERROR_CODE_MISSING_PACKET 3
#define ERROR_CODE_WDT 4

// We need to provide the RFM95 module's chip select and interrupt pins to the
// rf95 instance below.On the SparkFun ProRF those pins are 12 and 6 respectively.
RH_RF95 rf95(12, 6);

int LED = 13;  //Status LED is on pin 13

int packetCounter = 0;         //Counts the number of packets sent
long timeSinceLastPacket = 0;  //Tracks the time stamp of last packet received

// The broadcast frequency is set to 921.2, but the SADM21 ProRf operates
// anywhere in the range of 902-928MHz in the Americas.
// Europe operates in the frequencies 863-870, center frequency at 868MHz.
// This works but it is unknown how well the radio configures to this frequency:
float frequency = 902;  //Broadcast frequency

bool myTurn = false;
bool readyToSend = false;

// Temperature related variables
TemperatureZero TempZero = TemperatureZero();
FlashStorage(storedCounter, int);
int tempCounter = storedCounter.read();
float storage[5];

//hold last packet counter
FlashStorage(node1Counter, int);
FlashStorage(node2Counter, int);
FlashStorage(node3Counter, int);
FlashStorage(node4Counter, int);

//arrays to hold last packet counter int
int arrayNode1[2] = { -1, node1Counter.read() };
int arrayNode2[2] = { -1, node2Counter.read() };
int arrayNode3[2] = { -1, node3Counter.read() };
int arrayNode4[2] = { -1, node4Counter.read() };

//a struct for flash storage
struct tempPackage {
  char data[50];
};

struct tempBool {
  bool haveError;
};

//flashstorage to hold temp data
FlashStorage(node1, tempPackage);
FlashStorage(node2, tempPackage);
FlashStorage(node3, tempPackage);
FlashStorage(node4, tempPackage);
FlashStorage(error, tempPackage);
FlashStorage(errorBool, tempBool);

//setting haveError bool
tempBool storageBool = errorBool.read();
bool haveError = storageBool.haveError;

void setup() {
  pinMode(LED, OUTPUT);

  SerialUSB.begin(9600);
  // It may be difficult to read serial messages on startup. The following line
  // will wait for serial to be ready before continuing. Comment out if not needed.
  while (!SerialUSB)
    ;
  SerialUSB.println("RFM Client!");

  //Initialize the Radio.
  if (rf95.init() == false) {
    SerialUSB.println("Radio Init Failed - Freezing");
    while (1)
      ;
  } else {
    //An LED inidicator to let us know radio initialization has completed.
    SerialUSB.println("Receiver up!");
    digitalWrite(LED, HIGH);
    delay(500);
    digitalWrite(LED, LOW);
    delay(500);
  }

  // Set frequency
  rf95.setFrequency(frequency);

  // Transmitter power can range from 14-20dbm.
  rf95.setTxPower(20, true);

  // Initialize temp sensor
  TempZero.init();

  //setup WDT
  startWDT();

  // start the timer (TC4) to run every 1sec
  startTimer(1);
}

void setTimerFrequency(int frequencyHz) {
  int compareValue = (CPU_HZ / (TIMER_PRESCALER_DIV * frequencyHz)) - 1;
  TcCount16* TC = (TcCount16*)TC4;

  TC->COUNT.reg = map(TC->COUNT.reg, 0, TC->CC[0].reg, 0, compareValue);
  TC->CC[0].reg = compareValue;
  while (TC->STATUS.bit.SYNCBUSY == 1)
    ;

  while (TC->STATUS.bit.SYNCBUSY == 1)
    ;
}

void startTimer(int frequencyHz) {
  REG_GCLK_CLKCTRL = (uint16_t)(GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_ID_TC4_TC5);
  while (GCLK->STATUS.bit.SYNCBUSY == 1)
    ;  // wait for sync

  TcCount16* TC = (TcCount16*)TC4;

  TC->CTRLA.reg &= ~TC_CTRLA_ENABLE;  //Disable timer
  while (TC->STATUS.bit.SYNCBUSY == 1)
    ;  // wait for sync

  // Use the 16-bit timer
  TC->CTRLA.reg = TC_CTRLA_MODE_COUNT16 | TC_CTRLA_WAVEGEN_MFRQ | TC_CTRLA_PRESCALER_DIV1024;
  while (TC->STATUS.bit.SYNCBUSY == 1)
    ;  // wait for sync

  setTimerFrequency(frequencyHz);

  TC->INTENSET.reg = 0;
  TC->INTENSET.bit.OVF = 1;

  NVIC_EnableIRQ(TC4_IRQn);
  TC->CTRLA.reg |= TC_CTRLA_ENABLE;
  while (TC->STATUS.bit.SYNCBUSY == 1)
    ;
}

void TC4_Handler() {
  TcCount16* TC = (TcCount16*)TC4;
  // check every time then if 5 send average
  // tempCounter++;
  float temperature = TempZero.readInternalTemperature();
  storage[tempCounter++] = temperature;  //fix

  if (TC->INTFLAG.bit.OVF == 1) {
    TC->INTFLAG.bit.OVF = 1;

    if (tempCounter == 5) {
      tempCounter = 0;
      readyToSend = true;
    }
  }
}

void sendPacket() {
  float sum = storage[0] + storage[1] + storage[2] + storage[3] + storage[4];
  float average = sum / 5;

  //sendTemp here
  char toSend[50];
  // if (myTurn) {
  SerialUSB.println("Sending message");
  packetCounter++;
  //Concatenate the packet counter value to the message
  sprintf(toSend, "%d,%d,%lu,%.2f", nodeID, packetCounter, millis(), average);

  //send packet 3 times
  //rf95.send((uint8_t*)toSend, sizeof(toSend));
  //rf95.waitPacketSent();
  //rf95.send((uint8_t*)toSend, sizeof(toSend));
  //rf95.waitPacketSent();
  rf95.send((uint8_t*)toSend, sizeof(toSend));
  rf95.waitPacketSent();
  
  // myTurn = false;
  SerialUSB.println("Sent message");
  // }
}


void loop() {
  if (myTurn && haveError) {
    //send error code
    sendError();
  }
  if (readyToSend && myTurn) {
    sendPacket();
    //kick WDT
    WDT->CLEAR.reg = WDT_CLEAR_CLEAR_KEY;

    myTurn = false;
    readyToSend = false;
  }
  if (rf95.available()) {
    // Should be a message for us now
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);

    if (rf95.recv(buf, &len)) {
      myTurn = false;
      digitalWrite(LED, HIGH);         //Turn on status LED
      timeSinceLastPacket = millis();  //Timestamp this packet

      SerialUSB.print("Got message: ");
      SerialUSB.print((char*)buf);
      SerialUSB.print(" RSSI: ");
      SerialUSB.print(rf95.lastRssi(), DEC);
      SerialUSB.println();

      //myTurn = true;  //here call test function
      test((char*)buf);

    } else
      logErrorInFlash(ERROR_CODE_PACKET_RECEPTION);
    //SerialUSB.println("Recieve failed");
  } else {
    // SerialUSB.println("No message available!");
  }
  //Turn off status LED if we haven't received a packet after 1s
  if (millis() - timeSinceLastPacket > 1000) {
    digitalWrite(LED, LOW);          //Turn off status LED
    timeSinceLastPacket = millis();  //Don't write LED but every 1s
  }
}

void test(char* buf) {
  //storing package in a struct
  tempPackage package;
  strcpy(package.data, (char*)buf);

  //kick WDT
  WDT->CLEAR.reg = WDT_CLEAR_CLEAR_KEY;

  int node;
  int counter;
  long timestamp;
  float average;
  sscanf((char*)buf, "%d,%d,%lu,%f", &node, &counter, &timestamp, &average);

  //check where goes //add check
  if (counter == -1) {  //check if error
    //DO nothing
  } else if (node == 1) {
    node1.write(package);
    //store the last packet counter
    arrayNode1[0] = arrayNode1[1];
    arrayNode1[1] = counter;
    if (arrayNode1[1] != (arrayNode1[0] + 1)) {
      //send error missing package
      logErrorInFlash(ERROR_CODE_MISSING_PACKET);
    }
  } else if (node == 2) {
    node2.write(package);
    //store last
    arrayNode2[0] = arrayNode2[1];
    arrayNode2[1] = counter;
    if (arrayNode3[1] != (arrayNode3[0] + 1)) {
      //send error missing package
      logErrorInFlash(ERROR_CODE_MISSING_PACKET);
    }
    delay(100);
    myTurn = true;
  } else if (node == 4) {
    node4.write(package);
    //store last
    arrayNode4[0] = arrayNode4[1];
    arrayNode4[1] = counter;
    if (arrayNode4[1] != (arrayNode4[0] + 1)) {
      //send error missing package
      logErrorInFlash(ERROR_CODE_MISSING_PACKET);
    }
  }
}

void logErrorInFlash(int errorCode) {
  char toSendError[50];
  sprintf(toSendError, "%d,-1,%lu,%.2f", nodeID, millis(), (float)errorCode);
  SerialUSB.print("Storing Error: ");
  SerialUSB.println(toSendError);
  tempPackage errorString;
  strcpy(errorString.data, toSendError);
  error.write(errorString);
  haveError = true;
  //add code here
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
    storedCounter.write(tempCounter);
    node1Counter.write(arrayNode1[1]);
    node2Counter.write(arrayNode2[1]);
    node3Counter.write(arrayNode3[1]);
    node4Counter.write(arrayNode4[1]);
    tempBool errorBoolStorage;
    errorBoolStorage.haveError = haveError;
    errorBool.write(errorBoolStorage);
    //WDT->CLEAR.reg = WDT_CLEAR_CLEAR_KEY;
    SerialUSB.println("WDT triggered");
    logErrorInFlash(ERROR_CODE_WDT);
    //REG_DSU_STATUSA
    //REG_DSU_STATUSB
    //only for node 1
    //myTurn = true;
  }
}

void sendError() {
  tempPackage errorPackage = error.read();                            // Read from flash storage
  rf95.send((uint8_t*)errorPackage.data, sizeof(errorPackage.data));  // Send the error package
  rf95.waitPacketSent();
  SerialUSB.println("Sent Error");
  haveError = false;
}