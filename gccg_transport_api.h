// -------------------------------------------------------------------------------------------
// Copyright Amazon.com Inc. or its affiliates. All Rights Reserved.
// This file is part of the AWS CDI-SDK, licensed under the BSD 2-Clause "Simplified" License.
// License details at: https://github.com/vsf-tv/gccg-api/blob/mainline/LICENSE
// -------------------------------------------------------------------------------------------

#ifndef GCCG_TRANSPORT_API_H__
#define GCCG_TRANSPORT_API_H__

/**
 * @file
 * @brief
 * This file declares the public API data types, structures and functions that comprise the GCCG transport API. Each
 * connection is considered a single flow that may contain one or more media elements (video, audio and ancillary data).
 **/

/// Specify C linkage when compiling as C++ and define API interface export for Windows.
#if defined(_WIN32) && defined(__cplusplus)
#define GCCG_INTERFACE extern "C" __declspec (dllexport)
#elif defined(_WIN32)
#define GCCG_INTERFACE __declspec (dllexport)
#elif defined(__cplusplus)
#define GCCG_INTERFACE extern "C"
#else
#define GCCG_INTERFACE
#endif

/**
 * @brief Values used for API function return codes.
 */
typedef enum {
    kGccgStatusOk                = 0,
    kGccgStatusTimeoutExpired    = 1,
    kGccgStatusInvalidParameter  = 2,
    kGccgStatusBufferToSmall     = 3,
    kGccgStatusError             = 4
} GccgReturnStatus;

/// @brief A structure for holding a timestamp defined in seconds and nanoseconds.
typedef struct GccgTimestamp {
    /// The number of seconds since the SMPTE Epoch which is 1970-01-01T00:00:00.
    uint32_t seconds;
    /// The number of fractional seconds as measured in nanoseconds. The value in this field is always less than 10^9.
    uint32_t nanoseconds;
} GccgTimestamp;

typedef struct GccgSglEntry GccgSglEntry;
/**
 * @brief This structure represents a single, contiguous region of memory as part of a scatter-gather list.
 */
struct GccgSglEntry {
    /// @brief The starting address of the data.
    void* address_ptr;

    /// @brief The size of the data in bytes.
    int size_in_bytes;

    /// @brief User defined parameter. This value is not modified by the API.
    void* user_param_ptr;

    /// @brief Handle to private data used within the SDK that relates to this SGL entry. Do not use or modify this
    /// value.
    void* internal_data_ptr;

    /// @brief The next entry in the list or NULL if this is the final entry in the list.
    GccgSglEntry* next_ptr;
};

/**
 * @brief This structure defines a scatter-gather list (SGL) which is used to represent an array of data comprising one
 * or more contiguous regions of memory.
 */
typedef struct {
    /// @brief Origination timestamp to associate with the SGL.
    GccgTimestamp origination_timestamp;

    /// @brief Total size of data in the list, in units of bytes. This value can be calculated by walking the sgl_array,
    /// but is provided here for convenience and efficiency. NOTE: This value must be the same as the value calculated
    /// from walking the list and summing the size_in_bytes for each GccgSglEntry.
    int total_data_size;

    /// @brief Pointer to the first entry in the singly-linked list of SGL entries.
    GccgSglEntry* sgl_head_ptr;

    /// @brief Pointer to the last entry in the singly-linked list of SGL entries.
    GccgSglEntry* sgl_tail_ptr;

    /// @brief User defined parameter. This value is not modified by the API.
    void* user_param_ptr;

    /// @brief Handle to internal data used within the SDK that relates to this SGL. Do not use or modify this value.
    void* internal_data_ptr;
} GccgSgList;

/**
 * @brief Type used to define a list of media elements. When transmitting media, this list is used as a parameter passed
 * to the GccgTxPayload() API that defines the media elements to transmit. When receiving media, this list defines the
 * media elements received as part of the GccgRxCallback() callback API. It is also used by the GccgRxFreeBuffer() API
 * when the application is done with received media elements to free their associated resources.
 */
typedef struct {
    /// @brief Number of media elements in media_array.
    ///
    /// Note: This value must match the number of media elements configured when the connection was created using one
    /// of the ...ConnectionCreate() API functions and cannot change. If a payload does not contain one or more media
    /// elements, than the respective pointer(s) in sgl_array must be NULL.
    int count;

    GccgSgList** sgl_array; ///< Pointer to start of the array of media element SGL pointers.
} GccgMediaElements;

/**
 * @brief Type used as the handle (pointer to an opaque structure) for a transmitter or receiver connection. Each handle
 * represents a single data flow.
 */
typedef void* GccgConnectionHandle;

/**
 * @brief A structure of this type is passed as the parameter to CdiAvmTxCallback(). It contains data related to the
 * transmission of a single payload to a receiver and data related to the Tx connection.
 */
