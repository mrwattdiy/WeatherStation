/*
 MR WATT - CECCHETTI SIMONE v.1.0 202006171611
 Below are the mapped pins:
Arduino = 0 -> WeMos = D3
Arduino = 1 -> WeMos= NC
Arduino = 2 -> WeMos= D4 (LED)
Arduino = 3 -> WeMos= NC
Arduino = 4 -> WeMos= D2
Arduino = 5 -> WeMos= D1
Arduino = 6 -> WeMos= Watchdog(4) Reset
Arduino = 7 -> WeMos= Watchdog(4) Reset
Arduino = 8 -> WeMos= Watchdog(4) Reset
Arduino = 9 -> WeMos= Watchdog(4) Reset
Arduino = 10 -> WeMos= NC
Arduino = 11 -> WeMos= Watchdog(4) Reset
Arduino = 12 -> WeMos= D7 low, D6 sporadic
Arduino = 13 -> WeMos= D0 high, D7 sporadic
Arduino = 14 -> WeMos= D0 high, D6 Low, D7 Low, D5 Sporadic
Arduino = 15 -> WeMos= High, D8 Sporadic
Arduino = 16 -> WeMos= D0
Arduino = 17 -> WeMos= NC
Arduino = 18 -> WeMos= NC
Arduino = 19 -> WeMos= NC
Arduino = 20 -> WeMos= Stack Trace and Pin-Based(2) Reset
Arduino = 21 -> WeMos= Stack Trace and Watchdog(4) Reset
Arduino = 22 -> WeMos= Stack Trace and Watchdog(4) Reset
Arduino = 23 -> WeMos= Stack Trace and Watchdog(4) Reset
Arduino = 24 -> WeMos= NC
Arduino = 25 -> WeMos= NC
Arduino = 26 -> WeMos= NC
Arduino = 27 -> WeMos= NC
Arduino = 28 -> WeMos= NC
Arduino = 29 -> WeMos= NC
 */



#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiUdp.h>
#include <MQ135.h>
#include <stdio.h>
#include <ThingSpeak.h>
#include <Wire.h>
#include <Adafruit_BMP085.h>
#include <DHT.h>
#include <Arduino.h>
#include <U8g2lib.h>
#include <FS.h> // FOR SPIFFS
#include <ctype.h> // for isNumber check
#include "Time.h"
#include "TimeLib.h"
#include "secrets.h"
#include <Adafruit_SSD1306.h>
#include <Arduino.h>
#include <SoftwareSerial.h>
#include <WiFiClientSecure.h>

// Define the pins being used

int pin_switch = D3;

#define FREQUENCY 80
extern "C" {
#include "user_interface.h"
}


// Initialize PM10/PM2.5 Sensor SEN0177
//#define LENG 31   //0x42 + 31 bytes equal to 32 bytes
//unsigned char buf[LENG];

//int PM01Value=0;          //define PM1.0 value of the air detector module
//int PM2_5Value=0;         //define PM2.5 value of the air detector module
//int PM10Value=0;         //define PM10 value of the air detector module

//SoftwareSerial PMSerial(D6, D7); // RX, TX



// Initialize Display
#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

// Please UNCOMMENT one of the contructor lines below
// U8g2 Contructor List (Frame Buffer)
// The complete list is available here: https://github.com/olikraus/u8g2/wiki/u8g2setupcpp
// Please update the pin numbers according to your setup. Use U8X8_PIN_NONE if the reset pin is not connected
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ SCL, /* data=*/ SDA, /* reset=*/ U8X8_PIN_NONE);   // All Boards without Reset of the Display



// Wifi Settings

char ssid[] = SECRET_SSID;  //  looking for your network SSID (name) in credentials.h file
char pass[] = SECRET_PASS;       // looking for your network password in credentials.h file 

const String fName = "props.txt"; // properties file

float UTCoffset = 0;
unsigned long lastMillis = 0;
unsigned long currentMillis = 0;
unsigned long secsSince1900 = 0;
bool daylightSavings = false;
bool hourTime = true;
int interval = 30000; //
String timeStr = "";
String webMessage = "";
String dateStr = "";
float dewpt=0;          // dew point tempf


WiFiClient  client;
WiFiClientSecure clientsecure;

//Initialize DHT22 Sensor
#define DHTPIN 2     // what digital pin we're connected to

// Uncomment whatever type you're using!
//#define DHTTYPE DHT11   // DHT 11
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
//#define DHTTYPE DHT21   // DHT 21 (AM2301)

// Connect pin 1 (on the left) of the sensor to +5V
// NOTE: If using a board with 3.3V logic like an Arduino Due connect pin 1
// to 3.3V instead of 5V!
// Connect pin 2 of the sensor to whatever your DHTPIN is
// Connect pin 4 (on the right) of the sensor to GROUND
// Connect a 10K resistor from pin 2 (data) to pin 1 (power) of the sensor

// Initialize DHT sensor.
// Note that older versions of this library took an optional third parameter to
// tweak the timings for faster processors.  This parameter is no longer needed
// as the current DHT reading algorithm adjusts itself to work on faster procs.
DHT dht(DHTPIN, DHTTYPE);


// Initialize MQ135 Sensor
#define PIN_MQ135 A0
MQ135 mq135_sensor = MQ135(PIN_MQ135);

//Initialize BMP180 Barometer Sensor
Adafruit_BMP085 bmp;


// ThingSpeak Settings

unsigned long myChannelNumber = SECRET_CH_ID_WEATHER_STATION; //Your ThingSpeak Channel Number 
const char * myWriteAPIKey = SECRET_WRITE_APIKEY;
const char * myReadAPIKey = SECRET_READ_APIKEY;

// Settaggi Barometro BMP180 
// Valore calcolato in base all'altitudine : http://www.calctool.org/CALC/phys/default/pres_at_alt ho utilizzato il valore medio
float seaLevelPressure = 100457;

unsigned int localPort = 2390;      // local port to listen for UDP packets

/* Don't hardwire the IP address or we won't get the benefits of the pool.
    Lookup the IP address for the host name instead */
IPAddress timeServerIP; // time.inrim.it NTP server address
const char* ntpServerName = "time.inrim.it";


const int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message

