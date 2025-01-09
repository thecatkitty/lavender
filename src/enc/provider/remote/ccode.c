#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#include "remote.h"

static char        _ccode[50];
encui_textbox_data encr_ccode_texbox = {_ccode, sizeof(_ccode), 0};

int
encr_ccode_page_proc(int msg, void *param, void *data)
{
    switch (msg)
    {
    case ENCUIM_CHECK: {
        int i;

        const char *ccode = (const char *)param;
        if (NULL == ccode)
        {
            return 1;
        }

        if (48 != strlen(ccode))
        {
            return 1;
        }

        for (i = 0; i < 48; i++)
        {
            if (6 == (i % 7))
            {
                if ('-' != ccode[i])
                {
                    return 1;
                }
            }
            else if (!isdigit(ccode[i]))
            {
                return 1;
            }
        }

        return 0;
    }

    case ENCUIM_NEXT: {
        int         i;
        const char *ccode = (const char *)param;
        uint16_t   *cwords = (uint16_t *)encr_response;
        uint8_t     rbytes[lengthof(encr_request)];

        for (i = 0; i < lengthof(encr_response) / 2; i++)
        {
            uint32_t group = atol(ccode + i * 7);
            if (0 != encr_decimal_complement(group))
            {
                char  fmt[GFX_COLUMNS * 2];
                char *msg;
                int   length;

                pal_load_string(IDS_INVALIDGROUP, fmt, lengthof(fmt));
                length = snprintf(NULL, 0, fmt, i + 1);
                msg = malloc(length + 1);
                snprintf(msg, length + 1, fmt, i + 1);
                ((encui_textbox_data *)encr_pages[PAGE_RCODE].fields[5].data)
                    ->alert = msg;
                return INT_MAX;
            }

            cwords[i] = (uint16_t)(group % 100000);
        }

        encr_encode_request(rbytes);
        for (i = 0; i < lengthof(encr_response); i++)
        {
            encr_response[i] ^= rbytes[i] ^ rbytes[i + 2] ^ rbytes[i + 4];
        }

        return __enc_decrypt_content((enc_context *)data);
    }
    }

    return -ENOSYS;
}
