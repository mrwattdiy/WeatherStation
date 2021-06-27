/*
 MR WATT - CECCHETTI SIMONE v.1.0 202106271846
 WEATHER STATION - Temperature, Humidity, Pressure, Altitude, Air Quality, Dew Point, Wet Bulb
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
//#include <ESP8266WebServer.h>
#include <WiFiUdp.h>
#include <WiFiClientSecure.h>
#include <ThingSpeak.h>
#include <Adafruit_BMP085.h>
#include <DHT.h>
#include <MQ135.h>
#include <stdio.h>
#include <Wire.h>
#include <SoftwareSerial.h>
#include <Arduino.h>
#include <U8g2lib.h>
#include <ctype.h> // for isNumber check
#include "Time.h"
#include "TimeLib.h"
#include "secrets.h"

int pin_switch = D3;

#define FREQUENCY 80
extern "C" {
#include "user_interface.h"
}

#define TS_ENABLE_SSL // For HTTPS SSL connection
// Fingerprint check, make sure that the certificate has not expired.
const char * fingerprint = NULL; // use SECRET_SHA1_FINGERPRINT for fingerprint check

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

// ThingSpeak Settings
unsigned long myChannelNumber = SECRET_CH_ID_WEATHER_STATION; //Your ThingSpeak Channel Number 
const char * myWriteAPIKey = SECRET_WRITE_APIKEY;
const char * myReadAPIKey = SECRET_READ_APIKEY;
unsigned long lastMillis = 0;
unsigned long currentMillis = 0;
float dewpt=0;          // dew point temp


WiFiClientSecure  clientsecure;
WiFiClient client;

// For NTP Syncronization
// NTP Servers:
static const char ntpServerName[] = "time.inrim.it";
//static const char ntpServerName[] = "time.nist.gov";
//static const char ntpServerName[] = "time-a.timefreq.bldrdoc.gov";
//static const char ntpServerName[] = "time-b.timefreq.bldrdoc.gov";
//static const char ntpServerName[] = "time-c.timefreq.bldrdoc.gov";

const int timeZone = 2;     // Central European Time
//const int timeZone = -5;  // Eastern Standard Time (USA)
//const int timeZone = -4;  // Eastern Daylight Time (USA)
//const int timeZone = -8;  // Pacific Standard Time (USA)
//const int timeZone = -7;  // Pacific Daylight Time (USA)

WiFiUDP Udp;
unsigned int localPort = 8888;  // local port to listen for UDP packets

time_t getNtpTime();
void digitalClockDisplay();
void printDigits(int digits);
void sendNTPpacket(IPAddress &address);

//Initialize DHT22 Sensor
#define DHTPIN 2     // what digital pin we're connected to - Wemos D1 R2 mini pin D4

// Uncomment whatever type you're using!
//#define DHTTYPE DHT11   // DHT 11
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
//#define DHTTYPE DHT21   // DHT 21 (AM2301)

// Connect pin 1 (on the left) of the sensor to +5V
// NOTE: If using a board with 3.3V logic like an Arduino Due connect pin 1
// to 3.3V instead of 5V!
// Connect pin 2 of the sensor to whatever your DHTPIN is ( Wemos D4 )
// Connect pin 4 (on the right) of the sensor to GROUND
// Connect a 10K resistor from pin 2 (data) to pin 1 (power) of the sensor

DHT dht(DHTPIN, DHTTYPE);

// Initialize MQ135 Sensor
#define PIN_MQ135 A0
MQ135 mq135_sensor = MQ135(PIN_MQ135);


//Initialize BMP180 Barometer Sensor
Adafruit_BMP085 bmp;

// Settaggi Barometro BMP180 
// Valore calcolato in base all'altitudine : http://www.calctool.org/CALC/phys/default/pres_at_alt ho utilizzato il valore medio
float seaLevelPressure = 101325;
float realAltitude = ALTITUDE;
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

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println();
  Serial.print("Sketch:   ");   Serial.println(__FILE__);
  Serial.print("Uploaded: ");   Serial.println(__DATE__);
  Serial.println(" ");
  Serial.println("WEATHER STATION - Temperature, Pressure, Humidity, Air Quality using Sensors DHT22, MQ135 and BMP180 - Powered By MR WATT");
  Serial.println("BootStrap...");
  pinMode(A0, INPUT);

  // We start by connecting to a WiFi network
  Serial.print("Connecting to ");
  Serial.println(ssid);

  
   // Set Static IP Block details...
/*
  IPAddress ip(192, 168, 1, 200);
  IPAddress gateway(192, 168, 1, 1);
  IPAddress subnet(255, 255, 255, 0);
  IPAddress dns(8, 8, 8, 8);
 
  // Static IP Setup Info Here...
  WiFi.config(ip, gateway, subnet, dns); //If you need Internet Access You should Add DNS also...
*/  
  WiFi.begin(ssid, pass);
  WiFi.mode(WIFI_STA);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");

  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  Serial.println("Starting UDP");
  Udp.begin(localPort);
  Serial.print("Local port: ");
  Serial.println(Udp.localPort());
  Serial.println("waiting for sync...");
  setSyncProvider(getNtpTime);
  setSyncInterval(300);

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

 // We start the I2C on the Arduino for communication with the BMP180 sensor.
  Wire.begin();

