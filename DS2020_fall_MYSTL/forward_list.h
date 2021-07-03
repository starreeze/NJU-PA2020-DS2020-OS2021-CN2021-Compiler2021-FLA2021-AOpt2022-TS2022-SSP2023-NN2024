#ifndef FORWARD_LIST_H
#define FORWARD_LIST_H

#include"iter.h"
template<class T>
struct ForwardNode { // single linked list without headNode
    T val; 
    ForwardNode* next; 
    ForwardNode(const T& value, ForwardNode* nextNode = nullptr): val(value), next(nextNode) {}
}; 

template<class T>
class ForwardList {
    ForwardNode<T>* head = nullptr; 
    unsigned len = 0; 
public: 
    class Iterator: public Iter<ForwardNode<T>, T> {
    public: 
        Iterator(ForwardNode<T>* p, ForwardList* container)
            : Iter<ForwardNode<T>, T>(p, container) {}
        Iterator& operator++() { Iter<ForwardNode<T>, T>::ref = Iter<ForwardNode<T>, T>::ref->next; return *this; }
        T& operator*() {return Iter<ForwardNode<T>, T>::ref->val; }
        T* operator->() {return &(Iter<ForwardNode<T>, T>::ref->val); }
        friend class ForwardList; 
    };
    ~ForwardList(); 
    unsigned size() const {return len; }
    Iterator begin() {return Iterator(head, this); }
    Iterator end() {return Iterator(nullptr, this); }
    Iterator insert_after(Iterator it, const T& val); 
    void push_front(const T& val); 
    void erase_after(Iterator it); 
    T pop_front(); 
    void clear(); 
};

template<class T>
typename ForwardList<T>::Iterator ForwardList<T>::insert_after(Iterator it, const T& val) {
    it.ref->next = new ForwardNode<T>(val, it.ref->next); 
    ++len; 
    return ++it; 
}

template<class T>
ForwardList<T>::~ForwardList() {
    while(head) {
        auto p = head->next; 
        delete head; 
        head = p; 
    }
}

template<class T>
void ForwardList<T>::erase_after(Iterator it) {
    ForwardNode<T>* itNextNext = it.ref->next->next; 
    delete it.ref->next; 
    it.ref->next = itNextNext;
    --len; 
}

template<class T>
void ForwardList<T>::clear() {
    this->~ForwardList(); 
    len = 0; 
    head = nullptr; 
}

template<class T>
void ForwardList<T>::push_front(const T& val) {
    head = new ForwardNode<T>(val, head); 
    ++len; 
}

template<class T>
T ForwardList<T>::pop_front() {
    T r = head->val;  
    ForwardNode<T>* headNext = head->next; 
    delete head; 
    head = headNext; 
    --len; 
    return r; 
}

#endif // FORWARD_LIST_H
