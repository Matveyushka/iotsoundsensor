#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>

#define LED D0
#define BTN D6
#define SND A0

#define THRESHOLD_VALUE 200
#define OUTPUT_SPEED_IN_BODS 115200

#define SENDED_STEPS_AMOUNT 60

#define TOUCH_SENSOR_READING_STEP_MS 100
#define TOUCH_SENSOR_ACTIVATION_TIME_MS 1000

#define SOUND_LOOP_STEP 10

#define BACKEND_ADRESS "http://192.168.43.116:1234/history"

#define _SSID "the_best_wifi"
#define PASSWORD "zaslonka"

bool isOn = false;

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
  pinMode(BTN, INPUT);
  
  delay(1000);
  Serial.begin(OUTPUT_SPEED_IN_BODS);
  
  connectToWifi();
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

void switchLedOn() { digitalWrite(LED, HIGH); }
void switchLedOff() { digitalWrite(LED, LOW); }


/*        Touch sensor loop        */
int touchSensorLastRead = -TOUCH_SENSOR_READING_STEP_MS;
int touchSensorState = 0;
void activationTouchSensorLoop()
{
  if (millis() - touchSensorLastRead >= TOUCH_SENSOR_READING_STEP_MS)
  {
    touchSensorLastRead = millis();
    touchSensorState = digitalRead(BTN) == 1 ? touchSensorState + 1 : 0;
    if (touchSensorState == TOUCH_SENSOR_ACTIVATION_TIME_MS / TOUCH_SENSOR_READING_STEP_MS)
    {
      isOn = !isOn;
      if (isOn) switchLedOn();
      else 
      {
        switchLedOff();
        resetRecorded();
      }
    }
  }
}

/*         Sound recording loop       */
int soundLoopLastRead = -SOUND_LOOP_STEP;
int values[SENDED_STEPS_AMOUNT];
int valuesPointer = 0;

int recordSteps = 10;
int recordedStep = 0;
int recordedValue = 0;
void record()
{
  int soundInput = analogRead(SND);
  if (soundInput > recordedValue) recordedValue = soundInput;
  recordedStep += 1;
  if (recordedStep == recordSteps)
  {
    Serial.println(String(valuesPointer) + " :: " + String(recordedValue));
    values[valuesPointer] = recordedValue;
    recordedStep = 0;
    recordedValue = 0;
    valuesPointer += 1;
    if (valuesPointer == SENDED_STEPS_AMOUNT)
    {
      valuesPointer = 0;
      sendValues(values);
    }
  }
}

void resetRecorded()
{
  recordedValue = 0;
  recordedStep = 0;
  valuesPointer = 0;
}

void soundLoop()
{
  if (millis() - soundLoopLastRead >= SOUND_LOOP_STEP)
  {
    soundLoopLastRead = millis();
    if (isOn)
    {
      record();
    }
  }
}

/*      Main loop      */
void loop() {
  activationTouchSensorLoop();
  soundLoop();
}
