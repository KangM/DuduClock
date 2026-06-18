#include <HTTPClient.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include "ArduinoZlib.h"
#include "common.h"
#include "PreferencesUtil.h"
#include "tftUtil.h"
#include "task.h"
#include <Arduino.h>
#include "DuduUtil.h"

// 鍜岄澶╂皵韬唤璁よ瘉锛岄渶瑕佹浛鎹㈡垚浣犱滑鑷繁鐨?
String privateKey = "";
String publicKey = "";
String keyId = "";
String projectId = "";
String apiHost = "";

void sendNTPpacket(IPAddress &address);
void startAP();
void startServer();
void scanWiFi();
void handleNotFound();
void handleRoot();
void handleWifiRoot();
void handleCityRoot();
void handleConfigWifi();
void handleConfigCity();
void restartSystem(String msg, bool endTips);
String urlEncode(const String& text);
bool hasHefengConfig();
bool serverStarted = false;

bool queryNowWeatherSuccess = false;
bool queryFutureWeatherSuccess = false;
bool queryAirSuccess = false;

// Wifi鐩稿叧
String ssid;  //WIFI鍚嶇О
String pass;  //WIFI瀵嗙爜
String city;  // 鍩庡競
String adm; // 涓婄骇鍩庡競鍖哄垝
String location; // 鍩庡競ID
String lat; // 缁忓害
String lon; // 绾害
String WifiNames; // 鏍规嵁鎼滅储鍒扮殑wifi鐢熸垚鐨刼ption瀛楃涓?
// SoftAP鐩稿叧
const char *APssid = "DuduClock";
IPAddress staticIP(192, 168, 1, 1);
IPAddress gateway(192, 168, 1, 254);
IPAddress subnet(255, 255, 255, 0);
WebServer server(80);
// 鏌ヨ澶╂皵瓒呮椂鏃堕棿(ms)
int queryTimeout = 6000;
// 鏄惁鏄垰鍚姩鏃舵煡璇㈠ぉ姘?
bool isStartQuery = true; 
// 澶╂皵鎺ュ彛鐩稿叧
static HTTPClient httpClient;
String data = "";
uint8_t *outbuf;

// 寮€鍚疭oftAP杩涜閰嶇綉
void wifiConfigBySoftAP(){
  // 寮€鍚疉P妯″紡锛屽鏋滃紑鍚け璐ワ紝閲嶅惎绯荤粺
  startAP();
  // 鎵弿WiFi,骞跺皢鎵弿鍒扮殑WiFi缁勬垚option閫夐」瀛楃涓?
  scanWiFi();
  // 鍚姩鏈嶅姟鍣?
  startServer();
  // 鏄剧ず閰嶇疆缃戠粶椤甸潰
  tft.pushImage(0, 0, 240, 320, QRcode);
}

void cityConfigByWiFi(){
  startServer();
  draw2LineText("天气配置", WiFi.localIP().toString());
}

// 澶勭悊鏈嶅姟鍣ㄨ姹?
void doClient(){
  server.handleClient();
}
// 澶勭悊404鎯呭喌鐨勫嚱鏁?handleNotFound'
void handleNotFound(){
  handleRoot();//璁块棶涓嶅瓨鍦ㄧ洰褰曞垯杩斿洖閰嶇疆椤甸潰
}
// 澶勭悊缃戠珯鏍圭洰褰曠殑璁块棶璇锋眰
void handleRoot(){
  if (currentPage == SETTING_WIFI && (WiFi.getMode() == WIFI_MODE_AP || WiFi.getMode() == WIFI_MODE_APSTA)) {
    if (WiFi.getMode() == WIFI_MODE_AP || WiFi.getMode() == WIFI_MODE_APSTA) {
      scanWiFi();
    }
    String page = ROOT_HTML_WIFI_PAGE;
    String currentWifiOption = "";
    if (ssid.length() > 0) {
      currentWifiOption = "<option value='" + ssid + "' selected>" + ssid + " (当前)</option>";
    }
    page.replace("__CURRENT_WIFI_OPTION__", currentWifiOption);
    page.replace("__WIFI_OPTIONS__", WifiNames);
    page.replace("__PASS__", pass);
    server.send(200,"text/html", page);
    return;
  }
  handleCityRoot();
}

