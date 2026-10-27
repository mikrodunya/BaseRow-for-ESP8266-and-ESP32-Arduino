#pragma once
#include <string>
#include <cstring>
#include <cstdint>
#include <cstdlib>
#include <utility>
#include <algorithm>
class String {
 protected: std::string value;
 public:
  String() = default;
  String(const char* s) : value(s ? s : "") {}
  String(const char* s, unsigned int n) : value(s, n) {}
  String(const std::string& s) : value(s) {}
  String(int n) : value(std::to_string(n)) {}
  String(unsigned long n) : value(std::to_string(n)) {}
  const char* c_str() const { return value.c_str(); }
  unsigned int length() const { return static_cast<unsigned int>(value.size()); }
  bool reserve(unsigned int n) { value.reserve(n); return true; }
  bool concat(const char* s, unsigned int n) { value.append(s,n); return true; }
  bool concat(const char* s) { value += s; return true; }
  bool concat(char c) { value += c; return true; }
  bool endsWith(const char* s) const { size_t n=strlen(s);return value.size()>=n && value.compare(value.size()-n,n,s)==0; }
  bool startsWith(const char* s) const { return value.compare(0,strlen(s),s)==0; }
  void remove(unsigned int n) { value.erase(n); }
  char operator[](size_t n) const { return value[n]; }
  String& operator+=(const String& s) { value+=s.value; return *this; }
  friend String operator+(const String& a,const String& b) { return a.value+b.value; }
  friend bool operator==(const String& a,const String& b) { return a.value==b.value; }
  friend bool operator!=(const String& a,const String& b) { return !(a==b); }
};
class Print {
 public: virtual ~Print()=default;
 virtual size_t write(uint8_t)=0;
 virtual size_t write(const uint8_t* p,size_t n) {size_t i=0;for(;i<n;++i)if(!write(p[i]))break;return i;}
};
class Stream: public Print {
 public: virtual int available()=0; virtual int read()=0; virtual int peek()=0;virtual void flush()=0;
};
extern uint32_t mockMillis;
inline uint32_t millis(){return mockMillis;}
