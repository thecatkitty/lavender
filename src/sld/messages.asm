%include "sld.inc"


section .ctors.errf


db      ERR_FACILITY_SLD,               "Slides$"


section .ctors.errm


db      ERR_SLD_INVALID_DELAY,          "Invalid delay$"
db      ERR_SLD_UNKNOWN_TYPE,           "Unknown type$"
db      ERR_SLD_INVALID_VERTICAL,       "Invalid vertical position$"
db      ERR_SLD_INVALID_HORIZONTAL,     "Invalid horizontal position$"
db      ERR_SLD_CONTENT_TOO_LONG,       "Content too long$"
