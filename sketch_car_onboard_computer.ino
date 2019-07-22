
//Version 0.1

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>
#include <EEPROM.h>

#define tanksize 42
#define SECONDS 3 // Ennyi másodpercnyi üzemanyagot vonunk le
#define THROTTLE_DEFAULT 14

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
int maf, kmcantravel;
double fuel_consuption_1l_100km,stored_av, fuel_in_tank, lfuel_in_tank;
int vss;

double consupl[5];
double fuel, sum_lpers;
struct lph {
  double lperh[5];
  double consup[25];
  int i;
};

double sum_consup = 0, av_consup = 0, sum_consupl = 0, av_consupl = 0;
int j,k,l,n,m;
boolean debug = true, inspeed = true, fuelsaved =false;
byte throttle, speed_r[]={10, 20, 30, 40, 60, 70, 90, 110, 130};

struct lph lph1;

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
  
  if (lph1.i <0 || lph1.i > 25) lph1.i = 0;
  
  for (k=0;k<5;k++) lph1.lperh[k] = 0;
  for (n=0;n<5;n++) consupl[n] =0;

  j=0, k=0, l=0, n=0, m=0;  

  throttle = 0; vss = 0; maf = 0;
  
  lcd.clear();

  
  EEPROM.get(0,fuel);
  
  EEPROM.get(4,lph1);
  //byte qq;
  //for (qq=0; qq<25; qq++) {
  //  Serial.println(lph1.consup[qq]);
  //}
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
        //Serial.println(key); // debug to show keycodes - it help to adjust your own AD keyboard button values
        //Serial.println(String(maxkey)+"-----");

        
        if (key >=1 && key <= 10 ) {
                  Serial.println("sw 1");                                   
                  // i2c_eeprom_read_byte(0x50, 1));
                  // ha a 2-es menü alatt nyomjuk meg, a benzin -1L
                  if (vss == 0) {
                  manageFuel(-1);
                  }
        }

        if (key >=20 && key <= 35) {                  
                   // sw2
                   Serial.println("sw 2");
                   // i2c_eeprom_write_byte(0x50, 1,19);
                   if (vss == 0) {
                   manageFuel(+0.25);
                   }                   
        }                    

        if (key >=55 && key <= 75 ) {
            // sw3           
                  Serial.println("sw 3");
                  if (vss == 0) {
                  manageFuel(-0.25);
                  }
        }

        if (key >= 100 && key <= 140 ) {
           //sw4
                  Serial.println("sw 4");
                  if (vss == 0) {

                
                    
                    // ha a 2-es menü alatt nyomjuk meg, a benzin +1L
                    manageFuel(+1);
                  }                  
        }

        if (key >= 230 && key <= 250) { // sw5
                  // sw5
           if (vss == 0) {
                  switch ((int)fuel) {
                     case 1:
                       /*
                        * if not moving and fuel set to 1L and push the button 5,
                        * it will reset the long term consuption array.
                        * this is for testing purpose.
                        */
                       null_consup();
                     break;
                      
                      
                     default:
                      fuel= -1 ;
                      manageFuel(0);
                     break;
                  }
            
                  
           }        
        }
      }
    }

    /*
     * 
     * Kiolvasok minden adatot amire szükségem lehet.
     */
    vss = VSS(ReadDataString("010D"));

    // if fuel not saved we save it. And save some consuption data too
    if (fuelsaved == false) {      
      EEPROM.put(0, fuel);
      EEPROM.put(4, lph1);
      fuelsaved = true;
      maf = 0;      
      //Serial.println(lph1.i);
    }

    // if we are not is moving show the fuel settings
    if (vss == 0) {
      if (inspeed == true) {
        lcd.clear();
        inspeed = false;
      }
      lcd.setCursor(0,0);
      lcd.print("Fuel: ");
      lcd.setCursor(6,0);
      lcd.print(String(fuel)+"L  ");
    }
    //vss=30;

    // if we are moving start calculation the consuption
    
    if (vss > 5) 
    {
      inspeed = true; fuelsaved = false;
      
      throttle = THROTTLE(ReadDataString("0111"));
      if (throttle > THROTTLE_DEFAULT ){
              //throttle = THROTTLE(ReadDataString("0111"));      
                      
              
        
              MAF(ReadDataString("0110"));
              //maf=2;
              fuel_consuption_1l_100km =   ((maf*0.3355)/vss)*100;              
              if (isnan(fuel_consuption_1l_100km)) fuel_consuption_1l_100km=0;
              if (isinf(fuel_consuption_1l_100km)) fuel_consuption_1l_100km=0;
              if (fuel_consuption_1l_100km <0) fuel_consuption_1l_100km=0;
              
              if ( lph1.i==25 ) {
                lph1.i=0;
                //storeAverage(av_consup);
              }

              if (k == 5) k=0;
              if (m == 5) { // ha a rövid átlag kiszámolva, eltárolom a hosszúban
                
                m=0;
                lph1.consup[lph1.i] = av_consupl;
                lph1.i++;   
              }

              lfuel_in_tank = (tanksize * fuel)/100;
              kmcantravel = (lfuel_in_tank * 100) / av_consup;
              //Serial.println(fuel);
                             
              
              consupl[m]= fuel_consuption_1l_100km;
              
              lph1.lperh[k] = maf * 0.335; // L/h

          //  lph1.i: hoszabb átlag fogyasztás
          //  k: rövid átlag L/h
          //  m: rövid átlag fogyasztás
          
              k++; m++; // csak akkor lépek tovább ha nyomjuk a gázt
              //Serial.println( av_consup );
              sum_consup=0;
              for (j=0;j<25;j++) {
                sum_consup = sum_consup + lph1.consup[j];
              }

              sum_consupl=0;
              for (n=0;n<5;n++) {
                sum_consupl = sum_consupl + consupl[n]; 
              }

              
              for (l=0; l<5; l++) {
                sum_lpers = sum_lpers + lph1.lperh[l];
              }

              if (lph1.lperh[4] == 0) {
                sum_lpers = sum_lpers /k; // átlag
                sum_lpers = sum_lpers / 3600; // adott másodpercben 
                fuel = fuel - (sum_lpers * SECONDS);
              }
              
              if (lph1.lperh[4] > 0 ) { 
                sum_lpers = sum_lpers /5; // átlag
                sum_lpers = sum_lpers / 3600; // adott másodpercben 
                fuel = fuel - (sum_lpers * SECONDS); 
              }
              
              //Serial.println(sum_consup);
              if ( lph1.consup[24] == 0 ) {
                av_consup = sum_consup/ lph1.i;
              } else {
                av_consup = sum_consup/25;  
              }
              
              
              if ( consupl[4] == 0 ) {
                av_consupl = sum_consupl/m;
              } else {
                av_consupl = sum_consupl/5;  
              }
              
        
         
              /*
               * Innen jönnek a kiiratások, amikor sebességben vagyunk 
               * 
               */
                                 
                  // lcd.print(inData);
                  // lcd.setCursor(1,0);
                  //Serial.println(maf);
                  lcd.setCursor(0,0);
                  lcd.print(String(vss)+" ");  

                  lcd.setCursor(0,1);
                  lcd.print("Km/h");
                  lcd.setCursor(5,1);
                  
                  lcd.setCursor(5,0);
                  //lcd.print( String(fuel_consuption_1l_100km)+" "  );
                  lcd.print( String(av_consupl)+" "  );
                  
                  lcd.setCursor(11,0);
                  //lcd.print( String(fuel_consuption_1l_100km)+" "  );
                  lcd.print( String(av_consup)+" "  );  
                  
                  lcd.setCursor(10,1);
                  lcd.print(String(fuel) + "L");
                  
                  //lcd.print(String(stored_av)+"   ");
                  
                  lcd.setCursor(5,1);
                  lcd.print(String(kmcantravel)+"Km");
    }
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

void manageFuel(double f) {

  fuel += f;
  if (fuel < 0) fuel = 0;
  EEPROM.put(0,fuel);
  // i2c_eeprom_write_byte(0x50, 1,(byte)fuel);
  //Serial.println(fuel);   
}

int VSS (String str) {
  long A;
  String work;  
  work = str.substring(11,13);
  A = strtol(work.c_str(), NULL, 16);
  return A;
}

byte THROTTLE(String str) {
  byte A, ret;
  String work;  
  work = str.substring(11,13);
  A = strtol(work.c_str(), NULL, 16);
  ret = A / 2.55;
  //Serial.print(ret);
  return (ret);
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
  return (int)((cnt << 12) + (int)value);
}

double Decode(int value) {
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

 void null_consup() {
  byte ti;
  lcd.noBacklight();
  delay(200);
  lcd.backlight();
  lph1.i = 0;
  for (ti=0; ti<25; ti++) {
    lph1.consup[ti]=0;
  }
 }
