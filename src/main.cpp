#include <Arduino.h>

#include <ModbusMaster.h>

// RS485 Serial2 Pins (No DE/RE control needed)
#define RS485_RX 27  // RX of ESP32 receives from TX of device
#define RS485_TX 14  // TX of ESP32 sends to RX of device

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

// Modbus data variables
int taeHigh, taeLow, activePower;
int pAvolt, pBvolt, pCvolt;
int lABvolt, lBCvolt, lCAvolt;
int pAcurrent, pBcurrent, pCcurrent;
int frequency, powerFactor;

ModbusMaster node;

// Function to read one holding register
// int readModbusData(uint16_t reg_address) {
//   uint8_t result = node.readHoldingRegisters(reg_address, 1);
//   if (result == node.ku8MBSuccess) {
//     return node.getResponseBuffer(0);
//   } else {
//     Serial.print("Modbus error at reg ");
//     Serial.print(reg_address, HEX);
//     Serial.print(": code ");
//     Serial.println(result);
//     return -1;
//   }
// }

int readModbusData(uint16_t reg_address, uint8_t max_retries) {
  
  vTaskDelay(pdMS_TO_TICKS(30));

  int value = -1;
  while (max_retries > 0) {  // Continue while there are retries left
    uint8_t result = node.readHoldingRegisters(reg_address, 1);
    if (result == node.ku8MBSuccess) {
      value = node.getResponseBuffer(0);
      Serial.print("Trying to get data from Modbus: ");
      Serial.print(reg_address);
      Serial.print(": ");
      Serial.println(value);
      break; // Exit the loop if a valid value is read
    }
    max_retries--;  // Decrease the retry count
    vTaskDelay(pdMS_TO_TICKS(60)); // Small delay between retries (100ms)
  }

  return value; // Return the value, -1 if all retries failed
}

void setup() {
  Serial.begin(115200);

  // Start Serial2 on your chosen RX/TX pins
  Serial2.begin(9600, SERIAL_8N1, RS485_RX, RS485_TX);

  // Init Modbus master (slave ID 1)
  node.begin(1, Serial2);

  Serial.println("Modbus setup complete.");
}

void loop() {
  // Read Modbus registers
  // taeHigh     = readModbusData(taeHigh_reg_addr);
  // taeLow      = readModbusData(taeLow_reg_addr);
  // activePower = readModbusData(activePower_reg_addr);
  // pAvolt      = readModbusData(pAvolt_reg_addr);
  // pBvolt      = readModbusData(pBvolt_reg_addr);
  // pCvolt      = readModbusData(pCvolt_reg_addr);
  // lABvolt     = readModbusData(lABvolt_reg_addr);
  // lBCvolt     = readModbusData(lBCvolt_reg_addr);
  // lCAvolt     = readModbusData(lCAvolt_reg_addr);
  // pAcurrent   = readModbusData(pAcurrent_reg_addr);
  // pBcurrent   = readModbusData(pBcurrent_reg_addr);
  // pCcurrent   = readModbusData(pCcurrent_reg_addr);
  // frequency   = readModbusData(frequency_reg_addr);
  // powerFactor = readModbusData(powerfactor_reg_addr);

  taeHigh = readModbusData(taeHigh_reg_addr, 3);       // Retry up to 3 times
  taeLow = readModbusData(taeLow_reg_addr, 3);         // Retry up to 3 times
  activePower = readModbusData(activePower_reg_addr, 2); // Retry up to 2 times
  pAvolt = readModbusData(pAvolt_reg_addr, 1);         // Retry up to 1 times
  pBvolt = readModbusData(pBvolt_reg_addr, 1);         // Retry up to 1 times
  pCvolt = readModbusData(pCvolt_reg_addr, 1);         // Retry up to 2 times
  lABvolt = readModbusData(lABvolt_reg_addr, 1);       // Retry up to 1 time
  lBCvolt = readModbusData(lBCvolt_reg_addr, 1);       // Retry up to 1 time
  lCAvolt = readModbusData(lCAvolt_reg_addr, 1);       // Retry up to 1 time
  pAcurrent = readModbusData(pAcurrent_reg_addr, 1);   // Retry up to 2 times
  pBcurrent = readModbusData(pBcurrent_reg_addr, 1);   // Retry up to 2 times
  pCcurrent = readModbusData(pCcurrent_reg_addr, 1);   // Retry up to 2 times
  frequency = readModbusData(frequency_reg_addr, 1);   // Retry up to 1 time
  powerFactor = readModbusData(powerfactor_reg_addr, 1); // Retry up to 1 time

  Serial.println("--------------------------------");
  Serial.println();
  Serial.println();

  // Print to Serial
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
  Serial.println();
  Serial.println();


  delay(30000);  // Wait 10 seconds
}
