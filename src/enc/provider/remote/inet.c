#include <stdio.h>

#include "remote.h"
#include "json/encjson.h"

static const char *REQUEST_HEADERS =
    "Content-Type: application/x-www-form-urlencoded\r\n"
    "Accept-Types: application/json\r\n";

static char                   _payload[64];
static palinet_response_param _response;
static char                  *_response_data = NULL;
static size_t                 _response_processed;

// ----- Internet-based verification

static encui_field _inet_fields[] = {
    {ENCUIFT_LABEL, ENCUIFF_DYNAMIC, (intptr_t) "Decoy\nDecoy"},
    {ENCUIFT_LABEL, ENCUIFF_DYNAMIC, (intptr_t) "Decoy\n\nDecoy"},
    {ENCUIFT_CHECKBOX, ENCUIFF_STATIC, IDS_STOREKEY},
};

void
encr_inet_init(enc_context *enc)
{
    if (NULL == enc->parameter)
    {
        return;
    }

    encr_pages[PAGE_METHOD].fields[encr_pages[PAGE_METHOD].length++].data =
        IDS_METHOD_INET;

    encr_pages[PAGE_INET - 1].data = (void *)PAGE_METHOD;
    encr_pages[PAGE_INET].data = enc;
    encr_pages[PAGE_INET].length = lengthof(_inet_fields);
    encr_pages[PAGE_INET].fields = _inet_fields;

    if (enc_has_key_store())
    {
        _inet_fields[2].flags |= ENCUIFF_CHECKED;
    }
    else
    {
        encr_pages[PAGE_INET].length--;
    }
}

void
encr_inet_cleanup(void)
{
    palinet_close();
    free(_response_data);
    _response_data = NULL;
}

int
encr_inet_get_status(void)
{
    return _response.status;
}

static void
_inet_error(int head, intptr_t desc, bool dynamic)
{
    _inet_fields[0].data = head;
    encui_refresh_field(encr_pages + PAGE_INET, 0);

    if (dynamic)
    {
        _inet_fields[1].flags |= ENCUIFF_DYNAMIC;
    }
    else
    {
        _inet_fields[1].flags &= ~ENCUIFF_DYNAMIC;
    }
    _inet_fields[1].data = desc;
    encui_refresh_field(encr_pages + PAGE_INET, 1);

    palinet_close();
    free(_response_data);
    _response_data = NULL;
}

