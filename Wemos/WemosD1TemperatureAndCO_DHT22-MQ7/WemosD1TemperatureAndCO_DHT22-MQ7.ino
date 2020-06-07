/*
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
#include <stdio.h>
#include <ThingSpeak.h>
#include <Wire.h>
#include <DHT.h>
#include <Arduino.h>
#include <ctype.h> // for isNumber check

// Wifi Settings
char ssid[] = "VodafoneSurfer";  //  your network SSID (name)
char pass[] = "B311a@P3r.Te";       // your network password

float dewpt=0;          // dew point tempf

WiFiClient  client;

//Initialize DHT22 Sensor
#define DHTPIN 14     // what digital pin we're connected to
// Uncomment whatever type you're using!
#define DHTTYPE DHT11   // DHT 11
//#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
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


// ThingSpeak Settings

unsigned long myChannelNumber = 634036;
const char * myWriteAPIKey = "40G12BFQSZ6TV3XO";


void setup() {
  Serial.begin(9600);
  Serial.println();
  Serial.print("Sketch:   ");   Serial.println(__FILE__);
  Serial.print("Uploaded: ");   Serial.println(__DATE__);
  Serial.println(" ");
  Serial.println("WEATHER STATION - Temperature using Sensor DHT22 - Powered By MR WATT");
  Serial.println("BootStrap...");

  
  // We start by connecting to a WiFi network
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");

  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

   pinMode(LED_BUILTIN, OUTPUT);     // Initialize the LED_BUILTIN pin as an output
 
 
  // DHT22 Test
  Serial.println("Initialize DHT22...");
  dht.begin();

 // Initialize ThingSpeak
  ThingSpeak.begin(client);

}


void loop() {
 // Wait a few seconds between measurements. 
 delay(5000);
 
 float temperature = dht.readTemperature();
 float humidity = dht.readHumidity();
 float dewpt = 0;

 // Check if any reads failed and exit early (to try again).
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  // Compute heat index in Celsius (isFahreheit = false)
  float hic = dht.computeHeatIndex(temperature, humidity, false);

 dewpt = (dewPoint(temperature, humidity));


 Serial.print("\t Temperature DHT22: ");
 Serial.print(temperature);
 Serial.print("C ");
 Serial.print("\t Humidity: "); 
 Serial.print(humidity);
 Serial.print("%");
 Serial.print("\t Dew Point: ");
 Serial.print(dewpt);

 // Write to ThingSpeak
   ThingSpeak.setField(1,temperature);
   ThingSpeak.setField(2,humidity);
   ThingSpeak.setField(3,dewpt);
   ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);  
   
delay(15000);
client.stop();

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