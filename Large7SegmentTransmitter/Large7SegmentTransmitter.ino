// PROJECT  :Large7SegmentTransmitter
// PURPOSE  :DES Day Counter Transmitter
// DEVICE   :Genuine Nano + nRF24L01 + Rotary Encoder
// AUTHOR   :C. D'Arcy
// DATE     :2025 03 16-...
// uC       :328p (Nano)
// COURSE   :*
// STATUS   :Working. Finally.  Paired with Large7SegmentReceiver.ino or RxSlave.ino (small 7 segment)
//          :It was the knockoff Nano that couldn't handle the power.
//          :When I switched to the Genuine Nano, it worked.
// PROJECT  :http://darcy.rsgc.on.ca/ACES/Projects/DayCounter/index.html
// REFERENCE:http://darcy.rsgc.on.ca/ACES/TEI3M/RotaryEncoder/images/RotaryEncoderDecisionTree.png
// PHOTO    :http://darcy.rsgc.on.ca/ACES/Projects/DayCounter/images/DayCounterTransmitter.jpg
#include <SPI.h>
#include <RF24.h>
RF24 radio(9, 10);  //CE (Chip Enable/Disable), CSN(SS)
byte addresses[][6] = { "DESDC", "2Node" };
#define CHANNEL 91  //Any value between 0 and 125 (2.400GHz to 2.525GHz)

#define DEBUG true  //Conditional use of Serial support
#define RE_PINA 2   // A
#define RE_PINB 3   // B
boolean CW = true;  //direction: CW or CCW (not CW)
volatile boolean triggered = false;
uint8_t day = 0;  // self-explanatory

void setup() {
  if (DEBUG) {
    Serial.begin(9600);
    while (!Serial)
      ;
  }

  radio.begin();                  //invoke the radio object
  radio.setPALevel(RF24_PA_MIN);  //MIN/LOW/MED/HIGH close range so minimum power sufficient
  radio.setChannel(CHANNEL);      //Tx and Rx communication on same channel
  //https://tmrh20.github.io/RF24/classRF24.html#af2e409e62d49a23e372a70b904ae30e1
  radio.openWritingPipe(addresses[0]);  //Transmit assumes these pipes
  // radio.openReadingPipe(1, addresses[1]); //...
  radio.stopListening();

  attachInterrupt(digitalPinToInterrupt(RE_PINA), ISR_Rotary, RISING);
}

void loop() {
  if (triggered) {
    triggered = false;
    //1. First, determine the direction (hardware (logic) strategy)...
    CW = digitalRead(RE_PINA) ^ digitalRead(RE_PINB);
    //2. Now, determine whether to decrement or increment the day number
    if (CW)
      day = (day + 1) % 10;
    else
      day = (day == 0) ? 9 : day - 1;
    if (DEBUG) {  //Conditional Serial support for debugging purposes
      Serial.println(CW);
      displayStates(day);
    }
    //3. Finally, Transmit...
    radio.write(&day, 1);  //transmit day number, one byte
  }
 // delay(30);
}

void ISR_Rotary() {  //Keep ISR bodies as short as possible
  triggered = true;
}

//for Debugging purposes, if necessary...
void displayStates(uint8_t dayNumber) {
  Serial.print(dayNumber);
  Serial.print(',');
  uint8_t stateA = digitalRead(RE_PINA);
  uint8_t stateB = digitalRead(RE_PINB);
  Serial.print(stateA);
  Serial.print(',');
  Serial.println(stateB);
}
