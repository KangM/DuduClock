#include <Base64Util.h>
#include <Ed25519.h>
#include <DuduUtil.h>

#define KEY_LENGTH  32

uint8_t signature[64];
uint8_t jwtPrivateKey[32];
uint8_t jwtPublicKey[32];

String generateJWT(const char *PrivateKey, const char *PublicKey, String KeyID, String ProjectID){
    int private_s_l = strlen(PrivateKey);
    char privateKeyInput[private_s_l + 1];
    memcpy(privateKeyInput, PrivateKey, private_s_l + 1);
    int private_b_l = base64Util.decodedLength(privateKeyInput, private_s_l);
    char privateKeyBytes_[private_b_l + 1];
    base64Util.decode(privateKeyBytes_, privateKeyInput, private_s_l);
    for(int i = 0; i < KEY_LENGTH; i++){
        jwtPrivateKey[i] = privateKeyBytes_[private_b_l - KEY_LENGTH + i];
    }
    int public_s_l = strlen(PublicKey);
    char publicKeyInput[public_s_l + 1];
    memcpy(publicKeyInput, PublicKey, public_s_l + 1);
    int public_b_l = base64Util.decodedLength(publicKeyInput, public_s_l);
    char publicKeyBytes_[public_b_l + 1];
    base64Util.decode(publicKeyBytes_, publicKeyInput, public_s_l);
    for(int i = 0; i < KEY_LENGTH; i++){
        jwtPublicKey[i] = publicKeyBytes_[public_b_l - KEY_LENGTH + i];
    }
    String headerJson = "{\"alg\": \"EdDSA\", \"kid\": \"" + KeyID + "\"}";
    char headerJsonCharArray[headerJson.length() + 1];
    headerJson.toCharArray(headerJsonCharArray, headerJson.length() + 1);
    int headerJsonLength = base64Util.encodedLength(headerJson.length());
    char headerJsonEncodedString[headerJsonLength + 1];
    base64Util.encodeURL(headerJsonEncodedString, headerJsonCharArray, headerJson.length());
    time_t tm;
    time(&tm);
    unsigned long iat = tm - 120;
    unsigned long exp = iat + 1800;
    String payloadJson = "{\"sub\": \"" + ProjectID + "\", \"iat\": " + String(iat) + ", \"exp\": " + String(exp) + "}";
    char payloadJsonCharArray[payloadJson.length() + 1];
    payloadJson.toCharArray(payloadJsonCharArray, payloadJson.length() + 1);
    int payloadJsonLength = base64Util.encodedLength(payloadJson.length());
    char payloadJsonEncodedString[payloadJsonLength + 1];
    base64Util.encodeURL(payloadJsonEncodedString, payloadJsonCharArray, payloadJson.length());
    String data = String(headerJsonEncodedString) + "." + String(payloadJsonEncodedString);
    Ed25519::sign(signature, jwtPrivateKey, jwtPublicKey, data.c_str(), data.length());
    int signatureLength = base64Util.encodedLength(64);
    char signatureEncodedString[signatureLength + 1];
    base64Util.encodeURL(signatureEncodedString, (char*)signature, 64);
    String jwt = data + "." + String(signatureEncodedString);
    return jwt;
}
