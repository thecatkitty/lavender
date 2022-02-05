#ifndef _NLS_H_
#define _NLS_H_

#define LCID_ENU 1033
#define LCID_PLP 1045

#ifndef LANG
#define LANG LCID_ENU
#endif

#if LANG == LCID_ENU
#define IF_LANG_ENU(x) x
#else
#define IF_LANG_ENU(x)
#endif

#if LANG == LCID_PLP
#define IF_LANG_PLP(x) x
#else
#define IF_LANG_PLP(x)
#endif

#define IF_LANG(lcid, x) IF_LANG_##lcid(x)

#endif // _NLS_H_
