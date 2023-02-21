/*
  Constants.h - Library API for sending data to the ThingsBoard
  Based on PubSub MQTT library.
  Created by Olender M. Oct 2018.
  Released into the public domain.
*/
#ifndef Constants_h
#define Constants_h

#include <stdarg.h>
#include <Arduino.h>
#if !THINGSBOARD_ENABLE_PROGMEM
#ifndef snprintf_P 
#define snprintf_P snprintf
#endif // snprintf_P
#ifndef vsnprintf_P 
#define vsnprintf_P vsnprintf
#endif // vsnprintf_P
#endif // !THINGSBOARD_ENABLE_PROGMEM

#define Default_Payload 64
#define Default_Fields_Amt 8
class ThingsBoardDefaultLogger;

/// ---------------------------------
/// Constant strings in flash memory,
/// used by both ThingsBoard and ThingsBoardHttp.
/// ---------------------------------
// Log messages.
#if THINGSBOARD_ENABLE_PROGMEM
constexpr char UNABLE_TO_SERIALIZE[] PROGMEM = "Unable to serialize key-value json";
constexpr char TOO_MANY_JSON_FIELDS[] PROGMEM = "Too many JSON fields passed (%u), increase MaxFieldsAmt (%u) accordingly";
constexpr char UNABLE_TO_DE_SERIALIZE_JSON[] PROGMEM = "Unable to de-serialize received json data with error (%s)";
constexpr char CONNECT_FAILED[] PROGMEM = "Connecting to server failed";
constexpr char UNABLE_TO_SERIALIZE_JSON[] PROGMEM = "Unable to serialize json data";
#else
constexpr char UNABLE_TO_SERIALIZE[] = "Unable to serialize key-value json";
constexpr char TOO_MANY_JSON_FIELDS[] = "Too many JSON fields passed (%u), increase MaxFieldsAmt (%u) accordingly";
constexpr char UNABLE_TO_DE_SERIALIZE_JSON[] = "Unable to de-serialize received json data with error (%s)";
constexpr char CONNECT_FAILED[] = "Connecting to server failed";
constexpr char UNABLE_TO_SERIALIZE_JSON[] = "Unable to serialize json data";
#endif // THINGSBOARD_ENABLE_PROGMEM

#endif // Constants_h
