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
#include <WiFiUdp.h>
#include <MQ135.h>
#include <stdio.h>
#include <ThingSpeak.h>
#include <Wire.h>
#include <Adafruit_BMP085.h>
#include <DHT.h>
//#include <ACROBOTIC_SSD1306.h>
#include <Arduino.h>
#include <U8g2lib.h>
#include <ESP8266WebServer.h>
#include <FS.h> // FOR SPIFFS
#include <ctype.h> // for isNumber check
#include "Time.h"
#include "TimeLib.h"
#include "SSD1306.h"

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
//U8G2_NULL u8g2(U8G2_R0);  // null device, a 8x8 pixel display which does nothing
//U8G2_SSD1306_128X64_NONAME_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
//U8G2_SSD1306_128X64_NONAME_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 12, /* dc=*/ 4, /* reset=*/ 6); // Arduboy (Production, Kickstarter Edition)
//U8G2_SSD1306_128X64_NONAME_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
//U8G2_SSD1306_128X64_NONAME_F_3W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* reset=*/ 8);
//U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
//U8G2_SSD1306_128X64_ALT0_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);   // same as the NONAME variant, but may solve the "every 2nd line skipped" problem
//U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* reset=*/ 8);
//U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ SCL, /* data=*/ SDA, /* reset=*/ U8X8_PIN_NONE);   // All Boards without Reset of the Display
//U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ 16, /* data=*/ 17, /* reset=*/ U8X8_PIN_NONE);   // ESP32 Thing, pure SW emulated I2C
//U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE, /* clock=*/ 16, /* data=*/ 17);   // ESP32 Thing, HW I2C with pin remapping
//U8G2_SSD1306_128X64_NONAME_F_6800 u8g2(U8G2_R0, 13, 11, 2, 3, 4, 5, 6, A4, /*enable=*/ 7, /*cs=*/ 10, /*dc=*/ 9, /*reset=*/ 8);
//U8G2_SSD1306_128X64_NONAME_F_8080 u8g2(U8G2_R0, 13, 11, 2, 3, 4, 5, 6, A4, /*enable=*/ 7, /*cs=*/ 10, /*dc=*/ 9, /*reset=*/ 8);
//U8G2_SSD1306_128X64_VCOMH0_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8); // same as the NONAME variant, but maximizes setContrast() range
//U8G2_SSD1306_128X64_ALT0_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8); // same as the NONAME variant, but may solve the "every 2nd line skipped" problem
//U8G2_SH1106_128X64_NONAME_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
//U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
//U8G2_SH1106_128X64_VCOMH0_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);    // same as the NONAME variant, but maximizes setContrast() range
//U8G2_SH1106_128X64_WINSTAR_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);   // same as the NONAME variant, but uses updated SH1106 init sequence
//U8G2_SH1106_72X40_WISE_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
//U8G2_SH1107_64X128_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
//U8G2_SH1107_128X128_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
//U8G2_SH1107_128X128_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ 8);
//U8G2_SH1107_SEEED_96X96_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
//U8G2_SH1108_160X160_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
//U8G2_SH1122_256X64_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);       // Enable U8G2_16BIT in u8g2.h
//U8G2_SSD1306_128X32_UNIVISION_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ 21, /* data=*/ 20, /* reset=*/ U8X8_PIN_NONE);   // Adafruit Feather M0 Basic Proto + FeatherWing OLED
//U8G2_SSD1306_128X32_UNIVISION_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ SCL, /* data=*/ SDA, /* reset=*/ U8X8_PIN_NONE);   // Adafruit Feather ESP8266/32u4 Boards + FeatherWing OLED
//U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);  // Adafruit ESP8266/32u4/ARM Boards + FeatherWing OLED
//U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE, /* clock=*/ SCL, /* data=*/ SDA);   // pin remapping with ESP8266 HW I2C
//U8G2_SSD1306_64X48_ER_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);   // EastRising 0.66" OLED breakout board, Uno: A4=SDA, A5=SCL, 5V powered
//U8G2_SSD1306_48X64_WINSTAR_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);   
//U8G2_SSD1306_64X32_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE); 
//U8G2_SSD1306_64X32_1F_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE); 
//U8G2_SSD1306_96X16_ER_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);   // EastRising 0.69" OLED
//U8G2_SSD1322_NHD_256X64_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8); // Enable U8G2_16BIT in u8g2.h
//U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);  // Enable U8G2_16BIT in u8g2.h
//U8G2_SSD1322_NHD_128X64_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
//U8G2_SSD1322_NHD_128X64_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
//U8G2_SSD1325_NHD_128X64_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8); 
//U8G2_SSD1325_NHD_128X64_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);  
//U8G2_SSD1326_ER_256X32_1_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);         // experimental driver for ER-OLED018-1
//U8G2_SSD1327_SEEED_96X96_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ SCL, /* data=*/ SDA, /* reset=*/ U8X8_PIN_NONE);  // Seeedstudio Grove OLED 96x96
//U8G2_SSD1327_SEEED_96X96_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE); // Seeedstudio Grove OLED 96x96
//U8G2_SSD1327_EA_W128128_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
//U8G2_SSD1327_EA_W128128_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
//U8G2_SSD1327_EA_W128128_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ 5, /* data=*/ 4, /* reset=*/ U8X8_PIN_NONE);
//U8G2_SSD1327_EA_W128128_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
//U8G2_SSD1327_MIDAS_128X128_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
//U8G2_SSD1327_MIDAS_128X128_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
//U8G2_SSD1329_128X96_NONAME_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
//U8G2_SSD1329_128X96_NONAME_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
//U8G2_SSD1305_128X32_NONAME_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
//U8G2_SSD1305_128X32_NONAME_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
//U8G2_SSD1306_128X64_ADAFRUIT_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
//U8G2_SSD1306_128X64_ADAFRUIT_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
//U8G2_SSD1309_128X64_NONAME0_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);  
//U8G2_SSD1309_128X64_NONAME0_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);  
//U8G2_SSD1309_128X64_NONAME2_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);  
//U8G2_SSD1309_128X64_NONAME2_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);  
//U8G2_SSD1317_96X96_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);  
//U8G2_SSD1317_96X96_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8); 
//U8G2_LD7032_60X32_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 11, /* data=*/ 12, /* cs=*/ 9, /* dc=*/ 10, /* reset=*/ 8); // SW SPI Nano Board
//U8G2_LD7032_60X32_F_4W_SW_I2C u8g2(U8G2_R0, /* clock=*/ 11, /* data=*/ 12, /* reset=*/ U8X8_PIN_NONE);  // NOT TESTED!
//U8G2_UC1701_EA_DOGS102_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
//U8G2_UC1701_EA_DOGS102_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
//U8G2_PCD8544_84X48_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);  // Nokia 5110 Display
//U8G2_PCD8544_84X48_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);     // Nokia 5110 Display
//U8G2_PCF8812_96X65_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);  // Could be also PCF8814
//U8G2_PCF8812_96X65_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);           // Could be also PCF8814
//U8G2_HX1230_96X68_F_3W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* reset=*/ 8);
//U8G2_HX1230_96X68_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
//U8G2_KS0108_128X64_F u8g2(U8G2_R0, 8, 9, 10, 11, 4, 5, 6, 7, /*enable=*/ 18, /*dc=*/ 17, /*cs0=*/ 14, /*cs1=*/ 15, /*cs2=*/ U8X8_PIN_NONE, /* reset=*/  U8X8_PIN_NONE);   // Set R/W to low!
//U8G2_KS0108_ERM19264_F u8g2(U8G2_R0, 8, 9, 10, 11, 4, 5, 6, 7, /*enable=*/ 18, /*dc=*/ 17, /*cs0=*/ 14, /*cs1=*/ 15, /*cs2=*/ 16, /* reset=*/  U8X8_PIN_NONE);  // Set R/W to low!
//U8G2_ST7920_192X32_F_8080 u8g2(U8G2_R0, 8, 9, 10, 11, 4, 5, 6, 7, /*enable=*/ 18, /*cs=*/ U8X8_PIN_NONE, /*dc=*/ 17, /*reset=*/ U8X8_PIN_NONE);
//U8G2_ST7920_192X32_F_SW_SPI u8g2(U8G2_R0, /* clock=*/ 18 /* A4 */ , /* data=*/ 16 /* A2 */, /* CS=*/ 17 /* A3 */, /* reset=*/ U8X8_PIN_NONE);
//U8G2_ST7920_128X64_F_8080 u8g2(U8G2_R0, 8, 9, 10, 11, 4, 5, 6, 7, /*enable=*/ 18 /* A4 */, /*cs=*/ U8X8_PIN_NONE, /*dc/rs=*/ 17 /* A3 */, /*reset=*/ 15 /* A1 */);  // Remember to set R/W to 0 
//U8G2_ST7920_128X64_F_SW_SPI u8g2(U8G2_R0, /* clock=*/ 18 /* A4 */ , /* data=*/ 16 /* A2 */, /* CS=*/ 17 /* A3 */, /* reset=*/ U8X8_PIN_NONE);
//U8G2_ST7920_128X64_F_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* CS=*/ 10, /* reset=*/ 8);
//U8G2_ST7920_128X64_F_HW_SPI u8g2(U8G2_R0, /* CS=*/ 10, /* reset=*/ 8);
//U8G2_ST7565_EA_DOGM128_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
//U8G2_ST7565_EA_DOGM128_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
//U8G2_ST7565_64128N_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
//U8G2_ST7565_64128N_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
//U8G2_ST7565_EA_DOGM132_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ U8X8_PIN_NONE);  // DOGM132 Shield
//U8G2_ST7565_EA_DOGM132_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ U8X8_PIN_NONE); // DOGM132 Shield
//U8G2_ST7565_ZOLEN_128X64_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
//U8G2_ST7565_ZOLEN_128X64_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
//U8G2_ST7565_LM6059_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);    // Adafruit ST7565 GLCD
//U8G2_ST7565_LM6059_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);   // Adafruit ST7565 GLCD
//U8G2_ST7565_LX12864_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
//U8G2_ST7565_LX12864_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
//U8G2_ST7565_ERC12864_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
//U8G2_ST7565_ERC12864_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
//U8G2_ST7565_NHD_C12832_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
//U8G2_ST7565_NHD_C12832_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
//U8G2_ST7565_NHD_C12864_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
//U8G2_ST7565_NHD_C12864_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
//U8G2_ST7565_JLX12864_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
//U8G2_ST7565_JLX12864_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
//U8G2_ST7567_PI_132X64_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 7, /* dc=*/ 9, /* reset=*/ 8);  // Pax Instruments Shield, LCD_BL=6
//U8G2_ST7567_PI_132X64_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 7, /* dc=*/ 9, /* reset=*/ 8);  // Pax Instruments Shield, LCD_BL=6
//U8G2_ST7567_JLX12864_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 7, /* dc=*/ 9, /* reset=*/ 8);  
//U8G2_ST7567_JLX12864_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 7, /* dc=*/ 9, /* reset=*/ 8);  
//U8G2_ST7567_ENH_DG128064_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8); 
//U8G2_ST7567_ENH_DG128064_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8); 
//U8G2_ST7567_ENH_DG128064I_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8); 
//U8G2_ST7567_ENH_DG128064I_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8); 
//U8G2_ST7567_64X32_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE); 
//U8G2_ST75256_JLX172104_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
//U8G2_ST75256_JLX172104_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
//U8G2_ST75256_JLX256128_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);  // Enable U8g2 16 bit mode for this display
//U8G2_ST75256_JLX256128_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);   // Enable U8g2 16 bit mode for this display
//U8G2_ST75256_JLX256128_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 9, /* data=*/ 8, /* cs=*/ 7, /* dc=*/ 6, /* reset=*/ 5);  // MKR Zero, Enable U8g2 16 bit mode for this display
//U8G2_ST75256_JLX256128_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 7, /* dc=*/ 6, /* reset=*/ 5);  // MKR Zero, Enable U8g2 16 bit mode for this display
//U8G2_ST75256_JLX256160_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);  // Enable U8g2 16 bit mode for this display
//U8G2_ST75256_JLX256160_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);   // Enable U8g2 16 bit mode for this display
//U8G2_ST75256_JLX240160_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
//U8G2_ST75256_JLX240160_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
//U8G2_ST75256_JLX25664_F_2ND_HW_I2C u8g2(U8G2_R0, /* reset=*/ 8);  // Due, 2nd I2C, enable U8g2 16 bit mode for this display
//U8G2_NT7534_TG12864R_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);  
//U8G2_NT7534_TG12864R_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);  
//U8G2_ST7588_JLX12864_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ SCL, /* data=*/ SDA, /* reset=*/ 5);  
//U8G2_ST7588_JLX12864_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ 5);
//U8G2_IST3020_ERC19264_F_6800 u8g2(U8G2_R0, 44, 43, 42, 41, 40, 39, 38, 37,  /*enable=*/ 28, /*cs=*/ 32, /*dc=*/ 30, /*reset=*/ 31); // Connect WR pin with GND
//U8G2_IST3020_ERC19264_F_8080 u8g2(U8G2_R0, 44, 43, 42, 41, 40, 39, 38, 37,  /*enable=*/ 29, /*cs=*/ 32, /*dc=*/ 30, /*reset=*/ 31); // Connect RD pin with 3.3V
//U8G2_IST3020_ERC19264_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
//U8G2_LC7981_160X80_F_6800 u8g2(U8G2_R0, 8, 9, 10, 11, 4, 5, 6, 7, /*enable=*/ 18, /*cs=*/ 14, /*dc=*/ 15, /*reset=*/ 16); // Connect RW with GND
//U8G2_LC7981_160X160_F_6800 u8g2(U8G2_R0, 8, 9, 10, 11, 4, 5, 6, 7, /*enable=*/ 18, /*cs=*/ 14, /*dc=*/ 15, /*reset=*/ 16); // Connect RW with GND
//U8G2_LC7981_240X128_F_6800 u8g2(U8G2_R0, 8, 9, 10, 11, 4, 5, 6, 7, /*enable=*/ 18, /*cs=*/ 14, /*dc=*/ 15, /*reset=*/ 16); // Connect RW with GND
//U8G2_LC7981_240X64_F_6800 u8g2(U8G2_R0, 8, 9, 10, 11, 4, 5, 6, 7, /*enable=*/ 18, /*cs=*/ 14, /*dc=*/ 15, /*reset=*/ 16); // Connect RW with GND
//U8G2_SED1520_122X32_F u8g2(U8G2_R0, 8, 9, 10, 11, 4, 5, 6, 7, /*dc=*/ A0, /*e1=*/ A3, /*e2=*/ A2, /* reset=*/  A4);   // Set R/W to low!
//U8G2_T6963_240X128_F_8080 u8g2(U8G2_R0, 8, 9, 10, 11, 4, 5, 6, 7, /*enable=*/ 17, /*cs=*/ 14, /*dc=*/ 15, /*reset=*/ 16); // Connect RD with +5V, FS0 and FS1 with GND
//U8G2_T6963_256X64_F_8080 u8g2(U8G2_R0, 8, 9, 10, 11, 4, 5, 6, 7, /*enable=*/ 17, /*cs=*/ 14, /*dc=*/ 15, /*reset=*/ 16); // Connect RD with +5V, FS0 and FS1 with GND
//U8G2_T6963_160X80_F_8080 u8g2(U8G2_R0, 8, 9, 10, 11, 4, 5, 6, 7, /*enable=*/ 17, /*cs=*/ 14, /*dc=*/ 15, /*reset=*/ 16); // Connect RD with +5V, FS0 and FS1 with GND
//U8G2_SED1330_240X128_F_8080 u8g2(U8G2_R0, 8, 9, 10, 11, 4, 5, 6, 7, /*enable=*/ 17, /*cs=*/ 14, /*dc=*/ 15, /*reset=*/ 16); // Connect RD with +5V, FG with GND
//U8G2_SED1330_240X128_F_6800 u8g2(U8G2_R0, 13, 11, 2, 3, 4, 5, 6, A4, /*enable=*/ 7, /*cs=*/ 10, /*dc=*/ 9, /*reset=*/ 8); // A0 is dc pin!
//U8G2_RA8835_NHD_240X128_F_8080 u8g2(U8G2_R0, 8, 9, 10, 11, 4, 5, 6, 7, /*enable=*/ 17, /*cs=*/ 14, /*dc=*/ 15, /*reset=*/ 16); // Connect /RD = E with +5V, enable is /WR = RW, FG with GND, 14=Uno Pin A0
//U8G2_RA8835_NHD_240X128_F_6800 u8g2(U8G2_R0, 8, 9, 10, 11, 4, 5, 6, 7,  /*enable=*/ 17, /*cs=*/ 14, /*dc=*/ 15, /*reset=*/ 16); // A0 is dc pin, /WR = RW = GND, enable is /RD = E
//U8G2_UC1604_JLX19264_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8); 
//U8G2_UC1604_JLX19264_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);  
//U8G2_UC1608_ERC24064_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);  // SW SPI, Due ERC24064-1 Test Setup
//U8G2_UC1608_ERC240120_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8); 
//U8G2_UC1608_240X128_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);  // SW SPI, Due ERC24064-1 Test Setup
//U8G2_UC1610_EA_DOGXL160_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/  U8X8_PIN_NONE);
//U8G2_UC1610_EA_DOGXL160_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/  U8X8_PIN_NONE);
//U8G2_UC1611_EA_DOGM240_F_2ND_HW_I2C u8g2(U8G2_R0, /* reset=*/ 8); // Due, 2nd I2C, DOGM240 Test Board
//U8G2_UC1611_EA_DOGM240_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);   // Due, SW SPI, DOGXL240 Test Board
//U8G2_UC1611_EA_DOGXL240_F_2ND_HW_I2C u8g2(U8G2_R0, /* reset=*/ 8);  // Due, 2nd I2C, DOGXL240 Test Board
//U8G2_UC1611_EA_DOGXL240_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);   // Due, SW SPI, DOGXL240 Test Board
//U8G2_UC1611_EW50850_F_8080 u8g2(U8G2_R0, 8, 9, 10, 11, 4, 5, 6, 7,  /*enable=*/ 18, /*cs=*/ 3, /*dc=*/ 16, /*reset=*/ 16); // 240x160, Connect RD/WR1 pin with 3.3V, CS is aktive high
//U8G2_UC1638_160X128_F_4W_HW_SPI u8g2(U8G2_R2, /* cs=*/ 2, /* dc=*/ 3, /* reset=*/ 4);    // Not tested
//U8G2_SSD1606_172X72_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);   // eInk/ePaper Display
//U8G2_SSD1607_200X200_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);  // eInk/ePaper Display, original LUT from embedded artists
//U8G2_SSD1607_GD_200X200_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8); // Good Display
//U8G2_IL3820_296X128_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8); // WaveShare 2.9 inch eInk/ePaper Display, enable 16 bit mode for this display!
//U8G2_IL3820_V2_296X128_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);  // ePaper Display, lesser flickering and faster speed, enable 16 bit mode for this display!
//U8G2_MAX7219_32X8_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 11, /* data=*/ 12, /* cs=*/ 10, /* dc=*/ U8X8_PIN_NONE, /* reset=*/ U8X8_PIN_NONE);
//U8G2_MAX7219_8X8_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 11, /* data=*/ 12, /* cs=*/ 10, /* dc=*/ U8X8_PIN_NONE, /* reset=*/ U8X8_PIN_NONE);
//U8G2_LS013B7DH03_128X128_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ U8X8_PIN_NONE, /* reset=*/ 8); // there is no DC line for this display
//U8G2_LS027B7DH01_400X240_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 10, /* dc=*/ U8X8_PIN_NONE, /* reset=*/ 8); // there is no DC line for this display

