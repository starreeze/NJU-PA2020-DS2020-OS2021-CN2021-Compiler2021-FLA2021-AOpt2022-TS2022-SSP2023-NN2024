#ifndef PQUEUE_H
#define PQUEUE_H

#include "vector.h"
template <class T, class Pred = Less<T>>
class PQueue //priority queue, implemeted using heap
{
    Vector<T> data;
    void shift_up();
    void shift_down();
    int parent(unsigned idx) {return (idx-1) / 2;}
    int left(unsigned idx) {return idx*2+1 >= data.size() ? -1 : idx*2+1;}
    int right(unsigned idx) {return idx*2+2 >= data.size() ? -1 : idx*2+2;}
public:
    void push(const T& val) {data.push_back(val); shift_up();}
    T pop() {T t = data[0]; data[0] = data[data.size()-1]; data.pop_back(); shift_down(); return t;}
    const T& top() const {return data[0];}
    void clear() {data.clear();}
    unsigned size() const {return data.size();}
};

template <class T, class Pred>
void PQueue<T, Pred>::shift_up()
{
    int p = data.size()-1;
    while(p) {
        int par = parent(p);
        if(Pred()(data[p], data[par])) {
            T t = data[p];
            data[p] = data[par];
            data[par] = t;
        }
        p = par;
    }
}

template <class T, class Pred>
void PQueue<T, Pred>::shift_down()
{
    int p = 0;
    while(true) {
        int l = left(p), r = right(p);
        if(l != -1 && Pred()(data[l], data[p]) && (r == -1 || Pred()(data[l], data[r]))) {
            T t = data[p];
            data[p] = data[l];
            data[l] = t;
            p = l;
        }
        else if(r != -1 && Pred()(data[r], data[p])) {
            T t = data[p];
            data[p] = data[r];
            data[r] = t;
            p = r;
        }
        else    break;
    }
}

#endif // PQUEUE_H
