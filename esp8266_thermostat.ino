#include <OneWire.h>
#include <DallasTemperature.h>

#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <time.h>

#include "temperature_controller.h"
#include "secrets.h"

constexpr int ONE_WIRE_BUS = 2;
constexpr int RELAY_PIN = 0;

constexpr char ssid[] = "fastnet";
constexpr char pass[] = "3Datomic";

float setTemp = 22;
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

BearSSL::X509List caCert(CA_CERT);
BearSSL::X509List clientCert(DEVICE_CERT);
BearSSL::PrivateKey clientKey(DEVICE_KEY);

WiFiClientSecure secureClient;
PubSubClient mqttClient(secureClient);


void messageHandler(char* topic, unsigned char* payload, unsigned int length) {
    Serial.print("Message received: ");
    Serial.print(topic);
    Serial.write(payload, length);
}

time_t now = 0;
const time_t nowish = 1510592825;

void NTPConnect(void)
{
    Serial.print("Setting time using NTP");
    configTime(TIME_ZONE * 3600, 0 * 3600, "pool.ntp.org", "time.nist.gov");
    now = time(nullptr);
    while (now < nowish) {
        delay(500); 
        Serial.print(".");
        now = time(nullptr);
    }
    Serial.println("done!");

    struct tm timeinfo;
    gmtime_r(&now, &timeinfo);
    Serial.print("Current time: ");
    Serial.print(asctime(&timeinfo));
}

void setup_wifi() {
    delay(10);

    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, pass);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println();
    Serial.println("WiFi connected");
    Serial.println("IP address");
    Serial.println(WiFi.localIP());

    delay(1000);

    NTPConnect();

    Serial.println("Setting CA");
    secureClient.setTrustAnchors(&caCert);
    Serial.println("Setting client cert");
    secureClient.setClientRSACert(&clientCert, &clientKey);
    Serial.println("Connecting to host...");
    secureClient.connect(MQTT_HOST, 8883);
    Serial.println("Connecting to broker...");

    mqttClient.setCallback(messageHandler);
    
    Serial.println("Connecting to broker...");
    while (!mqttClient.connect(THING_NAME)) {
        Serial.print(".");
        delay(100);
    }
    Serial.println("Connected to brocker.");
    mqttClient.subscribe("topic_1");
}

void setup() {
    // put your setup code here, to run once:
    Serial.begin(115200);

    setup_wifi();
    sensors.begin();
    int numDevices = sensors.getDeviceCount();
    
    pinMode(RELAY_PIN, OUTPUT);
}

TemperatureController controller(20, 20, SystemClock());

void readTemp() {
    sensors.requestTemperatures();
    float floorTemp = sensors.getTempCByIndex(0);
    float airTemp = sensors.getTempCByIndex(1);

    const auto state = controller.feed(floorTemp, airTemp) ? HIGH : LOW;

    digitalWrite(RELAY_PIN, state);
    
    StaticJsonDocument<200> doc;
    doc["florTemp"] = floorTemp;
    doc["airTemp"] = airTemp;
    char serializedJson[200];
    serializeJson(doc, serializedJson);
    Serial.println(serializedJson);
    mqttClient.publish("topic_2", serializedJson);
}

void loop() {
    // put your main code here, to run repeatedly:
    readTemp();

    while (!mqttClient.connected()) {
        Serial.print("Lost connection to broker, reconnecting");
        if (mqttClient.connect(THING_NAME)) {
            mqttClient.subscribe("topic_1");
        }
    }

    mqttClient.loop();
    delay(1000);
}
