#ifdef _WIN32
#include <winnt.rh>
#endif

#include <nls.h>

#include <generated/config.h>

#define IDS_ERROR     0x00
#define IDS_NOARCHIVE 0x01
#define IDS_UNSUPPENV 0x0F

#if defined(CONFIG_ENCRYPTED_CONTENT)
#define IDS_ENTERDSN       0x10
#define IDS_ENTERDSN_DESC  0x11
#define IDS_ENTERPASS      0x12
#define IDS_ENTERPASS_DESC 0x13
#define IDS_ENTERPKEY      0x14
#define IDS_ENTERPKEY_DESC 0x15
#define IDS_INVALIDDSNPASS 0x16
#define IDS_INVALIDPASS    0x17
#define IDS_INVALIDPKEY    0x18
#define IDS_OK             0x1E
#define IDS_CANCEL         0x1F
#endif

#define IDS_LOADERROR 0x20
#define IDS_EXECERROR 0x21
#define IDS_NOEXECCTX 0x22

#define IDS_INVALIDDELAY  0x30
#define IDS_UNKNOWNTYPE   0x31
#define IDS_INVALIDVPOS   0x32
#define IDS_INVALIDHPOS   0x33
#define IDS_LONGNAME      0x34
#define IDS_LONGCONTENT   0x35
#define IDS_NOLABEL       0x36
#define IDS_INVALIDCMPVAL 0x37
#define IDS_NOASSET       0x38
#define IDS_BADENCODING   0x39
#define IDS_UNSUPPORTED   0x3A
#define IDS_TOOMANYAREAS  0x3B
#if defined(CONFIG_ENCRYPTED_CONTENT)
#define IDS_UNKNOWNKEYSRC 0x3C
#endif
#define IDS_BADSOUND 0x3D

#define IDS_DESCRIPTION 0x40
#define IDS_COPYRIGHT   0x41
#define IDS_ABOUT       0x42
#define IDS_ABOUT_LONG  0x43

#ifdef _WIN32
#define IDS_SIZE 0x50
#define IDS_FULL 0x51
#endif
