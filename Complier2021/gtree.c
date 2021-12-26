#include "gtree.h"
#include <stdio.h>

ValType float_val(const char* s) {
	ValType ret;
	ret.fval = atof(s);
	return ret;
}
ValType int_val(const char* s) {
	ValType ret;
	ret.ival = strtol(s, NULL, 0);
	return ret;
}
ValType string_val(const char* s) {
	ValType ret;
	ret.sval = (char*)malloc(strlen(s) + 1);
	strcpy(ret.sval, s);
	return ret;
}
ValType none_val(const char* s) {
	ValType ret;
	return ret;
}

// create new node and link children
pNode new_node(int line, const char* type, int count, ...) {
	assert(count);
	va_list vaList;
	va_start(vaList, count);
	// find first non-null arg -> tmp
	pNode tmp = NULL;
	int i = 0;
	for (; i < count; ++i) {
		tmp = va_arg(vaList, pNode);
		if (tmp)	break;
	}
	if (i == count) {
		va_end(vaList);
		return NULL;
	}
	// init & set child to tmp
	pNode ret = (pNode)malloc(sizeof(Node));
	ret->lineNo = line;
	ret->info.type = (char*)malloc(strlen(type) + 1);
	strcpy(ret->info.type, type);
	ret->next = NULL;
	ret->child = tmp;
	// link other children
	for (++i; i < count; ++i) {
		pNode next_tmp = va_arg(vaList, pNode);
		if (next_tmp) {
			tmp->next = next_tmp;
			tmp = next_tmp;
		}
	}
	va_end(vaList);
	return ret;
}

// create a new token node (leaf node in grammer tree)
pNode new_token_node(int line, ValType val, const char* type) {
	pNode ret = (pNode)malloc(sizeof(Node));
	ret->lineNo = line;
	ret->info.type = (char*)malloc(strlen(type) + 1);
	strcpy(ret->info.type, type);
	ret->info.val = val;
	ret->next = NULL;
	ret->child = NULL;
	return ret;
}

// print a single node
static void print_node(pNode pnode, int depth) {
	char* name = pnode->info.type;
	for (++depth; --depth;)
		printf("  ");
	printf("%s", name);
	if (pnode->child)
		printf(" (%d)", pnode->lineNo);
	else if (!strcmp(name, "TYPE") || !strcmp(name, "ID"))
		printf(": %s", pnode->info.val.sval);
	else if (!strcmp(name, "INT"))
		printf(": %d", pnode->info.val.ival);
	else if (!strcmp(name, "FLOAT"))
		printf(": %f", pnode->info.val.fval);
	printf("\n");
}
// print entire tree
void print_tree(pNode root, int depth) {
	if (!root)	return;
	print_node(root, depth);
	print_tree(root->child, depth + 1);
	print_tree(root->next, depth);
}

int idx, * lines = NULL;

bool unique(int line) {
	if (!lines) {
		lines = (int*)malloc(100 * sizeof(int));
		idx = 0;
	}
	for (int i = 0; i < idx; ++i)
		if (line == lines[i])
			return false;
	lines[idx++] = line;
	return true;
}
