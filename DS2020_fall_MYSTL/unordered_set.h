#ifndef UNORDERED_SET_H
#define UNORDERED_SET_H
#include "vector.h"
#include "forward_list.h"

template<class T>
struct DefaultHash {
    unsigned long operator()(const T& val) {
//         unsigned long* p = (unsigned long*)&val; 
        return (unsigned long)val; 
    }
}; 

template<class T>
struct EqualTo {
    bool operator()(const T& a, const T& b) {return a == b; }
}; 

template<class T, class hash = DefaultHash<T>, class Pred = EqualTo<T>>
class UnorderedSet {
        unsigned address(const T& val) const; 
        unsigned len = 0, capacity; // 2 ^ capacity
        Vector<ForwardList<T>> data /* = Vector<ForwardList<T>>(1 << default_cap) */ ; 
    public: 
        class Iterator {
            unsigned idx; // end: idx == len
            typename ForwardList<T>::Iterator listIt; 
            UnorderedSet* obj;
            public: 
            Iterator(unsigned index, typename ForwardList<T>::Iterator listIter, UnorderedSet* container)
                : idx(index), listIt(listIter), obj(container) {}
            T operator*() {return *listIt; }
            typename ForwardList<T>::Iterator operator->() {return listIt; }
            Iterator& operator++();
            bool operator==(const Iterator& it);
            bool operator!=(const Iterator& it) {return !(*this == it);}
            friend class UnorderedSet; 
        }; 
        //     UnorderedSet() {}
        UnorderedSet(unsigned size): capacity(get_capacity(size) + 1), data(1 << capacity) {} // reserve capacity
        template<class It>
            UnorderedSet(It b, It e) {
                while(b != e) {
                    insert(*b); 
                    ++b; 
                }
            }
        unsigned size() const {return len; }
        void insert(const T& val); 
        Iterator find(const T& val); 
        void erase(const T& val) {erase(find(val)); }
        void erase(Iterator pos); 
        Iterator begin(); 
        Iterator end() {return Iterator(data.size(), data[0].end(), this); } 
}; 

    template<class T, class hash, class Pred>
bool UnorderedSet<T, hash, Pred>::Iterator::operator==(const UnorderedSet::Iterator &it)
{
    return idx == it.idx && (idx == obj->data.size() || listIt == it.listIt);
}


    template<class T, class hash, class Pred>
typename UnorderedSet<T, hash, Pred>::Iterator &UnorderedSet<T, hash, Pred>::Iterator::operator++()
{
    if(idx == obj->data.size()) throw IterException("Iterator out of range!\n");
    if(++listIt == obj->data[idx].end()) {
        while(++idx < obj->data.size() && !obj->data[idx].size());
        if(idx < obj->data.size()) // != end()
            listIt = obj->data[idx].begin();
    }
    return *this; 
}

template<class T, class hash, class Pred>
unsigned UnorderedSet<T, hash, Pred>::address(const T& val) const {
    // square mid method
    unsigned long num = hash()(val); 
    num = num * num; 
    unsigned digits = 0; 
    for(unsigned t = num; t; ++digits, t >>= 1); 
    return (num >> (digits > capacity ? (digits - capacity) / 2 : 0)) & ((1 << capacity) - 1); 
}

template<class T, class hash, class Pred>
void UnorderedSet<T, hash, Pred>::insert(const T &val)
{
    assert((len + 1) * 2 <= data.size()); 
    ForwardList<T>& bucket = data[address(val)]; 
    auto p = bucket.begin(); 
    for(; p != bucket.end() && !Pred()(*p, val); ++p); 
    if(p == bucket.end()) {
        data[address(val)].push_front(val); 
        ++len; 
    }
    else    *p = val; 
}

template<class T, class hash, class Pred>
typename UnorderedSet<T, hash, Pred>::Iterator UnorderedSet<T, hash, Pred>::find(const T& val) {
    unsigned addr = address(val); 
    ForwardList<T>& bucket = data[addr]; 
    auto p = bucket.begin();
    for(; p != bucket.end() && !Pred()(*p, val); ++p); 
    if(p == bucket.end())   return end(); 
    return Iterator(addr, p, this); 
}

template<class T, class hash, class Pred>
void UnorderedSet<T, hash, Pred>::erase(typename UnorderedSet<T, hash, Pred>::Iterator pos) {
    ForwardList<T>& bucket = data[pos.idx]; 
    if(bucket.begin() == pos.listIt)    bucket.pop_front(); 
    else {
        auto p = bucket.begin(), q = p; 
        for(++q; q != bucket.end(); p = q, ++q)
            if(q == pos.listIt) {
                bucket.erase_after(p); 
                break; 
            }
    }
    --len; 
}

