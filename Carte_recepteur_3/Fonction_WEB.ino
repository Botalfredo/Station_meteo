const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html>
  <p>Station m&eacute;t&eacute;o</p>
    <p>%Date_heure%</p>
  <table border="1">
    <tbody>
      <tr>
        <td>Int&eacute;rieur</td>
        <td>Ext&eacute;rieur</td>
      </tr>
      <tr>
        <td>%temperature_int%&deg;C</td>
        <td>%temperature_ext%&deg;C</td>
      </tr>
      <tr>
        <td>%humidite_int% / 100</td>
        <td>%humidite_ext% / 100</td>
      </tr>
      <tr>
        <td></td>
        <td>%pression_ext% hPa</td>
      </tr>
    </tbody>
  </table>
  <p>%tension_bat%V</p>
</html>
)rawliteral";


// Replaces placeholder with button section in your web page
String processor(const String& var){
  //Serial.println(var);
  if(var == "temperature_int"){
    //varable température intérieure
    return String(readTemperature());
  }
  else if(var == "humidite_int"){
    //varable humidité intérieur
    return String(readHumidity());
  }
  else if(var == "temperature_ext"){
    //varable humidité intérieur
    return temp_c;
  }
  else if(var == "humidite_ext"){
    //varable humidité intérieur
    return hum_c;
  }
  else if(var == "pression_ext"){
    //varable humidité intérieur
    return press_c;
  }
  else if(var == "tension_bat"){
    //varable humidité intérieur
    return bat_c;
  }
  else if(var == "Date_heure"){
    return returnHeureDS1307();
  }
  return String();
}

String outputState(int output){
  if(digitalRead(output)){
    return "checked";
  }
  else {
    return "";
  }
}

void setupServer(){
  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });
}
