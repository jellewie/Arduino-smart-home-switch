/*Written by JelleWho https://github.com/jellewie
  TODO:  https://github.com/jellewie/Arduino-smart-home-switch/issues
*/
#if !defined(ESP32)
#error "Please check if the 'DOIT ESP32 DEVKIT V1' board is selected, which can be downloaded at https://dl.espressif.com/dl/package_esp32_index.json"
#endif

//#define SerialEnabled

#ifdef SerialEnabled
#include <rom/rtc.h>                                            //This is for rtc_get_reset_reason
#define OTA_EnabledOnBoot
#define WiFiManager_SerialEnabled
#define SerialEnabled_CheckButton         //CB:
//#define Button_SerialEnabled            //BU:
//#define SerialEnabled_Speed             //SP:
//#define SerialEnabled_Convert           //CV:
#endif //SerialEnabled

#include "WiFiManagerBefore.h"                                  //Define what options to use/include or to hook into WiFiManager
#include "WiFiManager/WiFiManager.h"                            //Includes <WiFi> and <WebServer.h> and setups up 'WebServer server(80)' if needed      https://github.com/jellewie/Arduino-WiFiManager
#include "functions.h"
#include "Button/Button.h"                                      //https://github.com/jellewie/Arduino-Button

char Hub_IP[16] = "192.168.255.255";                            //The url to connect to
int Hub_Port = 80;
const byte AB = 4;                                              //Amount_Buttons
Button ButtonA[AB] = {{32, INPUT_PULLUP, 21}, {33, INPUT_PULLUP, 19}, {25, INPUT_PULLUP, 18}, {26, INPUT_PULLUP, 5}}; //Bunch up the 4 buttons to be 1 switch set (Only used for reference pin pares, not which command is connected to which pin)
Button ButtonB[AB] = {{27, INPUT_PULLUP, 23}, {14, INPUT_PULLUP, 22}, {12, INPUT_PULLUP, 4}, {13, INPUT_PULLUP, 15}}; // ^
String Path_A[AB] = {"", "", "", ""};                           //SOFT_SETTING
String Path_B[AB] = {"", "", "", ""};                           // ^
String Json_A[AB] = {"", "", "", ""};                           //SOFT_SETTING
String Json_B[AB] = {"", "", "", ""};                           // ^
byte RotationA = NORMAL;                                        //SOFT_SETTING Rotation of the PCB seen from the case
byte RotationB = UNUSED;                                        // ^           RIGHT=PCB 90° clockwise to case
const byte BootButtonsWaitMs = 100;                             //Wait for a period of this length on boot for the buttons to be NOT pressed

#include "WiFiManagerLater.h"                                   //Define options of WiFiManager (can also be done before), but WiFiManager can also be called here (example for DoRequest)

