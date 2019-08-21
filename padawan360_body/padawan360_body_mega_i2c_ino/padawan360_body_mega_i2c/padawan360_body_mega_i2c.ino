 
// =======================================================================================
// /////////////////////////Padawan360 Body Code - Mega I2C v1.1 ////////////////////////////////////
// =======================================================================================
/*


v1.1
by Steven Sloan (with code taken from Robert Corvus and Andy Smith (locqust))
Code for YX5300 sound card added (Code for Sparkfun MP3 Trigger retained but not tested)
Code for L298N Dome motor driver added - but not tested (Code for Syren motor controller retained but not tested)
Serial connection to FlthyHP breakout board added.  Removed I2C comms to FlthyHP.
Added additional key combinations for extra sounds and actions.


V1.0
by Dan Kraus
dskraus@gmail.com
Astromech: danomite4047

Heavily influenced by DanF's Padwan code which was built for Arduino+Wireless PS2
controller leveraging Bill Porter's PS2X Library. I was running into frequent disconnect
issues with 4 different controllers working in various capacities or not at all. I decided
that PS2 Controllers were going to be more difficult to come by every day, so I explored
some existing libraries out there to leverage and came across the USB Host Shield and it's
support for PS3 and Xbox 360 controllers. Bluetooth dongles were inconsistent as well
so I wanted to be able to have something with parts that other builder's could easily track
down and buy parts even at your local big box store.

Hardware:
***Arduino Mega 2560***
USB Host Shield from circuits@home
Microsoft Xbox 360 Controller
Xbox 360 USB Wireless Reciver
Sabertooth Motor Controller
Syren Motor Controller
Sparkfun MP3 Trigger

This sketch supports I2C and calls events on many sound effect actions to control lights and sounds.
It is NOT set up for Dan's method of using the serial packet to transfer data up to the dome
to trigger some light effects.If you want that, you'll need to reference DanF's original.

It uses Hardware Serial pins on the Mega to control Sabertooth and Syren

Set Sabertooth 2x25/2x12 Dip Switches 1 and 2 Down, All Others Up
For SyRen Simple Serial Set Switches 1 and 2 Down, All Others Up
For SyRen Simple Serial Set Switchs 2 & 4 Down, All Others Up
Placed a 10K ohm resistor between S1 & GND on the SyRen 10 itself

Pins in use
1 = MP3 Trigger Rx
3 = EXTINGUISHER relay pin
5 = Rx to YX5300 MP3 player
6 = Tx to YX5300 MP3 player
8 = L298N Dome Speed (ENA)
10 = L298N Dome Dir1 pin (IN1)
11 = L298N Dome Dir2 pin (IN2)

14 = Serial3 (Tx3) = FlthyHP 
15 = Serial3 (Rx3) = FlthpHP
16 = Serial2 (Tx2) = Syren10 S1
18 = Serial1 (Tx1) = Sabertooth 2x25 S1
19 = Serial1 (Rx1) = Sabertooth 2x25 S2

20 = SDA (I2C Comms)
21 = SCL (I2C Comms)

*/

//************************** Set speed and turn speeds here************************************//

//set these 3 to whatever speeds work for you. 0-stop, 127-full speed.
const byte DRIVESPEED1 = 50;
//Recommend beginner: 50 to 75, experienced: 100 to 127, I like 100.
const byte DRIVESPEED2 = 100;
//Set to 0 if you only want 2 speeds.
const byte DRIVESPEED3 = 127; 

byte drivespeed = DRIVESPEED1;

// the higher this number the faster the droid will spin in place, lower - easier to control.
// Recommend beginner: 40 to 50, experienced: 50 $ up, I like 70
const byte TURNSPEED = 40;

// If using a speed controller for the dome, sets the top speed. You'll want to vary it potenitally
// depending on your motor. My Pittman is really fast so I dial this down a ways from top speed.
// Use a number up to 127 for serial
const byte DOMESPEED = 90;

// Ramping- the lower this number the longer R2 will take to speedup or slow down,
// change this by incriments of 1
const byte RAMPING = 5;

