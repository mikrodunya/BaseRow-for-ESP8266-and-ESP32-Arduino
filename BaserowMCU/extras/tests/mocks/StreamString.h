#pragma once
#include "Arduino.h"
class StreamString:public Stream,public String {public:
 size_t write(uint8_t c)override{return write(&c,1);}
 size_t write(const uint8_t*p,size_t n)override{concat(reinterpret_cast<const char*>(p),n);return n;}
 int available()override{return length();}int read()override{return -1;}int peek()override{return -1;}void flush()override{}
};