byte packetBuffer[ NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets

// A UDP instance to let us send and receive packets over UDP
WiFiUDP udp;

// Initialize the OLED display using Wire library
Adafruit_SSD1306  display(0x3c, SDA, SCL);



ESP8266WebServer server(80);

// Disegno logo aziendale in fase di BootStrap
static unsigned char ACROBOT[] PROGMEM ={
0x80, 0x00, 0x40, 0x00, 
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x7F, 0x1F, 0x0F, 0x0F, 0x0F, 0x3F, 0xFF, 0x07, 0x01, 0x00, 0x00,
0x00, 0x00, 0x01, 0x03, 0xFF, 0xFF, 0xFB, 0x07, 0x07, 0x07, 0x07, 0x07, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0x07, 0x07, 0x07, 0x07, 0x07, 0xFB, 0xFF, 0xFF, 0x07, 0x07, 0x07, 0x07,
0x07, 0x07, 0x07, 0x07, 0x07, 0x0F, 0x0F, 0x1F, 0x7F, 0xFF, 0x87, 0x07, 0x07, 0x07, 0x07, 0x07,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0xFF, 0xFF, 0xFF, 0xFF,
0x1F, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0xF7, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
0x01, 0x00, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x80, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x3F, 0xFF, 0xFF,
0xFF, 0xFF, 0x3F, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00,
0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xF8, 0x00, 0x00, 0x00, 0x01, 0xFF, 0xC0, 0x00, 0x00, 0x00, 0x00,
0x00, 0x7F, 0xFF, 0xFF, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0xFF, 0xFF, 0xFF,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x78, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x10, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF0, 0xF0, 0xF0,
0xF0, 0xF0, 0xF0, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0,
0x80, 0x80, 0x80, 0xC0, 0xFE, 0xD0, 0xE0, 0x40, 0x60, 0xD0, 0xFC, 0xFF, 0xF8, 0x70, 0x30, 0x30,
0x30, 0x78, 0xBE, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x06, 0xE0, 0x00, 0x00, 0x03, 0x7F,
0x7F, 0x03, 0x00, 0x00, 0xC0, 0xFF, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00,
0x3F, 0x3F, 0x3F, 0x3F, 0x1F, 0x07, 0x00, 0x00, 0x80, 0xE0, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x3F, 0x7F, 0x00, 0x00, 0x00, 0x00, 0xF8, 0x00, 0x00, 0x00, 0x00, 0x0F, 0xFF, 0x07,
0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xC0, 0x00, 0x00, 0x00,
0x00, 0x00, 0x03, 0x3F, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0x01, 0x00, 0x00, 0x00, 0x3F, 0x01, 0x00, 0x00, 0x00, 0x00, 0x03, 0xFF, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x03, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xC0, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0xC0, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00,
0xF0, 0xF0, 0xF0, 0xF0, 0xC0, 0x00, 0x00, 0x02, 0x0F, 0x7F, 0xFF, 0xFF, 0xF0, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x0F, 0x08, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x7F, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0x80, 0x00, 0x00, 0x80, 0xFC, 0x80, 0x00, 0x00, 0x00, 0x80, 0xC0, 0xFF, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0xC0, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFC, 0xE0, 0xE0,
0xE0, 0xE0, 0xFC, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF8, 0x80, 0x00, 0x00, 0x03, 0x1F, 0xFF, 0xF8, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFE, 0xFF, 0xFF, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xE0,
0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0x01, 0x00, 0x00, 0x01, 0xFF, 0x07, 0x03, 0x03, 0x07, 0x0E, 0x3F, 0xFF, 0x1F, 0x0E, 0x0C, 0x0C,
0x0C, 0x3E, 0x7D, 0xFF, 0xFF, 0xFF, 0xFF, 0xE0, 0xE0, 0xE0, 0xE0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xE0, 0xE0, 0xE0, 0xFF, 0xFF, 0xFF, 0xE0, 0xE0, 0xE0, 0xE0,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xE0, 0xE0, 0xE0, 0xE0, 0xEF, 0xFF, 0xDC, 0xE0,
0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF8, 0xE0, 0xE0, 0xE0, 0xE0,
0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE1, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0x80, 0x00, 0x00, 0x00, 0xF8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC7, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x03, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0x00, 0x00, 0x07, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x78, 0x00,
0x00, 0xCC, 0xCC, 0xFF, 0xE0, 0x01, 0x01, 0x80, 0x00, 0xF8, 0xFC, 0x00, 0x00, 0x7F, 0xFF, 0x00,
0x00, 0x00, 0x00, 0x00, 0xFF, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x88, 0x88, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
0x00, 0x88, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x08, 0x08, 0x00, 0x00, 0x00, 0xC0, 0xF0,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0xF8, 0xF0, 0xF0, 0xF8, 0xFC, 0xFF, 0xC0, 0x80, 0x00, 0x00,
0x00, 0x00, 0x80, 0xC0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xF8, 0xF8, 0xFE, 0xF8, 0xF8, 0xF8, 0xF8, 0xFF, 0xF8, 0xF8, 0xF8, 0xFC, 0xF8, 0xF8,
0xF8, 0xF8, 0xF8, 0xF9, 0xFF, 0xF8, 0xF8, 0xFF, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xFC,
0xF8, 0xF8, 0xFE, 0xF8, 0xFB, 0xFE, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8,
0xF8, 0xF8, 0xFE, 0xF8, 0xF8, 0xFF, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xFC, 0xF8, 0xF8, 0xF8,
0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xFC, 0xF8, 0xF8, 0xF8, 0xFC, 0xF8, 0xF8, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};

#define SUN  0
#define SUN_CLOUD  1
#define CLOUD 2
#define RAIN 3
#define THUNDER 4
#define TEMPERATUREDAY 5
#define HUMIDITY 6
#define PRESSURE 7
#define AIRQ 8
#define DEWPT 9
#define TEMPERATURENIGHT 10
#define PM2.5 11
#define PM10 12

void setup() {
//  PMSerial.begin(9600);   
//  PMSerial.setTimeout(1500);
  Serial.begin(9600);
  Serial.println();
  Serial.println();
  Serial.print("Sketch:   ");   Serial.println(__FILE__);
  Serial.print("Uploaded: ");   Serial.println(__DATE__);
  Serial.println(" ");
  Serial.println("WEATHER STATION - Temperature, Pressure, Humidity, Air Quality using Sensors DHT22, MQ135 and BMP180 - Powered By MR WATT");
  Serial.println("BootStrap...");
  //pinMode(D6,INPUT);
  //pinMode(D7,OUTPUT);
  pinMode(A0, INPUT);
// New Feature WPS WiFI
// Serial.println("\nPress WPS button on your router ...");
//  delay(5000);
//  startWPSPBC();
  
  //Settings for Pin Switch
  pinMode(pin_switch, INPUT);
  
  //display.init();
  // display.flipScreenVertically();
  // display.setFont(ArialMT_Plain_16);
  // display.setTextAlignment(TEXT_ALIGN_LEFT);

  // We start by connecting to a WiFi network
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");

  // create and/or read properties file
  initPropFile();

  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  Serial.println("Starting UDP");
  udp.begin(localPort);
  Serial.print("Local port: ");
  Serial.println(udp.localPort());


  server.on("/", handle_root);
  server.on("/submit", handle_submit);
  server.on("/time", handle_time); // Used for AJAX call
  server.begin();
  delay(100);

  setDateTime();

  if (secsSince1900 == 0) // try again, NTP server may not have responded in time
    setDateTime();

  
 //  Wire.begin();  
  // Initialize Display
  u8g2.begin();
  u8g2.enableUTF8Print();
  u8g2.clearBuffer(); // clear the internal memory
  u8g2.setFlipMode(0);
  u8g2.firstPage();
  do {
    drawLogo();
    drawURL();
  } while ( u8g2.nextPage() );
  delay(4000);

  
  pinMode(LED_BUILTIN, OUTPUT);     // Initialize the LED_BUILTIN pin as an output
  // We start by connecting to a WiFi network
 /*
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
*/
 // We start the I2C on the Arduino for communication with the BMP180 sensor.
  Wire.begin();
 
if (!bmp.begin()) {
  Serial.println("Could not find a valid BMP085 sensor, check wiring!");
  while (1) {}
  }

  // DHT22 Test
  Serial.println("Initialize DHT22...");
  dht.begin();

  Serial.println("Initialize ThingSpeak Channel...");
 // Initialize ThingSpeak
  ThingSpeak.begin(clientsecure);

}


void loop() {
 // Wait a few seconds between measurements. 
 delay(5000);

 const int deepSleepSecs = 300;
 unsigned long int actSleepTime = deepSleepSecs * 1000000;
 float rzero = mq135_sensor.getRZero();
 float temperature = dht.readTemperature();
 float humidity = dht.readHumidity();
 float correctedRZero = mq135_sensor.getCorrectedRZero(temperature, humidity);
 float resistance = mq135_sensor.getResistance();
 float ppm = mq135_sensor.getPPM();
 float correctedPPM = mq135_sensor.getCorrectedPPM(temperature, humidity);
 float sensorVoltage; 
 float sensorValue;
 long currentPressure = bmp.readPressure();
 float currentAltitude = bmp.readAltitude(seaLevelPressure);
 float currentTemperature = bmp.readTemperature();
 float currentSealevelPressure = bmp.readSealevelPressure();
 int currentPressurehPA = int(bmp.readPressure()/100);
 //float currentAltitude = getAltitude(bmp.readPressure(), bmp.readTemperature());
 unsigned int raw=0;
 float volt=0.0;
 // Time to sleep (in seconds):
 const int sleepTimeS = 60;
 String oledTemp;
 String oledHum;
 String oledPressure;
 String oledPPM;
 String oledDew;
 float dewpt = 0;
 float wetBulbT = 0;
 float BatteryVoltage=0.0;

 oledTemp = currentTemperature;
 oledHum = humidity;
 oledPressure = currentPressurehPA;
 oledPPM = ppm;
 oledDew = dewpt;

 currentMillis = millis();

  String AmPm = "";

  u8g2.sleepOff();
  WiFi.mode(WIFI_STA);  
  wifi_station_connect();
  WiFi.begin(ssid, pass);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");

  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

   // How much time has passed, accounting for rollover with subtraction!
  if ((unsigned long)(currentMillis - lastMillis) >= interval )
  {
    setDateTime();
  }

  // setup the time and dateString

  int thour = hour();

  if (!hourTime) // if not 24 hour time
  {
    AmPm = "AM";
    if (thour > 12)
    {
      thour = thour - 12;
      AmPm = "PM";
    }
    else
    {
      AmPm = "AM";
      if (thour == 12)
        AmPm = "PM";
    }
    if (thour == 0)
      thour = 12;
  }
  timeStr = thour;

  Serial.print("The time is ");
  Serial.print(hour());
  Serial.print(':');
  timeStr += ":";

  if ( minute() < 10 ) {
    // In the first 10 minutes of each hour, we'll want a leading '0'
    Serial.print('0');
    timeStr += "0";
  }

  Serial.print(minute());

  timeStr += minute();
  Serial.print(':');
  timeStr += ":";

  if ( second() < 10 ) {
    // In the first 10 seconds of each minute, we'll want a leading '0'
    Serial.print('0');
    timeStr += "0";
  }

  Serial.println(second());

  timeStr += second();
  timeStr += " " + AmPm;

  dateStr = day();
  dateStr += "/" ;
  dateStr += month();
  dateStr += "/";
  dateStr += year();
  Serial.print("Date: ");
  Serial.print(dateStr);
  Serial.print("\n");

 server.handleClient();

 delay(1000);

 // Check if any reads failed and exit early (to try again).
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  // Compute heat index in Celsius (isFahreheit = false)
  float hic = dht.computeHeatIndex(temperature, humidity, false);

 /*
 * Wemos battery shield, measure Vbat
 * add 100k between Vbat and ADC
 * Voltage divider of 100k+220k over 100k
 * gives 100/420k
 * ergo 4.2V -> 1Volt
 * Max input on A0=1Volt ->1023
 * 4.2*(Raw/1023)=Vbat
 */

  BatteryVoltage=0.0;
  raw = analogRead(A0);
  BatteryVoltage=(raw*4.2);
  
 
 //sensorValue = analogRead(A0);
 //sensorVoltage = sensorValue/1024*5.0;

 dewpt = (dewPoint(temperature, humidity));
 wetBulbT = (wetBulb(temperature, humidity, currentPressurehPA));

 
 Serial.print("\t Temperature DHT22: ");
 Serial.print(temperature);
 Serial.print("C ");
 Serial.print("\n");
 Serial.print("\t Humidity: "); 
 Serial.print(humidity);
 Serial.print("%");
 Serial.print("\n");
 Serial.print("\t Sensor Voltage: ");
 Serial.print(sensorVoltage);
 Serial.print("V");
 Serial.print("\n");
 Serial.print("\t MQ135 RZero: ");
 Serial.print(rzero);
 Serial.print("\n");
 Serial.print("\t Corrected RZero: ");
 Serial.print(correctedRZero);
 Serial.print("\n");
 Serial.print("\t Resistance: ");
 Serial.print(resistance);
 Serial.print("\n");
 Serial.print("\t PPM: ");
 Serial.print(ppm);
 Serial.print("\n");
 Serial.print("\t Corrected PPM: ");
 Serial.print(correctedPPM);
 Serial.print("ppm");
 Serial.print("\n");
 Serial.print("\t Altitude: ");
 Serial.print(currentAltitude);
 Serial.print("m ");
 Serial.print("\n");
 Serial.print("\t Pressure: ");
 Serial.print(currentPressure);
 Serial.print("Pa ");
 Serial.print("\n");
 Serial.print("\t Temperature BMP180: ");
 Serial.print(currentTemperature);
 Serial.print("C");
 Serial.print("\n");
 Serial.print("\t Pressure Sealevel BMP180: ");
 Serial.print(currentSealevelPressure);
 Serial.print("m ");
 Serial.print("\n");
 Serial.print("\t Dew Point: ");
 Serial.print(dewpt);
 Serial.print("\n");
 Serial.print("\t Wet Bulb Temperature: ");
 Serial.print(wetBulbT);
 Serial.print("C ");
 Serial.print("\n");
 Serial.print("\t Battery Spanning: ");
 Serial.print(BatteryVoltage);
 Serial.print("V ");
// Serial.print(getVoltage());
// Serial.print("V ");
// Serial.print("\t Battery Level: ");
// Serial.print(getLevel());
// Serial.print("% ");

/*
  if(PMSerial.find(0x42)){    
    PMSerial.readBytes(buf,LENG);

    if(buf[0] == 0x4d){
      if(checkValue(buf,LENG)){
        PM01Value=transmitPM01(buf); //count PM1.0 value of the air detector module
        PM2_5Value=transmitPM2_5(buf);//count PM2.5 value of the air detector module
        PM10Value=transmitPM10(buf); //count PM10 value of the air detector module 
      }           
    } 
  }

  static unsigned long OledTimer=millis();  
    if (millis() - OledTimer >=1000) 
    {
      OledTimer=millis(); 
      
      Serial.print("PM1.0: ");  
      Serial.print(PM01Value);
      Serial.println("  ug/m3");            
    
      Serial.print("PM2.5: ");  
      Serial.print(PM2_5Value);
      Serial.println("  ug/m3");     
      
      Serial.print("PM1 0: ");  
      Serial.print(PM10Value);
      Serial.println("  ug/m3");   
      Serial.println();
    }
*/

 Serial.print("\n");
 Serial.print("Sending data to MRWATT ThingSpeak Account...");
 Serial.print("\n");

 // Write to ThingSpeak
   ThingSpeak.setField(1,temperature);
   ThingSpeak.setField(2,humidity);
   ThingSpeak.setField(3,dewpt);
   ThingSpeak.setField(4,currentPressurehPA);
   ThingSpeak.setField(5,ppm);
   ThingSpeak.setField(6,wetBulbT);
   ThingSpeak.setField(7,sensorVoltage);
   ThingSpeak.setField(8,BatteryVoltage);
//   ThingSpeak.setField(7,PM2_5Value);
//   ThingSpeak.setField(8,PM10Value);

       int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
       if(x == 200){
             Serial.println("Channel update successful.");
       }
        else{
         Serial.println("Problem updating channel. HTTP error code " + String(x));
       } 
  
  // digitalWrite(LED_BUILTIN, HIGH); // Turn LED on.
  // delay(10000);
  // digitalWrite(LED_BUILTIN, LOW); // Turn LED off.
   Serial.print("-------------------------------------------------------------------------------------------------"); 
   Serial.print("\n");
  
// draw("Temperature (Celsius) ...", TEMPERATURE, int(currentTemperature));
if ( hour() > 19 && hour() < 6 ) {draw("Temperature (Celsius) ...", TEMPERATURENIGHT, int(currentTemperature));}
if ( hour() > 6 && hour() < 19 ) {draw("Temperature (Celsius) ...", TEMPERATUREDAY, int(currentTemperature));}

draw("Humidity (%) ...", HUMIDITY, int(humidity));
draw("Pressure (hPa) ...", PRESSURE, int(currentPressurehPA));
 if ( correctedPPM > 1 && correctedPPM < 700 ) {draw("Air Quality (PPM)... >>>EXCELLENT<<<", AIRQ, int(ppm));}
 if ( correctedPPM > 700 && correctedPPM < 800 ) {draw("Air Quality (PPM)... >>>GOOD<<<", AIRQ, int(ppm));}
 if ( correctedPPM > 800 && correctedPPM < 1000 ) {draw("Air Quality (PPM)... >>>FAIR<<<", AIRQ, int(ppm));}
 if ( correctedPPM > 1000 && correctedPPM < 1500 ) {draw("Air Quality (PPM)... >>>!MEDIOCRE!<<<", AIRQ, int(ppm));}
 if ( correctedPPM > 1500 && correctedPPM < 2000 ) {draw("Air Quality (PPM)... >>>!!BAD!!<<<", AIRQ, int(ppm));}
 if ( correctedPPM > 2000 && correctedPPM < 50000 ) {draw("Air Quality (PPM)... >>>!!!TOXIC!!!<<<", AIRQ, int(ppm));}
 if ( correctedPPM > 50000 || correctedPPM == 0 ) {draw("Air Quality (PPM)... >>>!PREHEAT MQ135!<<<", AIRQ, int(ppm));}
draw("Air Quality (PPM) ...", AIRQ, int(ppm));
draw("Dew Point (Celsius) ...", DEWPT, int(dewpt));

 
//delay(1000); // delay one second before OLED display update
u8g2.sendBuffer();          // transfer internal memory to the display
//}

  Serial.print("\n Power saving mode and wait 5 minutes");
   u8g2.sleepOn();
   ESP.deepSleep(actSleepTime);
   client.stop();
   
   delay(30000);
}


// This is section the function that the interrupt calls to increment the rotation count
//-------------------------------------------------------------------------------------------------------------
////////////////////////////////////FUNCTIONS//////////////////////////////////////////////////////////////
//-------------------------------------------------------------------------------------------------------------


// Functions

// Print Wifi Status

void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}


  float getVoltage(){
  float raw = analogRead(A0);                      
  float volt = map(raw, 140, 227, 338, 511);             // Avec une résistance 1M5 - With a 1M5 resistor
  volt = volt / 100;
 // Serial.print("\tA0 "); Serial.println(raw);
  Serial.print("\tVoltage "); Serial.print(volt);
  return volt;
}


