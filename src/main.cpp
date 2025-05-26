#include <Arduino.h>

#include <ModbusMaster.h>

// RS485 Serial2 Pins (No DE/RE control needed)
#define RS485_RX 27  // RX of ESP32 receives from TX of device
#define RS485_TX 14  // TX of ESP32 sends to RX of device

// Modbus register addresses
#define tNetEnergy_reg_addr 0x3A
#define tImpEnergy_reg_addr 0x60
#define activePower_reg_addr 0x2A
#define pAvolt_reg_addr 0x00
#define pBvolt_reg_addr 0x02
#define pCvolt_reg_addr 0x04
#define lABvolt_reg_addr 0x08
#define lBCvolt_reg_addr 0x0A
#define lCAvolt_reg_addr 0x0C
#define pAcurrent_reg_addr 0x10
#define pBcurrent_reg_addr 0x12
#define pCcurrent_reg_addr 0x14
#define frequency_reg_addr 0x38
#define powerfactor_reg_addr 0x36

// Modbus data variables
float tNetEnergy, tImpEnergy, activePower;
float pAvolt, pBvolt, pCvolt;
float lABvolt, lBCvolt, lCAvolt;
float pAcurrent, pBcurrent, pCcurrent;
float frequency, powerFactor;

ModbusMaster node;


float readModbusData(uint16_t regAddress, uint8_t maxRetries) {
  
  vTaskDelay(pdMS_TO_TICKS(150));
  while (maxRetries > 0) {
    uint8_t result = node.readInputRegisters(regAddress, 2);
    
    if (result == node.ku8MBSuccess) {
      uint16_t lowWord = node.getResponseBuffer(0);  // LSB stored in lower register
      uint16_t highWord = node.getResponseBuffer(1); // MSB stored in higher register

      union {
        uint32_t intVal;
        float floatVal;
      } converter;

      converter.intVal = ((uint32_t)highWord << 16) | lowWord;
      return converter.floatVal; // Return value if read is successful
    } else {
      maxRetries--;
      Serial.println("Modbus Read Error, Retrying...");
      vTaskDelay(pdMS_TO_TICKS(100)); // Optionally add a delay between retries
    }
  }
  
  // If all retries failed, return NaN to indicate an error
  Serial.println("Modbus Read Failed after retries");
  return NAN;
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

  tNetEnergy = readModbusData(tNetEnergy_reg_addr, 2);       // Retry up to 3 times
  tImpEnergy = readModbusData(tImpEnergy_reg_addr, 2);         // Retry up to 3 times
  activePower = readModbusData(activePower_reg_addr, 2); // Retry up to 2 times
  pAvolt = readModbusData(pAvolt_reg_addr, 2);         // Retry up to 1 times
  pBvolt = readModbusData(pBvolt_reg_addr, 2);         // Retry up to 1 times
  pCvolt = readModbusData(pCvolt_reg_addr, 2);         // Retry up to 2 times
  lABvolt = readModbusData(lABvolt_reg_addr, 2);       // Retry up to 1 time
  lBCvolt = readModbusData(lBCvolt_reg_addr, 2);       // Retry up to 1 time
  lCAvolt = readModbusData(lCAvolt_reg_addr, 2);       // Retry up to 1 time
  pAcurrent = readModbusData(pAcurrent_reg_addr, 2);   // Retry up to 2 times
  pBcurrent = readModbusData(pBcurrent_reg_addr, 2);   // Retry up to 2 times
  pCcurrent = readModbusData(pCcurrent_reg_addr, 2);   // Retry up to 2 times
  frequency = readModbusData(frequency_reg_addr, 2);   // Retry up to 1 time
  powerFactor = readModbusData(powerfactor_reg_addr, 2); // Retry up to 1 time

  // Print to Serial
  Serial.println("--------- Modbus Data ---------");
  Serial.printf("Total Net Energy: %.2f\n", tNetEnergy);
  Serial.printf("Total Import Energy: %.2f\n", tImpEnergy);
  Serial.printf("Active Power: %.2f\n", activePower);

  Serial.printf("Phase A Voltage: %.2f\n", pAvolt);
  Serial.printf("Phase B Voltage: %.2f\n", pBvolt);
  Serial.printf("Phase C Voltage: %.2f\n", pCvolt);

  Serial.printf("Line AB Voltage: %.2f\n", lABvolt);
  Serial.printf("Line BC Voltage: %.2f\n", lBCvolt);
  Serial.printf("Line CA Voltage: %.2f\n", lCAvolt);

  Serial.printf("Phase A Current: %.2f\n", pAcurrent);
  Serial.printf("Phase B Current: %.2f\n", pBcurrent);
  Serial.printf("Phase C Current: %.2f\n", pCcurrent);

  Serial.printf("Frequency: %.2f Hz\n", frequency);
  Serial.printf("Power Factor: %.2f\n", powerFactor);
  Serial.println("--------------------------------");

  Serial.println();
  Serial.println();


  delay(30000);  // Wait 10 seconds
}
