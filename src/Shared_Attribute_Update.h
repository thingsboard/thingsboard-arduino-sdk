#ifndef Shared_Attribute_Update_h
#define Shared_Attribute_Update_h

// Local includes.
#include "Shared_Attribute_Callback.h"
#include "IAPI_Implementation.h"


// Log messages.
#if !THINGSBOARD_ENABLE_DYNAMIC
char constexpr SHARED_ATTRIBUTE_UPDATE_SUBSCRIPTIONS[] = "shared attribute update";
#endif // !THINGSBOARD_ENABLE_DYNAMIC


/// @brief Handles the internal implementation of the ThingsBoard shared attribute update API.
/// See https://thingsboard.io/docs/reference/mqtt-api/#subscribe-to-attribute-updates-from-the-server for more information
/// @tparam Logger Implementation that should be used to print error messages generated by internal processes and additional debugging messages if THINGSBOARD_ENABLE_DEBUG is set, default = DefaultLogger
#if THINGSBOARD_ENABLE_DYNAMIC
template <typename Logger = DefaultLogger>
#else
/// @tparam MaxSubscriptions Maximum amount of simultaneous server side rpc subscriptions.
/// Once the maximum amount has been reached it is not possible to increase the size, this is done because it allows to allcoate the memory on the stack instead of the heap, default = Default_Subscriptions_Amount (1)
/// @tparam MaxAttributes Maximum amount of attributes that will ever be requested with the Shared_Attribute_Callback, allows to use an array on the stack in the background, default = Default_Attributes_Amount (5)
template<size_t MaxSubscriptions = Default_Subscriptions_Amount, size_t MaxAttributes = Default_Attributes_Amount, typename Logger = DefaultLogger>
#endif // THINGSBOARD_ENABLE_DYNAMIC
class Shared_Attribute_Update : public IAPI_Implementation {
  public:
    /// @brief Constructor
    Shared_Attribute_Update() = default;

    /// @brief Subscribes multiple shared attribute callbacks,
    /// that will be called if the key-value pair from the server for the given shared attributes is received.
    /// Can be called even if we are currently not connected to the cloud,
    /// this is the case because the only interaction that requires an active connection is the subscription of the topic that we receive the response on
    /// and that subscription is also done automatically by the library once the device has established a connection to the cloud.
    /// Therefore this method can simply be called once at startup before a connection has been established
    /// and will then automatically handle the subscription of the topic once the connection has been established.
    /// See https://thingsboard.io/docs/reference/mqtt-api/#subscribe-to-attribute-updates-from-the-server for more information
    /// @tparam InputIterator Class that points to the begin and end iterator
    /// of the given data container, allows for using / passing either std::vector or std::array.
    /// See https://en.cppreference.com/w/cpp/iterator/input_iterator for more information on the requirements of the iterator
    /// @param first Iterator pointing to the first element in the data container
    /// @param last Iterator pointing to the end of the data container (last element + 1)
    /// @return Whether subscribing the given callbacks was successful or not
    template<typename InputIterator>
    bool Shared_Attributes_Subscribe(InputIterator const & first, InputIterator const & last) {
#if !THINGSBOARD_ENABLE_DYNAMIC
        size_t const size = Helper::distance(first, last);
        if (m_shared_attribute_update_callbacks.size() + size > m_shared_attribute_update_callbacks.capacity()) {
            Logger::printfln(MAX_SUBSCRIPTIONS_EXCEEDED, MAX_SUBSCRIPTIONS_TEMPLATE_NAME, SHARED_ATTRIBUTE_UPDATE_SUBSCRIPTIONS);
            return false;
        }
#endif // !THINGSBOARD_ENABLE_DYNAMIC
        (void)m_subscribe_topic_callback.Call_Callback(ATTRIBUTE_TOPIC);
        // Push back complete vector into our local m_shared_attribute_update_callbacks vector.
        m_shared_attribute_update_callbacks.insert(m_shared_attribute_update_callbacks.end(), first, last);
        return true;
    }

