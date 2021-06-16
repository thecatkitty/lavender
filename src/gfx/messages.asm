%include "gfx.inc"


section .errf


db      ERR_FACILITY_GFX,               "Graphics$"


section .errm


db      ERR_GFX_FORMAT,                 "Unsupported format$"
db      ERR_GFX_MALFORMED_FILE,         "Malformed file$"
