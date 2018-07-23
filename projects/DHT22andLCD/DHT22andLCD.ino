//#include <BMP180.h>
#include <Dhcp.h>
#include <Dns.h>
#include <Ethernet.h>
#include <EthernetClient.h>
#include <EthernetServer.h>
#include <EthernetUdp.h>
#include <ThingSpeak.h>
#include <DHT22.h>
#include <LiquidCrystal_I2C.h>
#include <MQ135.h>
#include <stdio.h>
#include <SoftwareSerial.h>

LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Set the LCD I2C address

#define DHT22_PIN 4
#define PIN_MQ135 A2

MQ135 mq135_sensor = MQ135(PIN_MQ135);
DHT22 myDHT22(DHT22_PIN);

// Settaggi ThingSpeak
const char* host = "api.thingspeak.com";
const int httpPort = 80;
const char * THINGSPEAK_API_KEY = "2S9B80A65Z7DDWOY";


// Preparo il display

// Ho un display 16x2 quindi...
int screenWidth = 16;
int screenHeight = 2;

// line1 = fissa
String line1;
// line2 = scrolla da dx verso sx 
String line2;

int stringStart, stringStop = 0;
int scrollCursor = screenWidth;

// Preparo la grafica in fase di bootstrap

byte a3[8]= { 
            B00011,         //Defining an "ARRAY" of type "byte" for each 5*8 pixel of LCD. in total LCD has total of 
            B00111,         //32 matrix of 5*8 pixels(16-TOP, 16-BOTTOM) each '1' will power ONE Pixel of each byte '0' will keep ONE pixel OFF. 
            B01100,                
            B11000,             
            B10000,                    
            B00000,             
            B00000   
};

/*how shapes can be created: //for example in the 4th row of of the Above ARRAY will work for first 5*8 matrix of LCD Top row
                               in this 4th row of the 5*8 matrix first 2 pixel will turn ON or you can see a BLACK DOT lit up
                               on the first 2 pixel and the remaining 3 will remain OFF. the process is 
                               same for every other 5*8 matrix and you can program this and create any kind of
                               shape by giving '1' & '0' to each pixel. */

byte a2[8]= {
            B00000,          //this is the 2nd array "a2" is just a name.
            B00000,
            B00000,
            B00000,
            B10001,
            B11111,
            B01110
};

byte a1[8]={
            B11000,            //3rd array with name "a1".
            B01100,
            B00110,
            B00011,
            B00001,
            B00000,
            B00000
};

byte a4[8]={ 
            B00000,               //4th array with name "a4".
            B00000,
            B00000,
            B00001,
            B00011,
            B00110,
            B01100,
            B11000
};

byte a5[8]={ 
            B00000,               //5th array with name "a5".
            B01110,
            B11111,
            B10001,
            B00000,
            B00000,
            B00000,
            B01110
};

byte a6[8]={  
            B00000,               //sixth array with name "a6".
            B00000,
            B00000,
            B10000,
            B11000,
            B01100,
            B00110,
            B00011
};

byte a7[8]={ 
            B00000,                //seventh array with name "a7".
            B01110,
            B11111,
            B10001,
            B00000,
            B00000,
            B00000,
            B00000,
};

byte a8[8]={
            B00100,             //Eighth array with name "a8".
            B01110,
            B00100,
            B00000,
            B10001,
            B11111,
            B01110
};
  
byte temp[8] = {
                B00100,
                B01010,
                B01010,
                B01110,
                B01110,
                B11111,
                B11111,
                B01110
};

byte hum[8] = {
               B00100,
               B00100,
               B01010,
               B01010,
               B10001,
               B10001,
               B10001,
               B01110
};

byte degree[8] = {
                  B11100,
                  B10100,
                  B11100,
                  B00000,
                  B00000,
                  B00000,
                  B00000,
                  B00000
};   
   
byte heart[8] = {
                 B00000,
                 B01010,
                 B11111,
                 B11111,
                 B01110,
                 B00100,
                 B00000,
};
// Definizione variabili
int check;
int led = 13;
int rxPin = 7;
int txPin = 6;
int TMax = 0;
int HMax = 0;
int TMin = 100;
int HMin = 100;
char MessageOut[21];
char MessageOut2[21];
int index = 19, index2 = 0;
unsigned long oldTime = 0, oldTime2 = 0;
String message;

