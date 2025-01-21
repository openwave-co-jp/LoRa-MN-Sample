/*
  Lora Send And Receive
  This sketch demonstrates how to send and receive data with the MKR WAN 1300/1310 LoRa module.
  This example code is in the public domain.
*/

#include <MKRWAN.h>
#include <DHT.h>
#include "arduino_secrets.h" // AppEUI, AppKeyを定義

#define DHTPIN 19
#define DHTTYPE DHT11

LoRaModem modem;
DHT dht(DHTPIN, DHTTYPE);

// 温度・湿度データを管理
// ※LoRaの最小ペイロードサイズは4バイトです。
static uint8_t mydata[4];

// 送信間隔
int snd_interval = 10000;


// Please enter your sensitive data in the Secret tab or arduino_secrets.h
String appEui = SECRET_APP_EUI;
String appKey = SECRET_APP_KEY;

void setup() {
  // put your setup code here, to run once:
  //Serial.begin(115200);
  while (!Serial);
  // change this to your regional band (eg. US915, AS923, ...)
  if (!modem.begin(AS923)) {
    Serial.println("Failed to start module");
    while (1) {}
  };
  Serial.print("Your module version is: ");
  Serial.println(modem.version());
  Serial.print("Your device EUI is: ");
  Serial.println(modem.deviceEUI());

  int connected = modem.joinOTAA(appEui, appKey);
  if (!connected) {
    Serial.println("Something went wrong; are you indoor? Move near a window and retry");
    while (1) {}
  }

  modem.minPollInterval(60);

  dht.begin();
    
}

void loop() {

  memset(mydata, 0x00, 4);
  
  int err;

  // 温度、湿度データ取得
  uint8_t h = dht.readHumidity();
  uint8_t t = dht.readTemperature();
  //uint8_t h = 20;
  //uint8_t t = 30;

  mydata[0] = h;
  mydata[1] = t;
  
  // LoRa 送信
  modem.beginPacket();
  // LoRaWANでは、最小ペイロードは、4byte送信だが、
  // modem.write(mydata, 4)のように送信バイト数をしていしないと、
  // 0でのパディング処理が入り、おかしなデータになります。
  modem.write(mydata, 4);
  err = modem.endPacket(true);

  // LoRa送信エラーハンドリング
  if (err > 0) {
    Serial.println("Message sent correctly!");
  }
  
  delay(1000);

  // Class A ダウンリンク受信
  if (!modem.available()) {
    Serial.println("No downlink message received at this time.");
  }
  char rcv[64];
  int i = 0;
  while (modem.available()) {
    rcv[i++] = (char)modem.read();
  }
  Serial.print("Received: ");
  for (unsigned int j = 0; j < i; j++) {
    Serial.print(rcv[j] >> 4, HEX);
    Serial.print(rcv[j] & 0xF, HEX);
    Serial.print(" ");
  }
  Serial.println();

  delay(snd_interval);
  
}
