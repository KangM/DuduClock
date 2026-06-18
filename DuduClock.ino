#include <OneButton.h>
#include <WiFi.h>
#include "net.h"
#include "common.h"
#include "preferencesUtil.h"
#include "tftUtil.h"
#include "task.h"

/**
Dudu澶╂皵鏃堕挓  鐗堟湰2.2

鏈鏇存柊鍐呭锛?

淇敼浜嗗拰椋庡ぉ姘旂殑绌烘皵璐ㄩ噺鎺ュ彛銆?
鍜岄澶╂皵鐨勭┖姘旇川閲廣7鎺ュ彛2026骞村仠姝㈡湇鍔★紝鎵€浠ュ皢鏈」鐩腑鐨勭┖姘旇川閲忔帴鍙ｆ洿鎹负浜哣1銆?

*/


unsigned int prevDisplay = 0; // 瀹炲喌澶╂皵椤甸潰涓婃鏄剧ず鐨勬椂闂?
unsigned int preTimerDisplay = 0; // 璁℃暟鍣ㄩ〉闈笂娆℃樉绀虹殑姣鏁?10,鍗?0姣鏄剧ず涓€娆?
unsigned long startMillis = 0; // 寮€濮嬭鏁版椂鐨勬绉掓暟
int synDataRestartTime = 60; // 鍚屾NTP鏃堕棿鍜屽ぉ姘斾俊鎭椂锛岃秴杩囧灏戠灏遍噸鍚郴缁燂紝闃叉缃戠粶涓嶅ソ鏃讹紝鍌荤瓑
bool isCouting = false; // 璁℃椂鍣ㄦ槸鍚︽鍦ㄥ伐浣?
OneButton myButton(BUTTON, true);

void setup() {
  Serial.begin(115200);
  Serial.println("boot: setup begin");
  // TFT鍒濆鍖?
  tftInit();
  Serial.println("boot: tftInit done");
  // 鏄剧ず绯荤粺鍚姩鏂囧瓧
  drawText("系统启动中...");
  getWiFiCity();
  getHefengConfig();
  // nvs涓病鏈塛iFi淇℃伅锛屼笅鍙厛閰嶇綉
  if(ssid.length() == 0 || pass.length() == 0){
    currentPage = SETTING_WIFI; // 将页面置为配置页面
    wifiConfigBySoftAP();
    return;
  }

  currentPage = WEATHER;
  connectWiFi(30);
  if(WiFi.status() != WL_CONNECTED){
    currentPage = SETTING_WIFI;
    wifiConfigBySoftAP();
    return;
  }

  startServer();

  if(city.length() == 0 || privateKey.length() == 0 || publicKey.length() == 0 ||
     keyId.length() == 0 || projectId.length() == 0 || apiHost.length() == 0){
    currentPage = SETTING_CITY;
    cityConfigByWiFi();
    return;
  }

  initDatas();
  drawWeatherPage();
  startRunner();
  startTimerQueryWeather();
  myButton.attachClick(click);
  myButton.attachDoubleClick(doubleclick);
  myButton.attachLongPressStart(longclick);
  myButton.setPressMs(2000);
  myButton.setDebounceMs(10);
}
void loop() {
  myButton.tick();
  switch(currentPage){
    case SETTING_WIFI:
    case SETTING_CITY:
      doClient();
      break;
    case WEATHER:  // 澶╂皵鏃堕挓椤甸潰
      executeRunner();
      time_t now;
      time(&now);
      if(now != prevDisplay){ // 姣忕鏇存柊涓€娆℃椂闂存樉绀?
        prevDisplay = now;
        // 缁樺埗鏃堕棿銆佹棩鏈熴€佹槦鏈?
        drawDateWeek();
      }
      break;
    case AIR:  // 绌烘皵璐ㄩ噺椤甸潰
      executeRunner();
      break;
    case FUTUREWEATHER:  // 鏈潵澶╂皵椤甸潰
      executeRunner();
      break;
    case THEME:  // 榛戠櫧涓婚璁剧疆椤甸潰
      executeRunner();
      break;
    case TIMER:  // 璁℃椂鍣ㄩ〉闈?
      if(isCouting && (millis() / 10) != preTimerDisplay){ // 姣忓崄姣鏇存柊涓€娆℃暟瀛楁樉绀?
        preTimerDisplay = millis() / 10;
        // 缁樺埗璁℃暟鍣ㄦ暟瀛?
        drawNumsByCount(timerCount + millis() - startMillis);
      }
      break;
    case RESET:  // 鎭㈠鍑哄巶璁剧疆椤甸潰
      executeRunner();
      break;
    default:
      break;
  }
}

