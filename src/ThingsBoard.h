/*
  ThingsBoard.h - Library API for sending data to the ThingsBoard
  Based on PubSub MQTT library.
  Created by Olender M. Oct 2018.
  Released into the public domain.
*/
#ifndef ThingsBoard_h
#define ThingsBoard_h

// Library includes.
#include <PubSubClient.h>
#include <HashGenerator.h>
#include <vector>
#include <array>
#include <functional>

#if defined(ESP8266)
#include <Updater.h>
#elif defined(ESP32)
#include <Update.h>
#endif

// Local includes.
#include "ThingsBoardDefaultLogger.h"
#include "Telemetry.h"
#include "Constants.h"

/// ---------------------------------
/// Constant strings in flash memory.
/// ---------------------------------
// Publish data topics.
constexpr char ATTRIBUTE_TOPIC[] PROGMEM = "v1/devices/me/attributes";
constexpr char TELEMETRY_TOPIC[] PROGMEM = "v1/devices/me/telemetry";

// RPC topics.
constexpr char RPC_SUBSCRIBE_TOPIC[] PROGMEM = "v1/devices/me/rpc/request/+";
constexpr char RPC_TOPIC[] PROGMEM = "v1/devices/me/rpc";

// Firmware topics.
constexpr char FIRMWARE_RESPONSE_TOPIC[] PROGMEM = "v2/fw/response";

// Shared attribute topics.
constexpr char ATTRIBUTE_REQUEST_TOPIC[] PROGMEM = "v1/devices/me/attributes/request/%u";
constexpr char ATTRIBUTE_RESPONSE_SUBSCRIBE_TOPIC[] PROGMEM = "v1/devices/me/attributes/response/+";
constexpr char ATTRIBUTE_RESPONSE_TOPIC[] PROGMEM = "v1/devices/me/attributes/response";

// Provision topics.
constexpr char PROV_RESPONSE_TOPIC[] PROGMEM = "/provision/response";

// Default login data.
constexpr char PROV_ACCESS_TOKEN[] PROGMEM = "provision";
constexpr char DEFAULT_CLIENT_ID[] PROGMEM = "TbDev";

// Shared attribute request keys.
constexpr char SHARED_KEYS[] PROGMEM = "sharedKeys";
constexpr char SHARED_KEY[] PROGMEM = "shared";

// RPC data keys.
constexpr char RPC_METHOD_KEY[] PROGMEM = "method";
constexpr char RPC_PARAMS_KEY[] PROGMEM = "params";
constexpr char RPC_REQUEST_KEY[] PROGMEM = "request";
constexpr char RPC_RESPONSE_KEY[] PROGMEM = "response";

// Log messages.
constexpr char INVALID_BUFFER_SIZE[] PROGMEM = "PayloadSize (%u) to small for the given payloads size (%u)";
constexpr char MAX_RPC_EXCEEDED[] PROGMEM = "Too many rpc subscriptions, increase MaxFieldsAmt or unsubscribe";
constexpr char MAX_SHARED_ATT_UPDATE_EXCEEDED[] PROGMEM = "Too many shared attribute update callback subscriptions, increase MaxFieldsAmt or unsubscribe";
constexpr char MAX_SHARED_ATT_REQUEST_EXCEEDED[] PROGMEM = "Too many shared attribute request callback subscriptions, increase MaxFieldsAmt";
constexpr char NUMBER_PRINTF[] PROGMEM = "%u";
constexpr char COMMA PROGMEM = ',';
constexpr char NO_KEYS_TO_REQUEST[] PROGMEM = "No keys to request were given";
constexpr char REQUEST_ATT[] PROGMEM = "Requesting shared attributes transformed from (%s) into json (%s)";
constexpr char UNABLE_TO_DE_SERIALIZE_RPC[] PROGMEM = "Unable to de-serialize RPC";
constexpr char UNABLE_TO_DE_SERIALIZE_ATT_REQUEST[] PROGMEM = "Unable to de-serialize shared attribute request";
constexpr char UNABLE_TO_DE_SERIALIZE_ATT_UPDATE[] PROGMEM = "Unable to de-serialize shared attribute update";
constexpr char RECEIVED_RPC_LOG_MESSAGE[] PROGMEM = "Received RPC:";
constexpr char RPC_METHOD_NULL[] PROGMEM = "RPC method is NULL";
constexpr char RPC_CB_NULL[] PROGMEM = "RPC callback is NULL";
constexpr char NO_RPC_PARAMS_PASSED[] PROGMEM = "No parameters passed with RPC, passing null JSON";
constexpr char CALLING_RPC[] PROGMEM = "Calling RPC:";
constexpr char RECEIVED_ATT_UPDATE[] PROGMEM = "Received shared attribute update";
constexpr char NOT_FOUND_ATT_UPDATE[] PROGMEM = "Shared attribute update key not found";
constexpr char ATT_CB_ID[] PROGMEM = "Shared attribute update callback id: (%u)";
constexpr char ATT_CB_IS_NULL[] PROGMEM = "Shared attribute update callback is NULL";
constexpr char ATT_CB_NO_KEYS[] PROGMEM = "No keys subscribed. Calling subscribed callback for any updated attributes (assumed to be subscribed to every possible key)";
constexpr char ATT_IS_NULL[] PROGMEM = "Subscribed shared attribute update key is NULL";
constexpr char ATT_IN_ARRAY[] PROGMEM = "Shared attribute update key: (%s) is subscribed";
constexpr char ATT_NO_CHANGE[] PROGMEM = "No keys that we subscribed too were changed, skipping callback";
constexpr char CALLING_ATT_CB[] PROGMEM = "Calling subscribed callback for updated shared attribute (%s)";
constexpr char RECEIVED_ATT[] PROGMEM = "Received shared attribute request";
constexpr char ATT_KEY_NOT_FOUND[] PROGMEM = "Shared attribute key not found";
constexpr char ATT_REQUEST_CB_IS_NULL[] PROGMEM = "Shared attribute request callback is NULL";
constexpr char PROVISION_CB_IS_NULL[] PROGMEM = "Provisioning callback is NULL";
constexpr char CALLING_REQUEST_ATT_CB[] PROGMEM = "Calling subscribed callback for response id (%u)";
constexpr char CB_ON_MESSAGE[] PROGMEM = "Callback onMQTTMessage from topic: (%s)";

#if defined(ESP8266) || defined(ESP32) || defined(ARDUINO_AVR_MEGA)
// Claim topics.
constexpr char CLAIM_TOPIC[] PROGMEM = "v1/devices/me/claim";

// Provision topics.
constexpr char PROV_REQUEST_TOPIC[] PROGMEM = "/provision/request";

// Claim data keys.
constexpr char SECRET_KEY[] PROGMEM = "secretKey";
constexpr char DURATION_KEY[] PROGMEM = "durationMs";
constexpr char DEVICE_NAME_KEY[] PROGMEM = "deviceName";
constexpr char PROV_DEVICE_KEY[] PROGMEM = "provisionDeviceKey";
constexpr char PROV_DEVICE_SECRET_KEY[] PROGMEM = "provisionDeviceSecret";

// Provision data keys.
constexpr char PROV_STATUS_KEY[] PROGMEM = "status";
constexpr char PROV_CRED_TYPE_KEY[] PROGMEM = "credentialsType";
constexpr char STATUS_SUCCESS[] PROGMEM = "SUCCESS";
constexpr char PROV_CRED_TYPE_VALUE[] PROGMEM = "X509_CERTIFICATE";

// Log messages.
constexpr char PROV_REQUEST[] PROGMEM = "Provision request:";
constexpr char UNABLE_TO_DE_SERIALIZE_PROV_RESPONSE[] PROGMEM = "Unable to de-serialize provision response";
constexpr char PROV_RESPONSE[] PROGMEM = "Process provisioning response";
constexpr char RECEIVED_PROV_RESPONSE[] PROGMEM = "Received provision response";
constexpr char X509_NOT_SUPPORTED[] PROGMEM = "Provision response contains X509_CERTIFICATE credentials, this is not supported yet";

#if !defined(ARDUINO_AVR_MEGA)
// Firmware topics.
constexpr char FIRMWARE_RESPONSE_SUBSCRIBE_TOPIC[] PROGMEM = "v2/fw/response/#";
constexpr char FIRMWARE_REQUEST_TOPIC[] PROGMEM = "v2/fw/request/0/chunk/%u";