/*
float getLevel(){
  float raw = analogRead(A0);
  int level = map(raw, 140, 227, 0, 100);                // Avec une résistance 1M5 - With a 1M5 resistor
  if ( level < 0 ) { level = 0; }
  if ( level > 100 ) { level = 100; }
  Serial.print("Level: "); Serial.println(level);
  return level;
}
*/

void drawLogo(void)
{
  uint8_t mdy = 0;
  if ( u8g2.getDisplayHeight() < 59 )
    mdy = 5;
 
    u8g2.setFontMode(1);  // Transparent
    u8g2.setDrawColor(1);
#ifdef MINI_LOGO

    u8g2.setFontDirection(0);
    u8g2.setFont(u8g2_font_inb16_mf);
    u8g2.drawStr(0, 22, "M");
    u8g2.drawStr(14, 22, "R");
    u8g2.drawStr(28, 22, " ");
    u8g2.drawStr(42, 22, "W");
    u8g2.drawStr(56, 22, "A");
    u8g2.drawStr(70, 22, "T");
    u8g2.drawStr(84, 22, "T");
    u8g2.drawHLine(2, 22, 109);
    u8g2.drawHLine(3, 22, 109);
    u8g2.drawVLine(107, 22, 12);
    u8g2.drawVLine(108, 22, 12);
#else

    u8g2.setFontDirection(0);
    u8g2.setFont(u8g2_font_inb24_mf);
    u8g2.drawStr(0, 30-mdy, "M");
    u8g2.drawStr(14, 30-mdy, "R");
    u8g2.drawStr(28, 30-mdy, " ");
    u8g2.drawStr(42, 30-mdy, "W");
    u8g2.drawStr(56, 30-mdy, "A");
    u8g2.drawStr(70, 30-mdy, "T");
    u8g2.drawStr(84, 30-mdy, "T");
    u8g2.drawHLine(2, 35-mdy, 109);
    u8g2.drawHLine(3, 36-mdy, 109);
    u8g2.drawVLine(107, 32-mdy, 12);
    u8g2.drawVLine(108, 33-mdy, 12);
    
#endif
}

