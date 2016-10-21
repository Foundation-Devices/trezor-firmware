/*
 * Copyright (c) Pavol Rusnak, SatoshiLabs
 *
 * Licensed under TREZOR License
 * see LICENSE file for details
 */

#include "py/objstr.h"

#include "trezor-crypto/ecdsa.h"
#include "trezor-crypto/secp256k1.h"

typedef struct _mp_obj_Secp256k1_t {
    mp_obj_base_t base;
} mp_obj_Secp256k1_t;

STATIC mp_obj_t mod_TrezorCrypto_Secp256k1_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    mp_arg_check_num(n_args, n_kw, 0, 0, false);
    mp_obj_Secp256k1_t *o = m_new_obj(mp_obj_Secp256k1_t);
    o->base.type = type;
    return MP_OBJ_FROM_PTR(o);
}

/// def trezor.crypto.curve.secp256k1.publickey(secret_key: bytes, compressed: bool=True) -> bytes:
///     '''
///     Computes public key from secret key.
///     '''
STATIC mp_obj_t mod_TrezorCrypto_Secp256k1_publickey(size_t n_args, const mp_obj_t *args) {
    mp_buffer_info_t sk;
    mp_get_buffer_raise(args[1], &sk, MP_BUFFER_READ);
    if (sk.len != 32) {
        mp_raise_ValueError("Invalid length of secret key");
    }
    bool compressed = n_args < 3 || args[2] == mp_const_true;
    vstr_t vstr;
    if (compressed) {
        vstr_init_len(&vstr, 33);
        ecdsa_get_public_key33(&secp256k1, (const uint8_t *)sk.buf, (uint8_t *)vstr.buf);
    } else {
        vstr_init_len(&vstr, 65);
        ecdsa_get_public_key65(&secp256k1, (const uint8_t *)sk.buf, (uint8_t *)vstr.buf);
    }
    return mp_obj_new_str_from_vstr(&mp_type_bytes, &vstr);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mod_TrezorCrypto_Secp256k1_publickey_obj, 2, 3, mod_TrezorCrypto_Secp256k1_publickey);

/// def trezor.crypto.curve.secp256k1.sign(secret_key: bytes, message: bytes) -> bytes:
///     '''
///     Uses secret key to produce the signature of message.
///     '''
STATIC mp_obj_t mod_TrezorCrypto_Secp256k1_sign(mp_obj_t self, mp_obj_t secret_key, mp_obj_t message) {
    mp_buffer_info_t sk, msg;
    mp_get_buffer_raise(secret_key, &sk, MP_BUFFER_READ);
    mp_get_buffer_raise(message, &msg, MP_BUFFER_READ);
    if (sk.len != 32) {
        mp_raise_ValueError("Invalid length of secret key");
    }
    if (msg.len == 0) {
        mp_raise_ValueError("Empty data to sign");
    }
    vstr_t vstr;
    vstr_init_len(&vstr, 65);
    uint8_t pby;
    if (0 != ecdsa_sign(&secp256k1, (const uint8_t *)sk.buf, (const uint8_t *)msg.buf, msg.len, (uint8_t *)vstr.buf, &pby, NULL)) { // TODO: is_canonical
        mp_raise_ValueError("Signing failed");
    }
    (void)pby;
    return mp_obj_new_str_from_vstr(&mp_type_bytes, &vstr);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(mod_TrezorCrypto_Secp256k1_sign_obj, mod_TrezorCrypto_Secp256k1_sign);

/// def trezor.crypto.curve.secp256k1.verify(public_key: bytes, signature: bytes, message: bytes) -> bool:
///     '''
///     Uses public key to verify the signature of the message
///     Returns True on success.
///     '''
STATIC mp_obj_t mod_TrezorCrypto_Secp256k1_verify(size_t n_args, const mp_obj_t *args) {
    mp_buffer_info_t pk, sig, msg;
    mp_get_buffer_raise(args[1], &pk, MP_BUFFER_READ);
    mp_get_buffer_raise(args[2], &sig, MP_BUFFER_READ);
    mp_get_buffer_raise(args[3], &msg, MP_BUFFER_READ);
    if (pk.len != 33 && pk.len != 65) {
        mp_raise_ValueError("Invalid length of public key");
    }
    if (sig.len != 65) {
        mp_raise_ValueError("Invalid length of signature");
    }
    if (msg.len == 0) {
        mp_raise_ValueError("Empty data to verify");
    }
    return mp_obj_new_bool(0 == ecdsa_verify(&secp256k1, (const uint8_t *)pk.buf, (const uint8_t *)sig.buf, (const uint8_t *)msg.buf, msg.len));
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mod_TrezorCrypto_Secp256k1_verify_obj, 4, 4, mod_TrezorCrypto_Secp256k1_verify);

/// def trezor.crypto.curve.secp256k1.multiply(secret_key: bytes, public_key: bytes) -> bytes:
///     '''
///     Multiplies point defined by public_key with scalar defined by secret_key
///     Useful for ECDH
///     '''
STATIC mp_obj_t mod_TrezorCrypto_Secp256k1_multiply(mp_obj_t self, mp_obj_t secret_key, mp_obj_t public_key) {
    mp_buffer_info_t sk, pk;
    mp_get_buffer_raise(secret_key, &sk, MP_BUFFER_READ);
    mp_get_buffer_raise(public_key, &pk, MP_BUFFER_READ);
    if (sk.len != 32) {
        mp_raise_ValueError("Invalid length of secret key");
    }
    if (pk.len != 33 && pk.len != 65) {
        mp_raise_ValueError("Invalid length of public key");
    }
    vstr_t vstr;
    vstr_init_len(&vstr, 65);
    if (0 != ecdh_multiply(&secp256k1, (const uint8_t *)sk.buf, (const uint8_t *)pk.buf, (uint8_t *)vstr.buf)) {
        mp_raise_ValueError("Multiply failed");
    }
    return mp_obj_new_str_from_vstr(&mp_type_bytes, &vstr);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(mod_TrezorCrypto_Secp256k1_multiply_obj, mod_TrezorCrypto_Secp256k1_multiply);

STATIC const mp_rom_map_elem_t mod_TrezorCrypto_Secp256k1_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_publickey), MP_ROM_PTR(&mod_TrezorCrypto_Secp256k1_publickey_obj) },
    { MP_ROM_QSTR(MP_QSTR_sign), MP_ROM_PTR(&mod_TrezorCrypto_Secp256k1_sign_obj) },
    { MP_ROM_QSTR(MP_QSTR_verify), MP_ROM_PTR(&mod_TrezorCrypto_Secp256k1_verify_obj) },
    { MP_ROM_QSTR(MP_QSTR_multiply), MP_ROM_PTR(&mod_TrezorCrypto_Secp256k1_multiply_obj) },
};
STATIC MP_DEFINE_CONST_DICT(mod_TrezorCrypto_Secp256k1_locals_dict, mod_TrezorCrypto_Secp256k1_locals_dict_table);

STATIC const mp_obj_type_t mod_TrezorCrypto_Secp256k1_type = {
    { &mp_type_type },
    .name = MP_QSTR_Secp256k1,
    .make_new = mod_TrezorCrypto_Secp256k1_make_new,
    .locals_dict = (void*)&mod_TrezorCrypto_Secp256k1_locals_dict,
};
