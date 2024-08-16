/*INCLUDES*/
#include <Arduino.h>
#include "WiFi.h"
#include "ThingSpeak.h"

/*MACROS*/
#define LEDPIN 2

/* WIFI */
#define WIFI_NETWORK "MA"
#define WIFI_PASSWORD "Mamama2021"
#define WIFI_TIMEOUT_MS 20000

/* ThingSpeak */
#define CHANNEL_ID 2627918
#define CHANNEL_WRITE_API_KEY "4NEXJOHINUEQNLVN"
#define CHANNEL_READ_API_KEY "XJMEC8DGO0LW7LMO"

/*GLOBAL DATA*/
WiFiClient client;  // Used by ThingSpeak to make http requests
unsigned int count = 0;
unsigned char rssi = 0;

/* TASK HANDLES */
TaskHandle_t keepWiFiAliveTaskHandle = NULL;
TaskHandle_t functionTaskHandle = NULL;

/*MY FUNCTIONS*/

/*MY TASKS*/
void keepWiFiAlive(void* parameters)
{
  unsigned long int startAttemptTime = 0;

  while(1)
  {
    //Check the WiFi connection 
    if(WiFi.status() == WL_CONNECTED) 
    {
      Serial.println("WiFi Still Connected");
      vTaskDelay(30000 / portTICK_PERIOD_MS);

    }else
    {
      Serial.println("Connecting to WiFi");
      WiFi.mode(WIFI_STA);  //WiFi mode WIFI_STA station mode
      WiFi.begin(WIFI_NETWORK, WIFI_PASSWORD);

      startAttemptTime = millis();

      //Keep looping while the wifi not connected and hasn't reached the timeout 
      while((WiFi.status() != WL_CONNECTED) && (millis() - startAttemptTime < WIFI_TIMEOUT_MS))
      {
        Serial.print(".");
        delay(100);
      }

      // In case if couldn't make a wifi connection
      if(WiFi.status() != WL_CONNECTED)
      {
        Serial.println("Failed To Connect!");
        //Wait for 20s before trying again
        vTaskDelay(20000 / portTICK_PERIOD_MS);
      }else{
        Serial.println(" Connected Successfully! IP = " + WiFi.localIP());

        // Resume other tasks after WiFi is connected
        vTaskResume(functionTaskHandle);
      }
    }
  }
}

void functionTask(void* parameters)
{
  while(1)
  {
      digitalWrite(LEDPIN, HIGH);
      count++;
      rssi = WiFi.RSSI();
      ThingSpeak.setField(1, static_cast<int>(count));
      ThingSpeak.setField(2, static_cast<int>(rssi));
      ThingSpeak.writeFields(CHANNEL_ID,CHANNEL_WRITE_API_KEY);
      digitalWrite(LEDPIN, LOW);
      vTaskDelay(15000 / portTICK_PERIOD_MS); // Update every 15s due to free usage of ThingSpeak

  }
}


/**********************************MAIN FUNCTIONS*************************************/
void setup() {

  Serial.begin(9600);
  pinMode(LEDPIN, OUTPUT);
  ThingSpeak.begin(client);


  //Creating Tasks
  xTaskCreate(
    keepWiFiAlive,
    "Keep WIFI Alive",
    5000,
    NULL,
    1,
    &keepWiFiAliveTaskHandle
  );

  xTaskCreate(
    functionTask,
    "The Function Task",
    5000,
    NULL,
    2,
    &functionTaskHandle
  );


  // Initially suspend function task until WiFi is connected
  vTaskSuspend(functionTaskHandle);
}

void loop() {
  // Do nothing here, everything is handled in tasks
}