// End of constructor list



// Wifi Settings
char ssid[] = "<YOUR-NETWORK-SSID>";  //  your network SSID (name)
char pass[] = "<YOUR-NETWORK-PASSWORD";       // your network password

const String fName = "props.txt"; // properties file

float UTCoffset = 0;
unsigned long lastMillis = 0;
unsigned long currentMillis = 0;
unsigned long secsSince1900 = 0;
bool daylightSavings = false;
bool hourTime = false;
int interval = 30000; //
String timeStr = "";
String webMessage = "";
String dateStr = "";
float dewpt=0;          // dew point tempf


WiFiClient  client;

//Initialize DHT22 Sensor
#define DHTPIN 14     // what digital pin we're connected to

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
unsigned long myChannelNumber = <YOUR_THING-SPEAK_CHANNEL>;
const char * myWriteAPIKey = "<YOUR_THING-SPEAK_WRITE_API-KEY>";

// Settaggi Barometro BMP180 
//float seaLevelPressure = 101325;
float seaLevelPressure = 102004;
// Valore calcolato in base all'altitudine : http://www.calctool.org/CALC/phys/default/pres_at_alt ho utilizzato il valore medio


unsigned int localPort = 2390;      // local port to listen for UDP packets

/* Don't hardwire the IP address or we won't get the benefits of the pool.
    Lookup the IP address for the host name instead */
