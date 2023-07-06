// Projet station météo 
// V2 ajoute :
// LCD
// Connection Wifi
// Gestion du temps
// retire le portail captif

#include <DNSServer.h>
#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <LiquidCrystal.h>
#include <AsyncTCP.h>
#include "ESPAsyncWebServer.h"
#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>

#define SCK_PIN 5   // Broche pour SCK
#define MISO_PIN 19 // Broche pour MISO
#define MOSI_PIN 27 // Broche pour MOSI
#define SS_PIN 18   // Broche pour SS
#define RST_PIN 14  // Broche pour RST
#define LCD_RS 15
#define LCD_EN 2
#define LCD_D4 0
#define LCD_D5 4
#define LCD_D6 16
#define LCD_D7 17
#define DS1307_ADDRESS 0x68

const char* ssid     = "Xiaomi 11T Pro";
const char* password = "felixleplussexy";
//const char* ssid = "Walala";
//const char* password = "@@@@@@@@";

DNSServer dnsServer;
AsyncWebServer server(80);
LiquidCrystal lcd(LCD_RS, LCD_EN, LCD_D4, LCD_D5, LCD_D6, LCD_D7);
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

const char* PARAM_INPUT_1 = "output";
const char* PARAM_INPUT_2 = "state";

String formattedDate;
String dayStamp;
String timeStamp;

//Proto
//Fonction WEB
String processor(const String& var);
String outputState(int output);
void setupServer();
//Fonction RTC
void RTC_Setup();
int bcdToDecimal(byte bcd);
byte decimalToBcd(int decimal);
void configurerHorlogeDS1307(int annee, int mois, int jour, int heures, int minutes, int secondes);
void afficherHeureDS1307();
String returnHeureDS1307();

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>ESP Web Server</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="icon" href="data:,">
  <style>
    html {font-family: Arial; display: inline-block; text-align: center;}
    h2 {font-size: 3.0rem;}
    p {font-size: 3.0rem;}
    body {max-width: 600px; margin:0px auto; padding-bottom: 25px;}
    .switch {position: relative; display: inline-block; width: 120px; height: 68px} 
    .switch input {display: none}
    .slider {position: absolute; top: 0; left: 0; right: 0; bottom: 0; background-color: #ccc; border-radius: 6px}
    .slider:before {position: absolute; content: ""; height: 52px; width: 52px; left: 8px; bottom: 8px; background-color: #fff; -webkit-transition: .4s; transition: .4s; border-radius: 3px}
    input:checked+.slider {background-color: #b30000}
    input:checked+.slider:before {-webkit-transform: translateX(52px); -ms-transform: translateX(52px); transform: translateX(52px)}
  </style>
</head>
<body>
  <h2>Station meteo</h2>
  %Date_heure%
  %BUTTONPLACEHOLDER%
<script>function toggleCheckbox(element) {
  var xhr = new XMLHttpRequest();
  if(element.checked){ xhr.open("GET", "/update?output="+element.id+"&state=1", true); }
  else { xhr.open("GET", "/update?output="+element.id+"&state=0", true); }
  xhr.send();
}
</script>
</body>
</html>
)rawliteral";

void setup(){
  // Serial port for debugging purposes
  pinMode(33, OUTPUT);
  digitalWrite(33, LOW);
    
  Serial.begin(115200);
  lcd.begin(20, 4);
  lcd.setCursor(3, 0);
  lcd.print("CONNEXION WIFI");

  WiFi.begin(ssid, password);
  int i = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");

    //Annimation de chargement 
    lcd.setCursor( i%14 + 3, 1);
    lcd.print(".");
    lcd.setCursor( (i+1)%14 + 3, 1);
    lcd.print(" ");
    i++;
  }
  Serial.print("AP IP address: ");Serial.println(WiFi.localIP());

  RTC_Setup();

  //WEB server
  Serial.println("Setting up Async WebServer");
  setupServer();
  //Serial.println("Starting DNS Server");
  //dnsServer.start(53, "*", WiFi.softAPIP());
  server.begin();
  Serial.println("All Done!");

  SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN, SS_PIN);
  LoRa.setPins(SS_PIN, RST_PIN, -1); // -1 signifie qu'aucune broche DIO n'est utilisée

  if (!LoRa.begin(866E6)) {
    Serial.println("Erreur lors de l'initialisation du module LoRa.");
  }

  LoRa.setSyncWord(0xAF);
  Serial.println("Lora Online");
}

void loop(){
  static unsigned long timer1 = millis();
  //Elements sycrones
  if(millis() - timer1 > 1000){
    afficherHeureDS1307();
    timer1 = millis();
  }
  
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    // Lire le message reçu
    String message = "";
    while (LoRa.available()) {
      message += (char)LoRa.read();
    }

    Serial.println("Message reçu : " + message);

    LoRa.beginPacket();  // Début de l'émission du paquet
    LoRa.print("OK"); // Envoi du message via LoRa
    LoRa.endPacket();    // Fin de l'émission du paquet
  }
}