////////////////////////// 鎸夐敭鍖?//////////////////////
// 鍗曞嚮鎿嶄綔锛岀敤鏉ュ垏鎹㈠悇涓〉闈?
void click(){
  if(currentPage == TIMER){
    if(!isCouting){
      // Serial.println("寮€濮嬭鏁?);
      startMillis = millis();
    }else{
      // Serial.println("鍋滄璁℃暟");
      timerCount += millis() - startMillis;
      // 缁樺埗璁℃暟鍣ㄦ暟瀛?
      drawNumsByCount(timerCount);
    }
    isCouting = !isCouting;
  }
}
void doubleclick(){
  switch(currentPage){
    case WEATHER:
      disableAnimScrollText();
      drawAirPage();
      currentPage = AIR;
      break;
    case AIR:
      drawFutureWeatherPage();
      currentPage = FUTUREWEATHER;
      break;
    case FUTUREWEATHER:
      drawThemePage();
      currentPage = THEME;
      break;
    case THEME:
      drawTimerPage();
      currentPage = TIMER;
      break;
    case TIMER:
      drawResetPage();
      currentPage = RESET;
      break;
    case RESET:
      drawWeatherPage();
      enableAnimScrollText();
      currentPage = WEATHER;
      break;
    default:
      break;
  }
}
void longclick(){
  if(currentPage == RESET){
    Serial.println("恢复出厂设置");
    // 鎭㈠鍑哄巶璁剧疆骞堕噸鍚?
    restartSystem("已恢复出厂设置", false);
  }else if(currentPage == THEME){
    Serial.println("更改主题");
    if(backColor == BACK_BLACK){ // 鍘熷厛涓洪粦鑹蹭富棰橈紝鏀逛负鐧借壊
      backColor = BACK_WHITE;
      backFillColor = 0xFFFF;
      penColor = 0x0000;
    }else{
      backColor = BACK_BLACK;
      backFillColor = 0x0000;
      penColor = 0xFFFF;
    }
    // 灏嗘柊鐨勪富棰樺瓨鍏vs
    setBackColor(backColor);
    // 杩斿洖瀹炴椂澶╂皵椤甸潰
    drawWeatherPage();
    enableAnimScrollText();
    currentPage = WEATHER;
    Serial.println("计数器归零");
    timerCount = 0; // 璁℃暟鍊煎綊闆?
    isCouting = false; // 璁℃暟鍣ㄦ爣蹇椾綅缃负false
    drawNumsByCount(timerCount); // 閲嶆柊缁樺埗璁℃暟鍖哄煙锛屾彁绀哄尯鍩熶笉鐢ㄥ彉
  }
}
////////////////////////////////////////////////////////


// 鍒濆鍖栦竴浜涘垪鏁版嵁:NTP瀵规椂銆佸疄鍐靛ぉ姘斻€佷竴鍛ㄥぉ姘?
void initDatas(){
  startTimerShowTips(); // 鑾峰彇鏁版嵁鏃讹紝寰幆鏄剧ず鎻愮ず鏂囧瓧
  // 璁板綍姝ゆ椂鐨勬椂闂达紝鍦ㄥ悓姝ユ暟鎹椂锛岃秴杩囦竴瀹氱殑鏃堕棿锛屽氨鐩存帴閲嶅惎
  time_t start;
  time(&start);
  // 鑾峰彇NTP骞跺悓姝ヨ嚦RTC,绗竴娆″悓姝ュけ璐ワ紝灏变竴鐩村皾璇曞悓姝?
  getNTPTime();
  struct tm timeinfo;
  while (!getLocalTime(&timeinfo)){
    time_t end;
    time(&end);
    if((end - start) > synDataRestartTime){
      restartSystem("同步数据失败", true);
    }
    Serial.println("时钟对时失败...");
    getNTPTime();
  }
  Serial.println("对时成功");
  // 鏌ヨ鏄惁鏈夊煄甯俰d锛屽鏋滄病鏈夛紝灏卞埄鐢╟ity鍜宎dm鏌ヨ鍑哄煄甯俰d锛屽苟淇濆瓨涓簂ocation
  if(location.equals("") || lat.equals("") || lon.equals("")){
    getCityID();
  }
  //绗竴娆℃煡璇㈠疄鍐靛ぉ姘?濡傛灉鏌ヨ澶辫触锛屽氨涓€鐩村弽澶嶆煡璇?
  getNowWeather();
  while(!queryNowWeatherSuccess){
    time_t end;
    time(&end);
    if((end - start) > synDataRestartTime){
      restartSystem("同步数据失败", true);
    }
    getNowWeather();
  }
  //绗竴娆℃煡璇㈢┖姘旇川閲?濡傛灉鏌ヨ澶辫触锛屽氨涓€鐩村弽澶嶆煡璇?
  getAir();
  while(!queryAirSuccess){
    time_t end;
    time(&end);
    if((end - start) > synDataRestartTime){
      restartSystem("同步数据失败", true);
    }
    getAir();
  }
  //绗竴娆℃煡璇竴鍛ㄥぉ姘?濡傛灉鏌ヨ澶辫触锛屽氨涓€鐩村弽澶嶆煡璇?
  getFutureWeather();
  while(!queryFutureWeatherSuccess){
    time_t end;
    time(&end);
    if((end - start) > synDataRestartTime){
      restartSystem("同步数据失败", true);
    }
    getFutureWeather();
  }
  //缁撴潫寰幆鏄剧ず鎻愮ず鏂囧瓧鐨勫畾鏃跺櫒
  timerEnd(timerShowTips);
  //灏唅sStartQuery缃负false,鍛婅瘔绯荤粺锛屽惎鍔ㄦ椂鏌ヨ澶╂皵宸插畬鎴?
  isStartQuery = false;
}