void handleWifiRoot(){
  if (WiFi.getMode() == WIFI_MODE_AP || WiFi.getMode() == WIFI_MODE_APSTA) {
    scanWiFi();
  }
  String page = ROOT_HTML_WIFI_PAGE;
  String currentWifiOption = "";
  if (ssid.length() > 0) {
    currentWifiOption = "<option value='" + ssid + "' selected>" + ssid + " (当前)</option>";
  }
  page.replace("__CURRENT_WIFI_OPTION__", currentWifiOption);
  page.replace("__WIFI_OPTIONS__", WifiNames);
  page.replace("__PASS__", pass);
  server.send(200,"text/html", page);
}
void handleCityRoot(){
  String page = ROOT_HTML_CITY_PAGE;
  page.replace("__SSID__", ssid);
  page.replace("__CITY__", city);
  page.replace("__ADM__", adm);
  page.replace("__PRIVATE_KEY__", privateKey);
  page.replace("__PUBLIC_KEY__", publicKey);
  page.replace("__KEY_ID__", keyId);
  page.replace("__PROJECT_ID__", projectId);
  page.replace("__API_HOST__", apiHost);
  server.send(200, "text/html", page);
}
// 鎻愪氦鏁版嵁鍚庣殑鎻愮ず椤甸潰
void handleConfigWifi(){
  if (!server.hasArg("ssid") || !server.hasArg("pass")) {
    server.send(200, "text/html", "<meta charset='UTF-8'>缺少 WiFi 信息");
    return;
  }
  ssid = server.arg("ssid");
  pass = server.arg("pass");
  setWiFiCity();
  currentPage = SETTING_WIFI;
  server.send(200, "text/html", "<meta charset='UTF-8'><style type='text/css'>body {font-size: 2rem;}</style><br/><br/>WiFi 已保存，设备正在重启，请重新连接到同一局域网后访问设备 IP。");
  delay(800);
  ESP.restart();
}

void handleConfigCity(){
  if (!server.hasArg("city")) {
    server.send(200, "text/html", "<meta charset='UTF-8'>缺少城市名");
    return;
  }
  city = server.arg("city");
  adm = server.hasArg("adm") ? server.arg("adm") : "";
  privateKey = server.hasArg("privateKey") ? server.arg("privateKey") : privateKey;
  publicKey = server.hasArg("publicKey") ? server.arg("publicKey") : publicKey;
  keyId = server.hasArg("keyId") ? server.arg("keyId") : keyId;
  projectId = server.hasArg("projectId") ? server.arg("projectId") : projectId;
  apiHost = server.hasArg("apiHost") ? server.arg("apiHost") : apiHost;
  setWiFiCity();
  setHefengConfig();
  currentPage = SETTING_CITY;
  server.send(200, "text/html", "<meta charset='UTF-8'><style type='text/css'>body {font-size: 2rem;}</style><br/><br/>天气配置已保存，设备正在重启。");
  delay(800);
  ESP.restart();
}

