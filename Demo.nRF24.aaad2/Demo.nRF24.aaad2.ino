#include <RF24.h>
#include <RF24_config.h>
#include <nRF24L01.h>
#include <printf.h>
#include <Wire.h>

#ifdef ARDUINO_SAMD_VARIANT_COMPLIANCE
#define SERIAL SerialUSB
#else
#define SERIAL Serial
#endif

unsigned char low_data[8] = { 0 };
unsigned char high_data[12] = { 0 };

#define NO_TOUCH 0xFE
#define THRESHOLD 100
#define ATTINY1_HIGH_ADDR 0x78
#define ATTINY2_LOW_ADDR 0x77

#include "Stappenmotor.h"
#include <Arduino.h>
#include <AccelStepper.h>
#include <Servo.h>
#include "ServoArm.h"
#include "Temperatuursensor.h"

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
// WaterLevelSensor waterLevelSensor;

void setup() {
  Serial.begin(9600);
  Serial.println("nRF24 Application ARO" + String(AAAD_ARO) + ", Module" + String(AAAD_MODULE) + " Started!\n");


  pinMode(VLOEISTOFPOMPPIN, INPUT);
  digitalWrite(VLOEISTOFPOMPPIN, LOW);
  Wire.begin();
  // waterLevelSensor.Attach();

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

    Serial.print("Hoi \n");
    Serial.println(rxData[0]);

    switch (rxData[0]) {
      case 0x01:
        Serial.print("Ontvangen getal: 1 - uitklappen \n");
        stappenmotor.KlapUit();
        servo.KlapUit();
        break;
      case 0x02:
        Serial.print("Ontvangen getal: 2 - inklappen \n");
        servo.KlapIn();
        // stappenmotor.KlapIn();
        stappenmotor.SetZeroPosition(BUTTONPIN);
        break;
      case 0x03:
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

  // Reservoir vullen
  while (GetWaterLevelPercentage() < 40) {
    // Vloeistof oppompen
    // Set vloeistofpomp HIGH
    digitalWrite(VLOEISTOFPOMPPIN, HIGH);
    delay(500);
  }
  // Set vloeistofpomp LOW
  digitalWrite(VLOEISTOFPOMPPIN, LOW);

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

  // radio.stopListening();  // First, stop listening so we can talk.
  // Serial.println(F("Now Sending"));

  // Transmit data to radio
  radio.write(&txData, sizeof(txData));

  radio.startListening();  // Now, continue listening
  Serial.println(F("Now Listing"));

  /****************** Receive Mode ***************************/
}

void GetHigh12SectionValue(void) {
  memset(high_data, 0, sizeof(high_data));
  Wire.requestFrom(ATTINY1_HIGH_ADDR, 12);
  while (12 != Wire.available())
    ;

  for (int i = 0; i < 12; i++) {
    high_data[i] = Wire.read();
  }
  delay(10);
}

void GetLow8SectionValue(void) {
  memset(low_data, 0, sizeof(low_data));
  Wire.requestFrom(ATTINY2_LOW_ADDR, 8);
  while (8 != Wire.available())
    ;

  for (int i = 0; i < 8; i++) {
    low_data[i] = Wire.read();  // receive a byte as character
  }
  delay(10);
}

int GetWaterLevelPercentage() {
  int sensorvalue_min = 250;
  int sensorvalue_max = 255;
  int low_count = 0;
  int high_count = 0;

  // while (1) {
  uint32_t touch_val = 0;
  uint8_t trig_section = 0;
  low_count = 0;
  high_count = 0;
  GetLow8SectionValue();
  GetHigh12SectionValue();

  Serial.println("low 8 sections value = ");
  for (int i = 0; i < 8; i++) {
    Serial.print(low_data[i]);
    Serial.print(".");
    if (low_data[i] >= sensorvalue_min && low_data[i] <= sensorvalue_max) {
      low_count++;
    }
    if (low_count == 8) {
      Serial.print("      ");
      Serial.print("PASS");
    }
  }
  Serial.println("  ");
  Serial.println("  ");
  Serial.println("high 12 sections value = ");
  for (int i = 0; i < 12; i++) {
    Serial.print(high_data[i]);
    Serial.print(".");

    if (high_data[i] >= sensorvalue_min && high_data[i] <= sensorvalue_max) {
      high_count++;
    }
    if (high_count == 12) {
      Serial.print("      ");
      Serial.print("PASS");
    }
  }

  Serial.println("  ");
  Serial.println("  ");

  for (int i = 0; i < 8; i++) {
    if (low_data[i] > THRESHOLD) {
      touch_val |= 1 << i;
    }
  }
  for (int i = 0; i < 12; i++) {
    if (high_data[i] > THRESHOLD) {
      touch_val |= (uint32_t)1 << (8 + i);
    }
  }

  while (touch_val & 0x01) {
    trig_section++;
    touch_val >>= 1;
  }

  int percentage = trig_section * 5;

  SERIAL.print("water level = ");
  SERIAL.print(trig_section * 5); // percentage
  SERIAL.println("% ");
  SERIAL.println(" ");
  SERIAL.println("*********************************************************");

  return percentage;
  // delay(1000);
  // }
}