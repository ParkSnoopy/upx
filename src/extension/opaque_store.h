/* Shared by the packer/unpacker and the Linux x86-64 executable stub. */

#ifndef UPX_EXTENSION_OPAQUE_STORE_H
#define UPX_EXTENSION_OPAQUE_STORE_H

/* 15 == M_DEFLATE, which the stubs do not use. Stock UPX rejects it as a
   block method, so a stored block cannot be unpacked by only editing the
   pack header. */
#define OPAQUE_B_METHOD 15

static inline unsigned
opaque_key(unsigned sz_unc, unsigned sz_cpr)
{
    return (sz_unc * 2654435761u) ^ (sz_cpr * 40503u) ^ 0xA5A5A5A5u;
}

static inline void
opaque_xor(unsigned char *data, unsigned size, unsigned sz_unc, unsigned sz_cpr)
{
    unsigned char key = (unsigned char)opaque_key(sz_unc, sz_cpr);
    unsigned i;
    for (i = 0; i < size; ++i)
        data[i] ^= key;
}

#endif
