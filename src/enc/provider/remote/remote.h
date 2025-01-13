#ifndef _ENCR_REMOTE_H_
#define _ENCR_REMOTE_H_

#include "../../../resource.h"
#include "../../enc_impl.h"
#include "../../ui/encui.h"

enum
{
    PAGE_PKEY,
    PAGE_METHOD,
    PAGE_RCODE = PAGE_METHOD + 2,
    PAGE_QR = PAGE_RCODE + 2,
#if defined(CONFIG_INTERNET)
    PAGE_INET = PAGE_QR + 2,
#endif
    PAGE_LAST
};

#define ENCR_REQUEST_SIZE  18
#define ENCR_RESPONSE_SIZE 14

extern encui_page         encr_pages[PAGE_LAST + 1];
extern encui_textbox_data encr_ccode_texbox;
extern uint8_t            encr_request[ENCR_REQUEST_SIZE];
extern uint8_t            encr_response[ENCR_RESPONSE_SIZE];
extern bool               encr_store;

extern void
encr_encode_request(uint8_t *out);

extern unsigned long
encr_decimal_complement(unsigned long n);

extern int
encr_ccode_page_proc(int msg, void *param, void *data);

extern void
encr_rcode_init(enc_context *enc);

extern int
encr_rcode_page_proc(int msg, void *param, void *data);

extern void
encr_qr_init(enc_context *enc);

extern void
encr_qr_enter(void *data);

extern int
encr_qr_page_proc(int msg, void *param, void *data);

#if defined(CONFIG_INTERNET)
extern void
encr_inet_init(enc_context *enc);

extern void
encr_inet_cleanup(void);

extern int
encr_inet_get_status(void);

extern int
encr_inet_page_proc(int msg, void *param, void *data);
#endif

#endif
