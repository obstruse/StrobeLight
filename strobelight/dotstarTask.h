//#include <Adafruit_DotStar.h>
#include "dotStar.h"
#include "SPI.h"

// globals, tagged with DOTSTAR
struct {
  int       mode = 1;    // defaulting to strobe mode
  TaskHandle_t taskHandle = NULL;
  int taskCore = 0;
  uint16_t  leds = 144;
  uint32_t  clock = 15000000L;
  float     height = 1.0;
  int       strobes = 8;
  int       onTime = 0;
  int       delay = 0;
} DOTSTAR;

//--------------------------------------------
// task
//--------------------------------------------
void dotstar(void * pvParameters) {
  (void) pvParameters;

  uint16_t NUMPIXELS = DOTSTAR.leds; // Number of LEDs in strip
  uint32_t CLOCK     = DOTSTAR.clock; //clock frequency
  unsigned long startTime = 0;

  #define DATAPIN    MOSI
  #define CLOCKPIN   SCK

  //Adafruit_DotStar strip(NUMPIXELS, DATAPIN, CLOCKPIN, DOTSTAR_BGR);
  Adafruit_DotStar strip(NUMPIXELS, DOTSTAR_BGR);       // use hardware SPI

  strip.begin(); // Initialize pins for output
  SPI.setFrequency(CLOCK);  // set clock rate
  strip.setBrightness(255);
  strip.show();  // Turn all LEDs off ASAP

  for (;;) {
    DOTSTAR.taskCore = xPortGetCoreID();

    if (DOTSTAR.leds != NUMPIXELS) {
      strip.clear();
      strip.show();
      NUMPIXELS = DOTSTAR.leds;
      strip.updateLength(NUMPIXELS);
    }

    if (DOTSTAR.clock != CLOCK) {
      strip.clear();
      strip.show();
      CLOCK = DOTSTAR.clock;
      SPI.setFrequency(CLOCK);
    }

    DOTSTAR.delay = (int)(1000 * sqrt(2.0 * DOTSTAR.height / 32.17) / DOTSTAR.strobes);

    if (DOTSTAR.mode > 0 ) {
          // fall time == sqrt(2 * fallHeight / 32.17 feetPersecond)
          // strobe delayMS = 1000 * fallTime / strobesPerFall
          delay(DOTSTAR.delay);  

          strip.fill( 0xffffffff );
          startTime = micros();
          strip.show();
          strip.clear();
          strip.show();
          DOTSTAR.onTime = (micros() - startTime)/2;

    } else if (DOTSTAR.mode < 0 ) {   // flash once
          DOTSTAR.mode = 0;
          strip.fill( 0xffffffff );
          strip.show();
          strip.clear();
          strip.show();

    } else {                          // don't do anything
          yield();

    }
  }
}

//--------------------------------------------
//--------------------------------------------
void dotstarTaskCreate() {
  xTaskCreate( dotstar, "DotStar", 5000, NULL, 1, &DOTSTAR.taskHandle );
  Serial.println("... dotstar task stared");
}
