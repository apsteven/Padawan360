//============================
//    MAIN LOOP             //
//============================

void loop() {
  Usb.Task();
  // if we're not connected, return so we don't bother doing anything else.
  // set all movement to 0 so if we lose connection we don't have a runaway droid!
  // a restraining bolt and jawa droid caller won't save us here!
  if (!Xbox.XboxReceiverConnected || !Xbox.Xbox360Connected[0]) {
    Sabertooth2x.drive(0);
    Sabertooth2x.turn(0);
    Syren10.motor(1, 0);
    firstLoadOnConnect = false;
    // If controller is disconnected, but was in automation mode, then droid will continue
    // to play random sounds and dome movements
    if(isInAutomationMode){
      triggerAutomation();
    }
    if(!manuallyDisabledController){     
    }
    return;
  }

  // After the controller connects, Blink all the LEDs so we know drives are disengaged at start
  if (!firstLoadOnConnect) {
    firstLoadOnConnect = true;
     Play_Sound(21);
    Xbox.setLedMode(ROTATING, 0);
    manuallyDisabledController=false;
    //triggerI2C(10, 0);
    //domeData.ctl = 1; domeData.dsp = 0; ET.sendData();  
    //Tell the dome that the controller is now connected
  }
  
  if (Xbox.getButtonClick(XBOX, 0)) {
    if(Xbox.getButtonPress(L1, 0) && Xbox.getButtonPress(R1, 0)){ 
      manuallyDisabledController=true;
      Xbox.disconnect(0);
    }
  }

  // enable / disable right stick (droid movement) & play a sound to signal motor state
  if (Xbox.getButtonClick(START, 0)) {
    if (isDriveEnabled) {
      isDriveEnabled = false;
      Xbox.setLedMode(ROTATING, 0);
      Play_Sound(53); // 3 Beeps
    } else {
      isDriveEnabled = true;
      Play_Sound(52); // 2 Beeps
      // //When the drive is enabled, set our LED accordingly to indicate speed
      if (drivespeed == DRIVESPEED1) {
        Xbox.setLedOn(LED1, 0);
      } else if (drivespeed == DRIVESPEED2 && (DRIVESPEED3 != 0)) {
        Xbox.setLedOn(LED2, 0);
      } else {
        Xbox.setLedOn(LED3, 0);
      }
    }
  }

  //Toggle automation mode with the BACK button
  if (Xbox.getButtonClick(BACK, 0)) {
    if (isInAutomationMode) {
      isInAutomationMode = false;
      automateAction = 0;
      Play_Sound(53); // 3 Beeps
      FlthySerial.print("S4\r");//S4 function ssloan
    } else {
      isInAutomationMode = true;
      Play_Sound(52); // 2 Beeps
      FlthySerial.print("S6\r");//S6 function
    }
  }

  // Plays random sounds or dome movements for automations when in automation mode
  if (isInAutomationMode) {
    triggerAutomation();
}

  // Volume Control of MP3 Trigger
  // Hold R1 and Press Up/down on D-pad to increase/decrease volume
  if (Xbox.getButtonClick(DOWN, 0)) {
    // volume down
    if (Xbox.getButtonPress(R1, 0)) {
      
       #ifdef MP3_YX5300
           YX5300.volumeDec();
      #else    
        if (vol > 0) {
          vol--;
          mp3Trigger.setVolume(vol);  
        }
      #endif
      
    }
  }
  if (Xbox.getButtonClick(UP, 0)) {
    //volume up
    if (Xbox.getButtonPress(R1, 0)) {

       #ifdef MP3_YX5300
           YX5300.volumeInc();
      #else      
        if (vol < 255) {
          vol++;
          mp3Trigger.setVolume(vol);  
        }
      #endif
      
    }
  }

  // Logic display brightness.
  // Hold L1 and press up/down on dpad to increase/decrease brightness
  if (Xbox.getButtonClick(UP, 0)) {
    if (Xbox.getButtonPress(L1, 0)) {
      triggerI2C(10, 24);
    }
  }
  if (Xbox.getButtonClick(DOWN, 0)) {
    if (Xbox.getButtonPress(L1, 0)) {
      triggerI2C(10, 25);
    }
  }


  //FIRE EXTINGUISHER
  // When holding L2-UP, extinguisher is spraying. When released, stop spraying

  // TODO: ADD SERVO DOOR OPEN FIRST. ONLY ALLOW EXTINGUISHER ONCE IT'S SET TO 'OPENED'
  // THEN CLOSE THE SERVO DOOR
  if (Xbox.getButtonPress(L1, 0)) {
    if (Xbox.getButtonPress(UP, 0)) {
      digitalWrite(EXTINGUISHERPIN, LOW);
    } else {
      digitalWrite(EXTINGUISHERPIN, HIGH);
    }
  }

#if defined(CHATPAD)
  Check_Chatpad();
#endif

  // Y Button and Y combo buttons
  if (Xbox.getButtonClick(Y, 0)) {
     if (Xbox.getButtonPress(L1, 0) && Xbox.getButtonPress(R1, 0)) {
       //Addams
       Play_Sound(168); // Addams family tune 53s
       //logic lights
       triggerI2C(10, 19);
       //HPEvent Disco for 53s
       FlthySerial.print("A0040|53\r");
       //Magic Panel event - Flash Q
       triggerI2C(20, 28);
     } else if (Xbox.getButtonPress(L1, 0)) {
        //Annoyed
        Play_Sound(8); // Annoyed sounds
        //logic lights, random
        triggerI2C(10, 0);
    } else if (Xbox.getButtonPress(L2, 0)) {
        //Chortle
        Play_Sound(2); // Chortle
        //logic lights, random
        triggerI2C(10, 0);
    } else if (Xbox.getButtonPress(R1, 0)) {
         //Theme
        Play_Sound(9); // Star Wars Theme 5m 29s
        //logic lights, random
        triggerI2C(10, 0);
        //Magic Panel event - Trace up 1
        triggerI2C(20, 8);
    } else if (Xbox.getButtonPress(R2, 0)) {
        //More Alarms
        Play_Sound(random(56, 71)); // More alarms
        //logic lights, random
        triggerI2C(10, 0);
        //Magic Panel event - FlashAll 5s
        triggerI2C(20, 26);
    } else {
        Play_Sound(random(13,17)); // Alarms
        //logic lights, random
        triggerI2C(10, 0);
        //Magic Panel event - FlashAll 5s
        triggerI2C(20, 26);
    }
  }

  // A Button and A combo Buttons
  if (Xbox.getButtonClick(A, 0)) {
   if (Xbox.getButtonPress(L1, 0) && Xbox.getButtonPress(R1, 0)) {
       //Gangnam
       Play_Sound(169); // Gangam Styles 24s
       //logic lights
       triggerI2C(10, 18);
       //HPEvent Disco for 24s
       FlthySerial.print("A0040|24\r");
       //Magic Panel event - Flash Q
       triggerI2C(20, 28);
   } else if (Xbox.getButtonPress(L1, 0)) {
       //shortcircuit
       Play_Sound(6); // Short Circuit
       //logic lights
       triggerI2C(10, 6);
       FlthySerial.print("A0070|5\r");
       //Magic Panel event - Fade Out
       triggerI2C(20, 25);
    } else if (Xbox.getButtonPress(L2, 0)) {
       //scream
       Play_Sound(1); // Scream
       //logic lights, alarm
       triggerI2C(10, 1);
       //HPEvent pulse Red for 4 seconds
       FlthySerial.print("A0031|4\r");
       //Magic Panel event - Alert 4s
       triggerI2C(20, 6);
    } else if (Xbox.getButtonPress(R1, 0)) {
       //Imperial March
       Play_Sound(11); // Imperial March 3m 5s
       //logic lights, alarm2Display
       triggerI2C(10, 11);
       //HPEvent - flash - I2C
       FlthySerial.print("A0030|175\r");
       //magic Panel event - Flash V
       triggerI2C(20, 27);
    } else if (Xbox.getButtonPress(R2, 0)) {
       //More Misc
       Play_Sound(random(72,99));  // Misc Sounds
       //logic lights, random
       triggerI2C(10, 0);
    } else {
       //Misc noises
       Play_Sound(random(17,25)); // More Misc Sounds
       //logic lights, random
       triggerI2C(10, 0);
    }
  }

  // B Button and B combo Buttons
  if (Xbox.getButtonClick(B, 0)) {
     if (Xbox.getButtonPress(L1, 0) && Xbox.getButtonPress(R1, 0)) {
        //Muppets
        Play_Sound(173); // Muppets Tune 30s
        //logic lights
        triggerI2C(10, 17);
        //HPEvent Disco for 30s
        FlthySerial.print("A0040|30\r");
        //Magic Panel event - Trace Up 1
        triggerI2C(20, 8);
     } else if (Xbox.getButtonPress(L1, 0)) {
        //patrol
        Play_Sound(7); // Quiet Beeps
        //logic lights, random
        triggerI2C(10, 0);
    } else if (Xbox.getButtonPress(L2, 0)) {
        //DOODOO
        Play_Sound(3); // DOODOO
       //logic lights, random
        triggerI2C(10, 0);
       //Magic Panel event - One loop sequence
        triggerI2C(20, 30);
    } else if (Xbox.getButtonPress(R1, 0)) {
       //Cantina
       Play_Sound(10); // Cantina 2m 50s
       //logic lights bargraph
       triggerI2C(10, 10);
       // HPEvent 1 - Cantina Music - Disco - I2C
       FlthySerial.print("A0040|165\r");
       //magic Panel event - Trace Down 1
       triggerI2C(20, 10);
    } else if (Xbox.getButtonPress(R2, 0)) {
       //Proc/Razz
       Play_Sound(random(100, 138)); // Proc/Razz
       //logic lights random
       triggerI2C(10, 10);
    } else {
       //Sent/Hum
       Play_Sound(random(32,52)); //Sent/Hum
       //logic lights, random
       triggerI2C(10, 0);
       //Magic Panel event - Expand 2
       triggerI2C(20, 17);
    }
  }

  // X Button and X combo Buttons
  if (Xbox.getButtonClick(X, 0)) {
   if (Xbox.getButtonPress(L1, 0) && Xbox.getButtonPress(R1, 0)) {
       //Leia Short
       Play_Sound(170); // Leia Short 6s
       //logic lights
       triggerI2C(10, 16);
       //HPEvent hologram for 6s
       FlthySerial.print("S1|6\r"); 
       //FlthySerial.print("R1014\r");       
       //FlthySerial.print("A001|6\r"); 
       //magic Panel event - Eye Scan
       triggerI2C(20, 23);
    } else if (Xbox.getButtonPress(L2, 0) && Xbox.getButtonPress(R1, 0)) {
       //Luke message
       Play_Sound(171); // Luke Message 26s
       //logic lights
       triggerI2C(10, 15);
       //HPEvent hologram for 26s
       FlthySerial.print("S1|26\r");
       //FlthySerial.print("F001|26\r");
       //magic Panel event - Cylon Row
       triggerI2C(20, 22);
    } else if (Xbox.getButtonPress(L1, 0)) {
       // leia message L1+X
       Play_Sound(5); // Leia Long 35s
       //logic lights, leia message
       triggerI2C(10, 5);
       // Front HPEvent 1 - HoloMessage leia message 35 seconds
       FlthySerial.print("S1|35\r");
       //FlthySerial.print("F001|35\r");
       //magic Panel event - Cylon Row
       triggerI2C(20, 22);
    } else if (Xbox.getButtonPress(L2, 0)) {
       //WolfWhistle
       Play_Sound(4); // Wolf whistle
       //logic lights
       triggerI2C(10, 4);
       FlthySerial.print("A00312|5\r");
       //magic Panel event - Heart
       triggerI2C(20, 40);
       // CBI panel event - Heart
       triggerI2C(30, 4);    
    } else if (Xbox.getButtonPress(R1, 0)) {
       //Duel of the Fates 
       Play_Sound(12); // Duel of Fates 4m 17s
       //logic lights, random
       triggerI2C(10, 0);
       //magic Panel event - Flash Q
       triggerI2C(20, 28);
    } else if (Xbox.getButtonPress(R2, 0)) {
       //Scream/Whistle 
       Play_Sound(random(139, 168));    // Scream/Whistle
       //logic lights, random
       triggerI2C(10, 0);
       //magic Panel event - Compress 2
       triggerI2C(20, 19);
    } else {
       //ohh (Sad Sound)
       Play_Sound(random(25, 31)); // ohh (Sad Sound)  
       //logic lights, random
       triggerI2C(10, 0);
    }
  }

  // turn hp light on & off with Right Analog Stick Press (R3) for left stick drive mode
  // turn hp light on & off with Left Analog Stick Press (L3) for right stick drive mode
  if (Xbox.getButtonClick(hpLightToggleButton, 0))  {
    // if hp light is on, turn it off
    if (isHPOn) {
      isHPOn = false;
      // turn hp light off
      // Disable LED and HP sequences
      FlthySerial.print("A098\r");
      FlthySerial.print("A198\r");
    } else {
      isHPOn = true;
      // turn hp light on
      // Enable LED and HP sequences
      FlthySerial.print("A099\r");
      FlthySerial.print("A199\r");
    }
  }


  // Change drivespeed if drive is enabled
  // Press Left Analog Stick (L3) for left stick drive mode
  // Press Right Analog Stick (R3) for right stick drive mode
  // Set LEDs for speed - 1 LED, Low. 2 LED - Med. 3 LED High
  if (Xbox.getButtonClick(speedSelectButton, 0) && isDriveEnabled) {
    //if in lowest speed
    if (drivespeed == DRIVESPEED1) {
      //change to medium speed and play sound 3-tone
      drivespeed = DRIVESPEED2;
      Xbox.setLedOn(LED2, 0);
      Play_Sound(53); // 3 Beeps
      triggerI2C(10, 22);
      //magic Panel event - AllOn 5s
      triggerI2C(20, 3);
    } else if (drivespeed == DRIVESPEED2 && (DRIVESPEED3 != 0)) {
      //change to high speed and play sound scream
      drivespeed = DRIVESPEED3;
      Xbox.setLedOn(LED3, 0);
      Play_Sound(1);  // Scream
      triggerI2C(10, 23);
      //magic Panel event - AllOn 10s
      triggerI2C(20, 4);      
    } else {
      //we must be in high speed
      //change to low speed and play sound 2-tone
      drivespeed = DRIVESPEED1;
      Xbox.setLedOn(LED1, 0);
      Play_Sound(52); // 2 Beeps
      triggerI2C(10, 21);
      //magic Panel event - AllOn 2s
      triggerI2C(20, 2);
    }
  }


  // FOOT DRIVES
  // Xbox 360 analog stick values are signed 16 bit integer value
  // Sabertooth runs at 8 bit signed. -127 to 127 for speed (full speed reverse and  full speed forward)
  // Map the 360 stick values to our min/max current drive speed
  throttleStickValue = (map(Xbox.getAnalogHat(throttleAxis, 0), -32768, 32767, -drivespeed, drivespeed));
  if (throttleStickValue > -DRIVEDEADZONERANGE && throttleStickValue < DRIVEDEADZONERANGE) {  
    // stick is in dead zone - don't drive
    driveThrottle = 0;
  } else {
    if (driveThrottle < throttleStickValue) {
      if (throttleStickValue - driveThrottle < (RAMPING + 1) ) {
        driveThrottle += RAMPING;
      } else {
         driveThrottle = throttleStickValue;
      }
    } else if (driveThrottle > throttleStickValue) {
      if (driveThrottle - throttleStickValue < (RAMPING + 1) ) {
        driveThrottle -= RAMPING;
      } else {
        driveThrottle = throttleStickValue;
      }
    }
  }

  turnThrottle = map(Xbox.getAnalogHat(turnAxis, 0), -32768, 32767, -TURNSPEED, TURNSPEED);

  // DRIVE!
  // right stick (drive)
  if (isDriveEnabled) {
    // Only do deadzone check for turning here. Our Drive throttle speed has some math applied
    // for RAMPING and stuff, so just keep it separate here
    if (turnThrottle > -DRIVEDEADZONERANGE && turnThrottle < DRIVEDEADZONERANGE) {
      // stick is in dead zone - don't turn
      turnThrottle = 0;
    }
    Sabertooth2x.turn(-turnThrottle);
    Sabertooth2x.drive(driveThrottle);
  }

  // DOME DRIVE!
          domeThrottle = (map(Xbox.getAnalogHat(domeAxis, 0), -32768, 32767, DOMESPEED, -DOMESPEED));
          
          if (domeThrottle > -DOMEDEADZONERANGE && domeThrottle < DOMEDEADZONERANGE) {
            //stick in dead zone - don't spin dome
            domeThrottle = 0;
          }
        
          Syren10.motor(1, domeThrottle);


} // END loop()
