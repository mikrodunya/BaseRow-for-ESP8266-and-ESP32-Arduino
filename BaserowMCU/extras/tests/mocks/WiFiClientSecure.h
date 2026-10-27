#pragma once
namespace BearSSL {class Session{};class X509List{public:explicit X509List(const char*){}};}
class WiFiClientSecure {public:
 bool connected=false;int stops=0; void stop(){connected=false;++stops;}
 void setInsecure(){} void setCACert(const char*){} void setSession(BearSSL::Session*){}
 void setTrustAnchors(const BearSSL::X509List*){}
};
