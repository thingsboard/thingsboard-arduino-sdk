#ifndef OTA_Firmware_Update_h
#define OTA_Firmware_Update_h

// Local includes.
#include "Attribute_Request.h"
#include "Shared_Attribute_Update.h"
#include "OTA_Handler.h"
#include "API_Implementation.h"


uint8_t constexpr OTA_ATTRIBUTE_KEYS_AMOUNT = 5U;
uint64_t constexpr OTA_REQUEST_TIMEOUT = 5000U * 1000U;
char constexpr NO_FW_REQUEST_RESPONSE[] = "Did not receive requested shared attribute firmware keys in (%lu) microseconds. Aborting firmware update, restart with the same call again after ensure the keys actually exist on the device and ensuring the device is connected to the MQTT broker";
// Firmware topics.
char constexpr FIRMWARE_RESPONSE_TOPIC[] = "v2/fw/response/0/chunk";
char constexpr FIRMWARE_RESPONSE_SUBSCRIBE_TOPIC[] = "v2/fw/response/#";
char constexpr FIRMWARE_REQUEST_TOPIC[] = "v2/fw/request/0/chunk/%u";
// Firmware data keys.
char constexpr CURR_FW_TITLE_KEY[] = "current_fw_title";
char constexpr CURR_FW_VER_KEY[] = "current_fw_version";
char constexpr FW_ERROR_KEY[] = "fw_error";
char constexpr FW_STATE_KEY[] = "fw_state";
char constexpr FW_VER_KEY[] = "fw_version";
char constexpr FW_TITLE_KEY[] = "fw_title";
char constexpr FW_CHKS_KEY[] = "fw_checksum";
char constexpr FW_CHKS_ALGO_KEY[] = "fw_checksum_algorithm";
char constexpr FW_SIZE_KEY[] = "fw_size";
char constexpr CHECKSUM_AGORITM_MD5[] = "MD5";
char constexpr CHECKSUM_AGORITM_SHA256[] = "SHA256";
char constexpr CHECKSUM_AGORITM_SHA384[] = "SHA384";
char constexpr CHECKSUM_AGORITM_SHA512[] = "SHA512";
// Log messages.
char constexpr NUMBER_PRINTF[] = "%u";
char constexpr NO_FW[] = "No new firmware assigned on the given device";
char constexpr EMPTY_FW[] = "Given firmware was NULL";
char constexpr FW_UP_TO_DATE[] = "Firmware version (%s) already up to date";
char constexpr FW_NOT_FOR_US[] = "Firmware title (%s) not same as received title (%s)";
char constexpr FW_CHKS_ALGO_NOT_SUPPORTED[] = "Checksum algorithm (%s) is not supported";
char constexpr NOT_ENOUGH_RAM[] = "Temporary allocating more internal client buffer failed, decrease OTA chunk size or decrease overall heap usage";
char constexpr RESETTING_FAILED[] = "Preparing for OTA firmware updates failed, attributes might be NULL";
#if THINGSBOARD_ENABLE_DEBUG
char constexpr PAGE_BREAK[] = "=================================";
char constexpr NEW_FW[] = "A new Firmware is available:";
char constexpr FROM_TOO[] = "(%s) => (%s)";
char constexpr DOWNLOADING_FW[] = "Attempting to download over MQTT...";
#endif // THINGSBOARD_ENABLE_DEBUG


