#ifndef STACK_H
#define STACK_H
#include "vector.h"

template <class T>
class Stack
{
#ifdef MYDEBUG
public:
#endif
    Vector<T> data;
public:
    unsigned size() {return data.size();}
    T& top() {return *(data.end() - 1);}
    T pop() {return data.pop_back();}
    Stack<T>& push(const T& ele) {data.push_back(ele); return *this;}
    Stack<T>& clear() {data.clear(); return *this;}
    Stack<T>& reserve(unsigned size) {data.reserve(size); return *this;}
};

#endif // STACK_H
