
// Fonction d'initalisation de l'horloge
void RTC_Setup(){
  // Initalisation client NTP
  timeClient.begin();
  // Définissez le temps de décalage en secondes pour l'adapter à votre fuseau horaire, par exemple :
  // GMT +1 = 3600
  // GMT +8 = 28800
  // GMT -1 = -3600
  // GMT 0 = 0
  timeClient.setTimeOffset(7200); // Actuellement heure d'été
  timeClient.forceUpdate();       // Actualisation heure
  formattedDate = timeClient.getFormattedDate();
  Serial.println(formattedDate);

  // Extraction de l'heure
  int splitT = formattedDate.indexOf("T");
  dayStamp = formattedDate.substring(0, splitT);
  Serial.print("DATE: ");
  Serial.println(dayStamp);
  // Extraction de la date
  timeStamp = formattedDate.substring(splitT+1, formattedDate.length()-1);
  Serial.print("HOUR: ");
  Serial.println(timeStamp);

  // Séparation des information de timeStamp
  String annee_ntp = dayStamp.substring(2, 4);
  String mois_ntp = dayStamp.substring(5, 7);
  String jour_ntp = dayStamp.substring(8, 10);

  //Détection des erreures du serveur NTP
  if(annee_ntp.toInt() == 70){ 
    Serial.println("Erreur NTP");
  }

  // Mise à jour de l'horloge RTC
  configurerHorlogeDS1307(annee_ntp.toInt(),mois_ntp.toInt(),jour_ntp.toInt(),timeClient.getHours(),timeClient.getMinutes(),timeClient.getSeconds());

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
  Serial.print(bcdToDecimal(heures));
  Serial.print(":");
  Serial.print(bcdToDecimal(minutes));
  Serial.print(":");
  Serial.print(bcdToDecimal(secondes));

  // Affichage de la date actuelle
  Serial.print(" ");
  Serial.print(bcdToDecimal(jour));
  Serial.print("/");
  Serial.print(bcdToDecimal(mois));
  Serial.print("/20");
  Serial.println(bcdToDecimal(annee));

  // Affichage sur le LCD
  lcd.setCursor(0, 0);
  if(bcdToDecimal(heures)<10){
    lcd.print("0");
  }
  lcd.print(bcdToDecimal(heures));
  lcd.print(":");
  if(bcdToDecimal(minutes)<10){
    lcd.print("0");
  }
  lcd.print(bcdToDecimal(minutes));
  lcd.print(":");
  if(bcdToDecimal(secondes)<10){
    lcd.print("0");
  }
  lcd.print(bcdToDecimal(secondes));
  lcd.print("  ");
  if(bcdToDecimal(jour)<10){
    lcd.print("0");
  }
  lcd.print(bcdToDecimal(jour));
  lcd.print("/");
  if(bcdToDecimal(mois)<10){
    lcd.print("0");
  }
  lcd.print(bcdToDecimal(mois));
  lcd.print("/20");
  lcd.print(bcdToDecimal(annee));
  
}

// return de l'heure de l'horloge DS1307
String returnHeureDS1307() {
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
  Serial.print(bcdToDecimal(heures));
  Serial.print(":");
  Serial.print(bcdToDecimal(minutes));
  Serial.print(":");
  Serial.print(bcdToDecimal(secondes));

  // Affichage de la date actuelle
  Serial.print(" ");
  Serial.print(bcdToDecimal(jour));
  Serial.print("/");
  Serial.print(bcdToDecimal(mois));
  Serial.print("/20");
  Serial.println(bcdToDecimal(annee));
  
  String R_heure   = String(bcdToDecimal(heures));
  String R_minute  = String(bcdToDecimal(minutes));
  String R_seconde = String(bcdToDecimal(secondes));

  String R_jour   = String(bcdToDecimal(jour));
  String R_mois   = String(bcdToDecimal(mois));
  String R_annee  = String(bcdToDecimal(annee));
  
  String R_date  = R_jour + "/" + R_mois + "/20" + R_annee;
  String R_temps = R_heure + ":" + R_minute + ":" + R_seconde;
  Serial.println(R_heure + " " + R_date);
  return(R_temps + " " + R_date);
}