// Compensation is for deadband/deadzone checking. There's a little play in the neutral zone
// which gets a reading of a value of something other than 0 when you're not moving the stick.
// It may vary a bit across controllers and how broken in they are, sometimex 360 controllers
// develop a little bit of play in the stick at the center position. You can do this with the
// direct method calls against the Syren/Sabertooth library itself but it's not supported in all
// serial modes so just manage and check it in software here
// use the lowest number with no drift
// DOMEDEADZONERANGE for the left stick, DRIVEDEADZONERANGE for the right stick
const byte DOMEDEADZONERANGE = 20;
const byte DRIVEDEADZONERANGE = 20;


// Set the baud rate for the Sabertooth motor controller (feet)
// 9600 is the default baud rate for Sabertooth packet serial.
// for packetized options are: 2400, 9600, 19200 and 38400. I think you need to pick one that works
// and I think it varies across different firmware versions.
const int SABERTOOTHBAUDRATE = 9600;

// Set the baud rate for the Syren motor controller (dome)
// for packetized options are: 2400, 9600, 19200 and 38400. I think you need to pick one that works
// and I think it varies across different firmware versions.
const int DOMEBAUDRATE = 2400;

String hpEvent = "";
char char_array[11];

// Set the baud rate for the YX5300 MP3 player
// 9600 is the default baud rate for YX5300 packet serial.
const int YX5300RATE = 9600;

// I have a pin set to pull a relay high/low to trigger my upside down compressed air like R2's extinguisher
#define EXTINGUISHERPIN 3

#include <Sabertooth.h>
#include <MP3Trigger.h>
#include <Wire.h>
#include <XBOXRECV.h>
#include <SoftwareSerial.h>


/////////////////////////////////////////////////////////////////
//Serial connection to Flthys HP's
#define FlthyTXPin 14 // OR OTHER FREE DIGITAL PIN
#define FlthyRXPin 15 // OR OTHER FREE DIGITAL PIN  // NOT ACTUALLY USED, BUT NEEDED FOR THE SOFTWARESERIAL FUNCTION
const int FlthyBAUD = 9600; // OR SET YOUR OWN BAUD AS NEEDED
SoftwareSerial FlthySerial(FlthyRXPin, FlthyTXPin); 

Sabertooth Sabertooth2x(128, Serial1);
Sabertooth Syren10(128, Serial2);

// Satisfy IDE, which only needs to see the include statement in the ino.
#ifdef dobogusinclude
#include <spi4teensy3.h>
#endif

// Set some defaults for start up
// 0 = full volume, 255 off
byte vol = 20;
// 0 = drive motors off ( right stick disabled ) at start
boolean isDriveEnabled = false;

// Automated function variables
// Used as a boolean to turn on/off automated functions like periodic random sounds and periodic dome turns
boolean isInAutomationMode = false;
unsigned long automateMillis = 0;
byte automateDelay = random(5, 20); // set this to min and max seconds between sounds
//How much the dome may turn during automation.
int turnDirection = 20;
// Action number used to randomly choose a sound effect or a dome turn
byte automateAction = 0;

char driveThrottle = 0; //int driveThrottle = 0;
char rightStickValue = 0; //int rightStick = 0;
int domeThrottle = 0; //int domeThrottle = 0;
char turnThrottle = 0; //int turnThrottle = 0;

boolean firstLoadOnConnect = false;
boolean manuallyDisabledController = false;

// this is legacy right now. The rest of the sketch isn't set to send any of this
// data to another arduino like the original Padawan sketch does
// right now just using it to track whether or not the HP light is on so we can
// fire the correct I2C event to turn on/off the HP light.
//struct SEND_DATA_STRUCTURE{
//  //put your variable definitions here for the data you want to send
//  //THIS MUST BE EXACTLY THE SAME ON THE OTHER ARDUINO
//  int hpl; // hp light
//  int dsp; // 0 = random, 1 = alarm, 5 = leia, 11 = alarm2, 100 = no change
//};
//SEND_DATA_STRUCTURE domeData;//give a name to the group of data

boolean isHPOn = false;



MP3Trigger mp3Trigger;
USB Usb;
XBOXRECV Xbox(&Usb);

