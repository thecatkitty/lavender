#ifndef _DEV_PIT_H_
#define _DEV_PIT_H_

#define INT_PIT        0x08
#define PIT_IO         0x40
#define PIT_IO_COMMAND (PIT_IO + 3)

#define PIT_CHANNEL      6
#define PIT_READ_BACK    3
#define PIT_CHANNEL_MASK 0x03

#define PIT_BYTE_HI 5
#define PIT_BYTE_LO 4

#define PIT_MODE                    1
#define PIT_MODE_INT_TERMINAL_COUNT 0
#define PIT_MODE_HW_ONE_SHOT        1
#define PIT_MODE_RATE_GEN           2
#define PIT_MODE_SQUARE_WAVE_GEN    3
#define PIT_MODE_SW_STROBE          4
#define PIT_MODE_HW_STROBE          5
#define PIT_MODE_MASK               0x07

#define PIT_BCD 0

#endif // _DEV_PIT_H_