void drawURL(void)
{
  String oledIPADDR;
  String oledWiFiSSID;
  oledIPADDR = WiFi.localIP().toString();
  oledWiFiSSID = WiFi.SSID();
#ifndef MINI_LOGO
  u8g2.setFont(u8g2_font_4x6_tr);
  //u8g2.setDrawColor(2);

  if ( u8g2.getDisplayHeight() < 59 )
  {
    u8g2.drawStr(89,20-5,"https://www.mrwatt.eu");
    u8g2.drawStr(73,29-5,"/");
  }
  else
  {
    u8g2.drawStr(1, 50, "https://www.mrwatt.eu");
    u8g2.drawStr(1, 57, "IP Address:");
    u8g2.drawStr(49, 57, oledIPADDR.c_str());
    u8g2.drawStr(1, 64, "SSID:");
    u8g2.drawStr(35, 64, oledWiFiSSID.c_str());
  }
#endif
}
// Dew Point function
  double dewPoint(double temperature, double humidity) //Calculate dew Point
{
  double A0= 373.15/(273.15 + temperature);
  double SUM = -7.90298 * (A0-1);
  SUM += 5.02808 * log10(A0);
  SUM += -1.3816e-7 * (pow(10, (11.344*(1-1/A0)))-1) ;
  SUM += 8.1328e-3 * (pow(10,(-3.49149*(A0-1)))-1) ;
  SUM += log10(1013.246);
  double VP = pow(10, SUM-3) * humidity;
  double T = log(VP/0.61078);   
  return (241.88 * T) / (17.558-T);
}

