#ifndef _DUDUUTIL_H
#define _DUDUUTIL_H

#include <Arduino.h>

String generateJWT(const char *PrivateKey, const char *PublicKey, String KeyID, String ProjectID);

#endif
