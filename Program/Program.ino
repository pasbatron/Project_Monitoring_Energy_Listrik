#include <ModbusRTU.h>
#include <SoftwareSerial.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

#define TX_G16 16
#define RX_G17 17
#define TX_G26 26
#define RX_G27 27
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

Adafruit_SH1106G display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

float InttoFloat(uint16_t Data0, uint16_t Data1) {
  float x;
  unsigned long *p;
  p = (unsigned long*)&x;
  *p = (unsigned long)Data0 << 16 | Data1; // Big-endian
  return (x);
}

bool cb(Modbus::ResultCode event, uint16_t transactionId, void* data) {
  if (event != Modbus::EX_SUCCESS) {
    Serial.print("Request result: 0x");
    Serial.println(event, HEX);
  }
  return true;
}

SoftwareSerial S(RX_G17, TX_G16); // RX, TX
SoftwareSerial mySerialLora(TX_G26, RX_G27); // TX, RX
ModbusRTU mb;

void updateDisplay(const char* message) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(10, 10);
  display.println("Data : ");
  display.setCursor(10, 30);
  display.println(message);
  display.display();
}

void setup() {
  mySerialLora.begin(9600);
  S.begin(19200, SWSERIAL_8E1);
  mb.begin(&S);
  mb.master();
  if (!display.begin(0x3C)) {
    Serial.println(F("OLED tidak terdeteksi!"));
    while (true);
  }
  delay(1000);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(10, 20);
  display.println("Selamat datang,");
  display.setCursor(10, 40);
  display.println("TPS Version 1.0");
  display.display();
  delay(3000); 
}

void readAndSendData(uint8_t slaveId, uint16_t reg, const char* dp, const char* voltage) {
  uint16_t data[2];

  if (!mb.slave()) {
    mb.readHreg(slaveId, reg, data, 2, cb);
    while (mb.slave()) {
      mb.task();
      delay(100);
    }

    float Data_Watt = InttoFloat(data[1], data[0]);
    mySerialLora.print("*,");
    mySerialLora.print(dp);
    mySerialLora.print(",");
    mySerialLora.print(voltage);
    mySerialLora.print(",");
    mySerialLora.print(String(Data_Watt));
    mySerialLora.print(",");
    mySerialLora.print("#");
    mySerialLora.println();
    String data_send = "*" + String(dp) + "," +String(voltage) +"," + String(Data_Watt) + ",#";
    updateDisplay(data_send.c_str());
    delay(8000);
  }
}

void loop() {
  readAndSendData(1, 3960, "DP-CH-E", "PM-220V");
  readAndSendData(2, 3960, "DP-CH-E", "PM-200V");
  readAndSendData(3, 3960, "DP-CP-1ABC", "PM-220V");
  readAndSendData(4, 3960, "DP-CP-1ABC", "PM-200V");
}
