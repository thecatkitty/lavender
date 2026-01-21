#include <stdarg.h>
#include <stdio.h>

#include <curl/curl.h>

#include <net.h>
#include <pal.h>

static CURL              *curl_ = NULL;
static net_proc          *proc_ = NULL;
static void              *data_ = 0;
static char               error_[CURL_ERROR_SIZE];
static struct curl_slist *headers_ = NULL;
static bool               receiving_;

bool
net_start(void)
{
    LOG("entry");

    CURLcode status = curl_global_init(CURL_GLOBAL_DEFAULT);
    if (0 != status)
    {
        LOG("exit, curl_global_init failed!");
        return false;
    }

    LOG("exit");
    return true;
}

void
net_stop(void)
{
    curl_global_cleanup();
}

static int
_rsnprintf(char *buff, size_t buflen, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    int written = vsnprintf(buff, buflen, fmt, args);
    va_end(args);

    if ((written < 0) || (written >= buflen))
    {
        *buff = 0;
        return -1;
    }

    return written;
}

static void
_build_user_agent(char *buff, size_t buflen)
{
    int   written = 0, remaining = buflen;
    char *ptr = buff;

    // Lavender/*
    if (0 >
        (written = _rsnprintf(ptr, remaining, "%s", pal_get_version_string())))
        goto end;
    ptr += written;
    remaining -= written;

    ptr = strrchr(buff, ' ');
    if (ptr)
    {
        *ptr = '/';
    }

    // operating system
    if (0 > (written = _rsnprintf(ptr, remaining, " (Linux")))
        goto end;
    ptr += written;
    remaining -= written;

    // architecture
    if (0 > (written = _rsnprintf(ptr, remaining, "; %s)",
#if defined(__i386__)
                                  "ia32"
#elif defined(__amd64__)
                                  "x64"
#else
#error "Unknown architecture!"
#endif
                                  )))
        goto end;

end:
    if (0 == *buff)
    {
        strcat(buff, "Lavender");
    }
}

bool
net_connect(const char *url, net_proc *proc, void *data)
{
    LOG("entry, '%s'", url);

    if (NULL != curl_)
    {
        LOG("exit, already connected!");
    }

    curl_ = curl_easy_init();
    if (NULL == curl_)
    {
        LOG("exit, curl_easy_init failed!");
        return false;
    }

    proc_ = proc;
    data_ = data;
    curl_easy_setopt(curl_, CURLOPT_ERRORBUFFER, error_);

    char user_agent[PATH_MAX] = "";
    _build_user_agent(user_agent, sizeof(user_agent));
    if (0 != curl_easy_setopt(curl_, CURLOPT_USERAGENT, user_agent))
    {
        proc_(NETM_ERROR, error_, data_);
        LOG("exit, CURLOPT_USERAGENT failed!");
        return false;
    }
    LOG("User-Agent: %s", user_agent);

    LOG("exit, ok");
    return true;
}

#define REQUIRE_ZERO(x)                                                        \
    if (0 != (x))                                                              \
    {                                                                          \
        proc_(NETM_ERROR, error_, data_);                                      \
        status = false;                                                        \
        goto end;                                                              \
    }

static size_t
_inet_write_callback(char *ptr, size_t size, size_t nmemb, void *userdata)
{
    bool status = true;

    if (!receiving_)
    {
        long status_code;
        REQUIRE_ZERO(
            curl_easy_getinfo(curl_, CURLINFO_RESPONSE_CODE, &status_code));

        curl_off_t content_length;
        REQUIRE_ZERO(curl_easy_getinfo(curl_, CURLINFO_SIZE_DOWNLOAD_T,
                                       &content_length));

        net_response_param response_param = {status_code, "", content_length};
        snprintf(response_param.status_text, sizeof(response_param.status_text),
                 "HTTP %d", response_param.status);
        proc_(NETM_RESPONSE, &response_param, data_);

        receiving_ = true;
    }

    if (status)
    {
        net_received_param param = {nmemb, (uint8_t *)ptr};
        proc_(NETM_RECEIVED, &param, data_);
    }

end:
    return size * nmemb;
}

bool
net_request(const char *method, const char *url)
{
    LOG("entry, %s %s", method, url);

    bool status = true;

    REQUIRE_ZERO(curl_easy_setopt(curl_, CURLOPT_CUSTOMREQUEST, method));
    REQUIRE_ZERO(curl_easy_setopt(curl_, CURLOPT_URL, url));
    REQUIRE_ZERO(
        curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, _inet_write_callback));

    char *headers = NULL;
    if (0 == proc_(NETM_GETHEADERS, (void *)&headers, (void *)data_))
    {
        char  header[PATH_MAX];
        char *start = headers, *end;

        while (NULL != (end = strstr(start, "\r\n")))
        {
            memcpy(header, start, end - start);
            header[end - start] = 0;
            start = end + 2;

            LOG("%s", header);
            headers_ = curl_slist_append(headers_, header);
        }

        REQUIRE_ZERO(curl_easy_setopt(curl_, CURLOPT_HTTPHEADER, headers_));
    }

    char *payload = NULL;
    int   payload_length = proc_(NETM_GETPAYLOAD, (void *)&payload, data_);
    if (0 < payload_length)
    {
        REQUIRE_ZERO(
            curl_easy_setopt(curl_, CURLOPT_POSTFIELDSIZE, payload_length));
        REQUIRE_ZERO(curl_easy_setopt(curl_, CURLOPT_COPYPOSTFIELDS, payload));
    }

    receiving_ = false;
    REQUIRE_ZERO(curl_easy_perform(curl_));

    proc_(NETM_COMPLETE, NULL, data_);

end:
    if (NULL != headers_)
    {
        curl_slist_free_all(headers_);
        headers_ = NULL;
    }

    LOG("exit, %s", status ? "ok" : "failed");
    return status;
}

void
net_close(void)
{
    LOG("entry");

    if (NULL != curl_)
    {
        curl_easy_cleanup(curl_);
        curl_ = NULL;
        proc_ = 0;
        data_ = 0;
    }

    LOG("exit");
}
