#pragma once
#define WL_CONNECTED 3
class MockWiFi { public: int state=WL_CONNECTED; int status()const{return state;} };
extern MockWiFi WiFi;
