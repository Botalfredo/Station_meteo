#include <Wire.h>
#include <LiquidCrystal.h>
#include <LoRa.h>
#include <SPI.h>
#include <math.h>
#include <LiquidCrystal.h>

//==================== Constantes ====================//

// Adresses des registres du BMP180 à lire
#define BMP180_ADDR 0x77 // adresse 7-bit du BMP180
#define BMP180_REG_CONTROL 0xF4
#define BMP180_REG_RESULT 0xF6
#define BMP180_COMMAND_TEMPERATURE 0x2E
#define BMP180_COMMAND_PRESSURE 0x34 //0x34 0x74 0xB4 0xF4

// Resistances pont diviseur
#define R1 10000
#define R2 20000
// Pin de lecture de la tension batterie et de la LED
#define PIN_BAT A0
#define LED 8

// Utilisation de const pour garder la data en memoire vive

// Adresse I2C du capteur SHT21
const int SHT21_ADDRESS = 0x40;

// Variables du BMP180 correspondant aux registres que l'on va lire.
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
const int d4 = 4;    // Broche D4 de l'écran LCD
const int d5 = 5;    // Broche D5 de l'écran LCD
const int d6 = 6;    // Broche D6 de l'écran LCD
const int d7 = 7;    // Broche D7 de l'écran LCD

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

