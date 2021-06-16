%define INT_PIT                         08h
%define PIT_IO                          40h
%define PIT_IO_COMMAND                  (PIT_IO + 3)

%define PIT_CHANNEL                     6
%define PIT_READ_BACK                   3
%define PIT_CHANNEL_MASK                11b

%define PIT_BYTE_HI                     5
%define PIT_BYTE_LO                     4

%define PIT_MODE                        1
%define PIT_MODE_INT_TERMINAL_COUNT     0
%define PIT_MODE_HW_ONE_SHOT            1
%define PIT_MODE_RATE_GEN               2
%define PIT_MODE_SQUARE_WAVE_GEN        3
%define PIT_MODE_SW_STROBE              4
%define PIT_MODE_HW_STROBE              5
%define PIT_MODE_MASK                   111b

%define PIT_BCD                         0
