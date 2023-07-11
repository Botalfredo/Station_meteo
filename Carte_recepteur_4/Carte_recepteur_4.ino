#include <WiFi.h>
#include <Wire.h>
#include <NTPClient.h> 
#include <WiFiUdp.h>
#include <LiquidCrystal.h>
#include <AsyncTCP.h>
#include <SPI.h>
#include <LoRa.h>
#include "ESPAsyncWebServer.h"
#include "SPIFFS.h"

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

// SI7034
#define SI7034_ADDRESS 0x70
#define hexCode 0x7866

//Matrice de pixel pour le symbole degré
byte Chardeg[] = {
  B00000,
  B00100,
  B01010,
  B00100,
  B00000,
  B00000,
  B00000,
  B00000
};

// Paramètres de connection WIFI
const char* ssid     = "Xiaomi 11T Pro";
const char* password = "felixleplussexy";
const char* DNS = "meteo";

//Déclaration des objets      
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


// --- Proto ---
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
void normalMeasurement();
float readTemperature(uint16_t rawValue);
float readHumidity(uint16_t rawValue);
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
  lcd.createChar(0, Chardeg);
  lcd.home();
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

  // Initalisation de l'I2C
  Wire.begin();
  
  //Définition horloge RTC
  RTC_Setup();

  //Configuration Serveur WEB
  Serial.println("Configuration serveur WEB");
  if(!SPIFFS.begin(true)){
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  //Initialisation du serveur web
  setupServer();
  server.begin();

  //Affichage Adresse IP du serveur web sur le LCD
  lcd.clear();
  lcd.setCursor(5, 0);
  lcd.print("Adresse IP");
  lcd.setCursor(2, 1);
  lcd.print(WiFi.localIP());
  delay(5000);
  lcd.clear();
  
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
    normalMeasurement();
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


// Fonction pour afficher les données capteurs sur le LCD
void PrintLCDdata() {
  lcd.createChar(0, Chardeg);
  lcd.setCursor(0, 1);
  lcd.print(press_c.toInt());
  lcd.print(" hPa");

  lcd.setCursor(0, 2);
  lcd.print("in: ");
  lcd.print((int)humidity);
  lcd.print("% ");
  lcd.print((float)temperature);
  lcd.write(byte(0)); //écriture degrée
  lcd.print("C");

  lcd.setCursor(0, 3);
  lcd.print("out:");
  lcd.print(hum_c.toInt());
  lcd.print("% ");
  lcd.print(temp_c);
  lcd.write(byte(0)); //écriture degrée
  lcd.print("C");
  lcd.setCursor(16, 3);
  lcd.print(bat_c.toFloat());
  lcd.setCursor(19, 3);
  lcd.print("V");
}


//Fonction pour traiter les données reçu du lora
void ParceData(String message) {
  //Serial.println("Message reçu : " + message);
  switch (message[0]) {
    case 'P':
      if (message.length() > 1) {
        message.remove(0, 1);         // Supprimer le premier caractère 'P'
        press_c = message.toFloat();  // Convertir le reste de la chaîne en float
        Serial.println("-Pression " + press_c);
      }
      break;
    case 'H':
      if (message.length() > 1) {
        message.remove(0, 1);       // Supprimer le premier caractère 'H'
        hum_c = message.toFloat();  // Convertir le reste de la chaîne en float
        Serial.println("-Humidite " + hum_c);
      }
      break;
    case 'B':
      if (message.length() > 1) {
        message.remove(0, 1);       // Supprimer le premier caractère 'B'
        bat_c = message.toFloat();  // Convertir le reste de la chaîne en float
        Serial.println("-Battrie " + bat_c);
      }
      break;
    case 'D':
      if (message.length() > 1) {
        message.remove(0, 1);       // Supprimer le premier caractère 'T'
        temp_c = message.toFloat(); // Convertir le reste de la chaîne en float
        Serial.println("-Temperature " + temp_c);
      }
      break;
  }
}
