# Ground-Cloud-Cloud-Ground (GCCG) Transport API

This API is part of the VSF Technical Recommendation TR-xx Signal Transport and Timing Considerations for Ground-Cloud-Cloud-Ground workflows.

## Introduction

The recommendation concerns the interchange and timing of signals to-, from-, and within an interconnected composition of media processes within a composable compute environment (either private or multi-tenant, i.e. “cloud”).  In such systems, the time consumed performing a given process might be highly variable - we refer to these software-abstracted media processes here as  “Time-Non-Linear” Workflow Steps.
Additionally, this recommendation concerns the transport and interchange of media signals between Time-Linear (traditional method) and Time-Non-Linear processing paradigms, in order to facilitate the integration of real-time production media signals into composable compute environments.
It builds on (and references) other work within the VSF on related subjects.  A novel timing model is introduced for managing buffering and latency in the Time-Non-Linear processing paths.

The rest of this document describes some of the details of the API.

# API Overview

The API consists of public API data types, structures and functions. A "connection" is considered a single flow that may contain one or more media elements (video, audio and ancillary data). The diagram below shows an overview of the API usage flow:

![diagram](flow_diagram.jpg)

# JSON strings

JSON is used by the API functions in order to pass configuration information.

## Create Connection APIs

The ```GccgTxConnectionCreate()``` and ```GccgRxConnectionCreate()``` API functions are used to create transmit and receive connections. JSON is used to pass parameters to the API and return information that is specific to the connection.


### ```connection_json_ptr```

This parameter points to a JSON string that use used to configure a new connection. Below is an example. For clarity, double hashes have been used to add in-line comments.
```
{
  "profileVersion": "01.00",  ## Version of this JSON ##
  "timing": {         ## Note: These values must not change over the lifetime of the connection except as noted below. ##
    "GMID": 12345678, ## 64-bit Grandmaster Clock Identifier. ##
    "COT": 12345678,  ## 64-bit Content Origination Timestamp. Upper 32-bits is the number of seconds since the SMPTE ##
                      ## Epoch. Lower 32-bits is the number of fractional seconds as measured in nanoseconds. ##
    "LAT": 12345678,  ## 64-bit Local Arrival Timestamp in same format as COT. ##
    "tMinAccumulated": 100, ## Accumulated minimum latency of the Workflow path up to this Workflow Step, in milliseconds. ##
                            ## Can change but the change is disruptive to the Workflow timing while the Workflow adapts. ##
    "t99Accumulated": 200   ## Accumulated maximum latency of the Workflow path up to this Workflow Step, in milliseconds. ##
                            ## Can change but the change is disruptive to the Workflow timing while the Workflow adapts. ##
  },

  ## A Media Flow containing one or more Media Elements that belong to the same media essence/stream. ##
  "mediaFlow": {
    "flowIdentifier": 12345678, ## Identifier used to associate this media with a flow. ##
    ## Array of Media Elements, containing one or more of the following media types: ##
    "mediaElement": [
      {
        "type": "video",
        "level": "1080p60"     ## 1080p30, 1080p60, UHD-1, UHD-2, HFR? ##
        "encodingName": "raw", ## Video options are from the IANA registered video media types. ##
                               ## See https://www.iana.org/assignments/media-types/media-types.xhtml#video ##
        "elementIdentifier": 12345670, ## Identifier used to uniquely identify this Media Element within the Media Flow. ##
        "attributes": {
          "fmtp": {
            "sampling": "YCbCr-4:2:2",
            "depth": 10,
            "width": 1920,
            "height": 1080,
            "exactframerate": "60000/1001",
            "colorimetry": "BT709",
            "interlace": false,  ## Type of video. true= interlaced, false= progressive. ##
            "firstField": true,  ## If interlaced, defines field. true= first field, false= second field. ##
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
        "encodingName": "pcm",      ## Audio options are: "st2110-31" or "pcm" ##
        "elementIdentifier": 12345671, ## Identifier used to uniquely identify this Media Element within the Media Flow. ##
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
          "encodingName": "rfc8331",## The only ancillary-data option is "rfc8331". ##
          "elementIdentifier": 12345672, ## Identifier used to uniquely identify this Media Element within the Media Flow. ##
          "packetCount": 100,       ## Number of ANC packets being transported. If there is no ANC data to be transmitted ##
                                    ## in a given period, the header shall still be sent in a timely manner indicating a ##
                                    ## count of zero. ##
          "interlace": false,       ## Type of video. true= interlaced, false= progressive. ##
          "evenField": true,        ## If interlace, defines field. true= even field, false= odd field. ##
          "lumaChannel": false,     ## Whether the ANC data corresponds to the luma (Y) channel or not. ##
          "lineNumber": 10,         ## Optional. The interface line number of the ANC data (in cases where legacy location is not ##
                                    ## required, users are encouraged to use the location-free indicators specified in RFC8331). ##
          "DID", 0                  ## Data Identifier Word that indicates the type of ancillary data that the packet corresponds to. ##
          "SDID", 0,                ## Secondary Data Identifier (8-bit value). Valid if DID is less than 128. ##
          "dataWordCount": 10       ## Number of data words for each ANC packet. ##
          ## Note: The horizontal offset and stream number, which are present in the RFC, are not used here. ##
        }
      ]
    }
  }
}
```

