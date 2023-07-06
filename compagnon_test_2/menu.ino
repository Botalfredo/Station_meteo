void menu(void){
  unsigned char index_action = 0;
  int pourc_bat = 0;
  while(draw_menu == 1){//variable affichage du menu
    display.clearDisplay();
    
   menu_back("MENU",1);
   
    pourc_bat = 0;
    for(int k=0 ; k<30 ; k++){
      pourc_bat = pourc_bat + map(analogRead(A3),535,625,0,100);
      delay(4);
    } 
    display.setCursor(53,0);
    display.println("MENU");
    display.setCursor(110,10);
   display.print(constrain((pourc_bat/30),0,100)); //affiche la battrie
    
    for(int i = 0 ; i<4 ; i++){
      display.setTextColor(SSD1306_WHITE);
      if(item_inverted == i)display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
      display.setCursor(1,i*9+10);
      if(index_action < 4){
        display.println(liste_action_menu1[i]); 
      }
      else if(index_action < 8){
        display.println(liste_action_menu2[i]); 
      }          
    }
    display.display();
      
    if (digitalRead(A1) == HIGH){
        if(mute == false)tone(5,2000,100);
        while((digitalRead(A1) == HIGH));
        action_menu(index_action);
    }  
    if(digitalRead(A0) == HIGH){
      item_inverted++;
      index_action++;
      if (index_action > nbr_item_menu){
        index_action=0; 
        item_inverted = 0;
     }
     if (item_inverted >= 4){
      item_inverted = 0;
     }
     
    if(mute == false)tone(5,1000,100);
      delay(110);
    }
    if (digitalRead(A2) == HIGH){ 
      if(mute == false)tone(5,2000,100);   
      draw_menu = 0;
      while((digitalRead(A2) == HIGH));
      index_action=0; 
        item_inverted = 0;
      delay(110);
    } 
  }
}

unsigned char action_menu(unsigned char action){
   switch(action){
    case 0:
      visage();
    break;
    case 1:
      random_face_fonc();
    break;
    case 2:
       de_manu();
    break;
    case 3:
       led_state = !led_state;
       digitalWrite(13,led_state);
    break; 
    case 4:   
       mute_fonc();
    break; 
    case 5:   
  delay(100);
  tone(PIN_BUZZER, 2637, 200);
  delay(400);
  tone(PIN_BUZZER, 1975, 200);
  delay(200);
  tone(PIN_BUZZER, 2093, 200);
  delay(200);
  tone(PIN_BUZZER, 2349, 200);
  delay(400);
  tone(PIN_BUZZER, 2093, 200);
  delay(200);
  tone(PIN_BUZZER, 1975, 200);
  delay(200);
  tone(PIN_BUZZER, 1760, 200);
  delay(400);
  tone(PIN_BUZZER, 1760, 200);
  delay(200);
  tone(PIN_BUZZER, 2093, 200);
  delay(200);
  tone(PIN_BUZZER, 2637, 200);
  delay(400);
  tone(PIN_BUZZER, 2349, 200);
  delay(200);
  tone(PIN_BUZZER, 2093, 200);
  delay(200);
  tone(PIN_BUZZER, 1975, 200);
  delay(400);
  tone(PIN_BUZZER, 1975, 200);
  delay(200);
  tone(PIN_BUZZER, 2093, 200);
  delay(200);
  tone(PIN_BUZZER, 2349, 200);
  delay(400);
  tone(PIN_BUZZER, 2637, 200);
  delay(400);
  tone(PIN_BUZZER, 2093, 200);
  delay(400);
  tone(PIN_BUZZER, 1760, 200);
  delay(400);
  tone(PIN_BUZZER, 1760, 200);
    break; 
    }
}

void menu_back(String nom,int correcteur){
  
  display.setTextColor(SSD1306_WHITE); // Draw white text
  display.setTextSize(1);      // Normal 1:1 pixel scale
  display.drawLine(0, 8, 128,8, SSD1306_WHITE);
  display.setCursor(0, 0);     // Start at top-left corner  
  display.setCursor(10,55);
  display.write(25);
  display.setCursor(60,55);
  display.write("ok");
  display.setCursor(110,54);
  display.write(27);
  
}


 
