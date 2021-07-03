#include<iostream>
#include"vector.h"
using namespace std; 

Vector<int> tree(524288u, -1); 
#define LEFT(par) tree[par * 2 + 1]
#define RIGHT(par)  tree[par * 2 + 2]
void input_tree() {
    int n; cin >> n; 
    int t; cin >> t; 
    tree[0] = t; 
    while(--n) {
        cin >> t; 
        bool dir; // false: left
        unsigned p = 0; 
        while(true) {
            if(t < tree[p]) {
                if(LEFT(p) == -1) {
                    dir = false; break; 
                }
                p = p * 2 + 1; 
            }
            else if(RIGHT(p) == -1) {
                dir = true; break; 
            }
            else    p = p * 2 + 2; 
        }
        if(dir) RIGHT(p) = t; 
        else    LEFT(p) = t; 
    }
}

int C(int n, int k) {
    k = std::min(k, n-k); 
    int r = 1; 
    for(int i = 1; i <= k; ++i)
        r = r * (n + 1 - i) / i; 
    return r; 
}

int node_num; // another return value: number of nodes in subtree
int proc(int root) { // fill in nodeNum and calculate
    if(tree[root] == -1) {
        node_num = 0; return 1; 
    }
    int left_r = proc(root * 2 + 1), left_num = node_num; 
    int right_r = proc(root * 2 + 2), right_num = node_num; 
    node_num = left_num + right_num + 1; 
    return left_r * right_r * C(node_num - 1, left_num); 
}

int main() {
    ios::sync_with_stdio(false); 
    input_tree(); 
    cout << proc(0); 
    return 0; 
}
