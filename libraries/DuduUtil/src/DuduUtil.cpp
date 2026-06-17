#include <Base64Util.h>
#include <Ed25519.h>
#include <DuduUtil.h>

#define KEY_LENGTH  32

uint8_t signature[64];
uint8_t privateKey[32];
uint8_t publicKey[32];

String generateJWT(char *PrivateKey, char *PublicKey, String KeyID, String ProjectID){
    // 计算私钥的原始字节数组
    int private_s_l = strlen(PrivateKey); // 计算私钥base64编码字符串长度
    int private_b_l = base64Util.decodedLength(PrivateKey, private_s_l); // 计算私钥base64解码后的字节数
    // Serial.println(private_b_l);
    char privateKeyBytes_[private_b_l + 1]; // 字节数组，保存解码后的私钥
    base64Util.decode(privateKeyBytes_, PrivateKey, private_s_l); // 解码
    // for(int i = 0; i < private_b_l; i++){
    //   Serial.print((int)privateKeyBytes_[i]);
    //   Serial.print(" ");
    // }
    // Serial.println();
    for(int i = 0; i < KEY_LENGTH; i++){
        privateKey[i] = privateKeyBytes_[private_b_l - KEY_LENGTH + i];
        // Serial.print(privateKey[i]);
        // Serial.print(" ");
    }
    // Serial.println();
    // 计算公钥的原始字节数组
    int public_s_l = strlen(PublicKey); // 计算公钥base64编码字符串长度
    int public_b_l = base64Util.decodedLength(PublicKey, public_s_l); // 计算公钥base64解码后的字节数
    // Serial.println(public_b_l);
    char publicKeyBytes_[public_b_l + 1]; // 字节数组，保存解码后的私钥
    base64Util.decode(publicKeyBytes_, PublicKey, public_s_l); // 解码
    // for(int i = 0; i < public_b_l; i++){
    //   Serial.print((int)publicKeyBytes_[i]);
    //   Serial.print(" ");
    // }
    // Serial.println();
    for(int i = 0; i < KEY_LENGTH; i++){
        publicKey[i] = publicKeyBytes_[public_b_l - KEY_LENGTH + i];
        // Serial.print(publicKey[i]);
        // Serial.print(" ");
    }
    // Serial.println();
    // Header
    String headerJson = "{\"alg\": \"EdDSA\", \"kid\": \"" + KeyID + "\"}"; // 组件字符串
    char headerJsonCharArray[headerJson.length() + 1]; // 将字符串变为字符数组
    headerJson.toCharArray(headerJsonCharArray, headerJson.length() + 1);
    int headerJsonLength = base64Util.encodedLength(headerJson.length()); // 编码后的字节数
    char headerJsonEncodedString[headerJsonLength + 1]; // 编码后的字节数组，多1个字节存放"\0"
    base64Util.encodeURL(headerJsonEncodedString, headerJsonCharArray, headerJson.length());
    // Serial.print("headerJson string is:\t");
    // Serial.println(headerJsonEncodedString);
    // Payload
    time_t tm;
    time(&tm);
    unsigned long iat = tm - 120;
    unsigned long exp = iat + 1800;
    // Serial.print("iat");
    // Serial.println(iat);
    String payloadJson = "{\"sub\": \"" + ProjectID + "\", \"iat\": " + String(iat) + ", \"exp\": " + String(exp) + "}"; // 组件字符串
    char payloadJsonCharArray[payloadJson.length() + 1]; // 将字符串变为字符数组
    payloadJson.toCharArray(payloadJsonCharArray, payloadJson.length() + 1);
    int payloadJsonLength = base64Util.encodedLength(payloadJson.length()); // 编码后的字节数
    char payloadJsonEncodedString[payloadJsonLength + 1]; // 编码后的字节数组，多1个字节存放"\0"
    base64Util.encodeURL(payloadJsonEncodedString, payloadJsonCharArray, payloadJson.length());
    // Serial.print("payloadJson string is:\t");
    // Serial.println(payloadJsonEncodedString);
    // Base64url header+payload
    String data = String(headerJsonEncodedString) + "." + String(payloadJsonEncodedString);
    // Serial.print("data string is:\t");
    // Serial.println(data);
    // 签名
    Ed25519::sign(signature, privateKey, publicKey, data.c_str(), data.length());
    int signatureLength = base64Util.encodedLength(64); // 编码后的字节数
    char signatureEncodedString[signatureLength + 1]; // 编码后的字节数组，多1个字节存放"\0"
    base64Util.encodeURL(signatureEncodedString, (char*)signature, 64);
    String jwt = data + "." + String(signatureEncodedString);
    // Serial.print("jwt string is:\t");
    // Serial.println(jwt);
    return jwt;
}