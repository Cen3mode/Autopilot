#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoOTA.h>
#include <MPU9250.h>
#include <Adafruit_BMP280.h>
#include <SPI.h>
#include <TelnetStream.h>
#include <TinyGPS++.h>
#include <Wire.h>

#include "structs.h"

const char * ssid = "TS";
const char * passwd = "_MantatzpdP_";

Adafruit_BMP280 bmp;
MPU9250 mpu(Wire, 0x68);
TinyGPSPlus gps;

Telemetry telemetry;

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

void imuDataCollectionTask(void * params) {
  for(;;) {

    mpu.readSensor();

    telemetry.imu.temp = (bmp.readTemperature()+mpu.getTemperature_C())/2.0f;
    telemetry.imu.pres = bmp.readPressure();

    telemetry.imu.ax = mpu.getAccelX_mss();
    telemetry.imu.ay = mpu.getAccelY_mss();
    telemetry.imu.az = mpu.getAccelZ_mss();

    telemetry.imu.gx = mpu.getGyroX_rads();
    telemetry.imu.gy = mpu.getGyroY_rads();
    telemetry.imu.gz = mpu.getGyroZ_rads();

    telemetry.imu.mx = mpu.getMagX_uT();
    telemetry.imu.my = mpu.getMagY_uT();
    telemetry.imu.mz = mpu.getMagZ_uT();

    telemetry.imu.alt = bmp.readAltitude();

    vTaskDelay(1);
  }
}

void gpsDataCollectionTask(void * params) {
  for(;;) {
    while (Serial.available() > 0) {
      if(gps.encode(Serial1.read())) {
        telemetry.gps.lat = gps.location.lat();
        telemetry.gps.lon = gps.location.lng();
        telemetry.gps.speed = gps.speed.kmph();
        telemetry.gps.alt = gps.altitude.meters();
        telemetry.gps.course = gps.course.deg();
      }
    }
    
    vTaskDelay(1);
  }
}

void setup() {

  Serial.begin(9600);
  Serial1.begin(9600);
  Wire.begin();

  wifiSetup();
  otaSetup();

  TelnetStream.begin();

  bmp.begin(0x77);
  mpu.begin();

  xTaskCreate(otaTask, "OTA Task", 100000, NULL, 1, NULL);
  xTaskCreate(imuDataCollectionTask, "Imu data collection task", 5000, NULL, 1, NULL);
  xTaskCreate(gpsDataCollectionTask, "Gps data collection task", 5000, NULL, 1, NULL);

}

void loop() {
  TelnetStream.println(telemetry.imu.ax);
  TelnetStream.println(telemetry.gps.lat);
  delay(1000);
}