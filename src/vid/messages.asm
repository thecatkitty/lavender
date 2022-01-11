%include "vid.inc"


section .ctors.errf


db      ERR_FACILITY_VID,               "Video$"


section .ctors.errm


db      ERR_VID_UNSUPPORTED,            "Unsupported feature requested$"
db      ERR_VID_FAILED,                 "Operation failed$"
db      ERR_VID_FORMAT,                 "Improper graphics format$"
