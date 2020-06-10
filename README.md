# WeatherStation Powered By MR WATT - Simone Cecchetti

Weather Station using the following components:

Wemos D1 Mini R2
Wemos Battery Shield
AM2302 Temperature/Humidity Sensor
BMP180 Barometric Pressure/Temperature/Altitude Sensor
MQ135 Air Quality Sensor ( NH3, NOx, alcohol, Benzene, smoke, CO2, etc. )
N.2 Li-ION Batteries 3200mA 3,8V
SSD_1306 0.96" I2C IIC Oled 128X64
ThingSpeak Account ( it's free )

Create credentials.h file with this structure 


#define SECRET_SSID "<YOURWIFISSID>"            // replace MySSID with your WiFi network name
#define SECRET_PASS "<YOURWIFIPASSWORD>"      // replace MyPassword with your WiFi password

#define SECRET_CH_ID_WEATHER_STATION 123456                     //MathWorks weather station

#define SECRET_CH_ID_COUNTER 123456                                     //Test channel for counting
#define SECRET_READ_APIKEY "<YOURTHINGSPEAKREADAPIKEY>"                 //Read API Key for Test channel
#define SECRET_WRITE_APIKEY "<YOURTHINGSPEAKWRITEAPIKEY>"               //Write API Key for Test channel    

