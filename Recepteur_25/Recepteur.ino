#include <DNSServer.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include "ESPAsyncWebServer.h"
#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include <LiquidCrystal.h>

LiquidCrystal lcd(15, 2, 0, 4, 16, 17);

// LoRa

#define SCK_PIN 5   // Broche pour SCK
#define MISO_PIN 19 // Broche pour MISO
#define MOSI_PIN 27 // Broche pour MOSI
#define SS_PIN 18   // Broche pour SS
#define RST_PIN 14  // Broche pour RST

// SI7021

#define SI7021_ADDRESS 0x70
#define SI7021_MEASURE_HUMIDITY_HOLD 0xE5
#define SI7021_MEASURE_TEMPERATURE_HOLD 0xE3

// Serveur DNSS

DNSServer dnsServer;
AsyncWebServer server(80);
String hum_c, temp_c, bat_c, press_c;

String user_name;
String proficiency;
bool name_received = false;
bool proficiency_received = false;

const char* ssid = "Station meteo";
const char* password = "aaaaaaaa";

const char* PARAM_INPUT_1 = "";
const char* PARAM_INPUT_2 = "state";

//Proto
String processor(const String& var);
String outputState(int output);
void setupServer();


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
  <h2>ESP Web Server</h2>
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

class CaptiveRequestHandler : public AsyncWebHandler {
public:
  CaptiveRequestHandler() {}
  virtual ~CaptiveRequestHandler() {}

  bool canHandle(AsyncWebServerRequest *request){
    //request->addInterestingHeader("ANY");
    return true;
  }

  void handleRequest(AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", index_html, processor); 
  }
};

void sendErrorResponse(AsyncWebServerRequest *request) {
  request->send(500, "text/plain", "Internal Server Error");
}

void setup(){
  // Serial port for debugging purposes
  Serial.begin(115200);
  Wire.begin();

  lcd.begin(16, 4); // Initialise l'écran LCD avec 8 colonnes et 2 lignes
  lcd.print("set up begin");
  lcd.clear();

  pinMode(2, OUTPUT);
  digitalWrite(2, LOW);
  Serial.println("Setting up AP Mode");
  WiFi.mode(WIFI_AP); 
  WiFi.softAP(ssid);
  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());

  Serial.println("Setting up Async WebServer");
  setupServer();
  Serial.println("Starting DNS Server");
  dnsServer.start(53, "*", WiFi.softAPIP());
  server.addHandler(new CaptiveRequestHandler()).setFilter(ON_AP_FILTER);//only when requested from AP
  //more handlers...
  server.begin();
  Serial.println("All Done!");

  SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN, SS_PIN);
  LoRa.setPins(SS_PIN, RST_PIN, -1); // -1 signifie qu'aucune broche DIO n'est utilisée

  if (!LoRa.begin(866E6)) {
    Serial.println("Erreur lors de l'initialisation du module LoRa.");
  }

  LoRa.setSyncWord(0xAF);
  Serial.println("LoRa Online");
}

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

void loop(){
  float humidity = readHumidity();
  float temperature = readTemperature();
  String message = "";

  dnsServer.processNextRequest();

  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.print("%\n");

  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.print("°C\n");

  delay(2000);

  if (name_received && proficiency_received) {
    Serial.print("Hello ");
    Serial.println(user_name);
    Serial.print("You have stated your proficiency to be ");
    Serial.println(proficiency);
    name_received = false;
    proficiency_received = false;
    Serial.println("We'll wait for the next client now");
  }

  LoRa.beginPacket();  // Début de l'émission du paquet
  LoRa.print("OK"); // Envoi du message via LoRa
  LoRa.endPacket();    // Fin de l'émission du paquet

  int packetSize = LoRa.parsePacket();
  while(!packetSize);
  if (packetSize) {
    // Read the received message
    while (LoRa.available()) {
      message += (char)LoRa.read();
    }
    switch (message[0]) {
      case 'O':
        // Traiter le cas 'O'
        //TODO Alumer la led
        break;
      case 'P':
        if (message.length() > 1) {
          message.remove(0, 1); // Supprimer le premier caractère 'P'
          press_c = message.toFloat(); // Convertir le reste de la chaîne en float
        }
        break;
      case 'H':
        if (message.length() > 1) {
          message.remove(0, 1); // Supprimer le premier caractère 'H'
          hum_c = message.toFloat(); // Convertir le reste de la chaîne en float
        }
        break;
      case 'B':
        if (message.length() > 1) {
          message.remove(0, 1); // Supprimer le premier caractère 'B'
          bat_c = message.toFloat(); // Convertir le reste de la chaîne en float
        }
        break;
      case 'T':
        if (message.length() > 1) {
          message.remove(0, 1); // Supprimer le premier caractère 'T'
          temp_c = message.toFloat(); // Convertir le reste de la chaîne en float
        }
        break;
    }
  }

  Serial.println("Message reçu : " + message);

  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print("Temperature: ");
  lcd.print(temperature);
  lcd.print(" C");
    
  lcd.setCursor(0, 2);
  lcd.print("Humidite: ");
  lcd.print(humidity);
  lcd.print(" %");
  }
}