void drawWeatherSymbol(u8g2_uint_t x, u8g2_uint_t y, uint8_t symbol)
{
  // fonts used:
  // u8g2_font_open_iconic_embedded_6x_t
  // u8g2_font_open_iconic_weather_6x_t
  // encoding values, see: https://github.com/olikraus/u8g2/wiki/fntgrpiconic
  
  switch(symbol)
  {
    case SUN:
      u8g2.setFont(u8g2_font_open_iconic_weather_6x_t);
      u8g2.drawGlyph(x, y, 69); 
      break;
 
    case SUN_CLOUD:
      u8g2.setFont(u8g2_font_open_iconic_weather_6x_t);
      u8g2.drawGlyph(x, y, 65); 
      break;
 
    case CLOUD:
      u8g2.setFont(u8g2_font_open_iconic_weather_6x_t);
      u8g2.drawGlyph(x, y, 64); 
      break;
  
    case RAIN:
      u8g2.setFont(u8g2_font_open_iconic_weather_6x_t);
      u8g2.drawGlyph(x, y, 67); 
      break;
   
    case THUNDER:
      u8g2.setFont(u8g2_font_open_iconic_embedded_6x_t);
      u8g2.drawGlyph(x, y, 67);
      break;    
    
    case TEMPERATUREDAY:
      u8g2.setFont(u8g2_font_open_iconic_weather_6x_t);  
      u8g2.drawGlyph(x, y, 69);
      break;
    
    case TEMPERATURENIGHT:
      u8g2.setFont(u8g2_font_open_iconic_weather_6x_t);  
      u8g2.drawGlyph(x, y, 66);
      break;

    case HUMIDITY:
      u8g2.setFont(u8g2_font_open_iconic_thing_6x_t);  
      u8g2.drawGlyph(x, y, 72);
      break;
    
    case PRESSURE:
      u8g2.setFont(u8g2_font_open_iconic_weather_6x_t);  
      u8g2.drawGlyph(x, y, 67);
      break;
    
    case AIRQ:
      u8g2.setFont(u8g2_font_open_iconic_weather_6x_t);  
      u8g2.drawGlyph(x, y, 64);
      break;
    
    case DEWPT:
      u8g2.setFont(u8g2_font_open_iconic_www_6x_t);  
      u8g2.drawGlyph(x, y, 78);
      break;
       
  }
  
}



void drawWeather(uint8_t symbol, int degree)
{
  drawWeatherSymbol(0, 48, symbol);
  u8g2.setFont(u8g2_font_logisoso32_tf);
  u8g2.setCursor(48+3, 42);
  switch(symbol)
  {
   case TEMPERATUREDAY:
      u8g2.print(degree);
      u8g2.print("°C");   // requires enableUTF8Print()
      break;
   case TEMPERATURENIGHT:
      u8g2.print(degree);
      u8g2.print("°C");   // requires enableUTF8Print()
      break;    
   case HUMIDITY:
      u8g2.print(degree);
      u8g2.print("%");   // requires enableUTF8Print()
      break;
   case PRESSURE:
      u8g2.setCursor(45+3, 40);
      u8g2.print(degree);
      u8g2.print("");  
      break;
    case AIRQ:
      u8g2.print(degree);
      u8g2.print("");  
      break;
    case DEWPT:
      u8g2.print(degree);
      u8g2.print("°C");   // requires enableUTF8Print()
      break;
  }
}

/*
  Draw a string with specified pixel offset. 
  The offset can be negative.
  Limitation: The monochrome font with 8 pixel per glyph
*/

void drawScrollString(int16_t offset, const char *s)
{
  static char buf[36];  // should for screen with up to 256 pixel width 
  size_t len;
  size_t char_offset = 0;
  u8g2_uint_t dx = 0;
  size_t visible = 0;
  len = strlen(s);
  if ( offset < 0 )
  {
    char_offset = (-offset)/8;
    dx = offset + char_offset*8;
    if ( char_offset >= u8g2.getDisplayWidth()/8 )
      return;
    visible = u8g2.getDisplayWidth()/8-char_offset+1;
    strncpy(buf, s, visible);
    buf[visible] = '\0';
    u8g2.setFont(u8g2_font_8x13_mf);
    u8g2.drawStr(char_offset*8-dx, 62, buf);
  }
  else
  {
    char_offset = offset / 8;
    if ( char_offset >= len )
      return; // nothing visible
    dx = offset - char_offset*8;
    visible = len - char_offset;
    if ( visible > u8g2.getDisplayWidth()/8+1 )
      visible = u8g2.getDisplayWidth()/8+1;
    strncpy(buf, s+char_offset, visible);
    buf[visible] = '\0';
    u8g2.setFont(u8g2_font_8x13_mf);
    u8g2.drawStr(-dx, 62, buf);
  }
}

void draw(const char *s, uint8_t symbol, int degree)
{
  int16_t offset = -(int16_t)u8g2.getDisplayWidth();
  int16_t len = strlen(s);
  for(;;)
  {
    u8g2.firstPage();
    do {
      drawWeather(symbol, degree);
      drawScrollString(offset, s);
    } while ( u8g2.nextPage() );
    delay(20);
    offset+=2;
    if ( offset > len*8+1 )
      break;
  }
}


