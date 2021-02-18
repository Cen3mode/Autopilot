#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoOTA.h>
#include <MPU9250.h>
#include <Adafruit_BMP280.h>
#include <SPI.h>
#include <Wire.h>

const char * ssid = "TS";
const char * passwd = "_MantatzpdP_";

void wifiSetup() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, passwd);
  int retryIntervalMs = 500;
  int timeoutCounter = 25 * (1000 / retryIntervalMs);
  while (WiFi.status() != WL_CONNECTED && timeoutCounter > 0)
  {
    delay(retryIntervalMs);
    if (timeoutCounter == (25 * 2 - 3))
    {
      WiFi.reconnect();
      Serial.println("Reconnecting...");
    }
    timeoutCounter--;
  }
  Serial.print("Connected to ");
  Serial.print(ssid);
  Serial.print(" at ");
  Serial.println(WiFi.localIP());
}

void otaSetup() {
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_FS
      type = "filesystem";
    }

    // NOTE: if updating FS this would be the place to unmount FS using FS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();
}

void otaTask(void * params) {
  for(;;) {
    ArduinoOTA.handle();
    vTaskDelay(1);
  }
}

void setup() {

  Serial.begin(9600);

  wifiSetup();
  otaSetup();

  xTaskCreate(otaTask, "OTA Task", 100000, NULL, 1, NULL);
}

void loop() {
  // put your main code here, to run repeatedly:
}