SoftwareSerial bluetooth(rxPin, txPin);


//#define USE_WIFI101_SHIELD
#define USE_ETHERNET_SHIELD

#if defined(ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_ESP32)
  #error "EPS8266 and ESP32 are not compatible with this example."
#endif

#if !defined(USE_WIFI101_SHIELD) && !defined(USE_ETHERNET_SHIELD) && !defined(ARDUINO_SAMD_MKR1000) && !defined(ARDUINO_AVR_YUN) 
  #error "Uncomment the #define for either USE_WIFI101_SHIELD or USE_ETHERNET_SHIELD"
#endif

#if defined(ARDUINO_AVR_YUN)
    #include "YunClient.h"
    YunClient client;
#else
  #if defined(USE_WIFI101_SHIELD) || defined(ARDUINO_SAMD_MKR1000)
    // Use WiFi
    #include <SPI.h>
    #include <WiFi101.h>
    char ssid[] = "VodafoneSurfer";    //  your network SSID (name) 
    char pass[] = "B311a@P3r.T3";   // your network password
    int status = WL_IDLE_STATUS;
    WiFiClient  client;
  #elif defined(USE_ETHERNET_SHIELD)
    // Use wired ethernet shield
    #include <SPI.h>
    #include <Ethernet.h>
    byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
    EthernetClient client;
  #endif
#endif

void setup(void)
{
  // start serial port
 Serial.begin(9600);
 Serial.println(F("Temperatura, Umidita' e Qualita' dell'Aria con Sensori DHT22 e MQ135 - Powered By MR WATT"));
 pinMode(led, OUTPUT);
 Serial3.begin(9600);
 Serial.println("Lista dei comandi Bluetooh HC-06:\n");
 Serial.println("AT              Se la comunicazione funziona il modulo risponde OK");
 Serial.println("AT+VERSION      Restituisce la versione del firmware");
 Serial.println("AT+BAUD7        Imposta il Baudrate, al posto di x mettere 1 per 1200 bps, 2=2400, 3=4800, 4=9600, 5=19200, 6=38400, 7=57600, 8=115200, 9=230400, A=460800, B=921600, C=1382400");
 Serial.println("AT+NAMEMeteo-Bluetooth   Al posto di string mettere il nome che vuoi dare al modulo (massimo 20 caratteri)");
 Serial.println("AT+PIN8375      Imposta il pincode del modulo bluetooth (es.1234)");
 lcd.begin(screenWidth,screenHeight);
 
 #ifdef ARDUINO_AVR_YUN
      Bridge.begin();
       #else   
           #if defined(USE_WIFI101_SHIELD) || defined(ARDUINO_SAMD_MKR1000)
              WiFi.begin(ssid, pass);
                #else
        Ethernet.begin(mac);
    #endif
  #endif
  Serial.println("");
  Serial.println("Ethernet connected");  
  Serial.println("IP address: ");
  Serial.println(Ethernet.localIP());
  Serial.print("connecting to ");
  Serial.println(host);

    // Inizializzo il display, numero di righe e colonne e poi costruisco il logo Arduino
  lcd.begin(16,2);
  lcd.clear();
  lcd.print("Arduino");     //the message to be printed when LCD is powered. initially the cursor is on "ROW=1, COLUMN=0"
  lcd.setCursor(1,1);       //set the Location at where the next message(logo) will be printed in this its on "ROW=1 & COLUMN=1".
  lcd.print("Logo");        //this message printed on ROW=1, COLUMN=1.
  lcd.createChar(0,a1);   //here we create a CUSTOM character for each 5*8 matrix in the brackets '0' is
  //the number(as in counting such as 0,1,2...) of ARRAY and " a1 " is the NAME of ARRAY
  lcd.createChar(1,a2);   //the process is same for the rest of the CUSTOM CHARACTERS.
  lcd.createChar(2,a3);  
  lcd.createChar(7,a8);
  lcd.createChar(3,a4);     
  lcd.createChar(4,a5);
  lcd.createChar(5,a6);
  lcd.createChar(6,a7);
  lcd.setCursor(10,0);     //now we Set cursor on Position having COLUMN= 10 and ROW 0.
  lcd.write(byte(3));     //this command will Display the data of array on location "3" or array "a4" on the LCD .
  lcd.write(byte(4));     //this command will Display the data of array on location "4" or array "a5" on the LCD .
  lcd.write(byte(5));     //this command will Display the data of array on location "5" or array "a6" on the LCD .
  lcd.write(byte(3));    
  lcd.write(byte(6));  // we have to create another byte so that we can replace this byte
  // with that and get the + design.
  lcd.write(byte(5));
  delay(2000);           // just a 2 second delay before showing the LOWER HALF of the logo
  lcd.setCursor(10,1);  //set the cursor on column=1 and ROW=2 so to Display the LOWER half on this position.
  lcd.write(byte(0));    //this command will Display the data of array on location "0" or array "a1" on the LCD .
  lcd.write(byte(1));    //this command will Display the data of array on location "1" or array "a2" on the LCD .
  lcd.write(byte(2));    //this command will Display the data of array on location "2" or array "a3" on the LCD .
  lcd.write(byte(0));    //this command will Display the data of array on location "0" or array "a1" on the LCD .
  lcd.write(byte(7));    // we have to create another byte so that we can replace this byte
  // with that and get the - design.
  lcd.write(byte(2));    //this command will Display the data of array on location "2" or array "a3" on the LCD .
  delay(3000);  
  lcd.clear();         
  lcd.createChar(9, temp);
  lcd.createChar(10, hum);
  lcd.createChar(11, degree);
  lcd.createChar(12, heart);
  lcd.setCursor(0,0);
  lcd.write(9);
  lcd.print("Weather Station");
  lcd.write(10);
  lcd.setCursor(0,1);
  lcd.write(12); 
  lcd.print("MR WATT SRL");
  lcd.write(12);
  delay(3000);
  lcd.clear(); 
}

