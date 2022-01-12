#ifndef _FMT_EDID_H_
#define _FMT_EDID_H_

#include <stdint.h>

#define EDID_MANUFACTURER_ID1      10 // ManufacturerId
#define EDID_MANUFACTURER_ID2      5  //   -111 1122  2223 3333
#define EDID_MANUFACTURER_ID3      0
#define EDID_MANUFACTURER_IDx_MASK 0b11111

#define EDID_INPUT_TYPE            7 // InputType (digital)
#define EDID_INPUT_TYPE_ANALOG     0 //   TDDD IIII
#define EDID_INPUT_TYPE_DIGITAL    1
#define EDID_INPUT_TYPE_MASK       0b1
#define EDID_INPUT_DEPTH           4
#define EDID_INPUT_DEPTH_6B        1
#define EDID_INPUT_DEPTH_8B        2
#define EDID_INPUT_DEPTH_10B       3
#define EDID_INPUT_DEPTH_12B       4
#define EDID_INPUT_DEPTH_14B       5
#define EDID_INPUT_DEPTH_16B       6
#define EDID_INPUT_DEPTH_MASK      0b111
#define EDID_INPUT_INTERFACE       0
#define EDID_INPUT_INTERFACE_HDMIA 2
#define EDID_INPUT_INTERFACE_HDMIB 3
#define EDID_INPUT_INTERFACE_MDDI  4
#define EDID_INPUT_INTERFACE_DP    5
#define EDID_INPUT_INTERFACE_MASK  0b1111

#define EDID_INPUT_LEVELS           5 // InputType (analog)
#define EDID_INPUT_LEVELS_0700_0300 0 //   -LLB SCGV
#define EDID_INPUT_LEVELS_0714_0286 1
#define EDID_INPUT_LEVELS_1000_0400 2
#define EDID_INPUT_LEVELS_0700_0000 3
#define EDID_INPUT_LEVELS_MASK      0b11
#define EDID_INPUT_BLANK_TO_BLACK   4
#define EDID_INPUT_SEPARATE_SYNC    3
#define EDID_INPUT_COMPOSITE_SYNC   2
#define EDID_INPUT_SYNC_ON_GREEN    1
#define EDID_INPUT_VSYNC_SERRATED   0

#define EDID_FEATURE_STANDBY       7 // Features (digital)
#define EDID_FEATURE_SUSPEND       6 //   SUA2 4RDC
#define EDID_FEATURE_ACTIVEOFF     5
#define EDID_FEATURE_YCRCB422      4
#define EDID_FEATURE_YCRCB444      3
#define EDID_FEATURE_TYPE          3 // Features (analog)
#define EDID_FEATURE_TYPE_MONO     0 //   SUAT TRDC
#define EDID_FEATURE_TYPE_RGB      1
#define EDID_FEATURE_TYPE_NONRGB   2
#define EDID_FEATURE_TYPE_MASK     0b11
#define EDID_FEATURE_SRGB          2
#define EDID_FEATURE_PREFERRED_DT1 1
#define EDID_FEATURE_CONTINUOUS    0

#define EDID_TIMING_ASPECT       14 // StandardTiming
#define EDID_TIMING_ASPECT_16_10 0  //   AAVV VVVV XXXX XXXX
#define EDID_TIMING_ASPECT_4_3   1
#define EDID_TIMING_ASPECT_5_4   2
#define EDID_TIMING_ASPECT_16_9  3
#define EDID_TIMING_ASPECT_MASK  0b11
#define EDID_TIMING_VFREQ        8
#define EDID_TIMING_VFREQ_MASK   0b111111
#define EDID_TIMING_XRES         0
#define EDID_TIMING_XRES_MASK    0b11111111

typedef struct
{
    uint8_t  Padding[8];
    uint16_t ManufacturerId;
    uint16_t ModelId;
    uint32_t SerialNumber;
    uint8_t  Week;
    uint8_t  Year;
    uint8_t  EdidVersion;
    uint8_t  EdidRevision;
    uint8_t  InputType;
    uint8_t  HorizontalSize;
    uint8_t  VerticalSize;
    uint8_t  Gamma;
    uint8_t  Features;
    uint8_t  Chroma[10];
    uint8_t  EstablishedModes[3];
    uint16_t StandardTiming[8];
    uint8_t  DetailedTiming1[18];
    uint8_t  DetailedTiming2[18];
    uint8_t  DetailedTiming3[18];
    uint8_t  DetailedTiming4[18];
    uint8_t  Extensions;
    uint8_t  Checksum;
} EDID;

#endif // _FMT_EDID_H_
