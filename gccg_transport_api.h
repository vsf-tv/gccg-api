/*

BSD-2-Clause

Copyright Amazon.com Inc. or its affiliates

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the
following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following
disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following
disclaimer in the documentation and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#ifndef GCCG_TRANSPORT_API_H__
#define GCCG_TRANSPORT_API_H__

/**
 * @file
 * @brief
 * This file declares the public API data types, structures and functions that comprise the GCCG transport API. Each
 * connection is considered a single flow that may contain one or more media elements (video, audio and ancillary data).
 **/

/**
 * connection_json_ptr : A JSON string used to configure a connection. It is used by the GccgTxConnectionCreate() and
 * GccgRxConnectionCreate() API functions.
 *
 * The JSON shown below is an example of a connection JSON string.
{
  ## Values that are valid for both source and destination connections. ##
  "profileVersion": "01.00",  ## Version of this JSON ##
  "protocol": "cdi",          ## TODO Other types: "rtp", "tcp", "ndi", "srt", "socket", "other". Platform specific? ##
  "bandwidth": 14000000,      ## Maximum required bandwidth for the connection. ##

  "timing": {         ## Note: These values should not change over the lifetime of the connection. ##
    "GMID": 12345678, ## 64-bit Grandmaster Clock Identifier ##
    "COT": 12345678,  ## 64-bit Content Origination Timestamp. Upper 32-bits is the number of seconds since the SMPTE
                      ## Epoch. Lower 32-bits is the number of fractional seconds as measured in nanoseconds. ##
    "LAT": 12345678,  ## 64-bit Local Arrival Timestamp in same format as COT. ##
    "tMin": 100,      ## Minimum latency of the Workflow Step in milliseconds. ##
    "t99": 200        ## Maximum latency of the Workflow Step in milliseconds. ##
  },

  ## Destination is only valid for Tx connections. ##
  ## Depending on protocol, one or more destination IP, port and bind addresses. ##
  "destination": [
    {
      "ip": "127.0.0.1",          ## Destination IP address to send to ###
      "port": 3000,               ## Port to send to ##
      "bindAddress": "127.0.0.1"  ## Local interface to use ##
      ## TODO Other values needed for specific protocol types ##
    }
  ]

  ## Source is only valid for Rx connections. ##
  "source": {
      "port": 3000          ## Source port to listen to ##
      "filter": "127.x.x.x" ## Optional source filter ##
      ## TODO Other values needed for specific protocol types ##
  }

  ## Array of media, containing one or more of the following media types: ##
  "media": [
    {
      "type": "video",
      "level": "1080p60"     ## 1080p30, 1080p60, UHD-1, UHD-2, HFR? ##
      "encodingName": "raw", ## raw (uncompressed), jxs (JPEG XS compressed), etc.
      "attributes": {
        "fmtp": {
          "sampling": "YCbCr-4:2:2",
          "depth": 10,
          "width": 1920,
          "height": 1080,
          "exactframerate": "60000/1001",
          "colorimetry": "BT709",
          "interlace": false,       ## Type of video. true= interlaced, false= progressive. ##
          "evenField": true,        ## If interlace, defines field. true= even field, false= odd field. ##
          "segmented": false,
          "TCS": "SDR",
          "RANGE": "NARROW",
          "PAR": "12:13",
          "alphaIncluded": false,
          "partialFrame": {
            "width": 32,
            "height": 32,
            "hOffset": 132,
            "vOffset": 132
          },
        },
      },
    },
    {
      "type": "audio",
      "encodingName": "pcm", ## Options are: "st2110-31" or "pcm" ##
      "attributes": {
          "totalChannels": 4      ## Total number of channels. Fixed for lifetime of connection. ##
          "activeChannels": 4     ## Total number of active channels. Can vary, but cannot exceed totalChannels. ##
          "channelOrder": "SMPTE2110.(SGRP)", ## Channel order string. ##
          "language": "EN",       ## Language code. ##
          "samplingRate": 48,     ## Sampling rate in Khz. Fixed for lifetime of connection. ##
          "originalBitDepth": 24, ## Original bit depth of the samples. ##
          "sampleCount": 100      ## Number of samples included in each channel. ##
        },
    },
    {
      "type": "ancillary-data",
      "encodingName": "rfc8331",
      "packetCount": 100,       ## Number of ANC packets being transported. If there is no ANC data to be transmitted
                                ## in a given period, the header shall still be sent in a timely manner indicating a
                                ## count of zero. ##
      "interlace": false,       ## Type of video. true= interlaced, false= progressive. ##
      "evenField": true,        ## If interlace, defines field. true= even field, false= odd field. ##
      "lumaChannel": false,     ## Whether the ANC data corresponds to the luma (Y) channel or not. ##
      "lineNumber": 10,         ## Optional. The interface line number of the ANC data (in cases where legacy location is not
                                ## required, users are encouraged to use the location-free indicators specified in RFC8331). ##
      "DID", 0                  ## Data Identifier Word that indicates the type of ancillary data that the packet corresponds to. ##
      "SDID", 0,                ## Secondary Data Identifier (8-bit value). Valid if DID is less than 128. ##
      "dataWordCount": 10       ## Number of data words for each ANC packet. ##
      ## Note the horizontal offset and stream number, which are present in the RFC, are not used here. ##
    }
  ]
}
**/

