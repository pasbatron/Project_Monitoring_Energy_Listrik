#include <LiquidCrystal_I2C.h>
#include <ModbusRTU.h>
#include <SoftwareSerial.h>


#define TX_G16 16
#define RX_G17 17

#define TX_G26 26
#define RX_G27 27
int lcdColumns = 16;
int lcdRows = 2;
// #define RE_DE_ENABLE 15

float InttoFloat(uint16_t Data0, uint16_t Data1){
  float x;
  unsigned long *p;

  p = (unsigned long*)&x;
  *p = (unsigned long)Data0 << 16 | Data1; //Bid-endian
  return (x);
}
SoftwareSerial S(RX_G27, TX_G26); // RX, TX
SoftwareSerial mySerialLora(TX_G16, RX_G17); //TX, RX
ModbusRTU mb;
LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);  

bool cb(Modbus::ResultCode event, uint16_t transactionId, void* data){
  if(event != Modbus::EX_SUCCESS){
    Serial.print("Request result : 0x");
    Serial.print(event, HEX);
  }
  return true;
}

void setup() {
  Serial.begin(9600);
  mySerialLora.begin(9600);
  S.begin(19200, SWSERIAL_8E1);
  mb.begin(&S);
  mb.master();
  lcd.init();                    
  lcd.backlight();
}

uint16_t data[2];
void loop() {

  if (!mb.slave()){
    mb.readHreg(2, 3960, data, 2, cb);
    while(mb.slave()) {
      mb.task();
      delay(100);
    }

    Serial.print("Register Values ");
    Serial.print(data[0]);
    Serial.print(",");
    Serial.println(data[1]);

    float Data= InttoFloat(data[1],data[0]);
    Serial.print("Data= ");
    Serial.print(Data);
    Serial.println(" Satuan");
    mySerialLora.println("line-cam-housing;" + String(Data) + ";");
    lcd.setCursor(0, 0);
    lcd.print("Data : ");
    lcd.setCursor(0,1);
    lcd.print(Data);
  }
  delay(10000);
  lcd.clear();
}