char checkValue(unsigned char *thebuf, char leng)
{  
  char receiveflag=0;
  int receiveSum=0;

  for(int i=0; i<(leng-2); i++){
  receiveSum=receiveSum+thebuf[i];
  }
  receiveSum=receiveSum + 0x42;
 
  if(receiveSum == ((thebuf[leng-2]<<8)+thebuf[leng-1]))  //check the serial data 
  {
    receiveSum = 0;
    receiveflag = 1;
  }
  return receiveflag;
}

// Wet Bulb function
   double wetBulb(double temperature, double humidity, float pressure) // Calculate wet-Bulb
 {

#define A 0.539126
#define B 0.1047837
#define C -0.0007493556
#define D -0.001077432
#define E 0.006414631
#define F -5.151526

float TWET = A*temperature; 
TWET += B*humidity; 
TWET += C*(pow(temperature,2)); 
TWET += D*(pow(humidity,2)); 
TWET += E*(temperature*humidity); 
TWET += F;

return TWET;
 }

/*
int transmitPM01(unsigned char *thebuf)
{
  int PM01Val;
  PM01Val=((thebuf[3]<<8) + thebuf[4]); //count PM1.0 value of the air detector module
  return PM01Val;
}

//transmit PM Value to PC
int transmitPM2_5(unsigned char *thebuf)
{
  int PM2_5Val;
  PM2_5Val=((thebuf[5]<<8) + thebuf[6]);//count PM2.5 value of the air detector module
  return PM2_5Val;
  }

//transmit PM Value to PC
int transmitPM10(unsigned char *thebuf)
{
  int PM10Val;
  PM10Val=((thebuf[7]<<8) + thebuf[8]); //count PM10 value of the air detector module  
  return PM10Val;
}
*/

// Funzione per calcolare l'altitudine in base alla pressione data dal BMP180
float getAltitude(float press, float temp) {
 const float sea_press = 1013.25;
 return ((pow((sea_press / press), 1/5.257) - 1.0) * (temp + 273.15)) / 0.0065;
}

///////////////////////////////////////////////////////////////////////////
// Time functions
////////////////////////////////////////////////////////////////////////////

// send an NTP request to the time server at the given address
unsigned long sendNTPpacket(IPAddress& address)
{
  Serial.println("sending NTP packet...");
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  udp.beginPacket(address, 123); //NTP requests are to port 123
  udp.write(packetBuffer, NTP_PACKET_SIZE);
  udp.endPacket();
}

/////////////////////////////////////////////////////////////////////////////////////


void setDateTime()
{

  unsigned long epoch;

  Serial.print("\t .....Updating Time.....");

 // display.clear();
 // display.drawString(0, 0,  timeStr);
 // display.drawString(0, 19, dateStr);
 // display.drawString(0, 40, "-Updating Time-");
 // display.display();

  lastMillis = currentMillis;

  //get a random server from the pool
  WiFi.hostByName(ntpServerName, timeServerIP);

  sendNTPpacket(timeServerIP); // send an NTP packet to a time server
  // wait to see if a reply is available
  delay(1000);

  int cb = udp.parsePacket();
  if (!cb) {
    Serial.println("\t no packet yet");
  }
  else {
    Serial.print("packet received, length=");
    Serial.println(cb);
    // We've received a packet, read the data from it
    udp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer

    //the timestamp starts at byte 40 of the received packet and is four bytes,
    // or two words, long. First, esxtract the two words:

    unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
    unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
    // combine the four bytes (two words) into a long integer
    // this is NTP time (seconds since Jan 1 1900):
    secsSince1900 = highWord << 16 | lowWord;

  }
  //Serial.print("secsSince1900: ");
  //Serial.println(secsSince1900);
  const unsigned long seventyYears = 2208988800UL;
  // subtract seventy years:
  epoch = secsSince1900 - seventyYears;

  epoch = epoch + (int)(UTCoffset * 3600);
  if (daylightSavings)
  {
    epoch = epoch + 3600;
  }

  setTime(epoch);

}

/////////////////////////////////////////
//        HTML functions
////////////////////////////////////////