/// @brief Handles the internal implementation of the ThingsBoard over the air firmware update API.
/// See https://thingsboard.io/docs/user-guide/ota-updates/ for more information
class OTA_Firmware_Update : public API_Implementation {
  public:
    /// @brief Constructor
    OTA_Firmware_Update()
      : m_fw_callback()
      , m_previous_buffer_size(0U)
      , m_change_buffer_size(false)
#if THINGSBOARD_ENABLE_STL
      , m_ota(std::bind(&OTA_Firmware_Update::Publish_Chunk_Request, this, std::placeholders::_1), std::bind(&OTA_Firmware_Update::Firmware_Send_State, this, std::placeholders::_1, std::placeholders::_2), std::bind(&OTA_Firmware_Update::Firmware_OTA_Unsubscribe, this))
#else
      , m_ota(OTA_Firmware_Update::staticPublishChunk, OTA_Firmware_Update::staticFirmwareSend, OTA_Firmware_Update::staticUnsubscribe)
#endif // THINGSBOARD_ENABLE_STL
      , m_fw_attribute_update(nullptr)
      , m_fw_attribute_request(nullptr)
    {
#if !THINGSBOARD_ENABLE_DYNAMIC
        Shared_Attribute_Update<1U, OTA_ATTRIBUTE_KEYS_AMOUNT> fw_attribute_update;
        Attribute_Request<1U, OTA_ATTRIBUTE_KEYS_AMOUNT> fw_attribute_request;
#else
        Shared_Attribute_Update fw_attribute_update;
        Attribute_Request fw_attribute_request;
#endif // !THINGSBOARD_ENABLE_DYNAMIC
        m_fw_attribute_update = m_subscribe_api_callback.Call_Callback(fw_attribute_update);
        m_fw_attribute_request = m_subscribe_api_callback.Call_Callback(fw_attribute_request);
#if THINGSBOARD_ENABLE_STL
        m_subscribedInstance = nullptr;
#endif // THINGSBOARD_ENABLE_STL
    }

    /// @brief Checks if firmware settings are assigned to the connected device and if they are attempts to use those settings to start a firmware update.
    /// Will only be checked once and if there is no firmware assigned or if the assigned firmware is already installed this method will not update.
    /// This firmware status is only checked once, meaning to recheck the status either call this method again or use the Subscribe_Firmware_Update method.
    /// to be automatically informed and start the update if firmware has been assigned and it is not already installed.
    /// See https://thingsboard.io/docs/user-guide/ota-updates/ for more information
    /// @param callback Callback method that will be called
    /// @return Whether subscribing the given callback was successful or not
    bool Start_Firmware_Update(OTA_Update_Callback const & callback) {
        if (!Prepare_Firmware_Settings(callback))  {
            Logger::println(RESETTING_FAILED);
            return false;
        }

        // Request the firmware information
        constexpr char const * const array[OTA_ATTRIBUTE_KEYS_AMOUNT] = {FW_CHKS_KEY, FW_CHKS_ALGO_KEY, FW_SIZE_KEY, FW_TITLE_KEY, FW_VER_KEY};
        char const * const * const begin = array;
        char const * const * const end = array + OTA_ATTRIBUTE_KEYS_AMOUNT;
#if THINGSBOARD_ENABLE_DYNAMIC
#if THINGSBOARD_ENABLE_STL
        const Attribute_Request_Callback fw_request_callback(std::bind(&ThingsBoardSized::Firmware_Shared_Attribute_Received, this, std::placeholders::_1), OTA_REQUEST_TIMEOUT, std::bind(&ThingsBoardSized::Request_Timeout, this), begin, end);
#else
        const Attribute_Request_Callback fw_request_callback(ThingsBoardSized::onStaticFirmwareReceived, OTA_REQUEST_TIMEOUT, ThingsBoardSized::onStaticRequestTimeout, begin, end);
#endif // THINGSBOARD_ENABLE_STL
#else
#if THINGSBOARD_ENABLE_STL
        const Attribute_Request_Callback<MaxAttributes> fw_request_callback(std::bind(&ThingsBoardSized::Firmware_Shared_Attribute_Received, this, std::placeholders::_1), OTA_REQUEST_TIMEOUT, std::bind(&ThingsBoardSized::Request_Timeout, this), begin, end);
#else
        const Attribute_Request_Callback<MaxAttributes> fw_request_callback(ThingsBoardSized::onStaticFirmwareReceived, OTA_REQUEST_TIMEOUT, ThingsBoardSized::onStaticRequestTimeout, begin, end);
#endif // THINGSBOARD_ENABLE_STL
#endif //THINGSBOARD_ENABLE_DYNAMIC
        return m_fw_attribute_request->Shared_Attributes_Request(fw_request_callback);
    }

