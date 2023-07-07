#include <Wire.h>
#include <LiquidCrystal.h>
#include <LoRa.h>
#include <SPI.h>
#include <math.h>
#include <LiquidCrystal.h>

//==================== Constantes ====================//

#define BMP180_ADDR 0x77 // 7-bit address
#define BMP180_REG_CONTROL 0xF4
#define BMP180_REG_RESULT 0xF6
#define BMP180_COMMAND_TEMPERATURE 0x2E
#define BMP180_COMMAND_PRESSURE 0x34 //0x34 0x74 0xB4 0xF4

#define R1 10000
#define R2 20000
#define PIN_BAT A0
#define LED 8

// Utilisation de const pour garder la data en memoire vive

// Adresse I2C du capteur SHT21
const int SHT21_ADDRESS = 0x40;

uint16_t unSignedIntTempVar,AC4,AC5,AC6;
int16_t signedIntTempVar,AC1,AC2,AC3,VB1,VB2,MB,MC,MD;

double c5,c6,mc,md,x0,x1,x2,y0,y1,y2,p0,p1,p2;
char _error;

// LoRa
int counter = 0;

#define SS_PIN 10  // Utilisez la broche de votre choix pour SS
#define RST_PIN 9  // Utilisez la broche de votre choix pour RST
#define MISO_PIN 12 // Utilisez la broche MISO (Master In Slave Out)

// Définir les broches de connexion de l'écran LCD
const int rs = 2;    // Broche RS de l'écran LCD
const int en = 3;    // Broche EN de l'écran LCD
const int d4 = 4;     // Broche D4 de l'écran LCD
const int d5 = 5;     // Broche D5 de l'écran LCD
const int d6 = 6;     // Broche D6 de l'écran LCD
const int d7 = 7;     // Broche D7 de l'écran LCD

// Initialiser l'objet LiquidCrystal
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

//====================== Fonctions ===================//

// Fonction pour mesurer la tension de la batterie avec un Arduino 328P
float mesurerTensionBatterie() {
  // Lecture de la valeur analogique sur la broche A0
  int valeurLue = analogRead(A0);

  // Conversion de la valeur lue en tension
  float tension = valeurLue * (5.0 / 1023.0); // 5.0V est la tension de référence de l'Arduino

  // Calcul de la tension réelle de la batterie en utilisant un diviseur de tension
  // Si vous utilisez un diviseur de tension, remplacez les valeurs de résistance R1 et R2 ci-dessous
  float tensionBatterie = tension * ((R1 + R2) / R2);

  return tensionBatterie;
}

char begin(){
  double c3,c4,b1;
  Wire.begin();

  if(readCompData(0xAA))
    AC1=signedIntTempVar;
  if(readCompData(0xAC))
    AC2=signedIntTempVar;
  if(readCompData(0xAE))
    AC3=signedIntTempVar;
  if(readCompData(0xB0))
    AC4=unSignedIntTempVar;
  if(readCompData(0xB2))
    AC5=unSignedIntTempVar;
  if(readCompData(0xB4))
    AC6=unSignedIntTempVar;
  if(readCompData(0xB6))
    VB1=signedIntTempVar;
  if(readCompData(0xB8))
    VB2=signedIntTempVar;
  if(readCompData(0xBA))
    MB=signedIntTempVar;
  if(readCompData(0xBC))
    MC=signedIntTempVar;
  if(readCompData(0xBE))
    MD=signedIntTempVar;

  c3 = 160.0 * pow(2,-15) * AC3;
  c4 = pow(10,-3) * pow(2,-15) * AC4;
  b1 = pow(160,2) * pow(2,-30) * VB1;
  c5 = (pow(2,-15) / 160) * AC5;
  c6 = AC6;
  mc = (pow(2,11) / pow(160,2)) * MC;
  md = MD / 160.0;
  x0 = AC1;
  x1 = 160.0 * pow(2,-13) * AC2;
  x2 = pow(160,2) * pow(2,-25) * VB2;
  y0 = c4 * pow(2,15);
  y1 = c4 * c3;
  y2 = c4 * b1;
  p0 = (3791.0 - 8.0) / 1600.0;
  p1 = 1.0 - 7357.0 * pow(2,-20);
  p2 = 3038.0 * 100.0 * pow(2,-36);
  
  return(1);
}


