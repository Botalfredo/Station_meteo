#include <LiquidCrystal.h>
#include <SPI.h>
#include <LoRa.h>

#define ss 5
#define rst 14
#define dio0 2

LiquidCrystal lcd(15, 2, 0, 4, 16, 17);

void setup() {
  Serial.begin(9600);
  while (!Serial);
  Serial.println("LoRa Receiver");
  LoRa.setPins(ss, rst, dio0);

  if (!LoRa.begin(915E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }

  LoRa.setSyncWord(0xF3);
  Serial.println("LoRa Initializing OK!");
  
  lcd.begin(16, 4); // Initialise l'écran LCD avec 8 colonnes et 2 lignes
  lcd.print("set up begin");
  lcd.clear();
  
  Serial.println("LoRa Receiver");
}

void loop() {
  lcd.clear();
  lcd.setCursor(0, 0); // Place le curseur à la position (0, 0) (première colonne, première ligne)
  lcd.print("Hello, World!"); // Affiche "Hello, World!" sur l'écran LCD
  delay(1000); // Attends une seconde
  //lcd.clear(); // Efface l'écran LCD
  delay(1000); // Attends une seconde

  // try to parse packet
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    // received a packet
    Serial.print("Received packet '");

    // read packet
    while (LoRa.available()) {
      String LoRaData = LoRa.readString();
      Serial.print(LoRaData); 
    }

    // print RSSI of packet
    Serial.print("' with RSSI ");
    Serial.println(LoRa.packetRssi());
  }
}