    /// @brief Stops the currently ongoing firmware update, calls the subscribed user finish callback with a failure if any update was stopped.
    /// See https://thingsboard.io/docs/user-guide/ota-updates/ for more information
    void Stop_Firmware_Update() {
        m_ota.Stop_Firmware_Update();
    }

    /// @brief Subscribes to any changes of the assigned firmware information on the connected device,
    /// meaning once we subscribed if we register any changes we will start the update if the given firmware is not already installed.
    /// Unlike Start_Firmware_Update this method only registers changes to the firmware information,
    /// meaning if the change occured while this device was asleep or turned off we will not update,
    /// to achieve that, it is instead recommended to call the Start_Firmware_Update method when the device has started once to check for that edge case.
    /// See https://thingsboard.io/docs/user-guide/ota-updates/ for more information
    /// @param callback Callback method that will be called
    /// @return Whether subscribing the given callback was successful or not
    bool Subscribe_Firmware_Update(OTA_Update_Callback const & callback) {
        if (!Prepare_Firmware_Settings(callback))  {
            Logger::println(RESETTING_FAILED);
            return false;
        }

        // Subscribes to changes of the firmware information
        char const * const array[OTA_ATTRIBUTE_KEYS_AMOUNT] = {FW_CHKS_KEY, FW_CHKS_ALGO_KEY, FW_SIZE_KEY, FW_TITLE_KEY, FW_VER_KEY};
        char const * const * const begin = array;
        char const * const * const end = array + OTA_ATTRIBUTE_KEYS_AMOUNT;
#if THINGSBOARD_ENABLE_DYNAMIC
#if THINGSBOARD_ENABLE_STL
        const Shared_Attribute_Callback fw_update_callback(std::bind(&ThingsBoardSized::Firmware_Shared_Attribute_Received, this, std::placeholders::_1), begin, end);
#else
        const Shared_Attribute_Callback fw_update_callback(ThingsBoardSized::onStaticFirmwareReceived, begin, end);
#endif // THINGSBOARD_ENABLE_STL
#else
#if THINGSBOARD_ENABLE_STL
        const Shared_Attribute_Callback<MaxAttributes> fw_update_callback(std::bind(&ThingsBoardSized::Firmware_Shared_Attribute_Received, this, std::placeholders::_1), begin, end);
#else
        const Shared_Attribute_Callback<MaxAttributes> fw_update_callback(ThingsBoardSized::onStaticFirmwareReceived, begin, end);
#endif // THINGSBOARD_ENABLE_STL
#endif //THINGSBOARD_ENABLE_DYNAMIC
        return m_fw_attribute_update->Shared_Attributes_Subscribe(fw_update_callback);
    }

    /// @brief Sends the given firmware title and firmware version to the cloud.
    /// See https://thingsboard.io/docs/user-guide/ota-updates/ for more information
    /// @param currFwTitle Current device firmware title
    /// @param currFwVersion Current device firmware version
    /// @return Whether sending the current device firmware information was successful or not
    bool Firmware_Send_Info(char const * const currFwTitle, char const * const currFwVersion) {
        StaticJsonDocument<JSON_OBJECT_SIZE(2)> currentFirmwareInfo;
        currentFirmwareInfo[CURR_FW_TITLE_KEY] = currFwTitle;
        currentFirmwareInfo[CURR_FW_VER_KEY] = currFwVersion;
        return m_send_telemtry_callback.Call_Callback(currentFirmwareInfo, Helper::Measure_Json(currentFirmwareInfo));
    }

