// This file is automatically generated from u2f_knownapps.h.mako
// DO NOT EDIT

#ifndef __U2F_KNOWNAPPS_H__
#define __U2F_KNOWNAPPS_H__

#include <stdint.h>
#include "u2f/u2f.h"

typedef struct {
  const uint8_t appid[U2F_APPID_SIZE];
  const char *appname;
} U2FWellKnown;

<%
from hashlib import sha256

def c_bytes(rp_id_hash):
    return "{ " + ", ".join(["0x%02x" % x for x in rp_id_hash]) + " }"

fido_entries = []
for app in fido:
    for app_id in app.u2f:
        fido_entries.append((app.label, bytes.fromhex(app_id), "U2F", app))
    for origin in app.webauthn:
        rp_id_hash = sha256(origin.encode()).digest()
        fido_entries.append((origin, rp_id_hash, "WebAuthn", app))
%>\
// clang-format off
static const U2FWellKnown u2f_well_known[] = {
% for label, rp_id_hash, type, app in fido_entries:
	{
		// ${type} for ${app.label}
		${c_bytes(rp_id_hash)},
		${c_str(label)}
	},
% endfor
};
// clang-format on

#endif  // __U2F_KNOWNAPPS_H__
