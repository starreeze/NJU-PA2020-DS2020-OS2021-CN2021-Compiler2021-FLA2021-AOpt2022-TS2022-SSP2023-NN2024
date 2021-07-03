#ifndef UNORDERED_MAP_H
#define UNORDERED_MAP_H
#include"unordered_set.h"
#define MAP_TEMPLATE template<class Key, class Val, class KeyHash, class KeyPred>
#define SET_INSTANCE <Pair<Key, Val>, MapPairHash<Key, Val, KeyHash>, MapPairEqual<Key, Val, KeyPred>>

template<class Key, class Val, class KeyHash>
struct MapPairHash {
    unsigned long operator()(const Pair<Key, Val>& t) {
        return KeyHash()(t.first); 
    }
}; 

template<class Key, class Val, class KeyPred>
struct MapPairEqual {
    unsigned long operator()(const Pair<Key, Val>& a, const Pair<Key, Val>& b) {
        return KeyPred()(a.first, b.first); 
    }
};


template<class Key, class Val, class KeyHash = DefaultHash<Key>, class KeyPred = EqualTo<Key>>
class UnorderedMap: public UnorderedSet SET_INSTANCE {
public: 
    UnorderedMap(unsigned size): UnorderedSet SET_INSTANCE(size) {}
    typename UnorderedSet SET_INSTANCE::Iterator find(const Key& key); 
    Val& operator[](const Key& key) {return find(key)->second; }
}; 

MAP_TEMPLATE
typename UnorderedSet SET_INSTANCE::Iterator UnorderedMap<Key, Val, KeyHash, KeyPred>::find(const Key& key) {
    return UnorderedSet SET_INSTANCE::find(Pair<Key, Val>(key, Val())); 
}

template<class Key, class Val, class KeyHash = DefaultHash<Key>, class KeyPred = EqualTo<Key>>
class UnorderedMultimap: public UnorderedMultiset SET_INSTANCE {
public:
    UnorderedMultimap(unsigned size): UnorderedMultiset SET_INSTANCE(size) {}
    typename UnorderedMultiset SET_INSTANCE::Iterator find(const Key& key);
};

MAP_TEMPLATE
typename UnorderedMultiset SET_INSTANCE::Iterator UnorderedMultimap<Key, Val, KeyHash, KeyPred>::find(const Key& key) {
    return UnorderedMultiset SET_INSTANCE::find(Pair<Key, Val>(key, Val()));
}

#endif // UNORDERED_MAP_H
