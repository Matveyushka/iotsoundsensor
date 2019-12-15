#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>

#define LED D0
#define BTN D6
#define SND A0

#define THRESHOLD_VALUE 100
#define OUTPUT_SPEED_IN_BODS 115200

#define SENDED_STEPS_AMOUNT 60
#define SENDED_STEP_MS 1000
#define SHOWED_STEP_MS 100
#define READED_STEP_MS 10

#define TOUCH_SENSOR_LOOP_STEP_MS 100
#define TOUCH_SENSOR_ACTIVATION_TIME_MS 1000

#define SOUND_LOOP_STEP 10

#define BACKEND_ADRESS "http://rocky-sea-48152.herokuapp.com/history"

#define _SSID "vk.com/ilfirin_tiuru"
#define PASSWORD "sdcd0113"

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
  pinMode(D4, OUTPUT);
  digitalWrite(D4, HIGH);
  digitalWrite(LED, HIGH);
 
  Serial.begin(OUTPUT_SPEED_IN_BODS);
  
  connectToWifi();
  digitalWrite(LED, LOW);
}

void indicateSoundLevel(int lvl)
{
  digitalWrite(LED, lvl > THRESHOLD_VALUE ? HIGH : LOW);
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
  Serial.println(postData);
  int httpCode = http.POST(postData);   
  
  /*String payload = http.getString();    
  Serial.println(httpCode);  
  Serial.println(payload);*/
  
  http.end(); 
}

void switchLedOn() { digitalWrite(D4, LOW); }
void switchLedOff() { digitalWrite(D4, HIGH); }

/*         Sound recording loop       */
int values[SENDED_STEPS_AMOUNT];
int valuePointer = 0;
int showingCounter = 0;
int recordingCounter = 0;
int lastReadingTime = -READED_STEP_MS;

int recordedValue = 0;
int sendedValue = 0;

void resetRecorded()
{
  valuePointer = 0;
  showingCounter = 0;
  recordingCounter = 0;
  lastReadingTime = -READED_STEP_MS;
  recordedValue = 0;
  sendedValue = 0;
}

void record()
{
  int recorded = analogRead(SND);
  if (recorded > recordedValue)
  {
    recordedValue = recorded;
  }
  recordingCounter += 1;
  if (recordingCounter >= SHOWED_STEP_MS / READED_STEP_MS)
  {
    recordingCounter = 0;
    indicateSoundLevel(recordedValue);
    if (recordedValue > sendedValue)
    {
      sendedValue = recordedValue;
    }
    recordedValue = 0;
    showingCounter += 1;
    if (showingCounter >= SENDED_STEP_MS / SHOWED_STEP_MS)
    {
      Serial.println(String(valuePointer) + " :: " + String(sendedValue));
      showingCounter = 0;
      values[valuePointer] = sendedValue;
      sendedValue = 0;
      valuePointer += 1;
      if (valuePointer >= SENDED_STEPS_AMOUNT)
      {
        valuePointer = 0;
        sendValues(values);
      }
    }
  }
}

void soundLoop()
{
  if (millis() - lastReadingTime  >= READED_STEP_MS)
  {
    lastReadingTime = millis();
    if (isOn)
    {
      record();
    }
  }
}

/*        Touch sensor loop        */
int touchSensorLastRead = -TOUCH_SENSOR_LOOP_STEP_MS;
int touchSensorState = 0;
void activationTouchSensorLoop()
{
  if (millis() - touchSensorLastRead >= TOUCH_SENSOR_LOOP_STEP_MS)
  {
    touchSensorLastRead = millis();
    touchSensorState = digitalRead(BTN) == 1 ? touchSensorState + 1 : 0;
    if (touchSensorState == TOUCH_SENSOR_ACTIVATION_TIME_MS / TOUCH_SENSOR_LOOP_STEP_MS)
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

/*      Main loop      */
void loop() {
  activationTouchSensorLoop();
  soundLoop();
}
