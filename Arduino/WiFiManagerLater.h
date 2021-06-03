/* Written by JelleWho https://github.com/jellewie
   https://github.com/jellewie/Arduino-WiFiManager
*/
//===========================================================================
// Things that can/need to be defined after including "WiFiManager.h"
//===========================================================================
bool WiFiManagerUser_Set_Value(byte ValueID, String Value) {
  Value.trim();
  switch (ValueID) {                                            //Note the numbers are shifted from what is in memory, 0 is the first user value
    case 0: {
        if (Value.length() > sizeof(Name))        return false; //Length is to long, it would not fit so stop here
        Value.toCharArray(Name, 16);              return true;
      } break;
    case 1: {
        if (Value.length() > sizeof(Hub_IP))      return false; //Length is to long, it would not fit so stop here
        Value.toCharArray(Hub_IP, 16);            return true;
      } break;
    case 2: {
        if (not StringIsDigit(Value))             return false; //No number given
        Hub_Port = Value.toInt();                 return true;
      } break;
    case 3: {
        byte Rotation = ConvertRotationToByte(Value);
        if (Rotation == UNK)                      return false; //Not set, Rotation is out of range
        if (Rotation == UNUSED)                   return false; //Switch 1 can not be disabled
        RotationA = Rotation;                     return true;    break;
      } break;

    case 4:   Path_A[0] = Value;                  return true;    break;
    case 5:   Path_A[1] = Value;                  return true;    break;
    case 6:   Path_A[2] = Value;                  return true;    break;
    case 7:   Path_A[3] = Value;                  return true;    break;

    case 8:   Json_A[0] = Value;                  return true;    break;
    case 9:   Json_A[1] = Value;                  return true;    break;
    case 10:  Json_A[2] = Value;                  return true;    break;
    case 11:  Json_A[3] = Value;                  return true;    break;
    case 12: {
        byte Rotation = ConvertRotationToByte(Value);
        if (Rotation == UNK)                      return false; //Not set, Rotation is out of range
        RotationB = Rotation;                     return true;    break;
      } break;
    case 13:  Path_B[0] = Value;                  return true;    break;
    case 14:  Path_B[1] = Value;                  return true;    break;
    case 15:  Path_B[2] = Value;                  return true;    break;
    case 16:  Path_B[3] = Value;                  return true;    break;
    case 17:  Json_B[0] = Value;                  return true;    break;
    case 18:  Json_B[1] = Value;                  return true;    break;
    case 19:  Json_B[2] = Value;                  return true;    break;
    case 20:  Json_B[3] = Value;                  return true;    break;
  }
  return false;                                                 //Report back that the ValueID is unknown, and we could not set it
}
String WiFiManagerUser_Get_Value(byte ValueID, bool Safe, bool Convert) {
  //if its 'Safe' to return the real value (for example the password will return '****' or '1234')
  //'Convert' the value to a readable string for the user (bool '0/1' to 'FALSE/TRUE')
  switch (ValueID) {                                            //Note the numbers are shifted from what is in memory, 0 is the first user value
    case 0:   return String(Name);                break;
    case 1:   return String(Hub_IP);              break;
    case 2:   return String(Hub_Port);            break;
    case 3:   return ConvertRotationToString(RotationA); break;
    case 4:   return Path_A[0];                   break;
    case 5:   return Path_A[1];                   break;
    case 6:   return Path_A[2];                   break;
    case 7:   return Path_A[3];                   break;
    case 8:   return Json_A[0];                   break;
    case 9:   return Json_A[1];                   break;
    case 10:  return Json_A[2];                   break;
    case 11:  return Json_A[3];                   break;
    case 12:  return ConvertRotationToString(RotationB); break;
    case 13:  return Path_B[0];                   break;
    case 14:  return Path_B[1];                   break;
    case 15:  return Path_B[2];                   break;
    case 16:  return Path_B[3];                   break;
    case 17:  return Json_B[0];                   break;
    case 18:  return Json_B[1];                   break;
    case 19:  return Json_B[2];                   break;
    case 20:  return Json_B[3];                   break;
  }
  return "";
}
void WiFiManagerUser_Status_Start() {                           //Called before start of WiFi
  digitalWrite(LED_BUILTIN, HIGH);
}
void WiFiManagerUser_Status_Done() {                            //Called after succesfull connection to WiFi
  digitalWrite(LED_BUILTIN, LOW);
}
void WiFiManagerUser_Status_Blink() { //Used when trying to connect/not connected
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
}
void WiFiManagerUser_Status_StartAP() {}
bool WiFiManagerUser_HandleAP() {                               //Called when in the While loop in APMode, this so you can exit it
  //Return true to leave APmode
#define TimeOutApMode 15 * 60 * 1000;                           //Example for a timeout, (time in ms)
  unsigned long StopApAt = millis() + TimeOutApMode;
  if (millis() > StopApAt)    return true;                      //If we are running for to long, then flag we need to exit APMode
  return false;
}