//IPAddress timeServer(129, 6, 15, 28); // time.nist.gov NTP server
IPAddress timeServerIP; // time.nist.gov NTP server address
const char* ntpServerName = "time.nist.gov";

const int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message

byte packetBuffer[ NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets

// A UDP instance to let us send and receive packets over UDP
WiFiUDP udp;

// Initialize the OLED display using Wire library
SSD1306  display(0x3c, SDA, SCL);



ESP8266WebServer server(80);

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
  webString +="<!DOCTYPE html><html><head><style>table\ {\ font-family:\ arial,\ sans-serif;\ border-collapse:\ collapse;\ width:\ 100%;\ }\ td,\ th\ {\ border:\ 1px\ solid\ #dddddd;\ text-align:\ left;\ padding:\ 8px;\ }\ tr:nth-child(even)\ {\ background-color:\ #dddddd;\ }</style></head><body><h2>HTML Table</h2><table><tr><th>Company</th><th>Contact</th><th>Country</th></tr><tr><td>Alfreds Futterkiste</td><td>Maria Anders</td><td>Germany</td></tr><tr><td>Centro comercial Moctezuma</td><td>Francisco Chang</td><td>Mexico</td></tr><tr><td>Ernst Handel</td><td>Roland Mendel</td><td>Austria</td></tr><tr><td>Island Trading</td><td>Helen Bennett</td><td>UK</td></tr><tr><td>Laughing Bacchus Winecellars</td><td>Yoshi Tannamuri</td><td>Canada</td></tr><tr><td>Magazzini Alimentari Riuniti</td><td>Giovanni Rovelli</td><td>Italy</td></tr></table></body></html>\n";
  
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
 //   display.clear();
 //   display.drawString(0, 0, " Updating");
 //   display.drawString(0, 19, "properties");
 //   display.drawString(0, 40, "file...");
 //   display.display();

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

 //   display.clear();
  //  display.drawString(0, 0,  "Formatting...");
 //   display.drawString(0, 19, "Please wait 30");
 //   display.drawString(0, 40, "seconds.");
 //   display.display();

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
  daylightSavings = false;
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



////////////////////////////////////////////////////////////////////////////////////////

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
  Serial.println("WEATHER STATION - Temperature, Pressure, Humidity, Air Quality using Sensors DHT22, MQ135 and BMP180 - Powered By MR WATT");
  Serial.println("BootStrap...");
  
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

 // Initialize ThingSpeak
  ThingSpeak.begin(client);

}


