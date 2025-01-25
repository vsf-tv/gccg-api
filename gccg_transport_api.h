// -------------------------------------------------------------------------------------------
// Copyright Amazon.com Inc. or its affiliates. All Rights Reserved.
// This file is part of the VSF GCCG API, licensed under the BSD 2-Clause "Simplified" License.
// License details at: https://github.com/vsf-tv/gccg-api/blob/mainline/LICENSE
// -------------------------------------------------------------------------------------------

#ifndef GCCG_TRANSPORT_API_H__
#define GCCG_TRANSPORT_API_H__

#include <stdint.h>

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

/**
 * @brief Type used as the handle (pointer to an opaque structure) for a transmitter or receiver connection. Each handle
 * represents a single data flow.
 */
typedef void* GccgConnectionHandle;

/**
 * @brief Type used as the handle (index to an opaque structure) for a buffer allocated by the api. Each api can use
 * this as required
 */
typedef uint32_t GccgBufferHandle;

/// @brief A structure for holding buffer information for TX, supports segmented frames with 8 segments
typedef struct GccgBuffer {
    /// Address of memory buffer to use
    void *buffer;
    /// length of buffer
    uint32_t bytes;
    /// set to 1 if the buffer is a segment of a buffer, 0 if its a contiguous element
    uint32_t is_segment;
    /// segment index, index of segment in a frame, 0 - 7, sub segments are only available in 1/8 chunk of a payload
    uint32_t segment_index;
    /// timestamp applied to buffer
    GccgTimestamp origination_timestamp;
    /// handle of connection that the buffers relates to
    GccgConnectionHandle connection_handle;
    /// handle fo the buffer managed by the api
    GccgBufferHandle buffer_handle;
} GccgBuffer;

/// @brief A structure for holding buffer segments information, number of segments fixed at 8,
#define GCCG_SEGMENTS (8)

typedef struct GccgBufferSegments {
    GccgBuffer segments[GCCG_SEGMENTS];
} GccgBufferSegments;

/**
 * @brief A structure of this type is passed as the parameter to GccgTxCallback(). It contains data related to the
 * transmission of a single payload to a receiver and data related to the Tx connection.
 */
typedef struct {
    GccgReturnStatus status_code;

    /// @brief The handle of the instance which was created using a previous call to the GccgTxConnectionCreate() API
    /// function.
    GccgConnectionHandle connection_handle;

    /// @brief User defined callback parameter. This value is set as a parameter of the GccgTxPayload() API function. The
    /// value is not modified by the SDK.
    void* user_cb_param_ptr;
} GccgTxCbData;

/**
 * @brief Prototype of transmit data callback function. The user code must implement a function with this prototype and
 * provide it to GccgTxConnectionCreate() as a parameter.
 *
 * This callback function is invoked when a complete payload has been transmitted.
 *
 * Note: In a single threaded event loop driven configuration, the GccgEventLoopPoll() API function must be called in order
 * for this callback function to be invoked. In a multi-threaded configuration, this function may be invoked on a thread
 * that is different from the thread that was used to create the connection. The SDK ensures that only one thread will call
 * this API at a time, so thread-safety does not have to be implemented in the application.
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

    /// @brief If no error occurred, a pointer to the payload configuration json string received with the payload.
    /// Otherwise the value will be NULL.
    const char *payload_json_str;

    /// @brief If no error occurred, a pointer to a GccgBuffer that contains the received payload data.
    /// Otherwise the value will be NULL.
    const GccgBuffer *buffer;

    /// @brief User defined callback parameter. This value is set as a parameter of the GccgRxConnectionCreate()
    /// API function. The value is not modified by the SDK.
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
 * for this callback function to be invoked. In a multi-threaded configuration, this function may be invoked on a thread
 * that is different from the thread that was used to create the connection. The SDK ensures that only one thread will call
 * this API at a time, so thread-safety does not have to be implemented in the application.
 *
 * @param data_ptr A pointer to a GccgRxData structure.
 */
typedef void (*GccgRxCallback)(const GccgRxCbData* data_ptr);

/**
 * @brief Initialize the GCCG transport API. This defines the number of threads and thread priority the underlying
 * implementation can use. It must be invoked once before using any other APIs.
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
 * API function to free-up resources that are being used by it. This API is thread-safe.
 *
 * @param connection_json_ptr Pointer to connection configuration data in json format.
 *                            Note: The number and ordering of media elements declared in the JSON defines media_count
 *                            and the ordering in media_array when using the GccgTxPayload() API function.
 *                            The remote target must use the same configuration data when calling the
 *                            GccgRxConnectionCreate() API function to create the receive side of the connection.
 * @param tx_buffer_size_bytes tx_buffer_size_bytes The size in bytes of a memory region for holding a single transmit payload data.
 * @param tx_buffer_count Positive integer count of buffers requested by the application for sending
 * @param tx_cb_ptr Address of the user function to call whenever a payload has been transmitted.
 * @param ret_connection_json_buffer_size Size of ret_connection_json_str buffer.
 * @param ret_connection_json_str Pointer where to write returned json string. If size of buffer is not large enough,
 *                                then kGccgStatusBufferToSmall will be returned.
 * @param ret_handle_ptr Pointer to returned connection handle. The handle is used as a parameter to other API functions
 *                       to identify this specific transmitter.
 *
 * @return A value from the GccgReturnStatus enumeration.
 */