// Firmware data keys.
constexpr char CURR_FW_TITLE_KEY[] PROGMEM = "current_fw_title";
constexpr char CURR_FW_VER_KEY[] PROGMEM = "current_fw_version";
constexpr char CURR_FW_STATE_KEY[] PROGMEM = "current_fw_state";
constexpr char FW_VER_KEY[] PROGMEM = "fw_version";
constexpr char FW_TITLE_KEY[] PROGMEM = "fw_title";
constexpr char FW_CHKS_KEY[] PROGMEM = "fw_checksum";
constexpr char FW_CHKS_ALGO_KEY[] PROGMEM = "fw_checksum_algorithm";
constexpr char FW_SIZE_KEY[] PROGMEM = "fw_size";
constexpr char FW_STATE_CHECKING[] PROGMEM = "CHECKING FIRMWARE";
constexpr char FW_STATE_NO_FW[] PROGMEM = "NO FIRMWARE FOUND";
constexpr char FW_STATE_UP_TO_DATE[] PROGMEM = "UP TO DATE";
constexpr char FW_STATE_INVALID_CHKS[] PROGMEM = "CHECKSUM ALGORITHM INVALID";
constexpr char FW_STATE_DOWNLOADING[] PROGMEM = "DOWNLOADING";
constexpr char FW_STATE_FAILED[] PROGMEM = "FAILED";
constexpr char FW_STATE_UPDATE_ERROR[] PROGMEM = "UPDATE ERROR";
constexpr char FW_STATE_CHKS_ERROR[] PROGMEM = "CHECKSUM ERROR";
constexpr char CHECKSUM_AGORITM_MD5[] PROGMEM = "MD5";
constexpr char CHECKSUM_AGORITM_SHA256[] PROGMEM = "SHA256";
constexpr char CHECKSUM_AGORITM_SHA384[] PROGMEM = "SHA384";
constexpr char CHECKSUM_AGORITM_SHA512[] PROGMEM = "SHA512";

// Log messages.
constexpr char NO_FW[] PROGMEM = "No new firmware assigned on the given device";
constexpr char EMPTY_FW[] PROGMEM = "Given firmware was NULL";
constexpr char FW_UP_TO_DATE[] PROGMEM = "Firmware is already up to date";
constexpr char FW_NOT_FOR_US[] PROGMEM = "Firmware is not for us (title is different)";
constexpr char FW_CHKS_ALGO_NOT_SUPPORTED[] PROGMEM = "Checksum algorithm (%s) is not supported";
constexpr char PAGE_BREAK[] PROGMEM = "=================================";
constexpr char NEW_FW[] PROGMEM = "A new Firmware is available:";
constexpr char FROM_TOO[] PROGMEM = "(%s) => (%s)";
constexpr char DOWNLOADING_FW[] PROGMEM = "Attempting to download over MQTT...";
constexpr char NOT_ENOUGH_RAM[] PROGMEM = "Not enough RAM";
constexpr char SLASH PROGMEM = '/';
constexpr char UNABLE_TO_WRITE[] PROGMEM = "Unable to write firmware";
constexpr char UNABLE_TO_DOWNLOAD[] PROGMEM = "Unable to download firmware";
constexpr char FW_CHUNK[] PROGMEM = "Receive chunk (%i), with size (%u) bytes";
constexpr char ERROR_UPDATE_BEGIN[] PROGMEM = "Error during Update.begin";
constexpr char ERROR_UPDATE_WRITE[] PROGMEM = "Error during Update.write";
constexpr char ERROR_UPDATE_END[] PROGMEM = "Error during Update.end, not all bytes written";
constexpr char HASH_ACTUAL[] PROGMEM = "(%s) actual checksum: (%s)";
constexpr char HASH_EXPECTED[] PROGMEM = "(%s) expected checksum: (%s)";
constexpr char CHKS_VER_FAILED[] PROGMEM = "Checksum verification failed";
constexpr char CHKS_VER_SUCCESS[] PROGMEM = "Checksum is the same as expected";
constexpr char FW_UPDATE_SUCCESS[] PROGMEM = "Update success";
#endif
#endif

// Convenient aliases
// JSON variant const (read only twice as small as JSON variant), is used to communicate RPC parameters to the client
using RPC_Data = const JsonVariantConst;
// JSON object const (read only twice as small as JSON object), is used to communicate Shared Attributes and Provision Data to the client
using Shared_Attribute_Data = const JsonObjectConst;
#if defined(ESP8266) || defined(ESP32) || defined(ARDUINO_AVR_MEGA)
using Provision_Data = const JsonObjectConst;
#endif

// RPC callback wrapper
class RPC_Callback {
  public:
    // RPC callback signature
    using returnType = RPC_Response;
    using argumentType = RPC_Data&;
    using processFn = std::function<returnType(const argumentType data)>;

    // Constructs empty callback
    inline RPC_Callback()
      : m_name(), m_cb(nullptr) {  }

    // Constructs callback that will be fired upon a RPC request arrival with
    // given method name
    inline RPC_Callback(const char* methodName, const processFn cb)
      : m_name(methodName), m_cb(cb) {  }

    // Calls the callback that was subscribed when this class instance was initally created.
    template<typename Logger>
    inline returnType Call_Callback(const argumentType data) const {
      // Check if the callback is a nullptr,
      // meaning it has not been assigned any valid callback method.
      if (!m_cb) {
        Logger::log(RPC_CB_NULL);
        return returnType();
      }
      return m_cb(data);
    }

    // Gets the name we gave the callback method on the cloud.
    inline const char* Get_Name() const {
      return m_name;
    }

  private:
    const char  *m_name;    // Method name
    processFn   m_cb;       // Callback to call
};

// Shared attributes callback wrapper
class Shared_Attribute_Callback {
  public:
    // Shared attributes callback signature
    using returnType = void;
    using argumentType = Shared_Attribute_Data&;
    using processFn = std::function<returnType(const argumentType data)>;

    // Constructs empty callback
    inline Shared_Attribute_Callback()
      : m_att(), m_cb(nullptr) {  }

    // Constructs callback that will be fired upon a Shared attribute update arrival,
    // where one of the given keys was changed.
    template<class InputIterator>
    inline Shared_Attribute_Callback(const InputIterator& first_itr, const InputIterator& last_itr, const processFn cb)
      : m_att(first_itr, last_itr), m_cb(cb) {  }

    // Constructs callback that will be fired upon a Shared attribute update arrival,
    // no matter which key was changed.
    inline Shared_Attribute_Callback(const processFn cb)
      : m_att(), m_cb(cb) {  }

    // Calls the callback that was subscribed when this class instance was initally created.
    template<typename Logger>
    inline returnType Call_Callback(const argumentType data) const {
      // Check if the callback is a nullptr,
      // meaning it has not been assigned any valid callback method.
      if (!m_cb) {
        Logger::log(ATT_CB_IS_NULL);
        return returnType();
      }
      return m_cb(data);
    }

    // Gets all the subscribed attributes that will result,
    // in the subscribed method being called if changed by the cloud.
    inline const std::vector<const char*>& Get_Attributes() const {
      return m_att;
    }

  private:
    const std::vector<const char*>       m_att;   // Attribute we want to request
    processFn                            m_cb;    // Callback to call
};

// Shared attributes request callback wrapper
class Shared_Attribute_Request_Callback {
  public:
    // Shared attributes callback signature
    using returnType = void;
    using argumentType = Shared_Attribute_Data&;
    using processFn = std::function<returnType(const argumentType data)>;

    // Constructs empty callback
    inline Shared_Attribute_Request_Callback()
      : m_request_id(0U), m_cb(nullptr) {  }

    // Constructs callback that will be fired upon a Shared attribute request arrival
    inline Shared_Attribute_Request_Callback(const processFn cb)
      : m_request_id(0U), m_cb(cb) {  }

    // Calls the callback that was subscribed when this class instance was initally created.
    template<typename Logger>
    inline returnType Call_Callback(const argumentType data) const {
      // Check if the callback is a nullptr,
      // meaning it has not been assigned any valid callback method.
      if (!m_cb) {
        Logger::log(ATT_REQUEST_CB_IS_NULL);
        return returnType();
      }
      return m_cb(data);
    }

    // Gets all the subscribed attributes that will result,
    // in the subscribed method being called if changed by the cloud.
    inline const uint32_t& Get_Request_ID() const {
      return m_request_id;
    }

