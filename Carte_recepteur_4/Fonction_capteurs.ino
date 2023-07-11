void normalMeasurement() {
  Wire.beginTransmission(SI7034_ADDRESS);                   // Début de la transmission à l'adresse du SI7034
  Wire.write(hexCode >> 8);                                 // Ecriture de 0x78 puis ecriture de 0x66 pour faire la mesure en mode No Hold normal
  Wire.write(hexCode);
  Wire.endTransmission();                                   // Fin de la transmission une fois la donnée écrite

  delay(15);                                                // Délais pour écrire dans le registre

  uint8_t buf[6];                                           // Déclaration d'un tableaude 6 octet
  Wire.requestFrom(SI7034_ADDRESS, 6);                      // Lecture à l'adresse du SI7034 de 6 octet                     
  if (Wire.available() == 6) {                              // Tant que il y à 6 octet à lire
    buf[0] = Wire.read();     // LSB                        // On remplis le tableau avec les octet recus
    buf[1] = Wire.read();     // MSB
    buf[2] = Wire.read();     // CRC
    buf[3] = Wire.read();     // LSB
    buf[4] = Wire.read();     // MSB
    buf[5] = Wire.read();     // CRC

    temperature = readTemperature(buf[0] << 8 | buf[1]);    // Température prends la valeur de buf[0] décalé de 8 bits à gauche concaténé avec buff[1]
    humidity = readHumidity(buf[3] << 8 | buf[4]);          // Humidité prends la valeur de buf[3] décalé de 8 bits à gauche concaténé avec buff[4]
  } else {                                                  // Si il n'y avait pas de data de recus
    // Erreur de lecture
    temperature = 0;                                        // On renvoie 0
    humidity = 0;
  }
}

float readTemperature(uint16_t rawValue) {
  return -45 + 175 * (rawValue / pow(2, 16));               // Calcul de la température avec la valeur raw obtenue
}

float readHumidity(uint16_t rawValue) {
  return 100 * (rawValue / pow(2, 16));                     // Calcul de l'humidité avec la valeur raw obtenue
}
}