// 杩炴帴WiFi
void connectWiFi(int timeOut_s){
  delay(1500);
  drawText("正在连接网络...");
  int connectTime = 0;
  pinMode(D4,OUTPUT);
  Serial.print("正在连接网络");
  Serial.println(ssid);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    digitalWrite(D4, !digitalRead(D4));
    delay(500);
    connectTime++;
    if (connectTime > 2 * timeOut_s){
      Serial.println("网络连接失败");
      currentPage = SETTING_WIFI;
      return;
    }
  }
  digitalWrite(D4, LOW);
  Serial.println("网络连接成功");
  Serial.print("本地IP:");
  Serial.println(WiFi.localIP());
}
// 妫€鏌iFi杩炴帴鐘舵€侊紝濡傛灉鏂紑浜嗭紝閲嶆柊杩炴帴
void checkWiFiStatus(){
  if(WiFi.status() != WL_CONNECTED){ // 缃戠粶鏂紑浜嗭紝杩涜閲嶈繛
    Serial.println("网络断开，即将重新连接...");
    WiFi.begin(ssid, pass);
  }
}
// 鍚姩鏈嶅姟鍣?
void startServer(){
  if (serverStarted) {
    return;
  }
  server.on("/", HTTP_GET, handleRoot);
  server.on("/wifi", HTTP_GET, handleWifiRoot);
  server.on("/city", HTTP_GET, handleCityRoot);
  server.on("/configwifi", HTTP_POST, handleConfigWifi);
  server.on("/configcity", HTTP_POST, handleConfigCity);
  server.onNotFound(handleNotFound);
  server.begin();
  serverStarted = true;
  Serial.println("服务器启动成功！");
}
// 寮€鍚疉P妯″紡锛屽鏋滃紑鍚け璐ワ紝閲嶅惎绯荤粺
void startAP(){
  Serial.println("开启AP模式...");
  WiFi.enableAP(true); // 浣胯兘AP妯″紡
  //浼犲叆鍙傛暟闈欐€両P鍦板潃,缃戝叧,鎺╃爜
  WiFi.softAPConfig(staticIP, gateway, subnet);
  if (!WiFi.softAP(APssid)) {
    Serial.println("AP模式启动失败");
    ESP.restart(); // Ap妯″紡鍚姩澶辫触锛岄噸鍚郴缁?
  }  
  Serial.println("AP模式启动成功");
  Serial.print("IP地址: ");
  Serial.println(WiFi.softAPIP());
}
// 鎵弿WiFi,骞跺皢鎵弿鍒扮殑Wifi缁勬垚option閫夐」瀛楃涓?
void scanWiFi(){
  Serial.println("开始扫描WiFi");
  int n = WiFi.scanNetworks();
  if (n){
    Serial.print("扫描到");
    Serial.print(n);
    Serial.println("个WIFI");
    WifiNames = "";
    for (size_t i = 0; i < n; i++){
      int32_t rssi = WiFi.RSSI(i);
      String signalStrength;
      if(rssi >= -35){
        signalStrength = " (信号极强)";
      }else if(rssi >= -50){
        signalStrength = " (信号强)";
      }else if(rssi >= -70){
        signalStrength = " (信号中)";
      }else{
        signalStrength = " (信号弱)";
      }
      WifiNames += "<option value='" + WiFi.SSID(i) + "'>" + WiFi.SSID(i) + signalStrength + "</option>";
      // Serial.print("WiFi鐨勫悕绉?SSID):");
      // Serial.println(WiFi.SSID(i));
    }
  }else{
    Serial.println("没有扫描到WiFi");
  }
}

