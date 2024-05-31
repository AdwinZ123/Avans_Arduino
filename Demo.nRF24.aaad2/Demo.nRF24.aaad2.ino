#include <RF24.h>
#include <RF24_config.h>
#include <nRF24L01.h>
#include <printf.h>

#include "Led.h"
#include "Stappenmotor.h"
#include <Arduino.h>
#include <AccelStepper.h>

#include <SPI.h>
#include <nRF24L01.h>  // to handle this particular modem driver
#include "RF24.h"      // the library which helps us to control the radio modem

#define LEDPIN 3   // Ditital pin connected to the LED.
#define LM35PIN A0 /* LM35 O/P pin */

#define STEPPIN 5
#define DIRPIN 2
#define ENPIN 8
#define MOTORINTERFACETYPE 1

// Initialise Sensors

// Initialise Actuators
Led led;
int ledState = LOW;  // ledState used to set the LED

#define RF24_PAYLOAD_SIZE 32
#define AAAD_ARO 3
#define AAAD_MODULE 3


/* Hardware configuration: Set up nRF24L01 radio on SPI bus plus pins 9 & 10, which are CE & CSN pins  */
RF24 radio(9, 10);
const uint8_t rf24_channel[] = { 1, 26, 51, 76, 101 };                                                            // Radio channels set depending on satellite number
const uint64_t addresses[] = { 0x4141414430LL, 0x4141414431LL, 0x4141414432LL, 0x4141414433LL, 0x4141414434LL };  //with radioNumber set to zero, the tx pipe will be 'AAAD0', which is basically HEX'4141414430', which is remote DESTINATION address for our transmitted data. The rx pipe code is the local receive address, which is what the remote device needs to set for the remote devices 'tx' pipe code.
uint8_t txData[RF24_PAYLOAD_SIZE];
uint8_t rxData[RF24_PAYLOAD_SIZE]; 

// Timing configuration
unsigned long previousMillis = 0;  // will store last time LED was updated
unsigned long currentMillis;
unsigned long sampleTime = 15000;  // milliseconds of on-time

unsigned long previousStepperMillis = 0;
unsigned long stepperTime = 200;  // microseconds of on-time

// int to hex converter
void printHex2(unsigned v) {
  Serial.print("0123456789ABCDEF"[v >> 4]);
  Serial.print("0123456789ABCDEF"[v & 0xF]);
}

void convertTemperatureToByteArray(double temperature, byte* buffer) {
  int value = int((temperature + 20.0) * 10);
  buffer[0] = value >> 8;
  buffer[1] = value & 0xFF;

  printHex2(buffer[0]);
  Serial.print(" ");
  printHex2(buffer[1]);
}

Stappenmotor stappenmotor(MOTORINTERFACETYPE, STEPPIN, DIRPIN, ENPIN);

void setup() {
  Serial.begin(9600);
  Serial.println("nRF24 Application ARO" + String(AAAD_ARO) + ", Module" + String(AAAD_MODULE) + " Started!\n");

  // Activate Radio
  radio.begin();                  // Ativate the modem
  radio.setPALevel(RF24_PA_MIN);  // Set the PA Level low to prevent power supply related issues
                                  // since this is a getting_started sketch, and the likelihood of close proximity of the devices. RF24_PA_MAX is default.
  radio.setDataRate(RF24_1MBPS);  // choosing 1 Mega bit per second radio frequency data rate
                                  // radio frequency data rate choices are:  //RF24_250KBPS    //RF24_2MBPS  //RF24_1MBPS
  radio.setChannel(rf24_channel[AAAD_ARO]);
  radio.setPayloadSize(RF24_PAYLOAD_SIZE);
  radio.openWritingPipe(addresses[AAAD_MODULE]);
  radio.openReadingPipe(1, addresses[AAAD_MODULE]);

  // Start the radio listening for data
  radio.startListening();
}

void loop() {

  // check to see if it's time to change the state of the LED
  currentMillis = millis();

  if (currentMillis - previousMillis >= sampleTime) {

    int temp_val = analogRead(LM35PIN) * 4.88 + 200; /* Read Temperature */
    Serial.print("Temperature = " + String(temp_val) + " Degree Celsius\n");

    uint8_t cursor = 0;
    txData[cursor++] = temp_val >> 8;
    txData[cursor++] = temp_val;
    while (cursor < RF24_PAYLOAD_SIZE) {
      txData[cursor++] = 0;
    }

    /****************** Transmit Mode ***************************/

    //  Print transmit data in Hex format
    Serial.print("txData: ");
    for (size_t i = 0; i < cursor; ++i) {
      if (i != 0) Serial.print(" ");
      printHex2(txData[i]);
    }
    Serial.println();

    radio.stopListening();  // First, stop listening so we can talk.
    // Serial.println(F("Now Sending"));

    // Transmit data to radio
    radio.write(&txData, sizeof(txData));

    radio.startListening();  // Now, continue listening
    // Serial.println(F("Now Listing"));

    previousMillis = currentMillis;
  }

  /****************** Receive Mode ***************************/

  if (radio.available()) {  //'available' means whether valid bytes have been received and are waiting to be read from the receive buffer
    Serial.print("Available \n");
    // Receive data from radio

    radio.read(&rxData, sizeof(rxData));
    // while (radio.available()) {             // While there is data ready
    //   radio.read(&rxData, sizeof(rxData));  // Get the payload
    //   Serial.print("Reading \n");
    // }
    // Print received data in Hex format
    Serial.print("rxData: ");
    for (size_t i = 0; i < RF24_PAYLOAD_SIZE; ++i) {
      if (i != 0) Serial.print(" ");
      printHex2(rxData[i]);
    }
    Serial.println();

    switch (rxData[0]) {
      case 0x01:
        Serial.print("Ontvangen getal: 1 - uitklappen \n");
        stappenmotor.KlapUit();
        break;
      case 0x02:
        Serial.print("Ontvangen getal: 2 - inklappen \n");
        stappenmotor.KlapIn();
        break;
      case 0x03:
        Serial.print("Ontvangen getal: 3 - start \n");
        break;
      case 0x04:
        Serial.print("Ontvangen getal: 4 - omhoog \n");
        break;
      case 0x05:
        Serial.print("Ontvangen getal: 5 - omlaag \n");
        break;
      case 0x06:
        Serial.print("Ontvangen getal: 6 - links \n");
        stappenmotor.NaarLinks();
        break;
      case 0x07:
        Serial.print("Ontvangen getal: 7 - rechts \n");
        stappenmotor.NaarRechts();
        break;
      default:
        Serial.print("Ongeldig getal ontvangen: ");
        printHex2(rxData[0]);
        Serial.print(" \n");
        break;
    }
  }

}  // Loop