
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
double fuel_consuption_1l_100km,stored_av, fuel_in_tank, lfuel_in_tank;
int vss;
double consup[250];
double fuelc, lperh[5],  sum_lpers;

double sum_consup = 0, av_consup = 0;
int i,j,k,l;
boolean debug = true, inspeed = true, fuelsaved =false;
byte speed_r[]={10, 20, 30, 40, 60, 70, 90, 110, 130};


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

  for (k=0;k<5;k++) lperh[k] = 0;

  

  i=0; j=0, k=0, l=0;  
  
  menu = 1;
  lcd.clear();
  
  
  fuelc = fuel;
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
    //Serial.println(adc_key_in);
    if (key != oldkey)
    {
      oldkey = key;
      if (key >=0){
        
        //pkey = ((key*100)/maxkey);
       // Serial.println(key); // debug to show keycodes - it help to adjust your own AD keyboard button values
        //Serial.println(String(maxkey)+"-----");
        
        if (key >=10 && key <= 29 ) {
                  Serial.println("sw 1");                                   
                  // i2c_eeprom_read_byte(0x50, 1));

                  // ha a 2-es menü alatt nyomjuk meg, a benzin -1L
                  manageFuel(-1);
        }

        if (key >=30 && key <= 60) {
                  
                   // sw2
                   Serial.println("sw 2");
                   // i2c_eeprom_write_byte(0x50, 1,19);

                   // ha a 2-es menü alatt nyomjuk meg, a benzin +5L
                   manageFuel(+5);
                   
        }                    

        if (key >=70 && key <= 90 ) {
            // sw3
           
                  Serial.println("sw 3");

                  // ha a 2-es menü alatt nyomjuk meg, a benzin -5L
                  manageFuel(-5);
                  
        }

        if (key >= 120 && key <= 140 ) {
           //sw4
                  Serial.println("sw 4");
                  
                  // ha a 2-es menü alatt nyomjuk meg, a benzin +1L
                  manageFuel(+1);
                  
        }

        if (key >= 230 && key <= 250) { // sw5
           // sw5 

                  


        
        }
      }
    }

    /*
     * 
     * Kiolvasok minden adatot amire szükségem lehet.
     */
    vss = VSS(ReadDataString("010D"));

    if (fuelsaved == false) {

      fuel = fuelc;
      manageFuel(0);
      fuelsaved = true;
      
    }
    
    //vss=60;
    if (vss > 0) 
    {
              inspeed = true; fuelsaved = false;
              
              MAF(ReadDataString("0110"));
              //maf=2;
              fuel_consuption_1l_100km =   ((maf*0.3355)/vss)*100;
              
              if (isnan(fuel_consuption_1l_100km)) fuel_consuption_1l_100km=0;
              if (isinf(fuel_consuption_1l_100km)) fuel_consuption_1l_100km=0;
              if (fuel_consuption_1l_100km <0) fuel_consuption_1l_100km=0;
              if ( i==250 ) {
                i=0;
                storeAverage(av_consup);
              }

              if (k == 5) k=0;


              lfuel_in_tank = (tanksize * fuelc)/100;
              kmcantravel = (lfuel_in_tank * 100) / av_consup;
              
              if ( i==0 ) {
                loadAverage();
                
                
                //  av_consupL  100km
                //   lfuel       ?             
              }
                
              
              consup[i] = fuel_consuption_1l_100km;
              
              lperh[k] = maf * 0.335; // L/h
              
              i++; k++;
              
              sum_consup=0;
              for (j=0;j<250;j++) {
                sum_consup = sum_consup + consup[j];
              }

              for (l=0; l<5; l++) {
                sum_lpers = sum_lpers + lperh[l];
              }
              
              sum_lpers = sum_lpers /5; // átlag
              sum_lpers = sum_lpers / 3600; // adott másodpercben
              fuelc = fuelc - sum_lpers; 
              Serial.println(fuelc,6);

              
              //Serial.println(sum_consup);
              if ( consup[249] == 0 ) {
                av_consup = sum_consup/i;
              } else {
                av_consup = sum_consup/250;  
              }
              
              
              
              
              /*
               * Innen jönbnek a kiiratások, amikor sebességben vagyunk 
               * 
               */
               
                  
                  // lcd.print(inData);
                  // lcd.setCursor(1,0);
                  //Serial.println(maf);
                  lcd.setCursor(0,0);
                  lcd.print(String(vss)+"  ");  

                  lcd.setCursor(0,1);
                  lcd.print("Km/h");
                  lcd.setCursor(5,1);
                  
                  lcd.setCursor(4,0);
                  //lcd.print( String(fuel_consuption_1l_100km)+"  "  );
                  lcd.print( String(av_consup)+"  "  );  
          
                  lcd.setCursor(13,1);
                  lcd.print(String(fuelc) + "L");
                  lcd.setCursor(11,0);
                  //lcd.print(String(av_consup)+"   ");
                  lcd.print(String(stored_av)+"   ");
                  
                  //if (debug==1) {
                    lcd.setCursor(5,1);
                    lcd.print(String(kmcantravel)+"Km");
                  //}  
               //}
              
    }

    if (vss == 0) {
      if (inspeed == true) {
        lcd.clear();
        inspeed = false;
      }
      
      lcd.setCursor(0,0);
      lcd.print("Fuel: ");
      lcd.setCursor(6,0);
      lcd.print(String(fuel)+"  ");
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
  i2c_eeprom_write_byte(0x51, 1,av1);
  i2c_eeprom_write_byte(0x51, 2,av2);
 }

 void loadAverage() {
  byte av1,av2;
  
  av1 = (byte)i2c_eeprom_read_byte(0x51, 1);
  av2 = (byte)i2c_eeprom_read_byte(0x51, 2);
  int av = ((av1 & 0xFF) << 8) | (av2 & 0xFF);
  stored_av = Decode(av);
  
 }
