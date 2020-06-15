 void triggerI2C(byte deviceID, byte eventID) {
  Wire.beginTransmission(deviceID);
  Wire.write(eventID);
  Wire.endTransmission();
}

void triggerAutomation(){
  // Plays random sounds or dome movements for automations when in automation mode
    unsigned long currentMillis = millis();

    if (currentMillis - automateMillis > (automateDelay * 1000)) {
      automateMillis = millis();
      automateAction = random(1, 5);

      if (automateAction > 1) {
        Play_Sound(random(32, 52)); // Sent/Hum
      }
      if (automateAction < 4) {

      //************* Move the dome for 750 msecs  **************
        
      #if defined(SYRENSIMPLE)
        Syren10.motor(turnDirection);
      #else
        Syren10.motor(1, turnDirection);
      #endif

        delay(750);

        //************* Stop the dome motor **************
      #if defined(SYRENSIMPLE)
        Syren10.motor(0);
      #else
        Syren10.motor(1, 0);
      #endif

        //************* Change direction for next time **************
        if (turnDirection > 0) {
          turnDirection = -45;
        } else {
          turnDirection = 45;
        }
      }

      // sets the mix, max seconds between automation actions - sounds and dome movement
      automateDelay = random(5,15);
    }
}

void Check_Chatpad() {

  // GENERAL SOUND PLAYBACK AND DISPLAY CHANGING
  if (Xbox.getChatpadModifierClick(MODIFER_ORANGEBUTTON)) {
    orangeToggle = !orangeToggle;
    Xbox.setChatpadLed(CHATPADLED_ORANGE, orangeToggle);
  }

  if (!orangeToggle) {

    if (Xbox.getChatpadClick(XBOX_CHATPAD_D1, 0)) {
       //Duel of the Fates 
       Play_Sound(12); // Duel of Fates 4m 17s
       //logic lights, random
       triggerI2C(10, 0);
       //magic Panel event - Flash Q
       triggerI2C(20, 28);
    }   

    if (Xbox.getChatpadClick(XBOX_CHATPAD_D2, 0)) {
         //Theme
        Play_Sound(9); // Star Wars Theme 5m 29s
        //logic lights, random
        triggerI2C(10, 0);
        //Magic Panel event - Trace up 1
        triggerI2C(20, 8);
    }   

    if (Xbox.getChatpadClick(XBOX_CHATPAD_D3, 0)) {
       //Imperial March
       Play_Sound(11); // Imperial March 3m 5s
       //logic lights, alarm2Display
       triggerI2C(10, 11);
       //HPEvent - flash - I2C
       FlthySerial.print("A0030|175\r");
       //magic Panel event - Flash V
       triggerI2C(20, 27);
    }   
    
    if (Xbox.getChatpadClick(XBOX_CHATPAD_D4, 0)) {
       //Cantina
       Play_Sound(10); // Cantina 2m 50s
       //logic lights bargraph
       triggerI2C(10, 10);
       // HPEvent 1 - Cantina Music - Disco - I2C
       FlthySerial.print("A0040|165\r");
       //magic Panel event - Trace Down 1
       triggerI2C(20, 10);
    }   

    if (Xbox.getChatpadClick(XBOX_CHATPAD_D8, 0)) {
       //Muppets
        Play_Sound(173); // Muppets Tune 30s
        //logic lights
        triggerI2C(10, 17);
        //HPEvent Disco for 30s
        FlthySerial.print("A0040|30\r");
        //Magic Panel event - Trace Up 1
        triggerI2C(20, 8);
    }   

    if (Xbox.getChatpadClick(XBOX_CHATPAD_D9, 0)) {
       //Addams
       Play_Sound(168); // Addams family tune 53s
       //logic lights
       triggerI2C(10, 19);
       //HPEvent Disco for 53s
       FlthySerial.print("A0040|53\r");
       //Magic Panel event - Flash Q
       triggerI2C(20, 28);
    }   

    if (Xbox.getChatpadClick(XBOX_CHATPAD_D0, 0)) {
       //Gangnam
       Play_Sound(169); // Gangam Styles 24s
       //logic lights
       triggerI2C(10, 18);
       //HPEvent Disco for 24s
       FlthySerial.print("A0040|24\r");
       //Magic Panel event - Flash Q
       triggerI2C(20, 28);
    }   

    if (Xbox.getChatpadClick(XBOX_CHATPAD_W, 0)) {
       //WolfWhistle
       Play_Sound(4); // Wolf whistle
       //logic lights
       triggerI2C(10, 4);
       FlthySerial.print("A00312|5\r");
       //magic Panel event - Heart
       triggerI2C(20, 40);
       // CBI panel event - Heart
       triggerI2C(30, 4);    
    }

    if (Xbox.getChatpadClick(XBOX_CHATPAD_O, 0)) {

    }
    
    if (Xbox.getChatpadClick(XBOX_CHATPAD_S, 0)) {
       //ohh (Sad Sound)
       Play_Sound(random(25, 31)); // ohh (Sad Sound)  
       //logic lights, random
       triggerI2C(10, 0);
    }

    if (Xbox.getChatpadClick(XBOX_CHATPAD_D, 0)) {
        //DOODOO
        Play_Sound(3); // DOODOO
       //logic lights, random
        triggerI2C(10, 0);
       //Magic Panel event - One loop sequence
        triggerI2C(20, 30);
    }

    if (Xbox.getChatpadClick(XBOX_CHATPAD_L, 0)) {
       Play_Sound(5); // Leia Long 35s
       //logic lights, leia message
       triggerI2C(10, 5);
       // Front HPEvent 1 - HoloMessage leia message 35 seconds
       FlthySerial.print("S1|35\r");
       //FlthySerial.print("F001|35\r");
       //magic Panel event - Cylon Row
       triggerI2C(20, 22);
    }
    
    if (Xbox.getChatpadClick(XBOX_CHATPAD_X, 0)) {
       //shortcircuit
       Play_Sound(6); // Short Circuit
       //logic lights
       triggerI2C(10, 6);
       FlthySerial.print("A0070|5\r");
       //Magic Panel event - Fade Out
       triggerI2C(20, 25);
    }

    if (Xbox.getChatpadClick(XBOX_CHATPAD_C, 0)) {
        //Chortle
        Play_Sound(2); // Chortle
        //logic lights, random
        triggerI2C(10, 0);
    }
        
  }
  else {
    if (Xbox.getChatpadClick(XBOX_CHATPAD_D1, 0)) {
        Play_Sound(3);
        //logic lights, random
        triggerI2C(10, 0);
    }
  
    if (Xbox.getChatpadClick(XBOX_CHATPAD_L, 0)) {
       //Luke message
       Play_Sound(171); // Luke Message 26s
       //logic lights
       triggerI2C(10, 15);
       //HPEvent hologram for 26s
       FlthySerial.print("S1|26\r");
       //FlthySerial.print("F001|26\r");
       //magic Panel event - Cylon Row
       triggerI2C(20, 22);
    }
  }
  
}

void Play_Sound(int Track_Num) {
 
  #ifdef MP3_YX5300
     YX5300.playTrack(Track_Num);
  #else
     mp3Trigger.play(Track_Num);  
  #endif 
 
}
