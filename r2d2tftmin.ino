#include <bb_spi_lcd.h>  
#include <AnimatedGIF.h> 
#include <Preferences.h> 

// Archivos GIF activos
//#include "gif_files/radar28.h"   
#include "gif_files/radar41.h"   

#define PUSH_BUTTON_PIN 13


BB_SPI_LCD tft;
AnimatedGIF gif;
Preferences prefs;

#define GIF_COUNT 1 
const uint8_t *gifData[GIF_COUNT] = { radar41}; 
const size_t gifSizes[GIF_COUNT] = { sizeof(radar41)}; 

unsigned long lastDebounce;
unsigned long lastChangeTime = 0; 
uint8_t btnState = HIGH, lastBtnState = HIGH;
uint8_t currentGif = 0;
bool gifLoaded = false;

// Buffer estático necesario para el modo optimizado
uint8_t gifFrameBuffer[153600]; 

void *myStaticAlloc(uint32_t size) { 
  return gifFrameBuffer; 
}

void GIFDraw(GIFDRAW *pDraw) {
  if (pDraw->y == 0) tft.setAddrWindow(pDraw->iX, pDraw->iY, pDraw->iWidth, pDraw->iHeight);
  tft.pushPixels((uint16_t *)pDraw->pPixels, pDraw->iWidth);
}

void changeGif(uint8_t index) {
  gifLoaded = false;
  gif.close(); 
  tft.fillScreen(TFT_BLACK);
  delay(5); 
  
  gif.begin(GIF_PALETTE_RGB565_BE);
  if (gif.open((uint8_t *)gifData[index], gifSizes[index], GIFDraw)) {
    gif.setDrawType(GIF_DRAW_COOKED);
    if (gif.allocFrameBuf(myStaticAlloc) == GIF_SUCCESS) {
      gifLoaded = true;
      gif.playFrame(false, NULL); 
    }
  }
}

void setup() {
  pinMode(PUSH_BUTTON_PIN, INPUT_PULLUP);
  lastDebounce = millis();

  tft.begin(LCD_ILI9341, FLAGS_NONE, 40000000, 8, 18, 17, -1, -1, 9, 3);
  tft.setRotation(LCD_ORIENTATION_90);

  prefs.begin("gif-prefs", false);
  currentGif = prefs.getInt("lastGif", 0);
  prefs.end();

  if (currentGif >= GIF_COUNT) currentGif = 0;

  // En el modo antiguo (flash blanco), la pantalla parpadea al arrancar el setup() por el tft.begin()
  changeGif(currentGif);
}

void loop() {
  uint8_t reading = digitalRead(PUSH_BUTTON_PIN);
  if (reading != lastBtnState) lastDebounce = millis();
  
  if ((millis() - lastDebounce) > 50 && reading != btnState) {
    btnState = reading;
    
    if (btnState == LOW && (millis() - lastChangeTime > 500)) {
      lastChangeTime = millis();
      currentGif = (currentGif + 1) % GIF_COUNT;

      // Guardar el estado en la memoria permanente
      prefs.begin("gif-prefs", false);
      prefs.putInt("lastGif", currentGif);
      prefs.end();

     
       changeGif(currentGif);
     
    }
  }
  lastBtnState = reading;

  if (gifLoaded) {
    gif.playFrame(false, NULL);
  }
}