/*
@brief Fonction d'initialisation des constantes du BMP180 celon les registres
@param : /
@return char : renvoie (1) si la fonction termine.
*/
char begin(){
  double c3,c4,b1;
  Wire.begin();                 // Init de la communication I2C avec le BMP180

  if(readCompData(0xAA))        // Lecture du registre 0xAA du BMP180
    AC1=signedIntTempVar;       // Stockage de la valeur lue dans AC1 (ici signée)
  if(readCompData(0xAC))
    AC2=signedIntTempVar;
  if(readCompData(0xAE))
    AC3=signedIntTempVar;
  if(readCompData(0xB0))        // Lecture du registre 0xAA du BMP180
    AC4=unSignedIntTempVar;     // Stockage de la valeur lue dans AC4 (ici non signée)
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

  // Utilisation des données récolté par la lecture des registres pour
  // initialisé les variables que l'on utilisera lors des calculs
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

/*
@brief Fonction de lecture des rigistre du BMP180
@param char adresse: Adresse du registre à écrire pour réaliser la lecture
@return char : renvoie (1) si la fonction termine, (0) sinon.
*/
char readCompData(char address){
  unsigned char data[2];                // Tableau de 2 char
  char x;

  Wire.beginTransmission(BMP180_ADDR);  // Début de la transmission à l'adresse du BMP180
  Wire.write(address);                  // Ecriture à l'adresse qui va nous permettre de lire la donnée souhaité
  _error = Wire.endTransmission();      // Fin de la transmission une fois la donnée écrite
  if (_error == 0){                     // Si la transmission c'est bien passée (_error = 0)
    Wire.requestFrom(BMP180_ADDR,2);    // Lecture à l'adresse du BMP180 de 2 octet de data demandé
    while(Wire.available() != 2) ;      // Tant que lesdeux octet ne sont pas lus
    for(x=0;x<2;x++){                   // Pour x = 0 à 1 qui s'incrélente de 1 (2 bouclage)
      data[x] = Wire.read();            // On écris l'octet lus dans le tableau data
    }
    signedIntTempVar = (int16_t)((data[0]<<8)|data[1]);               // signedIntTempVar est la concaténation de data[0] décalé de 8 bit avec data[1] transtypé en int16_t
    unSignedIntTempVar = (((uint16_t)data[0]<<8)|(uint16_t)data[1]);  // unSignedIntTempVar est la concaténation de data[0] décalé de 8 bit avec data[1] transtypé en uint16_t
    return(1);
  }
  signedIntTempVar= unSignedIntTempVar= 0;   // Si _error != 1 on reset signedIntT & unSignedIntT
  return(0);
}

/*
@brief Fonction d'écriture pour le BMP180
@param unsigned char *values : Pointeur vers la valeur à écrire
@param char length : Taille en octet de la valeur à écrire
@return char : renvoie (1) si la fonction termine, (0) sinon.
*/
char writeBytes(unsigned char *values, char length){
  char x;

  Wire.beginTransmission(BMP180_ADDR);  // Début de la transmission à l'adresse du BMP180
  Wire.write(values,length);            // Ecriture de values d'une taille length sur le BMP180
  _error = Wire.endTransmission();      // Fin de la transmission une fois la donnée écrite 
  if (_error == 0)                      // Si _error = 0 alors le programme c'est bien terminé et on renvoie (1)
    return(1);
  else                                  // Sinon on renvoie (0)
    return(0);
} 

/*
@brief Mersure des paramètres pression et température
@param unsigned char *values : Pointeur vers la valeur à écrire
@param char length : Taille en octet de la valeur à écrire
@return char : renvoie (1) si la fonction termine, (0) sinon.
*/
char measureParameters(double &P, double &T){
  unsigned char data[3],delay1,x1;
  char result;
  double up,ut;

  data[0] = BMP180_REG_CONTROL;               // data[0] prend la valeur de l'adresse BMP180_REG_CONTROL
  data[1] = BMP180_COMMAND_TEMPERATURE;       // data[0] prend la valeur de l'adresse BMP180_COMMAND_TEMPERATURE
  writeBytes(data, 2);                        // Appel de la fonction writeBytes pour ecrire les adresse contenues dans data, et donc 2 octet la taille de data
  delay(5);                                   // delay le temps de l'écriture

  Wire.beginTransmission(BMP180_ADDR);        // Début de la transmission à l'adresse du BMP180
  Wire.write(BMP180_REG_RESULT);              // Ecriture de l'adresse de BMP180_REG_RESULT

  if (Wire.endTransmission() == 0){           // Si la transmission c'est bien terminée
    Wire.requestFrom(BMP180_ADDR,2);          // Lecture à l'adresse du BMP180 de 2 octet de data demandé
    while(Wire.available() != 2) ;            // Tant que lesdeux octet ne sont pas lus
    for(x1=0;x1<2;x1++){                      // Pour x = 0 à 1 qui s'incrélente de 1 (2 bouclage)
      data[x1] = Wire.read();                 // On écris l'octet lus dans le tableau data
    }
    ut = (data[0] * 256.0) + data[1];         // Calcult de ut (cf doc BMP180) décalage à gauche de data[0] et concaténation de deta[1]
    T = calculateTemperature(ut);             // Appel de calculateTemperature avec ut en paramètre
  }
  else
    return(0);                                // Si la transmisison c'est mal passé, renvoie (0)
                                              // On répète la même opération pour la lecture de la pression a quelques différences près
  data[0] = BMP180_REG_CONTROL;
  data[1] = BMP180_COMMAND_PRESSURE;          // data[1] prend la valeur de l'adresse BMP180_COMMAND_PRESSURE
  delay1 = 26;//5,8,14,26                     // delay de 26ms, temps de delay recommandé par la documentation
  result = writeBytes(data, 2);               
  delay(delay1);                              // delay de 26ms
  
  Wire.beginTransmission(BMP180_ADDR);
  Wire.write(BMP180_REG_RESULT);
  
  if (Wire.endTransmission() == 0){
    Wire.requestFrom(BMP180_ADDR,3);          // La donnée renvoyée pour la pression est de 3 octet
    while(Wire.available() != 3) ;            // Tant que les 3 octet n'ont pas été lus
    for(x1=0;x1<3;x1++){
      data[x1] = Wire.read();                 // Lecture de l'octet correspondant
    }
    
    up = (data[0] * 256.0) + data[1] + (data[2]/256.0); // Calcul de up : data[0] déalé à gauche concaténé avec data[1] concaténé avec data[2] décalé à droite
    P = calculatePressure(up,T);              // Appel de la fonction calculatePressure
    return(1);
  }
  return(0);
}

/*
@brief Conversion des octets lus en donnée en température
@param double ut : bits concaténé correspondant à la valeur de la temprature mesurée
@return double : Température en degrée celsius
*/
double calculateTemperature(double ut){
  double T,a;

  // Calcul de la température du BMP180 (cf doc BMP180)
  a = c5 * (ut - c6);
  T = a + (mc / (a + md));

  return T;
}

/*
@brief Conversion des octets lus en donnée en température
@param double up : bits concaténé correspondant à la valeur de la pression mesurée
@param double T : Température mesurée par le BMP180
@return double : Pression en hecto pascal
*/
double calculatePressure(double up,double T){
  double P;
  double s,x,y,z;

  // Calcul de la pression du BMP180 (cf doc BMP180)
  s = T - 25.0;
  x = (x2 * pow(s,2)) + (x1 * s) + x0;
  y = (y2 * pow(s,2)) + (y1 * s) + y0;
  z = (up - x) / y;
  P = (p2 * pow(z,2)) + (p1 * z) + p0;
  
  return P;
}

//================================setup============================//

void setup()
{
  Wire.begin();                               // Init de la communication I2C
  
  Serial.begin(9600);                         // Début de la liaison série à 9600 bauds                    
  
  lcd.begin(8, 2);                            // init du LCD lageur 2 longeure 8
  lcd.clear();                                // Efface l'écran LCD
  lcd.setCursor(0, 0);                        // Place le curseur à la position (0, 0) (i colonne, première ligne)
  lcd.print("Station ");                      // Affichage du spash screen
  lcd.setCursor(0, 1);                        // Place le curseur à la position (0, 1) (i colonne, seconde ligne)
  lcd.print(" Meteo ");                       // Affichage du spash screen
  delay(2700);                                // Delay pour laisser le splash screen s'afficher

  // défilement du splash screen
  for(int i = 0; i < 9; i++) {                // 8 case donc i de 0 à 8
    lcd.clear();                              // Efface l'écran LCD
    lcd.setCursor(i, 0);                      // Place le curseur à la position (i, 0) (i colonne, première ligne)
    lcd.print("Station ");                    // Affichage de station décalé de i
    lcd.setCursor(i, 1);                      // Place le curseur à la position (i, 1) (i colonne, seconde ligne)
    lcd.print(" Meteo ");                     // Affichage de météo décalé de i
    delay(300);                               // Attends 1/3 de seconde avant le prochain décalage
  }

  if (begin())                                // Si l'initialisation du BMP180 c'est bien faite
    Serial.println("BMP180 init success");    // Affichage du succès ou de l'echec de l'initialisation
  } else {
    Serial.println("BMP180 init fail\n\n");
    while(1);
  }
  
  while (!Serial);                            // Tant que la liaison série n'a pas démarré on attends avant de passer à la suite
  LoRa.setPins(SS_PIN, RST_PIN, MISO_PIN);    // Définir les broches SS, RST et MISO pour le LORA
  if (!LoRa.begin(848E6)) {                   // Initialisation du LoRa à 848MHz (pas 866 car la fréquenece était polluée par d'autres). Si le LoRa ne s'inistialise pas ou pas bien on affiche sur le port série l'echec
    Serial.println("Erreur lors de l'initialisation du module LoRa.");
    while (1);                                // On reste dans la condition pour ne pas casser le reste ou cas ou
  }
  LoRa.setSyncWord(0xAF);                     // On choisis un syncWord pour que seils ceux ayant se mot sur cette fréquence puissent recevoir et emetré des message qui nous correspondent.
}

