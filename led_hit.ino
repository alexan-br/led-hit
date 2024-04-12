#include <FastLED.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>

#define NUM_LEDS 12
#define DATA_PIN 19
#define BUTTON_PIN 26

LiquidCrystal_I2C lcd(0x27, 16, 2);
CRGB leds[NUM_LEDS];

bool ledState[NUM_LEDS]; // Array to store LED states (true = clicked, false = not clicked)
bool buttonPressed = false; // Variable pour suivre l'état du bouton
bool isclicked = false;
bool started = false;
bool gameOver = false;
int vitesse = 400;
int NombreLedValide = 0;
int score = 0; 
int niveauActuelle = 1;

const char* ssid = "HUAWEI P30 lite";
const char* password = "bakanoconnection";
// Set your server URL
WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

const int led = 2;
bool etatLed = 0;
char texteEtatLed[2][10] = {"ÉTEINTE!","ALLUMÉE!"};

void webSocketEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t length) {
  switch (type) {
    case WStype_DISCONNECTED:
      Serial.printf("[%u] Déconnecté!\n", num);
      break;
    case WStype_CONNECTED:
      {
        IPAddress ip = webSocket.remoteIP(num);
        Serial.printf("[%u] Connecté à: %d.%d.%d.%d\n", num, ip[0], ip[1], ip[2], ip[3]);
        break;
      }
    case WStype_TEXT:
      {
        String message = String((char*)payload);
        Serial.printf("[%u] Reçu un message texte: %s\n", num, message.c_str());
        break;
      }
    case WStype_BIN:
      Serial.printf("[%u] Reçu un message binaire de longueur: %u\n", num, length);
      break;
    default:
      break;
  }
}

void handleRoot()
{
    String page = "<!DOCTYPE html>";

    page += "<html lang='fr'>";

    page += "<head>";
    page += "    <title>LeaderBoard</title>";
    page += "    <meta charset='UTF-8' />";
    page += "    <meta name='viewport' content='width=device-width, initial-scale=1.0' />";
    page += "</head>";

    page += "<body>";
    page += "    <h1>Leds Hit</h1>";
    page += "    <img src='https://i.pinimg.com/originals/ba/ce/57/bace57c5e51b79fa303026d754fef8b5.gif'>";
    page += "    <p class='niveau-en-direct'>Niveau : <span id='niveau'>";page += niveauActuelle; +"</span></p>";
    page += "    <p class='score-en-direct'>Score : <span id='score'>";page += score; +"</span></p>";
    page += "    <style> @import url('https://fonts.googleapis.com/css2?family=Archivo+Black&display=swap'); * { margin: 0; padding: 0; box-sizing: border-box; } body { color: white; display: flex; justify-content: center; flex-direction: column; align-items: center; height: 100vh; font-family: 'Archivo Black', sans-serif; font-weight: 400; font-style: normal; background-color: #010000; } h1 { font-size: 3rem; margin-bottom: 1rem; } p { font-size: 5rem; margin-bottom: 1rem; } span { font-weight: bold; } </style>";
    page += "<script type='text/javascript'>";
    page += "document.addEventListener('DOMContentLoaded', () => {";
    page += "    const score = document.getElementById('score');";
    page += "    const niveau = document.getElementById('niveau');";
    page += "    const perdu = document.querySelector('.perdu');";
    page += "    const scoreFinal = ";page += score; +";";
    page += "    ;var socketScore = new WebSocket('ws://' + window.location.hostname + ':81/');";
    page += "    socketScore.onmessage = function(event) {";
    page += "        document.getElementById('score').innerHTML = event.data;";
    page += "        niveau.textContent = Math.floor(parseInt(event.data) / 12) + 1;";
    page += "    };";
    page += "});";
    page += "</script>";
    page += "</body>";

    page += "</html>";


    server.setContentLength(page.length());
    server.send(200, "text/html", page);
}
void handleOn()
{
    etatLed = 1;
    digitalWrite(led, HIGH);
    server.sendHeader("Location","/");
    server.send(303);
}

