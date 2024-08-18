#include <assert.h>
#include <conio.h>

#include <pal.h>
#include <snd.h>

#define MPU401_DATA    0x330
#define MPU401_CONTROL 0x331

#define MPU401_RX_EMPTY 0x80
#define MPU401_TX_FULL  0x40

#define MPU401_RESET      0xFF
#define MPU401_ENTER_UART 0x3F

#define MPU401_ACK 0xFE

#define MPU401_TIMEOUT_CONTROL_MS 250
#define MPU401_TIMEOUT_DATA_MS    25

typedef struct
{
    bool is_good;
} mpu401_data;

static_assert(sizeof(mpu401_data) <= sizeof(far void *),
              "Device data size exceeds the field size.");

#define get_data(dev) ((mpu401_data *)(&(dev)->data))

inline static bool
_can_write(void)
{
    return 0 == (inp(MPU401_CONTROL) & MPU401_TX_FULL);
}

inline static bool
_can_read(void)
{
    return 0 == (inp(MPU401_CONTROL) & MPU401_RX_EMPTY);
}

inline static bool
_flush_read(uint32_t timeout)
{
    while (_can_read())
    {
        if (timeout < pal_get_counter())
        {
            return false;
        }

        inp(MPU401_DATA);
    }

    return true;
}

inline static bool
_wait_write(uint32_t timeout)
{
    do
    {
        if (!_flush_read(timeout))
        {
            return false;
        }

        if (timeout < pal_get_counter())
        {
            return false;
        }
    } while (!_can_write());

    return true;
}

inline static void
_write(uint8_t data)
{
    outp(MPU401_DATA, data);
}

inline static uint8_t
_read(void)
{
    return inp(MPU401_DATA);
}

static bool
_reset(unsigned ms)
{
    uint32_t timeout = pal_get_counter() + pal_get_ticks(ms);
    _wait_write(timeout);
    outp(MPU401_CONTROL, MPU401_RESET);

    while (true)
    {
        if (_can_read())
        {
            if (MPU401_ACK == inp(MPU401_DATA))
            {
                break;
            }
        }

        if (timeout < pal_get_counter())
        {
            return false;
        }
    }

    return _flush_read(timeout);
}

static bool
_init_uart(unsigned ms)
{
    uint32_t timeout = pal_get_counter() + pal_get_ticks(ms);
    if (!_wait_write(timeout))
    {
        return false;
    }

    outp(MPU401_CONTROL, MPU401_ENTER_UART);
    return true;
}

static bool ddcall
mpu401_write(snd_device *dev, const midi_event *event);

static bool ddcall
mpu401_open(snd_device *dev)
{
    if (!_reset(MPU401_TIMEOUT_CONTROL_MS))
    {
        return false;
    }

    if (!_init_uart(MPU401_TIMEOUT_CONTROL_MS))
    {
        return false;
    }

    char       program = 0;
    midi_event event = {0, 0, &program, sizeof(program)};
    for (int i = 0; i < 16; i++)
    {
        if (9 == i)
        {
            continue;
        }

        event.status = MIDI_MSG_PROGRAM | i;
        mpu401_write(dev, &event);
    }

    get_data(dev)->is_good = true;
    return true;
}

static void ddcall
mpu401_close(snd_device *dev)
{
    if (!get_data(dev)->is_good)
    {
        return;
    }

    char       message[2] = {0, 0};
    midi_event event = {0, 0, message, 2};
    for (int i = 0; i < 16; i++)
    {
        event.status = MIDI_MSG_CONTROL | i;

        // All notes off
        message[0] = 123;
        mpu401_write(dev, &event);

        // All sounds off
        message[0] = 120;
        mpu401_write(dev, &event);

        // All controllers off
        message[0] = 121;
        mpu401_write(dev, &event);
    }
    _reset(MPU401_TIMEOUT_CONTROL_MS);
}

static bool ddcall
mpu401_write(snd_device *dev, const midi_event *event)
{
    if (!get_data(dev)->is_good)
    {
        return false;
    }

    if (MIDI_MSG_META == event->status)
    {
        return true;
    }

    uint32_t ticks = pal_get_ticks(MPU401_TIMEOUT_DATA_MS);

    if (!_wait_write(pal_get_counter() + ticks))
    {
        get_data(dev)->is_good = false;
        return false;
    }

    _write(event->status);

    const uint8_t *begin = (const uint8_t *)event->msg;
    const uint8_t *end = begin + event->msg_length;
    while (begin < end)
    {
        if (!_wait_write(pal_get_counter() + ticks))
        {
            get_data(dev)->is_good = false;
            return false;
        }

        _write(*begin++);
    }
    return true;
}

static snd_device DRV_DATA     _dev = {"mpu", "Roland MPU-401 UART"};
static snd_device_ops DRV_DATA _ops = {mpu401_open, mpu401_close, mpu401_write};

DRV_INIT(mpu401)(void)
{
    _dev.ops = &_ops;
    get_data(&_dev)->is_good = false;
    return snd_register_device(&_dev);
}
