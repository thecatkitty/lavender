#ifdef __MINGW32__
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
#define IDS_INVALIDKEY     0x14
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
