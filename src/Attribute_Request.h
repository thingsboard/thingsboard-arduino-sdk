#ifndef Client_Side_RPC_h
#define Client_Side_RPC_h

// Local include.
#include "Attribute_Request_Callback.h"
#include "IAPI_Implementation.h"


// Attribute request API topics.
char constexpr ATTRIBUTE_REQUEST_TOPIC[] = "v1/devices/me/attributes/request/%u";
char constexpr ATTRIBUTE_RESPONSE_SUBSCRIBE_TOPIC[] = "v1/devices/me/attributes/response/+";
char constexpr ATTRIBUTE_RESPONSE_TOPIC[] = "v1/devices/me/attributes/response";


/// @brief Handles the internal implementation of the ThingsBoard shared and server-side Attribute API.
/// More specifically it handles the part for both types, where we can request the current value from the cloud.
/// See https://thingsboard.io/docs/user-guide/attributes/#shared-attributes and https://thingsboard.io/docs/user-guide/attributes/#client-side-attributes for more information
#if !THINGSBOARD_ENABLE_DYNAMIC
/// @tparam MaxSubscribtions Maximum amount of simultaneous shared or client scope attribute requests.
/// Once the maximum amount has been reached it is not possible to increase the size, this is done because it allows to allcoate the memory on the stack instead of the heap, default = Default_Subscriptions_Amount (2)
/// @tparam MaxAttributes Maximum amount of attributes that will ever be requested with the Attribute_Request_Callback, allows to use an array on the stack in the background.
/// Be aware though the size set in this template and the size passed to the ThingsBoard MaxAttributes template need to be the same or the value in this class lower, if not some of the requested keys may be lost.
/// Furthermore, if OTA Updates are utilized the size should never be decreased to less than 5, because that amount of attributes needs to be requested or updated to start the OTA update.
/// Meaning if the number is decreased the OTA update will not work correctly anymore and will not be able to be started anymore, default = Default_Attributes_Amount (5)
template<size_t MaxSubscribtions = Default_Subscriptions_Amount, size_t MaxAttributes = Default_Attributes_Amount>
#endif // !THINGSBOARD_ENABLE_DYNAMIC
class Attribute_Request : public IAPI_Implementation {
  public:
    /// @brief Constructor
    Attribute_Request() = default;

