/*
  Program written by JelleWho
  Board: https://dl.espressif.com/dl/package_esp32_index.json
  Sketch from: https://github.com/jellewie/Arduino-smart-home-switch
*/
#if !defined(ESP32)
#error "Please check if the 'DOIT ESP32 DEVKIT V1' board is selected, which can be downloaded at https://dl.espressif.com/dl/package_esp32_index.json"
#endif

//#define SerialEnabled

#ifdef SerialEnabled
#include <rom/rtc.h>                                            //This is for rtc_get_reset_reason
#define WiFiManager_SerialEnabled
//#define Button_SerialEnabled
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
Button ButtonA[AB] = {{34, INPUT_PULLUP, 21}, {35, INPUT_PULLUP, 19}, {32, INPUT_PULLUP, 18}, {33, INPUT_PULLUP, 5}}; //Bunch up the 4 buttons to be 1 switch set (Only used for reference pin pares, not which command is connected to which pin)
Button ButtonB[AB] = {{26, INPUT_PULLUP, 23}, {27, INPUT_PULLUP, 22}, {14, INPUT_PULLUP, 4}, {12, INPUT_PULLUP, 15}}; // ^
String Path_A[AB] = {"", "", "", ""};                           //SOFT_SETTING
String Path_B[AB] = {"", "", "", ""};                           // ^
String Json_A[AB] = {"", "", "", ""};                           //SOFT_SETTING
String Json_B[AB] = {"", "", "", ""};                           // ^
byte RotationA = NORMAL;                                        //SOFT_SETTING Rotation of the PCB seen from the case
byte RotationB = UNUSED;                                        // ^           RIGHT=PCB 90° clockwise to case

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
  byte ButtonPressedID = 1;
  while (ButtonPressedID > 0) {
#ifdef SerialEnabled
    Serial.println("Waiting on a button(s) " + String(ButtonPressedID, BIN) + " before starting up");
#endif //SerialEnabled
    ButtonPressedID = 0;                                        //Set to NOT pressed by default, will be overwritten
    static unsigned long LastTimeA = 0;                         //Set to 0, so the first call is FALSE
    if (TickEveryXms(&LastTimeA, 50)) {                         //Wait here for 50ms (so an error blink would look nice)
      //Returns the button states in bits; Like 0000<button1><b2><b3><b4> where 1 is HIGH and 0 is LOW
      //Example '00001001' = Buttons 1 and 4 are HIGH (Note we count from LSB)
      byte ButtonID = 0;
      for (byte i = 0; i < AB; i++) {
        ButtonID = ButtonID << 1;                               //Move bits 1 to the left (it’s like *2)
        Button_Time Value = ButtonA[i].CheckButton();
        if (Value.Pressed) {
          ButtonID += 1;                                        //Flag this button as on
          if (ButtonA[i].PIN_LED > 0) digitalWrite(ButtonA[i].PIN_LED, !digitalRead(ButtonA[i].PIN_LED));
        } else if (ButtonA[i].PIN_LED > 0)
          digitalWrite(ButtonA[i].PIN_LED, LOW);
      }
      if (RotationB != UNUSED) {
        for (byte i = 0; i < AB; i++) {
          ButtonID = ButtonID << 1;                             //Move bits 1 to the left (it’s like *2)
          Button_Time Value = ButtonB[i].CheckButton();
          if (Value.Pressed) {
            ButtonID += 1;                                      //Flag this button as on
            if (ButtonA[i].PIN_LED > 0) digitalWrite(ButtonB[i].PIN_LED, !digitalRead(ButtonB[i].PIN_LED));
          } else if (ButtonB[i].PIN_LED > 0)
            digitalWrite(ButtonB[i].PIN_LED, LOW);
        }
      }
      ButtonPressedID = ButtonID;                               //Get the button state, here 1 is HIGH in the form of '0000<Button 1><2><3><4> '
    }
  }
  for (byte i = 0; i < AB; i++) {
    digitalWrite(ButtonA[i].PIN_LED, LOW);                      //Make sure all LED's are off
    if (RotationB != UNUSED)
      digitalWrite(ButtonB[i].PIN_LED, LOW);                    //Make sure all LED's are off
  }
  //===========================================================================
  //Start WIFI
  //===========================================================================
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
  //Get Reset reason (This could be/is useful for power outage)
  //===========================================================================
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
    Check(ButtonA[i].CheckButton(),                                             //The button state (contains info like if its just pressed and such)
          Path_A[i] == "" ? Path_A[1] : Path_A[RotatedButtonID(RotationA, i)] ,  //The URL, if(non given){default to first given path}
          Json_A[RotatedButtonID(RotationA, i)],                                 //The command to execute
          ButtonA[i].PIN_LED);                                                  //The LED corospanding to this button
    if (RotationB != UNUSED)
      Check(ButtonB[i].CheckButton(),
            Path_B[i] == "" ? Path_B[1] : Path_A[RotatedButtonID(RotationB, i)] ,
            Json_B[RotatedButtonID(RotationB, i)],
            ButtonB[i].PIN_LED);
  }
}
//===========================================================================
void Check(Button_Time Value, String Path, String Json, byte LEDpin) {
#ifdef SerialEnabled
  Serial.println(String(Value.Pressed) + " S=" + String(Value.StartPress) + " L=" + String(Value.PressedLong) + " SL=" + String(Value.StartLongPress) + " LEDpin=" + String(LEDpin) + " Path=" + String(Path) + " Json=" + String(Json));
#endif //SerialEnabled

  if (Value.StartPress) {                                       //If button is just pressed in

#ifdef SerialEnabled_Speed
    unsigned long StartMS = millis();
#endif //SerialEnabled_Speed

    if (LEDpin > 0) digitalWrite(LEDpin, HIGH);                 //If a LED pin was given; Set that buttons LED on
    byte Answer = WiFiManager.DoRequest(Hub_IP, Hub_Port, Path, Json);
    if (LEDpin > 0) digitalWrite(LEDpin, LOW);                  //If a LED pin was given; Set that buttons LED off

    Answer = Answer + 0;                                        //mask compiler warning if 'SerialEnabled' is not defined
#ifdef SerialEnabled
    Serial.println("DoRequest executed with responce code '" + DoRequestReasonToString(Answer) + "'"); //The return codes can be found in "WiFiManager.cpp" in "CWiFiManager::DoRequest("
#  ifdef SerialEnabled_Speed
    Serial.println("SP: Command processing time (start to end)=" + String(millis() - StartMS) + "ms");
#  endif //SerialEnabled_Speed
#endif //SerialEnabled

  } else if (Value.StartLongPress) {
    WiFiManager.OTA_Enabled = !WiFiManager.OTA_Enabled;         //Toggle OTA on/off
    if (LEDpin > 0) digitalWrite(LEDpin, LOW);                  //If a LED pin was given; Set that buttons LED off
  }
  if (Value.PressedLong) {                                      //If it is/was a long press
    if (Value.Pressed) {                                        //If we are still pressing
      if (Value.PressedTime > Time_ESPrestartMS - 1000) {
        if (LEDpin > 0) BlinkEveryMs(LEDpin, 10);               //If a LED pin was given; Blink that button LED
      } else
        digitalWrite(LED_BUILTIN, HIGH);
    } else {
      digitalWrite(LED_BUILTIN, LOW);
      if (LEDpin > 0) digitalWrite(LEDpin, LOW);        //If a LED pin was given; Blink that button LED
    }
  }
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
