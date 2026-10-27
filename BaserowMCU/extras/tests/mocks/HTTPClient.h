#pragma once
#include "Arduino.h"
#include "WiFiClientSecure.h"
#include <deque>
#include <vector>
#include <map>
#include <cassert>
struct Reply {int code;std::string body;int size=-2;bool keep=true;bool incomplete=false;};
struct Request {std::string method,url,payload;std::map<std::string,std::string> headers;};
extern std::deque<Reply> replies;
extern std::vector<Request> requests;
extern int connections,drained,uncleanReuse;
class HTTPClient {
 WiFiClientSecure* client=nullptr;bool reuse=false,pending=false;Reply reply;String url;
 std::map<std::string,std::string> headers;
 public:
 ~HTTPClient(){if(client)client->stop();}
 void setReuse(bool b){reuse=b;}void setTimeout(uint16_t){}void setConnectTimeout(int32_t){}
 bool begin(WiFiClientSecure& c,const String& u){client=&c;url=u;return true;}
 void addHeader(const String& k,const String& v){headers[k.c_str()]=v.c_str();}
 int sendRequest(const char*method,uint8_t*payload,size_t n){
   if(pending && client->connected)++uncleanReuse;
   if(!client->connected){++connections;client->connected=true;}
   requests.push_back({method,url.c_str(),payload?std::string(reinterpret_cast<char*>(payload),n):"",headers});
   assert(!replies.empty());reply=replies.front();replies.pop_front();pending=reply.code>0 && !reply.body.empty();
   mockMillis+=20;return reply.code;
 }
 int getSize(){return reply.size==-2?static_cast<int>(reply.body.size()):reply.size;}
 int writeToStream(Stream* s){
   size_t n=reply.body.size();if(reply.incomplete)n/=2;
   for(size_t i=0;i<n;){size_t part=std::min<size_t>(7,n-i);if(s->write(reinterpret_cast<const uint8_t*>(reply.body.data()+i),part)!=part)return -10;i+=part;}
   if(reply.incomplete)return -11;
   pending=false;++drained;return static_cast<int>(n);
 }
 void end(){headers.clear();if(client && (!reuse || !reply.keep)){client->stop();pending=false;}}
 static String errorToString(int n){return String("transport ")+String(n);}
};
