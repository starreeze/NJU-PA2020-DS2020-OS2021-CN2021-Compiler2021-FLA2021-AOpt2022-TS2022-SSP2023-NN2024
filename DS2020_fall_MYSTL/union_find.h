#ifndef UNION_FIND_H
#define UNION_FIND_H

#include<cstring>
class UnionFind {
    unsigned len; 
    unsigned* parent; 
    unsigned sets_num; 
public: 
    UnionFind(unsigned size): len(size), sets_num(size) {
        parent = (unsigned*)operator new(size * sizeof(unsigned)); 
        for(unsigned i = 0; i < size; ++i)
            parent[i] = i;  
    }
    UnionFind(const UnionFind& ufs): len(ufs.len), sets_num(ufs.sets_num) {
        parent = (unsigned*)operator new(len * sizeof(unsigned)); 
        memcpy(parent, ufs.parent, len * sizeof(unsigned));
    }
    unsigned get_parent(unsigned x) {
        if(parent[x] != x)   return parent[x] = get_parent(parent[x]);
        return x;
    }
    unsigned merge(unsigned a, unsigned b) {
        unsigned pa = get_parent(a), pb = get_parent(b); 
        if(pa == pb)    return -1; 
        --sets_num; 
        parent[pa] = pb; 
        return pa;
    }
    unsigned get_sets_num() const {return sets_num; }
}; 

#endif // UNION_FIND_H