    /// @brief Sends the given firmware state to the cloud.
    /// See https://thingsboard.io/docs/user-guide/ota-updates/ for more information
    /// @param currFwState Current firmware download state
    /// @param fwError Firmware error message that describes the current firmware state,
    /// pass nullptr or an empty string if the current state is not a failure state
    /// and therefore does not require any firmware error messsages, default = nullptr
    /// @return Whether sending the current firmware download state was successful or not
    bool Firmware_Send_State(char const * const currFwState, char const * const fwError = nullptr) {
        StaticJsonDocument<JSON_OBJECT_SIZE(2)> currentFirmwareState;
        if (!Helper::stringIsNullorEmpty(fwError)) {
            currentFirmwareState[FW_ERROR_KEY] = fwError;
        }
        currentFirmwareState[FW_STATE_KEY] = currFwState;
        return m_send_telemtry_callback.Call_Callback(currentFirmwareState, Helper::Measure_Json(currentFirmwareState));
    }

    const char * const Get_Response_Topic_String() const {
        return FIRMWARE_RESPONSE_TOPIC;
    }

    void Process_Response(char * const topic, uint8_t * const payload, size_t const & length) {
        size_t const request_id = Helper::parseRequestId(FIRMWARE_RESPONSE_TOPIC, topic);

        // Check if the remaining stack size of the current task would overflow the stack,
        // if it would allocate the memory on the heap instead to ensure no stack overflow occurs.
        if (getMaximumStackSize() < length) {
            uint8_t* binary = new uint8_t[length]();
            (void)memcpy(binary, payload, length);
            m_ota.Process_Firmware_Packet(request_id, binary, length);
            // Ensure to actually delete the memory placed onto the heap, to make sure we do not create a memory leak
            // and set the pointer to null so we do not have a dangling reference.
            delete[] binary;
            binary = nullptr;
        }
        else {
            uint8_t binary[length] = {};
            (void)memcpy(binary, payload, length);
            m_ota.Process_Firmware_Packet(request_id, binary, length);
        }
    }

    void Process_Json_Response(char * const topic, JsonObjectConst const & data) const override {
        // Nothing to do
    }

    bool Unsubscribe_Topic() override {
        Stop_Firmware_Update();
    }

    bool Resubscribe_Topic() {
        // Nothing to do.
    }

#if !THINGSBOARD_USE_ESP_TIMER
    /// @brief Updates the internal OTA timeout timer, that is used to detect if a chunk of the firmware was not received in time.
    /// Has to be used for boards that do not have the ESP Timer, because they have to update the internal time by hand
    void loop() {
        m_ota.update();
    }
#endif // !THINGSBOARD_USE_ESP_TIMER

  private:
    /// @brief Checks the included information in the callback,
    /// and attempts to sends the current device firmware information to the cloud
    /// @param callback Callback method that will be called
    /// @return Whether checking and sending the current device firmware information was successful or not
    bool Prepare_Firmware_Settings(OTA_Update_Callback const & callback) {
        char const * const currFwTitle = callback.Get_Firmware_Title();
        char const * const currFwVersion = callback.Get_Firmware_Version();

        if (m_fw_attribute_request == nullptr || m_fw_attribute_update == nullptr) {
            return false;
        }
        else if (Helper::stringIsNullorEmpty(currFwTitle) || Helper::stringIsNullorEmpty(currFwVersion)) {
            return false;
        }
        else if (!Firmware_Send_Info(currFwTitle, currFwVersion)) {
            return false;
        }

        m_fw_callback = callback;
        return true;
    }

