#include “SCD30.h”
#include <Wire.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>

ESP8266WiFiMulti wifiMulti;

const int roomID = 1;//部屋のID番号は1が真壁研究室。それ以外は2桁。10〜


float result[3] = {0};

const char* host = “データを送信するサーバのドメイン名”;
const int ledBlinkTime = 1;
const uint32_t connectTimeoutMs = 10000;

void setup() {
  Wire.begin();
  Serial.begin(115200);

  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(scd30_V, OUTPUT);

  digitalWrite(LED1, LOW);
  digitalWrite(LED2, LOW);
  digitalWrite(scd30_V, HIGH);

  delay(1000);
  Serial.println(“”);
  Serial.println(WiFi.macAddress());
  Serial.setDebugOutput(true);

  //wifi setup
  wifiMulti.addAP(“wi-fiのSSID”, “wi-fiのパスワード”);


  //WIFI_PHY_MODE_11B,  WIFI_PHY_MODE_11G,  WIFI_PHY_MODE_11N
  WiFiPhyMode_t wifiMode = WIFI_PHY_MODE_11N;
  if (WiFi.setPhyMode(wifiMode)) {
    delay(100);
    Serial.print(“Set PHY mode:”);
    //Serial.println(wifiMode);
    switch (wifiMode) {
      case WIFI_PHY_MODE_11B:
        Serial.println(“WIFI_PHY_MODE_11B”);
        break;
      case WIFI_PHY_MODE_11G:
        Serial.println(“WIFI_PHY_MODE_11G”);
        break;
      case WIFI_PHY_MODE_11N:
        Serial.println(“WIFI_PHY_MODE_11N”);
        break;
    }
  } else {
    Serial.println(“ERROR setPhyMode is FALSE”);
  }



  WiFi.disconnect(true);
  WiFi.mode(WIFI_STA);

  int wifiCheckCount = 0;
  //if (wifiMulti.run(connectTimeoutMs) == WL_CONNECTED) {
  if (wifiMulti.run() == WL_CONNECTED) {
    //delay(1000);
    //Serial.print(“.”);
    Serial.println(WiFi.status());

    //接続した後にステータスを出力
    Serial.println(“========wi-fi status printDiag========”);
    WiFi.printDiag(Serial);
    Serial.println(“=======================================”);
  } else {
    Serial.println(“***************************************”);
    digitalWrite(scd30_V, LOW);
    WiFi.disconnect(true);
    WiFi.begin(“0”, “0”);
    ESP.restart();
    delay(10000);
  }

}


void loop() {
  float co2 = 0.0;
  float temp = 0.0;
  float humidity = 0.0;

  int scd30ErrorCount = 0;
  int meashureCount = 0;


  delay(2000);
  scd30.initialize();


  while (meashureCount < 3) {
    delay(1000);
    if (scd30.isAvailable()) {
      delay(2000);
      scd30.getCarbonDioxideConcentration(result);
      if (!isnan(result[0]) && !isnan(result[1]) && !isnan(result[2])) {
        if ((result[0] > 0.0) && ((result[1] != 0.0) && (result[2] != 0.0))) {
          digitalWrite(LED1, HIGH);
          delay(ledBlinkTime);
          digitalWrite(LED1, LOW);

          co2 = co2 + result[0];
          temp = temp + result[1];
          humidity = humidity +  result[2];
          meashureCount++;
          delay(2000);
        }
      }
    } else {// is Available
      Serial.println(“not Available”);
      scd30ErrorCount++;
      if (scd30ErrorCount > 10) {
        digitalWrite(scd30_V, LOW);
        WiFi.disconnect(true);//wi-fiの切断処理
        WiFi.begin(“0”, “0”);
        //1:μ秒での復帰までのタイマー時間設定  2:復帰するきっかけの設定（モード設定）
        ESP.deepSleep(1 * 60 * 1000 * 1000, WAKE_RF_DEFAULT);//1min
        delay(1000);//deepsleepモード移行までのダミー命令
      }
    }
  }//while

  //測定終了
  digitalWrite(scd30_V, LOW);

  co2 = co2 / 3.0;
  temp = temp / 3.0;
  humidity = humidity / 3.0;

  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.println(“connection failed”);
    //wifiがつながらない時はリセット
    digitalWrite(scd30_V, LOW);
    WiFi.disconnect(true);
    WiFi.begin(“0”, “0”);
    ESP.restart();
    delay(10000);
  }

  String co2String = String(co2);
  String tempString = String(temp);
  String humidityString = String(humidity);

  String url = “phpへのパス/upload.php?c=” + co2String + “&t=” + tempString + “&h=” + humidityString + “&id=” + roomID;
  Serial.print(“Requesting URL: “);
  Serial.println(url);

  client.println(String(“GET “) + url + “ HTTP/1.1\r\n” +
                 “Host: “ + host + “\r\n” );

  //レスポンスが帰ってくるのを待つ
  for (int i = 0 ; i < 10 ; i++) {
    delay(1000);
  }

  while (client.available()) {
    String line = client.readStringUntil(‘\r’);
    Serial.print(“line:”);
    Serial.print(line);
    delay(500);
  }

  digitalWrite(scd30_V, LOW);
  //wi-fiの切断処理
  WiFi.disconnect(true);
  WiFi.begin(“0”, “0”);

  //1:μ秒での復帰までのタイマー時間設定  2:復帰するきっかけの設定（モード設定）
  ESP.deepSleep(2 * 60 * 1000 * 1000, WAKE_RF_DEFAULT);//2min = 2min * 60sec * 1000msec * 1000uSec
  delay(1000);//deepsleepモード移行までのダミー命令
}
