#include <DNSServer.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include "ESPAsyncWebServer.h"
#include <SPI.h>
#include <LoRa.h>

#define SCK_PIN 5   // Broche pour SCK
#define MISO_PIN 19 // Broche pour MISO
#define MOSI_PIN 27 // Broche pour MOSI
#define SS_PIN 18   // Broche pour SS
#define RST_PIN 14  // Broche pour RST

DNSServer dnsServer;
AsyncWebServer server(80);

String user_name;
String proficiency;
bool name_received = false;
bool proficiency_received = false;

const char* ssid = "Station meteo";
const char* password = "aaaaaaaa";

const char* PARAM_INPUT_1 = "output";
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



void setup(){
  // Serial port for debugging purposes
  Serial.begin(115200);

  pinMode(2, OUTPUT);
  digitalWrite(2, LOW);
  Serial.println("Setting up AP Mode");
  WiFi.mode(WIFI_AP); 
  WiFi.softAP(ssid);
  Serial.print("AP IP address: ");Serial.println(WiFi.softAPIP());
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
  Serial.println("Lora Online");
}

void loop(){
  dnsServer.processNextRequest();
  
  if(name_received && proficiency_received){
      Serial.print("Hello ");Serial.println(user_name);
      Serial.print("You have stated your proficiency to be ");Serial.println(proficiency);
      name_received = false;
      proficiency_received = false;
      Serial.println("We'll wait for the next client now");
    }
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    // Lire le message reçu
    String message = "";
    while (LoRa.available()) {
      message += (char)LoRa.read();
    }

    Serial.println("Message reçu : " + message);

    LoRa.beginPacket();  // Début de l'émission du paquet
    LoRa.print("OK"); // Envoi du message via LoRa
    LoRa.endPacket();    // Fin de l'émission du paquet
  }
}
