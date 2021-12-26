%option yylineno
%option noyywrap
%{
	#include <stdio.h>
	#include "gtree.h"
	#define YYSTYPE pNode
	#include "syntax.tab.h"

	extern void yyerror(char*);

    int yycolumn = 1;
    #define YY_USER_ACTION \
        yylloc.first_line = yylloc.last_line = yylineno; \
	    yylloc.first_column = yycolumn; \
	    yylloc.last_column = yycolumn + yyleng - 1; \
	    yycolumn += yyleng;
    
    bool print_lex = false;
	#define handle_token(type, name) \
		if(print_lex)	printf(#name"\n"); \
		yylval = new_token_node(yylineno, type##_val(yytext), #name); \
		return name;

	extern bool syn_error;
%}

digit [0-9]
letter [_a-zA-Z]
sign [+-]?
int_10 0|[1-9]+{digit}*
int_8 0[0-7]+
int_16 0[xX][0-9a-fA-F]+

INT {int_10}|{int_16}|{int_8}
FLOAT ({digit}*\.{digit}+|{digit}+\.?)[eE]{sign}{digit}+|({digit}*\.{digit}+|{digit}+\.)
TYPE int|float
ID {letter}({digit}|{letter})*
RELOP >|<|>=|<=|==|!=
BLANK [ \n\t\r]

%%

"//" {
	char c = input();
	while (c != '\n' && c)
		c = input();
}

"/*" {
	char pre = input(), next = input();
	while (next && !(pre == '*' && next == '/')) {
		pre = next;
		next = input();
	}
	if (!next) {
		printf("Error type B at line %d: comment unclosed at end of file.\n", yylineno);
		syn_error = true;
	}
}

{BLANK}	{;}
";"     { handle_token(none, SEMI); }
","     { handle_token(none, COMMA); }
"="     { handle_token(none, ASSIGNOP); }
"+"     { handle_token(string, PLUS); }
"-"     { handle_token(string, MINUS); }
"*"     { handle_token(string, STAR); }
"/"     { handle_token(string, DIV); }
"&&"    { handle_token(none, AND); }
"||"    { handle_token(none, OR); }
"."     { handle_token(none, DOT); }
"!"     { handle_token(none, NOT); }
"("     { handle_token(none, LP); }
")"     { handle_token(none, RP); }
"["     { handle_token(none, LB); }
"]"     { handle_token(none, RB); }
"{"     { handle_token(none, LC); }
"}"     { handle_token(none, RC); }
if      { handle_token(none, IF); }
else    { handle_token(none, ELSE); }
while   { handle_token(none, WHILE); }
struct  { handle_token(none, STRUCT); }
return  { handle_token(none, RETURN); }
{RELOP} { handle_token(string, RELOP); }
{TYPE}	{ handle_token(string, TYPE); }
{ID}	{ handle_token(string, ID); }
{INT}	{ handle_token(int, INT); }
{FLOAT}	{ handle_token(float, FLOAT);}

. {
	printf("Error type A at Line %d: mysterious characters \'%s\'.\n", yylineno, yytext);
	syn_error = true;
}
{digit}+{ID} {
    printf("Error type A at Line %d: Illegal id  \'%s\' .\n", yylineno, yytext); 
	syn_error = true;
}

%%
