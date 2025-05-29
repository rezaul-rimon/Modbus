#define TINY_GSM_MODEM_SIM7600
#define TINY_GSM_USE_GPRS true
#define TINY_GSM_USE_WIFI false

#include <Arduino.h>
#include <ModbusMaster.h>
#include <TinyGsmClient.h>
#include <PubSubClient.h>
#include <HardwareSerial.h>

// GSM settings
#define SerialAT Serial1
#define MODEM_TX 17
#define MODEM_RX 16
#define MODEM_PWR 15
#define SIM_BAUD 115200
#define MQTT_PORT 1883
#define MQTT_HB "DMA/EM/HB"
#define MQTT_PUB "DMA/EM/PUB"
#define MQTT_SUB "DMA/EM/SUB"

const char apn[] = "blweb";
const char user[] = "";
const char pass[] = "";
const char* broker = "broker2.dma-bd.com";
const char* mqttUser = "broker2";
const char* mqttPass = "Secret!@#$1234";

#define DEVICE_ID "1191012505290001"

char mqttSubTopic[64];  // Global subscribe topic buffer

TinyGsm modem(SerialAT);
TinyGsmClient gsmClient(modem);
PubSubClient mqtt(gsmClient);

// RS485 Serial2 Pins
#define RS485_RX 27
#define RS485_TX 14

unsigned long lastPublishTime = 0;
const unsigned long publishInterval = 30000; // 30 seconds

// Modbus register addresses
#define taeHigh_reg_addr     0x30
#define taeLow_reg_addr      0x31
#define activePower_reg_addr 0x1A
#define pAvolt_reg_addr      0x14
#define pBvolt_reg_addr      0x15
#define pCvolt_reg_addr      0x16
#define lABvolt_reg_addr     0x17
#define lBCvolt_reg_addr     0x18
#define lCAvolt_reg_addr     0x19
#define pAcurrent_reg_addr   0x10
#define pBcurrent_reg_addr   0x11
#define pCcurrent_reg_addr   0x12
#define frequency_reg_addr   0x1E
#define powerfactor_reg_addr 0x1D

// Data variables
int taeHigh, taeLow, activePower;
int pAvolt, pBvolt, pCvolt;
int lABvolt, lBCvolt, lCAvolt;
int pAcurrent, pBcurrent, pCcurrent;
int frequency, powerFactor;

char em_data[128];
ModbusMaster node;

int readModbusData(uint16_t reg_address, uint8_t max_retries) {
  vTaskDelay(pdMS_TO_TICKS(30));
  int value = -1;
  while (max_retries-- > 0) {
    uint8_t result = node.readHoldingRegisters(reg_address, 1);
    if (result == node.ku8MBSuccess) {
      value = node.getResponseBuffer(0);
      Serial.print("Modbus Read 0x");
      Serial.print(reg_address, HEX);
      Serial.print(": ");
      Serial.println(value);
      break;
    }
    vTaskDelay(pdMS_TO_TICKS(60));
  }
  return value;
}

void ParsingModbusData() {
  snprintf(em_data, sizeof(em_data),
           "%s,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
           DEVICE_ID,
           taeHigh, taeLow, activePower,
           pAvolt, pBvolt, pCvolt,
           lABvolt, lBCvolt, lCAvolt,
           pAcurrent, pBcurrent, pCcurrent,
           frequency, powerFactor);
}

bool connectGSM() {
  Serial.println("[GSM] Initializing modem...");
  modem.restart();
  delay(3000);
  if (modem.getSimStatus() != SIM_READY) return false;
  Serial.println("[GSM] Connecting to network...");
  if (!modem.waitForNetwork()) return false;
  if (!modem.gprsConnect(apn)) return false;
  Serial.println("[GSM] Connected to GPRS!");
  return true;
}