//==============================loop===========================//

void loop()
{
  char status;
  double T,P,p0,a;
  
  float humidity, temperature_SHT21;
  float temperature;

  String message;
  String isOK;

  static long timer1 = millis();

//============================Humidité=========================//

  // Envoie la commande de mesure d'humidité
  Wire.beginTransmission(SHT21_ADDRESS);      // Début de la transmission à l'adresse du SHT21
  Wire.write(0xF5);                           // Commande pour mesurer l'humidité en lisant le registre 0xF5
  Wire.endTransmission();                     // Fin de la transmission
  delay(100);                                 // Attente de la mesure

  // Lit les données d'humidité
  Wire.requestFrom(SHT21_ADDRESS, 3);         // Lecture à l'adresse du BMP180 de 3 octet
  if (Wire.available() == 3) {                // Tant que il y à de la data encore disponnible
    byte msb = Wire.read();                   // lescute du msb, lsb et crc
    byte lsb = Wire.read();
    byte crc = Wire.read();

    humidity = ((unsigned)(msb << 8) | lsb) * 100.0 / 65536.0;    // calcul de l'humidité du SHT21
  }

//==========================Température========================//

  // Envoie la commande de mesure de température
  Wire.beginTransmission(SHT21_ADDRESS);      // Début de la transmission à l'adresse du SHT21
  Wire.write(0xF3);                           // Commande pour mesurer la température en lisant le registre 0xF3
  Wire.endTransmission();                     // Fin de la transmission
  delay(100);                                 // Attente de la mesure

  // Lit les données de température
  Wire.requestFrom(SHT21_ADDRESS, 3);         // Lecture à l'adresse du BMP180 de 3 octet
  if (Wire.available() == 3) {                // Tant que il y à de la data encore disponnible
    byte msb = Wire.read();                   // lescute du msb, lsb et crc
    byte lsb = Wire.read();
    byte crc = Wire.read();

    temperature_SHT21 = (((msb << 8) | lsb) * 175.72 / 65536.0) - 46.85;    // calcul de l'humidité du SHT21
    temperature_SHT21 = temperature_SHT21 - 3;                              // On retire une constante pour réduire l'erreur du SHT21
  }
//=======================Tension Bat============================//

  float tensionBatterie = mesurerTensionBatterie();       // Mesure de la tension de la batterie

//=======================Affichage==============================//
  
  // Attendre la réception d'un paquet LoRa
  int packetSize = LoRa.parsePacket();  // packetSize prends la valeur de LoRa.parsePacket() qui correspond au message recus
  
  if (packetSize) {                     // Si il y a des données
    // Lire le message reçu
    String isOK = "";
    while (LoRa.available()) {          // Tant que le LoRa est disponnible et donc qu'il y a de la donnée à lire
      isOK += (char)LoRa.read();        // isOk prends la valeur du message recus
    }
    Serial.println("Message reçu : " + isOK); // Indique que le message est recu en le marquant sur le canal série, la communication est donc bien établie
  }
  
  if (2000 < (millis()- timer1)) {      // Si 2 secondes sont passé depuis la dernière actualisation

    if (measureParameters(P,T) != 0){   // Si measureParameters à bien terminé on affiche 
      Serial.print("temperature: ");
      Serial.print(T,2);
      Serial.print(" deg C, ");
    
      Serial.print("absolute pressure: ");
      Serial.print(P,2);
      Serial.print(" mb, ");
    }
    else {                              // Si echec de measureParameters on affiche un message d'erreur
      Serial.println("error retrieving pressure measurement\n");
    }
    
    // Affiche les valeurs d'humidité et de température

    // Affichage de l'humidité et de la température venue du SHT21
    Serial.print("Humidite: ");
    Serial.print(humidity);
    Serial.print(" %\n");
    Serial.print("Temperature SHT21: ");
    Serial.print(temperature_SHT21);
    Serial.println(" °C\n\n");

    // Affichage de la tension de la batterie
    Serial.print("Tension de la batterie : ");
    Serial.print(tensionBatterie);
    Serial.println("V");

    // Affichage sur le LCD. A chaque nouvel affichage on incrémente counter de 1 pour passer à la prochaine valeur.
    // On concatnène la valeur avec une lettre pour la transmission du message pour que les valeurs soient
    // bien identifié lorsde la reception.
    if ((counter % 4) == 0) {
      lcd.clear();                      // Efface l'écran LCD
      lcd.setCursor(0, 0);              // Place le curseur à la position (0, 0) (première colonne, première ligne)
      lcd.print(" Press :");            // Affiche "Press :" pour la pression
      lcd.setCursor(0, 1);              // Place le curseur à la position (0, 0) (première colonne, seconde ligne)
      lcd.print(P);                     // Affichage de la valeur de la pression
      lcd.print(" mb");                 // Affichage de l'unitée
      message = "P" + (String)P;        // message prends la valeur de P concat"n" avec la valeur de la pression.
    }
    if ((counter % 4) == 1) {           // On répète la même opération pour la température...
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(" Temp : ");
      lcd.setCursor(0, 1);
      lcd.print(temperature_SHT21);
      //lcd.print(T); // Plus précise
      lcd.print(" C");
      message = "D" + (String)temperature_SHT21;
    }
  
    if ((counter % 4) == 2) {           // .. et pour l'humidité ...
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(" Hum : ");
      lcd.setCursor(0, 1);
      lcd.print(humidity);
      lcd.print(" %");
      message = "H" + (String)humidity;
    }

    if ((counter % 4) == 3) {           // ... et pour la batterie.
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(" Bat : ");
      lcd.setCursor(0, 1);
      lcd.print(tensionBatterie);
      lcd.print(" V");
      message = "B" + (String)tensionBatterie;
    }
    
    LoRa.beginPacket();                 // Début de l'émission du paquet
    LoRa.print((String)message);        // Envoi du message via LoRa
    LoRa.endPacket();                   // Fin de l'émission du paquet
    Serial.println("Message envoyé : " + message);

    if(isOK == "OK") {
      digitalWrite(8, HIGH);            // Si la connexion est bien établie on allume la LED
    } else {
      digitalWrite(8, LOW);             // Si la connexion n'est pas établie on éteinds la LED
    }
    
    timer1 = millis();                  // timer1 prends la valeur du temps écoulé en ms
    counter++;                          // On incrémente counter
    message = "";                       // On reset le message
  }
}