// 鏌ヨ鍩庡競id
void getCityID(){
  // 璁＄畻jwt
  String jwt = generateJWT(privateKey.c_str(), publicKey.c_str(), keyId, projectId);
  // Serial.println(jwt);
  bool flag = false; // 鏄惁鎴愬姛鑾峰彇鍒板煄甯俰d鐨勬爣蹇?
  String url = "https://" + apiHost + cityURL + "?location=" + urlEncode(city) + "&adm=" + urlEncode(adm);
  // Serial.println(url);
  httpClient.setConnectTimeout(queryTimeout * 5);
  httpClient.begin(url);
  httpClient.addHeader("Authorization", "Bearer " + jwt);
  //鍚姩杩炴帴骞跺彂閫丠TTP璇锋眰
  int httpCode = httpClient.GET();
  Serial.println("正在获取城市id");
  // 澶勭悊鏈嶅姟鍣ㄧ瓟澶?
  if (httpCode == HTTP_CODE_OK) {
    // 瑙ｅ帇Gzip鏁版嵁娴?
    int len = httpClient.getSize();
    uint8_t buff[2048] = { 0 };
    WiFiClient *stream = httpClient.getStreamPtr();
    while (httpClient.connected() && (len > 0 || len == -1)) {
      size_t size = stream->available();  // 杩樺墿涓嬪灏戞暟鎹病鏈夎瀹岋紵
      // Serial.println(size);
      if (size) {
        size_t realsize = ((size > sizeof(buff)) ? sizeof(buff) : size);
        // Serial.println(realsize);
        size_t readBytesSize = stream->readBytes(buff, realsize);
        // Serial.write(buff,readBytesSize);
        if (len > 0) len -= readBytesSize;
        outbuf = (uint8_t *)malloc(sizeof(uint8_t) * 5120);
        uint32_t outprintsize = 0;
        int result = ArduinoZlib::libmpq__decompress_zlib(buff, readBytesSize, outbuf, 5120, outprintsize);
        // Serial.write(outbuf, outprintsize);
        for (int i = 0; i < outprintsize; i++) {
          data += (char)outbuf[i];
        }
        free(outbuf);
        Serial.println(data);
      }
      delay(1);
    }
    // 瑙ｅ帇瀹岋紝杞崲json鏁版嵁
    StaticJsonDocument<2048> doc; //澹版槑涓€涓潤鎬丣sonDocument瀵硅薄
    DeserializationError error = deserializeJson(doc, data); //鍙嶅簭鍒楀寲JSON鏁版嵁
    if(!error){ //妫€鏌ュ弽搴忓垪鍖栨槸鍚︽垚鍔?
      //璇诲彇json鑺傜偣
      String code = doc["code"].as<const char*>();
      if(code.equals("200")){
        flag = true;
        // 澶氱粨鏋滅殑鎯呭喌涓嬶紝鍙栫涓€涓?
        city = doc["location"][0]["name"].as<const char*>();
        location = doc["location"][0]["id"].as<const char*>();
        lat = doc["location"][0]["lat"].as<const char*>();
        lon = doc["location"][0]["lon"].as<const char*>();
        Serial.println("城市id: " + location);
        // 灏嗕俊鎭瓨鍏vs涓?
        setWiFiCity();
      }
    }  
  }
  if(!flag){
    Serial.print("获取城市id错误：");
    Serial.println(httpCode);
    currentPage = SETTING_CITY;
    httpClient.end();
    return;
  }
  httpClient.end();
}