char readCompData(char address){
  unsigned char data[2];
  char x;

  Wire.beginTransmission(BMP180_ADDR);
  Wire.write(address);
  _error = Wire.endTransmission();
  if (_error == 0){
    Wire.requestFrom(BMP180_ADDR,2);
    while(Wire.available() != 2) ;
    for(x=0;x<2;x++){
      data[x] = Wire.read();
    }
    signedIntTempVar = (int16_t)((data[0]<<8)|data[1]);
    unSignedIntTempVar = (((uint16_t)data[0]<<8)|(uint16_t)data[1]);
    return(1);
  }
  signedIntTempVar= unSignedIntTempVar= 0;
  return(0);
}


char writeBytes(unsigned char *values, char length){
  char x;

  Wire.beginTransmission(BMP180_ADDR);
  Wire.write(values,length);
  _error = Wire.endTransmission();
  if (_error == 0)
    return(1);
  else
    return(0);
} 

char measureParameters(double &P, double &T){
  unsigned char data[3],delay1,x1;
  char result;
  double up,ut;

  data[0] = BMP180_REG_CONTROL;
  data[1] = BMP180_COMMAND_TEMPERATURE;
  writeBytes(data, 2);
  delay(5);

  Wire.beginTransmission(BMP180_ADDR);
  Wire.write(BMP180_REG_RESULT);

  if (Wire.endTransmission() == 0){
    Wire.requestFrom(BMP180_ADDR,2);
    while(Wire.available() != 2) ;
    for(x1=0;x1<2;x1++){
      data[x1] = Wire.read();
    }
    ut = (data[0] * 256.0) + data[1];
    T= calculateTemperature(ut);
  }
  else
    return(0);
 
  data[0] = BMP180_REG_CONTROL;
  data[1] = BMP180_COMMAND_PRESSURE;
  delay1 = 26;//5,8,14,26
  result = writeBytes(data, 2);
  delay(delay1);
  
  Wire.beginTransmission(BMP180_ADDR);
  Wire.write(BMP180_REG_RESULT);
  
  if (Wire.endTransmission() == 0){
    Wire.requestFrom(BMP180_ADDR,3);
    while(Wire.available() != 3) ;
    for(x1=0;x1<3;x1++){
      data[x1] = Wire.read();
    }
    
    up = (data[0] * 256.0) + data[1] + (data[2]/256.0);
    P=calculatePressure(up,T);
    return(1);
  }
  return(0);
}

double calculateTemperature(double ut){
  double T,a;

  a = c5 * (ut - c6);
  T = a + (mc / (a + md));

  return T;
}


double calculatePressure(double up,double T){
  double P;
  double s,x,y,z;
  
  s = T - 25.0;
  x = (x2 * pow(s,2)) + (x1 * s) + x0;
  y = (y2 * pow(s,2)) + (y1 * s) + y0;
  z = (up - x) / y;
  P = (p2 * pow(z,2)) + (p1 * z) + p0;
  
  return P;
}

//======================== setup =====================//

void setup()
{
  Wire.begin();
  
  Serial.begin(9600);
  
  lcd.begin(8, 2);
  lcd.clear(); // Efface l'écran LCD
  lcd.setCursor(0, 0); // Place le curseur à la position (0, 0) (première colonne, première ligne)
  lcd.print("Station ");
  lcd.setCursor(0, 1);
  lcd.print(" Meteo ");
  delay(2700); // Attends une seconde

  // Splash screen
  for(int i = 0; i < 9; i++) {
    lcd.clear(); // Efface l'écran LCD
    lcd.setCursor(i, 0); // Place le curseur à la position (0, 0) (première colonne, première ligne)
    lcd.print("Station ");
    lcd.setCursor(i, 1);
    lcd.print(" Meteo ");
    delay(300); // Attends une seconde
  }

  Serial.println("REBOOT");
  if (begin())
    Serial.println("BMP180 init success");
  else{
    Serial.println("BMP180 init fail\n\n");
    while(1);
  }
  
  while (!Serial);
  LoRa.setPins(SS_PIN, RST_PIN, MISO_PIN); // Définir les broches SS, RST et MISO
  if (!LoRa.begin(848E6)) {
    Serial.println("Erreur lors de l'initialisation du module LoRa.");
    while (1);
  }
  LoRa.setSyncWord(0xAF);
}

//======================== loop ======================//