### ```ret_connection_json_str```

This parameter points to where returned connection data should be written in the form of a JSON string. Below is an example of a returned JSON string. For clarity, double hashes have been used to add in-line comments.

```
{
  "profileVersion": "01.00",  ## Version of this JSON ##

  "timing": {
    "tMin": 60,      ## Minimum latency of the Workflow Step in milliseconds. ##
    "t99": 120,      ## Maximum latency of the Workflow Step in milliseconds. ##
    "tMinAccumulated": 160, ## Accumulated minimum latency of the Workflow path up to this Workflow Step's output. ##
    "t99Accumulated": 320   ## Accumulated maximum latency of the Workflow path up to this Workflow Step's output. ##
  }
}
```

## Payload APIs

The ```GccgRxCallback()``` callback function is invoked when a payload has been received. The ```GccgTxPayload()``` function is used to transmit a payload to a remote receiver.

## ```payload_json_str```

This parameter points to a JSON string that is used for informational purposes when transmitting and receiving payloads. When transmitting, it can be use to define configurable changes to a payload. Below is an example of a payload JSON string. For clarity, double hashes have been used to add in-line comments.

```
{
  "profileVersion": "01.00",  ## Version of this JSON ##
  "timing" : []               ## Same as the connection's timing array. ##
  "media": []                 ## Same as connection's media array. ##
}
```

# Uncompressed Video Data Format

Raw (uncompressed) video data is stored in pgroup format as defined in ST2110-20. Note: For interlaced video the fields shall be transmitted in time order, first field first. An example of a 5 Octet 4:2:2 10-bit pgroup is shown below:
```
 0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ 
 |   C’B (10 bits)   |   Y0’ (10 bits)   |   C’R (10 bits)   |   Y1’ (10 bits)   |
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
```
 
The value "raw" must be used for the JSON configuration "encodingName" element as shown below:
```
 "encodingName": "raw"
```

# Compressed Video Data Formats

They shall be indentified by using the Internet Assigned Numbers Authority (IANA) video name strings, which can be found at https://www.iana.org/assignments/media-types/media-types.xhtml#video, for the JSON configuration "encodingName" element.
 
An example for H.264 compressed video is shown below:

```
"encodingName": "H264"
```

# Uncompressed Audio Format

32-bit PCM (uncompressed) audio data is stored in the following format:
```
             +-----------------------+------------+------------+------------------------+
 1 sample:   | most significant byte |   byte 2   |   byte 1   | least significant byte |
             +-----------------------+------------+------------+------------------------+
```

Audio samples with multiple channels are interleaved. An example using multiple channels is shown below:
```
  +--------------------+--------------------+--------------------+--------------------+
  | sample 0 channel 0 | sample 0 channel 1 | sample 0 channel 2 | sample 0 channel 3 |
  +--------------------+--------------------+--------------------+--------------------+
  | sample 1 channel 0 | sample 1 channel 1 | sample 1 channel 2 | sample 1 channel 3 |
  +--------------------+--------------------+--------------------+--------------------+
                                           ...
  +--------------------+--------------------+--------------------+--------------------+
  | sample N channel 0 | sample N channel 1 | sample N channel 2 | sample N channel 3 |
  +--------------------+--------------------+--------------------+--------------------+
```

# Ancillary Data Format
Ancillary packet data is based on the packing model of RFC 8331, as shown below:
```
   0                   1                   2                   3
   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |           ANC_Count           | F |         reserved          |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
```

The section below is repeated once for each ancillary data packet, as specified by ANC_Count.
```
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |C|   Line_Number       |   Horizontal_Offset   |S|  StreamNum  |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |         DID       |        SDID       |   Data_Count      |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
                           User_Data_Words...
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
                                  |   Checksum_Word   |word_align |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
```
