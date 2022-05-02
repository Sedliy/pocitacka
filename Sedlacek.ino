// http://haneefputtur.com/7-segment-4-digit-led-display-sma420564-using-arduino.html

#include "SevSeg.h"
#include "WiFiEsp.h"
#include "ThingSpeak.h"

#ifndef HAVE_HWSerial3
#define ESP_BAUDRATE  19200
#else
#define ESP_BAUDRATE  115200
#endif

byte numDigits = 4; 
byte digitPins[] = {23, 29, 31, 32};
byte segmentPins[] = {25, 33, 28, 24, 22, 27, 30, 26};
int senzory[] = {52, 50, 48, 49, 51, 53};

int prepinac = 8;
int reset = 9;

int hodnoty[] = {1, 2, 5, 10, 20, 50, 0};
int pocet[] = {0, 0, 0, 0, 0, 0, 0};
int posledniStav[6];

int resetStav, posledniResetStav, stav = 0;
double cas;

char ssid[] = "KTJ CR s.r.o. 2.4G";
char pass[] = "8211182309";
int keyIndex = 0;

unsigned long myChannelNumber = 1721919;
const char * myWriteAPIKey = "SACC3UIB30Q4PXSS";

WiFiEspClient  client;
SevSeg displej;

void setup() {
  Serial.begin(115200);
  for (int i = 0; i < sizeof(senzory) / 2; i++) {
    pinMode(senzory[i], INPUT);
  }
  displej.begin(COMMON_ANODE, numDigits, digitPins, segmentPins);
  displej.setBrightness(90);
  displej.blank();
  
  setEspBaudRate(ESP_BAUDRATE);
  WiFi.init(&Serial3);
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // don't continue
    while (true);
  }
  Serial.println("found it!");
    
  ThingSpeak.begin(client);
}

void loop() {
  if (digitalRead(prepinac)) {
    if (pocet[6] == 0) {
      displej.setNumber(0);
    }
    for (int i = 0; i < sizeof(senzory) / 2; i++) {
      stav = digitalRead(senzory[i]);
      if (stav && stav != posledniStav[i]) {
        pocet[6] += hodnoty[i];
        pocet[i]++;
        displej.setNumber(pocet[6]);
      }
      posledniStav[i] = stav;
    }
    resetStav = digitalRead(reset);
    if (resetStav && resetStav != posledniResetStav) {
      hodnoty[6] = pocet[6];
      odeslat();
      for (int i = 0; i < sizeof(pocet) / 2; i++) {
        pocet[i] = 0;
      }
      displej.setNumber(pocet[6]);
    }
    posledniResetStav = resetStav;
    displej.refreshDisplay();
  } else {
    for (int i = 0; i < sizeof(pocet) / 2; i++) {
      pocet[i] = 0;
    }
    displej.blank();
  }
}

void odeslat() {
  for (int i = 1; i <= sizeof(pocet) / 2; i++) {
    displej.blank();
    if(WiFi.status() != WL_CONNECTED){
      Serial.print("Attempting to connect to SSID: ");
      Serial.println(ssid);
      while(WiFi.status() != WL_CONNECTED){
        WiFi.begin(ssid, pass);
        Serial.print(".");
        cas = millis();
        while (millis() - cas < 5000) {
        }     
      } 
      Serial.println("\nConnected.");
    }
    displej.blank();
    int x = ThingSpeak.writeField(myChannelNumber, i, pocet[i - 1], myWriteAPIKey);
    if(x == 200){
      Serial.println("Channel update successful.");
    }
    else{
      Serial.println("Problem updating channel. HTTP error code " + String(x));
    }
    displej.setNumber(hodnoty[i - 1]);
    cas = millis();
    while (millis() - cas < 15000) {
      displej.refreshDisplay();
    }
  }
}

void setEspBaudRate(unsigned long baudrate){
  long rates[6] = {115200,74880,57600,38400,19200,9600};

  Serial.print("Setting ESP8266 baudrate to ");
  Serial.print(baudrate);
  Serial.println("...");

  for(int i = 0; i < 6; i++){
    Serial3.begin(rates[i]);
    delay(100);
    Serial3.print("AT+UART_DEF=");
    Serial3.print(baudrate);
    Serial3.print(",8,1,0,0\r\n");
    delay(100);  
  }
    
  Serial3.begin(baudrate);
}
