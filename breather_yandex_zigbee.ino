#include <Arduino.h>
#include "Zigbee.h"

/**
 * ESP32-C6 Zigbee sketch for a breather device:
 *  - control: On/Off + fan speed (0..100%)
 *  - telemetry: temperature, humidity, filter resource (%)
 *
 * Works as a Zigbee End Device and can be paired with a Zigbee coordinator
 * that is connected to Yandex Smart Home.
 */

// ------------------------------
// Hardware/configuration section
// ------------------------------
constexpr uint8_t PIN_FAN_PWM = 3;
constexpr uint8_t PIN_FAN_ENABLE = 2;
constexpr uint8_t PIN_TEMP_HUM_SENSOR = 4;  // placeholder (I2C/SPI/OneWire/etc)

constexpr uint8_t PWM_CHANNEL = 0;
constexpr uint32_t PWM_FREQ_HZ = 25000;
constexpr uint8_t PWM_RES_BITS = 8;

constexpr uint8_t ZB_ENDPOINT_FAN = 1;
constexpr uint8_t ZB_ENDPOINT_TEMP = 2;
constexpr uint8_t ZB_ENDPOINT_HUM = 3;
constexpr uint8_t ZB_ENDPOINT_FILTER = 4;

constexpr uint32_t TELEMETRY_PERIOD_MS = 30000;

// ------------------------------
// Zigbee endpoints
// ------------------------------
ZigbeeFan zbFan(ZB_ENDPOINT_FAN);
ZigbeeTempSensor zbTemp(ZB_ENDPOINT_TEMP);
ZigbeeHumiditySensor zbHum(ZB_ENDPOINT_HUM);
ZigbeeAnalog zbFilterResource(ZB_ENDPOINT_FILTER);

// ------------------------------
// Runtime state
// ------------------------------
volatile bool g_powerOn = false;
volatile uint8_t g_fanSpeedPercent = 30;  // 0..100

uint32_t g_lastTelemetryMs = 0;

// ------------------------------
// Application-specific telemetry
// Replace these stubs with real sensors/logic from your hardware
// ------------------------------
float readTemperatureC() {
  // TODO: read from your real sensor, for example SHT3x/BME280/DS18B20
  return 23.4f;
}

float readHumidityPercent() {
  // TODO: read from your real sensor
  return 41.5f;
}

float readFilterResourcePercent() {
  // TODO: implement your filter life algorithm/counter
  // 100 = new filter, 0 = must replace
  static float fake = 100.0f;
  fake -= 0.01f;
  if (fake < 0.0f) fake = 0.0f;
  return fake;
}

// ------------------------------
// Fan control
// ------------------------------
void applyFanOutput() {
  const bool enabled = g_powerOn && g_fanSpeedPercent > 0;
  digitalWrite(PIN_FAN_ENABLE, enabled ? HIGH : LOW);

  const uint8_t pwm = enabled ? map(g_fanSpeedPercent, 0, 100, 0, 255) : 0;
  ledcWrite(PWM_CHANNEL, pwm);
}

void onFanStateChanged(bool on) {
  g_powerOn = on;
  applyFanOutput();
}

void onFanSpeedChanged(uint8_t speedPercent) {
  if (speedPercent > 100) speedPercent = 100;
  g_fanSpeedPercent = speedPercent;
  applyFanOutput();
}

// ------------------------------
// Zigbee callbacks
// ------------------------------
void configureCallbacks() {
  // Called when coordinator/Yandex changes ON/OFF state
  zbFan.onLightChange([](bool on) {
    onFanStateChanged(on);
    Serial.printf("[ZB] Power changed: %s\n", on ? "ON" : "OFF");
  });

  // Called when coordinator/Yandex changes fan speed
  zbFan.onFanSpeedChange([](uint8_t speedPercent) {
    onFanSpeedChanged(speedPercent);
    Serial.printf("[ZB] Fan speed: %u%%\n", speedPercent);
  });
}

void publishTelemetry() {
  const float t = readTemperatureC();
  const float h = readHumidityPercent();
  const float filter = readFilterResourcePercent();

  // Push values into Zigbee attributes
  zbTemp.setTemperature(t);
  zbHum.setHumidity(h);

  // Expose filter resource as generic analog value (percent)
  zbFilterResource.setAnalogInput(filter);

  Serial.printf("[TEL] T=%.2fC H=%.2f%% Filter=%.2f%%\n", t, h, filter);
}

// ------------------------------
// Arduino lifecycle
// ------------------------------
void setup() {
  Serial.begin(115200);
  delay(500);

  pinMode(PIN_FAN_ENABLE, OUTPUT);
  digitalWrite(PIN_FAN_ENABLE, LOW);

  ledcSetup(PWM_CHANNEL, PWM_FREQ_HZ, PWM_RES_BITS);
  ledcAttachPin(PIN_FAN_PWM, PWM_CHANNEL);

  // Zigbee endpoint metadata for better display in ecosystems
  zbFan.setManufacturerAndModel("BreatherDIY", "ESP32C6-Breather");
  zbTemp.setManufacturerAndModel("BreatherDIY", "ESP32C6-Breather");
  zbHum.setManufacturerAndModel("BreatherDIY", "ESP32C6-Breather");
  zbFilterResource.setManufacturerAndModel("BreatherDIY", "ESP32C6-Breather");

  // Configure and register endpoints
  configureCallbacks();
  Zigbee.addEndpoint(&zbFan);
  Zigbee.addEndpoint(&zbTemp);
  Zigbee.addEndpoint(&zbHum);
  Zigbee.addEndpoint(&zbFilterResource);

  // Optional: force rejoin/pairing every boot for development
  // Zigbee.factoryReset();

  if (!Zigbee.begin()) {
    Serial.println("[ERR] Zigbee start failed. Rebooting...");
    delay(2000);
    ESP.restart();
  }

  // Initial states for coordinator
  onFanStateChanged(false);
  onFanSpeedChanged(30);
  publishTelemetry();

  Serial.println("[OK] Zigbee started. Put coordinator in pairing mode.");
}

void loop() {
  Zigbee.loop();

  const uint32_t now = millis();
  if (now - g_lastTelemetryMs >= TELEMETRY_PERIOD_MS) {
    g_lastTelemetryMs = now;
    publishTelemetry();
  }

  delay(10);
}
