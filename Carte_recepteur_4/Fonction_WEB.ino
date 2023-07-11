
// Remplace l'espace réservé par les valeurs des capteurs
String processor(const String& var){
  //Serial.println(var);
  if(var == "temperature_int"){
    //varable température intérieure
    return String(temperature);
  }
  else if(var == "humidite_int"){
    //varable humidité intérieur
    return String(humidity);
  }
  else if(var == "temperature_ext"){
    //varable Température extérieure
    return temp_c;
  }
  else if(var == "humidite_ext"){
    //varable humidité extérieure
    return hum_c;
  }
  else if(var == "pression_ext"){
    //varable Pression extérieure
    return press_c;
  }
  else if(var == "tension_bat"){
    //varable Tension battrie
    return bat_c;
  }
  else if(var == "Date_heure"){
    //Date et heure
    return returnHeureDS1307();
  }
  return String();
}


// Configuration serveur WEB
void setupServer(){

  // Page index du serveur
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });
  
  // Pages de CSS
  server.on("/css/styles.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/css/styles.css", "text/css");
  });
  server.on("/css/bootstrap.min.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/css/bootstrap.min.css", "text/css");
  });

  //Logo ENSTA 
  server.on("/image/logo.png", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/image/logo.png", "image/png");
  });  
}