// 鏌ヨ瀹炴椂澶╂皵
void getNowWeather(){
  // 璁＄畻jwt
  String jwt = generateJWT(privateKey.c_str(), publicKey.c_str(), keyId, projectId);
  data = "";
  queryNowWeatherSuccess = false; // 鍏堢疆涓篺alse
  String url = "https://" + apiHost + nowURL + "?location=" + location;
  httpClient.setConnectTimeout(queryTimeout);
  httpClient.begin(url);
  httpClient.addHeader("Authorization", "Bearer " + jwt);
  //鍚姩杩炴帴骞跺彂閫丠TTP璇锋眰
  int httpCode = httpClient.GET();
  // Serial.println(ESP.getFreeHeap());
  Serial.println("正在获取天气数据");
  //濡傛灉鏈嶅姟鍣ㄥ搷搴擮K鍒欎粠鏈嶅姟鍣ㄨ幏鍙栧搷搴斾綋淇℃伅骞堕€氳繃涓插彛杈撳嚭
  if (httpCode == HTTP_CODE_OK) {
    // 瑙ｅ帇Gzip鏁版嵁娴?
    int len = httpClient.getSize();
    uint8_t buff[2048] = { 0 };
    WiFiClient *stream = httpClient.getStreamPtr();
    while (httpClient.connected() && (len > 0 || len == -1)) {
      size_t size = stream->available();  // 杩樺墿涓嬪灏戞暟鎹病鏈夎瀹岋紵
      // Serial.println(size);
      if (size) {
        size_t realsize = ((size > sizeof(buff)) ? sizeof(buff) : size);
        // Serial.println(realsize);
        size_t readBytesSize = stream->readBytes(buff, realsize);
        // Serial.write(buff,readBytesSize);
        if (len > 0) len -= readBytesSize;
        outbuf = (uint8_t *)malloc(sizeof(uint8_t) * 5120);
        uint32_t outprintsize = 0;
        int result = ArduinoZlib::libmpq__decompress_zlib(buff, readBytesSize, outbuf, 5120, outprintsize);
        // Serial.write(outbuf, outprintsize);
        for (int i = 0; i < outprintsize; i++) {
          data += (char)outbuf[i];
        }
        free(outbuf);
        Serial.println(data);
      }
      delay(1);
    }
    // 瑙ｅ帇瀹岋紝杞崲json鏁版嵁
    StaticJsonDocument<2048> doc; //澹版槑涓€涓潤鎬丣sonDocument瀵硅薄
    DeserializationError error = deserializeJson(doc, data); //鍙嶅簭鍒楀寲JSON鏁版嵁
    if(!error){ //妫€鏌ュ弽搴忓垪鍖栨槸鍚︽垚鍔?
      //璇诲彇json鑺傜偣
      String code = doc["code"].as<const char*>();
      if(code.equals("200")){
        queryNowWeatherSuccess = true;       
        //璇诲彇json鑺傜偣
        nowWeather.text = doc["now"]["text"].as<const char*>();
        nowWeather.icon = doc["now"]["icon"].as<int>();
        nowWeather.temp = doc["now"]["temp"].as<int>();
        nowWeather.humidity = doc["now"]["humidity"].as<int>();
        String feelsLike = doc["now"]["feelsLike"]; // 浣撴劅娓╁害
        nowWeather.feelsLike = "体感温度" + feelsLike + "℃";
        String windDir = doc["now"]["windDir"];
        String windScale = doc["now"]["windScale"];
        nowWeather.win = windDir + windScale + "级";
        String vis = doc["now"]["vis"];
        nowWeather.vis = "能见度" + vis + " KM";
      }
    }  
  }
  if(!queryNowWeatherSuccess){
    Serial.print("请求实时天气错误：");
    Serial.println(httpCode);
  }
  //鍏抽棴涓庢湇鍔″櫒杩炴帴
  httpClient.end();
}

