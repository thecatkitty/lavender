#ifndef COMPAT_ASSERT_H
#define COMPAT_ASSERT_H

#include <assert.h>

#if !defined(__cplusplus) && !defined(static_assert)

#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L)

#define static_assert _Static_assert

#else /* defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L) */

#define COMPAT_ASSERT_JOIN_(a, b) a##b
#define COMPAT_ASSERT_JOIN(a, b)  COMPAT_ASSERT_JOIN_(a, b)

#define static_assert(expr, msg)                                               \
    typedef char COMPAT_ASSERT_JOIN(assert_, __LINE__)[(expr) ? 1 : -1]

#endif /* defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L) */

#endif /* !defined(__cplusplus) && !defined(static_assert) */

#endif /* COMPAT_ASSERT_H */