String getDropDown()
{
  String webString = "";
  webString += "<select name=\"timezone\">\n";
  webString += "   <option value=\"-12\" ";
  if (UTCoffset == -12)
    webString += " selected=\"seleted\" ";
  webString += ">(GMT -12:00) Eniwetok, Kwajalein</option>\n";

  webString += "   <option value=\"-11\" ";
  if (UTCoffset == -11)
    webString += " selected=\"seleted\" ";
  webString += ">(GMT -11:00) Midway Island, Samoa</option>\n";

  webString += "   <option value=\"-10\" ";
  if (UTCoffset == -10)
    webString += " selected=\"seleted\" ";
  webString += ">(GMT -10:00) Hawaii</option>\n";

  webString += "   <option value=\"-9\" ";
  if (UTCoffset == -9)
    webString += " selected=\"seleted\" ";
  webString += ">(GMT -9:00) Alaska</option>\n";

  webString += "   <option value=\"-8\" ";
  if (UTCoffset == -8)
    webString += " selected=\"seleted\" ";
  webString += ">(GMT -8:00) Pacific Time (US &amp; Canada)</option>\n";

  webString += "   <option value=\"-7\" ";
  if (UTCoffset == -7)
    webString += " selected=\"seleted\" ";
  webString += ">(GMT -7:00) Mountain Time (US &amp; Canada)</option>\n";

  webString += "   <option value=\"-6\" ";
  if (UTCoffset == -6)
    webString += " selected=\"seleted\" ";
  webString += ">(GMT -6:00) Central Time (US &amp; Canada), Mexico City</option>\n";

  webString += "   <option value=\"-5\" ";
  if (UTCoffset == -5)
    webString += " selected=\"seleted\" ";
  webString += ">(GMT -5:00) Eastern Time (US &amp; Canada), Bogota, Lima</option>\n";

  webString += "   <option value=\"-4.5\" ";
  if (UTCoffset == -4.5)
    webString += " selected=\"seleted\" ";
  webString += ">(GMT -4:30) Caracas</option>\n";

  webString += "   <option value=\"-4\" ";
  if (UTCoffset == -4)
    webString += " selected=\"seleted\" ";
  webString += ">(GMT -4:00) Atlantic Time (Canada), La Paz, Santiago</option>\n";

  webString += "   <option value=\"-3.5\" ";
  if (UTCoffset == -3.5)
    webString += " selected=\"seleted\" ";
  webString += ">(GMT -3:30) Newfoundland</option>\n";

  webString += "   <option value=\"-3\" ";
  if (UTCoffset == -3)
    webString += " selected=\"seleted\" ";
  webString += ">(GMT -3:00) Brazil, Buenos Aires, Georgetown</option>\n";

  webString += "   <option value=\"-2\" ";
  if (UTCoffset == -2)
    webString += " selected=\"seleted\" ";
  webString += ">(GMT -2:00) Mid-Atlantic</option>\n";

  webString += "   <option value=\"-1\" ";
  if (UTCoffset == -1)
    webString += " selected=\"seleted\" ";
  webString += ">(GMT -1:00 hour) Azores, Cape Verde Islands</option>\n";

  webString += "   <option value=\"0\" ";
  if (UTCoffset == 0)
    webString += " selected=\"seleted\" ";
  webString += ">(GMT) Western Europe Time, London, Lisbon, Casablanca, Greenwich</option>\n";

  webString += "   <option value=\"1\" ";
  if (UTCoffset == 1)
    webString += " selected=\"seleted\" ";
  webString += ">(GMT +1:00 hour) Brussels, Copenhagen, Madrid, Paris</option>\n";

  webString += "   <option value=\"2\" ";
  if (UTCoffset == 2)
    webString += " selected=\"seleted\" ";
  webString += ">(GMT +2:00) Kaliningrad, South Africa, Cairo</option>\n";

  webString += "   <option value=\"3\" ";
  if (UTCoffset == 3)
    webString += " selected=\"seleted\" ";
  webString += ">(GMT +3:00) Baghdad, Riyadh, Moscow, St. Petersburg</option>\n";

  webString += "   <option value=\"3.5\" ";
  if (UTCoffset == 3.5)
    webString += " selected=\"seleted\" ";
  webString += ">(GMT +3:30) Tehran</option>\n";

  webString += "   <option value=\"4\" ";
  if (UTCoffset == 4)
    webString += " selected=\"seleted\" ";
  webString += ">(GMT +4:00) Abu Dhabi, Muscat, Yerevan, Baku, Tbilisi</option>\n";

  webString += "   <option value=\"4.5\" ";
  if (UTCoffset == 4.5)
    webString += " selected=\"seleted\" ";
  webString += ">(GMT +4:30) Kabul</option>\n";

  webString += "   <option value=\"5\" ";
  if (UTCoffset == 5)
    webString += " selected=\"seleted\" ";
  webString += ">(GMT +5:00) Ekaterinburg, Islamabad, Karachi, Tashkent</option>\n";

  webString += "   <option value=\"5.5\" ";
  if (UTCoffset == 5.5)
    webString += " selected=\"seleted\" ";
  webString += ">(GMT +5:30) Mumbai, Kolkata, Chennai, New Delhi</option>\n";

  webString += "   <option value=\"5.75\" ";
  if (UTCoffset == 5.75)
    webString += " selected=\"seleted\" ";
  webString += ">(GMT +5:45) Kathmandu</option>\n";

  webString += "   <option value=\"6\" ";
  if (UTCoffset == 6)
    webString += " selected=\"seleted\" ";
  webString += ">(GMT +6:00) Almaty, Dhaka, Colombo</option>\n";

  webString += "   <option value=\"6.5\" ";
  if (UTCoffset == 6.5)
    webString += " selected=\"seleted\" ";
  webString += ">(GMT +6:30) Yangon, Cocos Islands</option>\n";

  webString += "   <option value=\"7\" ";
  if (UTCoffset == 7)
    webString += " selected=\"seleted\" ";
  webString += ">(GMT +7:00) Bangkok, Hanoi, Jakarta</option>\n";

  webString += "   <option value=\"8\" ";
  if (UTCoffset == 8)
    webString += " selected=\"seleted\" ";
  webString += ">(GMT +8:00) Beijing, Perth, Singapore, Hong Kong</option>\n";

  webString += "   <option value=\"9\" ";
  if (UTCoffset == 9)
    webString += " selected=\"seleted\" ";
  webString += ">(GMT +9:00) Tokyo, Seoul, Osaka, Sapporo, Yakutsk</option>\n";

  webString += "   <option value=\"9.5\" ";
  if (UTCoffset == 9.5)
    webString += " selected=\"seleted\" ";
  webString += ">(GMT +9:30) Adelaide, Darwin</option>\n";

  webString += "   <option value=\"10\" ";
  if (UTCoffset == 10)
    webString += " selected=\"seleted\" ";
  webString += ">(GMT +10:00) Eastern Australia, Guam, Vladivostok</option>\n";

  webString += "   <option value=\"11\" ";
  if (UTCoffset == 11)
    webString += " selected=\"seleted\" ";
  webString += ">(GMT +11:00) Magadan, Solomon Islands, New Caledonia</option>\n";

  webString += "   <option value=\"12\" ";
  if (UTCoffset == 12)
    webString += " selected=\"seleted\" ";
  webString += ">(GMT +12:00) Auckland, Wellington, Fiji, Kamchatka</option>\n";
  webString += "  </select>\n";

  return webString;
}

////////////////////////////////////////////////////////////////////

String getAJAXcode()
{

  String webStr = "";
  webStr += "<script src=\"http://ajax.googleapis.com/ajax/libs/jquery/1.10.2/jquery.min.js\"></script> \n";
  webStr += "<script>\n";

  webStr += " function loadTime() { \n";
  webStr += " $(\"#timeDiv\").load(\"http://" + WiFi.localIP().toString() + "/time\"); \n";
  webStr += "} \n\n";

  webStr += " setInterval(loadTime, 1000); \n"; // every x milli seconds
  webStr += " loadTime(); \n";// on load

  webStr += " </script> \n";
  return webStr;
}

///////////////////////////////////////////////////////////////////////////////////////