    /// @brief Subscribes to the firmware response topic
    /// @return Whether subscribing to the firmware response topic was successful or not
    bool Firmware_OTA_Subscribe() {
        if (!m_subscribe_callback.Call_Callback(FIRMWARE_RESPONSE_SUBSCRIBE_TOPIC)) {
            char message[JSON_STRING_SIZE(strlen(SUBSCRIBE_TOPIC_FAILED)) + JSON_STRING_SIZE(strlen(FIRMWARE_RESPONSE_SUBSCRIBE_TOPIC))] = {};
            (void)snprintf(message, sizeof(message), SUBSCRIBE_TOPIC_FAILED, FIRMWARE_RESPONSE_SUBSCRIBE_TOPIC);
            Logger::println(message);
            Firmware_Send_State(FW_STATE_FAILED, message);
            return false;
        }
        return true;
    }

    /// @brief Unsubscribes from the firmware response topic and clears any memory associated with the firmware update,
    /// should not be called before actually fully completing the firmware update.
    /// @return Whether unsubscribing from the firmware response topic was successful or not
    bool Firmware_OTA_Unsubscribe() {
        // Buffer size has been set to another value before the update,
        // to allow to receive ota chunck packets that might be much bigger than the normal
        // buffer size would allow, therefore we return to the previous value to decrease overall memory usage
        if (m_change_buffer_size) {
            (void)setBufferSize(m_previous_buffer_size);
        }
        // Reset now not needed private member variables
        m_fw_callback = OTA_Update_Callback();
        // Unsubscribe from the topic
        return m_unsubscribe_callback.Call_Callback(FIRMWARE_RESPONSE_SUBSCRIBE_TOPIC);
    }

    /// @brief Publishes a request via MQTT to request the given firmware chunk
    /// @param request_chunck Chunk index that should be requested from the server
    /// @return Whether publishing the message was successful or not
    bool Publish_Chunk_Request(size_t const & request_chunck) {
        // Calculate the number of chuncks we need to request,
        // in order to download the complete firmware binary
        uint16_t const & chunk_size = m_fw_callback.Get_Chunk_Size();

        // Convert the interger size into a readable string
        char size[Helper::detectSize(NUMBER_PRINTF, chunk_size)] = {};
        (void)snprintf(size, sizeof(size), NUMBER_PRINTF, chunk_size);

        // Size adjuts dynamically to the current length of the currChunk number to ensure we don't cut it out of the topic string.
        char topic[Helper::detectSize(FIRMWARE_REQUEST_TOPIC, request_chunck)] = {};
        (void)snprintf(topic, sizeof(topic), FIRMWARE_REQUEST_TOPIC, request_chunck);

        return Send_Json_String(topic, size);
    }

    /// @brief Handler if the firmware shared attribute request times out without getting a response.
    /// Is used to signal that the update could not be started, because the current firmware information could not be fetched
    void Request_Timeout() {
        Logger::printfln(NO_FW_REQUEST_RESPONSE, OTA_REQUEST_TIMEOUT);
    }

