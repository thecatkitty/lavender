%include "vid.inc"


section .errf


db      ERR_FACILITY_VID,               "Video$"


section .errm


db      ERR_VID_UNSUPPORTED,            "Unsupported feature requested$"
db      ERR_VID_FAILED,                 "Operation failed$"
db      ERR_VID_FORMAT,                 "Improper graphics format$"