/****************** L298N Configuration  **********************/
#define L298N        // Uncomment if using an L298N motor controller for the dome
int Dome_Speed_Pin = 8; 
int Dome_dir1_Pin = 10; 
int Dome_dir2_Pin = 11;  
int Dome_Speed_PWM = 0;
boolean Dome_Direction = false;
const byte L298N_DOMEDEADZONERANGE = 60; //Set this to the lowest value 

/****************** YX5300 Configuration  **********************/
#define MP3_YX5300   //Uncomment if using a YX5300 for sound
#define YX5300_RX 5  //should connect to TX of the Serial MP3 Player module
#define YX5300_TX 6  //connect to RX of the module

SoftwareSerial YX5300(YX5300_RX, YX5300_TX);

static int8_t Send_buf[8] = {0}; // Buffer for Send commands.  // BETTER LOCALLY
//static uint8_t ansbuf[10] = {0}; // Buffer for the answers.    // BETTER LOCALLY

//static int8_t pre_vol, volume = 0x0f; // Volume. 0-30 DEC values. 0x0f = 15. 
int volume = 12;
String mp3Answer;           // Answer from the MP3.   

//boolean playing = false;    // Sending 'p' the module switch to Play to Pause or viceversa.

/************ YX5300 Command byte Definition **************************/
#define CMD_NEXT_SONG     0X01  // Play next song.
#define CMD_PREV_SONG     0X02  // Play previous song.
#define CMD_PLAY_W_INDEX  0X03
#define CMD_VOLUME_UP     0X04
#define CMD_VOLUME_DOWN   0X05
#define CMD_SET_VOLUME    0X06

#define CMD_SNG_CYCL_PLAY 0X08  // Single Cycle Play.
#define CMD_SEL_DEV       0X09
#define CMD_SLEEP_MODE    0X0A
#define CMD_WAKE_UP       0X0B
#define CMD_RESET         0X0C
#define CMD_PLAY          0X0D
#define CMD_PAUSE         0X0E
//#define CMD_PLAY_FOLDER_FILE 0X0F

//#define CMD_STOP_PLAY     0X16
//#define CMD_FOLDER_CYCLE  0X17
//#define CMD_SHUFFLE_PLAY  0x18 //
#define CMD_SET_SNGL_CYCL 0X19 // Set single cycle.

#define CMD_SET_DAC 0X1A
#define DAC_ON  0X00
#define DAC_OFF 0X01
  
#define CMD_PLAY_W_VOL    0X22
#define CMD_PLAYING_N     0x4C

/************ Opitons **************************/  
#define DEV_TF            0X02  
#define SINGLE_CYCLE_ON   0X00
#define SINGLE_CYCLE_OFF  0X01


/*********************************************************************/

//=====================================
//   SETUP                           //
//=====================================