template<class T, class hash, class Pred>
typename UnorderedSet<T, hash, Pred>::Iterator UnorderedSet<T, hash, Pred>::begin() {
    for(unsigned i = 0; i < data.size(); ++i)
        if(data[i].size())  return Iterator(i, data[i].begin(), this); 
    return end(); 
}


template<class T, class hash = DefaultHash<T>, class Pred = EqualTo<T>>
class UnorderedMultiset {
        unsigned address(const T& val) const; 
        unsigned len = 0, capacity; // 2 ^ capacity
        Vector<ForwardList<T>> data /* = Vector<ForwardList<T>>(1 << default_cap) */ ; 
    public: 
        class Iterator {
            unsigned idx; // end: idx == len
            typename ForwardList<T>::Iterator listIt; 
            UnorderedMultiset* obj;
            public: 
            Iterator(unsigned index, typename ForwardList<T>::Iterator listIter, UnorderedMultiset* container)
                : idx(index), listIt(listIter), obj(container) {}
            T operator*() {return *listIt; }
            typename ForwardList<T>::Iterator operator->() {return listIt; }
            Iterator& operator++();
            bool operator==(const Iterator& it);
            bool operator!=(const Iterator& it) {return !(*this == it);}
            friend class UnorderedMultiset; 
        }; 
        //     UnorderedMultiset() {}
        UnorderedMultiset(unsigned size): capacity(get_capacity(size) + 1), data(1 << capacity) {} // reserve capacity
        template<class It>
            UnorderedMultiset(It b, It e) {
                while(b != e) {
                    insert(*b); 
                    ++b; 
                }
            }
        unsigned size() const {return len; }
        void insert(const T& val); 
        Iterator find(const T& val); 
        void erase(const T& val) {erase(find(val)); }
        void erase(Iterator pos); 
        Iterator begin(); 
        Iterator end() {return Iterator(data.size(), data[0].end(), this); } 
}; 

    template<class T, class hash, class Pred>
bool UnorderedMultiset<T, hash, Pred>::Iterator::operator==(const UnorderedMultiset::Iterator &it)
{
    return idx == it.idx && (idx == obj->data.size() || listIt == it.listIt);
}


    template<class T, class hash, class Pred>
typename UnorderedMultiset<T, hash, Pred>::Iterator &UnorderedMultiset<T, hash, Pred>::Iterator::operator++()
{
    if(idx == obj->data.size()) throw IterException("Iterator out of range!\n");
    if(++listIt == obj->data[idx].end()) {
        while(++idx < obj->data.size() && !obj->data[idx].size());
        if(idx < obj->data.size()) // != end()
            listIt = obj->data[idx].begin();
    }
    return *this; 
}

template<class T, class hash, class Pred>
unsigned UnorderedMultiset<T, hash, Pred>::address(const T& val) const {
    // square mid method
    unsigned long num = hash()(val); 
    num = num * num; 
    unsigned digits = 0; 
    for(unsigned t = num; t; ++digits, t >>= 1); 
    return (num >> (digits > capacity ? (digits - capacity) / 2 : 0)) & ((1 << capacity) - 1); 
}

    template<class T, class hash, class Pred>
void UnorderedMultiset<T, hash, Pred>::insert(const T &val)
{
    if((len + 1) * 2 > data.size()) {
        assert(0); 
        // expand
        UnorderedMultiset<T, hash, Pred> temp(len * 2); 
        for(auto p = begin(); p != end(); ++p)
            temp.insert(*p); 
        temp.insert(val); 
        *this = temp; 
    }
    else {
        data[address(val)].push_front(val); 
        ++len; 
    }
}

template<class T, class hash, class Pred>
typename UnorderedMultiset<T, hash, Pred>::Iterator UnorderedMultiset<T, hash, Pred>::find(const T& val) {
    unsigned addr = address(val); 
    ForwardList<T>& bucket = data[addr]; 
    auto p = bucket.begin();
    for(; p != bucket.end() && !Pred()(*p, val); ++p); 
    if(p == bucket.end())   return end(); 
    return Iterator(addr, p, this); 
}

template<class T, class hash, class Pred>
void UnorderedMultiset<T, hash, Pred>::erase(typename UnorderedMultiset<T, hash, Pred>::Iterator pos) {
    ForwardList<T>& bucket = data[pos.idx]; 
    if(bucket.begin() == pos.listIt)    bucket.pop_front(); 
    else {
        auto p = bucket.begin(), q = p; 
        for(++q; q != bucket.end(); p = q, ++q)
            if(q == pos.listIt) {
                bucket.erase_after(p); 
                break; 
            }
    }
    --len; 
}

template<class T, class hash, class Pred>
typename UnorderedMultiset<T, hash, Pred>::Iterator UnorderedMultiset<T, hash, Pred>::begin() {
    for(unsigned i = 0; i < data.size(); ++i)
        if(data[i].size())  return Iterator(i, data[i].begin(), this); 
    return end(); 
}

#endif // UNORDERED_SET_H
