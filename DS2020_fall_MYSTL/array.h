#ifndef ARRAY_H
#define ARRAY_H
#include<cstring>
#include<cassert>
template<class T, unsigned len>
class Array {
    T* buf; 
public: 
    Array(): buf(new T[len]) {}
    Array(const T& val): buf(new T[len]) {
        for(unsigned i = 0; i < len; ++i)
            buf[i] = val; 
    }
    ~Array() {delete[] buf; }
    unsigned size() const {return len; }
    unsigned& operator[](unsigned idx) {return buf[idx]; }
    bool operator==(const Array& array) {
        if(array.size() != len) return false; 
        for(unsigned i = 0; i < len; ++i)
            if(buf[i] != array.buf[i])  return false; 
        return true; 
    }
    Array<T, len>& operator=(const Array& array) {
        assert(array.size() == len); 
        delete[] buf; 
        buf = new T[array.size()]; 
        for(unsigned i = 0; i < len; ++i)
            buf[i] = array.buf[i]; 
        return *this; 
    }
    Array<T, len>& operator=(Array&& array) {
        assert(array.size() == len);
        buf = array.buf; 
        return *this; 
    }
}; 
#endif