void loop()
{
  char status;
  double T,P,p0,a;
  
  float humidity, temperature_SHT21;
  float temperature;

  String message;
  String isOK;

  static long timer1 = millis();

//=======================Humidité====================//

  // Envoie la commande de mesure d'humidité
  Wire.beginTransmission(SHT21_ADDRESS);
  Wire.write(0xF5);  // Commande pour mesurer l'humidité
  Wire.endTransmission();
  delay(100);  // Attente de la mesure

  // Lit les données d'humidité
  Wire.requestFrom(SHT21_ADDRESS, 3);
  if (Wire.available() == 3) {
    byte msb = Wire.read();
    byte lsb = Wire.read();
    byte crc = Wire.read();

    humidity = ((unsigned)(msb << 8) | lsb) * 125.0 / 65536.0 - 6.0;
  }

//======================Température====================//

  // Envoie la commande de mesure de température
  Wire.beginTransmission(SHT21_ADDRESS);
  Wire.write(0xF3);  // Commande pour mesurer la température
  Wire.endTransmission();
  delay(100);  // Attente de la mesure

  // Lit les données de température
  Wire.requestFrom(SHT21_ADDRESS, 3);
  if (Wire.available() == 3) {
    byte msb = Wire.read();
    byte lsb = Wire.read();
    byte crc = Wire.read();

    temperature_SHT21 = ((msb << 8) | lsb) * 175.72 / 65536.0 - 46.85;
  }
//=======================Tension Bat============================//

  float tensionBatterie = mesurerTensionBatterie();

//=======================Affichage==============================//
  
  // Attendre la réception d'un paquet LoRa
  int packetSize = LoRa.parsePacket();
  
  if (packetSize) {
    // Lire le message reçu
    String isOK = "";
    while (LoRa.available()) {
      isOK += (char)LoRa.read();
    }
    Serial.println("Message reçu : " + isOK);
  }
  
  if (2000 < (millis()- timer1)) {

    if (measureParameters(P,T) != 0){
      /*Serial.print("temperature: ");
      Serial.print(T,2);
      Serial.print(" deg C, ");
      Serial.print((9.0/5.0)*T+32.0,2);
      Serial.println(" deg F");
    
      Serial.print("absolute pressure: ");
      Serial.print(P,2);
      Serial.print(" mb, ");*/
      
      a = 44330.0*(1-pow(P/1013.25,1/5.255));
      /*Serial.print("computed altitude: ");
      Serial.print(a,0);
      Serial.print(" meters, ");
      Serial.print(a*3.28084,0);
      Serial.println(" feet");*/
    }
    else {
      Serial.println("error retrieving pressure measurement\n");
    }
    
    // Affiche les valeurs d'humidité et de température
    /*
    Serial.print("Humidite: ");
    Serial.print(humidity);
    Serial.print(" %\n");
    Serial.print("Temperature SHT21: ");
    Serial.print(temperature_SHT21);
    Serial.println(" °C\n\n");

    Serial.print("Tension de la batterie : ");
    Serial.print(tensionBatterie);
    Serial.println("V");
    */
    if ((counter % 4) == 0) {
      lcd.clear(); // Efface l'écran LCD
      lcd.setCursor(0, 0); // Place le curseur à la position (0, 0) (première colonne, première ligne)
      lcd.print(" Press :"); // Affiche "Hello, World!" sur l'écran LCD
      lcd.setCursor(0, 1);
      lcd.print(P);
      lcd.print(" mb");
      message = "P" + (String)P;
    }
    if ((counter % 4) == 1) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(" Temp : ");
      lcd.setCursor(0, 1);
      // lcd.print(temperature_SHT21);
      lcd.print(T); // Plus précise
      lcd.print(" C");
      message = "D" + (String)T;
    }
  
    if ((counter % 4) == 2) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(" Hum : ");
      lcd.setCursor(0, 1);
      lcd.print(humidity);
      lcd.print(" %");
      message = "H" + (String)humidity;
    }

    if ((counter % 4) == 3) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(" Bat : ");
      lcd.setCursor(0, 1);
      lcd.print(tensionBatterie);
      lcd.print(" V");
      message = "B" + (String)tensionBatterie;
    }
    
    LoRa.beginPacket();  // Début de l'émission du paquet
    LoRa.print((String)message); // Envoi du message via LoRa
    LoRa.endPacket();    // Fin de l'émission du paquet
    Serial.println("Message envoyé : " + message);
    digitalWrite(8, HIGH);

    if(isOK == "OK") {
    } else {
      digitalWrite(8, LOW);
    }
    timer1 = millis();
    counter++;
    message = "";
  }
}
