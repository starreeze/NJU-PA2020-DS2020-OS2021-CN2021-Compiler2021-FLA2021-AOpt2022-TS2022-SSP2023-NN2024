#ifndef GTREE_H
#define GTREE_H
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>
#include <string.h>

typedef union {
	int ival;
	float fval;
	char* sval;
} ValType;
ValType float_val(const char* s);
ValType int_val(const char* s);
ValType string_val(const char* s);
ValType none_val(const char* s);

typedef struct {
	char* type;
	ValType val;
} Info;

typedef struct node {
	int lineNo;
	Info info;
	struct node* child;
	struct node* next; // next node on the same depth
} Node;

typedef Node* pNode;

// create new node and link children
pNode new_node(int line, const char* type, int count, ...);

// create a new token node (leaf node in grammer tree)
pNode new_token_node(int line, ValType val, const char* type);

// print entire tree
void print_tree(pNode root, int depth);

#define bool unsigned char
#define true 1
#define false 0

extern int* lines, idx;
bool unique(int line);

#define HANDLE_ERROR(msg) \
	syn_error = true; \
	if(unique(yylineno))\
		printf("Error type B at line %d: syntax error in %s.\n", yylineno, msg);

#endif
