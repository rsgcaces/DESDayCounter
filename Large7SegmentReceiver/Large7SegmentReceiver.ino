// PROJECT  :Large7SegmentReceiver
// PURPOSE  :Large 7-Segment Day Counter Receiver (Receiver for TxMasterRotaryEncoder.ino)
// DEVICE   :Nano + nRF24L01 + 74HC595/6 + Big 7-Segment Display
// AUTHOR   :C. D'Arcy
// DATE     :2025 03 17-...
// uC       :328p (Nano)
// COURSE   :*
// STATUS   :Working (when used with TxMasterRotaryEncoder.ino)
// REFERENCE:http://darcy.rsgc.on.ca/ACES/TEI3M/RotaryEncoder/images/RotaryEncoderDecisionTree.png
//          :https://learn.sparkfun.com/tutorials/large-digit-driver-hookup-guide/all
//          :http://darcy.rsgc.on.ca/ACES/TEI3M/CommunicationProtocols.html#RF1
//          :https://tmrh20.github.io/RF24/classRF24.html
//          :http://darcy.rsgc.on.ca/ACES/Datasheets/nRF24L01.pdf
// PROJECT  :http://darcy.rsgc.on.ca/ACES/Projects/DayCounter/index.html
// PHOTO    :http://darcy.rsgc.on.ca/ACES/Projects/DayCounter/images/DayCounterReceiver.jpg
#include <SPI.h>  //Not sure if this is required....
#include <RF24.h>
#define CHANNEL 91                              //any value between 0 and 125 (2.400GHz to 2.525GHz)
uint8_t receivedData;                           //for receiver to store the incoming byte
RF24 radio(9, 10);                              //CE (Chip Enable/Disable), CSN(SS)
uint8_t addresses[][6] = { "DESDC", "2Node" };  //pipe names (See Reference 2)

#define DEBUG false  //Conditional use of Serial support
#define DATA 7       //74HC595/TPIC6C596 on the Sparkfun Driver Board (WIG-13279)
#define CLOCK 6      //
#define LATCH 5      //
#define RE_PINA 2    // possibly nRF24L01: IRQ at some point
#define RE_PINB 3    // ?

// Developed on Small 0.56" 7-Segment Display
uint8_t segments[] = {
  //abcdefg0
  B11111100, B01100000, B11011010, B11110010,  //0..3
  B01100110, B10110110, B10111110, B11100000,  //4..7
  B11111110, B11100110                         //8..9
};
// Live on Large 6.5" Sparkfun 7-Segment Display (COM-08530)
uint8_t Segments[] = {
  //Bafgedcb0  LSBFIRST!  Kind of an unusal arrangment
  B11011110, B00000110, B10111010, B10101110,  //0..3
  B01100110, B11101100, B11111100, B10000110,  //4..7
  B11111110, B11100110                         //8..9
};
uint8_t numSegments = sizeof(segments);

volatile boolean triggered = false;
uint8_t day = 0;

void setup() {
  if (DEBUG) {
    Serial.begin(9600);
    while (!Serial)
      ;
  }
  //prepare shift register control pins
  pinMode(DATA, OUTPUT);
  pinMode(CLOCK, OUTPUT);
  pinMode(LATCH, OUTPUT);
  // Test shiftout...
  if (DEBUG) {
    digitalWrite(LATCH, LOW);
    shiftOut(DATA, CLOCK, LSBFIRST, Segments[day]);
    digitalWrite(LATCH, HIGH);
    //while(1);
  }
  // (Not Implemented yet) External interrupt signalling new incoming RF data...
  attachInterrupt(digitalPinToInterrupt(RE_PINA), ISR_nRFIRQ, CHANGE);

  radio.begin();                  //invoke the radio
  radio.setPALevel(RF24_PA_MIN);  //set low power amplification for close proximity
  radio.setChannel(CHANNEL);      //mutually agreeable frequency offset(2.400GHz to 2.525GHz)

  radio.openWritingPipe(addresses[1]);     //select ONE addresses to write to
  radio.openReadingPipe(1, addresses[0]);  //receiver and transmit are reversed
  radio.startListening();                  //should be good to go...

  if (DEBUG)
    displayAll();
}

void loop() {
  if (DEBUG) {
    for (uint8_t i = 0; i < 10; i++) {
      digitalWrite(LATCH, LOW);
      shiftOut(DATA, CLOCK, LSBFIRST, Segments[i]);
      digitalWrite(LATCH, HIGH);
      delay(1000);
    }
  }
  if (radio.available()) {  //anything to be read?
    radio.read(&day, 1);    //grab the day number [0..9]
    digitalWrite(LATCH, LOW);
    shiftOut(DATA, CLOCK, LSBFIRST, Segments[day]);
    digitalWrite(LATCH, HIGH);
    if (DEBUG)
      Serial.println(day);  //local echo
  }
  //if and when IRQ interrupt enabled...
  if (triggered) {
    triggered = false;
    digitalWrite(LATCH, LOW);
    shiftOut(DATA, CLOCK, LSBFIRST, segments[day]);
    digitalWrite(LATCH, HIGH);
  }
  //delay(30);
}

void ISR_nRFIRQ() {  //Keep ISR bodies as short as possible
  triggered = true;
}

//for Debugging purposes if necessary...
void displayAll() {
  for (uint8_t d = 0; d < numSegments; d++) {
    digitalWrite(LATCH, LOW);
    shiftOut(DATA, CLOCK, LSBFIRST, Segments[d]);
    digitalWrite(LATCH, HIGH);
    Serial.println(d);
    delay(1000);
  }
}
