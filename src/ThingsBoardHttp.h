/*
  ThingsBoardHttp.h - Library API for sending data to the ThingsBoard
  Based on PubSub MQTT library.
  Created by Olender M. Oct 2018.
  Released into the public domain.
*/
#ifndef ThingsBoard_Http_h
#define ThingsBoard_Http_h

// Local includes.
#include "Constants.h"
#include "Telemetry.h"
#include "Helper.h"
#include "IHTTP_Client.h"
#include "ILogger.h"


// HTTP topics.
#if THINGSBOARD_ENABLE_PROGMEM
constexpr char HTTP_TELEMETRY_TOPIC[] PROGMEM = "/api/v1/%s/telemetry";
constexpr char HTTP_ATTRIBUTES_TOPIC[] PROGMEM = "/api/v1/%s/attributes";
constexpr char HTTP_POST_PATH[] PROGMEM = "application/json";
constexpr int HTTP_RESPONSE_SUCCESS_RANGE_START PROGMEM = 200;
constexpr int HTTP_RESPONSE_SUCCESS_RANGE_END PROGMEM = 299;
#else
constexpr char HTTP_TELEMETRY_TOPIC[] = "/api/v1/%s/telemetry";
constexpr char HTTP_ATTRIBUTES_TOPIC[] = "/api/v1/%s/attributes";
constexpr char HTTP_POST_PATH[] = "application/json";
constexpr int HTTP_RESPONSE_SUCCESS_RANGE_START = 200;
constexpr int HTTP_RESPONSE_SUCCESS_RANGE_END = 299;
#endif // THINGSBOARD_ENABLE_PROGMEM

// Log messages.
#if THINGSBOARD_ENABLE_PROGMEM
constexpr char POST[] PROGMEM = "POST";
constexpr char GET[] PROGMEM = "GET";
constexpr char HTTP_FAILED[] PROGMEM = "(%s) failed HTTP response (%d)";
#else
constexpr char POST[] = "POST";
constexpr char GET[] = "GET";
constexpr char HTTP_FAILED[] = "(%s) failed HTTP response (%d)";
#endif // THINGSBOARD_ENABLE_PROGMEM


#if THINGSBOARD_ENABLE_DYNAMIC
/// @brief Wrapper around the ArduinoHttpClient or HTTPClient to allow connecting and sending / retrieving data from ThingsBoard over the HTTP orHTTPS protocol.
/// BufferSize of the underlying data buffer as well as the maximum amount of data points that can ever be sent are either dynamic or can be changed during runtime.
/// If this feature is not needed and the values can be sent once as template arguements it is recommended to use the static ThingsBoard instance instead.
/// Simply set THINGSBOARD_ENABLE_DYNAMIC to 0, before including ThingsBoardHttp.h
#else
/// @brief Wrapper around the ArduinoHttpClient or HTTPClient to allow connecting and sending / retrieving data from ThingsBoard over the HTTP orHTTPS protocol.
/// BufferSize of the underlying data buffer as well as the maximum amount of data points that can ever be sent have to defined as template arguments.
/// Changing is only possible if a new instance of this class is created. If theese values should be changeable and dynamic instead.
/// Simply set THINGSBOARD_ENABLE_DYNAMIC to 1, before including ThingsBoardHttp.h.
/// @tparam MaxFieldsAmount Maximum amount of key value pair that we will be able to sent to ThingsBoard in one call, default = 8
template<size_t MaxFieldsAmount = Default_Fields_Amount>
#endif // THINGSBOARD_ENABLE_DYNAMIC
class ThingsBoardHttpSized {
  public:
    /// @brief Initalizes the underlying client with the needed information
    /// so it can initally connect to the given host over the given port
    /// @param client Client that should be used to establish the connection
    /// @param access_token Token used to verify the devices identity with the ThingsBoard server
    /// @param host Host server we want to establish a connection to (example: "demo.thingsboard.io")
    /// @param port Port we want to establish a connection over (80 for HTTP, 443 for HTTPS)
    /// @param keepAlive Attempts to keep the establishes TCP connection alive to make sending data faster
    /// @param maxStackSize Maximum amount of bytes we want to allocate on the stack, default = Default_Max_Stack_Size
    inline ThingsBoardHttpSized(IHTTP_Client& client, const ILogger& logger, const char *access_token, const char *host, const uint16_t& port = 80U, const bool& keepAlive = true, const size_t& maxStackSize = Default_Max_Stack_Size)
      : m_client(client)
      , m_logger(logger)
      , m_max_stack(maxStackSize)
      , m_token(access_token)
    {
      m_client.set_keep_alive(keepAlive);
      m_client.connect(host, port);
    }