typedef struct {
    GccgReturnStatus status_code;

    /// @brief The handle of the instance which was created using a previous call to the GccgTxConnectionCreate() API
    /// function.
    GccgConnectionHandle connection_handle;

    /// @brief User defined callback parameter. This value is set as a parameter of the GccgTxConnectionCreate()
    /// API function.
    void* user_cb_param_ptr;
} GccgTxCbData;

/**
 * @brief Prototype of transmit data callback function. The user code must implement a function with this prototype and
 * provide it to GccgTxConnectionCreate() as a parameter.
 *
 * This callback function is invoked when a complete payload has been transmitted.
 * 
 * Note: In a single threaded event loop driven configuration, the GccgEventLoopPoll() API function must be called in order
 * for this callback function to be invoked. In a multi-threaded configuration, this function will be invoked on a thread
 * that is different from the thread that was used to create the connection.
 *
 * @param data_ptr A pointer to a GccgTxCbData structure.
 */
typedef void (*GccgTxCallback)(const GccgTxCbData* data_ptr);

/**
 * @brief A structure of this type is passed as the parameter to GccgRxCallback(). It contains a single payload sent
 * from a transmitter and data related to the Rx connection. Once the application has completed use of the buffer, it
 * must be freed using the GccgRxFreeBuffer() API.
 */
typedef struct {
    GccgReturnStatus status_code;

    /// @brief The handle of the instance which was created using a previous call to the GccgTxConnectionCreate() API
    /// function.
    GccgConnectionHandle connection_handle;

    /// @brief If no error occurred, a pointer to the payload configuration json string received with the payload.
    /// Otherwise the value will be NULL.
    const char *payload_json_str;

    /// @brief If no error occurred, a pointer to an array of MediaElements that contain the received payload data.
    /// Otherwise the value will be NULL.
    const GccgMediaElements* media_array;

    /// @brief User defined callback parameter. This value is set as a parameter of the GccgRxConnectionCreate()
    /// API function.
    void* user_cb_param_ptr;
} GccgRxCbData;

/**
 * @brief Prototype of receive data callback function. The user code must implement a function with this prototype and
 * provide it to the GccgRxConnectionCreate() API function as a parameter.
 *
 * This callback function is invoked when a complete payload has been received. The application must use the
 * GccgRxFreeBuffer() API function to free the buffer. This can either be done within the user callback function or
 * at a later time whenever the application is done with the buffer.
 * 
 * Note: In a single threaded event loop driven configuration, the GccgEventLoopPoll() API function must be called in order
 * for this callback function to be invoked. In a multi-threaded configuration, this function will be invoked on a thread
 * that is different from the thread that was used to create the connection.
 *
 * @param data_ptr A pointer to a GccgRxData structure.
 */
typedef void (*GccgRxCallback)(const GccgRxCbData* data_ptr);

/**
 * @brief Initialize the GCCG transport API. This defines the number of threads and thread priority the underlying
 * implementation can use. It must be invoked before using any other APIs.
 * 
 * @param maximum_thread_count Maximum number of threads the underlying API can use. If zero is specified, then the
 *                             GccgEventLoopPoll() API must be invoked as part of the application's single-threaded
 *                             event loop. Use -1 to not restrict the implementation.
 * @param maximum_thread_priority Maximum thread priority the underlying API can use. The range is 0 (lowest) to
 *                                99 (highest). Use -1 to not restrict the implementation.
 *
 * @return A value from the GccgReturnStatus enumeration.
 */
GCCG_INTERFACE GccgReturnStatus GccgInitialize(int maximum_thread_count, int maximum_thread_priority);

/**
 * Create an instance of a transmitter. When the instance is no longer needed, use the GccgConnectionDestroy()
 * API function to free-up resources that are being used by it.
 *
 * @param connection_json_ptr Pointer to connection configuration data in json format.
 *                            Note: The number and ordering of media elements declared in the JSON defines media_count
 *                            and the ordering in media_array when using the GccgTxPayload() API function.
 *                            The remote target must use the same configuration data when calling the
 *                            GccgRxConnectionCreate() API function to create the receive side of the connection.
 * @param tx_cb_ptr Address of the user function to call whenever a payload has been transmitted.
 * @param user_cb_param_ptr User defined callback parameter. This value is set as part of the GccgTxCbData data
 *                          whenever the rx_cb_ptr callback function is invoked.
 * @param ret_connection_json_buffer_size : Size of ret_connection_json_str buffer.
 * @param ret_connection_json_str : Pointer where to write returned json string. If size of buffer is not large enough,
 *                                  then kGccgStatusBufferToSmall will be returned.
 * @param ret_handle_ptr Pointer to returned connection handle. The handle is used as a parameter to other API functions
 *                       to identify this specific transmitter.
 *
 * @return A value from the GccgReturnStatus enumeration.
 */
