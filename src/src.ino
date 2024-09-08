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

#include "config.h"
#include "my_wifi.h"
#include "wifi_communicator.h"
#include <OneWire.h>
#include <DallasTemperature.h>
#include "DHT.h"
#include "MQ7.h"
#include <MyButton.h>
#include "esp_task_wdt.h"

#define ENABLE_DEBUG /* <-- Commenting this line will remove any trace of debug printing */
#include <MacroDebugger.h>

// Button object to simulate input events
MyButton my_btn(TDSPIN, NORMAL_DOWN, 50);

// Communication messages
char incoming_msg[MAX_BUFFER_LEN] = {0};
char response[MAX_BUFFER_LEN] = {0};
int analogBuffer[SCOUNT];    // store the analog value in the array, read from ADC
int analogBufferTemp[SCOUNT];
int analogBufferIndex = 0;
int copyIndex = 0;
float averageVoltage = 0;
float tdsValue = 0;
float temperature = 0;
bool ler;
int tempo;

//====================| Parâmetros turbidez |====================
int sensor = 0; // variable for averaging
int n = 25; // number of samples to average
int sensorValue = 0;
float voltage = 0.00;
float turbidity = 0.00;
float Vclear = 0.812; // Output voltage to calibrate

//====================| Sensores |====================
MQ7 mq7(MQ7PIN,3.3);
DHT dht(DHTPIN, DHTTYPE);
OneWire oneWire(DS18B20PIN);    // Setup a oneWire instance to communicate with any OneWire devices
DallasTemperature sensors(&oneWire);    // Pass our oneWire reference to Dallas Temperature sensor
 

void setup(){
  esp_task_wdt_init(10, true);
  DEBUG_BEGIN();
  setup_wifi();
  setup_wifi_communicator();
  pinMode(FILL_PIN, OUTPUT);
  pinMode(EMPTY_PIN, OUTPUT);
  pinMode(2, INPUT);
  sensors.begin();
  dht.begin();
  DEBUG_I("Done setting up!");
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, RXp2, TXp2);
}

void loop(){

  // if we lost connection, we attempt to reconnect (blocking)
  if(!is_client_connected()){ connect_client(); }
  
  bool received = get_message(incoming_msg);

  if(received){
    DEBUG_I("Received: %s", incoming_msg);
    uint8_t start = 0;

    if(incoming_msg[0] == 'A'){
      sprintf(response, "%s", ACK);
      start++;
    }

    //switch the number and do the appropriate action
    switch(incoming_msg[start]){
      case 'f':
        digitalWrite(FILL_PIN, HIGH);
        break;

      case 'e':
        digitalWrite(EMPTY_PIN, HIGH);
        break;
    
      case 'm':
        digitalWrite(FILL_PIN, LOW);
        executaMedicao();
        break;

      case 's':
        digitalWrite(EMPTY_PIN, LOW);
        digitalWrite(FILL_PIN, LOW);
        break;

      default:
        break;
    }
  }
}

