
//************************** Set speed and turn speeds here************************************//

// SPEED AND TURN SPEEDS
//set these 3 to whatever speeds work for you. 0-stop, 127-full speed.
const byte DRIVESPEED1 = 50;
// Recommend beginner: 50 to 75, experienced: 100 to 127, I like 100.
// These may vary based on your drive system and power system
const byte DRIVESPEED2 = 100;
//Set to 0 if you only want 2 speeds.
const byte DRIVESPEED3 = 127;

// Default drive speed at startup
byte drivespeed = DRIVESPEED1;

// the higher this number the faster the droid will spin in place, lower - easier to control.
// Recommend beginner: 40 to 50, experienced: 50 $ up, I like 70
// This may vary based on your drive system and power system
const byte TURNSPEED = 40;

// Set isLeftStickDrive to true for driving  with the left stick
// Set isLeftStickDrive to false for driving with the right stick (legacy and original configuration)
boolean isLeftStickDrive = true; 

// If using a speed controller for the dome, sets the top speed. You'll want to vary it potenitally
// depending on your motor. My Pittman is really fast so I dial this down a ways from top speed.
// Use a number up to 127 for serial
const byte DOMESPEED = 127;

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
const int DOMEBAUDRATE = 9600;

String hpEvent = "";
char char_array[11];

// I have a pin set to pull a relay high/low to trigger my upside down compressed air like R2's extinguisher
#define EXTINGUISHERPIN 3

#include <Sabertooth.h>
#include <MP3Trigger.h>
#include <Wire.h>
#include <XBOXRECV.h>  // needs to be the Chatpad fork https://github.com/willtoth/USB_Host_Shield_2.0

#include <Servos.h>
#include <SoftwareSerial.h>
#include <SyRenSimplified.h>
#include <Adafruit_PWMServoDriver.h>

#include <MD_YX5300.h> // https://github.com/MajicDesigns/MD_YX5300

/////////////////////////////////////////////////////////////////
//Serial connection to Flthys HP's
#define FlthyTXPin 14 // OR OTHER FREE DIGITAL PIN
#define FlthyRXPin 15 // OR OTHER FREE DIGITAL PIN  // NOT ACTUALLY USED, BUT NEEDED FOR THE SOFTWARESERIAL FUNCTION
const int FlthyBAUD = 9600; // OR SET YOUR OWN BAUD AS NEEDED
SoftwareSerial FlthySerial(FlthyRXPin, FlthyTXPin); 

/////////////////////////////////////////////////////////////////
Sabertooth Sabertooth2x(128, Serial1);
Sabertooth Syren10(128, Serial2);

// Satisfy IDE, which only needs to see the include statement in the ino.
#ifdef dobogusinclude
#include <spi4teensy3.h>
#endif

// Set some defaults for start up
// 0 = full volume, 255 off
byte vol = 20;  //Sparkfun MP3 player volume
// false = drive motors off ( drive stick disabled ) at start
boolean isDriveEnabled = false;

// Automated functionality
// Used as a boolean to turn on/off automated functions like periodic random sounds and periodic dome turns
boolean isInAutomationMode = false;
unsigned long automateMillis = 0;
byte automateDelay = random(5, 20); // set this to min and max seconds between sounds
//How much the dome may turn during automation.
int turnDirection = 20;
// Action number used to randomly choose a sound effect or a dome turn
byte automateAction = 0;

char driveThrottle = 0; 
//char rightStickValue = 0; 
int throttleStickValue = 0;
int domeThrottle = 0; //int domeThrottle = 0; //ssloan
char turnThrottle = 0; 

boolean firstLoadOnConnect = false;

AnalogHatEnum throttleAxis;
AnalogHatEnum turnAxis;
AnalogHatEnum domeAxis;
ButtonEnum speedSelectButton;
ButtonEnum hpLightToggleButton;

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

/****************** YX5300 Configuration  **********************/
#define MP3_YX5300   //Uncomment if using a YX5300 for sound
// Connections for serial interface to the YX5300 module
#ifdef MP3_YX5300
  const uint8_t YX5300_RX = 5;    // connect to TX of MP3 Player module
  const uint8_t YX5300_TX = 6;    // connect to RX of MP3 Player module
  MD_YX5300 YX5300(YX5300_RX, YX5300_TX); 
  uint16_t volume =12; //YX5300 Start volume
#endif

/***************************************************************/

/************* Xbox360 Chatpad Configuration  ******************/
#define CHATPAD    //Uncomment if using a chatpad 
bool orangeToggle = false;