void loop(void)
{ 
 void clearLCD();
 DHT22_ERROR_t errorCode;
 float rzero = mq135_sensor.getRZero();
 float temperature = myDHT22.getTemperatureC();
 float humidity = myDHT22.getHumidity();
 float correctedRZero = mq135_sensor.getCorrectedRZero(temperature, humidity);
 float resistance = mq135_sensor.getResistance();
 float ppm = mq135_sensor.getPPM();
 float correctedPPM = mq135_sensor.getCorrectedPPM(temperature, humidity);
 float sensorVoltage; 
 float sensorValue;
 

 sensorValue = analogRead(A2);
 sensorVoltage = sensorValue/1024*5.0;

 if (Serial3.available())
  {  
    Serial.write(Serial3.read());
  }
  if (Serial.available())
  {
    Serial3.write(Serial.read());
  }

 // Attendo il riscaldamento del sensore MQ135
 if ( sensorVoltage > 2 ) {correctedPPM = 0;}
     // Attenzione!! La lettura dell'umidita e della temperatura richiedono circa 250ms quindi impostare un delay superiore a 2 secondi!
     delay(2000);
     //Serial.print("Requesting data...");
     //Serial.print("\n");
     errorCode = myDHT22.readData();
     switch(errorCode)
           {
            case DHT_ERROR_NONE:
            //Serial.print(myDHT22.getTemperatureC(),1);
            //Serial.print("C ");
            //Serial.print(",\t\t");
            //Serial.print(myDHT22.getHumidity(),1);
            //Serial.println("%");
            // Alternately, with integer formatting which is clumsier but more compact to store and
	          // can be compared reliably for equality:
	          //	  
            char buf[128];
            sprintf(buf, "Integer-only reading: Temperature %hi.%01hi C, Humidity %i.%01i %% RH",
                     myDHT22.getTemperatureCInt()/10, abs(myDHT22.getTemperatureCInt()%10),
                     myDHT22.getHumidityInt()/10, myDHT22.getHumidityInt()%10);
            // Serial.println(buf);
            break;
    case DHT_ERROR_CHECKSUM:
      Serial.print(F("check sum error "));
      Serial.print(myDHT22.getTemperatureC());
      Serial.print(F("C "));
      Serial.print(myDHT22.getHumidity());
      Serial.println(F("%"));
      break;
      
    case DHT_BUS_HUNG:
      Serial.println(F("BUS Hung "));
      break;
    case DHT_ERROR_NOT_PRESENT:
      Serial.println(F("Not Present "));
      break;
    case DHT_ERROR_ACK_TOO_LONG:
      Serial.println(F("ACK time out "));
      break;
    case DHT_ERROR_SYNC_TIMEOUT:
      Serial.println(F("Sync Timeout "));
      break;
    case DHT_ERROR_DATA_TIMEOUT:
      Serial.println(F("Data Timeout "));
      break;
    case DHT_ERROR_TOOQUICK:
      Serial.println(F("Polled to quick "));
      break;
      
 }

 Serial.print(F("Temperature: "));
 Serial.print(myDHT22.getTemperatureC());
 Serial.print(F("C "));
 Serial.print(F("\t Humidity: ")); 
 Serial.print(myDHT22.getHumidity());
 Serial.print(F("%"));
 Serial.print(F("\t Sensor Voltage: "));
 Serial.print(sensorVoltage);
 Serial.print(F("V"));
 Serial.print(F("\t MQ135 RZero: "));
 Serial.print(rzero);
 Serial.print(F("\t Corrected RZero: "));
 Serial.print(correctedRZero);
 Serial.print(F("\t Resistance: "));
 Serial.print(resistance);
 Serial.print(F("\t PPM: "));
 Serial.print(ppm);
 Serial.print(F("\t Corrected PPM: "));
 Serial.print(correctedPPM);
 Serial.println(F("ppm"));
 delay(300);
 lcd.setCursor(0,0);
 lcd.write(9);
 lcd.print(myDHT22.getTemperatureC());
 lcd.write(11);
 lcd.print("C ");
 lcd.write(10);
 lcd.print(myDHT22.getHumidity());
 lcd.print("%");
 // Condizioni per impostare la Temperatura e Umidita Minima e Massima registrata
 if ( myDHT22.getTemperatureC() > TMax ) {TMax = myDHT22.getTemperatureC();}
 if ( myDHT22.getHumidity() > HMax ) {HMax = myDHT22.getHumidity();}
 if ( myDHT22.getTemperatureC() < TMin ) {TMin = myDHT22.getTemperatureC();}
 if ( myDHT22.getHumidity() < HMin ) { HMin = myDHT22.getHumidity();}
 // Aspetto 5 secondi per visualizzare le TMin e HMin

 
 delay(5000); 
  
 for(int iLoopTemp=0;iLoopTemp<6;iLoopTemp++)
     {
      clearLCD();
      lcd.setCursor(0, 1);
      lcd.print("MAX: ");
      lcd.write(9);
      lcd.print(TMax,DEC);
      lcd.write(11);
      lcd.print("C ");
      lcd.write(10);
      lcd.print(HMax,DEC);
      lcd.print("%");
      // Aspetto 5 secondi per visualizzare le TMax e HMax
      delay(5000);
      clearLCD(); 
      lcd.setCursor(0, 1);
      lcd.print("MIN: ");
      lcd.write(9);
      lcd.print(TMin,DEC);
      lcd.write(11);
      lcd.print("C ");
      lcd.write(10);
      lcd.print(HMin,DEC);
      lcd.print("%");
      delay(5000);
      clearLCD(); 
      lcd.setCursor(0, 1);
      lcd.print("Q.Aria:");
      String AirQuality;
      // Livelli di CO2 OUTDOOR
      // normal outdoor level: 350 - 450 ppm
      // acceptable levels: < 600 ppm
      // complaints of stiffness and odors: 600 - 1000 ppm
      // ASHRAE and OSHA standards: 1000 ppm
      // general drowsiness: 1000 - 2500 ppm
      // adverse health effects may be expected: 2500 - 5000 ppm
      // maximum allowed concentration within a 8 hour working period: 5000 - 10000 ppm  
      // maximum allowed concentration within a 15 minute working period: 30000 ppm 
      // Livelli di CO2 INDOOR
      // Excellent : 400 -600 ppm
      // Good : 700 - 800 ppm
      // Fair : 900 - 1000 ppm
      // Mediocre : 1100 - 1500 ppm
      // Bad : 1600 - 2000 ppm
      // Toxic : > 2100
      
       if ( correctedPPM > 1 && correctedPPM < 700 ) {AirQuality = ">>>EXCELLENT<<<";}
       if ( correctedPPM > 700 && correctedPPM < 800 ) {AirQuality = ">>>GOOD<<<";}
       if ( correctedPPM > 800 && correctedPPM < 1000 ) {AirQuality = ">>>FAIR<<<";}
       if ( correctedPPM > 1000 && correctedPPM < 1500 ) {AirQuality = ">>>MEDIOCRE!<<<";}
       if ( correctedPPM > 1500 && correctedPPM < 2000 ) {AirQuality = ">>>!!!BAD!!!<<<";}
       if ( correctedPPM > 2000 && correctedPPM < 50000 ) {AirQuality = ">>>!!!TOXIC!!!<<<";}
       if ( correctedPPM > 50000 || correctedPPM == 0 ) {AirQuality = ">PREHEAT MQ135<";correctedPPM = 0;}  
       
      lcd.print(correctedPPM);
      lcd.print("PPM");
      delay(3000); 
      clearLCD(); 
      lcd.setCursor(0, 1);
      lcd.print(AirQuality);
      delay(3000);

// Funzione che scrolla il testo Ancora non attiva
/*
lcd.setCursor(scrollCursor, 1);
  lcd.print(line2.substring(stringStart,stringStop));
  lcd.setCursor(0, 0);
  lcd.print(line2);
  delay(300);
  lcd.clear();
  if(stringStart == 0 && scrollCursor > 0){
    scrollCursor--;
    stringStop++;
  } else if (stringStart == stringStop){
    stringStart = stringStop = 0;
    scrollCursor = screenWidth;
  } else if (stringStop == line1.length() && scrollCursor == 0) {
    stringStart++;
  } else {
    stringStart++;
    stringStop++;
  }
*/

 }
     // DISPLAT DATA
     tone(2,2000,250);
     digitalWrite(led, HIGH);
     delay(500);
     digitalWrite(led, LOW);  
     sendGET(ppm);
}