static int
_verify_inet_proc(int msg, void *param, void *data)
{
    switch (msg)
    {
    case PALINETM_CONNECTED: {
        _inet_fields[0].data &= ~ENCUIFF_DYNAMIC;
        _inet_fields[0].data = IDS_INET_SEND;
        encui_refresh_field(encr_pages + PAGE_INET, 0);
        break;
    }

    case PALINETM_GETHEADERS: {
        *(const char **)param = REQUEST_HEADERS;
        return 0;
    }

    case PALINETM_GETPAYLOAD: {
        uint8_t rbytes[lengthof(encr_request)];
        char   *ppayload = _payload;
        int     i;

        ppayload += sprintf(ppayload, "rc=");
        encr_encode_request(rbytes);
        for (i = 0; i < lengthof(rbytes); i++)
        {
            sprintf(ppayload, "%02x", rbytes[i]);
            ppayload += 2;
        }

        *(char **)param = _payload;
        return ppayload - _payload;
    }

    case PALINETM_RESPONSE: {
        _inet_fields[0].data = IDS_INET_RECV;
        encui_refresh_field(encr_pages + PAGE_INET, 0);

        memcpy(&_response, param, sizeof(_response));
        _response_data = (char *)malloc((size_t)_response.content_length + 1);
        if (NULL == _response_data)
        {
            // memory allocation error
            _inet_error(IDS_INET_RESPERR, IDS_LONGCONTENT, false);
            break;
        }

        _response_data[_response.content_length] = 0;
        _response_processed = 0;
        break;
    }

    case PALINETM_RECEIVED: {
        palinet_received_param *received_param =
            (palinet_received_param *)param;
        if (NULL == _response_data)
        {
            break;
        }

        if (_response.content_length <
            (_response_processed + received_param->size))
        {
            // size mismatch error
            _inet_error(IDS_INET_RESPERR, IDS_INVALIDRESP, false);
            break;
        }

        memcpy(_response_data + _response_processed, received_param->data,
               received_param->size);
        _response_processed += received_param->size;
        break;
    }

    case PALINETM_COMPLETE: {
        int         i;
        size_t      message_length = 0;
        const char *message = NULL;
        uint8_t     rbytes[lengthof(encr_request)];

        message = encjson_get_child_string(_response_data, _response_processed,
                                           "detail", &message_length);

        if (NULL != message)
        {
            memmove(_response_data, message, message_length);
            _response_data[message_length] = 0;

            if (403 == _response.status)
            {
                // forbidden
                _inet_error(IDS_INET_AUTHERR, (intptr_t)_response_data, true);
                break;
            }

            // pass through
        }

        if (200 != _response.status)
        {
            // server side error
            if (NULL == message)
            {
                _inet_error(IDS_INET_SERVERR, (intptr_t)_response.status_text,
                            true);
                break;
            }

            _inet_error(IDS_INET_SERVERR, (intptr_t)_response_data, true);
            break;
        }

        // status OK
        message = encjson_get_child_string(_response_data, _response_processed,
                                           "code", &message_length);

        if ((NULL == message) ||
            ((2 * lengthof(encr_response)) != message_length))
        {
            // code length error
            _inet_error(IDS_INET_RESPERR, IDS_INVALIDRESP, false);
            break;
        }

        // code OK
        encr_encode_request(rbytes);
        for (i = 0; i < lengthof(encr_response); i++)
        {
            encr_response[i] = xtob(message + 2 * i) ^ rbytes[i] ^
                               rbytes[i + 2] ^ rbytes[i + 4];
        }

        _inet_fields[0].data = IDS_INET_SUCCESS;
        encui_refresh_field(encr_pages + PAGE_INET, 0);
        encui_check_page(encr_pages + PAGE_INET, (void *)1);

        palinet_close();
        free(_response_data);
        _response_data = NULL;
        break;
    }

    case PALINETM_ERROR: {
        _inet_fields[0].data = IDS_INET_CONNERR;
        encui_refresh_field(encr_pages + PAGE_INET, 0);

        if (NULL != param)
        {
            _inet_fields[1].data = (intptr_t)param;
            encui_refresh_field(encr_pages + PAGE_INET, 1);
        }

        free(_response_data);
        _response_data = NULL;
        break;
    }
    }

    return -ENOSYS;
}

enum
{
    NOTIFY_START,
    NOTIFY_CONNECT,
    NOTIFY_REQUEST,
};

int
encr_inet_page_proc(int msg, void *param, void *data)
{
    switch (msg)
    {
    case ENCUIM_INIT: {
        _inet_fields[0].data = (intptr_t) "";
        _inet_fields[1].data = (intptr_t) "";
        return 0;
    }

    case ENCUIM_CHECK: {
        return NULL == param ? 1 : 0;
    }

    case ENCUIM_ENTERED: {
        encui_request_notify(NOTIFY_START);
        return 0;
    }

    case ENCUIM_NOTIFY: {
        int notify = (intptr_t)param;

        if (NOTIFY_START == notify)
        {
            _inet_fields[0].flags &= ~ENCUIFF_DYNAMIC;
            if (palinet_start())
            {
                _inet_fields[0].data = IDS_INET_CONN;
                encui_refresh_field(encr_pages + PAGE_INET, 0);
                encui_request_notify(NOTIFY_CONNECT);
            }
            else
            {
                _inet_fields[0].data = IDS_INET_INITERR;
                encui_refresh_field(encr_pages + PAGE_INET, 0);
            }

            return 0;
        }

        if (NOTIFY_CONNECT == notify)
        {
            enc_context *enc = (enc_context *)data;

            if (palinet_connect((const char *)enc->parameter, _verify_inet_proc,
                                (void *)1))
            {
                encui_request_notify(NOTIFY_REQUEST);
            }
            // palinet_connect errors handled in _verify_inet_proc

            return 0;
        }

        if (NOTIFY_REQUEST == notify)
        {
            enc_context *enc = (enc_context *)data;

            char endpoint[PATH_MAX];
            sprintf(endpoint, "%s/verify", (const char *)enc->parameter);

            palinet_request("POST", endpoint);
            // palinet_request errors handled in _verify_inet_proc

            return 0;
        }

        return 0;
    }

    case ENCUIM_NEXT: {
        encr_store = ENCUIFF_CHECKED & _inet_fields[2].flags;
        return __enc_decrypt_content((enc_context *)data);
    }
    }

    return -ENOSYS;
}
