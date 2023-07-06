//=====================================================
//  ______               _   _             
// |  ____|             | | (_)            
// | |__ ___  _ __   ___| |_ _  ___  _ __  
// |  __/ _ \| '_ \ / __| __| |/ _ \| '_ \ 
// | | | (_) | | | | (__| |_| | (_) | | | |
// |_|  \___/|_| |_|\___|\__|_|\___/|_| |_|
//                                         
//=====================================================    

void base_screen(void){
  
}

unsigned char random_oled(void){
    unsigned int c_tete;
    c_tete = random(0,1000);
    
    if(c_tete ==1){
      oled=0;
      EEPROM.write(1, oled);
    }
    else if(c_tete ==2){
      oled=1;
      EEPROM.write(1, oled);
    }
    else if(c_tete ==3){
      oled=2;
      EEPROM.write(1, oled);
    }
      else if(c_tete ==4){
      oled=3;
      EEPROM.write(1, oled);
    }
      else if(c_tete ==5){
      oled=4;
      EEPROM.write(1, oled);
    }
}

void choix(void){
  rando = random(0,2);
  display.clearDisplay();
  display.setTextSize(3);
  display.setTextColor(WHITE);
  display.setCursor(40,20);    
  switch(rando){
    case 0:    
    display.println("NON");
    break;
    case 1:
    display.println("OUI");  
    break;
    }
   display.display(); 
   delay(200);
}

void choix2(void){
  rando = random(1,11);
  display.clearDisplay();
  display.setTextSize(3);
  display.setTextColor(WHITE);
  if (rando>9)display.setCursor(48,20);
  else display.setCursor(58,20);    
  display.println(rando);
  display.display(); 
  delay(200);
}

void choix_manu( unsigned int vmax){
  rando = random(1,vmax+1);
  display.clearDisplay();
  display.setTextSize(3);
  display.setTextColor(WHITE);
    if (rando>9999)display.setCursor(32,20);
    else if (rando>999)display.setCursor(38,20);
    else if (rando>99)display.setCursor(40  ,20);
    else if (rando>9)display.setCursor(48,20);
    else display.setCursor(58,20);  
  display.println(rando);
  display.display(); 
  
  delay(200);
}

void de_manu(void){
  unsigned char act_de_manu = 1;
  unsigned int scroll_plus = 0; 
  unsigned int scroll_moins = 0; 
  int vmax = readIntFromEEPROM(2); 
  while(act_de_manu == 1){
    if ( vmax <= 1) vmax = 2;
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(45,0);
    display.println("Tirage");
    display.drawLine(0, 8, 128,8, SSD1306_WHITE);
    display.setCursor(10,55);
    display.println("-");
    display.setCursor(60,55);
    display.println("ok");
    display.setCursor(115,55);
    display.println("+");  
    display.setCursor(0,10);
    display.println("Choisir la valeur Max");
    display.setTextSize(2);
    
    if (vmax>9999)display.setCursor(34,25);
    else if (vmax>999)display.setCursor(40,25);
    else if (vmax>99)display.setCursor(48,25);
    else if (vmax>9)display.setCursor(55,25);
    else display.setCursor(60,25);
    
    display.println(vmax);
    display.display(); 
     
    if (digitalRead(A0) == HIGH){
      scroll_moins++;
      if      (scroll_moins++ > 10000)vmax = vmax-10000;
      else if (scroll_moins++ > 1000)vmax = vmax-1000;
      else if (scroll_moins++ > 100)vmax = vmax-100;
      else if (scroll_moins++ > 10)vmax = vmax-10;
      else{ 
        vmax--;
        delay(50);
      }
      delay(50);
      }

    else if(digitalRead(A1) == HIGH){
     writeIntIntoEEPROM(2, vmax);
     choix_manu(vmax);
     while((digitalRead(A1) == HIGH));
     act_de_manu = 0;
     delay(110);
  
    }
    else if (digitalRead(A2) == HIGH){    
           scroll_plus++;
      if      (scroll_plus++ > 10000)vmax = vmax+10000;
      else if (scroll_plus++ > 1000)vmax = vmax+1000;
      else if (scroll_plus++ > 100)vmax = vmax+100;
      else if (scroll_plus++ > 10)vmax = vmax+10;
           else{ 
        vmax++;
        delay(50);
      }
      delay(50);
      }
   
    else{
      scroll_moins=0;
      scroll_plus =0;
      }
   }
}