GCCG_INTERFACE GccgReturnStatus GccgTxConnectionCreate(const char* connection_json_str,
                                                       GccgTxCallback tx_cb_ptr,
                                                       void* user_cb_param_ptr,
                                                       int ret_connection_json_buffer_size,
                                                       char* ret_connection_json_str,
                                                       GccgConnectionHandle* ret_handle_ptr);

/**
 * Create an instance of a receiver. When the instance is no longer needed, use the GccgConnectionDestroy()
 * API function to free-up resources that are being used by it.
 *
 * @param connection_json_ptr Pointer to connection configuration data in json format.
 *                            Note: The number and ordering of media elements declared in the JSON defines media_count
 *                            and the ordering in media_array when the GccgRxCallback() callback API function is invoked.
 *                            The remote host must use the same configuration data when calling the
 *                            GccgTxConnectionCreate() API function to create the transmit side of the connection.
 * @param use_linear_buffer If true, received payload data will be stored in a linear buffer. Otherwise, depending on
 *                          the underlying transport, payload data may be stored in a scatter-gather list of buffers.
 * @param rx_cb_ptr Address of the user function to call whenever a payload has been received.
 * @param user_cb_param_ptr User defined callback parameter. This value is set as part of the GccgRxCallback data
 *                          whenever the rx_cb_ptr callback function is invoked.
 * @param ret_connection_json_buffer_size : Size of ret_connection_json_str buffer.
 * @param ret_connection_json_str : Pointer where to write returned json string. If size of buffer is not large enough,
 *                                  then kGccgStatusBufferToSmall will be returned.
 * @param ret_handle_ptr Pointer to returned connection handle. The handle is used as a parameter to other API functions
 *                       to identify this specific receiver.
 *
 * @return A value from the GccgReturnStatus enumeration.
 */
GCCG_INTERFACE GccgReturnStatus GccgRxConnectionCreate(const char *connection_json_str,
                                                       bool use_linear_buffer,
                                                       GccgRxCallback rx_cb_ptr,
                                                       void* user_cb_param_ptr,
                                                       int ret_connection_json_buffer_size,
                                                       char* ret_connection_json_str,
                                                       GccgConnectionHandle* ret_handle_ptr);

/**
 * Destroy a specific Tx or Rx connection and free resources that were created for it.
 *
 * @param handle Connection handle returned by one of the ...ConnectionCreate() API functions.
 *
 * @return A value from the GccgReturnStatus enumeration.
 */
GCCG_INTERFACE GccgReturnStatus GccgConnectionDestroy(GccgConnectionHandle handle);

/**
 * Transmit a payload of data to the receiver. The connection must have been created with GccgTxConnectionCreate().
 * This function is asynchronous and will immediately return. The user callback function GccgTxCallback() registered
 * through GccgTxConnectionCreate() will be invoked when the payload has been acknowledged by the remote receiver or a
 * transmission timeout occurred.
 *
 * @param handle Connection handle returned by the GccgTxConnectionCreate() API function.
 * @param payload_json_str Pointer to payload configuration json string.
 * @param media_array Array of SGL's that define the size and location of each media element to transmit in this
 *                    payload. If a pointer within the array is NULL, then the payload does not contain an element for
 *                    that media.
 * @param timeout_microsecs Timeout period in microseconds. If the payload is not transmitted within this period,
 *                          transmission is canceled and the GccgTxCallback() callback API function invoked with
 *                          kGccgStatusTimeoutExpired returned as the status_code in GccgTxCbData.
 *
 * @return A value from the GCCG_INTERFACE enumeration.
 */
GCCG_INTERFACE GccgReturnStatus GccgTxPayload(GccgConnectionHandle handle,
                                              const char *payload_json_str,
                                              GccgMediaElements media_array,
                                              int timeout_microsecs);

/**
 * Free an array of receive buffers that was used by the GccgRxCallback() callback API function.
 *
 * @param media_array Pointer to a structure that contains an array of media elements to free and the number of elements
 *                    in the array.
 *
 * @return A value from the GCCG_INTERFACE enumeration.
 */
GCCG_INTERFACE GccgReturnStatus GccgRxFreeBuffer(GccgMediaElements *media_array);

/**
 * @brief Only required when using a single-threaded, event loop to service the API. Must specify a value of zero for
 *        maximum_thread_count when invoking the GccgInitialize() API function.
 * 
 * @param handle Connection handle returned by one of the create connection functions.
 * 
 * @return A value from the GCCG_INTERFACE enumeration.
 */
GCCG_INTERFACE GccgReturnStatus GccgEventLoopPoll(GccgConnectionHandle handle);

#endif // GCCG_TRANSPORT_API_H__
