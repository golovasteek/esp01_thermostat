#include <OneWire.h>
#include <DallasTemperature.h>
#include <BlynkSimpleEsp8266.h>


constexpr int ONE_WIRE_BUS = 2;
constexpr int RELAY_PIN = 0;

constexpr char BLYNK_TOKEN[] = "ZXfo-h4X5kLfvBgBWhkXLle2KARo-dpa";

constexpr char ssid[] = "fastnet";
constexpr char pass[] = "3Datomic";

float setTemp = 22;
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

WidgetLED led(V0);
BlynkTimer timer;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  sensors.begin();
  int numDevices = sensors.getDeviceCount();

  //Serial.print("Num devices:");
  Serial.println(numDevices, DEC);

  pinMode(RELAY_PIN, OUTPUT);
  Blynk.begin(BLYNK_TOKEN, ssid, pass);
  Blynk.syncAll();
  timer.setInterval(1000L, readTemp);
}

void readTemp() {
   sensors.requestTemperatures();
  float temperature0 = sensors.getTempCByIndex(0);
  float temperature1 = sensors.getTempCByIndex(1);
  Serial.print(temperature0);
  Serial.print("\t  ");
  Serial.print(temperature1);
  Serial.println("");
  Blynk.virtualWrite(V5, temperature0);
  Blynk.virtualWrite(V6, temperature1);
  if (temperature0 > setTemp + 0.5) {
    digitalWrite(RELAY_PIN, HIGH);
    led.off();
  }
  if (temperature0 < setTemp - 0.5) {
    digitalWrite(RELAY_PIN, LOW);
    led.on();
  }
}

BLYNK_WRITE(V1) {
  setTemp = param.asFloat();
  Serial.print("Set temp: ");
  Serial.println(setTemp);
  readTemp();
}

void loop() {
  // put your main code here, to run repeatedly:
  Blynk.run();
  timer.run();
}