String setHTML()
{

float temperaturehtml = dht.readTemperature();
 float humidityhtml = dht.readHumidity();

  
  String webString = "<html><head>\n";
  //  webString += "<meta http-equiv=\"refresh\" content=\"30;url=http://" + WiFi.localIP().toString() + "\"> \n";
  webString += getAJAXcode();
  webString += "</head><body>\n";
  webString += "<form action=\"http://" + WiFi.localIP().toString() + "/submit\" method=\"POST\">";
  webString += "<h1>MR WATT WEATHER STATION Clock IoT </h1>\n";
  webString += "<div style=\"color:red\">" + webMessage + "</div>\n";

  webString += "<div id=\"timeDiv\" style=\"color:blue; font-size:24px; font-family:'Comic Sans MS'\">" + dateStr + " &nbsp; &nbsp; &nbsp; " + timeStr + "</div>\n";

  webString += "<br><br>" + getDropDown();

  webString += "<br><table style=\"width:400px;\"><tr>";
  webString += "<td style=\"text-align:right\">";
  webString += "<div>Time Server Update Interval: </div>\n";
  webString += "<div>Daylight Savings: </div>\n";
  webString += "<div>24 hour: </div>\n";
  webString += "</td>";
  webString += "<td>";
  webString += " <input type='text' value='" + String(interval / 1000) + "' name='interval' maxlength='10' size='4'> (secs)<br>\n";

  if (daylightSavings)
    webString += " <input type='checkbox' checked name='daySave' value='" +  String(daylightSavings) + "'/>";
  else
    webString += " <input type='checkbox' name='daySave' value='" +  String(daylightSavings) + "'  />";


  if (hourTime)
    webString += "<br><input type='checkbox' checked name='24hour' value='" +  String(hourTime) + "'/>";
  else
    webString += "<br><input type='checkbox' name='24hour' value='" +  String(hourTime) + "'  />";

  webString += " <input type='submit' value='Submit' >\n";

  webString += "</td></tr></table>\n";

  webString += "<div><a href=\"/\">Refresh</a></div> \n";
 // webString += "<table style=\"width:100%\"><tr><tr><th>Firstname</th><th>Lastname</th><th>Age</th></tr><tr><td>Jill</td><td>Smith</td><td>50</td></tr><tr><td>Eve</td><td>Jackson</td><td>94</td></tr></table>\n";
  webString +="<!DOCTYPE html><html><head><style>table\ {\ font-family:\ arial,\ sans-serif;\ border-collapse:\ collapse;\ width:\ 100%;\ }\ td,\ th\ {\ border:\ 1px\ solid\ #dddddd;\ text-align:\ left;\ padding:\ 8px;\ }\ tr:nth-child(even)\ {\ background-color:\ #dddddd;\ }</style></head><body><h2>WEATHER STATION SUMMARY</h2><table><tr><th>SENSOR</th><th>VALUE</th></tr><tr><td>Temperature (C)</td><td>" + String((float)temperaturehtml,1) + "</td></tr><tr><td>Humidity (%)</td><td>" + String((float)humidityhtml,1) + "</td></tr><tr><td>Dew Point (C)</td><td></td></tr><tr><td>Pressure (hPa)</td><td></td></tr><tr><td>BMP180 Temp. (C)</td><td></td></tr><tr><td>Wet Bulb Temp. (C)</td><td></td></tr><tr><td>Air Quality (PPM)</td><td></td></tr></table></body></html>\n";


  webString += "</form></body></html>\n";
  

  return webString;
}



/////////////////////////////////////////////////////////////
//   File functions
/////////////////////////////////////////////////////////////

void updateProperties()
{
  File f = SPIFFS.open(fName, "w");
  if (!f) {

    Serial.println("file open for properties failed");
  }
  else
  {
    Serial.println("====== Updating to properties file =========");
    f.print(UTCoffset); f.print( ","); f.print(daylightSavings);
    f.print("~"); f.print(interval);
    f.print(":"); f.println(hourTime);

    Serial.println("Properties file updated");

    f.close();
  }
}

/////////////////////////////////////////

void initPropFile()
{

  SPIFFS.begin();
  delay(10);
  /////////////////////////////////////////////////////////
  // SPIFFS.format(); // uncomment to completely clear data
  // return;
  ///////////////////////////////////////////////////////////
  File f = SPIFFS.open(fName, "r");

  if (!f) {

    Serial.println("Please wait 30 secs for SPIFFS to be formatted");
    SPIFFS.format();
    Serial.println("Spiffs formatted");
    updateProperties();
  }
  else
  {
    Serial.println("Properties file exists. Reading.");

    while (f.available()) {

      //Lets read line by line from the file
      String str = f.readStringUntil('\n');

      String offsetStr = str.substring(0, str.indexOf(",")  );
      String dSavStr = str.substring(str.indexOf(",") + 1, str.indexOf("~") );
      String intervalStr = str.substring(str.indexOf("~") + 1, str.indexOf(":") );
      String hourStr = str.substring(str.indexOf(":") + 1 );

      UTCoffset = offsetStr.toFloat();
      daylightSavings = dSavStr.toInt();
      interval = intervalStr.toInt();
      hourTime = hourStr.toInt();

    }

    f.close();
  }

}

//////////////////////////////////////////////////////////
// used to error check the text box input
/////////////////////////////////////////////////////////

boolean isValidNumber(String str) {
  for (byte i = 0; i < str.length(); i++)
  {
    if (isDigit(str.charAt(i))) return true;
  }
  return false;
}

//////////////////////////////////////////////////////////////////////////////////////
/// client handlers
//////////////////////////////////////////////////////////////////////////////////////

void handle_submit() {

  webMessage = "";
  daylightSavings = true;
  hourTime = false;

  if (server.args() > 0 ) {
    for ( uint8_t i = 0; i < server.args(); i++ ) {

      // can be useful to determine the values from a form post.
      //  webMessage += "<br>Server arg " + server.arg(i);
      //  webMessage += "<br>Server argName " + server.argName(i);

      if (server.argName(i) == "daySave") {
        // checkbox is checked
        daylightSavings = true;
      }

      if (server.argName(i) == "24hour") {
        // checkbox is checked
        hourTime = true;
      }

      if (server.argName(i) == "interval") {

        if (isValidNumber(server.arg(i)) ) // error checking to make sure we have a number
        {
          interval = server.arg(i).toInt() * 1000;
        }
        else
        {
          webMessage = "Interval must be a valid number";
        }

      }

      if (server.argName(i) == "timezone") {

        UTCoffset = server.arg(i).toFloat();
      }

    }
  }

  if (webMessage == "")
  {
    updateProperties();
    webMessage = "Settings Updated";
  }

  setDateTime();

  String webString = setHTML();
  server.send(200, "text/html", webString);            // send to someones browser when asked

}


//////////////////////////////////////////////////////////////////////////////////////

void handle_time() // this function handles the AJAX call
{
  server.send(200, "text/html", dateStr + " &nbsp; &nbsp; &nbsp; " + timeStr);
}

////////////////////////////////////////////////////////////////////////////////////////

void handle_root() {

  webMessage = "";
  String webString = setHTML();
  server.send(200, "text/html", webString);            // send to someones browser when asked

}

/////////// WPS WiFI Functions //////////////////////////////////////////////////////////
bool startWPSPBC() {
  Serial.println("WPS config start");
  // WPS works in STA (Station mode) only -> not working in WIFI_AP_STA !!! 
  WiFi.mode(WIFI_STA);
  delay(1000);
  WiFi.begin("foobar",""); // make a failed connection
  while (WiFi.status() == WL_DISCONNECTED) {
    delay(500);
    Serial.print(".");
  }

  bool wpsSuccess = WiFi.beginWPSConfig();
  if(wpsSuccess) {
      // Well this means not always success :-/ in case of a timeout we have an empty ssid
      String newSSID = WiFi.SSID();
      if(newSSID.length() > 0) {
        // WPSConfig has already connected in STA mode successfully to the new station. 
        Serial.printf("WPS finished. Connected successfull to SSID '%s'", newSSID.c_str());
        // save to config and use next time or just use - WiFi.begin(WiFi.SSID().c_str(),WiFi.psk().c_str());
        //qConfig.wifiSSID = newSSID;
        //qConfig.wifiPWD = WiFi.psk();
        //saveConfig();
      } else {
        wpsSuccess = false;
      }
  }
  return wpsSuccess; 
}


////////////////////////////////////////////////////////////////////////////////////////
