#include <Wire.h>
#include <MCP342x.h>

//0x68 is the default address for the MCP3426
#define MCP3426_ADDRESS 0x68

MCP342x adc = MCP342x(MCP3426_ADDRESS);
long value = 0;

void initMCP3426(){
  Wire.begin();
  
  MCP342x::generalCallReset();
  delay(1);                                   //MC342x needs 300us to settle, wait 1ms
  
  //check device present
  Wire.requestFrom(MCP3426_ADDRESS, 1);
  
  /*if (!Wire.available()) {
    Serial.print("No device found at address ");
    Serial.println(address, HEX);
    while (1)
      ;
  }*/
}

long readMCP3426(){
  MCP342x::Config status;

  //initiate a conversion; convertAndRead() will wait until it can be read
  uint8_t err = adc.convertAndRead(MCP342x::channel1, MCP342x::oneShot, MCP342x::resolution16, MCP342x::gain1, 1000000, value, status);

  /*if (err) {
    Serial.print("Convert error: ");
    Serial.println(err);
  }
  else {
    Serial.print("Value: ");
    Serial.println(value);
    voltage = 0.0000625*value;
    Serial.print("Voltage: ");
    Serial.println(voltage);
  }*/

  return value;
}