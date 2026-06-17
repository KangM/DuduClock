#ifndef _DUDUUTIL_H
#define _DUDUUTIL_H

#include <Arduino.h>

String generateJWT(char *PrivateKey, char *PublicKey, String KeyID, String ProjectID);

#endif