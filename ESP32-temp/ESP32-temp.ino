#include <Wire.h>
#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

#define DS1307_ADDRESS 0x68

const char* ssid     = "Walala";
const char* password = "@@@@@@@@";

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

// Variables to save date and time
String formattedDate;
String dayStamp;
String timeStamp;

void setup() {
  Serial.begin(115200);
  
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

// Initialize a NTPClient to get time
  timeClient.begin();
  // Set offset time in seconds to adjust for your timezone, for example:
  // GMT +1 = 3600
  // GMT +8 = 28800
  // GMT -1 = -3600
  // GMT 0 = 0
  timeClient.setTimeOffset(7200);
  timeClient.forceUpdate();
  formattedDate = timeClient.getFormattedDate();
  Serial.println(formattedDate);

  // Extract date
  int splitT = formattedDate.indexOf("T");
  dayStamp = formattedDate.substring(0, splitT);
  Serial.print("DATE: ");
  Serial.println(dayStamp);
  // Extract time
  timeStamp = formattedDate.substring(splitT+1, formattedDate.length()-1);
  Serial.print("HOUR: ");
  Serial.println(timeStamp);
  String annee_ntp = dayStamp.substring(2, 4);
  String mois_ntp = dayStamp.substring(5, 7);
  String jour_ntp = dayStamp.substring(8, 10);

  Wire.begin();
  configurerHorlogeDS1307(annee_ntp.toInt(),mois_ntp.toInt(),jour_ntp.toInt(),timeClient.getHours(),timeClient.getMinutes(),timeClient.getSeconds());
}

void loop() {
  afficherHeureDS1307();
  delay(1000);
}

// Convertir un nombre binaire codé décimal (BCD) en décimal
int bcdToDecimal(byte bcd) {
  return ((bcd / 16) * 10) + (bcd % 16);
}

// Convertir un nombre décimal en nombre binaire codé décimal (BCD)
byte decimalToBcd(int decimal) {
  return ((decimal / 10) * 16) + (decimal % 10);
}

// Configurer l'horloge DS1307
void configurerHorlogeDS1307(int annee, int mois, int jour, int heures, int minutes, int secondes) {
  Wire.beginTransmission(DS1307_ADDRESS);
  Wire.write(0x00); // Adresse de départ pour écrire les données de l'horloge DS1307

  // Écriture de la nouvelle heure dans le format BCD
  Wire.write(decimalToBcd(secondes));
  Wire.write(decimalToBcd(minutes));
  Wire.write(decimalToBcd(heures));
  Wire.write(decimalToBcd(0)); // Jour de la semaine (non utilisé)
  Wire.write(decimalToBcd(jour));
  Wire.write(decimalToBcd(mois));
  Wire.write(decimalToBcd(annee));
  delay(10);
  Wire.endTransmission();
}

// Afficher l'heure de l'horloge DS1307
void afficherHeureDS1307() {
  Wire.beginTransmission(DS1307_ADDRESS);
  Wire.write(0x00); // Adresse de départ pour lire les données de l'horloge DS1307
  Wire.endTransmission();

  Wire.requestFrom(DS1307_ADDRESS, 7); // Lire 7 octets (secondes, minutes, heures, jour de la semaine, jour, mois, année)

  byte secondes = Wire.read() & 0x7F; // Les 7 premiers bits représentent les secondes
  byte minutes = Wire.read();
  byte heures = Wire.read() & 0x3F; // Les 6 premiers bits représentent les heures (format 24 heures)
  byte jourSemaine = Wire.read();
  byte jour = Wire.read();
  byte mois = Wire.read();
  byte annee = Wire.read();

  // Affichage de l'heure actuelle
  Serial.print("Heure: ");
  Serial.print(bcdToDecimal(heures));
  Serial.print(":");
  Serial.print(bcdToDecimal(minutes));
  Serial.print(":");
  Serial.println(bcdToDecimal(secondes));

  // Affichage de la date actuelle
  Serial.print("Date: ");
  Serial.print(bcdToDecimal(jour));
  Serial.print("/");
  Serial.print(bcdToDecimal(mois));
  Serial.print("/20");
  Serial.println(bcdToDecimal(annee));
}