    // Sets the request id to the actual used request id to later identify,
    // which Shared_Attribute_Request_Callback is connected to which received data.
    inline void Set_Request_ID(const uint32_t request_id) {
      m_request_id = request_id;
    }

  private:
    uint32_t        m_request_id;   // Id the request was called with
    processFn       m_cb;           // Callback to call
};

#if defined(ESP8266) || defined(ESP32) || defined(ARDUINO_AVR_MEGA)
// Provisioning callback wrapper
class Provision_Callback {
  public:
    // Provisioning callback signature
    using returnType = void;
    using argumentType = Provision_Data&;
    using processFn = std::function<returnType(const argumentType data)>;

    // Constructs empty callback
    inline Provision_Callback()
      : m_cb(nullptr) {  }

    // Constructs callback that will be fired upon a Provision request arrival with
    // given attribute key
    inline Provision_Callback(const processFn cb)
      : m_cb(cb) {  }

    // Calls the callback that was subscribed when this class instance was initally created.
    template<typename Logger>
    inline returnType Call_Callback(const argumentType data) const {
      // Check if the callback is a nullptr,
      // meaning it has not been assigned any valid callback method.
      if (!m_cb) {
        Logger::log(PROVISION_CB_IS_NULL);
        return returnType();
      }
      return m_cb(data);
    }

  private:
    processFn   m_cb;       // Callback to call
};
#endif

// ThingsBoardSized client class
template<size_t PayloadSize = Default_Payload,
         size_t MaxFieldsAmt = Default_Fields_Amt,
         typename Logger = ThingsBoardDefaultLogger>
class ThingsBoardSized
{
  public:
    // Initializes ThingsBoardSized class with network client.
    // Certain private members can not be set in the constructor initalizor list,
    // because using 2 instances of the ThingsBoard class (for example. provision and connected client)
    // will result in the variables being reset between method calls. Resulting in unexpected behaviour.
    inline ThingsBoardSized(Client& client, const bool& enableQoS = false)
      : m_client()
      , m_requestId(0)
      , m_qos(enableQoS)
#if defined(ESP8266) || defined(ESP32)
      , m_fwResponseCallback(std::bind(&ThingsBoardSized::Firmware_Shared_Attribute_Received, this, std::placeholders::_1))
#endif
    {
      reserve_callback_size();
      setClient(client);
    }

    // Initializes ThingsBoardSized class without network client. Ensure it is set via. setClient() before using the connect method.
    inline ThingsBoardSized()
      : m_client()
      , m_requestId(0)
      , m_qos(false)
#if defined(ESP8266) || defined(ESP32)
      , m_fwResponseCallback(std::bind(&ThingsBoardSized::Firmware_Shared_Attribute_Received, this, std::placeholders::_1))
#endif
    {
      reserve_callback_size();
    }

    // Destroys ThingsBoardSized class with network client.
    inline ~ThingsBoardSized() {
      // Nothing to do.
    }

    // Returns a reference to the PubSubClient.
    inline PubSubClient& getClient(void) {
      return m_client;
    }

    // Enables mqtt quality of service for the given underlying MQTT connection.
    // See https://assetwolf.com/learn/mqtt-qos-understanding-quality-of-service for more information, on quality of service.
    inline void enableMQTTQoS(const bool& enableQoS) {
      m_qos = enableQoS;
    }