void executaMedicao()
{
  sensors.requestTemperatures();
  float temperature = sensors.getTempCByIndex(0);
  
  static unsigned long analogSampleTimepoint = millis();
  if (millis() - analogSampleTimepoint > 40U)  //every 40 milliseconds,read the analog value from the ADC
  {
      analogSampleTimepoint = millis();
      analogBuffer[analogBufferIndex] = analogRead(TDSPIN);    //read the analog value and store into the buffer
      analogBufferIndex++;
      if (analogBufferIndex == SCOUNT)
          analogBufferIndex = 0;
  }

  static unsigned long printTimepoint = millis();
  if (millis() - printTimepoint > 800U)
  {
      printTimepoint = millis();
      for (copyIndex = 0; copyIndex < SCOUNT; copyIndex++)
          analogBufferTemp[copyIndex] = analogBuffer[copyIndex];
      averageVoltage = getMedianNum(analogBufferTemp, SCOUNT) * (float)VREF / 1024.0; // read the analog value more stable by the median filtering algorithm, and convert to voltage value
      float compensationCoefficient = 1.0 + 0.02 * (temperature - 25.0); //temperature compensation formula: fFinalResult(25^C) = fFinalResult(current)/(1.0+0.02*(fTP-25.0));
      float compensationVolatge = averageVoltage / compensationCoefficient; //temperature compensation
      tdsValue = (133.42 * compensationVolatge * compensationVolatge * compensationVolatge - 255.86 * compensationVolatge * compensationVolatge + 857.39 * compensationVolatge) * 0.5; //convert voltage value to tds value
  }
  //====================| DHT22 |====================
  // Read air humidity
  float h = dht.readHumidity();
  // Read temperature as Celsius
  float t = dht.readTemperature();

  //===================| Sensor de turbidez |===================
  /*for (int i=0; i < n; i++){
      sensor += analogRead(TURBIDITYPIN); // read the input on analog pin 1 (turbidity sensor analog output)
      delay(10);
  }*/
  sensorValue = analogRead(TURBIDITYPIN); // average the n values
  voltage = sensorValue * (3.3/ 4095.000); // Convert analog (0-1023) to voltage (0 - 5V)
  turbidity = 100.00 - (voltage / Vclear) * 100.00; // as relative percentage; 0% = clear water;
  if(turbidity < 0) {turbidity = 0;}
  float ppm = mq7.getPPM();
  Serial2.begin(9600, SERIAL_8N1, RXp2, TXp2);
  String pH = Serial2.readString();

  //char* buf = (char*)malloc(10*sizeof(char));
  while (Serial2.available()) {
    Serial2.read(); // Read and discard bytes
  }
  char hehe[10];
  delay(250);
  pH.toCharArray(hehe, 10);
  send_message(hehe);
  Serial.printf("pH: c%s\n", hehe);
  memset(hehe, 0, 10);
  delay(250);
  dtostrf(t, 2, 2, hehe);
  send_message(hehe);
  Serial.printf("Temperatura externa: c%s\n", hehe);
  memset(hehe, 0, 10);
  delay(250);
  dtostrf(h, 2, 2, hehe);
  send_message(hehe);
  Serial.printf("Umidade: c%s\n", hehe);
  memset(hehe, 0, 10);
  delay(250);
  dtostrf(ppm, 2, 2, hehe);
  send_message(hehe);
  Serial.printf("CO: c%s\n", hehe);
  memset(hehe, 0, 10);
  delay(250);
  dtostrf(temperature ,2, 2, hehe);
  send_message(hehe);
  Serial.printf("Temperatura agua: c%s\n", hehe);
  memset(hehe, 0, 10);
  delay(250);
  dtostrf(tdsValue , 2, 2, hehe);
  send_message(hehe);
  Serial.printf("TDS: c%s\n", hehe);
  memset(hehe, 0, 10);
  delay(250);
  dtostrf(turbidity, 2, 2, hehe);
  send_message(hehe);
  Serial.printf("Turbidez: c%s\n", hehe);
  memset(hehe, 0, 10);
  delay(250);
}

int getMedianNum(int bArray[], int iFilterLen)
{
    int bTab[iFilterLen];
    for (byte i = 0; i < iFilterLen; i++)
        bTab[i] = bArray[i];
    int i, j, bTemp;
    for (j = 0; j < iFilterLen - 1; j++)
    {
        for (i = 0; i < iFilterLen - j - 1; i++)
        {
            if (bTab[i] > bTab[i + 1])
            {
                bTemp = bTab[i];
                bTab[i] = bTab[i + 1];
                bTab[i + 1] = bTemp;
            }
        }
    }
    if ((iFilterLen & 1) > 0)
      bTemp = bTab[(iFilterLen - 1) / 2];
    else
      bTemp = (bTab[iFilterLen / 2] + bTab[iFilterLen / 2 - 1]) / 2;
    return bTemp;
}

/*
  if(my_btn.readRisingClick()){
    // Choose a random response to send back
    uint8_t idx = random(NUM_RANDOM_RESPONSES);
    strncpy(response, responses[idx], MAX_BUFFER_LEN);
    send_message(response);
    memset(response, 0, MAX_BUFFER_LEN);
  }
}
*/


