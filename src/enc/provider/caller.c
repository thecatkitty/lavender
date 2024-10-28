#include "../enc_impl.h"

int
__enc_caller_acquire(enc_context *enc)
{
    if (ENC_XOR == enc->cipher)
    {
        enc->key.qw = rstrtoull(enc->parameter, 16);
        return 0;
    }

    if (ENC_DES == enc->cipher)
    {
        for (int i = 0; i < sizeof(uint64_t); i++)
        {
            enc->key.b[i] = xtob((const char *)enc->parameter + (2 * i));
        }

        return 0;
    }

    return -EINVAL;
}

int
__enc_caller_handle(enc_context *enc)
{
    if (ENCS_INVALID == enc->state)
    {
        return -EACCES;
    }

    return -ENOSYS;
}

enc_provider_impl __enc_caller_impl = {&__enc_caller_acquire,
                                       &__enc_caller_handle};