    /// @brief Sets the maximum amount of bytes that we want to allocate on the stack, before the memory is allocated on the heap instead
    /// @param maxStackSize Maximum amount of bytes we want to allocate on the stack
    inline void setMaximumStackSize(const size_t& maxStackSize) {
      m_max_stack = maxStackSize;
    }

    /// @brief Attempts to send key value pairs from custom source over the given topic to the server
    /// @tparam TSource Source class that should be used to serialize the json that is sent to the server
    /// @param topic Topic we want to send the data over
    /// @param source Data source containing our json key value pairs we want to send
    /// @param jsonSize Size of the data inside the source
    /// @return Whether sending the data was successful or not
    template <typename TSource>
    inline bool Send_Json(const char* topic, const TSource& source, const size_t& jsonSize) {
      // Check if allocating needed memory failed when trying to create the JsonObject,
      // if it did the isNull() method will return true. See https://arduinojson.org/v6/api/jsonvariant/isnull/ for more information
      if (source.isNull()) {
        m_logger.log(UNABLE_TO_ALLOCATE_MEMORY);
        return false;
      }
#if !THINGSBOARD_ENABLE_DYNAMIC
      const size_t amount = source.size();
      if (MaxFieldsAmount < amount) {
        m_logger.logf(TOO_MANY_JSON_FIELDS, amount, MaxFieldsAmount);
        return false;
      }
#endif // !THINGSBOARD_ENABLE_DYNAMIC
      bool result = false;

      if (getMaximumStackSize() < jsonSize) {
        char* json = new char[jsonSize];
        if (serializeJson(source, json, jsonSize) < jsonSize - 1) {
          m_logger.log(UNABLE_TO_SERIALIZE_JSON);
        }
        else {
          result = Send_Json_String(topic, json);
        }
        // Ensure to actually delete the memory placed onto the heap, to make sure we do not create a memory leak
        // and set the pointer to null so we do not have a dangling reference.
        delete[] json;
        json = nullptr;
      }
      else {
        char json[jsonSize];
        if (serializeJson(source, json, jsonSize) < jsonSize - 1) {
          m_logger.log(UNABLE_TO_SERIALIZE_JSON);
          return result;
        }
        result = Send_Json_String(topic, json);
      }

      return result;
    }

    /// @brief Attempts to send custom json string over the given topic to the server
    /// @param topic Topic we want to send the data over
    /// @param json String containing our json key value pairs we want to attempt to send
    /// @return Whether sending the data was successful or not
    inline bool Send_Json_String(const char* topic, const char* json) {
      if (json == nullptr || m_token == nullptr) {
        return false;
      }

      char path[Helper::detectSize(topic, m_token)];
      snprintf(path, sizeof(path), topic, m_token);
      return postMessage(path, json);
    }

    //----------------------------------------------------------------------------
    // Telemetry API

    /// @brief Attempts to send telemetry data with the given key and value of the given type.
    /// See https://thingsboard.io/docs/user-guide/telemetry/ for more information
    /// @tparam T Type of the passed value
    /// @param key Key of the key value pair we want to send
    /// @param value Value of the key value pair we want to send
    /// @return Whether sending the data was successful or not
    template<typename T>
    inline bool sendTelemetryData(const char *key, T value) {
      return sendKeyValue(key, value);
    }

    /// @brief Attempts to send aggregated telemetry data.
    /// See https://thingsboard.io/docs/user-guide/telemetry/ for more information
    /// @param data Array containing all the data we want to send
    /// @param data_count Amount of data entries in the array that we want to send
    /// @return Whetherr sending the data was successful or not
    inline bool sendTelemetry(const Telemetry *data, size_t data_count) {
      return sendDataArray(data, data_count);
    }

    /// @brief Attempts to send custom json telemetry string.
    /// See https://thingsboard.io/docs/user-guide/telemetry/ for more information
    /// @param json String containing our json key value pairs we want to attempt to send
    /// @return Whetherr sending the data was successful or not
    inline bool sendTelemetryJson(const char *json) {
      return Send_Json_String(HTTP_TELEMETRY_TOPIC, json);
    }