/**
 * Raw (uncompressed) video data is stored in pgroup format as defined in ST2110-20. Note: For interlaced video
 * the fields shall be transmitted in time order, first field first. An example of a 5 Octet 4:2:2 10-bit pgroup
 * is shown below:
 *
 *  0                   1                   2                   3
 *  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ 
 * |   C’B (10 bits)   |   Y0’ (10 bits)   |   C’R (10 bits)   |   Y1’ (10 bits)   |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */

/**
 * 32-bit PCM audio data is stored in the following format:
 *             +-----------------------+------------+------------+------------------------+
 * 1 sample:   | most significant byte |   byte 2   |   byte 1   | least significant byte |
 *             +-----------------------+------------+------------+------------------------+
 *
 * Audio samples with multiple channels are interleaved. An example using 4 channels is shown below:
 *  +--------------------+--------------------+--------------------+--------------------+
 *  | sample 0 channel 0 | sample 0 channel 1 | sample 0 channel 2 | sample 0 channel 3 |
 *  +--------------------+--------------------+--------------------+--------------------+
 *  | sample 1 channel 0 | sample 1 channel 1 | sample 1 channel 2 | sample 1 channel 3 |
 *  +--------------------+--------------------+--------------------+--------------------+
 *                                           ...
 *  +--------------------+--------------------+--------------------+--------------------+
 *  | sample N channel 0 | sample N channel 1 | sample N channel 2 | sample N channel 3 |
 *  +--------------------+--------------------+--------------------+--------------------+
 **/

/**
 * Ancillary packet data is based on the packing model of RFC 8331.
 * 
 *   0                   1                   2                   3
 *   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |           ANC_Count           | F |         reserved          |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 * The section below is repeated once for each ancillary data packet, as specified by ANC_Count.
 *
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |C|   Line_Number       |   Horizontal_Offset   |S|  StreamNum  |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |         DID       |        SDID       |   Data_Count      |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *                           User_Data_Words...
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *                                  |   Checksum_Word   |word_align |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 **/

/**
 * payload_json_ptr : A JSON string used for informational purposes when transmitting and receiving payloads. When
 * transmitting, it can be use to define configurable changes to a payload as described below. It is used by the
 * GccgTxPayload() API function and the GccgRxCallback() callback function.
 *
 * The JSON shown below is an example of a payload JSON string.
{
  "profileVersion": "01.00",  ## Version of this JSON ##
  "timing" : []               ## Same as the connection's timing array. ##
  "media": []                 ## Same as connection's media array. ##
}
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
    kGccgStatusError             = 3
} GccgReturnStatus;

/// @brief A structure for holding a PTP timestamp defined in seconds and nanoseconds. This PTP time as defined by
/// SMPTE ST 2059-2 and IEEE 1588-2008 with the exception that the seconds field is an unsigned 32 bit integer instead
/// of the specified 48 bit integer.
typedef struct GccgPtpTimestamp {
    /// The number of seconds since the SMPTE Epoch which is 1970-01-01T00:00:00.
    uint32_t seconds;
    /// The number of fractional seconds as measured in nanoseconds. The value in this field is always less than 10^9.
    uint32_t nanoseconds;
} GccgPtpTimestamp;

typedef struct GccgSglEntry GccgSglEntry;
/**
 * @brief This structure represents a single, contiguous region of memory as part of a scatter-gather list.
 */
struct GccgSglEntry {
    /// @brief The starting address of the data.
    void* address_ptr;

    /// @brief The size of the data in bytes.
    int size_in_bytes;

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
    /// @brief Origination timestamp to associate with the SGL. This timestamp is a PTP timestamp as outlined
    /// by SMPTE ST 2059-2. The one exception is the seconds field is stored as an unsigned 32 bit integer instead of
    /// the specified unsigned 48 bit integer.
    GccgPtpTimestamp origination_ptp_timestamp;

    /// @brief Total size of data in the list, in units of bytes. This value can be calculated by walking the sgl_array,
    /// but is provided here for convenience and efficiency. NOTE: This value must be the same as the value calculated
    /// from walking the list and summing the size_in_bytes for each GccgSglEntry.
    int total_data_size;

