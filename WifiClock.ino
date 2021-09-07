//
// Code by Jack Machiela.....
//

#include <WiFiManager.h>          // Library "WifiManager by tablatronix" - Tested at v2.0.3-alpha (aka "WifiManager by tzapu" - lib at https://github.com/tzapu/WiFiManager)
#include <ezTime.h>               // Library "EZTime by Rop Gonggrijp" - Tested at v0.8.3 (lib & docs at https://github.com/ropg/ezTime, docs at https://awesomeopensource.com/project/ropg/ezTime
#include <TM1637Display.h>        // Library "TM1637 by Avishay Orpaz" - Tested at v1.2.0


// Define some wibbley wobbley timey wimey stuff
String TimeZoneDB = "Pacific/Auckland";              // Olson format "Pacific/Auckland" - Posix: "NZST-12NZDT,M9.5.0,M4.1.0/3" (as at 2020-06-18)
                                                     // For other Olson codes, go to: https://en.wikipedia.org/wiki/List_of_tz_database_time_zones 
String localNtpServer = "nz.pool.ntp.org";           // [Optional] - See full list of public NTP servers here: http://support.ntp.org/bin/view/Servers/WebHome
Timezone myTimeZone;                                 // Define this as a global so all functions have access to correct timezoned time


// Define where the Display is located
const int CLK = D2; //Set the CLK pin connection to the display
const int DIO = D3; //Set the DIO pin connection to the display

TM1637Display display(CLK, DIO); //set up the 4-Digit Display.

int timeValue = 0;         // We're sending the time to the LCD as an integer - 11:34pm is simply "2343".
int brightValue = 7;       // Start at full brightness
int LDRValue = 0;          // the value of the LDR (Light Dependent Resistor)
int LDRPin = A0;           // The pin to read the LDR
bool showDots = 1;         // Toggles every second


const uint8_t SEG_LAN[] = {                        // A bit unwieldy, but this bit sets up the alphanumeric "LAN-" display at clock boot time
  SEG_D | SEG_E | SEG_F,                           // L
  SEG_A | SEG_B | SEG_C | SEG_E | SEG_F | SEG_G,   // A
  SEG_A | SEG_B | SEG_C | SEG_E | SEG_F,           // N
  SEG_G                                            // -
  };

int redLedPin    = D7;                  // Using an LED to show some error conditions


time_t prevDisplay = 0; // when the digital clock was last displayed



void setup() {
  Serial.begin(115200);                                       // was 74880
  pinMode(redLedPin, OUTPUT);

  digitalWrite(redLedPin, HIGH);
  TestBrightness();

  display.setSegments(SEG_LAN);      //Display "-LAN" to screen
  delay(500);                        // ...for half a second. If wifi fails, it will stay on screen much longer.

  WiFiManager wifiManager;             // Start a Wifi link
  wifiManager.autoConnect("");         //   (on first use: will set up as a Wifi name; set up by selecting new Wifi network on iPhone for instance)

  setInterval(30);                     // [Optional] - How often should the time be reset to NTP server
  setDebug(INFO);                      // [Optional] - Print some info to the Serial monitor
  setServer(localNtpServer);           // [Optional] - Set the NTP server to the closest pool
  waitForSync();                       // Before you start the main loop, make sure the internal clock has synced at least once.
  myTimeZone.setLocation(TimeZoneDB);  // Set the Timezone
  
  myTimeZone.setDefault();             // Set the local timezone as the default time, so all time commands should now work local.

  
  


  digitalWrite(redLedPin, LOW);


}


void loop() {

 // Serial.println("[A] setup] UTC:   " + UTC.dateTime());                // <---- Example: prints UTC time          - https://time.is/UTC
 // Serial.println("[B] Default time: " + dateTime());                    // <---- Example: prints UTC time          - https://time.is/UTC
 // Serial.println("[C] Local time:   " + myTimeZone.dateTime());         // <---- Example: prints NZST or NZDS time - https://time.is/
 // Serial.println(" ");

  events();                        // Check if any scheduled ntp events, including NTP resyncs.
 
  
  if (now() != prevDisplay) {          //update the display only if time:seconds has changed

    checkNtpStatus();

    LDRValue = 0;
    LDRValue = analogRead(LDRPin);                   // reads the value of the LDR (value between 0 and 1023)
    brightValue = map(LDRValue, 0, 1025, -2, 7);     // scale it to use it with the brightness (value between 0 and 8)
    if (brightValue < 0)  brightValue = 0;
    prevDisplay = now();
    
    updateLCD();  

 }


  
}

void checkNtpStatus() {
  if (timeStatus() == timeSet) {
    digitalWrite(redLedPin, LOW);
  //  Serial.println("Status: Time set.  ");        // commented out - no need for an error message if all is well
  }
  if (timeStatus() == timeNotSet) {
    digitalWrite(redLedPin, HIGH);
    Serial.println("Status: Time NOT set.  ");
  }
  if (timeStatus() == timeNeedsSync) {
    digitalWrite(redLedPin, HIGH);
    Serial.println("Status: Time needs Syncing.      ");
  }
}




void TestBrightness() {               // ========== Brightness Test Run ============================================
  // Think of this as a simple POST for the clock
  
  int k;

  for(k = 0; k < 8; k++) {
    display.setBrightness(k);
    display.showNumberDec(k); //Display the timeValue value;
    delay(300);
  }

}




void updateLCD() {

    timeValue  = ((myTimeZone.hour())*100);
    timeValue += ((myTimeZone.minute()));

    display.setBrightness(brightValue);      //set the display to appropriate brightness

    showDots = !showDots;
    display.showNumberDecEx(timeValue, (showDots ? 127 : 0), true);   //(int num, uint8_t dots, bool leading_zero, uint8_t length, uint8_t pos)  

 }