//Funzione che pulisce la seconda riga del display
void clearLCD(void)
     {
      lcd.setCursor(0, 1);
      for(int iclear=0;iclear<16;iclear++)
          {
           lcd.print(" ");
          }
     }


void sendGET(float QAir) //client function to send/receive GET request data.
             {
              float h = myDHT22.getHumidity();
              float t = myDHT22.getTemperatureC();
              String url = "/update?api_key=";
              if (client.connect(host,httpPort)) {  //starts client connection, checks for connection
                  Serial.println("connected");
                  url += THINGSPEAK_API_KEY;
                  url += "&field1=";
                  url += String(t);
                  url += "&field2=";
                  url += String(h);
                  url += "&field3=";
                  url += String(QAir);
                  Serial.print("Requesting URL: ");
                  Serial.println(url);
                  // This will send the request to the server
                  client.println("GET " + url + " HTTP/1.1");
                  client.println("Host: api.thingspeak.com");
                  client.println("Connection: close");
                  delay(10);
                  client.println();
                  } 
                  else {
                        Serial.println("connection failed"); //error message if no client connect
                        Serial.println();
                  }

                  while(client.connected() && !client.available()) delay(1); //waits for data
                  while (client.connected() || client.available()) { //connected or data available
                                                                    char c = client.read(); //gets byte from ethernet buffer
                                                                    Serial.print(c); //prints byte to serial monitor 
                    }

  Serial.println();
  Serial.println("disconnecting.");
  Serial.println("==================");
  Serial.println();
  client.stop(); //stop client

}