    /// @brief Requests one client-side attribute calllback,
    /// that will be called if the key-value pair from the server for the given client-side attributes is received.
    /// See https://thingsboard.io/docs/reference/mqtt-api/#request-attribute-values-from-the-server for more information
    /// @param callback Callback method that will be called
    /// @return Whether requesting the given callback was successful or not
#if THINGSBOARD_ENABLE_DYNAMIC
    bool Client_Attributes_Request(Attribute_Request_Callback const & callback) {
#else
    bool Client_Attributes_Request(Attribute_Request_Callback<MaxAttributes> const & callback) {
#endif // THINGSBOARD_ENABLE_DYNAMIC
        return Attributes_Request(callback, CLIENT_REQUEST_KEYS, CLIENT_RESPONSE_KEY);
    }

    /// @brief Requests one shared attribute calllback,
    /// that will be called if the key-value pair from the server for the given shared attributes is received.
    /// See https://thingsboard.io/docs/reference/mqtt-api/#request-attribute-values-from-the-server for more information
    /// @param callback Callback method that will be called
    /// @return Whether requesting the given callback was successful or not
#if THINGSBOARD_ENABLE_DYNAMIC
    bool Shared_Attributes_Request(Attribute_Request_Callback const & callback) {
#else
    bool Shared_Attributes_Request(Attribute_Request_Callback<MaxAttributes> const & callback) {
#endif // THINGSBOARD_ENABLE_DYNAMIC
        return Attributes_Request(callback, SHARED_REQUEST_KEY, SHARED_RESPONSE_KEY);
    }

    /// @brief Requests one client-side or shared attribute calllback,
    /// that will be called if the key-value pair from the server for the given client-side or shared attributes is received
    /// @param callback Callback method that will be called
    /// @param attributeRequestKey Key of the key-value pair that will contain the attributes we want to request
    /// @param attributeResponseKey Key of the key-value pair that will contain the attributes we got as a response
    /// @return Whether requesting the given callback was successful or not
#if THINGSBOARD_ENABLE_DYNAMIC
    bool Attributes_Request(Attribute_Request_Callback const & callback, char const * const attributeRequestKey, char const * const attributeResponseKey) {
#else
    bool Attributes_Request(Attribute_Request_Callback<MaxAttributes> const & callback, char const * const attributeRequestKey, char const * const attributeResponseKey) {
#endif // THINGSBOARD_ENABLE_DYNAMIC
        auto const & attributes = callback.Get_Attributes();

        // Check if any sharedKeys were requested
        if (attributes.empty()) {
            Logger::println(NO_KEYS_TO_REQUEST);
            return false;
        }
        else if (attributeRequestKey == nullptr || attributeResponseKey == nullptr) {
#if THINGSBOARD_ENABLE_DEBUG
            Logger::println(ATT_KEY_NOT_FOUND);
#endif // THINGSBOARD_ENABLE_DEBUG
            return false;
        }

#if THINGSBOARD_ENABLE_DYNAMIC
        Attribute_Request_Callback * registeredCallback = nullptr;
#else
        Attribute_Request_Callback<MaxAttributes> * registeredCallback = nullptr;
#endif // THINGSBOARD_ENABLE_DYNAMIC
        if (!Attributes_Request_Subscribe(callback, registeredCallback)) {
            return false;
        }
        else if (registeredCallback == nullptr) {
            return false;
        }

        // String are const char* and therefore stored as a pointer --> zero copy, meaning the size for the strings is 0 bytes,
        // Data structure size depends on the amount of key value pairs passed + the default clientKeys or sharedKeys
        // See https://arduinojson.org/v6/assistant/ for more information on the needed size for the JsonDocument
        StaticJsonDocument<JSON_OBJECT_SIZE(1)> requestBuffer;

        // Calculate the size required for the char buffer containing all the attributes seperated by a comma,
        // before initalizing it so it is possible to allocate it on the stack
        size_t size = 0U;
        for (const auto & att : attributes) {
            if (Helper::stringIsNullorEmpty(att)) {
                continue;
            }

            size += strlen(att);
            size += strlen(COMMA);
        }

        // Initalizes complete array to 0, required because strncat needs both destination and source to contain proper null terminated strings
        char request[size] = {};
        for (const auto & att : attributes) {
            if (Helper::stringIsNullorEmpty(att)) {
#if THINGSBOARD_ENABLE_DEBUG
                Logger::println(ATT_IS_NULL);
#endif // THINGSBOARD_ENABLE_DEBUG
                continue;
            }

            strncat(request, att, size);
            size -= strlen(att);
            strncat(request, COMMA, size);
            size -= strlen(COMMA);
        }

        // Ensure to cast to const, this is done so that ArduinoJson does not copy the value but instead simply store the pointer, which does not require any more memory,
        // besides the base size needed to allocate one key-value pair. Because if we don't the char array would be copied
        // and because there is not enough space the value would simply be "undefined" instead. Which would cause the request to not be sent correctly
        requestBuffer[attributeRequestKey] = static_cast<const char*>(request);

        m_request_id++;
        registeredCallback->Set_Request_ID(m_request_id);
        registeredCallback->Set_Attribute_Key(attributeResponseKey);
        registeredCallback->Start_Timeout_Timer();

        char topic[Helper::detectSize(ATTRIBUTE_REQUEST_TOPIC, m_request_id)] = {};
        (void)snprintf(topic, sizeof(topic), ATTRIBUTE_REQUEST_TOPIC, m_request_id);

        size_t const objectSize = Helper::Measure_Json(requestBuffer);
        return m_send_callback.Call_Callback(topic, requestBuffer, objectSize);
    }

    /// @brief Subscribes to attribute response topic
    /// @param callback Callback method that will be called
    /// @param registeredCallback Editable pointer to a reference of the local version that was copied from the passed callback
    /// @return Whether requesting the given callback was successful or not
#if THINGSBOARD_ENABLE_DYNAMIC
    bool Attributes_Request_Subscribe(Attribute_Request_Callback const & callback, Attribute_Request_Callback * & registeredCallback) {
#else
    bool Attributes_Request_Subscribe(Attribute_Request_Callback<MaxAttributes> const & callback, Attribute_Request_Callback<MaxAttributes> * & registeredCallback) {
#endif // THINGSBOARD_ENABLE_DYNAMIC
#if !THINGSBOARD_ENABLE_DYNAMIC
        if (m_attribute_request_callbacks.size() + 1 > m_attribute_request_callbacks.capacity()) {
            Logger::printfln(MAX_SUBSCRIPTIONS_EXCEEDED, CLIENT_SHARED_ATTRIBUTE_SUBSCRIPTIONS);
            return false;
        }
#endif // !THINGSBOARD_ENABLE_DYNAMIC
        if (!m_subscribe_callback.Call_Callback(ATTRIBUTE_RESPONSE_SUBSCRIBE_TOPIC)) {
            Logger::printfln(SUBSCRIBE_TOPIC_FAILED, ATTRIBUTE_RESPONSE_SUBSCRIBE_TOPIC);
          return false;
        }

        // Push back given callback into our local vector
        m_attribute_request_callbacks.push_back(callback);
        registeredCallback = &m_attribute_request_callbacks.back();
        return true;
    }

    /// @brief Unsubscribes all client-side or shared attributes request callbacks
    /// @return Whether unsubcribing the previously subscribed callbacks
    /// and from the  attribute response topic, was successful or not
    bool Attributes_Request_Unsubscribe() {
        m_attribute_request_callbacks.clear();
        return m_unsubscribe_callback.Call_Callback(ATTRIBUTE_RESPONSE_SUBSCRIBE_TOPIC);
    }

    const char * const get_response_topic_string() const override {
        return ATTRIBUTE_RESPONSE_TOPIC;
    }

#if !THINGSBOARD_USE_ESP_TIMER
    void loop() override {
        for (auto & attribute_request : m_attribute_request_callbacks) {
            attribute_request.Update_Timeout_Timer();
        }
    }
#endif // !THINGSBOARD_USE_ESP_TIMER

    void proces_response(char * const topic, JsonObjectConst & data) const override {
        size_t const request_id = Helper::parseRequestId(ATTRIBUTE_RESPONSE_TOPIC, topic);

        for (auto it = m_attribute_request_callbacks.begin(); it != m_attribute_request_callbacks.end(); ++it) {
            auto & attribute_request = *it;

            if (attribute_request.Get_Request_ID() != request_id) {
                continue;
            }
            char const * const attributeResponseKey = attribute_request.Get_Attribute_Key();
            if (attributeResponseKey == nullptr) {
#if THINGSBOARD_ENABLE_DEBUG
                Logger::println(ATT_KEY_NOT_FOUND);
#endif // THINGSBOARD_ENABLE_DEBUG
                goto delete_callback;
            }
            else if (!data) {
#if THINGSBOARD_ENABLE_DEBUG
                Logger::println(ATT_KEY_NOT_FOUND);
#endif // THINGSBOARD_ENABLE_DEBUG
                goto delete_callback;
            }

            if (data.containsKey(attributeResponseKey)) {
                data = data[attributeResponseKey];
            }

#if THINGSBOARD_ENABLE_DEBUG
            Logger::printfln(CALLING_REQUEST_CB, request_id);
#endif // THINGSBOARD_ENABLE_DEBUG
            attribute_request.Stop_Timeout_Timer();
            attribute_request.Call_Callback(data);

            delete_callback:
            // Delete callback because the changes have been requested and the callback is no longer needed
            Helper::remove(m_attribute_request_callbacks, it);
            break;
        }

        // Unsubscribe from the shared attribute request topic,
        // if we are not waiting for any further responses with shared attributes from the server.
        // Will be resubscribed if another request is sent anyway
        if (m_attribute_request_callbacks.empty()) {
            (void)Attributes_Request_Unsubscribe();
        }
    }

    // Vectors or array (depends on wheter if THINGSBOARD_ENABLE_DYNAMIC is set to 1 or 0), hold copy of the actual passed data, this is to ensure they stay valid,
    // even if the user only temporarily created the object before the method was called.
    // This can be done because all Callback methods mostly consists of pointers to actual object so copying them
    // does not require a huge memory overhead and is acceptable especially in comparsion to possible problems that could
    // arise if references were used and the end user does not take care to ensure the Callbacks live on for the entirety
    // of its usage, which will lead to dangling references and undefined behaviour.
    // Therefore copy-by-value has been choosen as for this specific use case it is more advantageous,
    // especially because at most we copy internal vectors or array, that will only ever contain a few pointers
#if THINGSBOARD_ENABLE_DYNAMIC
    Vector<Attribute_Request_Callback>                  m_attribute_request_callbacks; // Client-side or shared attribute request callback vector
#else
    Array<Attribute_Request_Callback, MaxSubscribtions> m_attribute_request_callbacks; // Client-side or shared attribute request callback array
#endif // THINGSBOARD_ENABLE_DYNAMIC
    size_t                                              m_request_id;                  // Allows nearly 4.3 million requests before wrapping back to 0
};

#endif // Client_Side_RPC_h
