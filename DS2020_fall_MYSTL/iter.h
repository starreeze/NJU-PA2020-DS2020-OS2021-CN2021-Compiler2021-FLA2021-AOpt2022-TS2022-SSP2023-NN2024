#ifndef ITER_H
#define ITER_H
#include"utils.h"
#include <string>
struct IterException {
    std::string description;
    IterException(const std::string& describe): description(describe) {}
};

#define CHECK_COMPARISON(iter_name) if(iter_name.obj != this->obj)\
    throw IterException("Comparison between iterators of different containers.");
/*#define CHECK_VALID if(!this->ref)\
    throw IterException("Iterator is invalid.");*/

template <class T, class Tv> // node_type and value_type
class Iter //iterator for sequence, supporting forward and backward
{
protected:
    T* ref = nullptr;
    const void* obj = nullptr;
public:
    Iter() {}
    Iter(T* pos, const void* container): ref(pos), obj(container) {}

    bool operator==(const Iter& iter) const;
    bool operator!=(const Iter& iter) const;
    bool same_container(const Iter& iter) const;

//     Tv& operator*() {return *ref; }
//     Tv* operator->() {return ref; }

//    virtual Iter& operator++() = 0;
//    virtual Tv& operator*() = 0;
//    virtual Tv* operator->() = 0;
};

template <class T, class Tv>
bool Iter<T, Tv>::operator==(const Iter &iter) const
{
    CHECK_COMPARISON(iter)
    return ref == iter.ref;
}

template <class T, class Tv>
bool Iter<T, Tv>::operator!=(const Iter &iter) const
{
    CHECK_COMPARISON(iter)
            return ref != iter.ref;
}

template<class T, class Tv>
bool Iter<T, Tv>::same_container(const Iter &iter) const
{
    return obj == iter.obj;
}


template <class T>
class RandomIter: public Iter<T, T>
{
public:
    RandomIter(T* pos, const void* container): Iter<T, T>(pos, container) {}
    T* operator->() {return this->ref;}
    T& operator*() {return *(this->ref);}
    RandomIter& operator+=(int bias);
    RandomIter& operator-=(int bias);
    bool operator<(RandomIter iter);
    bool operator<=(RandomIter iter);
    bool operator>(RandomIter iter);
    bool operator>=(RandomIter iter);
};

template<class T>
RandomIter<T> &RandomIter<T>::operator+=(int bias)
{
    this->ref += bias;
    return *this;
}

template<class T>
RandomIter<T> &RandomIter<T>::operator-=(int bias)
{
    this->ref -= bias;
    return *this;
}

template<class T>
bool RandomIter<T>::operator<(RandomIter iter)
{
    CHECK_COMPARISON(iter)
    return this->ref < iter.ref;
}

template<class T>
bool RandomIter<T>::operator<=(RandomIter iter)
{
    CHECK_COMPARISON(iter)
    return this->ref <= iter.ref;
}

template<class T>
bool RandomIter<T>::operator>(RandomIter iter)
{
    CHECK_COMPARISON(iter)
    return this->ref > iter.ref;
}

template<class T>
bool RandomIter<T>::operator>=(RandomIter iter)
{
    CHECK_COMPARISON(iter)
    return this->ref >= iter.ref;
}

#endif // ITER_H
