#include <BaserowMCU.h>
#include <cassert>
#include <iostream>
uint32_t mockMillis=0;
MockWiFi WiFi;
std::deque<Reply> replies;
std::vector<Request> requests;
int connections=0,drained=0,uncleanReuse=0;
void reset(){replies.clear();requests.clear();connections=drained=uncleanReuse=0;mockMillis=0;WiFi.state=WL_CONNECTED;}
void reply(int code,const char* body,int size=-2,bool keep=true,bool incomplete=false){replies.push_back({code,body,size,keep,incomplete});}
int main(){
#if defined(ESP8266)
 assert(std::string(BaserowMCU::platform())=="ESP8266");
#else
 assert(std::string(BaserowMCU::platform())=="ESP32");
#endif
 {reset();BaserowMCU b("12","token","https://example.test/");JsonDocument row;
  reply(200,R"({"id":42,"Temp":23.5,"State":"ok"})");assert(b.readRow(42,row));
  assert(row["Temp"].as<float>()==23.5f);assert(row["State"].as<String>()=="ok");
  assert(requests.size()==1);assert(requests[0].url=="https://example.test/api/database/rows/table/12/42/?user_field_names=true");
  assert(requests[0].headers.at("Authorization")=="Token token");assert(b.lastRequestMs()==20);
  JsonDocument fields;fields["Temp"]=24;fields["State"]="new";
  reply(200,R"({"id":42,"Temp":24,"State":"new"})");if(!b.updateRow(42,fields)){std::cerr<<"update error: "<<b.lastError().c_str()<<" HTTP "<<b.lastHttpCode()<<"\n";return 1;}
  assert(requests.size()==2 && requests[1].method=="PATCH");
  JsonDocument sent;assert(!deserializeJson(sent,requests[1].payload));assert(sent.size()==2);
  reply(200,R"({"id":42,"Temp":24})");assert(b.readRow(42,row));
  assert(connections==1 && drained==3 && uncleanReuse==0);
  assert(!requests[2].headers.count("Content-Type"));assert(b.lastError().length()==0);
 }
 {reset();BaserowMCU b("1","t");
  reply(200,R"({"results":[{"id":91,"n":3,"s":"yes"}]})");assert(b.readNumber("n",2)==3);
  assert(requests[0].url.find("size=1&page=2")!=std::string::npos);
  reply(200,R"({"results":[{"id":91,"n":4,"s":"changed"}]})");assert(b.readText("s",2)=="changed");
  assert(requests.size()==2); // Cache is off by default.
  b.setReadCacheMs(100);
  reply(200,R"({"results":[{"id":91,"n":5,"s":"cached"}]})");assert(b.readNumber("n",2)==5);
  mockMillis+=50;assert(b.readText("s",2)=="cached");assert(b.lastReadFromCache());assert(b.lastHttpCode()==0);
  assert(requests.size()==3);mockMillis+=51;
  reply(200,R"({"results":[{"id":91,"n":6}]})");assert(b.readNumber("n",2)==6);assert(!b.lastReadFromCache());
  JsonDocument fields;fields["n"]=7;reply(200,"{}");assert(b.updateRow(91,fields));
  reply(200,R"({"results":[{"id":91,"n":7}]})");assert(b.readNumber("n",2)==7);assert(requests.size()==6);
  WiFi.state=0;assert(b.readNumber("n",2)==-1);assert(!b.lastReadFromCache());
 }
 {reset();BaserowMCU b("1","t");b.setReadCacheMs(100);mockMillis=UINT32_MAX-50;
  reply(200,R"({"results":[{"n":8}]})");assert(b.readNumber("n",1)==8);mockMillis+=40;
  assert(b.readNumber("n",1)==8 && b.lastReadFromCache());mockMillis+=61;
  reply(200,R"({"results":[{"n":9}]})");assert(b.readNumber("n",1)==9 && !b.lastReadFromCache());
 }
 {reset();BaserowMCU b("1","t");JsonDocument row;
  reply(-1,"");reply(200,R"({"id":5})");assert(b.readRow(5,row));assert(requests.size()==2 && connections==2);
  reply(-1,"");reply(-1,"");assert(!b.readRow(5,row));assert(requests.size()==4 && row.isNull());
  JsonDocument fields;fields["n"]=1;reply(-1,"");assert(!b.insertRow(fields));assert(requests.size()==5);
  reply(-1,"");assert(!b.updateRow(5,fields));assert(requests.size()==6); // No write replay.
 }
 {reset();BaserowMCU b("1","t");JsonDocument row;
  reply(403,R"({"error":"denied"})");assert(!b.readRow(5,row));assert(b.lastHttpCode()==403 && drained==1);
  reply(200,R"({"id":5})");assert(b.readRow(5,row));assert(connections==1 && uncleanReuse==0);
  reply(200,R"({"id":5})",-2,true,true);assert(!b.readRow(5,row));assert(row.isNull());
  reply(200,R"({"id":5})");assert(b.readRow(5,row));assert(connections==2);
 }
 {reset();BaserowMCU b("1","t");JsonDocument row;b.setMaxResponseSize(10);
  reply(200,R"({"id":5,"large":"xxxxxxxxxxxx"})");assert(!b.readRow(5,row));assert(row.isNull());
  reply(200,R"({"id":5,"large":"xxxxxxxxxxxx"})",-1);assert(!b.readRow(5,row));assert(row.isNull());
  b.setMaxResponseSize(1024);reply(200,R"({"id":5,"ok":true})",-1);assert(b.readRow(5,row));
  reply(200,"invalid");assert(!b.readRow(5,row) && row.isNull());
  reply(200,R"({"id":6})");assert(!b.readRow(5,row) && row.isNull());
  reply(200,"[]");assert(!b.readRow(5,row) && row.isNull());
  assert(!b.readRow(0,row));assert(requests.size()==6);
 }
 {reset();BaserowMCU b("1","t");JsonDocument fields;fields["n"]=1;
  reply(201,"{}");assert(b.insertRow(fields));assert(requests[0].method=="POST");
  reply(204,"");assert(b.updateRow(5,fields));assert(b.lastResponseBytes()==0);
  JsonDocument empty;assert(!b.insertRow(empty));assert(!b.updateRow(0,fields));assert(requests.size()==2);
  b.setKeepAlive(false);JsonDocument row;
  reply(200,R"({"id":5})");assert(b.readRow(5,row));reply(200,R"({"id":5})");assert(b.readRow(5,row));
  assert(connections==3);
 }
 {reset();BaserowMCU b("1","t");JsonDocument row;
  reply(200,R"({"id":5})",-2,false);assert(b.readRow(5,row));
  reply(200,R"({"id":5})");assert(b.readRow(5,row));assert(connections==2);
  reply(200,R"({"results":[]})");assert(b.readNumber("n",1)==-1);
  reply(200,R"({"results":[{"n":1}]})");assert(b.readText("missing",0)=="KOLON_BULUNAMADI");
  assert(requests.back().url.find("page=1")!=std::string::npos);
 }
 {reset();BaserowMCU b("1","t","http://example.test");JsonDocument row;assert(!b.readRow(1,row));assert(requests.empty());}
 {reset();BaserowMCU b("1","t");
  reply(201,"{}");assert(b.insertNumber("n",1));reply(201,"{}");assert(b.insertText("s","x"));
  reply(200,"{}");assert(b.updateNumber(10,"n",2));reply(200,"{}");assert(b.updateText(10,"s","y"));
  assert(requests.size()==4 && connections==1 && drained==4);
  assert(requests[0].method=="POST" && requests[1].method=="POST");
  assert(requests[2].url.find("/10/")!=std::string::npos);
  b.setReadCacheMs(1000);reply(200,R"({"results":[{"n":2}]})");assert(b.readNumber("n",1)==2);
  reply(-1,"");assert(!b.updateNumber(10,"n",3));
  reply(200,R"({"results":[{"n":3}]})");assert(b.readNumber("n",1)==3 && !b.lastReadFromCache());
 }
 assert(replies.empty());std::cout<<"PASS: "<<BaserowMCU::platform()<<" transport, cache, batching, errors and compatibility tests\n";
}