    /// @brief Pointer to the first entry in the singly-linked list of SGL entries.
    GccgSglEntry* sgl_head_ptr;

    /// @brief Pointer to the last entry in the singly-linked list of SGL entries.
    GccgSglEntry* sgl_tail_ptr;

    /// @brief Handle to internal data used within the SDK that relates to this SGL. Do not use or modify this value.
    void* internal_data_ptr;
} GccgSgList;

/**
 * @brief Type used to define the size and location of a media element.
 */
typedef struct {
    /// @brief Number of media elements in media_array.
    /// Note: This value must match the number of media elements configured when the connection was created using one
    /// of the ...ConnectionCreate() API functions and cannot change. If a payload does not contain one or more media
    /// elements, than the respective pointer(s) in sql_array must be NULL.
    int count;

    GccgSgList* sgl_array; ///< Pointer to start of the media element SGL array.
} GccgMediaElements;

/**
 * @brief Type used as the handle (pointer to an opaque structure) for a transmitter or receiver connection. Each handle
 * represents a single data flow.
 */
typedef struct void* GccgConnectionHandle;

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
 * @param data_ptr A pointer to an GccgTxCbData structure.
 */
typedef void (*GccgTxCallback)(const GccgTxCbData* data_ptr);

/**
 * @brief A structure of this type is passed as the parameter to GccgRxCallback(). It contains a single payload sent
 * from a transmitter and data related to the Rx connection.
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
 * @param data_ptr A pointer to an GccgRxData structure.
 */
typedef void (*GccgRxCallback)(const GccgRxCbData* data_ptr);

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
 * @param ret_handle_ptr Pointer to returned connection handle. The handle is used as a parameter to other API functions
 *                       to identify this specific transmitter.
 *
 * @return A value from the GccgReturnStatus enumeration.
 */
GCCG_INTERFACE GccgReturnStatus GccgTxConnectionCreate(const char *connection_json_str,
                                                       GccgTxCallback tx_cb_ptr,
                                                       void* user_cb_param_ptr,
                                                       GccgConnectionHandle* ret_handle_ptr);

/**
 * Create an instance of a receiver. When the instance is no longer needed, use the GccgConnectionDestroy()
 * API function to free-up resources that are being used by it.
 *
 * @param connection_json_ptr Pointer to connection configuration data in json format.
 *                            Note: The number and ordering of media elements declared in the JSON defines media_count
 *                            and the ordering in media_array when the GccgRxCallback function is invoked.
 *                            The remote host must use the same configuration data when calling the
 *                            GccgTxConnectionCreate() API function to create the transmit side of the connection.
 * @param use_linear_buffer If true, received payload data will be stored in a linear buffer. Otherwise, depending on
 *                          the underlying transport, payload data may be stored in a scatter-gather list of buffers.
 * @param rx_cb_ptr Address of the user function to call whenever a payload has been received.
 * @param user_cb_param_ptr User defined callback parameter. This value is set as part of the GccgRxCallback data
 *                          whenever the rx_cb_ptr callback function is invoked.
 * @param ret_handle_ptr Pointer to returned connection handle. The handle is used as a parameter to other API functions
 *                       to identify this specific receiver.
 *
 * @return A value from the GccgReturnStatus enumeration.
 */
GCCG_INTERFACE GccgReturnStatus GccgRxConnectionCreate(const char *connection_json_str,
                                                       bool use_linear_buffer,
                                                       GccgRxCallback rx_cb_ptr,
                                                       void* user_cb_param_ptr,
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
 * @param payload_json_ptr Pointer to payload configuration json string.
 * @param media_array Array of SGL's that define the size and location of each media element to transmit in this
 *                    payload. If a pointer within the array is NULL, then the payload does not contain an element for
 *                    that media.
 * @param timeout_microsecs Timeout period in microseconds. If the payload is not transmitted within this period,
 *                          transmission is canceled and the GccgTxCallback API function invoked with
 *                          kGccgStatusTimeoutExpired returned as the status_code in GccgTxCbData.
 *
 * @return A value from the GCCG_INTERFACE enumeration.
 */
GCCG_INTERFACE GccgReturnStatus GccgTxPayload(GccgConnectionHandle handle,
                                              const char *payload_json_ptr,
                                              GccgMediaElements media_array,
                                              int timeout_microsecs);

/**
 * Free the receive buffer that was used by the GccgRxCallback() callback function.
 *
 * @param media_array
 *
 * @return A value from the GCCG_INTERFACE enumeration.
 */
GCCG_INTERFACE GccgReturnStatus GccgRxFreeBuffer(GccgMediaElements *media_array);

#endif // GCCG_TRANSPORT_API_H__