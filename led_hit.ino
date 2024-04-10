#include <FastLED.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define NUM_LEDS 12
#define DATA_PIN 19
#define BUTTON_PIN 26

LiquidCrystal_I2C lcd(0x27, 16, 2);

CRGB leds[NUM_LEDS];
bool ledState[NUM_LEDS]; // Array to store LED states (true = clicked, false = not clicked)
bool buttonPressed = false; // Variable pour suivre l'état du bouton
bool isclicked = false;
bool gameOver = false;

void setup() {
  lcd.init();
  lcd.backlight();
    FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    // Initialize LED states to false (not clicked)
    for (int i = 0; i < NUM_LEDS; i++) {
        ledState[i] = false;
    }
}

void loop() {
  lcd.setCursor(0, 0);
  lcd.print("ça fonctionne");
  // Display the LED states

    for (int i = 0; i < NUM_LEDS; i++) {
        // buttonPressed = digitalRead(BUTTON_PIN) == LOW;
        leds[i] = CRGB::Blue;
        FastLED.show();
        // delay(500);
        clickedBtn(i);
        if (ledState[i] == true && !gameOver) {
          leds[i] = CRGB::Green;
        } else {
          leds[i] = CRGB::Black;
        }
    }
}

void clickedBtn(int ledIndex) {
  for (int i = 0; i < 200; i++) {
    if(digitalRead(BUTTON_PIN) == LOW && !isclicked) {
      isclicked = true;
      if(ledState[ledIndex]) {
        for(int j = 0; j < NUM_LEDS; j++) {
          leds[j] = CRGB::Red;
        }
        delay(500);
        gameOver = true;
      }
      ledState[ledIndex] = true;
    }
    if(digitalRead(BUTTON_PIN) == HIGH) {
      isclicked = false;
    }
    delay(2);
  }
    if(gameOver){
      // delay(50000);
    }
}