    /// @brief Attempts to send telemetry key value pairs from custom source to the server.
    /// See https://thingsboard.io/docs/user-guide/telemetry/ for more information
    /// @tparam TSource Source class that should be used to serialize the json that is sent to the server
    /// @param source Data source containing our json key value pairs we want to send
    /// @param jsonSize Size of the data inside the source
    /// @return Whether sending the data was successful or not
    template <typename TSource>
    inline bool sendTelemetryJson(const TSource& source, const size_t& jsonSize) {
      return Send_Json(HTTP_TELEMETRY_TOPIC, source, jsonSize);
    }

    /// @brief Attempts to send a GET request over HTTP or HTTPS
    /// @param path API path we want to get data from (example: /api/v1/$TOKEN/rpc)
    /// @param response String the GET response will be copied into,
    /// will not be changed if the GET request wasn't successful
    /// @return Whetherr sending the GET request was successful or not
#if THINGSBOARD_ENABLE_STL
    inline bool sendGetRequest(const char* path, std::string& response) {
#else
    inline bool sendGetRequest(const char* path, String& response) {
#endif // THINGSBOARD_ENABLE_STL
      return getMessage(path, response);
    }

    /// @brief Attempts to send a POST request over HTTP or HTTPS
    /// @param path API path we want to send data to (example: /api/v1/$TOKEN/attributes)
    /// @param json String containing our json key value pairs we want to attempt to send
    /// @return Whetherr sending the POST request was successful or not
    inline bool sendPostRequest(const char* path, const char* json) {
      return postMessage(path, json);
    }

    //----------------------------------------------------------------------------
    // Attribute API

    /// @brief Attempts to send attribute data with the given key and value of the given type.
    /// See https://thingsboard.io/docs/user-guide/attributes/ for more information
    /// @tparam T Type of the passed value
    /// @param key Key of the key value pair we want to send
    /// @param value Value of the key value pair we want to send
    /// @return Whether sending the data was successful or not
    template<typename T>
    inline bool sendAttributeData(const char *key, T value) {
      return sendKeyValue(key, value, false);
    }

    /// @brief Attempts to send aggregated attribute data.
    /// See https://thingsboard.io/docs/user-guide/attributes/ for more information
    /// @param data Array containing all the data we want to send
    /// @param data_count Amount of data entries in the array that we want to send
    /// @return Whetherr sending the data was successful or not
    inline bool sendAttributes(const Attribute *data, size_t data_count) {
      return sendDataArray(data, data_count, false);
    }

    /// @brief Attempts to send custom json attribute string.
    /// See https://thingsboard.io/docs/user-guide/attributes/ for more information
    /// @param json String containing our json key value pairs we want to attempt to send
    /// @return Whetherr sending the data was successful or not
    inline bool sendAttributeJSON(const char *json) {
      return Send_Json_String(HTTP_ATTRIBUTES_TOPIC, json);
    }

    /// @brief Attempts to send attribute key value pairs from custom source to the server.
    /// See https://thingsboard.io/docs/user-guide/attributes/ for more information
    /// @tparam TSource Source class that should be used to serialize the json that is sent to the server
    /// @param source Data source containing our json key value pairs we want to send
    /// @param jsonSize Size of the data inside the source
    /// @return Whether sending the data was successful or not
    template <typename TSource>
    inline bool sendAttributeJSON(const TSource& source, const size_t& jsonSize) {
      return Send_Json(HTTP_ATTRIBUTES_TOPIC, source, jsonSize);
    }

  private:
    IHTTP_Client& m_client;  // HttpClient instance
    const ILogger& m_logger; // Logging instance used to print messages
    size_t m_max_stack;      // Maximum stack size we allocate at once on the stack.
    const char *m_token;     // Access token used to connect with

    /// @brief Returns the maximum amount of bytes that we want to allocate on the stack, before the memory is allocated on the heap instead
    /// @return Maximum amount of bytes we want to allocate on the stack
    inline const size_t& getMaximumStackSize() const {
      return m_max_stack;
    }

    /// @brief Clears any remaining memory of the previous conenction,
    /// and resets the TCP as well, if data is resend the TCP connection has to be re-established
    inline void clearConnection() {
      m_client.stop();
    }

    /// @brief Attempts to send a POST request over HTTP or HTTPS
    /// @param path API path we want to send data to (example: /api/v1/$TOKEN/attributes)
    /// @param json String containing our json key value pairs we want to attempt to send
    /// @return Whetherr sending the POST request was successful or not
    inline bool postMessage(const char* path, const char* json) {
      bool result = true;

      const int success = m_client.post(path, HTTP_POST_PATH, json);
      const int status = m_client.get_response_status_code();

      if (!success || status < HTTP_RESPONSE_SUCCESS_RANGE_START || status > HTTP_RESPONSE_SUCCESS_RANGE_END) {
        m_logger.logf(HTTP_FAILED, POST, status);
        result = false;
      }

      clearConnection();
      return result;
    }

    /// @brief Attempts to send a GET request over HTTP or HTTPS
    /// @param path API path we want to get data from (example: /api/v1/$TOKEN/rpc)
    /// @param response String the GET response will be copied into,
    /// will not be changed if the GET request wasn't successful
    /// @return Whetherr sending the GET request was successful or not
#if THINGSBOARD_ENABLE_STL
    inline bool getMessage(const char* path, std::string& response) {
#else
    inline bool getMessage(const char* path, String& response) {
#endif // THINGSBOARD_ENABLE_STL
      bool result = true;

      const bool success = m_client.get(path);
      const int status = m_client.get_response_status_code();

      if (!success || status < HTTP_RESPONSE_SUCCESS_RANGE_START || status > HTTP_RESPONSE_SUCCESS_RANGE_END) {
        m_logger.logf(HTTP_FAILED, GET, status);
        result = false;
        goto cleanup;
      }

      response = m_client.get_response_body();

      cleanup:
      clearConnection();
      return result;
    }

    /// @brief Attempts to send aggregated attribute or telemetry data
    /// @param data Array containing all the data we want to send
    /// @param data_count Amount of data entries in the array that we want to send
    /// @param telemetry Whetherr the aggregated data is telemetry (true) or attribut (false)
    /// @return Whetherr sending the data was successful or not
    inline bool sendDataArray(const Telemetry *data, size_t data_count, bool telemetry = true) {
#if THINGSBOARD_ENABLE_DYNAMIC
      // String are const char* and therefore stored as a pointer --> zero copy, meaning the size for the strings is 0 bytes,
      // Data structure size depends on the amount of key value pairs passed.
      // See https://arduinojson.org/v6/assistant/ for more information on the needed size for the JsonDocument
      const size_t dataStructureMemoryUsage = JSON_OBJECT_SIZE(data_count);
      TBJsonDocument jsonBuffer(dataStructureMemoryUsage);
#else
      StaticJsonDocument<JSON_OBJECT_SIZE(MaxFieldsAmount)> jsonBuffer;
#endif // THINGSBOARD_ENABLE_DYNAMIC
      JsonVariant object = jsonBuffer.template to<JsonVariant>();

      for (size_t i = 0; i < data_count; ++i) {
        if (!data[i].SerializeKeyValue(object)) {
          m_logger.log(UNABLE_TO_SERIALIZE);
          return false;
        }
      }

#if THINGSBOARD_ENABLE_DYNAMIC
      // Resize internal JsonDocument buffer to only use the actually needed amount of memory.
      requestBuffer.shrinkToFit();
#endif // !THINGSBOARD_ENABLE_DYNAMIC

      return telemetry ? sendTelemetryJson(object, Helper::Measure_Json(object)) : sendAttributeJSON(object, Helper::Measure_Json(object));
    }

    /// @brief Sends single key-value attribute or telemetry data in a generic way
    /// @tparam T Type of the passed value
    /// @param key Key of the key value pair we want to send
    /// @param val Value of the key value pair we want to send
    /// @param telemetry Whetherr the aggregated data is telemetry (true) or attribut (false)
    /// @return Whetherr sending the data was successful or not
    template<typename T>
    inline bool sendKeyValue(const char *key, T value, bool telemetry = true) {
      const Telemetry t(key, value);
      if (t.IsEmpty()) {
        // Message is ignored and not sent at all.
        return false;
      }

      StaticJsonDocument<JSON_OBJECT_SIZE(1)> jsonBuffer;
      JsonVariant object = jsonBuffer.template to<JsonVariant>();
      if (!t.SerializeKeyValue(object)) {
        m_logger.log(UNABLE_TO_SERIALIZE);
        return false;
      }

      return telemetry ? sendTelemetryJson(object, Helper::Measure_Json(object)) : sendAttributeJSON(object, Helper::Measure_Json(object));
    }
};

using ThingsBoardHttp = ThingsBoardHttpSized<>;

#endif // ThingsBoard_Http_h
