/* Written by JelleWho https://github.com/jellewie */
enum {UNUSED, NORMAL, RIGHT, UPSIDE_DOWN, LEFT, UNK};
String RotationNames[] = {"UNUSED", "NORMAL", "LEFT", "UPSIDE_DOWN", "RIGHT", "UNK"};
const byte Rotation_Amount = sizeof(RotationNames) / sizeof(RotationNames[0]); //Why filling this in if we can automate that? :)

bool StringIsDigit(String IN, char IgnoreCharA = '0', char IgnoreCharB = '0');
bool StringIsDigit(String IN, char IgnoreCharA, char IgnoreCharB) {
  //IgnoreChar can be used to ignore ',' or '.' or '-'
  for (byte i = 0; i < IN.length(); i++) {
    if (isDigit(IN.charAt(i))) {                                  //If it is a digit, do nothing
    } else if (IN.charAt(i) == IgnoreCharA) {                     //If it is IgnoreCharA, do nothing
    } else if (IN.charAt(i) == IgnoreCharB) {                     //If it is IgnoreCharB, do nothing
    } else {
      return false;
    }
  }
  return true;
}
byte ConvertRotationToByte(String IN) {
#ifdef SerialEnabled_Convert
  Serial.println("CV: ConvertRotationToByte '" + String(IN) + "'");
#endif //SerialEnabled_Convert
  if (StringIsDigit(IN)) {
    if (IN.toInt() < Rotation_Amount)
      return IN.toInt();
    else
      return UNK;
  }
  IN.trim();
  IN.toUpperCase();
  for (byte i = 0; i < Rotation_Amount; i++) {
    if (IN == RotationNames[i])
      return i;
  }
  return UNK;
}
String ConvertRotationToString(byte IN) {
#ifdef SerialEnabled_Convert
  Serial.println("CV: ConvertRotationToString '" + String(IN) + "'");
#endif //SerialEnabled_Convert
  if (IN < Rotation_Amount)
    return RotationNames[IN];
  return "UNK";
}
bool TickEveryXms(unsigned long *_LastTime, unsigned long _Delay) {
  //With overflow, can be adjusted, no overshoot correction, true when (Now < _LastTime + _Delay)
  /* Example:   static unsigned long LastTime;
                if (TickEveryXms(&LastTime, 1000)) {//Code to run}    */
  static unsigned long _Middle = -1;                              //We just need a really big number, if more than 0 and less than this amount of ms is passed, return true)
  if (_Middle == -1) _Middle = _Middle / 2;                       //Somehow declairing middle on 1 line does not work
  if (millis() - (*_LastTime + _Delay + 1) < _Middle) {           //If it's time to update (keep brackets for overflow protection). If diffrence between Now (millis) and Nextupdate (*_LastTime + _Delay) is smaller than a big number (_Middle) then update. Note that all negative values loop around and will be really big (for example -1 = 4,294,967,295)
    *_LastTime = millis();                                        //Set new LastTime updated
    return true;
  }
  return false;
}
void BlinkEveryMs(byte _LED, int _Delay) {
  static unsigned long LastTimeA = 2147999999;                    //Set really high above mid point, so the first call is true
  if (TickEveryXms(&LastTimeA, _Delay))
    digitalWrite(_LED, !digitalRead(_LED));                       //Blink LED
}




#ifdef SerialEnabled
String ResetReasonToString(byte Reason) {
  //https://github.com/espressif/esp-idf/blob/release/v3.0/components/esp32/include/rom/rtc.h#L80
  switch (Reason) {
    case 0  : return "NO_MEAN";                   break;
    case 1  : return "POWERON_RESET";             break; //1,  Vbat power on reset                         Rebooted by; power |or| Reset button |or| Software upload USB
    case 3  : return "SW_RESET";                  break; //3,  Software reset digital core                 rebooted by; software command "ESP.restart();"
    case 4  : return "OWDT_RESET";                break; //4,  Legacy watch dog reset digital core
    case 5  : return "DEEPSLEEP_RESET";           break; //5,  Deep Sleep reset digital core
    case 6  : return "SDIO_RESET";                break; //6,  Reset by SLC module, reset digital core
    case 7  : return "TG0WDT_SYS_RESET";          break; //7,  Timer Group0 Watch dog reset digital core
    case 8  : return "TG1WDT_SYS_RESET";          break; //8,  Timer Group1 Watch dog reset digital core
    case 9  : return "RTCWDT_SYS_RESET";          break; //9,  RTC Watch dog Reset digital core
    case 10 : return "INTRUSION_RESET";           break; //10, Instrusion tested to reset CPU
    case 11 : return "TGWDT_CPU_RESET";           break; //11, Time Group reset CPU
    case 12 : return "SW_CPU_RESET";              break; //12, Software reset CPU
    case 13 : return "RTCWDT_CPU_RESET";          break; //13, RTC Watch dog Reset CPU
    case 14 : return "EXT_CPU_RESET";             break; //14, for APP CPU, reseted by PRO CPU
    case 15 : return "RTCWDT_BROWN_OUT_RESET";    break; //15, Reset when the vdd voltage is not stable
    case 16 : return "RTCWDT_RTC_RESET";          break; //16, RTC Watch dog reset digital core and rtc module
  }
  return "UNK:" + String(Reason);
}
byte GetResetReason() {
  byte ResetReason = rtc_get_reset_reason(0);
  if (ResetReason == 0)
    ResetReason = rtc_get_reset_reason(1);
  return ResetReason;
}
String DoRequestReasonToString(byte Reason) {
  //https://github.com/espressif/esp-idf/blob/release/v3.0/components/esp32/include/rom/rtc.h#L80
  switch (Reason) {
    case 0  : return "REQ_UNK";                   break;
    case 1  : return "REQ_SUCCES";                break;
    case 2  : return "REQ_HUB_CONCECT_ERROR";     break;
    case 3  : return "REQ_TIMEOUT";               break;
    case 4  : return "REQ_PAGE_NOT_FOUND";        break;
    case 5  : return "REQ_SETUP_REQUIRED";        break;
    default : {
        if (Reason >= 10 and Reason <= 19)
          return "UNK response=" + String(Reason - 10) + "xx";
      } break;
  }
  return "UNK:" + String(Reason);
}
#endif //SerialEnabled