// 鏌ヨ绌烘皵璐ㄩ噺
void getAir(){
  String jwt = generateJWT(privateKey.c_str(), publicKey.c_str(), keyId, projectId);
  data = "";
  queryAirSuccess = false;
  String url = "https://" + apiHost + airURL + lat + "/" + lon;
  httpClient.setConnectTimeout(queryTimeout);
  httpClient.begin(url);
  httpClient.addHeader("Authorization", "Bearer " + jwt);
  int httpCode = httpClient.GET();
  Serial.println("正在获取空气质量数据");
  if (httpCode == HTTP_CODE_OK) {
    int len = httpClient.getSize();
    uint8_t buff[2048] = { 0 };
    WiFiClient *stream = httpClient.getStreamPtr();
    while (httpClient.connected() && (len > 0 || len == -1)) {
      size_t size = stream->available();
      if (size) {
        size_t realsize = ((size > sizeof(buff)) ? sizeof(buff) : size);
        size_t readBytesSize = stream->readBytes(buff, realsize);
        if (len > 0) len -= readBytesSize;
        outbuf = (uint8_t *)malloc(sizeof(uint8_t) * 20480);
        uint32_t outprintsize = 0;
        ArduinoZlib::libmpq__decompress_zlib(buff, readBytesSize, outbuf, 20480, outprintsize);
        for (int i = 0; i < outprintsize; i++) {
          data += (char)outbuf[i];
        }
        free(outbuf);
      }
      delay(1);
    }
    StaticJsonDocument<2048> doc;
    DeserializationError error = deserializeJson(doc, data);
    if(!error){
      queryAirSuccess = true;
      nowWeather.air = doc["indexes"][0]["aqi"].as<int>();
      JsonArray pollutants = doc["pollutants"];
      for (JsonObject pollutant : pollutants) {
        String code = pollutant["code"].as<const char*>();
        unsigned int value = pollutant["concentration"]["value"].as<int>();
        if(code.equals("pm2p5")){
          nowWeather.pm2p5 = String(value);
        }else if(code.equals("pm10")){
          nowWeather.pm10 = String(value);
        }else if(code.equals("no2")){
          nowWeather.no2 = String(value);
        }else if(code.equals("so2")){
          nowWeather.so2 = String(value);
        }else if(code.equals("co")){
          nowWeather.co = String(value);
        }else if(code.equals("o3")){
          nowWeather.o3 = String(value);
        }
      }
      Serial.println("获取成功");
    }
  }
  if(!queryAirSuccess){
    Serial.print("请求空气质量错误：");
    Serial.println(httpCode);
  }
  httpClient.end();
}
// 查询未来天气
void getFutureWeather(){
  String jwt = generateJWT(privateKey.c_str(), publicKey.c_str(), keyId, projectId);
  data = "";
  queryFutureWeatherSuccess = false;
  String url = "https://" + apiHost + futureURL + "?location=" + location;
  httpClient.setConnectTimeout(queryTimeout);
  httpClient.begin(url);
  httpClient.addHeader("Authorization", "Bearer " + jwt);
  int httpCode = httpClient.GET();
  Serial.println("正在获取一周天气数据");
  if (httpCode == HTTP_CODE_OK) {
    int len = httpClient.getSize();
    uint8_t buff[2048] = { 0 };
    WiFiClient *stream = httpClient.getStreamPtr();
    while (httpClient.connected() && (len > 0 || len == -1)) {
      size_t size = stream->available();
      if (size) {
        size_t realsize = ((size > sizeof(buff)) ? sizeof(buff) : size);
        size_t readBytesSize = stream->readBytes(buff, realsize);
        if (len > 0) len -= readBytesSize;
        outbuf = (uint8_t *)malloc(sizeof(uint8_t) * 5120);
        uint32_t outprintsize = 0;
        ArduinoZlib::libmpq__decompress_zlib(buff, readBytesSize, outbuf, 5120, outprintsize);
        for (int i = 0; i < outprintsize; i++) {
          data += (char)outbuf[i];
        }
        free(outbuf);
      }
      delay(1);
    }
    StaticJsonDocument<2048> doc;
    DeserializationError error = deserializeJson(doc, data);
    if(!error){
      String code = doc["code"].as<const char*>();
      if(code.equals("200")){
        queryFutureWeatherSuccess = true;
        futureWeather.day0wea = doc["daily"][0]["textDay"].as<const char*>();
        futureWeather.day0wea_img = doc["daily"][0]["iconDay"].as<int>();
        futureWeather.day0date = doc["daily"][0]["fxDate"].as<const char*>();
        futureWeather.day0tem_day = doc["daily"][0]["tempMax"].as<int>();
        futureWeather.day0tem_night = doc["daily"][0]["tempMin"].as<int>();
        futureWeather.day1wea = doc["daily"][1]["textDay"].as<const char*>();
        futureWeather.day1wea_img = doc["daily"][1]["iconDay"].as<int>();
        futureWeather.day1date = doc["daily"][1]["fxDate"].as<const char*>();
        futureWeather.day1tem_day = doc["daily"][1]["tempMax"].as<int>();
        futureWeather.day1tem_night = doc["daily"][1]["tempMin"].as<int>();
        futureWeather.day2wea = doc["daily"][2]["textDay"].as<const char*>();
        futureWeather.day2wea_img = doc["daily"][2]["iconDay"].as<int>();
        futureWeather.day2date = doc["daily"][2]["fxDate"].as<const char*>();
        futureWeather.day2tem_day = doc["daily"][2]["tempMax"].as<int>();
        futureWeather.day2tem_night = doc["daily"][2]["tempMin"].as<int>();
        futureWeather.day3wea = doc["daily"][3]["textDay"].as<const char*>();
        futureWeather.day3wea_img = doc["daily"][3]["iconDay"].as<int>();
        futureWeather.day3date = doc["daily"][3]["fxDate"].as<const char*>();
        futureWeather.day3tem_day = doc["daily"][3]["tempMax"].as<int>();
        futureWeather.day3tem_night = doc["daily"][3]["tempMin"].as<int>();
        futureWeather.day4wea = doc["daily"][4]["textDay"].as<const char*>();
        futureWeather.day4wea_img = doc["daily"][4]["iconDay"].as<int>();
        futureWeather.day4date = doc["daily"][4]["fxDate"].as<const char*>();
        futureWeather.day4tem_day = doc["daily"][4]["tempMax"].as<int>();
        futureWeather.day4tem_night = doc["daily"][4]["tempMin"].as<int>();
        futureWeather.day5wea = doc["daily"][5]["textDay"].as<const char*>();
        futureWeather.day5wea_img = doc["daily"][5]["iconDay"].as<int>();
        futureWeather.day5date = doc["daily"][5]["fxDate"].as<const char*>();
        futureWeather.day5tem_day = doc["daily"][5]["tempMax"].as<int>();
        futureWeather.day5tem_night = doc["daily"][5]["tempMin"].as<int>();
        futureWeather.day6wea = doc["daily"][6]["textDay"].as<const char*>();
        futureWeather.day6wea_img = doc["daily"][6]["iconDay"].as<int>();
        futureWeather.day6date = doc["daily"][6]["fxDate"].as<const char*>();
        futureWeather.day6tem_day = doc["daily"][6]["tempMax"].as<int>();
        futureWeather.day6tem_night = doc["daily"][6]["tempMin"].as<int>();
        Serial.println("获取成功");
      }
    }
  }
  if(!queryFutureWeatherSuccess){
    Serial.print("请求一周天气错误：");
    Serial.println(httpCode);
  }
  httpClient.end();
}