void setup() {
#ifdef SerialEnabled
  Serial.begin(115200);
#endif //SerialEnabled
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  //===========================================================================
  //Load data, since we need this to check if RotationB is !UNUSED)
  //===========================================================================
  WiFiManager.LoadData();
  //===========================================================================
  //Attach interrupts to the button pins so we can response if they change
  //===========================================================================
  attachInterrupt(ButtonA[0].PIN_Button, ISR_A0, CHANGE);
  attachInterrupt(ButtonA[1].PIN_Button, ISR_A1, CHANGE);
  attachInterrupt(ButtonA[2].PIN_Button, ISR_A2, CHANGE);
  attachInterrupt(ButtonA[3].PIN_Button, ISR_A3, CHANGE);
  if (RotationB != UNUSED) {                                    //Do not attach interrupt if this switch set is unused
    attachInterrupt(ButtonB[0].PIN_Button, ISR_B0, CHANGE);
    attachInterrupt(ButtonB[1].PIN_Button, ISR_B1, CHANGE);
    attachInterrupt(ButtonB[2].PIN_Button, ISR_B2, CHANGE);
    attachInterrupt(ButtonB[3].PIN_Button, ISR_B3, CHANGE);
  }
  //===========================================================================
  //Wait for all buttons to be NOT pressed
  //===========================================================================
  unsigned long WaitUntil = millis() + BootButtonsWaitMs;
  while (WaitUntil > millis()) {
    byte ButtonStatesA = 0, ButtonStatesB = 0;
    for (byte i = 0; i < AB; i++) {
      if (ButtonA[i].CheckButton().Pressed) {
        bitSet(ButtonStatesA, i);
        WaitUntil = millis() + BootButtonsWaitMs;
      }
      if (RotationB != UNUSED) {
        if (ButtonB[i].CheckButton().Pressed) {
          bitSet(ButtonStatesB, i);
          WaitUntil = millis() + BootButtonsWaitMs;
        }
      }
    }
    static unsigned long LastTimeA = 0;                         //Set to 0, so the first call is FALSE
    if (TickEveryXms(&LastTimeA, 50)) {                         //Execute every 50ms (so an error blink would look nice)
      for (byte i = 0; i < AB; i++) {
        if (bitRead(ButtonStatesA, i)) {
          bitClear(ButtonStatesA, i);
          if (ButtonA[i].PIN_LED > 0) digitalWrite(ButtonA[i].PIN_LED, !digitalRead(ButtonA[i].PIN_LED));
        } else {
          if (ButtonA[i].PIN_LED > 0) digitalWrite(ButtonA[i].PIN_LED, LOW);
        }
        if (bitRead(ButtonStatesB, i)) {
          bitClear(ButtonStatesB, i);
          if (ButtonB[i].PIN_LED > 0) digitalWrite(ButtonB[i].PIN_LED, !digitalRead(ButtonB[i].PIN_LED));
        } else {
          if (ButtonB[i].PIN_LED > 0) digitalWrite(ButtonB[i].PIN_LED, LOW);
        }
      }
    }
#ifdef SerialEnabled
    Serial.println("Waiting on a button(s) " + String(ButtonStatesA, BIN) + "_" + String(ButtonStatesB, BIN) + " before starting up, wait ms=" + String(WaitUntil - millis()));
#endif //SerialEnabled
  }
  for (byte i = 0; i < AB; i++) {
    digitalWrite(ButtonA[i].PIN_LED, LOW);                      //Make sure all LED's are off
    if (RotationB != UNUSED)
      digitalWrite(ButtonB[i].PIN_LED, LOW);                    //Make sure all LED's are off
  }
  //===========================================================================
  //Start WIFI
  //===========================================================================
  WiFi.setSleep(false);                                         //Disable going to sleep, basically the same as 'esp_wifi_set_ps(WIFI_PS_NONE)' https://github.com/espressif/arduino-esp32/blob/master/libraries/WiFi/src/WiFiGeneric.cpp#L1070
  server.on("/test", handle_Test);                              //Declair the TEST urls
  byte Answer = WiFiManager.Start();
  if (Answer != 1) {
#ifdef SerialEnabled
    Serial.println("setup error code '" + String(Answer) + "'");  delay(10);
#endif //SerialEnabled
    ESP.restart();                                              //Restart the ESP
  }
  WiFiManager.OTA_Enabled = false;
  WiFiManager.EnableSetup(true);                                //Start the server (if you also need it for other stuff)
  //===========================================================================
  //Get Reset reason (This could be/is useful for power outage?)
  //===========================================================================
#ifdef OTA_EnabledOnBoot
  WiFiManager.OTA_Enabled = true;
#  ifdef SerialEnabled
  Serial.println("Due to 'OTA_EnabledOnBoot' debug mode, OTA is enabled after boot by default");
#  endif //SerialEnabled
#endif //OtaEnabledOnBoot
#ifdef SerialEnabled
  Serial.println("Done with boot, resetted due to " + ResetReasonToString(GetResetReason()) + " boottime=" + String(millis()));
#endif //SerialEnabled
}
//===========================================================================
void loop() {
  if (WiFiManager.OTA_Enabled) {
    WiFiManager.RunServer();                                    //Do WIFI server stuff if needed
    BlinkEveryMs(LED_BUILTIN, 1000);                            //Blink every x MS to show OTA is on
  }

  static byte OldRotationB = RotationB;                         //Check if a second switch is added (if a rotation is set)
  if (RotationB != OldRotationB) {
    if (OldRotationB == 0 or RotationB == 0) {
#ifdef SerialEnabled
      Serial.println("ESP must reset due to Second pair added/removed");  delay(10);
#endif //SerialEnabled
      ESP.restart();                                            //Restart the ESP
    }
  }

  for (byte i = 0; i < AB; i++) {
    Check(ButtonA[i].CheckButton(),                             //The button state (contains info like if its just pressed and such)
          Path_A[RotatedButtonID(RotationA, i)] == "" ? Path_A[0] : Path_A[RotatedButtonID(RotationA, i)] , //The URL, if(non given){default to first given path}
          Json_A[RotatedButtonID(RotationA, i)],                //The command to execute
          ButtonA[i].PIN_LED);                                  //The LED corospanding to this button
    if (RotationB != UNUSED)
      Check(ButtonB[i].CheckButton(),
            Path_B[RotatedButtonID(RotationB, i)] == "" ? Path_B[0] : Path_B[RotatedButtonID(RotationB, i)] ,
            Json_B[RotatedButtonID(RotationB, i)],
            ButtonB[i].PIN_LED);
  }
}
//===========================================================================
byte Check(Button_Time Value, String Path, String Json, byte LEDpin) {
  byte Answer = 0;
#ifdef SerialEnabled_CheckButton
  if (Value.StartPress or Value.Pressed or
      Value.StartLongPress or Value.PressedLong or
      Value.StartDoublePress or Value.DoublePress or
      Value.StartRelease)                   //If there was an update
    Serial.println("CB: S" + String(Value.StartPress) + "_" + String(Value.Pressed) + " "
                   "L" + String(Value.StartLongPress) + "_" + String(Value.PressedLong) + " "
                   "D" + String(Value.StartDoublePress) + "_" + String(Value.DoublePress) + " "
                   "R" + String(Value.StartRelease) + "_" + String(Value.PressEnded) + " "
                   "T" + String(Value.PressedTime) + " "
                   "LEDpin=" + String(LEDpin) + " Path=" + String(Path) + " Json=" + String(Json));
#endif //SerialEnabled_CheckButton

  if (Value.StartPress) {                                       //If button is just pressed in

#ifdef SerialEnabled_Speed
    unsigned long StartMS = millis();
#endif //SerialEnabled_Speed

    if (LEDpin > 0) digitalWrite(LEDpin, HIGH);                 //If a LED pin was given; Set that buttons LED on
    Answer = WiFiManager.DoRequest(Hub_IP, Hub_Port, Path, Json);
    if (LEDpin > 0) digitalWrite(LEDpin, LOW);                  //If a LED pin was given; Set that buttons LED off

#ifdef SerialEnabled
    Serial.println("DoRequest executed with responce code '" + DoRequestReasonToString(Answer) + "'"); //The return codes can be found in "WiFiManager.cpp" in "CWiFiManager::DoRequest("
#  ifdef SerialEnabled_Speed
    Serial.println("SP: Command processing time (start to end)=" + String(millis() - StartMS) + "ms");
#  endif //SerialEnabled_Speed
#endif //SerialEnabled

  } else if (Value.StartLongPress) {
    WiFiManager.OTA_Enabled = !WiFiManager.OTA_Enabled;         //Toggle OTA on/off
    if (LEDpin > 0) digitalWrite(LEDpin, LOW);                  //If a LED pin was given; Set that buttons LED off
#ifdef SerialEnabled
    Serial.println("Toggled OTA " + String(WiFiManager.OTA_Enabled ? "ON" : "OFF"));
#endif //SerialEnabled
  }
  if (Value.PressedLong) {                                      //If it is/was a long press
    if (Value.Pressed) {                                        //If we are still pressing
      if (Value.PressedTime > Time_ESPrestartMS - 2000) {
        if (LEDpin > 0) BlinkEveryMs(LEDpin, 10);               //If a LED pin was given; Blink that button LED
      } else
        digitalWrite(LED_BUILTIN, HIGH);
    } else {
      digitalWrite(LED_BUILTIN, LOW);
      if (LEDpin > 0) digitalWrite(LEDpin, LOW);                //If a LED pin was given; Blink that button LED
    }
  }
  return Answer;
}
byte RotatedButtonID(byte Rotation, byte i) {
  switch (Rotation) {
    case NORMAL:
      return i;
    case LEFT:
      switch (i) {
        case 0:   return 1;
        case 1:   return 3;
        case 2:   return 0;
        case 3:   return 2;
      }
    case UPSIDE_DOWN:
      switch (i) {
        case 0:   return 3;
        case 1:   return 2;
        case 2:   return 1;
        case 3:   return 0;
      }
    case RIGHT:
      switch (i) {
        case 0:   return 2;
        case 1:   return 0;
        case 2:   return 3;
        case 3:   return 1;
      }
  }
  return 0;
}
void handle_Test() {
  byte i = 0;
  char m = 'A';
  if (server.args()) {
    String ArguName = server.argName(i);
    ArguName.toUpperCase();
    ArguName.substring(0, 1);
    if (ArguName == "B") m = 'B';
    i = server.arg(i).toInt();
    if (i > 4) i = 3;
    if (i < 1) i = 1;
  }
  String Path = Path_A[RotatedButtonID(RotationA, i - 1)] == "" ? Path_A[0] : Path_A[RotatedButtonID(RotationA, i - 1)];
  String Json = Json_A[RotatedButtonID(RotationA, i - 1)];
  byte LEDpin = ButtonA[i].PIN_LED;
  if (m == 'B') {
    Path = Path_B[RotatedButtonID(RotationB, i - 1)] == "" ? Path_B[0] : Path_B[RotatedButtonID(RotationB, i - 1)];
    Json = Json_B[RotatedButtonID(RotationB, i - 1)];
    LEDpin = ButtonB[i].PIN_LED;
  }

  Button_Time Dummy;
  Dummy.StartPress = true;
  byte Answer = Check(Dummy, Path, Json, LEDpin);

  server.send(200, "text/plain", "DoRequest "  + String(m) + String(i) + " with result " + String(Answer) + "\n\n"
              "" + String(Hub_IP) + ":" + String(Hub_Port) + String(Path) + "\n"
              "" + String(Json));
}
//===========================================================================
void ISR_A0() {
  ButtonA[0].Pinchange();
}
void ISR_A1() {
  ButtonA[1].Pinchange();
}
void ISR_A2() {
  ButtonA[2].Pinchange();
}
void ISR_A3() {
  ButtonA[3].Pinchange();
}
void ISR_B0() {
  ButtonB[0].Pinchange();
}
void ISR_B1() {
  ButtonB[1].Pinchange();
}
void ISR_B2() {
  ButtonB[2].Pinchange();
}
void ISR_B3() {
  ButtonB[3].Pinchange();
}
