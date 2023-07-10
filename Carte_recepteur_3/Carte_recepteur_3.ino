

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

//LoRa
#define SCK_PIN 5   // Broche pour SCK
#define MISO_PIN 19 // Broche pour MISO
#define MOSI_PIN 27 // Broche pour MOSI
#define SS_PIN 18   // Broche pour SS
#define RST_PIN 14  // Broche pour RST

//LCD
#define LCD_RS 15
#define LCD_EN 2
#define LCD_D4 0
#define LCD_D5 4
#define LCD_D6 16
#define LCD_D7 17

//RTC
#define DS1307_ADDRESS 0x68

// SI7021

#define SI7021_ADDRESS 0x70
#define SI7021_MEASURE_HUMIDITY_HOLD 0xE5
#define SI7021_MEASURE_TEMPERATURE_HOLD 0xE3

// Paramètres de connection WIFI
const char* ssid     = "Xiaomi 11T Pro";
const char* password = "felixleplussexy";
//const char* ssid = "Bbox-49156F55";
//const char* password = "911C12521962D936EA2429499F1593";

//Déclaration des objets
DNSServer dnsServer;
AsyncWebServer server(80);
LiquidCrystal lcd(LCD_RS, LCD_EN, LCD_D4, LCD_D5, LCD_D6, LCD_D7);
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

//Varaible NTP
String formattedDate;
String dayStamp;
String timeStamp;

//variable carte capteur
String hum_c, temp_c, bat_c, press_c;
float humidity, temperature;


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
//Fonction capteur
float readTemperature();
float readHumidity();
//Fonction main
void PrintLCDdata();
void ParceData(String message);

void setup() {
  //Déclaration de la LED
  pinMode(33, OUTPUT);
  digitalWrite(33, LOW);

  //Déclaration du port série
  Serial.begin(115200);

  //Déclaration de l'écran LCD
  lcd.begin(20, 4);
  lcd.setCursor(3, 0);
  lcd.print("CONNEXION WIFI");

  //Connection WiFi
  WiFi.begin(ssid, password);
  int i = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");

    //Annimation de chargement
    lcd.setCursor( i % 14 + 3, 1);
    lcd.print(".");
    lcd.setCursor( (i + 1) % 14 + 3, 1);
    lcd.print(" ");
    i++;
  }
  Serial.print("Adresse IP: "); Serial.println(WiFi.localIP());

  //Définition horloge RTC
  RTC_Setup();

  //Configuration Serveur WEB
  Serial.println("Configuration serveur WEB");
  setupServer();
  server.begin();

  //Configuration LORA
  SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN, SS_PIN);
  LoRa.setPins(SS_PIN, RST_PIN, -1); // -1 signifie qu'aucune broche DIO n'est utilisée

  if (!LoRa.begin(848E6)) {
    Serial.println("Erreur lors de l'initialisation du module LoRa.");
  }
  LoRa.setSyncWord(0xAF);
}

void loop() {
  //Déclaration des TIMERS
  static unsigned long timer1 = millis();
  static unsigned long timer2 = millis();

  //Fonction synchrones
  //Toute les secondes
  if (millis() - timer1 > 1000) {

    PrintLCDdata();
    afficherHeureDS1307();

    timer1 = millis();
  }
  //Toute les 5 secondes
  if (millis() - timer2 > 5000) {
    humidity = readHumidity();
    temperature = readTemperature();
    timer2 = millis();
  }

  //Lecture receptions données LoRa
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    digitalWrite(33, 1);
    // Lire le message reçu
    String message = "";
    while (LoRa.available()) {
      message += (char)LoRa.read();
    }
    ParceData(message);
    digitalWrite(33, 0);
  }
}



void PrintLCDdata() {
  lcd.setCursor(0, 1);
  lcd.print(press_c.toInt());
  lcd.print(" hPa");

  lcd.setCursor(0, 2);
  lcd.print("in: ");
  lcd.print((int)humidity);
  lcd.print("% ");
  lcd.print((int)temperature);
  lcd.print("*C");

  lcd.setCursor(0, 3);
  lcd.print("out:");
  lcd.print(hum_c.toInt());
  lcd.print("% ");
  lcd.print(temp_c);
  lcd.print("*C ");
  lcd.print(bat_c.toInt());
  lcd.print("V");
}


void ParceData(String message) {
  //Serial.println("Message reçu : " + message);
  switch (message[0]) {
    case 'P':
      if (message.length() > 1) {
        message.remove(0, 1); // Supprimer le premier caractère 'P'
        press_c = message.toFloat(); // Convertir le reste de la chaîne en float
        Serial.println("-Pression " + press_c);
      }
      break;
    case 'H':
      if (message.length() > 1) {
        message.remove(0, 1); // Supprimer le premier caractère 'H'
        hum_c = message.toFloat(); // Convertir le reste de la chaîne en float
        Serial.println("-Humiditee " + hum_c);
      }
      break;
    case 'B':
      if (message.length() > 1) {
        message.remove(0, 1); // Supprimer le premier caractère 'B'
        bat_c = message.toFloat(); // Convertir le reste de la chaîne en float
        Serial.println("-Battrie " + bat_c);
      }
      break;
    case 'D':
      if (message.length() > 1) {
        message.remove(0, 1); // Supprimer le premier caractère 'T'
        temp_c = message.toFloat(); // Convertir le reste de la chaîne en float
        Serial.println("-Temperature " + temp_c);
      }
      break;
  }


}