// BMP180 Test 
  Serial.println("Initialize BMP180...");
if (!bmp.begin()) {
  Serial.println("Could not find a valid BMP085 sensor, check wiring!");
  while (1) {}
  }

  // DHT22 Test
  Serial.println("Initialize DHT22...");
  dht.begin();


 //  Serial.println("Initialize MQ135...");
 
  

// Initialize ThingSpeak
  Serial.println("Initialize ThingSpeak Channel...");

/*
 if(fingerprint!=NULL){
    client.setFingerprint(fingerprint);
  }
  else{
    client.setInsecure(); // To perform a simple SSL Encryption
  }
*/  
  ThingSpeak.begin(client);  // Initialize ThingSpeak

}

time_t prevDisplay = 0; // when the digital clock was displayed

void loop() {
 // Wait a few seconds between measurements. 
 delay(5000);

 const int deepSleepSecs = 1800;
 unsigned long int actSleepTime = deepSleepSecs * 1000000;
 float rzero = mq135_sensor.getRZero();
 float temperature = dht.readTemperature();
 float humidity = dht.readHumidity();
 float correctedRZero = mq135_sensor.getCorrectedRZero(temperature, humidity);
 float resistance = mq135_sensor.getResistance();
 float ppm = mq135_sensor.getPPM();
 float correctedPPM = mq135_sensor.getCorrectedPPM(temperature, humidity);
 long currentPressure = bmp.readPressure();
 //float currentAltitude = bmp.readAltitude();
 float currentAltitude = getAltitude(bmp.readPressure(), bmp.readTemperature(),realAltitude);
 float currentTemperature = bmp.readTemperature();
 //long currentSealevelPressure = bmp.readSealevelPressure();
 long currentPressurehPA = int(bmp.readPressure()/100);
 unsigned int raw=0;
  // Time to sleep (in seconds):
 const int sleepTimeS = 60;
 String oledTemp;
 String oledHum;
 String oledPressure;
 String oledPPM;
 String oledDew;
 float dewpt = 0;
 float wetBulbT = 0;
 
 oledTemp = currentTemperature;
 oledHum = humidity;
 oledPressure = currentPressurehPA;
 oledPPM = ppm;
 oledDew = dewpt;

 currentMillis = millis();

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

  if (timeStatus() != timeNotSet) {
    if (now() != prevDisplay) { //update the display only if time has changed
      prevDisplay = now();
      digitalClockDisplay();
    }
  }
   
  // Check if any reads failed and exit early (to try again).
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  // Compute heat index in Celsius (isFahreheit = false)
  float hic = dht.computeHeatIndex(temperature, humidity, false);

// Call dewPoint and wetBulb functions...
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
// Serial.print("\t Pressure Sealevel BMP180: ");
// Serial.print(currentSealevelPressure);
// Serial.print("Pa ");
// Serial.print("\n");
 Serial.print("\t Dew Point: ");
 Serial.print(dewpt);
 Serial.print("\n");
 Serial.print("\t Wet Bulb Temperature: ");
 Serial.print(wetBulbT);
 Serial.print("C ");
 Serial.print("\n");

 Serial.print("Sending data to MRWATT ThingSpeak Account...");
 Serial.print("\n");

 // Write to ThingSpeak
   ThingSpeak.setField(1, temperature);
   ThingSpeak.setField(2, humidity);
   ThingSpeak.setField(3, dewpt);
   ThingSpeak.setField(4, currentPressurehPA);
   ThingSpeak.setField(5, ppm);
   ThingSpeak.setField(6, wetBulbT);

   // write to the ThingSpeak channel
   int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
   if(x == 200){
    Serial.println("Channel update successful.");
   }
    else{
    Serial.println("Problem updating channel. HTTP error code " + String(x));
    Serial.print("\t ChannelNumber: ");
    Serial.print(myChannelNumber);
    Serial.print("\n");
    Serial.print("\t WriteAPIKEY: ");
    Serial.print(myWriteAPIKey);
    Serial.print("\n");
  }
  
   digitalWrite(LED_BUILTIN, HIGH); // Turn LED on.
   delay(5000);
   digitalWrite(LED_BUILTIN, LOW); // Turn LED off.
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

 //  u8g2.sleepOn();
 //  ESP.deepSleep(actSleepTime);
   client.stop();
   delay(5000);
}


