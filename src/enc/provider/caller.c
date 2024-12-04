#include "../enc_impl.h"

int
__enc_caller_proc(int msg, enc_context *enc)
{
    switch (msg)
    {
    case ENCM_INITIALIZE: {
        return 0;
    }

    case ENCM_TRANSFORM: {
        if (ENC_XOR == enc->cipher)
        {
            enc->key.qw = rstrtoull(enc->parameter, 16);
            return 0;
        }

        if ((ENC_DES == enc->cipher) || (ENC_TDES == enc->cipher))
        {
            int i,
                size = sizeof(uint64_t) * ((ENC_TDES == enc->cipher) ? 2 : 1);
            for (i = 0; i < size; i++)
            {
                enc->key.b[i] = xtob((const char *)enc->parameter + (2 * i));
            }

            return 0;
        }

        return -EINVAL;
    }

    case ENCM_GET_STORAGE_POLICY: {
        return ENCSTORPOL_NONE;
    }
    }

    return -ENOSYS;
}
