int mainCore = 0;

char fileName[] = __FILE__;
char temp[2000];

#include "wifiTask.h"
#include "dotstarTask.h"
#include "httpTask.h"
#include "httpWifiTask.h"

void setup() {
  Serial.begin(57600);

  Serial.println("create task: wifi");
  wifiTaskCreate();
  
  Serial.println("create task: http");
  httpTaskCreate();
  httpWifiTaskCreate();

  Serial.println("create task: dotstar");
  dotstarTaskCreate();

  delay(2000);  
}
  
void loop() {
  vTaskDelay(portMAX_DELAY);
}
