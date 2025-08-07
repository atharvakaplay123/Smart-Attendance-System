#include <SPI.h>
#include <MFRC522.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <WiFiClientSecureBearSSL.h>
#include "secrets.h"

#define RST_PIN D3
#define SS_PIN D4

#define BUZZER D2
#define LED_R D1
#define LED_G D0

String RFID;
MFRC522 mfrc522(SS_PIN, RST_PIN);  // Instance of the class
MFRC522::MIFARE_Key key;
ESP8266WiFiMulti WiFiMulti;
MFRC522::StatusCode status;

/* Be aware of Sector Trailer Blocks */
int blockNum = 2;

/* Create another array to read data from Block */
/* Legthn of buffer should be 2 Bytes more than the size of Block (16 Bytes) */
byte bufferLen = 18;
byte readBlockData[18];

String data2;
const String data1 = Google_Spreadsheet_link; //replace with your credentials

void setup() {
  Serial.begin(9600);
  delay(100);
  pinMode(BUZZER, OUTPUT);
  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);

  digitalWrite(BUZZER, LOW);

  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP(Wifi_SSID, Wifi_PASS);  //replace with your credentials

  while (WiFiMulti.run() != WL_CONNECTED) {
    digitalWrite(LED_G, LOW);
    digitalWrite(LED_R, HIGH);
    delay(100);
    digitalWrite(LED_R, LOW);
    delay(100);
  }
  digitalWrite(LED_R, LOW);
  digitalWrite(LED_G, HIGH);
  SPI.begin();
}

void loop() {
      digitalWrite(LED_G, HIGH);
  /////////////////////////////////////////////////////////////////////////////////////////////
  //*********************************scaning connectivity status*********************************
  if (WiFiMulti.run() != WL_CONNECTED) {
    while (WiFiMulti.run() != WL_CONNECTED) {
      digitalWrite(LED_G, LOW);
      digitalWrite(LED_R, HIGH);
      delay(100);
      digitalWrite(LED_R, LOW);
      delay(100);
    }
    if (WiFiMulti.run() == WL_CONNECTED) {
      digitalWrite(LED_R, LOW);
      digitalWrite(LED_G, HIGH);
    }
  }
  /////////////////////////////////////////////////////////////////////////////////////////////
  /* Initialize MFRC522 Module */
  mfrc522.PCD_Init();
  /* Look for new cards */
  /* Reset the loop if no new card is present on RC522 Reader */
  if (!mfrc522.PICC_IsNewCardPresent()) {
    return;
  }
  /* Select one of the cards */
  if (!mfrc522.PICC_ReadCardSerial()) {
    return;
  }
  /////////////////////////////////////////////////////////////////////////////////////////////
  //*********************************scaning RFID*********************************

  /* Read data from the same block */
  Serial.println();
  Serial.println(F("Reading last data from RFID..."));
  ReadDataFromBlock(blockNum, readBlockData);
  /* If you want to print the full memory dump, uncomment the next line */
  //mfrc522.PICC_DumpToSerial(&(mfrc522.uid));

  /* Print the data read from block */
  Serial.println();
  Serial.print(F("Last data in RFID:"));
  Serial.print(blockNum);
  Serial.print(F(" --> "));
  RFID.remove(0, RFID.length());
  int j = 0;
  while (int(readBlockData[j]) > 0) {
    RFID.concat(char(readBlockData[j]));
    j++;
  }
  Serial.println(RFID);
  digitalWrite(BUZZER, HIGH);
  delay(200);
  digitalWrite(BUZZER, LOW);
  delay(200);
  digitalWrite(BUZZER, HIGH);
  delay(200);
  digitalWrite(BUZZER, LOW);
  digitalWrite(LED_G, LOW);
  /////////////////////////////////////////////////////////////////////////////////////////////
  //*********************************sending data to spreadsheet*********************************

  if ((WiFiMulti.run() == WL_CONNECTED)) {
    std::unique_ptr<BearSSL::WiFiClientSecure> client(new BearSSL::WiFiClientSecure);

    //client->setFingerprint(fingerprint);
    // Or, if you happy to ignore the SSL certificate, then use the following line instead:
    client->setInsecure();

    data2 = data1 + RFID;
    data2.trim();
    Serial.println(data2);

    HTTPClient https;
    Serial.print(F("[HTTPS] begin...\n"));
    if (https.begin(*client, (String)data2)) {
      //HTTP
      Serial.print(F("[HTTPS] GET...\n"));
      //start connection and send HTTP header
      int httpCode = https.GET();

      // httpCode will be negative on error
      if (httpCode > 0) {
        //HTTP header has been send and Server response header has been handled
        Serial.printf("[HTTPS] GET... code: %d\n", httpCode);
        //file found at server
        digitalWrite(LED_R, LOW);
        digitalWrite(LED_G, HIGH);
        delay(200);
        digitalWrite(LED_G, LOW);
        delay(200);
        digitalWrite(LED_G, HIGH);
        delay(200);
        digitalWrite(LED_G, LOW);
      } else {
        Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
        digitalWrite(LED_G, LOW);
        digitalWrite(LED_R, HIGH);
      }
      https.end();
      delay(500);
    } else {
      Serial.printf("[HTTPS} Unable to connect\n");
      digitalWrite(LED_G, LOW);
      digitalWrite(LED_R, HIGH);
    }
  }
}
