#ifndef _PAL_DOS_HW_H_
#define _PAL_DOS_HW_H_

// Programmable Interrupt Controller definitions
#define PIC1_IO         0x20
#define PICx_IO_COMMAND 0
#define PICx_IO_DATA    1

#define PIC1_IO_COMMAND (PIC1_IO + PICx_IO_COMMAND)
#define PIC1_IO_DATA    (PIC1_IO + PICx_IO_DATA)

#define PIC_COMMAND_EOI 0x20

#define INT_PIT        0x08
#define PIT_IO         0x40
#define PIT_DATA(x)    (PIT_IO + (x))
#define PIT_IO_COMMAND (PIT_IO + 3)

// Programmable Interval Timer definitions
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

// 1193181.6667 Hz ~= 1193181 + 683/1024 ~= 1221818027/1024
#define PIT_INPUT_FREQ_F10 1221818027ULL
#define PIT_FREQ_POWER     11
#define PIT_FREQ_DIVISOR   (1ULL << PIT_FREQ_POWER)

// PC Speaker definitions
#define SPKR_ENABLE 3

extern void
pit_initialize(void);

extern void
pit_cleanup(void);

extern void
pit_init_channel(unsigned channel, unsigned mode, unsigned divisor);

#endif // _PAL_DOS_HW_H_
