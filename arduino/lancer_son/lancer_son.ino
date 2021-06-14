///              MP3 PLAYER PROJECT
/// http://educ8s.tv/arduino-mp3-player/
//////////////////////////////////////////
#include <Arduino.h>
#include "DFRobotDFPlayerMini.h"
#include <Adafruit_NeoPixel.h> // Charge la librairie Neo Pixel d'Adafruit utilisé pour piloter le ruban de LED

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

HardwareSerial mySerial(1);
DFRobotDFPlayerMini myDFPlayer;

const char* ssid = "LPDW-IOT";
const char* password = "LPDWIOTROUTER2015";
const char* api = "http://victorgaubin.fr/api/api.json";

HTTPClient http;

# define Start_Byte 0x7E
# define Version_Byte 0xFF
# define Command_Length 0x06
# define End_Byte 0xEF
# define Acknowledge 0x00 //Returns info with command 0x41 [0x01: info, 0x00: no info]
# define ACTIVATED LOW

#define PIXEL_PIN 12 // On définit le pin où est connecté la patte DATA du bandeau
#define PIXEL_COUNT 2 // On définit le nombre de LED compris sur le Ruban de LED soit 150 pour le ruban de 5m50

Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIXEL_COUNT, PIXEL_PIN, NEO_GRB + NEO_KHZ800); // Paramètre l'objet strip qui correspond à toute les LED du ruban

unsigned long startTime = millis();
int loopCount = 0;
String msg;
int alreadyOn = 0;
int M0HSI = 0;
int M0ESI = 0;
int M1HSI = 0;
int M1ESI = 0;

void setup () {
  mySerial.begin (9600, SERIAL_8N1, 16, 17);

  //Serial.begin (19200);
  delay(1000);
  strip.begin(); // Lance la connection
  strip.show(); // Initialise toute les led à 'off'

  Serial.begin(115200);

  WiFi.begin(ssid, password);
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
 
  Serial.println("Connected to the WiFi network");

  Serial.println();
  Serial.println(F("DFRobot DFPlayer Mini Demo"));
  Serial.println(F("Initializing DFPlayer ... (May take 3~5 seconds)"));

  if (!myDFPlayer.begin(mySerial)) {  //Use softwareSerial to communicate with mp3.
    Serial.println(F("Unable to begin:"));
    Serial.println(F("1.Please recheck the connection!"));
    Serial.println(F("2.Please insert the SD card!"));
    while(true);
  }
  Serial.println(F("DFPlayer Mini online."));

  myDFPlayer.setTimeOut(500);
}

/* Définition des couleurs */
int COLOR[3];
int OFF[3] = {0, 0, 0}; // Éteint

void loop () { 
  
  if ((WiFi.status() == WL_CONNECTED)) { //Check the current connection status
 
    HTTPClient http;
 
    http.begin(api); //Specify the URL
    int httpCode = http.GET();  //Make the request
 
    if (httpCode > 0) { //Check for the returning code
 
        String payload = http.getString();

        //strip.setBrightness(100); // Règle la luminosité à 100 % de la luminosité maximale

        DynamicJsonDocument doc(2048);
        DeserializationError error = deserializeJson(doc, payload);
        
        int M0HS = doc["matches"][0]["scoreHome"];
        int M0ES = doc["matches"][0]["scoreExt"];
        int M1HS = doc["matches"][1]["scoreHome"];
        int M1ES = doc["matches"][1]["scoreExt"];
      
        msg = Serial.readString();

        if (M0HS != M0HSI) {
          int music = doc["matches"][0]["teamHomeMusic"];

          int RP = doc["matches"][0]["teamHomeColorRP"];
          int GP = doc["matches"][0]["teamHomeColorGP"];
          int BP = doc["matches"][0]["teamHomeColorBP"];
          int RS = doc["matches"][0]["teamHomeColorRS"];
          int GS = doc["matches"][0]["teamHomeColorGS"];
          int BS = doc["matches"][0]["teamHomeColorBS"];

          int COLOR_P[3] = {RP, GP, BP};
          int COLOR_S[3] = {RS, GS, BS};
          
          ambiance(music, COLOR_P,COLOR_S);
      

          M0HSI = M0HS;
          
        } else if (M0ES != M0ESI) {
          int music = doc["matches"][0]["teamExtMusic"];

          int RP = doc["matches"][0]["teamExtColorRP"];
          int GP = doc["matches"][0]["teamExtColorGP"];
          int BP = doc["matches"][0]["teamExtColorBP"];
          int RS = doc["matches"][0]["teamExtColorRS"];
          int GS = doc["matches"][0]["teamExtColorGS"];
          int BS = doc["matches"][0]["teamExtColorBS"];

          int COLOR_P[3] = {RP, GP, BP};
          int COLOR_S[3] = {RS, GS, BS};
          
          ambiance(music, COLOR_P,COLOR_S);
      

          M0ESI = M0ES;
          
        } else if (M1HS != M1HSI) {
          int music = doc["matches"][1]["teamHomeMusic"];

          int RP = doc["matches"][1]["teamHomeColorRP"];
          int GP = doc["matches"][1]["teamHomeColorGP"];
          int BP = doc["matches"][1]["teamHomeColorBP"];
          int RS = doc["matches"][1]["teamHomeColorRS"];
          int GS = doc["matches"][1]["teamHomeColorGS"];
          int BS = doc["matches"][1]["teamHomeColorBS"];

          int COLOR_P[3] = {RP, GP, BP};
          int COLOR_S[3] = {RS, GS, BS};
          
          ambiance(music, COLOR_P,COLOR_S);
      

          M1HSI = M1HS;
          
        } else if (M1ES != M1ESI) {
          int music = doc["matches"][1]["teamExtMusic"];

          int RP = doc["matches"][1]["teamExtColorRP"];
          int GP = doc["matches"][1]["teamExtColorGP"];
          int BP = doc["matches"][1]["teamExtColorBP"];
          int RS = doc["matches"][1]["teamExtColorRS"];
          int GS = doc["matches"][1]["teamExtColorGS"];
          int BS = doc["matches"][1]["teamExtColorBS"];

          int COLOR_P[3] = {RP, GP, BP};
          int COLOR_S[3] = {RS, GS, BS};
          
          ambiance(music, COLOR_P,COLOR_S);
      

          M1ESI = M1ES;
        }
        

     } else {
      Serial.println("Error on HTTP request");
    }
 
    http.end(); //Free the resources
  }
 
  //delay(30000);
  
}

void ambiance(int MUSIC, int COLOR_LEFT[], int COLOR_RIGHT[])
{
  for(int i = 0 ; i < PIXEL_COUNT/2 ; i++)
  {
    strip.setPixelColor(i, COLOR_LEFT[0], COLOR_LEFT[1], COLOR_LEFT[2]);
  }
  
  for(int i = PIXEL_COUNT/2 ; i < PIXEL_COUNT ; i++)
  {
    strip.setPixelColor(i, COLOR_RIGHT[0], COLOR_RIGHT[1], COLOR_RIGHT[2]);
  }
  myDFPlayer.playFolder(15, MUSIC);
  strip.show();
  delay(30000);
  strip.clear();
  strip.show();
  
} 