// 鑾峰彇NTP骞跺悓姝TC鏃堕棿
void getNTPTime(){
  // 8 * 3600 涓滃叓鍖烘椂闂翠慨姝?
  // 浣跨敤澶忎护鏃?daylightOffset_sec 灏卞～鍐?600锛屽惁鍒欏氨濉啓0锛?
  Serial.println("NTP对时...");
  configTime( 8 * 3600, 0, NTP1, NTP2, NTP3);
}
// 閲嶅惎绯荤粺
// bool endTips  鏄惁闇€瑕佹妸鈥滃悓姝ュぉ姘旀暟鎹€濇枃瀛楃殑瀹氭椂鍣ㄤ换鍔″彇娑?
void restartSystem(String msg, bool endTips){
  if(endTips){
    //缁撴潫寰幆鏄剧ず鎻愮ず鏂囧瓧鐨勫畾鏃跺櫒
    timerEnd(timerShowTips);
  }
  reflashTFT();
  for(int i = 3; i > 0; i--){
    String text = "";
    text = text + i + "秒后系统重启";
    draw2LineText(msg,text);
    delay(1000);
  }
  ESP.restart();
}

String urlEncode(const String& text) {
  String encodedText = "";
  for (size_t i = 0; i < text.length(); i++) {
    char c = text[i];
    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '-' || c == '_' || c == '.' || c == '~') {
      encodedText += c;
    } else if (c == ' ') {
      encodedText += '+';
    } else {
      encodedText += '%';
      char hex[4];
      sprintf(hex, "%02X", (uint8_t)c);
      encodedText += hex;
    }
  }
  return encodedText;
}

bool hasHefengConfig() {
  return city.length() > 0 &&
         privateKey.length() > 0 &&
         publicKey.length() > 0 &&
         keyId.length() > 0 &&
         projectId.length() > 0 &&
         apiHost.length() > 0;
}

