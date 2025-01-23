#include <stdio.h>

#include <windows.h>
#include <wininet.h>

#include <net.h>
#include <pal.h>

static HINTERNET session_ = NULL;
static HINTERNET connection_ = NULL;
static HINTERNET request_ = NULL;
static net_proc *proc_ = NULL;
static DWORD_PTR data_ = 0;

bool
net_start(void)
{
    char  user_agent[MAX_PATH] = "";
    char *ptr = NULL;

    if (NULL != session_)
    {
        return true;
    }

    strcpy(user_agent, pal_get_version_string());
    ptr = strrchr(user_agent, ' ');
    if (ptr)
    {
        *ptr = '/';
    }

    strcat(user_agent, " (");
    ptr = user_agent + strlen(user_agent);

    {
        DWORD version = GetVersion();
        if (0x80000000 & version)
        {
            ptr += sprintf(ptr, "Windows");
        }
        else if (10 == LOBYTE(version))
        {
            ptr += sprintf(ptr, "Windows NT 10.0.%u", HIWORD(version));
        }
        else
        {
            ptr += sprintf(ptr, "Windows NT %u.%u", LOBYTE(LOWORD(version)),
                           HIBYTE(LOWORD(version)));
        }
    }

    strcpy(ptr, "; ");
    ptr += 2;
    {
#if defined(_M_IX86)
        ptr += sprintf(ptr, "ia32");
#elif defined(_M_X64)
        ptr += sprintf(ptr, "x64");
#elif defined(_M_IA64)
        ptr += sprintf(ptr, "ia64");
#elif defined(_M_ARM)
        ptr += sprintf(ptr, "arm");
#elif defined(_M_ARM64)
        ptr += sprintf(ptr, "arm64");
#else
#error "Unknown architecture!"
#endif
    }

    strcpy(ptr, "; ");
    ptr += 2;
    {
        LCID locale = GetThreadLocale();
#if WINVER >= 0x0600
        WCHAR locale_name[LOCALE_NAME_MAX_LENGTH];
        if (0 <
            LCIDToLocaleName(locale, locale_name, LOCALE_NAME_MAX_LENGTH, 0))
        {
            ptr += sprintf(ptr, "%S", locale_name);
        }
#else
        switch (PRIMARYLANGID(locale))
        {
        case LANG_CZECH:
            ptr += sprintf(ptr, "cs");
            break;
        case LANG_ENGLISH:
            ptr += sprintf(ptr, "en");
            break;
        case LANG_POLISH:
            ptr += sprintf(ptr, "pl");
            break;
        default:
            ptr += sprintf(ptr, "x-lcid-%04lx", locale);
        }
#endif
    }

    strcpy(ptr, ")");

    session_ = InternetOpenA(user_agent, INTERNET_OPEN_TYPE_PRECONFIG, NULL,
                             NULL, INTERNET_FLAG_ASYNC);
    return NULL != session_;
}

void
net_stop(void)
{
}

static char *
format_message(DWORD error)
{
    char  *msg = NULL;
    LPWSTR wmsg = NULL;
    DWORD  length = 0;
    FormatMessageW(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_HMODULE,
        GetModuleHandleW(L"wininet.dll"), error, 0, (LPWSTR)&wmsg, 0, NULL);

    length = WideCharToMultiByte(CP_UTF8, 0, wmsg, -1, NULL, 0, NULL, NULL);
    msg = (char *)malloc(length);
    if (NULL == msg)
    {
        return NULL;
    }
    WideCharToMultiByte(CP_UTF8, 0, wmsg, -1, msg, length, NULL, NULL);

    LocalFree(wmsg);
    return msg;
}

static void
set_error(net_proc *proc, DWORD error, void *data)
{
    char *msg = format_message(error);
    proc(NETM_ERROR, msg, data);
    free(msg);
}

