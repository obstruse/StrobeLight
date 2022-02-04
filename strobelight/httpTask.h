#ifndef HTTP_H
#define HTTP_H
#include <WebServer.h>

struct {
  TaskHandle_t taskHandle = NULL;
  int taskCore = 0;
} HTTP;

WebServer server ( 80 );
#endif

//--------------------------------------------
void handleRoot() {

  int sec = millis() / 1000;
  int min = sec / 60;
  int hr = min / 60;
        
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.sendHeader("Content-Type","text/html",true);
  server.sendHeader("Cache-Control","no-cache");
  server.send(200);
   
/*-------------------------------------------*/
  sprintf ( temp,
"<html>\
  <head>\
    <title>Strobelight</title>\
    <style>\
      body { background-color: #aaaaaa; font-family: Arial, Helvetica, Sans-Serif; Color: #000000; }\
      table, td, th, input, button { font-size: 32px; }\
    </style>\
  </head>\
  <body>");
  server.sendContent ( temp );

//--------------------------------------------
  sprintf ( temp,
"<table border='1' cellpadding='5' style='border-collapse: collapse;'>\
<form action='/settings'>\
 <tr><th colspan='2'>Strobelight</th></tr>\
 <tr><th>LEDs</th><td><input type='number' min='1' max='500' name='leds' value='%d'></td></tr>\
 <tr><th>Clock MHz</th><td><input type='number' min='0.1' max='35' step='0.1' name='clock' value='%.1f'></td></tr>\
 <tr><th>Fall Height</th><td><input type='number' min='0.001' max='500' step='0.001' name='height' value='%s'></td></tr>\
 <tr><th>Strobes/Fall</th><td><input type='number' min='1' max='500' name='strobes' value='%d'></td></tr>\
 <tr><th colspan=2><center><button type='submit' name='set'>Set</button></center></th></tr>\
</form>\
 <tr><td colspan=2></td></tr>\
 <form action='/setMode'>\
 <tr><td colspan=2><table border='0' style='width:100%%;text-align:center'><tr><td><button type ='submit' name='mode' value='1'  >Strobe</button></td>\
     <td><button type ='submit' name='mode' value='-1' >Flash</button></td></tr></table></td></tr>\
</form>\
 <tr><td colspan=2></td></tr>\
 <tr><td colspan=2>\
  <font size='3'>\
    onTime: %d usecs<br>\
    Exposure: 1/%d sec<br>\
    Delay: %d msecs<br>\
    Clock: %.1f mhz<br>\
  </font>\
 </td></tr>\
</table>\
<br>",
  DOTSTAR.leds, DOTSTAR.clock/1000000.0,
  String(DOTSTAR.height,3).c_str(),
  DOTSTAR.strobes,
  DOTSTAR.onTime, 1000000/DOTSTAR.onTime, DOTSTAR.delay,
  1.0 / DOTSTAR.onTime * 8 * ((DOTSTAR.leds * 4) + 4 + ((DOTSTAR.leds + 15) / 16))
  );
  server.sendContent ( temp );

/*
//--------------------------------------------
  sprintf ( temp,
  "\
<table border='1' cellpadding='5'>\
<tr><th colspan=2>Core</th><tr>\
<tr><th>HTTP</th><td>%d</td></tr>\
<tr><th>WIFI</th><td>%d</td></tr>\
<tr><th>I2S</th><td>%d</td></tr>\
<tr><th>DotStar</th><td>%d</td></tr>\
</table>\
<br>",
    HTTP.taskCore, 
    WIFI.taskCore,
    I2S.taskCore,
    DOTSTAR.taskCore
  );
  server.sendContent ( temp );

//--------------------------------------------
  sprintf ( temp,
  "\
<table border='1' cellpadding='5'>\
<tr><th colspan=2>Stack</th><tr>\
<tr><th>HTTP</th><td>%d</td></tr>\
<tr><th>WIFI</th><td>%d</td></tr>\
<tr><th>I2S</th><td>%d</td></tr>\
<tr><th>DotStar</th><td>%d</td></tr>\
</table>\
<br>",
    uxTaskGetStackHighWaterMark(HTTP.taskHandle),
    uxTaskGetStackHighWaterMark(WIFI.taskHandle),
    uxTaskGetStackHighWaterMark(I2S.taskHandle),
    uxTaskGetStackHighWaterMark(DOTSTAR.taskHandle)
  );
  server.sendContent ( temp );
*/

//--------------------------------------------
  sprintf ( temp,
  "\
    <p>Uptime: %02d:%02d:%02d</p>\
    <p>%s %s %s</p>\
    </body></html>",
    hr, min % 60, sec % 60,
    fileName, __DATE__, __TIME__
  );
  server.sendContent ( temp );

  server.sendContent (" ");
}

//--------------------------------------------
void settings() {

  DOTSTAR.leds    = server.arg("leds").toInt();
  DOTSTAR.clock   = server.arg("clock").toFloat() * 1000000;
  DOTSTAR.height  = server.arg("height").toFloat();
  DOTSTAR.strobes = server.arg("strobes").toInt();
  
  // 204: No Content
  //      The server successfully processed the request and is not returning any content
  //server.send ( 204, "text/plain", "");
  
  // refresh root
  server.sendHeader("Location", String("/"), true);
  server.send ( 302, "text/plain", "");
  
}

//--------------------------------------------
void setMode() {

  DOTSTAR.mode = server.arg("mode").toInt();

  // 204: No Content
  //      The server successfully processed the request and is not returning any content
  server.send ( 204, "text/plain", "");
    
}


//--------------------------------------------
void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += ( server.method() == HTTP_GET ) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  for ( uint8_t i = 0; i < server.args(); i++ ) {
    message += " " + server.argName ( i ) + ": " + server.arg ( i ) + "\n";
  }

  server.send ( 404, "text/plain", message );
}

//--------------------------------------------
// task
//--------------------------------------------
void http(void * pvParameters) {
  (void) pvParameters;
  
  for (;;) {
    HTTP.taskCore = xPortGetCoreID();
    server.handleClient();
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

//--------------------------------------------
//--------------------------------------------
void httpTaskCreate() {
  
  server.on ( "/", handleRoot );
  server.on ( "/settings", settings );
  server.on ( "/setMode", setMode );
  server.onNotFound ( handleNotFound );
  server.begin();
  
#ifndef HTTP_TASK
#define HTTP_TASK
  xTaskCreate( http, "HTTP", 5000, NULL, 1, &HTTP.taskHandle );
  Serial.println("...http task started");
#endif

}
