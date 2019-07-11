
//Version 0.1

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>

#define tanksize 42

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
int fuel = 0, maf, kmcantravel;
double fuel_consuption_1l_100km,stored_av;
int vss;
double consup[250];
double sum_consup = 0, av_consup = 0;
int i,j;
byte pkey, maxkey, fuel_in_tank, lfuel_in_tank;
boolean debug = true;



void setup()
{
  
  //pinMode(9,OUTPUT); digitalWrite(9,HIGH);
  //Serial.begin(9600); // 9600 bps
  Wire.begin();
  Wire.setClock(400000);
  mySerial.begin(9600);
  Serial.begin(9600);

  
  fuel = (byte)i2c_eeprom_read_byte(0x50, 1); // 0x50-0x57, 0-7
  //i2c_eeprom_write_byte(0x50, 0,0); // null my fuel register just once, when I start using AT24C02

  lcd.init(); //initialize the lcd
  lcd.backlight(); //open the backlight

  lcd.setCursor(0,0);
  lcd.print("Connecting ...");
  
  delay(10000);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Init...");
  lcd.setCursor(0,1);
  lcd.print(ReadDataString("ATZ"));
  
  delay(1000);
  
  lcd.setCursor(0,1);
  lcd.print(ReadDataString("ATI"));
  delay(1000);
  
  lcd.setCursor(0,1);
  lcd.print(ReadDataString("0100"));
  delay(1000);
  

  for (i=0; i<250; i++) {
    consup[i]=0;
  }

  i=0; j=0;  
  maxkey= analogRead(0);
  menu = 1;
  lcd.clear();
  
  lcd.setCursor(0,1);
  lcd.print("Km/h");
  lcd.setCursor(5,1);
  //lcd.print("L/100Km");
}

