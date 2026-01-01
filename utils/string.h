#pragma once
#include "kheap.h"

class String {
    private : 
    char* buffer;
    size_t length;
    size_t capacity;
    
    public: 
    String();
    String(const char* str);
    String(const String&other);
    
    ~String();
    void append(char c);             
    void append(const char* str);    
    void clear();
    const char* c_str() const;       
    size_t len() const;
    void pop_back(); 
    String& operator = (const char* str);
    String& operator = (const String& other);
    String& operator += (const char* str);
    
    char operator[](size_t index)const;
};