void loop() {
 // Wait a few seconds between measurements. 
 delay(5000);
 
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

oledTemp = currentTemperature;
oledHum = humidity;
oledPressure = currentPressurehPA;
oledPPM = ppm;
oledDew = dewpt;


currentMillis = millis();

  String AmPm = "";

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

  Serial.println(dateStr);

 // display.clear();
//  display.drawString(0, 0,  timeStr);
//  display.drawString(0, 19, dateStr);
//  display.drawString(0, 40, WiFi.localIP().toString());

 // display.display();

  server.handleClient();

 delay(1000);

 // Check if any reads failed and exit early (to try again).
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  // Compute heat index in Celsius (isFahreheit = false)
  float hic = dht.computeHeatIndex(temperature, humidity, false);

 sensorValue = analogRead(A0);
 sensorVoltage = sensorValue/1024*5.0;

 dewpt = (dewPoint(temperature, humidity));
 
 Serial.print("\t");   
 Serial.print("\t Temperature DHT22: ");
 Serial.print(temperature);
 Serial.print("C ");
 Serial.print("\t Humidity: "); 
 Serial.print(humidity);
 Serial.print("%");
 Serial.print("\t Sensor Voltage: ");
 Serial.print(sensorVoltage);
 Serial.print("V");
 Serial.print("\t MQ135 RZero: ");
 Serial.print(rzero);
 Serial.print("\t Corrected RZero: ");
 Serial.print(correctedRZero);
 Serial.print("\t Resistance: ");
 Serial.print(resistance);
 Serial.print("\t PPM: ");
 Serial.print(ppm);
 Serial.print("\t Corrected PPM: ");
 Serial.print(correctedPPM);
 Serial.println("ppm");
 Serial.print("\t Altitude: ");
 Serial.print(currentAltitude);
 Serial.print("m ");
 Serial.print("\t Pressure: ");
 Serial.print(currentPressure);
 Serial.print("Pa ");
 Serial.print("\t Temperature BMP180: ");
 Serial.print(currentTemperature);
 Serial.print("C");
 Serial.print("\t Pressure Sealevel BMP180: ");
 Serial.print(currentSealevelPressure);
 Serial.print("m ");
 Serial.print("\t Dew Point: ");
 Serial.print(dewpt);
 Serial.print(hour());
// Serial.print("\t Battery Spanning: ");
// Serial.print(getVoltage());
// Serial.print("V ");
// Serial.print("\t Battery Level: ");
// Serial.print(getLevel());
// Serial.print("% ");
 
 // Write to ThingSpeak
   ThingSpeak.setField(1,temperature);
   ThingSpeak.setField(2,humidity);
   ThingSpeak.setField(3,currentPressurehPA);
   ThingSpeak.setField(4,ppm);
   ThingSpeak.setField(5,getVoltage());
   ThingSpeak.setField(6,dewpt);
   ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);  