    /// @brief Callback that will be called upon firmware shared attribute arrival
    /// @param data Json data containing key-value pairs for the needed firmware information,
    /// to ensure we have a firmware assigned and can start the update over MQTT
    void Firmware_Shared_Attribute_Received(JsonObjectConst const & data) {
        // Check if firmware is available for our device
        if (!data.containsKey(FW_VER_KEY) || !data.containsKey(FW_TITLE_KEY) || !data.containsKey(FW_CHKS_KEY) || !data.containsKey(FW_CHKS_ALGO_KEY) || !data.containsKey(FW_SIZE_KEY)) {
            Logger::println(NO_FW);
            Firmware_Send_State(FW_STATE_FAILED, NO_FW);
            return;
        }

        char const * const fw_title = data[FW_TITLE_KEY];
        char const * const fw_version = data[FW_VER_KEY];
        char const * const fw_checksum = data[FW_CHKS_KEY];
        char const * const fw_algorithm = data[FW_CHKS_ALGO_KEY];
        size_t const fw_size = data[FW_SIZE_KEY];

        char const * const curr_fw_title = m_fw_callback.Get_Firmware_Title();
        char const * const curr_fw_version = m_fw_callback.Get_Firmware_Version();

        if (fw_title == nullptr || fw_version == nullptr || curr_fw_title == nullptr || curr_fw_version == nullptr || fw_algorithm == nullptr || fw_checksum == nullptr) {
            Logger::println(EMPTY_FW);
            Firmware_Send_State(FW_STATE_FAILED, EMPTY_FW);
            return;
        }
        // If firmware version and title is the same, we do not initiate an update, because we expect the type of binary to be the same one we are currently using and therefore updating would be useless
        else if (strncmp(curr_fw_title, fw_title, strlen(curr_fw_title)) == 0 && strncmp(curr_fw_version, fw_version, strlen(curr_fw_version)) == 0) {
            char message[JSON_STRING_SIZE(strlen(FW_UP_TO_DATE)) + JSON_STRING_SIZE(strlen(curr_fw_version))] = {};
            (void)snprintf(message, sizeof(message), FW_UP_TO_DATE, curr_fw_version);
            Logger::println(message);
            Firmware_Send_State(FW_STATE_FAILED, message);
            return;
        }
        // If firmware title is not the same, we do not initiate an update, because we expect the binary to be for another type of device and downloading it on this device could possibly cause hardware issues
        else if (strncmp(curr_fw_title, fw_title, strlen(curr_fw_title)) != 0) {
            char message[JSON_STRING_SIZE(strlen(FW_NOT_FOR_US)) + JSON_STRING_SIZE(strlen(curr_fw_title)) + JSON_STRING_SIZE(strlen(fw_title))] = {};
            (void)snprintf(message, sizeof(message), FW_NOT_FOR_US, curr_fw_title, fw_title);
            Logger::println(message);
            Firmware_Send_State(FW_STATE_FAILED, message);
            return;
        }

        mbedtls_md_type_t fw_checksum_algorithm = mbedtls_md_type_t{};

        if (strncmp(CHECKSUM_AGORITM_MD5, fw_algorithm, strlen(CHECKSUM_AGORITM_MD5)) == 0) {
            fw_checksum_algorithm = mbedtls_md_type_t::MBEDTLS_MD_MD5;
        }
        else if (strncmp(CHECKSUM_AGORITM_SHA256, fw_algorithm, strlen(CHECKSUM_AGORITM_SHA256)) == 0) {
            fw_checksum_algorithm = mbedtls_md_type_t::MBEDTLS_MD_SHA256;
        }
        else if (strncmp(CHECKSUM_AGORITM_SHA384, fw_algorithm, strlen(CHECKSUM_AGORITM_SHA384)) == 0) {
            fw_checksum_algorithm = mbedtls_md_type_t::MBEDTLS_MD_SHA384;
        }
        else if (strncmp(CHECKSUM_AGORITM_SHA512, fw_algorithm, strlen(CHECKSUM_AGORITM_SHA512)) == 0) {
            fw_checksum_algorithm = mbedtls_md_type_t::MBEDTLS_MD_SHA512;
        }
        else {
            char message[JSON_STRING_SIZE(strlen(FW_CHKS_ALGO_NOT_SUPPORTED)) + JSON_STRING_SIZE(strlen(fw_algorithm))] = {};
            (void)snprintf(message, sizeof(message), FW_CHKS_ALGO_NOT_SUPPORTED, fw_algorithm);
            Logger::println(message);
            Firmware_Send_State(FW_STATE_FAILED, message);
            return;
        }

        if (!Firmware_OTA_Subscribe()) {
            return;
        }

#if THINGSBOARD_ENABLE_DEBUG
        Logger::println(PAGE_BREAK);
        Logger::println(NEW_FW);
        char firmware[JSON_STRING_SIZE(strlen(FROM_TOO)) + JSON_STRING_SIZE(strlen(curr_fw_version)) + JSON_STRING_SIZE(strlen(fw_version))] = {};
        (void)snprintf(firmware, sizeof(firmware), FROM_TOO, curr_fw_version, fw_version);
        Logger::println(firmware);
        Logger::println(DOWNLOADING_FW);
#endif // THINGSBOARD_ENABLE_DEBUG

        // Calculate the number of chuncks we need to request,
        // in order to download the complete firmware binary
        const uint16_t& chunk_size = m_fw_callback.Get_Chunk_Size();

        // Get the previous buffer size and cache it so the previous settings can be restored.
        m_previous_buffer_size = m_get_size_callback.Call_Callback();
        m_change_buffer_size = m_previous_buffer_size < (chunk_size + 50U);

        // Increase size of receive buffer
        if (m_change_buffer_size && !setBufferSize(chunk_size + 50U)) {
            Logger::println(NOT_ENOUGH_RAM);
            Firmware_Send_State(FW_STATE_FAILED, NOT_ENOUGH_RAM);
            return;
        }

        m_ota.Start_Firmware_Update(m_fw_callback, fw_size, fw_checksum, fw_checksum_algorithm);
    }

#if !THINGSBOARD_ENABLE_STL
    static void onStaticFirmwareReceived(JsonObjectConst const & data) {
        if (m_subscribedInstance == nullptr) {
            return;
        }
        m_subscribedInstance->Firmware_Shared_Attribute_Received(data);
    }

