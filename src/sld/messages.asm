%include "sld.inc"


section .errf


db      ERR_FACILITY_SLD,               "Slides$"


section .errm


db      ERR_SLD_INVALID_DELAY,          "Invalid delay$"
db      ERR_SLD_UNKNOWN_TYPE,           "Unknown type$"
db      ERR_SLD_INVALID_VERTICAL,       "Invalid vertical position$"
db      ERR_SLD_INVALID_HORIZONTAL,     "Invalid horizontal position$"