    // Sets the underlying Client of the PubSubClient.
    inline void setClient(Client& client) {
      m_client.setClient(client);
      m_client.setBufferSize(PayloadSize);
      // Initalize callback.
      m_client.setCallback(std::bind(&ThingsBoardSized::onMQTTMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    }

    // Connects to the specified ThingsBoard server and port.
    // Access token is used to authenticate a client.
    // Returns true on success, false otherwise.
    inline const bool connect(const char *host, const char *access_token = PROV_ACCESS_TOKEN, const uint16_t& port = 1883, const char *client_id = DEFAULT_CLIENT_ID, const char *password = NULL) {
      if (!host) {
        return false;
      }
      m_client.setServer(host, port);
      return connect_to_host(access_token, client_id, password);
    }

    inline const bool connect(const IPAddress& host, const char *access_token = PROV_ACCESS_TOKEN, const uint16_t& port = 1883, const char *client_id = DEFAULT_CLIENT_ID, const char *password = NULL) {
      m_client.setServer(host, port);
      return connect_to_host(access_token, client_id, password);
    }

    // Disconnects from ThingsBoard.
    inline void disconnect() {
      m_client.disconnect();
    }

    // Returns true if connected, false otherwise.
    inline const bool connected() {
      return m_client.connected();
    }

    // Executes an event loop for PubSub client.
    inline void loop() {
      m_client.loop();
    }

    //----------------------------------------------------------------------------
    // Claiming API

#if defined(ESP8266) || defined(ESP32) || defined(ARDUINO_AVR_MEGA)

    inline const bool sendClaimingRequest(const char *secretKey, uint32_t durationMs) {
      StaticJsonDocument<JSON_OBJECT_SIZE(1)> requestBuffer;
      JsonObject resp_obj = requestBuffer.to<JsonObject>();

      resp_obj[SECRET_KEY] = secretKey;
      resp_obj[DURATION_KEY] = durationMs;

      uint8_t objectSize = JSON_STRING_SIZE(measureJson(requestBuffer));
      char responsePayload[objectSize];
      serializeJson(resp_obj, responsePayload, objectSize);

      return m_client.publish(CLAIM_TOPIC, responsePayload, m_qos);
    }

    // Provisioning API
    inline const bool sendProvisionRequest(const char* deviceName, const char* provisionDeviceKey, const char* provisionDeviceSecret) {
      // TODO Add ability to provide specific credentials from client side.
      StaticJsonDocument<JSON_OBJECT_SIZE(3)> requestBuffer;
      JsonObject requestObject = requestBuffer.to<JsonObject>();

      requestObject[DEVICE_NAME_KEY] = deviceName;
      requestObject[PROV_DEVICE_KEY] = provisionDeviceKey;
      requestObject[PROV_DEVICE_SECRET_KEY] = provisionDeviceSecret;

      uint8_t objectSize = JSON_STRING_SIZE(measureJson(requestBuffer));
      char requestPayload[objectSize];
      serializeJson(requestObject, requestPayload, objectSize);

      Logger::log(PROV_REQUEST);
      Logger::log(requestPayload);
      return m_client.publish(PROV_REQUEST_TOPIC, requestPayload, m_qos);
    }

#endif
    //----------------------------------------------------------------------------
    // Telemetry API

    // Sends telemetry data to the ThingsBoard, returns true on success.
    template<class T>
    inline const bool sendTelemetryData(const char *key, T value)
    {
      return sendKeyValue(key, value);
    }

    // Sends integer telemetry data to the ThingsBoard, returns true on success.
    inline const bool sendTelemetryInt(const char *key, int value) {
      return sendKeyValue(key, value);
    }

    // Sends boolean telemetry data to the ThingsBoard, returns true on success.
    inline const bool sendTelemetryBool(const char *key, bool value) {
      return sendKeyValue(key, value);
    }

    // Sends float telemetry data to the ThingsBoard, returns true on success.
    inline const bool sendTelemetryFloat(const char *key, float value) {
      return sendKeyValue(key, value);
    }

    // Sends string telemetry data to the ThingsBoard, returns true on success.
    inline const bool sendTelemetryString(const char *key, const char *value) {
      return sendKeyValue(key, value);
    }

    // Sends aggregated telemetry to the ThingsBoard.
    inline const bool sendTelemetry(const Telemetry *data, size_t data_count) {
      return sendDataArray(data, data_count);
    }

    // Sends custom JSON telemetry string to the ThingsBoard.
    inline const bool sendTelemetryJson(const char *json) {
      if (json == nullptr) {
        return false;
      }

      const uint32_t json_size = JSON_STRING_SIZE(strlen(json));
      if (PayloadSize < json_size) {
        char message[detect_size(INVALID_BUFFER_SIZE, PayloadSize, json_size)];
        snprintf_P(message, sizeof(message), INVALID_BUFFER_SIZE, PayloadSize, json_size);
        Logger::log(message);
        return false;
      }
      return m_client.publish(TELEMETRY_TOPIC, json, m_qos);
    }

    // Sends custom JSON telemetry JsonObject to the ThingsBoard.
    inline const bool sendTelemetryJson(const JsonObject& jsonObject, const uint32_t& jsonSize) {
      const uint32_t jsonObjectSize = jsonObject.size();
      if (MaxFieldsAmt < jsonObjectSize) {
        char message[detect_size(TOO_MANY_JSON_FIELDS, jsonObjectSize, MaxFieldsAmt)];
        snprintf_P(message, sizeof(message), TOO_MANY_JSON_FIELDS, jsonObjectSize, MaxFieldsAmt);
        Logger::log(message);
        return false;
      }
      char json[jsonSize];
      serializeJson(jsonObject, json, jsonSize);
      return sendTelemetryJson(json);
    }

    // Sends custom JSON telemetry JsonVariant to the ThingsBoard.
    inline const bool sendTelemetryJson(const JsonVariant& jsonVariant, const uint32_t& jsonSize) {
      const uint32_t jsonVariantSize = jsonVariant.size();
      if (MaxFieldsAmt < jsonVariantSize) {
        char message[detect_size(TOO_MANY_JSON_FIELDS, jsonVariantSize, MaxFieldsAmt)];
        snprintf_P(message, sizeof(message), TOO_MANY_JSON_FIELDS, jsonVariantSize, MaxFieldsAmt);
        Logger::log(message);
        return false;
      }
      char json[jsonSize];
      serializeJson(jsonVariant, json, jsonSize);
      return sendTelemetryJson(json);
    }

    //----------------------------------------------------------------------------
    // Attribute API

    // Sends an attribute with given name and value.
    template<class T>
    inline const bool sendAttributeData(const char *attrName, T value) {
      return sendKeyValue(attrName, value, false);
    }

    // Sends integer attribute with given name and value.
    inline const bool sendAttributeInt(const char *attrName, int value) {
      return sendKeyValue(attrName, value, false);
    }

    // Sends boolean attribute with given name and value.
    inline const bool sendAttributeBool(const char *attrName, bool value) {
      return sendKeyValue(attrName, value, false);
    }

    // Sends float attribute with given name and value.
    inline const bool sendAttributeFloat(const char *attrName, float value) {
      return sendKeyValue(attrName, value, false);
    }

    // Sends string attribute with given name and value.
    inline const bool sendAttributeString(const char *attrName, const char *value) {
      return sendKeyValue(attrName, value, false);
    }

    // Sends aggregated attributes to the ThingsBoard.
    inline const bool sendAttributes(const Attribute *data, size_t data_count) {
      return sendDataArray(data, data_count, false);
    }

    // Sends custom JSON with attributes to the ThingsBoard.
    inline const bool sendAttributeJSON(const char *json) {
      if (json == nullptr) {
        return false;
      }

      const uint32_t json_size = JSON_STRING_SIZE(strlen(json));
      if (PayloadSize < json_size) {
        char message[detect_size(INVALID_BUFFER_SIZE, PayloadSize, json_size)];
        snprintf_P(message, sizeof(message), INVALID_BUFFER_SIZE, PayloadSize, json_size);
        Logger::log(message);
        return false;
      }
      return m_client.publish(ATTRIBUTE_TOPIC, json, m_qos);
    }

    // Sends custom JsonObject with attributes to the ThingsBoard.
    inline const bool sendAttributeJSON(const JsonObject& jsonObject, const uint32_t& jsonSize) {
      const uint32_t jsonObjectSize = jsonObject.size();
      if (MaxFieldsAmt < jsonObjectSize) {
        char message[detect_size(TOO_MANY_JSON_FIELDS, jsonObjectSize, MaxFieldsAmt)];
        snprintf_P(message, sizeof(message), TOO_MANY_JSON_FIELDS, jsonObjectSize, MaxFieldsAmt);
        Logger::log(message);
        return false;
      }
      char json[jsonSize];
      serializeJson(jsonObject, json, jsonSize);
      return sendAttributeJSON(json);
    }

    // Sends custom JsonVariant with attributes to the ThingsBoard.
    inline const bool sendAttributeJSON(const JsonVariant& jsonVariant, const uint32_t& jsonSize) {
      const uint32_t jsonVariantSize = jsonVariant.size();
      if (MaxFieldsAmt < jsonVariantSize) {
        char message[detect_size(TOO_MANY_JSON_FIELDS, jsonVariantSize, MaxFieldsAmt)];
        snprintf_P(message, sizeof(message), TOO_MANY_JSON_FIELDS, jsonVariantSize, MaxFieldsAmt);
        Logger::log(message);
        return false;
      }
      char json[jsonSize];
      serializeJson(jsonVariant, json, jsonSize);
      return sendAttributeJSON(json);
    }

    //----------------------------------------------------------------------------
    // Server-side RPC API

    // Subscribes multiple RPC callbacks.
    template<class InputIterator>
    inline const bool RPC_Subscribe(const InputIterator& first_itr, const InputIterator& last_itr) {
      const uint32_t size = std::distance(first_itr, last_itr);
      if (m_rpcCallbacks.size() + size > m_rpcCallbacks.capacity()) {
        Logger::log(MAX_RPC_EXCEEDED);
        return false;
      }
      if (!m_client.subscribe(RPC_SUBSCRIBE_TOPIC, m_qos ? 1 : 0)) {
        return false;
      }

      // Push back complete vector into our local m_rpcCallbacks vector.
      m_rpcCallbacks.insert(m_rpcCallbacks.end(), first_itr, last_itr);
      return true;
    }

    // Subscribe one RPC callback.
    inline const bool RPC_Subscribe(const RPC_Callback& callback) {
      if (m_rpcCallbacks.size() + 1 > m_rpcCallbacks.capacity()) {
        Logger::log(MAX_RPC_EXCEEDED);
        return false;
      }
      if (!m_client.subscribe(RPC_SUBSCRIBE_TOPIC, m_qos ? 1 : 0)) {
        return false;
      }

      // Push back given callback into our local m_rpcCallbacks vector.
      m_rpcCallbacks.push_back(callback);
      return true;
    }

    inline const bool RPC_Unsubscribe() {
      // Empty all callbacks.
      m_rpcCallbacks.clear();
      return m_client.unsubscribe(RPC_SUBSCRIBE_TOPIC);
    }

    //----------------------------------------------------------------------------
    // Firmware OTA API

#if defined(ESP8266) || defined(ESP32)
    inline const bool Start_Firmware_Update(const char* currFwTitle, const char* currFwVersion, const std::function<void(const bool&)>& updatedCallback, const uint8_t& chunkRetries = 5U, const uint16_t& chunkSize = 4096U) {
      m_fwState = nullptr;
      m_fwChecksum.clear();
      m_fwAlgorithm.clear();

      // Send current firmware version
      if (currFwTitle == nullptr || currFwVersion == nullptr) {
        return false;
      }
      else if (!Firmware_Send_FW_Info(currFwTitle, currFwVersion)) {
        return false;
      }

      // Update state
      if (!Firmware_Send_State(FW_STATE_CHECKING)) {
        return false;
      }

      // Set private members needed for update.
      m_currFwTitle = currFwTitle;
      m_currFwVersion = currFwVersion;
      m_fwUpdatedCallback = updatedCallback;
      m_fwChunkRetries = chunkRetries;
      m_fwChunckSize = chunkSize;

      // Request the firmware informations
      const std::array<const char*, 5U> fwSharedKeys {FW_CHKS_KEY, FW_CHKS_ALGO_KEY, FW_SIZE_KEY, FW_TITLE_KEY, FW_VER_KEY};
      return Shared_Attributes_Request(fwSharedKeys.cbegin(), fwSharedKeys.cend(), m_fwResponseCallback);
    }

    inline const bool Firmware_Send_FW_Info(const char* currFwTitle, const char* currFwVersion) {
      // Send our firmware title and version
      StaticJsonDocument<JSON_OBJECT_SIZE(2)> currentFirmwareInfo;
      JsonObject currentFirmwareInfoObject = currentFirmwareInfo.to<JsonObject>();

      currentFirmwareInfoObject[CURR_FW_TITLE_KEY] = currFwTitle;
      currentFirmwareInfoObject[CURR_FW_VER_KEY] = currFwVersion;
      return sendTelemetryJson(currentFirmwareInfoObject, JSON_STRING_SIZE(measureJson(currentFirmwareInfoObject)));
    }

    inline const bool Firmware_Send_State(const char* currFwState) {
      // Send our firmware title and version
      StaticJsonDocument<JSON_OBJECT_SIZE(1)> currentFirmwareState;
      JsonObject currentFirmwareStateObject = currentFirmwareState.to<JsonObject>();

      currentFirmwareStateObject[CURR_FW_STATE_KEY] = currFwState;
      return sendTelemetryJson(currentFirmwareStateObject, JSON_STRING_SIZE(measureJson(currentFirmwareStateObject)));
    }

    inline const bool Firmware_OTA_Subscribe() {
      if (!m_client.subscribe(FIRMWARE_RESPONSE_SUBSCRIBE_TOPIC, m_qos ? 1 : 0)) {
        return false;
      }

      return true;
    }

    inline const bool Firmware_OTA_Unsubscribe() {
      if (!m_client.unsubscribe(FIRMWARE_RESPONSE_SUBSCRIBE_TOPIC)) {
        return false;
      }

      return true;
    }

    inline void Firmware_Shared_Attribute_Received(const Shared_Attribute_Data& data) {
      // Print out firmware shared attributes.
      int jsonSize = JSON_STRING_SIZE(measureJson(data));
      char buffer[jsonSize];
      serializeJson(data, buffer, jsonSize);
      Logger::log(buffer);

      // Check if firmware is available for our device
      if (!data.containsKey(FW_VER_KEY) || !data.containsKey(FW_TITLE_KEY)) {
        Logger::log(NO_FW);
        Firmware_Send_State(FW_STATE_NO_FW);
        return;
      }

      const char* fw_title = data[FW_TITLE_KEY].as<const char*>();
      const char* fw_version = data[FW_VER_KEY].as<const char*>();
      m_fwChecksum = data[FW_CHKS_KEY].as<std::string>();
      m_fwAlgorithm = data[FW_CHKS_ALGO_KEY].as<std::string>();
      m_fwSize = data[FW_SIZE_KEY].as<const uint32_t>();

      if (fw_title == nullptr || fw_version == nullptr || m_currFwTitle == nullptr || m_currFwVersion == nullptr || m_fwAlgorithm.empty() || m_fwChecksum.empty()) {
        Logger::log(EMPTY_FW);
        Firmware_Send_State(FW_STATE_NO_FW);
        return;
      }
      // If firmware is the same, we do not update it
      else if (strncmp_P(m_currFwTitle, fw_title, JSON_STRING_SIZE(strlen(m_currFwTitle))) == 0 && strncmp_P(m_currFwVersion, fw_version, JSON_STRING_SIZE(strlen(m_currFwVersion))) == 0) {
        Logger::log(FW_UP_TO_DATE);
        Firmware_Send_State(FW_STATE_UP_TO_DATE);
        return;
      }
      // If firmware title is not the same, we quit now
      else if (strncmp_P(m_currFwTitle, fw_title, JSON_STRING_SIZE(strlen(m_currFwTitle))) != 0) {
        Logger::log(FW_NOT_FOR_US);
        Firmware_Send_State(FW_STATE_NO_FW);
        return;
      }

      // Set used firmware algorithm, depending on received message
      if (m_fwAlgorithm.compare(CHECKSUM_AGORITM_MD5) == 0) {
        m_fwChecksumAlgorithm = mbedtls_md_type_t::MBEDTLS_MD_MD5;
      }
      else if (m_fwAlgorithm.compare(CHECKSUM_AGORITM_SHA256) == 0) {
        m_fwChecksumAlgorithm = mbedtls_md_type_t::MBEDTLS_MD_SHA256;
      }
      else if (m_fwAlgorithm.compare(CHECKSUM_AGORITM_SHA384) == 0) {
        m_fwChecksumAlgorithm = mbedtls_md_type_t::MBEDTLS_MD_SHA384;
      }
      else if (m_fwAlgorithm.compare(CHECKSUM_AGORITM_SHA512) == 0) {
        m_fwChecksumAlgorithm = mbedtls_md_type_t::MBEDTLS_MD_SHA512;
      }
      else {
        char message[JSON_STRING_SIZE(strlen(FW_CHKS_ALGO_NOT_SUPPORTED)) + JSON_STRING_SIZE(m_fwAlgorithm.size())];
        snprintf_P(message, sizeof(message), FW_CHKS_ALGO_NOT_SUPPORTED, m_fwAlgorithm.c_str());
        Logger::log(message);
        Firmware_Send_State(FW_STATE_INVALID_CHKS);
        return;
      }

      // Subscribe to the needed OTA fw topics.
      Firmware_OTA_Subscribe();

      Logger::log(PAGE_BREAK);
      Logger::log(NEW_FW);
      char firmware[JSON_STRING_SIZE(strlen(FROM_TOO)) + JSON_STRING_SIZE(strlen(m_currFwVersion)) + JSON_STRING_SIZE(strlen(fw_version))];
      snprintf_P(firmware, sizeof(firmware), FROM_TOO, m_currFwVersion, fw_version);
      Logger::log(firmware);
      Logger::log(DOWNLOADING_FW);

      const uint32_t numberOfChunk = (m_fwSize / m_fwChunckSize) + 1U;
      uint32_t currChunk = 0U;
      uint8_t nbRetry = m_fwChunkRetries;

      // Get the previous buffer size and cache it so the previous settings can be restored.
      const uint16_t previousBufferSize = m_client.getBufferSize();
      const bool changeBufferSize = previousBufferSize < (m_fwChunckSize + 50U);

      // Increase size of receive buffer
      if (changeBufferSize && !m_client.setBufferSize(m_fwChunckSize + 50U)) {
        Logger::log(NOT_ENOUGH_RAM);
        return;
      }

      // Update state
      m_fwState = FW_STATE_DOWNLOADING;
      Firmware_Send_State(m_fwState);

      char size[detect_size(NUMBER_PRINTF, m_fwChunckSize)];
      // Download the firmware
      do {
        // Size adjuts dynamically to the current length of the currChunk number to ensure we don't cut it out of the topic string.
        char topic[detect_size(FIRMWARE_REQUEST_TOPIC, currChunk)];
        snprintf_P(topic, sizeof(topic), FIRMWARE_REQUEST_TOPIC, currChunk);
        snprintf_P(size, sizeof(size), NUMBER_PRINTF, m_fwChunckSize);
        m_client.publish(topic, size, m_qos);

        // Amount of time we wait until we declare the download as failed in milliseconds.
        const uint64_t timeout = millis() + 3000U;
        do {
          delay(5);
          loop();
        } while ((m_fwChunkReceive != currChunk) && (timeout >= millis()));

        if (m_fwChunkReceive == currChunk) {
          // Check if the current chunk is not the last one.
          if (numberOfChunk != (currChunk + 1)) {
            // Check if state is still DOWNLOADING and did not fail.
            if (strncmp_P(FW_STATE_DOWNLOADING, m_fwState, strlen(FW_STATE_DOWNLOADING)) == 0) {
              currChunk++;
              // Current chunck was downloaded successfully, resetting retries accordingly.
              nbRetry = m_fwChunkRetries;
            }
            else {
              nbRetry--;
              // Reset any possible errors that might have occured and retry them.
              // Until we run out of retries.
              m_fwState = FW_STATE_DOWNLOADING;
              if (nbRetry == 0) {
                Logger::log(UNABLE_TO_WRITE);
                break;
              }
            }
          }
          // The last chunk
          else {
            currChunk++;
          }
        }
        // Timeout
        else {
          nbRetry--;
          if (nbRetry == 0) {
            Logger::log(UNABLE_TO_DOWNLOAD);
            break;
          }
        }
      } while (numberOfChunk != currChunk);

      // Buffer size has been set to another value by the method return to the previous value.
      if (changeBufferSize) {
        m_client.setBufferSize(previousBufferSize);
      }

      // Unsubscribe from now not needed topics anymore.
      Firmware_OTA_Unsubscribe();

      bool success = false;
      // Update current_fw_title and current_fw_version if updating was a success.
      if (strncmp_P(STATUS_SUCCESS, m_fwState, strlen(STATUS_SUCCESS)) == 0) {
        Firmware_Send_FW_Info(fw_title, fw_version);
        Firmware_Send_State(STATUS_SUCCESS);
        success = true;
      }
      else {
        Firmware_Send_State(FW_STATE_FAILED);
      }

      if (m_fwUpdatedCallback != nullptr) {
        m_fwUpdatedCallback(success);
      }
    }
#endif

    //----------------------------------------------------------------------------
    // Shared attributes API

    template<class InputIterator>
    inline const bool Shared_Attributes_Request(const InputIterator& first_itr, const InputIterator& last_itr, Shared_Attribute_Request_Callback& callback) {
      StaticJsonDocument<JSON_OBJECT_SIZE(1)> requestBuffer;
      JsonObject requestObject = requestBuffer.to<JsonObject>();

      std::string sharedKeys;
      for (auto itr = first_itr; itr != last_itr; ++itr) {
        // Check if the given attribute is null, if it is skip it.
        if (*itr == nullptr) {
          continue;
        }
        sharedKeys.append(*itr);
        sharedKeys.push_back(COMMA);
      }

      // Check if any sharedKeys were requested.
      if (sharedKeys.empty()) {
        Logger::log(NO_KEYS_TO_REQUEST);
        return false;
      }

      // Remove latest not needed ,
      sharedKeys.pop_back();

      requestObject[SHARED_KEYS] = sharedKeys.c_str();
      int objectSize = measureJson(requestBuffer) + 1;
      char buffer[objectSize];
      serializeJson(requestObject, buffer, objectSize);

      // Print requested keys.
      char message[JSON_STRING_SIZE(strlen(REQUEST_ATT)) + sharedKeys.length() + JSON_STRING_SIZE(strlen(buffer))];
      snprintf_P(message, sizeof(message), REQUEST_ATT, sharedKeys.c_str(), buffer);
      Logger::log(message);

      m_requestId++;
      callback.Set_Request_ID(m_requestId);
      Shared_Attributes_Request_Subscribe(callback);

      char topic[detect_size(ATTRIBUTE_REQUEST_TOPIC, m_requestId)];
      snprintf_P(topic, sizeof(topic), ATTRIBUTE_REQUEST_TOPIC, m_requestId);
      return m_client.publish(topic, buffer, m_qos);
    }

    // Subscribes multiple Shared attributes callbacks.
    template<class InputIterator>
    inline const bool Shared_Attributes_Subscribe(const InputIterator& first_itr, const InputIterator& last_itr) {
      const uint32_t size = std::distance(first_itr, last_itr);
      if (m_sharedAttributeUpdateCallbacks.size() + size > m_sharedAttributeUpdateCallbacks.capacity()) {
        Logger::log(MAX_SHARED_ATT_UPDATE_EXCEEDED);
        return false;
      }
      if (!m_client.subscribe(ATTRIBUTE_TOPIC, m_qos ? 1 : 0)) {
        return false;
      }

      // Push back complete vector into our local m_sharedAttributeUpdateCallbacks vector.
      m_sharedAttributeUpdateCallbacks.insert(m_sharedAttributeUpdateCallbacks.end(), first_itr, last_itr);
      return true;
    }

    // Subscribe one Shared attributes callback.
    inline const bool Shared_Attributes_Subscribe(const Shared_Attribute_Callback& callback) {
      if (m_sharedAttributeUpdateCallbacks.size() + 1U > m_sharedAttributeUpdateCallbacks.capacity()) {
        Logger::log(MAX_SHARED_ATT_UPDATE_EXCEEDED);
        return false;
      }
      if (!m_client.subscribe(ATTRIBUTE_TOPIC, m_qos ? 1 : 0)) {
        return false;
      }

      // Push back given callback into our local m_sharedAttributeUpdateCallbacks vector.
      m_sharedAttributeUpdateCallbacks.push_back(callback);
      return true;
    }

    inline const bool Shared_Attributes_Unsubscribe() {
      // Empty all callbacks.
      m_sharedAttributeUpdateCallbacks.clear();
      if (!m_client.unsubscribe(ATTRIBUTE_TOPIC)) {
        return false;
      }
      return true;
    }

    // -------------------------------------------------------------------------------
    // Provisioning API

    // Subscribes to get provision response

#if defined(ESP8266) || defined(ESP32) || defined(ARDUINO_AVR_MEGA)
    inline const bool Provision_Subscribe(const Provision_Callback callback) {
      if (!m_client.subscribe(PROV_RESPONSE_TOPIC, m_qos ? 1 : 0)) {
        return false;
      }
      m_provisionCallback = callback;
      return true;
    }

    inline const bool Provision_Unsubscribe() {
      if (!m_client.unsubscribe(PROV_RESPONSE_TOPIC)) {
        return false;
      }
      return true;
    }
#endif

  private:

    // Connects to the client previously set with m_client.setServer().
    inline const bool connect_to_host(const char *access_token, const char *client_id, const char *password) {
      const bool connection_result = m_client.connect(client_id, access_token, password);
      if (connection_result) {
        this->RPC_Unsubscribe(); // Cleanup all RPC subscriptions
        this->Shared_Attributes_Unsubscribe(); // Cleanup all shared attributes subscriptions
        this->Shared_Attributes_Request_Unsubscribe(); // Cleanup all shared attributes requests
#if defined(ESP8266) || defined(ESP32) || defined(ARDUINO_AVR_MEGA)
        this->Provision_Unsubscribe();
#endif
#if defined(ESP8266) || defined(ESP32)
        this->Firmware_OTA_Unsubscribe();
#endif
      }
      else {
        Logger::log(CONNECT_FAILED);
      }
      return connection_result;
    }

    // Returns the length in chars needed for a given value with the given argument string to be displayed completly.
    inline const uint8_t detect_size(const char* msg, ...) const {
      va_list args;
      va_start(args, msg);
      // Result is what would have been written if the passed buffer would have been large enough not counting null character,
      // or if an error occured while creating the string a negative number is returned instead. TO ensure this will not crash the system
      // when creating an array with negative size we assert beforehand with a clear error message.
      const int32_t result = JSON_STRING_SIZE(vsnprintf_P(nullptr, 0U, msg, args));
      assert(result >= 0);
      va_end(args);
      return result;
    }

    // Reserves size of callback vectors to improve performance
    inline void reserve_callback_size(void) {
      // Reserve size of both m_sharedAttributeUpdateCallbacks, rpcCallbacks and m_sharedAttributeRequestCallbacks beforehand for performance reasons.
      m_rpcCallbacks.reserve(MaxFieldsAmt);
      m_sharedAttributeUpdateCallbacks.reserve(MaxFieldsAmt);
      m_sharedAttributeRequestCallbacks.reserve(MaxFieldsAmt);
    }

    // Subscribe one Shared attributes request callback.
    inline const bool Shared_Attributes_Request_Subscribe(const Shared_Attribute_Request_Callback& callback) {
      if (m_sharedAttributeRequestCallbacks.size() + 1 > m_sharedAttributeRequestCallbacks.capacity()) {
        Logger::log(MAX_SHARED_ATT_REQUEST_EXCEEDED);
        return false;
      }
      if (!m_client.subscribe(ATTRIBUTE_RESPONSE_SUBSCRIBE_TOPIC, m_qos ? 1 : 0)) {
        return false;
      }

      // Push back given callback into our local m_sharedAttributeRequestCallbacks vector.
      m_sharedAttributeRequestCallbacks.push_back(callback);
      return true;
    }

    inline const bool Shared_Attributes_Request_Unsubscribe() {
      // Empty all callbacks.
      m_sharedAttributeRequestCallbacks.clear();
      if (!m_client.unsubscribe(ATTRIBUTE_RESPONSE_SUBSCRIBE_TOPIC)) {
        return false;
      }
      return true;
    }

    // Sends single key-value in a generic way.
    template<typename T>
    inline const bool sendKeyValue(const char *key, T value, bool telemetry = true) {
      Telemetry t(key, value);
      if (t.IsEmpty()) {
        // Message is ignored and not sent at all.
        return false;
      }

      StaticJsonDocument<JSON_OBJECT_SIZE(1)>jsonBuffer;
      JsonVariant object = jsonBuffer.template to<JsonVariant>();
      if (!t.SerializeKeyValue(object)) {
        Logger::log(UNABLE_TO_SERIALIZE);
        return false;
      }

      return telemetry ? sendTelemetryJson(object, JSON_STRING_SIZE(measureJson(object))) : sendAttributeJSON(object, JSON_STRING_SIZE(measureJson(object)));
    }

    // Processes RPC message
    inline void process_rpc_message(char* topic, uint8_t* payload, uint32_t length) {
      RPC_Response r;
      {
        StaticJsonDocument<JSON_OBJECT_SIZE(MaxFieldsAmt)> jsonBuffer;
        DeserializationError error = deserializeJson(jsonBuffer, payload, length);
        if (error) {
          Logger::log(UNABLE_TO_DE_SERIALIZE_RPC);
          return;
        }
        const JsonObject &data = jsonBuffer.template as<JsonObject>();
        const char *methodName = data[RPC_METHOD_KEY].as<const char*>();
        const char *params = data[RPC_PARAMS_KEY].as<const char*>();

        if (methodName) {
          Logger::log(RECEIVED_RPC_LOG_MESSAGE);
          Logger::log(methodName);
        } else {
          Logger::log(RPC_METHOD_NULL);
          return;
        }

        for (const RPC_Callback& callback : m_rpcCallbacks) {
          const char *subscribedMethodName = callback.Get_Name();
          if (subscribedMethodName == nullptr) {
            Logger::log(RPC_METHOD_NULL);
            continue;
          }
          // Strncmp returns the ascis value difference of the ascii characters that are different,
          // meaning 0 is the same string and less and more than 0 is the difference in ascci values between the 2 chararacters.
          else if (strncmp(subscribedMethodName, methodName, strlen(subscribedMethodName)) != 0) {
            continue;
          }

          Logger::log(CALLING_RPC);
          Logger::log(methodName);
          // Do not inform client, if parameter field is missing for some reason
          if (!data.containsKey(RPC_PARAMS_KEY)) {
            Logger::log(NO_RPC_PARAMS_PASSED);
          }

          // try to de-serialize params
          StaticJsonDocument<JSON_OBJECT_SIZE(MaxFieldsAmt)> doc;
          DeserializationError err_param = deserializeJson(doc, params);
          //if failed to de-serialize params then send JsonObject instead
          Logger::log(RPC_PARAMS_KEY);
          if (err_param) {
            const JsonVariant &param = data[RPC_PARAMS_KEY].as<JsonVariant>();
            const uint32_t json_size = JSON_STRING_SIZE(measureJson(param));
            char json[json_size];
            serializeJson(param, json, json_size);
          	Logger::log(json);
            r = callback.Call_Callback<Logger>(param);
          } else {
            Logger::log(params);
            const JsonObject &param = doc.template as<JsonObject>();
            // Getting non-existing field from JSON should automatically
            // set JSONVariant to null
            r = callback.Call_Callback<Logger>(param);
          }
          break;
        }
      }
      // Fill in response
      char responsePayload[PayloadSize] = {0};
      StaticJsonDocument<JSON_OBJECT_SIZE(1)> respBuffer;
      JsonVariant resp_obj = respBuffer.template to<JsonVariant>();

      if (r.IsEmpty()) {
        // Message is ignored and not sent at all.
        return;
      }
      else if (!r.SerializeKeyValue(resp_obj)) {
        Logger::log(UNABLE_TO_SERIALIZE);
        return;
      }

      const uint32_t json_size = JSON_STRING_SIZE(measureJson(respBuffer));
      if (PayloadSize < json_size) {
        char message[detect_size(INVALID_BUFFER_SIZE, PayloadSize, json_size)];
        snprintf_P(message, sizeof(message), INVALID_BUFFER_SIZE, PayloadSize, json_size);
        Logger::log(message);
        return;
      }
      serializeJson(resp_obj, responsePayload, sizeof(responsePayload));

      String responseTopic = topic;
      responseTopic.replace(RPC_REQUEST_KEY, RPC_RESPONSE_KEY);
      Logger::log(RPC_RESPONSE_KEY);
      Logger::log(responseTopic.c_str());
      Logger::log(responsePayload);
      m_client.publish(responseTopic.c_str(), responsePayload, m_qos);
    }

#if defined(ESP8266) || defined(ESP32)

    // Processes firmware response
    inline void process_firmware_response(char* topic, uint8_t* payload, uint32_t length) {
      static uint32_t sizeReceive = 0;
      static HashGenerator hash(m_fwChecksumAlgorithm);

      m_fwChunkReceive = atoi(strrchr(topic, SLASH) + 1U);

      char message[detect_size(FW_CHUNK, m_fwChunkReceive, length)];
      snprintf_P(message, sizeof(message), FW_CHUNK, m_fwChunkReceive, length);
      Logger::log(message);

      if (m_fwChunkReceive == 0) {
        sizeReceive = 0;

        // Initialize Flash
        if (!Update.begin(m_fwSize)) {
          Logger::log(ERROR_UPDATE_BEGIN);
          m_fwState = FW_STATE_UPDATE_ERROR;
          Firmware_Send_State(m_fwState);
          // Ensure to call Update.abort after calling Update.begin even if it failed,
          // to make sure that any possibly started processes are stopped and freed.
#if defined(ESP32)
          Update.abort();
#endif
          return;
        }
      }

      // Write data to Flash
      if (Update.write(payload, length) != length) {
        Logger::log(ERROR_UPDATE_WRITE);
#if defined(ESP32)
          Update.abort();
#endif
        m_fwState = FW_STATE_UPDATE_ERROR;
        Firmware_Send_State(m_fwState);
        return;
      }

      // Update value only if writing to flash was a success
      hash.update(payload, length);
      sizeReceive += length;

      // Check if we received the full firmware already
      if (m_fwSize != sizeReceive) {
        return;
      }

      std::string calculatedHash = hash.get_hash_string();
      char actual[JSON_STRING_SIZE(strlen(HASH_ACTUAL)) + JSON_STRING_SIZE(m_fwAlgorithm.size()) + calculatedHash.size()];
      snprintf_P(actual, sizeof(actual), HASH_ACTUAL, m_fwAlgorithm.c_str(), calculatedHash.c_str());
      Logger::log(actual);
      char expected[JSON_STRING_SIZE(strlen(HASH_EXPECTED)) + JSON_STRING_SIZE(m_fwAlgorithm.size()) + JSON_STRING_SIZE(m_fwChecksum.size())];
      snprintf_P(expected, sizeof(expected), HASH_EXPECTED, m_fwAlgorithm.c_str(), m_fwChecksum.c_str());
      Logger::log(expected);
      // Check MD5
      if (m_fwChecksum.compare(calculatedHash) != 0) {
        Logger::log(CHKS_VER_FAILED);
#if defined(ESP32)
        Update.abort();
#endif
        m_fwState = FW_STATE_CHKS_ERROR;
        Firmware_Send_State(m_fwState);
      }
      else {
        Logger::log(CHKS_VER_SUCCESS);
        if (!Update.end()) {
          Logger::log(ERROR_UPDATE_END);
          m_fwState = FW_STATE_UPDATE_ERROR;
          Firmware_Send_State(m_fwState);
          return;
        }
        Logger::log(FW_UPDATE_SUCCESS);
        m_fwState = STATUS_SUCCESS;
      }
    }
#endif

    // Processes shared attribute update message
    inline void process_shared_attribute_update_message(char* topic, uint8_t* payload, uint32_t length) {
      StaticJsonDocument<JSON_OBJECT_SIZE(MaxFieldsAmt)> jsonBuffer;
      DeserializationError error = deserializeJson(jsonBuffer, payload, length);
      if (error) {
        Logger::log(UNABLE_TO_DE_SERIALIZE_ATT_UPDATE);
        return;
      }
      JsonObject data = jsonBuffer.template as<JsonObject>();

      if (data && (data.size() >= 1)) {
        Logger::log(RECEIVED_ATT_UPDATE);
        if (data.containsKey(SHARED_KEY)) {
          data = data[SHARED_KEY];
        }
      }
      else {
        Logger::log(NOT_FOUND_ATT_UPDATE);
        return;
      }

      for (size_t i = 0; i < m_sharedAttributeUpdateCallbacks.size(); i++) {
        char id_message[detect_size(ATT_CB_ID, i)];
        snprintf_P(id_message, sizeof(id_message), ATT_CB_ID, i);
        Logger::log(id_message);

        if (m_sharedAttributeUpdateCallbacks.at(i).Get_Attributes().empty()) {
          Logger::log(ATT_CB_NO_KEYS);
          // No specifc keys were subscribed so we call the callback anyway.
          m_sharedAttributeUpdateCallbacks.at(i).Call_Callback<Logger>(data);
          continue;
        }

        bool containsKey = false;
        const char* requested_att;
        for (const char* att : m_sharedAttributeUpdateCallbacks.at(i).Get_Attributes()) {
          if (att == nullptr) {
            Logger::log(ATT_IS_NULL);
            continue;
          }
          // Check if the request contained any of our requested keys.
          containsKey = containsKey || data.containsKey(att);
          // Break early if the key was requested from this callback.
          if (containsKey) {
            char contained_message[JSON_STRING_SIZE(strlen(ATT_IN_ARRAY)) + JSON_STRING_SIZE(strlen(att))];
            snprintf_P(contained_message, sizeof(contained_message), ATT_IN_ARRAY, att);
            Logger::log(contained_message);
            requested_att = att;
            break;
          }
        }

        // This callback did not request any keys that were in this response,
        // therefore we continue with the next element in the loop.
        if (!containsKey || requested_att == nullptr) {
          Logger::log(ATT_NO_CHANGE);
          continue;
        }

        char calling_message[JSON_STRING_SIZE(strlen(CALLING_ATT_CB)) + JSON_STRING_SIZE(strlen(requested_att))];
        snprintf_P(calling_message, sizeof(calling_message), CALLING_ATT_CB, requested_att);
        Logger::log(calling_message);
        // Getting non-existing field from JSON should automatically
        // set JSONVariant to null
        m_sharedAttributeUpdateCallbacks.at(i).Call_Callback<Logger>(data);
      }
    }

    // Processes shared attribute request message
    inline void process_shared_attribute_request_message(char* topic, uint8_t* payload, uint32_t length) {
      StaticJsonDocument<JSON_OBJECT_SIZE(MaxFieldsAmt)> jsonBuffer;
      DeserializationError error = deserializeJson(jsonBuffer, payload, length);
      if (error) {
        Logger::log(UNABLE_TO_DE_SERIALIZE_ATT_REQUEST);
        return;
      }
      JsonObject data = jsonBuffer.template as<JsonObject>();

      if (data && (data.size() >= 1)) {
        Logger::log(RECEIVED_ATT);
        if (data.containsKey(SHARED_KEY)) {
          data = data[SHARED_KEY];
        }
      } else {
        Logger::log(ATT_KEY_NOT_FOUND);
        return;
      }

      std::string response = topic;
      // Remove the not needed part of the topic.
      size_t index = strlen(ATTRIBUTE_RESPONSE_TOPIC) + 1U;
      response = response.substr(index, response.length() - index);
      // convert the remaining text to an integer
      uint32_t response_id = atoi(response.c_str());

      for (size_t i = 0; i < m_sharedAttributeRequestCallbacks.size(); i++) {
        if (m_sharedAttributeRequestCallbacks.at(i).Get_Request_ID() != response_id) {
          continue;
        }

        char message[detect_size(CALLING_REQUEST_ATT_CB, response_id)];
        snprintf_P(message, sizeof(message), CALLING_REQUEST_ATT_CB, response_id);
        Logger::log(message);
        // Getting non-existing field from JSON should automatically
        // set JSONVariant to null
        m_sharedAttributeRequestCallbacks.at(i).Call_Callback<Logger>(data);
        // Delete callback because the changes have been requested and the callback is no longer needed.
        m_sharedAttributeRequestCallbacks.erase(std::next(m_sharedAttributeRequestCallbacks.begin(), i));
      }
    }

    // Processes provisioning response
#if defined(ESP8266) || defined(ESP32) || defined(ARDUINO_AVR_MEGA)
    inline void process_provisioning_response(char* topic, uint8_t* payload, uint32_t length) {
      Logger::log(PROV_RESPONSE);

      StaticJsonDocument<JSON_OBJECT_SIZE(MaxFieldsAmt)> jsonBuffer;
      DeserializationError error = deserializeJson(jsonBuffer, payload, length);
      if (error) {
        Logger::log(UNABLE_TO_DE_SERIALIZE_PROV_RESPONSE);
        return;
      }

      const JsonObject &data = jsonBuffer.template as<JsonObject>();

      Logger::log(RECEIVED_PROV_RESPONSE);

      const char* provision_status = data[PROV_STATUS_KEY].as<const char*>();
      const char* provision_credentials = data[PROV_CRED_TYPE_KEY].as<const char*>();

      if (strncmp_P(STATUS_SUCCESS, provision_status, strlen(STATUS_SUCCESS)) == 0 && strncmp_P(PROV_CRED_TYPE_VALUE, provision_credentials, strlen(PROV_CRED_TYPE_VALUE)) == 0) {
        Logger::log(X509_NOT_SUPPORTED);
        return;
      }

      m_provisionCallback.Call_Callback<Logger>(data);
    }
#endif

    // Sends array of attributes or telemetry to ThingsBoard
    inline const bool sendDataArray(const Telemetry *data, size_t data_count, bool telemetry = true) {
      StaticJsonDocument<JSON_OBJECT_SIZE(MaxFieldsAmt)> jsonBuffer;
      JsonVariant object = jsonBuffer.template to<JsonVariant>();

      for (size_t i = 0; i < data_count; ++i) {
        if (!data[i].SerializeKeyValue(object)) {
          Logger::log(UNABLE_TO_SERIALIZE);
          return false;
        }
      }

      return telemetry ? sendTelemetryJson(object, JSON_STRING_SIZE(measureJson(object))) : sendAttributeJSON(object, JSON_STRING_SIZE(measureJson(object)));
    }

    PubSubClient m_client; // PubSub MQTT client instance.
    std::vector<RPC_Callback> m_rpcCallbacks; // RPC callbacks array
    std::vector<Shared_Attribute_Callback> m_sharedAttributeUpdateCallbacks; // Shared attribute update callbacks array
    std::vector<Shared_Attribute_Request_Callback> m_sharedAttributeRequestCallbacks; // Shared attribute request callbacks array

#if defined(ESP8266) || defined(ESP32) || defined(ARDUINO_AVR_MEGA)
    Provision_Callback m_provisionCallback; // Provision response callback
#endif
    uint32_t m_requestId; // Allows nearly 4.3 million requests before wrapping back to 0.
    bool m_qos; // Wheter QoS level 1 should be enabled or disabled (Resends the packet until the message was received and a PUBACK packet was returned).

#if defined(ESP8266) || defined(ESP32)
    // For Firmware Update
    const char *m_currFwTitle;
    const char *m_currFwVersion;
    const char *m_fwState;
    // Allows for a binary size of up to theoretically 4 GB.
    uint32_t m_fwSize;
    mbedtls_md_type_t m_fwChecksumAlgorithm;
    std::string m_fwAlgorithm;
    std::string m_fwChecksum;
    std::function<void(const bool&)> m_fwUpdatedCallback;
    // Number of tries we can request each chunk for,
    // after successfully getting one chunck the number is reset,
    // but if getting one chunck fails X amount of times the update process is aborted.
    uint8_t m_fwChunkRetries;
    uint16_t m_fwChunckSize;
    Shared_Attribute_Request_Callback m_fwResponseCallback;
    uint16_t m_fwChunkReceive;
#endif

    // The callback for when a PUBLISH message is received from the server.
    inline void onMQTTMessage(char* topic, uint8_t* payload, uint32_t length) {
      char message[JSON_STRING_SIZE(strlen(CB_ON_MESSAGE)) + JSON_STRING_SIZE(strlen(topic))];
      snprintf_P(message, sizeof(message), CB_ON_MESSAGE, topic);
      Logger::log(message);

      if (strncmp_P(RPC_TOPIC, topic, strlen(RPC_TOPIC)) == 0) {
        process_rpc_message(topic, payload, length);
      } else if (strncmp_P(ATTRIBUTE_RESPONSE_TOPIC, topic, strlen(ATTRIBUTE_RESPONSE_TOPIC)) == 0) {
        process_shared_attribute_request_message(topic, payload, length);
      } else if (strncmp_P(ATTRIBUTE_TOPIC, topic, strlen(ATTRIBUTE_TOPIC)) == 0) {
        process_shared_attribute_update_message(topic, payload, length);
      } else {
        if (strncmp_P(PROV_RESPONSE_TOPIC, topic, strlen(PROV_RESPONSE_TOPIC)) == 0) {
#if defined(ESP8266) || defined(ESP32) || defined(ARDUINO_AVR_MEGA)
          process_provisioning_response(topic, payload, length);
#endif
        } else if (strncmp_P(FIRMWARE_RESPONSE_TOPIC, topic, strlen(FIRMWARE_RESPONSE_TOPIC)) == 0) {
#if defined(ESP8266) || defined(ESP32)
          process_firmware_response(topic, payload, length);
#endif
        }
      }
    }
};

using ThingsBoard = ThingsBoardSized<>;

#endif // ThingsBoard_h