static void CALLBACK
inet_callback(HINTERNET inet,
              DWORD_PTR context,
              DWORD     status,
              LPVOID    status_info,
              DWORD     status_length)
{
    switch (status)
    {
    case INTERNET_STATUS_REQUEST_COMPLETE: {
        net_response_param param;
        DWORD              query_number;
        DWORD              query_size;

        char  buffer[4096] = "";
        DWORD bytes_size = 0;

        INTERNET_ASYNC_RESULT *result = (INTERNET_ASYNC_RESULT *)status_info;
        if (ERROR_SUCCESS != result->dwError)
        {
            if (ERROR_INTERNET_OPERATION_CANCELLED == result->dwError)
            {
                break;
            }

            set_error(proc_, result->dwError, (void *)context);
            InternetSetStatusCallbackA(session_, NULL);
            InternetCloseHandle(request_);
            InternetCloseHandle(connection_);
            request_ = connection_ = NULL;
            break;
        }

        query_size = sizeof(query_number);
        if (!HttpQueryInfoA(request_,
                            HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER,
                            &query_number, &query_size, NULL))
        {
            // Cannot query status code
            set_error(proc_, GetLastError(), (void *)context);
            break;
        }
        param.status = query_number;

        query_size = lengthof(param.status_text);
        HttpQueryInfoA(request_, HTTP_QUERY_STATUS_TEXT, param.status_text,
                       &query_size, NULL);

        query_size = sizeof(query_number);
        if (!HttpQueryInfoA(request_,
                            HTTP_QUERY_CONTENT_LENGTH | HTTP_QUERY_FLAG_NUMBER,
                            &query_number, &query_size, NULL))
        {
            // Cannot query content length
            set_error(proc_, GetLastError(), (void *)context);
            break;
        }
        param.content_length = query_number;

        proc_(NETM_RESPONSE, &param, (void *)context);

        while (InternetReadFile(request_, buffer, sizeof(buffer) - 1,
                                &bytes_size) &&
               (0 < bytes_size))
        {
            net_received_param param = {bytes_size, (uint8_t *)buffer};
            buffer[bytes_size] = 0;
            if (0 != bytes_size)
            {
                proc_(NETM_RECEIVED, &param, (void *)context);
            }
        }

        if ((ERROR_SUCCESS != GetLastError()) &&
            (ERROR_NO_MORE_ITEMS != GetLastError()))
        {
            // Reading error
            set_error(proc_, GetLastError(), (void *)context);
            break;
        }

        proc_(NETM_COMPLETE, NULL, (void *)context);

        InternetSetStatusCallbackA(session_, NULL);
        InternetCloseHandle(request_);
        InternetCloseHandle(connection_);
        request_ = connection_ = NULL;
        break;
    }
    }
}

bool
net_connect(const char *url, net_proc *proc, void *data)
{
    URL_COMPONENTSA parts;
    char            url_hostname[MAX_PATH] = "";

    if (NULL != connection_)
    {
        // Already connected
        return false;
    }

    ZeroMemory(&parts, sizeof(parts));
    parts.dwStructSize = sizeof(parts);
    parts.lpszHostName = url_hostname;
    parts.dwHostNameLength = MAX_PATH;
    if (!InternetCrackUrlA(url, 0, 0, &parts))
    {
        // URL processing error
        set_error(proc, GetLastError(), data);
        return false;
    }

    data_ = (DWORD_PTR)data;
    proc_ = proc;
    InternetSetStatusCallbackA(session_, inet_callback);

    if (NULL == (connection_ = InternetConnectA(
                     session_, parts.lpszHostName, parts.nPort, NULL, NULL,
                     INTERNET_SERVICE_HTTP, 0, data_)))
    {
        // Connecting error
        set_error(proc, GetLastError(), data);
        return false;
    }

    proc(NETM_CONNECTED, NULL, data);
    return true;
}

bool
net_request(const char *method, const char *url)
{
    URL_COMPONENTSA parts;
    const char     *headers = NULL;
    const char     *payload = NULL;
    int             payload_length;

    if (NULL == connection_)
    {
        // Not connected
        return false;
    }

    ZeroMemory(&parts, sizeof(parts));
    parts.dwStructSize = sizeof(parts);
    parts.dwUrlPathLength = MAX_PATH;
    if (!InternetCrackUrlA(url, 0, 0, &parts))
    {
        // URL processing error
        proc_(NETM_ERROR, NULL, (void *)data_);
        return false;
    }

    if (NULL == (request_ = HttpOpenRequestA(connection_, method,
                                             parts.lpszUrlPath, NULL, NULL,
                                             NULL, INTERNET_FLAG_NO_UI, data_)))
    {
        // Request opening error
        set_error(proc_, GetLastError(), (void *)data_);
        goto end;
    }

    if (0 > proc_(NETM_GETHEADERS, (void *)&headers, (void *)data_))
    {
        headers = NULL;
    }

    if (0 > (payload_length =
                 proc_(NETM_GETPAYLOAD, (void *)&payload, (void *)data_)))
    {
        payload = NULL;
    }

    if (!HttpSendRequestA(request_, headers, headers ? -1 : 0, (LPVOID)payload,
                          payload ? payload_length : 0))
    {
        if (ERROR_IO_PENDING == GetLastError())
        {
            return true;
        }

        // Sending error
        set_error(proc_, GetLastError(), (void *)data_);
        goto end;
    }

end:
    net_close();
    return false;
}

void
net_close(void)
{
    if (NULL != request_)
    {
        InternetCloseHandle(request_);
        request_ = NULL;
    }

    if (NULL != connection_)
    {
        InternetCloseHandle(connection_);
        connection_ = NULL;
    }
}