void handleOff()
{
    etatLed = 0;
    digitalWrite(led, LOW);
    server.sendHeader("Location","/");
    server.send(303);
}

void handleNotFound()
{
    server.send(404, "text/plain", "404: Not found");
}

void setup() {
  Serial.begin(9600);
  WiFi.persistent(false);
    WiFi.begin(ssid, password);
    Serial.print("Tentative de connexion...");

    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(100);
    }
  server.on("/", handleRoot);
    // server.on("/on", handleOn);
    // server.on("/off", handleOff);
    server.onNotFound(handleNotFound);
    server.begin();
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
    
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
  server.handleClient();
  webSocket.loop();

  if(!started && !gameOver) {
    score = 0;
     NombreLedValide = 0;
     niveauActuelle = 1;
     vitesse = 400;
     for (int LedPosition = 0; LedPosition < NUM_LEDS; LedPosition++) {
          leds[LedPosition] = CRGB::Black;
          ledState[LedPosition] = false;
          FastLED.show();
       }
    lcd.setCursor(0, 0);
    lcd.print("    Jouer !    ");
    lcd.setCursor(3, 1);
    lcd.print("v Clique v");
 
    if (digitalRead(BUTTON_PIN) == LOW) { // Vérifie si le bouton est enfoncé
      while (digitalRead(BUTTON_PIN) == LOW) {} // Attend que le bouton soit relâché pour éviter le rebondissement
      started = true; // Inverse l'état de démarrage lorsque le bouton est enfoncé
    }
 
  }
  if(!gameOver && started){
    LoopDeJeu();
  }
  if(gameOver){
      lcd.setCursor(0, 0);
      lcd.print("   Perdu!   ");
        for (int LedPosition = 0; LedPosition < NUM_LEDS; LedPosition++) {
          leds[LedPosition] = CRGB::Red;
          ledState[LedPosition] = false;
          FastLED.show();
        }
 
        if (digitalRead(BUTTON_PIN) == LOW) { // Vérifie si le bouton est enfoncé
        while (digitalRead(BUTTON_PIN) == LOW) {} // Attend que le bouton soit relâché pour éviter le rebondissement
          gameOver = false; // Inverse l'état de démarrage lorsque le bouton est enfoncé
          started = false; // Inverse l'état de démarrage lorsque le bouton est 
      }
  }
}
 
void LoopDeJeu(){
   for (int LedPosition = 0; LedPosition < NUM_LEDS; LedPosition++) {
      leds[LedPosition] = CRGB::Blue;
      FastLED.show();
 
      lcd.setCursor(0, 0);
      lcd.print("Niveau : ");
      lcd.print(niveauActuelle);
      lcd.print("   ");
      lcd.setCursor(3, 1);
      lcd.print("Score: ");
      lcd.print(score);
      lcd.print("   ");
 
      for (int j = 0; j < vitesse; j++) {
        if(digitalRead(BUTTON_PIN) == LOW && !isclicked) {
          score++;
          isclicked = true; // Utilisez l'opérateur d'assignation pour mettre isclicked à true
          webSocket.broadcastTXT(""+String(score));
          if(ledState[LedPosition] == true) {
            LedPosition = 11;
            gameOver = true;
            score = 0;
          }
          ledState[LedPosition] = true;  
          NombreLedValide = NombreLedValide + 1;
 
          if(NombreLedValide == NUM_LEDS){
            lcd.setCursor(0, 0);
            NombreLedValide = 0;
              vitesse = vitesse/1.5;
              niveauActuelle = niveauActuelle + 1;
               for (int ResetLed = 0; ResetLed < NUM_LEDS; ResetLed++) {
                  ledState[ResetLed] = false;
                  leds[ResetLed] = CRGB::Black;
                  FastLED.show();
              }
          }
        }
 
        if(digitalRead(BUTTON_PIN) == HIGH) {
          isclicked = false;
        }
 
        delay(1);
      }
 
      if (ledState[LedPosition] == true) {
        leds[LedPosition] = CRGB::Green;
      } else {
        leds[LedPosition] = CRGB::Black;
      }
  } 
}