void reconnectMqtt() {
  if (!mqtt.connected()) {
    char clientId[32];
    snprintf(clientId, sizeof(clientId), "sim7600_%04X", random(0xffff));
    Serial.print("[MQTT] Connecting as client ID: ");
    Serial.println(clientId);
    if (mqtt.connect(clientId, mqttUser, mqttPass)) {
      Serial.println("[MQTT] Connected");

      // Prepare and subscribe to topic
      snprintf(mqttSubTopic, sizeof(mqttSubTopic), "%s/%s", MQTT_SUB, DEVICE_ID);
      mqtt.subscribe(mqttSubTopic);
      Serial.print("[MQTT] Subscribed to topic: ");
      Serial.println(mqttSubTopic);
    } else {
      Serial.print("[MQTT] Failed, rc=");
      Serial.println(mqtt.state());
    }
  }
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String message;
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  Serial.println("[MQTT IN] Topic: " + String(topic));
  Serial.println("[MQTT IN] Message: " + message);

  // You can add command handling here if needed
}

void setup() {
  Serial.begin(115200);
  delay(2000);

  SerialAT.begin(SIM_BAUD, SERIAL_8N1, MODEM_RX, MODEM_TX);
  pinMode(MODEM_PWR, OUTPUT);
  digitalWrite(MODEM_PWR, HIGH);

  // GSM Connect Retry
  for (int i = 0; i < 5 && !connectGSM(); i++) {
    Serial.println("[GSM] Retry connecting...");
    delay(5000);
  }

  mqtt.setServer(broker, MQTT_PORT);
  mqtt.setCallback(mqttCallback);

  Serial2.begin(9600, SERIAL_8N1, RS485_RX, RS485_TX);
  node.begin(1, Serial2);

  Serial.println("[Setup] Complete");
}

void loop() {
  mqtt.loop();

  if (!modem.isGprsConnected()) connectGSM();
  if (!mqtt.connected()) reconnectMqtt();

  unsigned long currentMillis = millis();
  if (currentMillis - lastPublishTime >= publishInterval) {
    lastPublishTime = currentMillis;

    taeHigh     = readModbusData(taeHigh_reg_addr, 3);
    taeLow      = readModbusData(taeLow_reg_addr, 3);
    activePower = readModbusData(activePower_reg_addr, 2);
    pAvolt      = readModbusData(pAvolt_reg_addr, 1);
    pBvolt      = readModbusData(pBvolt_reg_addr, 1);
    pCvolt      = readModbusData(pCvolt_reg_addr, 2);
    lABvolt     = readModbusData(lABvolt_reg_addr, 1);
    lBCvolt     = readModbusData(lBCvolt_reg_addr, 1);
    lCAvolt     = readModbusData(lCAvolt_reg_addr, 1);
    pAcurrent   = readModbusData(pAcurrent_reg_addr, 1);
    pBcurrent   = readModbusData(pBcurrent_reg_addr, 1);
    pCcurrent   = readModbusData(pCcurrent_reg_addr, 1);
    frequency   = readModbusData(frequency_reg_addr, 1);
    powerFactor = readModbusData(powerfactor_reg_addr, 1);

    Serial.println("--------- Modbus Data ---------");
    Serial.printf("taeHigh: %d\n", taeHigh);
    Serial.printf("taeLow: %d\n", taeLow);
    Serial.printf("Active Power: %d\n", activePower);
    Serial.printf("Phase A Voltage: %d\n", pAvolt);
    Serial.printf("Phase B Voltage: %d\n", pBvolt);
    Serial.printf("Phase C Voltage: %d\n", pCvolt);
    Serial.printf("Line AB Voltage: %d\n", lABvolt);
    Serial.printf("Line BC Voltage: %d\n", lBCvolt);
    Serial.printf("Line CA Voltage: %d\n", lCAvolt);
    Serial.printf("Phase A Current: %d\n", pAcurrent);
    Serial.printf("Phase B Current: %d\n", pBcurrent);
    Serial.printf("Phase C Current: %d\n", pCcurrent);
    Serial.printf("Frequency: %d Hz\n", frequency);
    Serial.printf("Power Factor: %d\n", powerFactor);
    Serial.println("--------------------------------");

    ParsingModbusData();
    Serial.println("[Formatted MQTT Data]");
    Serial.println(em_data);

    mqtt.publish(MQTT_PUB, em_data);
  }
}