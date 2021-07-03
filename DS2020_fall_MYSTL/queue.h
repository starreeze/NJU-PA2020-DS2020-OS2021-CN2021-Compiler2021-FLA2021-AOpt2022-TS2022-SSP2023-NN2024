#ifndef QUEUE_H
#define QUEUE_H
#include "list.h"

template <class T>
class Queue
{
#ifdef MYDEBUG
public:
#endif
    List<T> data;
public:
    unsigned size() {return data.size(); }
    T& front() {return *data.begin();}
    T pop() {T val = *data.begin(); data.del(data.begin()); return val;}
    Queue<T>& push(const T& ele) {data.insert_back(ele); return *this;}
    Queue<T>& clear() {data.clear(); return *this;}
};

#endif // QUEUE_H
