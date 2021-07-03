#ifndef VECTOR_H
#define VECTOR_H
#include <cstring>
#include <cassert>
#include "iter.h"

template <class T> class Vector;

template<class T>
class VectorIter: public RandomIter<T>
{
public:
    VectorIter(T* pos, const Vector<T>* container): RandomIter<T>(pos, container) {}
    VectorIter& operator++();
    VectorIter& operator--();
    VectorIter operator+(unsigned bias);
    VectorIter operator-(unsigned bias);
    int operator-(VectorIter iter);
};

template<class T>
VectorIter<T> &VectorIter<T>::operator++()
{
    ++this->ref;
    return *this;
}

template<class T>
VectorIter<T> &VectorIter<T>::operator--()
{
    --this->ref;
    return *this;
}

template<class T>
VectorIter<T> VectorIter<T>::operator+(unsigned bias)
{
    VectorIter r(*this);
    r += bias;
    return r;
}

template<class T>
VectorIter<T> VectorIter<T>::operator-(unsigned bias)
{
    VectorIter r(*this);
    r -= bias;
    return r;
}

template<class T>
int VectorIter<T>::operator-(VectorIter iter)
{
    CHECK_COMPARISON(iter)
    return this->ref - iter.ref;
}

unsigned get_capacity(unsigned curlen)
{
    --curlen;
    unsigned exp = 0;
    for(; curlen; ++exp, curlen >>= 1);
    return exp; 
}

template <class T>
class Vector
{
    unsigned len = 0, capacity = 1;
    T* buf = (T*)operator new(sizeof(T));
    void expand_to(unsigned newlen); //expand to at least newlen(elements) and copy
public:
    Vector() {}
    Vector(unsigned initial_len);
    Vector(unsigned initial_len, const T& element);
    Vector(const T* pos, unsigned inputlen);
    template <class seqIter>
    Vector(seqIter b, seqIter e);
    template <class seqIter>
    Vector(seqIter b, unsigned inputlen);

    Vector(const Vector& v);
    Vector(Vector&& v): len(v.len), capacity(v.capacity), buf(v.buf) {}

    ~Vector() {
        for(unsigned i = 0; i < len; ++i)   buf[i].~T();
        //free(buf);
    }

    Vector& operator=(const Vector& v);
    Vector operator+(const Vector& v) const; //concat
    Vector& operator+=(const Vector& v);

    VectorIter<T> begin() const {return VectorIter<T>(buf, this);}
    VectorIter<T> end() const {return VectorIter<T>(buf + len, this);}
    T& operator[](unsigned idx) const {
        assert(idx < len);
        return buf[idx];
    }
    T& back() {return buf[len-1]; }
    unsigned size() const {return len;}

    void push_back(const T& ele);
    T pop_back() {--len; return *end();}
    void insert(VectorIter<T> pos, const T& ele); //insert before pos
    template <class seqIter>
    unsigned insert(VectorIter<T> pos, seqIter b, seqIter e); // return the number of elements inserted
    template <class seqIter>
    void insert(VectorIter<T> pos, seqIter b, unsigned inputlen);
    VectorIter<T> erase(VectorIter<T> pos) {return erase(pos, pos+1);}
    VectorIter<T> erase(VectorIter<T> b, VectorIter<T> e);

    void clear();
    void reserve(unsigned tarlen) {if (tarlen > capacity) expand_to(tarlen);}
};


template<class T>
void Vector<T>::expand_to(unsigned newlen)
{
    capacity = 1 << get_capacity(newlen);
    T* tmp = (T*)operator new(capacity*sizeof(T)); 
    std::memcpy(tmp, buf, len * sizeof(T));
    delete[] buf; 
    buf = tmp;
}

template<class T>
Vector<T>::Vector(unsigned initial_len): len(initial_len),
    capacity(1 << get_capacity(initial_len)), buf((T*)operator new(capacity*sizeof(T))) {}

template<class T>
Vector<T>::Vector(unsigned initial_len, const T &element): len(initial_len),
    capacity(1 << get_capacity(initial_len)), buf((T*)operator new(capacity*sizeof(T)))
{
    for(unsigned i = 0; i < initial_len; ++i)
        new(&buf[i]) T(element); 
}

template<class T>
Vector<T>::Vector(const Vector &v): len(v.len), capacity(v.capacity),
    buf((T*)operator new(capacity*sizeof(T)))
{
    for(unsigned i = 0; i < len; ++i)
        new(&buf[i]) T(v[i]); 
}

template<class T>
template<class seqIter>
Vector<T>::Vector(seqIter b, seqIter e)
{
    for(; b != e; ++b)
        push_back(*b);
}

template<class T>
template<class seqIter>
Vector<T>::Vector(seqIter b, unsigned inputlen): len(inputlen),
    capacity(1 << get_capacity(inputlen)), buf((T*)operator new(capacity*sizeof(T)))
{
    for(unsigned i=0; i<inputlen; ++i, ++b)
        new(&buf[i]) T(*b); 
}

template<class T>
Vector<T> &Vector<T>::operator=(const Vector &v)
{
    len = v.len; capacity = v.capacity;
    delete[] buf; 
    buf = (T*)operator new(capacity*sizeof(T)); 
    for(unsigned i = 0; i < len; ++i)
        new(&buf[i]) T(v[i]); 
    return *this;
}

template<class T>
Vector<T> Vector<T>::operator+(const Vector &v) const
{
    Vector r(*this);
    r += v;
    return r;
}

template<class T>
Vector<T> &Vector<T>::operator+=(const Vector &v)
{
    unsigned futurelen = len + v.len;
    if(futurelen > capacity)
        expand_to(futurelen);
    for(unsigned i = 0; i < v.len; ++i)
        buf[len + i] = v.buf[i];
    return *this;
}

template<class T>
VectorIter<T> Vector<T>::erase(VectorIter<T> b, VectorIter<T> e)
{
    int deleted = e - b;
    for(auto p = b; e != end(); ++p, ++e)
        *p = *e;
    len -= deleted;
    return b + 1;
}

template<class T>
void Vector<T>::clear()
{
    len = 0;
}

template<class T>
void Vector<T>::insert(VectorIter<T> pos, const T &ele)
{
    return insert(pos, VectorIter<T>(const_cast<int*>(&ele), nullptr), 1u);
}

template<class T>
void Vector<T>::push_back(const T &ele)
{
    if(len + 1 > capacity) expand_to(len + 1);
    buf[len++] = ele;
}

template<class T>
template<class seqIter>
unsigned Vector<T>::insert(VectorIter<T> pos, seqIter b, seqIter e)
{
    Vector<T> tmp(b, e);
    insert(pos, tmp.begin(), tmp.size());
    return tmp.size();
}

template<class T>
template<class seqIter>
void Vector<T>::insert(VectorIter<T> pos, seqIter b, unsigned inputlen)
{
    unsigned extend = inputlen;
    if(len + inputlen > capacity) {
        int bias = pos - begin();
        expand_to(len + inputlen);
        pos = begin() + bias;
    }
    auto stop = end();
    for(auto t = stop - 1; t >= pos; --t)
        *(t + inputlen) = *t;
    for(auto t = pos; inputlen--; ++t, ++b)
        *t = *b;
    len += extend;
}

#endif // VECTOR_H
