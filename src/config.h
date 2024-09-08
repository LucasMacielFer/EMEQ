/* 
 * -----------------------------------------------------------------------------
 * Example: Two way communication between ESP32 and Python using WIFI
 * -----------------------------------------------------------------------------
 * Author: Radhi SGHAIER: https://github.com/Rad-hi
 * -----------------------------------------------------------------------------
 * Date: 07-05-2023 (7th of May, 2023)
 * -----------------------------------------------------------------------------
 * License: Do whatever you want with the code ...
 *          If this was ever useful to you, and we happened to meet on 
 *          the street, I'll appreciate a cup of dark coffee, no sugar please.
 * -----------------------------------------------------------------------------
 */

#ifndef __CONFG_H___
#define __CONFG_H___

/* Pins definitions */  
#define FILL_PIN                     21
#define EMPTY_PIN                    19
//#define BTN_PIN                      25
#define RXp2 16
#define TXp2 17

#define DHTPIN 2     // what pin we're connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302)
#define DS18B20PIN 25 // GPIO where the DS18B20 is connected to
#define TDSPIN 27
#define TURBIDITYPIN 35
#define VREF 3.3      // analog reference voltage(Volt) of the ADC
#define SCOUNT 30           // sum of sample point 
#define MQ7PIN 4 

/* Communication params */
#define ACK                         "A" // acknowledgment packet
#define QUEUE_LEN                   5
#define MAX_BUFFER_LEN              128

/* WiFi params */
#define WIFI_SSID                   "EMEQ"
#define WIFI_PASSWORD               "monsterdemanga"

/* Socket */
#define SERVER_ADDRESS              "192.168.146.73"
#define SERVER_PORT                 11111

#endif // __CONFG_H___