void loop()
{
  adc_key_in = analogRead(0);    // read the value from the sensor
  key = adc_key_in;  // convert into key press
  if (key != oldkey)   // if keypress is detected
   {
    //delay(50);  // wait for debounce time
    adc_key_in = analogRead(0);    // read the value from the sensor
    key = adc_key_in;   
    
    if (key != oldkey)
    {
      oldkey = key;
      if (key >=0){
        
        //pkey = ((key*100)/maxkey);
        //Serial.println(key); // debug to show keycodes - it help to adjust your own AD keyboard button values
        
        
        switch(pkey)
        {
          //   ˇˇˇ - you will need to change these values for your own AD keyboard
           case 7: // sw1
           case 8: // sw1
           case 9: // sw1
           case 10: // sw1
           case 11: // sw1
                  Serial.println("sw 1");                                   
                  // i2c_eeprom_read_byte(0x50, 1));

                  // ha a 2-es menü alatt nyomjuk meg, a benzin -1L
                  if (menu == 2) manageFuel(-1);
                  break;
                  
           case 24:
           case 25:
           case 26:
           case 27:
           case 28:
           case 29: // sw2
                   Serial.println("sw 2");
                   // i2c_eeprom_write_byte(0x50, 1,19);

                   // ha a 2-es menü alatt nyomjuk meg, a benzin +5L
                   if (menu == 2) manageFuel(+5);
                   break;
                                      
           case 54: // sw3
           case 55:
           case 56:
           case 57:
           case 58:
           case 59:
           case 60:
                  Serial.println("sw 3");

                  // ha a 2-es menü alatt nyomjuk meg, a benzin -5L
                  if (menu == 2) manageFuel(-5);
                  break;

           case 95:
           case 96:
           case 97:
           case 98:
           case 99:
           case 100:
           case 101: //sw4
                  Serial.println("sw 4");
                  
                  // ha a 2-es menü alatt nyomjuk meg, a benzin +1L
                  if (menu == 2) manageFuel(+1);
                  
                  break;
           case 190:
           case 191:
           case 192:
           case 193:
           case 194:
           case 195:
           case 196:
           case 197:
           case 198: 
           case 199:
           case 200:
           // sw5 
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
    if (vss == 0) vss=1;
    //vss=0; maf=1;
    fuel_consuption_1l_100km =   ((maf*0.3355)/vss)*100;
    
    if (isnan(fuel_consuption_1l_100km)) fuel_consuption_1l_100km=0;
    if (isinf(fuel_consuption_1l_100km)) fuel_consuption_1l_100km=0;
    if (fuel_consuption_1l_100km <0) fuel_consuption_1l_100km=0;
    if ( i==250 ) {
      i=0;
      storeAverage(av_consup);
    }
    if ( i==0 ) {
      loadAverage();
      tank("012F");
      lfuel_in_tank = (tanksize * fuel_in_tank)/100;
      kmcantravel = (lfuel_in_tank * 100) / av_consup;
      //  av_consupL  100km
      //   lfuel       ?             
    }
      
    
    consup[i] = fuel_consuption_1l_100km;
    i++;
    sum_consup=0;
    for (j=0;j<250;j++) {
      sum_consup = sum_consup + consup[j];
    }
    //Serial.println(sum_consup);
    if ( consup[249] == 0 ) {
      av_consup = sum_consup/i;
    } else {
      av_consup = sum_consup/250;  
    }
    
    
    
    
    /*
     * Ide jön az 1-es menü kódja. Figyeli a odb2 szenzorokat és kalkulál.
     * Akkor is, ha a 2-es menüben vagyok, vagyis nem kell 1 és 2 menü
     * esetében if -es szerkezet. 
     * 
     */
     // if (menu == 1) {
        
        // lcd.print(inData);
        // lcd.setCursor(1,0);
        //Serial.println(maf);
        lcd.setCursor(0,0);
        if (vss==1) {
          lcd.print("[0] ");
        } else {
          lcd.print(String(vss)+"  ");  
        }
        
        lcd.setCursor(4,0);
        //lcd.print( String(fuel_consuption_1l_100km)+"  "  );
        lcd.print( String(av_consup)+"  "  );  

        lcd.setCursor(13,1);
        lcd.print(String(lfuel_in_tank)+"L  ");
        lcd.setCursor(11,0);
        //lcd.print(String(av_consup)+"   ");
        lcd.print(String(stored_av)+"   ");
        
        //if (debug==1) {
          lcd.setCursor(5,1);
          lcd.print(String(kmcantravel)+"Km");
        //}  
     //}
    /*
     if (menu == 2) {
        lcd.clear();
     }
    */
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

void tank(String str) {
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
  fuel_in_tank =  B ; //((100/255) * B);
  //Serial.println(maf);
  
}


String ReadDataString(String cmd)
{
  // BuildINString="";
  byte inData;
  char inChar;
  mySerial.println(cmd);
  delay(700);
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


// encode and decode 3 digits float on 2 byte

int Encode(double value) {
  int cnt = 0;
  while (value != floor(value)) {
    value *= 10.0;
    cnt++;
  }
  return (short)((cnt << 12) + (int)value);
}

double Decode(short value) {
  int cnt = value >> 12;
  double result = value & 0xfff;
  while (cnt > 0) {
    result /= 10.0;
    cnt--;
  }
  return result;
}

/*
 * 
 *
      
      For converting two bytes the cleanest solution is
      
      data[0] = (byte) width;
      data[1] = (byte) (width >>> 8);
      For converting an integer to four bytes the code would be
      
      data[0] = (byte) width;
      data[1] = (byte) (width >>> 8);
      data[2] = (byte) (width >>> 16);
      data[3] = (byte) (width >>> 24);
 * 
 */

 void storeAverage(double av) {
  // at the moment it only store one average
  int av_int = Encode(av);
  byte av1, av2;
  av1 = (byte) (av_int & 0xFF);
  av2 = (byte) ((av_int >> 8) & 0xFF);
  i2c_eeprom_write_byte(0x51, 0,av1);
  i2c_eeprom_write_byte(0x51, 1,av2);
 }

 void loadAverage() {
  byte av1,av2;
  
  av1 = (byte)i2c_eeprom_read_byte(0x51, 0);
  av2 = (byte)i2c_eeprom_read_byte(0x51, 1);
  int av = ((av1 & 0xFF) << 8) | (av2 & 0xFF);
  stored_av = Decode(av);
  
 }
