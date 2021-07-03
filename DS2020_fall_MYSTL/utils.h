#ifndef UTILS_H
#define UTILS_H
#include<iostream>
// pair
template<class T1, class T2>
struct Pair {
    T1 first; 
    T2 second; 
    Pair() {}
    Pair(const T1& f, const T2& s): first(f), second(s) {}
    bool operator==(const Pair& t) const {return first == t.first && second == t.second; }
    bool operator!=(const Pair& t) const {return !operator==(t); }
}; 
template<class T1, class T2>
Pair<T1, T2> to_pair(const T1& v1, const T2& v2) {
    return Pair<T1, T2>(v1, v2); 
}

// template less and more
template<class T>
struct Less
{
    bool operator()(const T& a, const T& b) const {
        return a < b;
    }
};
template<class T>
struct More {
    bool operator()(const T& a, const T& b) const {
        return a > b; 
    }
};

// min and max
template<class T>
T min(const T& a, const T& b) {return a < b ? a : b; }
template<class T>
T max(const T& a, const T& b) {return a > b ? a : b; }
template<class T>
T min(std::initializer_list<T> lst) {
    T r = *lst.begin();
    auto p = lst.begin(); 
    for(++p; p != lst.end(); ++p)
        if(*p < r)  r = *p; 
    return r; 
}
template<class T>
T max(std::initializer_list<T> lst) {
    T r = *lst.begin();
    auto p = lst.begin();
    for(++p; p != lst.end(); ++p)
        if(*p > r)  r = *p;
    return r;
}
template<class Iter>
Iter min_element(Iter begin, Iter end) {
    auto val = *begin, r = begin; 
    for(++begin; begin != end; ++begin)
        if(*begin < val) {
            r = begin; val = *begin; 
        }
    return r; 
}
template<class Iter>
Iter max_element(Iter begin, Iter end) {
    auto val = *begin, r = begin; 
    for(++begin; begin != end; ++begin)
        if(*begin > val) {
            r = begin; val = *begin; 
        }
    return r; 
}

// swap
template<class P>
void swap(P a, P b) {
    auto t = *a; 
    *a = *b; 
    *b = t; 
}

// binary find
template<class RanIter, class Tv>
RanIter binary_find(RanIter l, RanIter r, const Tv& val) {
    RanIter end = r; 
    while(l < r) {
        RanIter mid = l + (r-l) / 2;
        if(val < *mid)  r = mid;
        else if(*mid < val) l = mid + 1;
        else    return mid;
    }
    return end;
}

template<class RanIter, class Tv, class Pred>
RanIter binary_find(RanIter l, RanIter r, const Tv& val, Pred comp) {
    RanIter end = r; 
    while(l < r) {
        RanIter mid = l + (r-l) / 2; 
        if(comp(val, *mid))  r = mid; 
        else if(comp(*mid, val)) l = mid + 1; 
        else    return mid; 
    }
    return end; 
}

// sort
template<class RanIter, class Pred>
void insert_sort(RanIter l, RanIter r, Pred comp) {
    for(auto p = l + 1; p != r; ++p)    if(comp(*p, *(p-1))) {
        auto rp = l; 
        for(; comp(*rp, *p); ++rp);  
        auto t = *p; 
        for(auto tp = p; tp != rp; --tp)    *tp = *(tp-1); 
        *rp = t; 
    }
}
template<class RanIter>
void insert_sort(RanIter l, RanIter r) {
    for(auto p = l + 1; p != r; ++p)    if(*p < *(p-1)) {
        auto rp = l; 
        for(; *rp < *p; ++rp); 
        auto t = *p; 
        for(auto tp = p; tp != rp; --tp)    *tp = *(tp-1); 
        *rp = t; 
    }
}

template<class RanIter>
RanIter quick_sort_partition(RanIter l, RanIter r){
    RanIter i = l, j = --r; 
    auto x = *l;
    while (i < j) {
        while(i < j && !(*j < x))   --j;
        if(i < j) {
            *i = *j; ++i; 
        }
        while(i < j && !(x < *i))   ++i;
        if(i < j) {
            *j = *i; --j; 
        }
    }
    *i = x; 
    return i;
}
template<class RanIter, class Pred>
RanIter quick_sort_partition(RanIter l, RanIter r, Pred comp){
    RanIter i = l, j = --r; 
    auto x = *l;
    while (i < j) {
        while(i < j && !comp(*j, x))   --j;
        if(i < j) {
            *i = *j; ++i; 
        }
        while(i < j && !comp(x, *i))   ++i;
        if(i < j) {
            *j = *i; --j; 
        }
    }
    *i = x; 
    return i;
}
template<class RanIter>
void quick_sort(RanIter l, RanIter r)
{
    if (r-l < 16)   return insert_sort(l, r);    
    auto i = quick_sort_partition(l, r);
    quick_sort(l, i);
    quick_sort(i + 1, r);
}
template<class RanIter, class Pred>
void quick_sort(RanIter l, RanIter r, Pred comp)
{
    if (r-l < 16)   return insert_sort(l, r, comp);
//     if(r-l < 2) return; 
    auto i = quick_sort_partition(l, r, comp);
    quick_sort(l, i, comp);
    quick_sort(i + 1, r, comp);
}

// print
template<class It>
void print_container(It l, It r) {
    if(l != r) {
        std::cout << *l; 
        for(++l; l != r; ++l)
            std::cout << ' ' << *l; 
    }
    std::cout << '\n'; 
}

// int
constexpr unsigned UINT_MAX = 0xffffffff; 
constexpr int INT_MAX = 0x7fffffff; 
constexpr int INT_MIN = 0x80000000;
#endif // UTILS_H
