#ifndef COMPAT_FAR_H
#define COMPAT_FAR_H

#if defined(__cplusplus) || !defined(__ia16__) || defined(__INTELLISENSE__)

#define far
#define ddcall

#endif

#ifndef far
#define far __far
#endif

#ifndef ddcall
#if !defined(__IA16_CMODEL_TINY__)
#define ddcall __attribute__((no_assume_ds_data)) __far
#else
#define ddcall
#endif
#endif

#endif