    static void onStaticRequestTimeout() {
        if (m_subscribedInstance == nullptr) {
            return;
        }
        m_subscribedInstance->Request_Timeout();
    }

    static bool staticPublishChunk(size_t const & request_chunck) {
        if (m_subscribedInstance == nullptr) {
            return false;
        }
        return m_subscribedInstance->Publish_Chunk_Request(request_chunck);
    }

    static bool staticFirmwareSend(char const * const currFwState, char const * const fwError = nullptr) {
        if (m_subscribedInstance == nullptr) {
            return false;
        }
        return m_subscribedInstance->Firmware_Send_State(currFwState, fwError);
    }

    static bool staticUnsubscribe() {
        if (m_subscribedInstance == nullptr) {
            return false;
        }
        return m_subscribedInstance->Firmware_OTA_Unsubscribe();
    }

    // Used API Implementation cannot call a instanced method when message arrives on subscribed topic.
    // Only free-standing function is allowed.
    // To be able to forward event to an instance, rather than to a function, this pointer exists.
    static OTA_Firmware_Update *m_subscribedInstance;
#endif // !THINGSBOARD_ENABLE_STL

    OTA_Update_Callback                                                  m_fw_callback;            // OTA update response callback
    uint16_t                                                             m_previous_buffer_size;   // Previous buffer size of the underlying client, used to revert to the previously configured buffer size if it was temporarily increased by the OTA update
    bool                                                                 m_change_buffer_size;     // Whether the buffer size had to be changed, because the previous internal buffer size was to small to hold the firmware chunks
    OTA_Handler<Logger>                                                  m_ota;                    // Class instance that handles the flashing and creating a hash from the given received binary firmware data
#if !THINGSBOARD_ENABLE_DYNAMIC
    Shared_Attribute_Update<1U, OTA_ATTRIBUTE_KEYS_AMOUNT>               *m_fw_attribute_update;   // API implementation to be informed if needed fw attributes have been updated
    Attribute_Request<1U, OTA_ATTRIBUTE_KEYS_AMOUNT>                     *m_fw_attribute_request;  // API implementation to request the needed fw attributes to start updating
#else
    Shared_Attribute_Update                                              *m_fw_attribute_update;   // API implementation to be informed if needed fw attributes have been updated
    Attribute_Request                                                    *m_fw_attribute_request;  // API implementation to request the needed fw attributes to start updating
#endif // !THINGSBOARD_ENABLE_DYNAMIC
};

#if !THINGSBOARD_ENABLE_STL
OTA_Firmware_Update *OTA_Firmware_Update::m_subscribedInstance = nullptr;
#endif

#endif // OTA_Firmware_Update_h
