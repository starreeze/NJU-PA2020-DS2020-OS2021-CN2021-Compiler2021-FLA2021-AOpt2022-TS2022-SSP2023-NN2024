#include"vector.h"
#include"pqueue.h"
#include"union_find.h"
#define size mat.size()
class MatGraph {
    Vector<Vector<int>> mat; // adjacent matrix
public: 
    MatGraph(unsigned s, int w = INT_MAX): mat(Vector<Vector<int>>(s, Vector<int>(s))) {
        for(unsigned i = 0; i < s; ++i) {
            for(unsigned j = 0; j < s; ++j) {
                if(j != i)  mat[i][j] = w; 
                else    mat[i][j] = 0;
            }
        }
    }
    void connect(unsigned a, unsigned b, int w) {
        mat[a][b] = mat[b][a] = w; 
    }
    int weight(unsigned a, unsigned b) {return mat[a][b]; }
    unsigned min_path(unsigned a, unsigned b, Vector<int>& path); 
    void min_paths(unsigned a, unsigned b, Vector<Vector<unsigned>>& pre); 
        // Paths is represented in a table(x[i] is i's possible previous node). Use dfs to get a path
}; 

unsigned MatGraph::min_path(unsigned a, unsigned b, Vector<int>& path) {
    auto dist = mat[a]; 
    Vector<bool> ready(size, false); 
    for(unsigned i = 0; i < size; ++i) {
        if(i != a && dist[i] != INT_MAX)    path[i] = a; 
        else    path[i] = -1; 
    }
    ready[a] = true; 
    for(unsigned i = 0; i < size-1; ++i) {
        unsigned min_node = 0, len = INT_MAX; 
        for(unsigned j = 1; j < size; ++j)
            if(!ready[j] && (unsigned)dist[j] < len) {
                min_node = j; len = dist[j]; 
            }
        ready[min_node] = true; 
        for(unsigned j = 0; j < size; ++j) {
            int w = mat[min_node][j]; 
            if(!ready[j] && w >= 0 && (unsigned)dist[min_node] + w < (unsigned)dist[j]) {
                path[j] = min_node; 
                dist[j] = dist[min_node] + w; 
            }
        }
    }
    return dist[b]; 
}

void MatGraph::min_paths(unsigned a, unsigned b, Vector<Vector<unsigned>>& pre) {
    Vector<bool> ready(size, false);
    auto dist = mat[a]; 
    for(unsigned i = 0; i < size; ++i)
        if(i != a && dist[i] != INT_MAX)
            pre[i].push_back(a);
    ready[a] = true; 
    while(true) {
        unsigned min_node = 0, len = INT_MAX;
        for(unsigned j = 1; j < size; ++j)
            if(!ready[j] && (unsigned)dist[j] < len) {
                min_node = j; len = dist[j];
            }
        if(len == INT_MAX)  break;  
        ready[min_node] = true;
        for(unsigned j = 0; j < size; ++j) {
            int w = mat[min_node][j];
            if(!ready[j] && (unsigned)dist[min_node] + w < (unsigned)dist[j]) {
                pre[j].clear(); pre[j].push_back(min_node); 
                dist[j] = dist[min_node] + w;
            }
            else if(!ready[j] && (unsigned)dist[min_node] + w == (unsigned)dist[j])
                pre[j].push_back(min_node); 
        }
    }
}
#undef size

class TableGraph {
    struct Edge {
        unsigned a, b; // node
        int w; // weight
        Edge(unsigned u, unsigned v, unsigned weight): a(u), b(v), w(weight) {}
        bool operator<(const Edge& e) {return w < e.w; }
    }; 
    Vector<Edge> edges; 
    Vector<Vector<unsigned>> table; // adjacent table, refering to an element in edges
public: 
    static Vector<Edge> min_tree(Vector<Edge>& edges, unsigned size); 
    TableGraph(unsigned n): table(n) {}
    void connect(unsigned u, unsigned v, int w = INT_MAX) {
        table[u].push_back(edges.size()); 
        table[v].push_back(edges.size()); 
        edges.push_back(Edge(u, v, w)); 
    }
    Vector<Edge> min_tree() {
        auto edge = edges; 
        return min_tree(edge, table.size());
    } 
    unsigned min_path(unsigned u, unsigned v, Vector<int>& path); 
}; 

Vector<TableGraph::Edge> TableGraph::min_tree(Vector<Edge>& edge, unsigned size) {
    Vector<Edge> r(size-1); 
    quick_sort(edge.begin(), edge.end()); 
    UnionFind ufs(size); 
    unsigned j = 0; 
    for(unsigned i = 0; i < edge.size(); ++i) {
        auto& e = edge[i];
        if(ufs.merge(e.a, e.b) != UINT_MAX)
            r[j++] = e; 
    }
    return r; 
}

unsigned TableGraph::min_path(unsigned u, unsigned v, Vector<int>& path) {
    PQueue<unsigned> pque; 
    pque.push(u); 
    Vector<unsigned> dist(table.size(), INT_MAX); 
    dist[u] = 0; 
    while(pque.size()) {
        unsigned node = pque.pop(); 
        if(node == v)   break; 
        auto& adjs = table[node]; 
        for(unsigned i = 0; i < adjs.size(); ++i) {
            auto& edge = edges[adjs[i]]; 
            unsigned w = edge.w, v = (edge.a == node ? edge.b : edge.a); 
            if(dist[node] + w < dist[v]) {
                dist[v] = dist[node] + w; 
                path[v] = node; 
                pque.push(v); 
            }
        }
    }
    return dist[v]; 
}