// draw("Temperature (Celsius) ...", TEMPERATURE, int(currentTemperature));
 if ( hour() > 6 && hour() < 19 ) {draw("Temperature (Celsius) ...", TEMPERATUREDAY, int(currentTemperature));}
 if ( hour() > 19 && hour() < 6 ) {draw("Temperature (Celsius) ...", TEMPERATURENIGHT, int(currentTemperature));}

draw("Humidity (%) ...", HUMIDITY, int(humidity));
draw("Pressure (hPa) ...", PRESSURE, int(currentPressurehPA));
 if ( correctedPPM > 1 && correctedPPM < 700 ) {draw("Air Quality (PPM)... >>>EXCELLENT<<<", AIRQ, int(ppm));}
 if ( correctedPPM > 700 && correctedPPM < 800 ) {draw("Air Quality (PPM)... >>>GOOD<<<", AIRQ, int(ppm));}
 if ( correctedPPM > 800 && correctedPPM < 1000 ) {draw("Air Quality (PPM)... >>>FAIR<<<", AIRQ, int(ppm));}
 if ( correctedPPM > 1000 && correctedPPM < 1500 ) {draw("Air Quality (PPM)... >>>!MEDIOCRE!<<<", AIRQ, int(ppm));}
 if ( correctedPPM > 1500 && correctedPPM < 2000 ) {draw("Air Quality (PPM)... >>>!!BAD!!<<<", AIRQ, int(ppm));}
 if ( correctedPPM > 2000 && correctedPPM < 50000 ) {draw("Air Quality (PPM)... >>>!!!TOXIC!!!<<<", AIRQ, int(ppm));}
 if ( correctedPPM > 50000 || correctedPPM == 0 ) {draw("Air Quality (PPM)... >>>!PREHEAT MQ135!<<<", AIRQ, int(ppm));}
// draw("Air Quality (PPM) ...", AIRQ, int(ppm));
draw("Dew Point (Celsius) ...", DEWPT, int(dewpt));

 
//  delay(1000); // delay one second before OLED display update
//u8g2.sendBuffer();          // transfer internal memory to the display
delay(10000);

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
  Serial.print("\tVoltage "); Serial.println(volt);
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
