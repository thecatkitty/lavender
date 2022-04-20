#ifndef _NLS_H_
#define _NLS_H_

#define LCID_CSY 1029
#define LCID_ENU 1033
#define LCID_PLK 1045

#ifndef LANG
#define LANG LCID_ENU
#endif

#if LANG == LCID_CSY
#define IF_LANG_CSY(x) x
#else
#define IF_LANG_CSY(x)
#endif

#if LANG == LCID_ENU
#define IF_LANG_ENU(x) x
#else
#define IF_LANG_ENU(x)
#endif

#if LANG == LCID_PLK
#define IF_LANG_PLK(x) x
#else
#define IF_LANG_PLK(x)
#endif

#define IF_LANG(lcid, x) IF_LANG_##lcid(x)

#endif // _NLS_H_
