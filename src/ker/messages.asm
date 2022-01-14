%include "err.inc"


section .ctors.errf


db      ERR_FACILITY_KER,               "Kernel$"


section .ctors.errm


db      ERR_OK,                         "OK$"
db      ERR_KER_UNSUPPORTED,            "Unsupported feature requested$"
db      ERR_KER_NOT_FOUND,              "Item not found$"
db      ERR_KER_ARCHIVE_NOT_FOUND,      "Archive not found$"
db      ERR_KER_ARCHIVE_TOO_LARGE,      "Archive is too large$"
db      ERR_KER_ARCHIVE_INVALID,        "Archive is invalid$"
db      ERR_KER_INVALID_SEQUENCE,       "Invalid or unsupported UTF-8 sequence$"
