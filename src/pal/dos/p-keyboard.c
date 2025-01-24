#include <ctype.h>

#include <arch/dos/bios.h>
#include <pal.h>

uint16_t
pal_get_keystroke(void)
{
    if (0 == bios_check_keystroke())
    {
        return 0;
    }

    uint16_t keystroke = bios_get_keystroke();
    if (keystroke & 0xFF)
    {
        if ('-' == (keystroke & 0xFF))
        {
            return VK_OEM_MINUS;
        }

        return toupper(keystroke & 0xFF);
    }

    switch (keystroke >> 8)
    {
    case 0x3B:
        return VK_F1;
    case 0x3C:
        return VK_F2;
    case 0x3D:
        return VK_F3;
    case 0x3E:
        return VK_F4;
    case 0x3F:
        return VK_F5;
    case 0x40:
        return VK_F6;
    case 0x41:
        return VK_F7;
    case 0x42:
        return VK_F8;
    case 0x43:
        return VK_F9;
    case 0x44:
        return VK_F10;
    case 0x47:
        return VK_HOME;
    case 0x48:
        return VK_UP;
    case 0x49:
        return VK_PRIOR;
    case 0x4B:
        return VK_LEFT;
    case 0x4D:
        return VK_RIGHT;
    case 0x4F:
        return VK_END;
    case 0x50:
        return VK_DOWN;
    case 0x51:
        return VK_NEXT;
    case 0x52:
        return VK_INSERT;
    case 0x53:
        return VK_DELETE;
    case 0x85:
        return VK_F11;
    case 0x86:
        return VK_F12;
    default:
        return 0;
    }
}
