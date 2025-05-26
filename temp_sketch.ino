#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <time.h>
#include <ArduinoOTA.h>

// ==== CONFIGURATION ====
// Wi-Fi
const char* ssid = "<your_ssid>"; // Replace with your Wi-Fi SSID
const char* password = "<your_password>"; // Replace with your Wi-Fi password

// MQTT
const char* mqtt_server = "<mqtt_server>"; // Replace with your MQTT server address
const int mqtt_port = 1883;
const char* mqtt_topic = "homelab/esp32/metrics";

// DHT Sensor
#define DHTPIN 4
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// Location of this device
const char* location = "nicks_office";  // Change this per ESP32

// NTP
const char* ntp_server = "pool.ntp.org";
const long gmt_offset_sec = 0;
const int daylight_offset_sec = 0;

// MQTT client
WiFiClient espClient;
PubSubClient client(espClient);

// Timing
unsigned long lastMeasurementTime = 0;
const unsigned long measurementInterval = 300000; // 5 minutes in ms

void setup_wifi() {
  Serial.print("Connecting to Wi-Fi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  int retries = 0;
  while (WiFi.status() != WL_CONNECTED && retries < 20) {
    delay(500);
    Serial.print(".");
    retries++;
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWi-Fi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nWi-Fi connection failed");
  }
}

void setup_time() {
  configTime(gmt_offset_sec, daylight_offset_sec, ntp_server);
  Serial.println("Waiting for NTP time sync...");
  time_t now = time(nullptr);
  int retries = 0;
  while (now < 1700000000 && retries < 20) {
    delay(500);
    Serial.print(".");
    now = time(nullptr);
  }
  Serial.println("\nNTP time synchronized.");
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Connecting to MQTT...");
    if (client.connect("ESP32Client")) {
      Serial.println("connected");
    } else {
      Serial.print(" failed, rc=");
      Serial.print(client.state());
      delay(2000);
    }
  }
}

void setup_ota() {
  ArduinoOTA.setHostname("esp32-nicks-office");

  ArduinoOTA.onStart([]() {
    Serial.println("OTA Update Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nOTA Update Complete");
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("OTA Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });

  ArduinoOTA.begin();
  Serial.println("OTA Ready");
}

void setup() {
  Serial.begin(115200);
  delay(2000);
  dht.begin();
  setup_wifi();
  setup_time();
  setup_ota();
  client.setServer(mqtt_server, mqtt_port);
}

void loop() {
  ArduinoOTA.handle();

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long nowMs = millis();
  if (nowMs - lastMeasurementTime >= measurementInterval || lastMeasurementTime == 0) {
    lastMeasurementTime = nowMs;

    float tempF = dht.readTemperature(true);
    float hum = dht.readHumidity();
    time_t now = time(nullptr);

    if (!isnan(tempF) && !isnan(hum) && now > 1700000000) {
      String payload = "{\"device\":\"" + WiFi.macAddress() +
                       "\",\"location\":\"" + location +
                       "\",\"temperature\":" + String(tempF, 2) +
                       ",\"humidity\":" + String(hum, 2) +
                       ",\"timestamp\":" + String(now) + "}";

      client.publish(mqtt_topic, payload.c_str());
      Serial.println(payload);
    } else {
      Serial.println("❌ Sensor read failed or time not ready");
    }
  }
}