// This is section of the functions
//-------------------------------------------------------------------------------------------------------------
////////////////////////////////////FUNCTIONS//////////////////////////////////////////////////////////////
//-------------------------------------------------------------------------------------------------------------



// Functions

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

void digitalClockDisplay()
{
  // digital clock display of the time
  Serial.print(hour());
  printDigits(minute());
  printDigits(second());
  Serial.print(" ");
  Serial.print(day());
  Serial.print(".");
  Serial.print(month());
  Serial.print(".");
  Serial.print(year());
  Serial.println();
}

void printDigits(int digits)
{
  // utility for digital clock display: prints preceding colon and leading 0
  Serial.print(":");
  if (digits < 10)
    Serial.print('0');
  Serial.print(digits);
}

/*-------- NTP code ----------*/

const int NTP_PACKET_SIZE = 48; // NTP time is in the first 48 bytes of message
byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming & outgoing packets

time_t getNtpTime()
{
  IPAddress ntpServerIP; // NTP server's ip address

  while (Udp.parsePacket() > 0) ; // discard any previously received packets
  Serial.println("Transmit NTP Request");
  // get a random server from the pool
  WiFi.hostByName(ntpServerName, ntpServerIP);
  Serial.print(ntpServerName);
  Serial.print(": ");
  Serial.println(ntpServerIP);
  sendNTPpacket(ntpServerIP);
  uint32_t beginWait = millis();
  while (millis() - beginWait < 1500) {
    int size = Udp.parsePacket();
    if (size >= NTP_PACKET_SIZE) {
      Serial.println("Receive NTP Response");
      Udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
      unsigned long secsSince1900;
      // convert four bytes starting at location 40 to a long integer
      secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
      secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
      secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
      secsSince1900 |= (unsigned long)packetBuffer[43];
      return secsSince1900 - 2208988800UL + timeZone * SECS_PER_HOUR;
    }
  }
  Serial.println("No NTP Response :-(");
  return 0; // return 0 if unable to get the time
}

// send an NTP request to the time server at the given address
void sendNTPpacket(IPAddress &address)
{
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12] = 49;
  packetBuffer[13] = 0x4E;
  packetBuffer[14] = 49;
  packetBuffer[15] = 52;
  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  Udp.beginPacket(address, 123); //NTP requests are to port 123
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}

// Funzione per calcolare la pressione a livello del mare dalla pressione atmosferica, temperatura forniti dal BMP180 e altezza attuale
// Calculates the sea-level pressure from the atmospheric pressure, temperature and altitude at the present location.
// Further details: https://keisan.casio.com/exec/system/1224575267

float getAltitude(float press, float temp, float h) {

 float p0;
 float altitude;
 //return ((pow((sea_press / press), 1/5.257) - 1.0) * (temp + 273.15)) / 0.0065;
 p0 = ((press/100) * pow(1 - (0.0065 * h / (temp + 0.0065 * h + 273.15)), -5.257));
 altitude = bmp.readAltitude(p0 * 100);
 return altitude;
}
