#include <RF24.h>
#include <RF24_config.h>
#include <nRF24L01.h>
#include <printf.h>

#include "Stappenmotor.h"
#include <Arduino.h>
#include <AccelStepper.h>
#include <Servo.h>
#include "ServoArm.h"
#include "Temperatuursensor.h"
#include "WaterLevelSensor.h"

#include <SPI.h>
#include <nRF24L01.h>  // to handle this particular modem driver
#include "RF24.h"      // the library which helps us to control the radio modem

#define LM35PIN A0 /* LM35 O/P pin - Temperatuursensor */

#define STEPPIN 5
#define DIRPIN 2
#define ENPIN 8
#define MOTORINTERFACETYPE 1

#define BUTTONPIN 3
#define VLOEISTOFPOMPPIN 4
#define SERVOPIN 7

#define RF24_PAYLOAD_SIZE 32
#define AAAD_ARO 3
#define AAAD_MODULE 3


/* Hardware configuration: Set up nRF24L01 radio on SPI bus plus pins 9 & 10, which are CE & CSN pins  */
RF24 radio(9, 10);
const uint8_t rf24_channel[] = { 1, 26, 51, 76, 101 };                                                            // Radio channels set depending on satellite number
const uint64_t addresses[] = { 0x4141414430LL, 0x4141414431LL, 0x4141414432LL, 0x4141414433LL, 0x4141414434LL };  //with radioNumber set to zero, the tx pipe will be 'AAAD0', which is basically HEX'4141414430', which is remote DESTINATION address for our transmitted data. The rx pipe code is the local receive address, which is what the remote device needs to set for the remote devices 'tx' pipe code.
uint8_t txData[RF24_PAYLOAD_SIZE];
uint8_t rxData[RF24_PAYLOAD_SIZE];

// int to hex converter
void printHex2(unsigned v) {
  Serial.print("0123456789ABCDEF"[v >> 4]);
  Serial.print("0123456789ABCDEF"[v & 0xF]);
}

Stappenmotor stappenmotor(MOTORINTERFACETYPE, STEPPIN, DIRPIN, ENPIN);
ServoArm servo(1);
Temperatuursensor temperatuursensor(LM35PIN);
WaterLevelSensor waterLevelSensor;

void setup() {
  Serial.begin(9600);
  Serial.println("nRF24 Application ARO" + String(AAAD_ARO) + ", Module" + String(AAAD_MODULE) + " Started!\n");

  pinMode(VLOEISTOFPOMPPIN, INPUT);
  digitalWrite(VLOEISTOFPOMPPIN, LOW);

  waterLevelSensor.Attach();

  servo.Attach(SERVOPIN);

  pinMode(BUTTONPIN, OUTPUT);
  stappenmotor.SetZeroPosition(BUTTONPIN);

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

  servo.Update();

  if (radio.available()) {  //'available' means whether valid bytes have been received and are waiting to be read from the receive buffer
    Serial.print("Available \n");

    // Receive data from radio
    radio.read(&rxData, sizeof(rxData));

    // Print received data in Hex format
    Serial.print("rxData: ");
    for (size_t i = 0; i < RF24_PAYLOAD_SIZE; ++i) {
      if (i != 0) Serial.print(" ");
      printHex2(rxData[i]);
    }
    Serial.println();

    switch (rxData[0]) {
      case 1:
        Serial.print("Ontvangen getal: 1 - uitklappen \n");
        stappenmotor.KlapUit();
        servo.KlapUit();
        break;
      case 2:
        Serial.print("Ontvangen getal: 2 - inklappen \n");
        servo.KlapIn();
        stappenmotor.SetZeroPosition(BUTTONPIN);
        break;
      case 3:
        Serial.print("Ontvangen getal: 3 - start \n");
        VoerMetingUit();
        break;
      case 4:
        Serial.print("Ontvangen getal: 4 - omhoog \n");
        servo.Omhoog();
        break;
      case 5:
        Serial.print("Ontvangen getal: 5 - omlaag \n");
        servo.Omlaag();
        break;
      case 6:
        Serial.print("Ontvangen getal: 6 - links \n");
        stappenmotor.NaarLinks();
        break;
      case 7:
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

void VoerMetingUit() {
  /****************** Transmit Mode ***************************/
  // Berichten ontvangen uitzetten voor later kunnen versturen, dan kan de arm ook niet bewegen tijdens de meting
  radio.stopListening();  // First, stop listening so we can talk.
  Serial.println(F("Now Sending"));

  //Temperatuur opmeten en versturen
  int gemiddeldeTemperatuur = ((temperatuursensor.MeetGemiddeldeTemperatuur() + 20) * 10); /* Read Temperature */
  Serial.print("Temperature = " + String(gemiddeldeTemperatuur) + " Degree Celsius\n");

  uint8_t cursor = 0;
  txData[cursor++] = gemiddeldeTemperatuur >> 8;
  txData[cursor++] = gemiddeldeTemperatuur;
  while (cursor < RF24_PAYLOAD_SIZE) {
    txData[cursor++] = 0;
  }

  //  Print transmit data in Hex format
  Serial.print("txData: ");
  for (size_t i = 0; i < cursor; ++i) {
    if (i != 0) Serial.print(" ");
    printHex2(txData[i]);
  }
  Serial.println();

  // Transmit data to radio
  radio.write(&txData, sizeof(txData));

  radio.startListening();  // Now, continue listening
  Serial.println(F("Now Listing"));

  /****************** Receive Mode ***************************/

  // Reservoir vullen
  int percentage = waterLevelSensor.GetWaterLevelPercentage();
  int vorigePercentage = percentage;
  int tijd = millis();
  bool isLow = true;

  while (percentage < 40) {
    // Vloeistof oppompen
    if (isLow) {
      digitalWrite(VLOEISTOFPOMPPIN, HIGH);
      isLow = false;
    }

    // Fail safe voor als de water level sensor dezelfde waarde blijft uitmeten, ook als er meer water inzit.
    if ((millis() - tijd) > 5000 && percentage == vorigePercentage) {
      Serial.println("Fail safe actief");
      break;
    }

    percentage = waterLevelSensor.GetWaterLevelPercentage();
    if (percentage != vorigePercentage) {
      vorigePercentage = percentage;
      tijd = millis();
    }
  }

  // Set vloeistofpomp LOW
  digitalWrite(VLOEISTOFPOMPPIN, LOW);
}