    /// @brief Subscribe one shared attribute callback,
    /// that will be called if the key-value pair from the server for the given shared attributes is received.
    /// Can be called even if we are currently not connected to the cloud,
    /// this is the case because the only interaction that requires an active connection is the subscription of the topic that we receive the response on
    /// and that subscription is also done automatically by the library once the device has established a connection to the cloud.
    /// Therefore this method can simply be called once at startup before a connection has been established
    /// and will then automatically handle the subscription of the topic once the connection has been established.
    /// See https://thingsboard.io/docs/reference/mqtt-api/#subscribe-to-attribute-updates-from-the-server for more information
    /// @param callback Callback method that will be called
    /// @return Whether subscribing the given callback was successful or not
#if THINGSBOARD_ENABLE_DYNAMIC
    bool Shared_Attributes_Subscribe(Shared_Attribute_Callback const & callback) {
#else
    bool Shared_Attributes_Subscribe(Shared_Attribute_Callback<MaxAttributes> const & callback) {
#endif // THINGSBOARD_ENABLE_DYNAMIC
#if !THINGSBOARD_ENABLE_DYNAMIC
        if (m_shared_attribute_update_callbacks.size() + 1U > m_shared_attribute_update_callbacks.capacity()) {
            Logger::printfln(MAX_SUBSCRIPTIONS_EXCEEDED, MAX_SUBSCRIPTIONS_TEMPLATE_NAME, SHARED_ATTRIBUTE_UPDATE_SUBSCRIPTIONS);
            return false;
        }
#endif // !THINGSBOARD_ENABLE_DYNAMIC
        (void)m_subscribe_topic_callback.Call_Callback(ATTRIBUTE_TOPIC);
        m_shared_attribute_update_callbacks.push_back(callback);
        return true;
    }

    /// @brief Unsubcribes all shared attribute callbacks.
    /// See https://thingsboard.io/docs/reference/mqtt-api/#subscribe-to-attribute-updates-from-the-server for more information
    /// @return Whether unsubcribing all the previously subscribed callbacks
    /// and from the attribute topic, was successful or not
    bool Shared_Attributes_Unsubscribe() {
        m_shared_attribute_update_callbacks.clear();
        return m_unsubscribe_topic_callback.Call_Callback(ATTRIBUTE_TOPIC);
    }

    API_Process_Type Get_Process_Type() const override {
        return API_Process_Type::JSON;
    }

    void Process_Response(char const * topic, uint8_t * payload, unsigned int length) override {
        // Nothing to do
    }

    void Process_Json_Response(char const * topic, JsonDocument const & data) override {
        JsonObjectConst object = data.template as<JsonObjectConst>();
        if (object.containsKey(SHARED_RESPONSE_KEY)) {
            object = object[SHARED_RESPONSE_KEY];
        }

#if THINGSBOARD_ENABLE_STL
#if THINGSBOARD_ENABLE_CXX20
#if THINGSBOARD_ENABLE_DYNAMIC
        auto filtered_shared_attribute_update_callbacks = m_shared_attribute_update_callbacks | std::views::filter([&object](Shared_Attribute_Callback const & shared_attribute) {
#else
        auto filtered_shared_attribute_update_callbacks = m_shared_attribute_update_callbacks | std::views::filter([&object](Shared_Attribute_Callback<MaxAttributes> const & shared_attribute) {
#endif // THINGSBOARD_ENABLE_DYNAMIC
#else
#if THINGSBOARD_ENABLE_DYNAMIC
        Vector<Shared_Attribute_Callback> filtered_shared_attribute_update_callbacks = {};
        std::copy_if(m_shared_attribute_update_callbacks.begin(), m_shared_attribute_update_callbacks.end(), std::back_inserter(filtered_shared_attribute_update_callbacks), [&object](Shared_Attribute_Callback const & shared_attribute) {
#else
        Array<Shared_Attribute_Callback<MaxAttributes>, MaxSubscriptions> filtered_shared_attribute_update_callbacks = {};
        std::copy_if(m_shared_attribute_update_callbacks.begin(), m_shared_attribute_update_callbacks.end(), std::back_inserter(filtered_shared_attribute_update_callbacks), [&object](Shared_Attribute_Callback<MaxAttributes> const & shared_attribute) {
#endif // THINGSBOARD_ENABLE_DYNAMIC
#endif // THINGSBOARD_ENABLE_CXX20
            return (shared_attribute.Get_Attributes().empty() || std::find_if(shared_attribute.Get_Attributes().begin(), shared_attribute.Get_Attributes().end(), [&object](const char * att) {
                return object.containsKey(att);
            }) != shared_attribute.Get_Attributes().end());
        });

        for (auto const & shared_attribute : filtered_shared_attribute_update_callbacks) {
#else
        for (auto const & shared_attribute : m_shared_attribute_update_callbacks) {
            if (shared_attribute.Get_Attributes().empty()) {
                // No specifc keys were subscribed so we call the callback anyway, assumed to be subscribed to any update
                shared_attribute.Call_Callback(object);
                continue;
            }

            char const * requested_att = nullptr;

            for (auto const & att : shared_attribute.Get_Attributes()) {
                if (Helper::stringIsNullorEmpty(att)) {
                    continue;
                }
                // Check if the request contained any of our requested keys and
                // break early if the key was requested from this callback.
                if (object.containsKey(att)) {
                    requested_att = att;
                    break;
                }
            }

            // Check if this callback did not request any keys that were in this response,
            // if there were not we simply continue with the next subscribed callback.
            if (requested_att == nullptr) {
                continue;
            }
#endif // THINGSBOARD_ENABLE_STL
            shared_attribute.Call_Callback(object);
        }
    }

    bool Compare_Response_Topic(char const * topic) const override {
        return strncmp(ATTRIBUTE_TOPIC, topic, strlen(ATTRIBUTE_TOPIC) + 1) == 0;
    }

    bool Unsubscribe() override {
        return Shared_Attributes_Unsubscribe();
    }

    bool Resubscribe_Topic() override {
        if (!m_shared_attribute_update_callbacks.empty() && !m_subscribe_topic_callback.Call_Callback(ATTRIBUTE_TOPIC)) {
            Logger::printfln(SUBSCRIBE_TOPIC_FAILED, ATTRIBUTE_TOPIC);
            return false;
        }
        return true;
    }

#if !THINGSBOARD_USE_ESP_TIMER
    void loop() override {
        // Nothing to do
    }
#endif // !THINGSBOARD_USE_ESP_TIMER

    void Initialize() override {
        // Nothing to do
    }

    void Set_Client_Callbacks(Callback<void, IAPI_Implementation &>::function subscribe_api_callback, Callback<bool, char const * const, JsonDocument const &, size_t const &>::function send_json_callback, Callback<bool, char const * const, char const * const>::function send_json_string_callback, Callback<bool, char const * const>::function subscribe_topic_callback, Callback<bool, char const * const>::function unsubscribe_topic_callback, Callback<uint16_t>::function get_size_callback, Callback<bool, uint16_t>::function set_buffer_size_callback, Callback<size_t *>::function get_request_id_callback) override {
        m_subscribe_topic_callback.Set_Callback(subscribe_topic_callback);
        m_unsubscribe_topic_callback.Set_Callback(unsubscribe_topic_callback);
    }

  private:
    Callback<bool, char const * const>                                       m_subscribe_topic_callback = {};          // Subscribe mqtt topic client callback
    Callback<bool, char const * const>                                       m_unsubscribe_topic_callback = {};        // Unubscribe mqtt topic client callback

    // Vectors or array (depends on wheter if THINGSBOARD_ENABLE_DYNAMIC is set to 1 or 0), hold copy of the actual passed data, this is to ensure they stay valid,
    // even if the user only temporarily created the object before the method was called.
    // This can be done because all Callback methods mostly consists of pointers to actual object so copying them
    // does not require a huge memory overhead and is acceptable especially in comparsion to possible problems that could
    // arise if references were used and the end user does not take care to ensure the Callbacks live on for the entirety
    // of its usage, which will lead to dangling references and undefined behaviour.
    // Therefore copy-by-value has been choosen as for this specific use case it is more advantageous,
    // especially because at most we copy internal vectors or array, that will only ever contain a few pointers
#if THINGSBOARD_ENABLE_DYNAMIC
    Vector<Shared_Attribute_Callback>                                        m_shared_attribute_update_callbacks = {}; // Shared attribute update callbacks vector
#else
    Array<Shared_Attribute_Callback<MaxAttributes>, MaxSubscriptions>        m_shared_attribute_update_callbacks = {}; // Shared attribute update callbacks array
#endif // THINGSBOARD_ENABLE_DYNAMIC
};

#endif // Shared_Attribute_Update_h
