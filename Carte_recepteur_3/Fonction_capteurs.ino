float readTemperature() {
  Wire.beginTransmission(SI7021_ADDRESS);
  Wire.write(SI7021_MEASURE_TEMPERATURE_HOLD);
  Wire.endTransmission();
  delay(500);

  Wire.requestFrom(SI7021_ADDRESS, 3);
  if (Wire.available() >= 3) {
    byte msb = Wire.read();
    byte lsb = Wire.read();
    byte crc = Wire.read();

    int rawTemperature = (msb << 8) | lsb;
    float temperature = ((175.72 * rawTemperature) / 65536.0) - 46.85 + 0.42;

    return temperature;
  }
  else {
    Serial.println("\nErreur de lecture de la température");
    return 0.0; // Valeur par défaut en cas d'erreur
  }
}


float readHumidity() {
  Wire.beginTransmission(SI7021_ADDRESS);
  Wire.write(SI7021_MEASURE_HUMIDITY_HOLD);
  Wire.endTransmission();
  delay(500);

  Wire.requestFrom(SI7021_ADDRESS, 3);
  if (Wire.available() >= 3) {
    byte msb = Wire.read();
    byte lsb = Wire.read();
    byte crc = Wire.read();

    int rawHumidity = (msb << 8) | lsb;
    float humidity = ((125.0 * rawHumidity) / 65536.0) - 6.0 + 3.5;
    humidity = humidity * 0.63;

    return humidity;
  }
  else {
    Serial.println("\nErreur de lecture de l'humidité");
    return 0.0; // Valeur par défaut en cas d'erreur
  }
}