void setup() {
  Serial.begin(9600);
  Serial1.begin(SABERTOOTHBAUDRATE);
  Serial2.begin(DOMEBAUDRATE);

#ifdef L298N
  pinMode(Dome_Speed_Pin,OUTPUT); 
  pinMode(Dome_dir1_Pin,OUTPUT); 
  pinMode(Dome_dir2_Pin,OUTPUT);
#endif

//Flthy HP
  FlthySerial.begin(FlthyBAUD);
  
#if defined(SYRENSIMPLE)
  Syren10.motor(0);
#else
  Syren10.autobaud();
#endif

  // Send the autobaud command to the Sabertooth controller(s).
  /* NOTE: *Not all* Sabertooth controllers need this command.
  It doesn't hurt anything, but V2 controllers use an
  EEPROM setting (changeable with the function setBaudRate) to set
  the baud rate instead of detecting with autobaud.
  If you have a 2x12, 2x25 V2, 2x60 or SyRen 50, you can remove
  the autobaud line and save yourself two seconds of startup delay.
  */
  Sabertooth2x.autobaud();
  // The Sabertooth won't act on mixed mode packet serial commands until
  // it has received power levels for BOTH throttle and turning, since it
  // mixes the two together to get diff-drive power levels for both motors.
  Sabertooth2x.drive(0);
  Sabertooth2x.turn(0);


  Sabertooth2x.setTimeout(950);
  Syren10.setTimeout(950);

  pinMode(EXTINGUISHERPIN, OUTPUT);
  digitalWrite(EXTINGUISHERPIN, HIGH);

  #ifdef MP3_YX5300
    YX5300.begin(YX5300RATE);
//    sendCommand(CMD_RESET, 0x00);
    delay(500);
    sendCommand(CMD_SEL_DEV, DEV_TF); // Set input device as microSD card
    Serial.println("Set microSD as input");
    delay(200);
    sendCommand(CMD_SET_VOLUME,volume);  //Set volume at 15 (HEX 0F). Volume can be set from 0-30 (0x001E)
    Serial.println("Set volume to 15");
//    sendCommand(SINGLE_CYCLE_OFF, 0x0001);
//    Serial.println("Switch off cycle/loop");
  #else
    mp3Trigger.setup();
    mp3Trigger.setVolume(vol);  
  #endif

  // Start I2C Bus. The body is the master.
  Wire.begin();

   #ifdef MP3_YX5300
     sendCommand(CMD_PLAY_W_INDEX, 23);
   #else
     mp3Trigger.play(23);  
   #endif

  // Wait for serial port to connect - used on Leonardo, Teensy and other boards with built-in USB CDC serial connection
  while (!Serial);
  if (Usb.Init() == -1) {
    //Serial.print(F("\r\nOSC did not start"));
    while (1); //halt
  }
  //Serial.print(F("\r\nXbox Wireless Receiver Library Started"));
}

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
    L298N_Dome_Stop;
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
         #ifdef MP3_YX5300
          sendCommand(CMD_PLAY_W_INDEX, 21);
        #else
          mp3Trigger.play(21);  
        #endif
    Xbox.setLedMode(ROTATING, 0);
    manuallyDisabledController=false;
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
        #ifdef MP3_YX5300
          sendCommand(CMD_PLAY_W_INDEX, 53);
          Serial.println("enable/disable sound");
        #else
          mp3Trigger.play(53); 
        #endif
    } else {
      isDriveEnabled = true;
        #ifdef MP3_YX5300
          sendCommand(CMD_PLAY_W_INDEX, 52);
        #else
          mp3Trigger.play(52);  
        #endif
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
        #ifdef MP3_YX5300
          sendCommand(CMD_PLAY_W_INDEX, 53);
        #else
          mp3Trigger.play(53);  
        #endif
    } else {
      isInAutomationMode = true;
        #ifdef MP3_YX5300
          sendCommand(CMD_PLAY_W_INDEX, 52);
        #else
          mp3Trigger.play(52);  
        #endif
    }
  }

  // Plays random sounds or dome movements for automations when in automation mode
  if (isInAutomationMode) {
    triggerAutomation();
}

  // Volume Control of MP3 Trigger
  // Hold R1 and Press Up/down on D-pad to increase/decrease volume
  if (Xbox.getButtonClick(DOWN, 0)) {
    // volume up
    if (Xbox.getButtonPress(R1, 0)) {
      
       #ifdef MP3_YX5300
         if (volume > 0) {
           volume--;
           Serial.println("volume ");
           Serial.println(volume);
           sendCommand(CMD_VOLUME_DOWN, 00);
         }
      #else    
        if (vol > 0) {
          vol--;
          mp3Trigger.setVolume(vol);  
        }
      #endif
      
    }
  }
  if (Xbox.getButtonClick(UP, 0)) {
    //volume down
    if (Xbox.getButtonPress(R1, 0)) {

       #ifdef MP3_YX5300
         if (volume < 30) {
           volume++;
           Serial.println("volume ");
           Serial.println(volume);
           sendCommand(CMD_VOLUME_UP, 00);
         }
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


  // GENERAL SOUND PLAYBACK AND DISPLAY CHANGING

  // Y Button and Y combo buttons
  if (Xbox.getButtonClick(Y, 0)) {
     if (Xbox.getButtonPress(L1, 0) && Xbox.getButtonPress(R1, 0)) {
       //Addams
         #ifdef MP3_YX5300
          sendCommand(CMD_PLAY_W_INDEX, 168);
        #else
          mp3Trigger.play(168);  
        #endif
       //logic lights
       triggerI2C(10, 19);
       //HPEvent Disco for 53s
       FlthySerial.print("A0040|53\r");
       //Magic Panel event - Flash Q
       triggerI2C(20, 28);
     } else if (Xbox.getButtonPress(L1, 0)) {
         //Annoyed
         #ifdef MP3_YX5300
          sendCommand(CMD_PLAY_W_INDEX, 8);
        #else
          mp3Trigger.play(8);  
        #endif
      //logic lights, random
      triggerI2C(10, 0);
    } else if (Xbox.getButtonPress(L2, 0)) {
        //Chortle
        #ifdef MP3_YX5300
          sendCommand(CMD_PLAY_W_INDEX, 2);
        #else
          mp3Trigger.play(2);  
        #endif
      //logic lights, random
      triggerI2C(10, 0);
    } else if (Xbox.getButtonPress(R1, 0)) {
         //Theme
         #ifdef MP3_YX5300
          sendCommand(CMD_PLAY_W_INDEX, 9);
        #else
          mp3Trigger.play(9);  
        #endif
      //logic lights, random
      triggerI2C(10, 0);
      //Magic Panel event - Trace up 1
      triggerI2C(20, 8);
    } else if (Xbox.getButtonPress(R2, 0)) {
        //More Alarms
        #ifdef MP3_YX5300
          sendCommand(CMD_PLAY_W_INDEX, random(56, 71));
        #else
          mp3Trigger.play(random(56, 71));  
        #endif
      //logic lights, random
      triggerI2C(10, 0);
      //Magic Panel event - FlashAll 5s
      triggerI2C(20, 26);
    } else {
        #ifdef MP3_YX5300
          sendCommand(CMD_PLAY_W_INDEX, random(13,17));
        #else
          mp3Trigger.play(random(13,17));  
        #endif      
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
         #ifdef MP3_YX5300
          sendCommand(CMD_PLAY_W_INDEX, 169);
        #else
          mp3Trigger.play(169);
        #endif
      //logic lights
      triggerI2C(10, 18);
      //HPEvent Disco for 24s
      FlthySerial.print("A0040|24\r");
      //Magic Panel event - Flash Q
      triggerI2C(20, 28);
   } else if (Xbox.getButtonPress(L1, 0)) {
         //shortcircuit
         #ifdef MP3_YX5300
          sendCommand(CMD_PLAY_W_INDEX, 6);
        #else
          mp3Trigger.play(6);  
        #endif
      //logic lights
      triggerI2C(10, 6);
      FlthySerial.print("A0070|5\r");
      //Magic Panel event - Fade Out
      triggerI2C(20, 25);
    } else if (Xbox.getButtonPress(L2, 0)) {
         //scream
         #ifdef MP3_YX5300
          sendCommand(CMD_PLAY_W_INDEX, 1);
        #else
          mp3Trigger.play(1);  
        #endif
      //logic lights, alarm
      triggerI2C(10, 1);
      //HPEvent pulse Red for 4 seconds
      FlthySerial.print("A0031|4\r");
      //Magic Panel event - Alert 4s
      triggerI2C(20, 6);
    } else if (Xbox.getButtonPress(R1, 0)) {
        //Imp March
        #ifdef MP3_YX5300
          sendCommand(CMD_PLAY_W_INDEX, 11);
        #else
          mp3Trigger.play(11);  
        #endif
      //logic lights, alarm2Display
      triggerI2C(10, 11);
      //HPEvent - flash - I2C
      FlthySerial.print("A0030|175\r");
      //magic Panel event - Flash V
      triggerI2C(20, 27);
    } else if (Xbox.getButtonPress(R2, 0)) {
      //More Misc
        #ifdef MP3_YX5300
          sendCommand(CMD_PLAY_W_INDEX, random(72, 99));
        #else
          mp3Trigger.play(random(72, 99));
        #endif
      //logic lights, random
      triggerI2C(10, 0);
    } else {
         //Misc noises
         #ifdef MP3_YX5300
          sendCommand(CMD_PLAY_W_INDEX, random(17,25));
        #else
          mp3Trigger.play(random(17,25));  
        #endif        
      //logic lights, random
      triggerI2C(10, 0);
    }
  }

  // B Button and B combo Buttons
  if (Xbox.getButtonClick(B, 0)) {
     if (Xbox.getButtonPress(L1, 0) && Xbox.getButtonPress(R1, 0)) {
        //Muppets
         #ifdef MP3_YX5300
          sendCommand(CMD_PLAY_W_INDEX, 172);
        #else
          mp3Trigger.play(172);  
        #endif
        //logic lights
        triggerI2C(10, 17);
        //HPEvent Disco for 30s
        FlthySerial.print("A0040|30\r");
        //Magic Panel event - Trace Up 1
        triggerI2C(20, 8);
     } else if (Xbox.getButtonPress(L1, 0)) {
         //patrol
         #ifdef MP3_YX5300
          sendCommand(CMD_PLAY_W_INDEX, 7);
        #else
          mp3Trigger.play(7);  
        #endif
      //logic lights, random
      triggerI2C(10, 0);
    } else if (Xbox.getButtonPress(L2, 0)) {
        //DOODOO
        #ifdef MP3_YX5300
          sendCommand(CMD_PLAY_W_INDEX, 3);
        #else
          mp3Trigger.play(3);  
        #endif
      //logic lights, random
      triggerI2C(10, 0);
      //Magic Panel event - One loop sequence
      triggerI2C(20, 30);
    } else if (Xbox.getButtonPress(R1, 0)) {
        //Cantina
        #ifdef MP3_YX5300
          sendCommand(CMD_PLAY_W_INDEX, 10);
        #else
          mp3Trigger.play(10);  
        #endif
      //logic lights bargrap
      triggerI2C(10, 10);
      // HPEvent 1 - Cantina Music - Disco - I2C
      FlthySerial.print("A0040|165\r");
      //magic Panel event - Trace Down 1
      triggerI2C(20, 10);
    } else if (Xbox.getButtonPress(R2, 0)) {
        //Proc/Jazz
        #ifdef MP3_YX5300
          sendCommand(CMD_PLAY_W_INDEX, random(100, 139));
        #else
          mp3Trigger.play(random(100, 139));  
        #endif
      //logic lights random
      triggerI2C(10, 10);
    } else {
        //Sent/Hum
        #ifdef MP3_YX5300
          sendCommand(CMD_PLAY_W_INDEX, random(32,52));
        #else
          mp3Trigger.play(random(32, 52));  
        #endif        
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
        #ifdef MP3_YX5300
          sendCommand(CMD_PLAY_W_INDEX, 170);
        #else
          mp3Trigger.play(170);  
        #endif
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
        #ifdef MP3_YX5300
          sendCommand(CMD_PLAY_W_INDEX, 171);
        #else
          mp3Trigger.play(171);  
        #endif
      //logic lights
      triggerI2C(10, 15);
      //HPEvent hologram for 26s
      FlthySerial.print("S1|26\r");
      //FlthySerial.print("F001|26\r");
      //magic Panel event - Cylon Row
      triggerI2C(20, 22);
    } else if (Xbox.getButtonPress(L1, 0)) {
        // leia message L1+X
        #ifdef MP3_YX5300
          sendCommand(CMD_PLAY_W_INDEX, 5);
        #else
          mp3Trigger.play(5);  
        #endif
      //logic lights, leia message
      triggerI2C(10, 5);
      // Front HPEvent 1 - HoloMessage leia message 35 seconds
      FlthySerial.print("S1|35\r");
      //FlthySerial.print("F001|35\r");
      //magic Panel event - Cylon Row
      triggerI2C(20, 22);
    } else if (Xbox.getButtonPress(L2, 0)) {
         //WolfWhistle
         #ifdef MP3_YX5300
          sendCommand(CMD_PLAY_W_INDEX, 4);
        #else
          mp3Trigger.play(4);  
        #endif
      //logic lights
      triggerI2C(10, 4);
      FlthySerial.print("A00312|5\r");
      //magic Panel event - Heart
      triggerI2C(20, 40);
      // CBI panel event - Heart
      triggerI2C(30, 4);    
    } else if (Xbox.getButtonPress(R1, 0)) {
        //Duel of the Fates 
        #ifdef MP3_YX5300
          sendCommand(CMD_PLAY_W_INDEX, 12);
        #else
          mp3Trigger.play(12);  
        #endif
      //logic lights, random
      triggerI2C(10, 0);
      //magic Panel event - Flash Q
      triggerI2C(20, 28);
    } else if (Xbox.getButtonPress(R2, 0)) {
        //Proc/Jazz 
        #ifdef MP3_YX5300
          sendCommand(CMD_PLAY_W_INDEX, random(139, 168));
        #else
          mp3Trigger.play(random(139, 168));  
        #endif
      //logic lights, random
      triggerI2C(10, 0);
      //magic Panel event - Compress 2
      triggerI2C(20, 19);
    } else {
        //ohh/sent
        #ifdef MP3_YX5300
          sendCommand(CMD_PLAY_W_INDEX, random(25, 32));
        #else
          mp3Trigger.play(random(25, 32));  
        #endif      
      //logic lights, random
      triggerI2C(10, 0);
    }
  }

  // turn hp light on & off with Left Analog Stick Press (L3)
  if (Xbox.getButtonClick(L3, 0))  {
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
  // Press Right Analog Stick (R3)
  // Set LEDs for speed - 1 LED, Low. 2 LED - Med. 3 LED High
  if (Xbox.getButtonClick(R3, 0) && isDriveEnabled) {
    //if in lowest speed
    if (drivespeed == DRIVESPEED1) {
      //change to medium speed and play sound 3-tone
      drivespeed = DRIVESPEED2;
      Xbox.setLedOn(LED2, 0);
         #ifdef MP3_YX5300
          sendCommand(CMD_PLAY_W_INDEX, 53);
        #else
          mp3Trigger.play(53);  
        #endif
      triggerI2C(10, 22);
      //magic Panel event - AllOn 5s
      triggerI2C(20, 3);
    } else if (drivespeed == DRIVESPEED2 && (DRIVESPEED3 != 0)) {
      //change to high speed and play sound scream
      drivespeed = DRIVESPEED3;
      Xbox.setLedOn(LED3, 0);
         #ifdef MP3_YX5300
          sendCommand(CMD_PLAY_W_INDEX, 1);
        #else
          mp3Trigger.play(1);  
        #endif
      triggerI2C(10, 23);
      //magic Panel event - AllOn 10s
      triggerI2C(20, 4);      
    } else {
      //we must be in high speed
      //change to low speed and play sound 2-tone
      drivespeed = DRIVESPEED1;
      Xbox.setLedOn(LED1, 0);
         #ifdef MP3_YX5300
          sendCommand(CMD_PLAY_W_INDEX, 52);
        #else
          mp3Trigger.play(52);  
        #endif
      triggerI2C(10, 21);
      //magic Panel event - AllOn 2s
      triggerI2C(20, 2);
    }
  }


  // FOOT DRIVES
  // Xbox 360 analog stick values are signed 16 bit integer value
  // Sabertooth runs at 8 bit signed. -127 to 127 for speed (full speed reverse and  full speed forward)
  // Map the 360 stick values to our min/max current drive speed
  rightStickValue = (map(Xbox.getAnalogHat(RightHatY, 0), -32768, 32767, -drivespeed, drivespeed));
  if (rightStickValue > -DRIVEDEADZONERANGE && rightStickValue < DRIVEDEADZONERANGE) {
    // stick is in dead zone - don't drive
    driveThrottle = 0;
  } else {
    if (driveThrottle < rightStickValue) {
      if (rightStickValue - driveThrottle < (RAMPING + 1) ) {
        driveThrottle += RAMPING;
      } else {
        driveThrottle = rightStickValue;
      }
    } else if (driveThrottle > rightStickValue) {
      if (driveThrottle - rightStickValue < (RAMPING + 1) ) {
        driveThrottle -= RAMPING;
      } else {
        driveThrottle = rightStickValue;
      }
    }
  }

  turnThrottle = map(Xbox.getAnalogHat(RightHatX, 0), -32768, 32767, -TURNSPEED, TURNSPEED);

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
        #ifdef L298N

          domeThrottle = (map(Xbox.getAnalogHat(LeftHatX, 0), -32768, 32767,-255, 255));

          if (domeThrottle > -L298N_DOMEDEADZONERANGE && domeThrottle < L298N_DOMEDEADZONERANGE) {
            //stick in dead zone - don't spin dome
            domeThrottle = 0;
          }

          if (domeThrottle > 0){
            Dome_Direction =false;
          }
          else if (domeThrottle < 0){
            Dome_Direction =true;
          }

          domeThrottle = abs(domeThrottle);

          L298N_Dome_Move(Dome_Speed_Pin, domeThrottle ); // set the second variable as the speed you want the dome to move at

        #else
        
          domeThrottle = (map(Xbox.getAnalogHat(LeftHatX, 0), -32768, 32767, DOMESPEED, -DOMESPEED));
          
          if (domeThrottle > -DOMEDEADZONERANGE && domeThrottle < DOMEDEADZONERANGE) {
            //stick in dead zone - don't spin dome
            domeThrottle = 0;
          }
        
          Syren10.motor(1, domeThrottle);

        #endif

} // END loop()

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
        #ifdef MP3_YX5300
          sendCommand(CMD_PLAY_W_INDEX, random(32, 52));
        #else
          mp3Trigger.play(random(32, 52));  // mp3Trigger.play(random(17, 167));
        #endif     
      }
      if (automateAction < 4) {

      //************* Move the dome for 750 msecs  **************
      #ifdef L298N

        L298N_Dome_Move(Dome_Speed_Pin,180 ); // set the second variable as the speed you want the dome to move at

      #endif
        
      #if defined(SYRENSIMPLE)
        Syren10.motor(turnDirection);
      #else
        Syren10.motor(1, turnDirection);
      #endif

        delay(750);

        //************* Stop the dome motor **************
        #ifdef L298N 

          L298N_Dome_Stop;

        #endif      

      #if defined(SYRENSIMPLE)
        Syren10.motor(0);
      #else
        Syren10.motor(1, 0);
      #endif

        //************* Change direction for next time **************
        if (Dome_Direction) {
          Dome_Direction = false;
        } else {
          Dome_Direction = true;  
        }
        
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

/********************************************************************************/
/*Function: Send command to the YX5300 MP3                                         */
/*Parameter:-int8_t command                                                     */
/*Parameter:-int16_ dat  parameter for the command                              */

void sendCommand(int8_t command, int16_t dat)
{
  delay(20);
  Send_buf[0] = 0x7e;   //
  Send_buf[1] = 0xff;   //
  Send_buf[2] = 0x06;   // Len 
  Send_buf[3] = command;//
  Send_buf[4] = 0x01;   // 0x00 NO, 0x01 feedback
  Send_buf[5] = (int8_t)(dat >> 8);  //datah
  Send_buf[6] = (int8_t)(dat);       //datal
  Send_buf[7] = 0xef;   //
  for(uint8_t i=0; i<8; i++)
  {
    YX5300.write(Send_buf[i]) ;
    Serial.print(Send_buf [i]);
    Serial.print(" ");
  }
Serial.println();
}

void L298N_Dome_Move(int Dome_Speed_Pin,int Dome_Speed_PWM ) {

if (Dome_Direction){
  digitalWrite(Dome_dir1_Pin,HIGH); 
  digitalWrite(Dome_dir2_Pin,LOW);
//  Serial.println("High-Low");
} 
else {
  digitalWrite(Dome_dir1_Pin,LOW); 
  digitalWrite(Dome_dir2_Pin,HIGH);
//  Serial.println("Low-High");
}  

//Serial.println(Dome_Speed_PWM);

analogWrite(Dome_Speed_Pin, Dome_Speed_PWM);

}

void L298N_Dome_Stop( ) {

analogWrite(Dome_Speed_Pin, 0);
  
}
