
//Version 0.1

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>

LiquidCrystal_I2C lcd(0x27,16,2); // set the LCD address to 0x27 for a 16 chars and 2 line display
SoftwareSerial mySerial(10, 11); // RX, TX

/*
 * AT24C02 (0x50-0x57) stored values:
 *  0x50,1 : fuel
 * 
 * 
 * 
 */



// Word to byte
// byte highByte(int x)
// byte lowByte(int x)

//                    sw1 sw2 sw3 sw4  sw5
//int adc_key_val[5] ={0, 19-20, 57, 109, 232-233 };
int NUM_KEYS = 5;
int adc_key_in;
int key=-1;
int oldkey=-1;
byte menu = 1;
int fuel = 0, maf;
double fuel_consuption_1l_100km;
int vss;

void setup()
{
  
  //pinMode(9,OUTPUT); digitalWrite(9,HIGH);
  //Serial.begin(9600); // 9600 bps
  Wire.begin();
  Wire.setClock(400000);
  mySerial.begin(9600);
  Serial.begin(9600);
  fuel = (byte)i2c_eeprom_read_byte(0x50, 1);
  //i2c_eeprom_write_byte(0x50, 0,0); // null my fuel register just once, when I start using AT24C02

  lcd.init(); //initialize the lcd
  lcd.backlight(); //open the backlight

  lcd.setCursor(1,0);
  lcd.print("Connecting ...");
  
  //Serial.println("Enter AT commands:");
  
  delay(10000);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("No Data");
  lcd.setCursor(0,0);
  lcd.print(ReadDataString("ATZ"));
  
  delay(2000);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(ReadDataString("ATI"));
  delay(2000);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Init... ERROR!");
  lcd.setCursor(0,0);
  lcd.print(ReadDataString("0100"));
  delay(2000);
  lcd.clear();
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
           case 0: // sw1
                  Serial.println("sw 1");                                   
                  // i2c_eeprom_read_byte(0x50, 1));

                  // ha a 2-es menü alatt nyomjuk meg, a benzin -1L
                  if (menu == 2) manageFuel(-1);
                  break;
                  
           case 20:
           case 19: // sw2
                   Serial.println("sw 2");
                   // i2c_eeprom_write_byte(0x50, 1,19);

                   // ha a 2-es menü alatt nyomjuk meg, a benzin +5L
                   if (menu == 2) manageFuel(+5);
                   break;
                                      
           case 57: // sw3
                  Serial.println("sw 3");

                  // ha a 2-es menü alatt nyomjuk meg, a benzin -5L
                  if (menu == 2) manageFuel(-5);
                  break;

           case 110:
           case 109: //sw4
                  Serial.println("sw 4");
                  
                  // ha a 2-es menü alatt nyomjuk meg, a benzin +1L
                  if (menu == 2) manageFuel(+1);
                  
                  break;
           case 232:
           case 233: // sw5 
                  // Serial.println("sw 5");
                  if (menu == 2 ) {menu = 1;}
                  else {menu = 2;}
                  
                  Serial.println(menu);
                  break;
        }
      }
    }

    /*
     * 
     * Kiolvasok minden adatot amire szükségem lehet.
     */
    vss = VSS(ReadDataString("010D"));
    MAF(ReadDataString("0110"));
    //vss=0; maf=1;
    fuel_consuption_1l_100km =  (vss * 9069.90) /
    ( maf *3600);
    
    if (isnan(fuel_consuption_1l_100km)) fuel_consuption_1l_100km=0;
    if (isinf(fuel_consuption_1l_100km)) fuel_consuption_1l_100km=0;
    if (fuel_consuption_1l_100km <0) fuel_consuption_1l_100km=0;
    
    //Serial.println(maf);
    lcd.setCursor(0,0);
    lcd.print(String(vss)+"  ");
    lcd.setCursor(4,0);
    lcd.print( String(fuel_consuption_1l_100km)+"  "  );  
    lcd.setCursor(0,1);
    lcd.print("Km/h");
    lcd.setCursor(6,1);
    lcd.print("L/100Km");
    
    /*
     * Ide jön az 1-es menü kódja. Figyeli a odb2 szenzorokat és kalkulál.
     * Akkor is, ha a 2-es menüben vagyok, vagyis nem kell 1 és 2 menü
     * esetében if -es szerkezet. 
     * 
     */
     if (menu == 1) {
        // lcd.print(inData);
        // lcd.setCursor(1,0);
        
     }
    
  }
 // delay(100);
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

void manageFuel(int f) {

  fuel += f;
  if (fuel < 0) fuel = 0;
  i2c_eeprom_write_byte(0x50, 1,(byte)fuel);
  //Serial.println(fuel);
  
    
}

int VSS (String str) {
  long A;
  String work;  
  work = str.substring(11,13);
  A = strtol(work.c_str(), NULL, 16);
  return A;
}

// 0110]41 10 01 58 >
void MAF(String str) {
  long A;
  int B;
  String work="";
  work = str.substring(11,13);
  //work="01";
  A = strtol(work.c_str(), NULL, 16);
  work = str.substring(14,16);
  //work="DC";
  B = strtol(work.c_str(), NULL, 16);
  //Serial.println(B);
  maf = ((256*A+B) / 100);
  //Serial.println(maf);
}


String ReadDataString(String cmd)
{
  // BuildINString="";
  byte inData;
  char inChar;
  mySerial.println(cmd);
  delay(1000);
  String BuildINString="";  
  
  while(mySerial.available() > 0)
  {
    inData=0;
    inChar=0;
    inData = mySerial.read();
    inChar=char(inData);
    BuildINString = BuildINString + inChar;
  }
  
  //Serial.println(BuildINString);
  return BuildINString;
}
