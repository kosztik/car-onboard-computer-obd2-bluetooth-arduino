
//Version 0.1

#include <Wire.h>

// Word to byte
// byte highByte(int x)
// byte lowByte(int x)

//                    sw1 sw2 sw3 sw4  sw5
//int adc_key_val[5] ={0, 19-20, 57, 109, 232-233 };
int NUM_KEYS = 5;
int adc_key_in;
int key=-1;
int oldkey=-1;
void setup()
{
  
  Serial.begin(9600); // 9600 bps
  Wire.begin();
  Wire.setClock(400000);
  
}
void loop()
{
  adc_key_in = analogRead(0);    // read the value from the sensor
  
  key = adc_key_in;  // convert into key press

  if (key != oldkey)   // if keypress is detected
   {
    delay(50);  // wait for debounce time
    adc_key_in = analogRead(0);    // read the value from the sensor
    key = adc_key_in;   
    if (key != oldkey)
    {
      oldkey = key;
      if (key >=0){
        //Serial.println(key); // debug to show keycodes - it help to adjust your own AD keyboard button values
        switch(key)
        {

          //   ˇˇˇ - you will need to change these values for your own AD keyboard
           case 0:Serial.println("sw 1");
                  
                  Serial.println(
                    i2c_eeprom_read_byte(0x50, 1));
                  break;
           case 20:
           case 19:Serial.println("sw 2");
                   i2c_eeprom_write_byte(0x50, 1,19);
                  break;
           case 57:Serial.println("sw 3");
                  break;
           case 109:Serial.println("sw 4");
                  break;
           case 232:
           case 233:Serial.println("sw 5");
                  break;
        }
      }
    }
  }
 delay(100);
}

uint8_t i2c_eeprom_read_byte(uint8_t deviceaddress, uint8_t eeaddress) {

  uint8_t rdata;
  
  Wire.beginTransmission(deviceaddress);  
  Wire.write(eeaddress);
  Wire.endTransmission();

  Wire.requestFrom(deviceaddress,1);
  
  if (Wire.available()) rdata = Wire.read();

  
  return rdata;
}

void i2c_eeprom_write_byte(uint8_t deviceaddress, uint8_t eeaddress, uint8_t data) {
  byte rdata = data;
  Wire.beginTransmission(deviceaddress);
  Wire.write((int)(eeaddress >> 8)); // MSB - Take just the lowest 8 bits.
  Wire.write((int)(eeaddress & 0xFF)); // LSB - Shift the integer right 8 bits
  Wire.write(rdata);
  Wire.endTransmission();
}
