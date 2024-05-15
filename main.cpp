#include <ModbusRTU.h>
#include <SoftwareSerial.h>

#define TX_G16 16
#define RX_G17 17
#define RE_DE_ENABLE 15

float InttoFloat(uint16_t Data0, uint16_t Data1){
  float x;
  unsigned long *p;

  p = (unsigned long*)&x;
  *p = (unsigned long)Data0 << 16 | Data1; //Bid-endian
  return (x);
}
SoftwareSerial S(RX_G17, TX_G16); // RX, TX
ModbusRTU mb;

bool cb(Modbus::ResultCode event, uint16_t transactionId, void* data){
  if(event != Modbus::EX_SUCCESS){
    Serial.print("Request result : 0x");
    Serial.print(event, HEX);
  }
  return true;
}


void setup() {
  Serial.begin(9600);
  S.begin(19200, SWSERIAL_8E1);
  mb.begin(&S,RE_DE_ENABLE);
  mb.master();

}

uint16_t val[2];

void loop() {
  
  if (!mb.slave()) {
    
    mb.readHreg(2, 3926, val, 2, cb); // Slave id is 1 and register address is 3926 and 
    //we are reading 2 bytes from the register and saving in val
    while(mb.slave()) { // Check if transaction is active
      mb.task();
      delay(100);
    }
    Serial.print("Register Values ");
    Serial.print(val[0]);
    Serial.print(",");
    Serial.println(val[1]);
    float voltage= InttoFloat(val[1],val[0]);
    Serial.print("Voltage= ");
    Serial.print(voltage);
    Serial.println(" V");

      }
  delay(1000);
}