void random_face_fonc(void){
  unsigned char rando_face = 1; 
  while(rando_face == 1){
    display.clearDisplay();
    //menu_back("Random face",20);
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    
    if (Random_face == HIGH){
      display.setCursor(42,20);
      display.println("TRUE");
      }
    else if (Random_face == LOW){
      display.setCursor(37,20);
      display.println(F("scroll"));
      }
    else {
      display.setCursor(25,20);
      display.println("ERREUR");
    }
    display.display(); 
     
    if (digitalRead(A0) == HIGH){
      Random_face = !Random_face;
      delay(110);
    }  
    if(digitalRead(A1) == HIGH){
     rando_face = 0;
     draw_menu = 0;
     while((digitalRead(A0) == HIGH));
     delay(110);
  
    }
    if (digitalRead(A2) == HIGH){    
     rando_face = 0;
     draw_menu = 1;
     while((digitalRead(A2) == HIGH));
     delay(110);
    }
    EEPROM.write(0, Random_face);
    }
}

void mute_fonc(void){
  unsigned char mute_buzzer = 1; 
  while(mute_buzzer == 1){
    display.clearDisplay();
    menu_back("Mute buzzer",30);
    display.setTextSize(2);
    display.setTextColor(WHITE);
    
    if (mute == HIGH){
      display.setCursor(42,20);
      display.println("TRUE");
      }
    else if (mute == LOW){
      display.setCursor(37,20);
      display.println("FALSE");
      }
    else {
      display.setCursor(25,20);
      display.println("ERREUR");
    }
    display.display(); 
     
    if (digitalRead(A0) == HIGH){
      mute = !mute;
      delay(110);
    }  
    if(digitalRead(A1) == HIGH){
     mute_buzzer = 0;
     draw_menu = 0;
     while((digitalRead(A0) == HIGH));
     delay(110);
  
    }
    if (digitalRead(A2) == HIGH){    
     mute_buzzer = 0;
     draw_menu = 1;
     while((digitalRead(A2) == HIGH));
     delay(110);
    }
    EEPROM.write(4, mute);
    }
}

void bombe(void){
  unsigned char bombe_true = 1; 
  while(bombe_true == 1){
    display.clearDisplay();
    menu_back("Mute buzzer",30);
    display.setTextSize(2);
    display.setTextColor(WHITE);
   
      display.setCursor(25,20);
      display.println("ERREUR");
    display.display(); 
     
    if (digitalRead(A0) == HIGH){
      mute = !mute;
      delay(110);
    }  
    if(digitalRead(A1) == HIGH){
     bombe_true = 0;
     draw_menu = 0;
     while((digitalRead(A0) == HIGH));
     delay(110);
  
    }
    if (digitalRead(A2) == HIGH){    
     bombe_true = 0;
     draw_menu = 1;
     while((digitalRead(A2) == HIGH));
     delay(110);
    }
    EEPROM.write(4, mute);
    }
}

int visage(void){
  draw_visage = 1; 
  while(draw_visage == 1){
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(50,0);
    display.println("Visage");
    menu_back("visage",7);
    display.setTextSize(3);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(56,20);
    display.write(oled+48);
    display.display(); 
     
    if (digitalRead(A0) == HIGH){
      oled = (oled + 1)%6;
      delay(80);
    }  
    if(digitalRead(A1) == HIGH){
     draw_visage = 0;
     draw_menu = 0;
     while((digitalRead(A0) == HIGH));
     delay(80);
  
    }
    if (digitalRead(A2) == HIGH){    
     draw_visage = 0;
     draw_menu = 1;
     while((digitalRead(A2) == HIGH));
     delay(80);
    }
    EEPROM.write(1, oled);
    }
}

void afficher_tete(unsigned char * adress_bitmap, unsigned long start) {
      if(millis()-start<300){ 
      display.drawBitmap(0,0, adress_bitmap, 128, 64, WHITE);
      display.display();  
    }
    else if(millis()-start<600){  
      display.drawBitmap(0,1,adress_bitmap, 128, 64, WHITE);   
     display.display();  
    }
    else{start = millis();}
  
}
