#include <Arduino.h>
#include <ModbusMaster.h>

// Define RX and TX pins for Serial2 (Modbus)
#define RS485_RX 27
#define RS485_TX 14

// Create ModbusMaster instance
ModbusMaster node;

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("Modbus Read Test: Address 0 to 100");

  // Initialize Serial2 for Modbus communication
  Serial2.begin(9600, SERIAL_8N1, RS485_RX, RS485_TX);

  // Initialize Modbus communication: slave ID 1
  node.begin(1, Serial2);
}

void loop() {
  Serial.println("Reading Modbus Holding Registers (0–100):");

  for (uint16_t addr = 0; addr <= 100; addr++) {
    uint8_t result = node.readInputRegisters(addr, 1);
    if (result == node.ku8MBSuccess) {
      uint16_t value = node.getResponseBuffer(0);
      Serial.printf("Address %03d: Value = %d\n", addr, value);
    } else {
      Serial.printf("Address %03d: Read Failed (Code: %d)\n", addr, result);
    }

    delay(100); // Small delay between reads to prevent overload
  }

  Serial.println("Done reading 0–100.\n");
  delay(5000); // Wait before next cycle
}
