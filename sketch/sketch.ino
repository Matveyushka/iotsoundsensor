#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>

#define LED D0
#define BTN D6
#define SND A0
#define RECORDING_STEPS_AMOUNT 10
#define SENDED_STEPS_AMOUNT 600
#define THRESHOLD_VALUE 200
#define OUTPUT_SPEED 115200

#define BACKEND_ADRESS "http://192.168.0.100:1234/history"

#define _SSID "437"
#define PASSWORD "orda_sasat"

void connectToWifi()
{
  WiFi.mode(WIFI_OFF);        
  delay(1000);
  WiFi.mode(WIFI_STA);        
  
  WiFi.begin(_SSID, PASSWORD); 
  Serial.println("");
 
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    Serial.print(".");
  }
 
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(_SSID);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void setup() {
  pinMode(LED, OUTPUT);
  
  delay(1000);
  Serial.begin(OUTPUT_SPEED);
  
  connectToWifi();
}

int recordSound(int readingStepsAmount)
{
  int value = 0;
  for (int i = 0; i != readingStepsAmount; i++)
  {
    int analogInput = analogRead(SND);
    if (analogInput > value)
    {
      value = analogInput;
    }
    delay(10);
  }
  return value;
}

int indicateThreshold(int value, int thresholdValue)
{
  digitalWrite(LED, value > thresholdValue ? HIGH : LOW);
}

void sendValues(int values[SENDED_STEPS_AMOUNT])
{
  Serial.println("Sending data...");  
  HTTPClient http;
  String postData;
  postData += "[";
  for (int i = 0; i != SENDED_STEPS_AMOUNT - 1; i++)
  {
    postData += String(values[i]) + ",";
  }
  postData += String(values[SENDED_STEPS_AMOUNT - 1]) + "]";
  http.begin(BACKEND_ADRESS); 
  http.addHeader("Content-Type", "text-plain");
  int httpCode = http.POST(postData);   
  String payload = http.getString();    
 
  Serial.println(httpCode);  
  Serial.println(payload);    
 
  http.end(); 
}

int onoff = 1;
int btnstate = 0;

void loop() {
  Serial.println("working");
  int values[SENDED_STEPS_AMOUNT];
  
  for (int i = 0; i != SENDED_STEPS_AMOUNT; i++)
  {
    int value = recordSound(RECORDING_STEPS_AMOUNT);
    Serial.println(String(i) + " :: " + String(value));
    indicateThreshold(value, THRESHOLD_VALUE);
    values[i] = value;
  }
  
  sendValues(values);
}
