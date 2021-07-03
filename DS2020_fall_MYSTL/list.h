#ifndef LIST_H
#define LIST_H
#include "iter.h"

template <class T>
struct Node
{
    T val;
    Node<T>* next;
    Node<T>* pre;
    Node(const T& value = T(), Node<T>* previous = nullptr, Node<T>* nextNode = nullptr)
        : val(value), next(nextNode), pre(previous) {}
};

template <class T>
class List;

template <class T>
class ListIter: public Iter<Node<T>, T>
{
public:
    ListIter(Node<T>* pos, const List<T>* obj): Iter<Node<T>, T>(pos, obj) {}
    ListIter& operator++();
    ListIter& operator--();
    T& operator*();
    T* operator->();

    friend class List<T>;
};

template<class T>
ListIter<T>& ListIter<T>::operator++()
{
    this->ref = this->ref->next;
    return *this;
}

template<class T>
ListIter<T>& ListIter<T>::operator--()
{
    this->ref = this->ref->pre;
    return *this;
}

template<class T>
T &ListIter<T>::operator*()
{
    return this->ref->val;
}

template<class T>
T *ListIter<T>::operator->()
{
    return &(this->ref->val);
}


template <class T>
class List
{
    Node<T>* head = new Node<T>;
    Node<T>* tail = head;
    unsigned len = 0;
    void insert_ptr(Node<T>* pos, const T& value);
public:
    List() {}
    List(T* begin, unsigned size); //construct from sequence
    List(const List& list);
    ~List();

    List& operator=(const List& list);
    unsigned size() const {return len;}

    ListIter<T> begin() const {return ListIter<T>(head->next, this);}
    ListIter<T> end() const {return ListIter<T>(tail->next, this);}
    ListIter<T> last() const {return ListIter<T>(tail, this);}

    List& insert(ListIter<T> pos, const T& value); //insert before pos
    List& insert_back(const T& value);
    List& insert_front(const T& value);
    Node<T>* del(ListIter<T> pos); //delete pos and return next element
    void clear();

    List operator+(const List& list); //concat two lists
};

template<class T>
void List<T>::insert_ptr(Node<T> *pos, const T &value)
{
    ++len;
    pos->next = new Node<T>(value, pos, pos->next);
    if(pos->next->next)
        pos->next->next->pre = pos->next;
    else    tail = pos->next;
}

template<class T>
List<T>::List(T *b, unsigned size)
{
    for(++size; --size; ++b)
        insert_back(*b);
}

template<class T>
List<T>::List(const List &list) :len(0), head(new Node<T>)
{
    for(auto p = list.head->next; p; p = p->next) {
        insert_back(p->val);
    }
}

template<class T>
List<T>::~List()
{
    for(auto p = head; p;) {
        auto q = p->next;
        delete p;
        p = q;
    }
}

template<class T>
List<T> &List<T>::operator=(const List &list)
{
    if(list.len < len) {
        auto i = begin(), j = list.begin();
        for(; j != list.end(); ++i,++j)
            *i = *j;
        while(list.len < len)
            del(last());
    }
    else {
        auto i = begin(), j = list.begin();
        for(; i != end(); ++i,++j)
            *i = *j;
        for(; j != list.end(); ++j)
            insert_back(*j);
    }
}

template<class T>
Node<T> *List<T>::del(ListIter<T> pos)
{
    --len;
    auto p = pos.ref->pre;
    p->next = pos.ref->next;
    if(pos.ref->next)
        pos.ref->next->pre = p;
    else    tail = p;
    delete pos.ref;
    return p->next;
}

template<class T>
void List<T>::clear()
{
    len = 0;
    auto h = head->next;
    while(h) {
        auto t = h->next;
        delete h;
        h = t;
    }
    head->next = nullptr;
}

template<class T>
List<T> List<T>::operator+(const List &list)
{
    List<T> r(*this);
    for(auto t = list.begin(); t != list.end(); ++t)
        r.insert_back(*t);
    return r;
}

template<class T>
List<T> &List<T>::insert_front(const T &value)
{
    insert_ptr(head, value);
    return *this;
}

template<class T>
List<T> &List<T>::insert_back(const T &value)
{
    insert_ptr(tail, value);
    return *this;
}

template<class T>
List<T> &List<T>::insert(ListIter<T> pos, const T &value)
{
    insert_ptr(pos.ref->pre, value);
    return *this;
}

#endif // LIST_H
