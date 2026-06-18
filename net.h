#ifndef __NET_H
#define __NET_H
#include "common.h"

void wifiConfigBySoftAP(void);
void cityConfigByWiFi(void);
void startServer(void);
void doClient(void);
void connectWiFi(int timeOut_s);
void getNowWeather(void);
void getFutureWeather(void);
void getAir(void);
void getNTPTime(void);
void getCityID(void);
void checkWiFiStatus(void);
void restartSystem(String msg, bool endTips);
void setHefengConfigValues(const String& privateKey, const String& publicKey, const String& keyId, const String& projectId, const String& apiHost);
String getHefengPrivateKey(void);
String getHefengPublicKey(void);
String getHefengKeyId(void);
String getHefengProjectId(void);
String getHefengApiHost(void);
extern bool queryNowWeatherSuccess;
extern bool queryFutureWeatherSuccess;
extern bool queryAirSuccess;
extern String ssid;
extern String pass;
extern String city;
extern String adm;
extern String location;
extern String lat;
extern String lon;
extern bool isStartQuery;
extern String privateKey;
extern String publicKey;
extern String keyId;
extern String projectId;
extern String apiHost;

#endif
