%define PIC1_IO                         20h
%define PICx_IO_COMMAND                 0
%define PICx_IO_DATA                    1

%define PIC1_IO_COMMAND                 (PIC1_IO + PICx_IO_COMMAND)
%define PIC1_IO_DATA                    (PIC1_IO + PICx_IO_DATA)

%define PIC_COMMAND_EOI                 20h
