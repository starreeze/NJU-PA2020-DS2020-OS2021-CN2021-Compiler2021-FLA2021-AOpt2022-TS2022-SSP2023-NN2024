#include <stdlib.h>
#include <stdio.h>

#include "gtree.h"
#include "syntax.tab.h"

extern pNode root;

extern bool syn_error;
extern int yyparse();
extern void yyrestart(FILE*);
extern void traverse_gtree(Node* root);
extern void print_intercodes();
extern void optimize_intercode();
extern void gen_asm();
extern void print_asm();

int main(int argc, char** argv) {
	FILE* f = fopen(argv[1], "r");
	if (!f) {
		perror(argv[1]);
		return 1;
	}
	yyrestart(f);
	yyparse();
	if (!syn_error) {
		// print_tree(root, 0);
		traverse_gtree(root);
#ifdef OPTIMIZE
		optimize_intercode();
#endif
		if (argc > 2 && !strcmp(argv[2], "-i"))
			print_intercodes();
		else {
			gen_asm();
			print_asm();
		}
	}
	return 0;
}
