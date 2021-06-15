///              MP3 PLAYER PROJECT
/// http://educ8s.tv/arduino-mp3-player/
//////////////////////////////////////////
#include <Arduino.h>
#include "DFRobotDFPlayerMini.h"
#include <Adafruit_NeoPixel.h> // Charge la librairie Neo Pixel d'Adafruit utilisé pour piloter le ruban de LED

#include <WiFi.h>
#include <BluetoothSerial.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Preferences.h>
#include <ArduinoWebsockets.h>
#include "esp_http_server.h"

HardwareSerial mySerial(1);
DFRobotDFPlayerMini myDFPlayer;

const char* ssid = "";
const char* password = "";
const char* api = "http://victorgaubin.fr/api/api.json";

String translateEncryptionType(wifi_auth_mode_t encryptionType) {
 
  switch (encryptionType) {
    case (WIFI_AUTH_OPEN):
      return "Open";
    case (WIFI_AUTH_WEP):
      return "WEP";
    case (WIFI_AUTH_WPA_PSK):
      return "WPA_PSK";
    case (WIFI_AUTH_WPA2_PSK):
      return "WPA2_PSK";
    case (WIFI_AUTH_WPA_WPA2_PSK):
      return "WPA_WPA2_PSK";
    case (WIFI_AUTH_WPA2_ENTERPRISE):
      return "WPA2_ENTERPRISE";
  }
}

HTTPClient http;

#include "BluetoothSerial.h"
#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth est non actif
#endif
BluetoothSerial SerialBT;

String ssids_array[50];
String network_string;
String connected_string;

const char* pref_ssid = "";
const char* pref_pass = "";
String client_wifi_ssid;
String client_wifi_password;

const char* bluetooth_name = "Home Stadium";

long start_wifi_millis;
long wifi_timeout = 10000;
bool bluetooth_disconnect = false;

enum wifi_setup_stages { NONE, SCAN_START, SCAN_COMPLETE, SSID_ENTERED, WAIT_PASS, PASS_ENTERED, WAIT_CONNECT, LOGIN_FAILED };
enum wifi_setup_stages wifi_stage = NONE;

using namespace websockets;
WebsocketsServer socket_server;
Preferences preferences;

int incoming;
int LED_BUILTIN = 2;

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
  //wifi_stage = 0;
  mySerial.begin (9600, SERIAL_8N1, 16, 17);

  //Serial.begin (19200);
  delay(1000);
  strip.begin(); // Lance la connection
  strip.show(); // Initialise toute les led à 'off'

  Serial.begin(115200);

  SerialBT.begin("Home Stadium"); //Bluetooth device name
  Serial.println("The device started, now you can pair it with bluetooth!");

  preferences.begin("wifi_access", false);

  if (!init_wifi()) { // Connect to Wi-Fi fails
    SerialBT.register_callback(callback);
  } else {
    SerialBT.register_callback(callback_show_ip);
  }

  SerialBT.begin(bluetooth_name);
  

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
  switch (wifi_stage)
  {
    case SCAN_START:
      SerialBT.println("Scanning Wi-Fi networks");
      Serial.println("Scanning Wi-Fi networks");
      scan_wifi_networks();
      SerialBT.println("Please enter the number for your Wi-Fi");
      wifi_stage = SCAN_COMPLETE;
      break;

    case SSID_ENTERED:
      SerialBT.println("Please enter your Wi-Fi password");
      Serial.println("Please enter your Wi-Fi password");
      wifi_stage = WAIT_PASS;
      break;

    case PASS_ENTERED:
      SerialBT.println("Please wait for Wi-Fi connection...");
      Serial.println("Please wait for Wi_Fi connection...");
      wifi_stage = WAIT_CONNECT;
      preferences.putString("pref_ssid", client_wifi_ssid);
      preferences.putString("pref_pass", client_wifi_password);
      if (init_wifi()) { // Connected to WiFi
        connected_string = "ESP32 IP: ";
        connected_string = connected_string + WiFi.localIP().toString();
        SerialBT.println(connected_string);
        Serial.println(connected_string);
        bluetooth_disconnect = true;
      } else { // try again
        wifi_stage = LOGIN_FAILED;
      }
      break;

    case LOGIN_FAILED:
      SerialBT.println("Wi-Fi connection failed");
      Serial.println("Wi-Fi connection failed");
      delay(2000);
      wifi_stage = SCAN_START;
      break;
  }

  if (socket_server.poll()) {
    disconnect_bluetooth();
    auto client = socket_server.accept();
  }
  
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
 
  delay(30000);
  
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

bool init_wifi()
{
  String temp_pref_ssid = preferences.getString("pref_ssid");
  String temp_pref_pass = preferences.getString("pref_pass");
  pref_ssid = temp_pref_ssid.c_str();
  pref_pass = temp_pref_pass.c_str();

  Serial.println(pref_ssid);
  Serial.println(pref_pass);

  WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE);

  start_wifi_millis = millis();
  WiFi.begin(pref_ssid, pref_pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if (millis() - start_wifi_millis > wifi_timeout) {
      WiFi.disconnect(true, true);
      return false;
    }
  }
  return true;
}

void scan_wifi_networks()
{
  WiFi.mode(WIFI_STA);
  // WiFi.scanNetworks will return the number of networks found
  int n =  WiFi.scanNetworks();
  if (n == 0) {
    SerialBT.println("no networks found");
  } else {
    SerialBT.println();
    SerialBT.print(n);
    SerialBT.println(" networks found");
    delay(1000);
    for (int i = 0; i < n; ++i) {
      ssids_array[i + 1] = WiFi.SSID(i);
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.println(ssids_array[i + 1]);
      network_string = i + 1;
      network_string = network_string + ": " + WiFi.SSID(i) + " (Strength:" + WiFi.RSSI(i) + ")";
      SerialBT.println(network_string);
      
    }
    wifi_stage = SCAN_COMPLETE;
  }
}

void callback(esp_spp_cb_event_t event, esp_spp_cb_param_t *param)
{
  
  if (event == ESP_SPP_SRV_OPEN_EVT) {
    wifi_stage = SCAN_START;
  }

  if (event == ESP_SPP_DATA_IND_EVT && wifi_stage == SCAN_COMPLETE) { // data from phone is SSID
    int client_wifi_ssid_id = SerialBT.readString().toInt();
    client_wifi_ssid = ssids_array[client_wifi_ssid_id];
    wifi_stage = SSID_ENTERED;
  }

  if (event == ESP_SPP_DATA_IND_EVT && wifi_stage == WAIT_PASS) { // data from phone is password
    client_wifi_password = SerialBT.readString();
    client_wifi_password.trim();
    wifi_stage = PASS_ENTERED;
  }

}

void callback_show_ip(esp_spp_cb_event_t event, esp_spp_cb_param_t *param)
{
  if (event == ESP_SPP_SRV_OPEN_EVT) {
    SerialBT.print("ESP32 IP: ");
    SerialBT.println(WiFi.localIP());
    bluetooth_disconnect = true;
  }
}

void disconnect_bluetooth()
{
  delay(1000);
  Serial.println("BT stopping");
  SerialBT.println("Bluetooth disconnecting...");
  delay(1000);
  SerialBT.flush();
  SerialBT.disconnect();
  SerialBT.end();
  Serial.println("BT stopped");
  delay(1000);
  bluetooth_disconnect = false;
}