GCCG_INTERFACE GccgReturnStatus GccgTxConnectionCreate(const char* connection_json_str,
                                                       uint64_t tx_buffer_size_bytes,
                                                       uint32_t tx_buffer_count,
                                                       GccgTxCallback tx_cb_ptr,
                                                       int ret_connection_json_buffer_size,
                                                       char* ret_connection_json_str,
                                                       GccgConnectionHandle* ret_handle_ptr);

/**
 * Create an instance of a receiver. When the instance is no longer needed, use the GccgConnectionDestroy()
 * API function to free-up resources that are being used by it. This API is thread-safe.
 *
 * @param connection_json_ptr Pointer to connection configuration data in json format.
 *                            Note: The number and ordering of media elements declared in the JSON defines media_count
 *                            and the ordering in media_array when the GccgRxCallback() callback API function is invoked.
 *                            The remote host must use the same configuration data when calling the
 *                            GccgTxConnectionCreate() API function to create the transmit side of the connection.
 * @param rx_buffer_size_bytes The size in bytes of a memory region for holding a single receive payload data.
 * @param rx_cb_ptr Address of the user function to call whenever a payload has been received.
 * @param user_cb_param_ptr User defined callback parameter. This value is set as part of the GccgRxCbData data
 *                          whenever the rx_cb_ptr callback function is invoked. The value is not modified by the SDK.
 * @param ret_connection_json_buffer_size Size of ret_connection_json_str buffer.
 * @param ret_connection_json_str Pointer where to write returned json string. If size of buffer is not large enough,
 *                                then kGccgStatusBufferToSmall will be returned.
 * @param ret_handle_ptr Pointer to returned connection handle. The handle is used as a parameter to other API functions
 *                       to identify this specific receiver.
 *
 * @return A value from the GccgReturnStatus enumeration.
 */
GCCG_INTERFACE GccgReturnStatus GccgRxConnectionCreate(const char *connection_json_str,
                                                       uint64_t rx_buffer_size_bytes,
                                                       GccgRxCallback rx_cb_ptr,
                                                       void* user_cb_param_ptr,
                                                       int ret_connection_json_buffer_size,
                                                       char* ret_connection_json_str,
                                                       GccgConnectionHandle* ret_handle_ptr);

/**
 * Destroy a specific Tx or Rx connection and free resources that were created for it. This API is thread-safe.
 *
 * @param handle Connection handle returned by one of the ...ConnectionCreate() API functions.
 *
 * @return A value from the GccgReturnStatus enumeration.
 */
GCCG_INTERFACE GccgReturnStatus GccgConnectionDestroy(GccgConnectionHandle handle);

/**
 * Request a buffer for the transmission of data payload to the receiver.
 * The connection must have been created with GccgTxConnectionCreate().
 * If no buffer is free a NULL pointer is returned
 * This API is thread-safe.
 *
 * @param handle Connection handle returned by the GccgTxConnectionCreate() API function.
 * @param buffer Pointer to a GccgBuffer to receive the data for a transmit buffer
 *
 * @return A value from the GCCG_INTERFACE enumeration.
 */
GCCG_INTERFACE GccgReturnStatus GccgRequestTxBuffer(GccgConnectionHandle handle, GccgBuffer *buffer);

/**
 * Request a set of buffers for the segmented transmission of data payload to the receiver.
 * The connection must have been created with GccgTxConnectionCreate().
 * If no buffer is free a NULL pointer is returned
 * This API is thread-safe.
 *
 * @param handle Connection handle returned by the GccgTxConnectionCreate() API function.
 * @param buffer_segments Pointer array of GccgBuffers to receive the data for a transmit buffers
 *
 * @return A value from the GCCG_INTERFACE enumeration.
 */
GCCG_INTERFACE GccgReturnStatus GccgRequestTxBufferSegments(GccgConnectionHandle handle, GccgBufferSegments *buffer_segments);

/**
 * Request a buffer for the Transmit a payload of data to the receiver.
 * The connection must have been created with GccgTxConnectionCreate().
 * If no buffer is free a NULL pointer is returned
 * This API is thread-safe.
 *
 * @param handle Connection handle returned by the GccgTxConnectionCreate() API function.
 * @param payload_json_str Pointer to payload configuration json string.
 * @param buffer pointer to a GccgBuffer for this connection
 * @param user_cb_param_ptr User defined callback parameter. This value is set as part of the GccgTxCbData data
 *                          whenever the tx_cb_ptr callback function specified in the GccgTxConnectionCreate() API
 *                          is invoked. The value is not modified by the SDK.
 * @param timeout_microsecs Timeout period in microseconds. If the payload is not transmitted within this period,
 *                          transmission is canceled and the GccgTxCallback() callback API function invoked with
 *                          kGccgStatusTimeoutExpired returned as the status_code in GccgTxCbData.
 *
 * @return A value from the GCCG_INTERFACE enumeration.
 */
GCCG_INTERFACE GccgReturnStatus GccgTxPayload(GccgConnectionHandle handle,
                                              const GccgBuffer *buffer,
                                              void* user_cb_param_ptr,
                                              int timeout_microsecs);

/**
 * Free a receive buffer that was used by the GccgRxCallback() callback API function. This API is thread-safe.
 *
 * @param buffer Pointer to a GccgBuffer *buffer that is to be freed
 *
 * @return A value from the GCCG_INTERFACE enumeration.
 */
GCCG_INTERFACE GccgReturnStatus GccgRxFreeBuffer(const GccgBuffer *buffer);

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
