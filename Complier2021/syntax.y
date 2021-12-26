%{
	#include <stdio.h>
	#include "gtree.h"
    #define YYSTYPE pNode
    pNode root = NULL;
    bool syn_error = false;
    extern int yylineno;
    extern int yylex(void);
    void yyerror(char* msg) {}
    
    int yydebug = 1;
%}

%token INT
%token FLOAT
%token ID
%token TYPE
%token COMMA
%token DOT
%token SEMI
%token RELOP
%token ASSIGNOP
%token PLUS MINUS STAR DIV
%token AND OR NOT 
%token LP RP LB RB LC RC
%token IF
%token ELSE
%token WHILE
%token STRUCT
%token RETURN

%right ASSIGNOP
%left OR
%left AND
%left RELOP
%left PLUS MINUS
%left STAR DIV
%right NOT
%left DOT
%left LB RB
%left LP RP
%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE


%%
// set syntax unit with empty string to NULL
Program:    ExtDefList                   { $$ = new_node(@$.first_line, "Program", 1, $1); root = $$; }
    ; 
ExtDefList: ExtDef ExtDefList            { $$ = new_node(@$.first_line, "ExtDefList", 2, $1, $2); }
    |                                    { $$ = NULL; } 
    ; 
ExtDef:     Specifier ExtDecList SEMI    { $$ = new_node(@$.first_line, "ExtDef", 3, $1, $2, $3); }
    |       Specifier SEMI               { $$ = new_node(@$.first_line, "ExtDef", 2, $1, $2); }
    // function declaration in lab2
    |       Specifier FunDec SEMI        { $$ = new_node(@$.first_line, "ExtDef", 3, $1, $2, $3); }
    |       Specifier FunDec CompSt      { $$ = new_node(@$.first_line, "ExtDef", 3, $1, $2, $3); }
    |       error SEMI                   { $$ = NULL; HANDLE_ERROR("global definition"); }
    |       Specifier error              { $$ = NULL; HANDLE_ERROR("global definition"); }
    ; 
ExtDecList: VarDec                       { $$ = new_node(@$.first_line, "ExtDecList", 1, $1); }
    |       VarDec COMMA ExtDecList      { $$ = new_node(@$.first_line, "ExtDecList", 3, $1, $2, $3); }
    ; 

Specifier:  TYPE                         { $$ = new_node(@$.first_line, "Specifier", 1, $1); }
    |       StructSpecifier              { $$ = new_node(@$.first_line, "Specifier", 1, $1); }
    ; 
StructSpecifier:    STRUCT OptTag LC DefList RC  { $$ = new_node(@$.first_line, "StructSpecifier", 5, $1, $2, $3, $4, $5); }
    |       STRUCT Tag                   { $$ = new_node(@$.first_line, "StructSpecifier", 2, $1, $2); }
    ; 
OptTag:     ID                           { $$ = new_node(@$.first_line, "OptTag", 1, $1); }
    |                                    { $$ = NULL; }
    ; 
Tag:        ID                           { $$ = new_node(@$.first_line, "Tag", 1, $1); }
    ; 

VarDec:     ID                           { $$ = new_node(@$.first_line, "VarDec", 1, $1); }
    |       VarDec LB INT RB             { $$ = new_node(@$.first_line, "VarDec", 4, $1, $2, $3, $4); }
    |       error RB                     { $$ = NULL; HANDLE_ERROR("variable description"); }
    ; 
FunDec:     ID LP VarList RP             { $$ = new_node(@$.first_line, "FunDec", 4, $1, $2, $3, $4); }
    |       ID LP RP                     { $$ = new_node(@$.first_line, "FunDec", 3, $1, $2, $3); }
    |       ID LP error RP               { $$ = NULL; HANDLE_ERROR("function description"); }
    ; 
VarList:    ParamDec COMMA VarList       { $$ = new_node(@$.first_line, "VarList", 3, $1, $2, $3); }
    |       ParamDec                     { $$ = new_node(@$.first_line, "VarList", 1, $1); }
    ; 
ParamDec:   Specifier VarDec             { $$ = new_node(@$.first_line, "ParamDec", 2, $1, $2); }
    ; 
CompSt:     LC DefList StmtList RC       { $$ = new_node(@$.first_line, "CompSt", 4, $1, $2, $3, $4); }
    |       error RC                     { $$ = NULL; HANDLE_ERROR("function body"); }
    ; 
StmtList:   Stmt StmtList                { $$ = new_node(@$.first_line, "StmtList", 2, $1, $2); }
    |                                    { $$ = NULL; }
    ;
Stmt:       Exp SEMI                     { $$ = new_node(@$.first_line, "Stmt", 2, $1, $2); }
    |       CompSt                       { $$ = new_node(@$.first_line, "Stmt", 1, $1); }
    |       RETURN Exp SEMI              { $$ = new_node(@$.first_line, "Stmt", 3, $1, $2, $3); }    
    |       IF LP Exp RP Stmt %prec LOWER_THAN_ELSE  { $$ = new_node(@$.first_line, "Stmt", 5, $1, $2, $3, $4, $5); }
    |       IF LP Exp RP Stmt ELSE Stmt  { $$ = new_node(@$.first_line, "Stmt", 7, $1, $2, $3, $4, $5, $6, $7); }
    |       WHILE LP Exp RP Stmt         { $$ = new_node(@$.first_line, "Stmt", 5, $1, $2, $3, $4, $5); }
    |       error SEMI                   { $$ = NULL; HANDLE_ERROR("statement"); }
    |       IF LP error RP Stmt %prec LOWER_THAN_ELSE{ $$ = NULL; HANDLE_ERROR("statement"); }
    |       IF LP error RP Stmt ELSE Stmt{ $$ = NULL; HANDLE_ERROR("statement"); }
    |       WHILE LP error RP Stmt       { $$ = NULL; HANDLE_ERROR("statement"); }
    ;
DefList:    Def DefList                  { $$ = new_node(@$.first_line, "DefList", 2, $1, $2); }
    |                                    { $$ = NULL; }
    ;
Def:        Specifier DecList SEMI       { $$ = new_node(@$.first_line, "Def", 3, $1, $2, $3); }
    |       Specifier error SEMI         { $$ = NULL; HANDLE_ERROR("local definition"); }
    |       Specifier DecList error      { $$ = NULL; HANDLE_ERROR("local definition"); }
    ; 
DecList:    Dec                          { $$ = new_node(@$.first_line, "DecList", 1, $1); }
    |       Dec COMMA DecList            { $$ = new_node(@$.first_line, "DecList", 3, $1, $2, $3); }
    ; 
Dec:        VarDec                       { $$ = new_node(@$.first_line, "Dec", 1, $1); }
    |       VarDec ASSIGNOP Exp          { $$ = new_node(@$.first_line, "Dec", 3, $1, $2, $3); }
    ; 
Exp:        Exp ASSIGNOP Exp             { $$ = new_node(@$.first_line, "Exp", 3, $1, $2, $3); }
    |       Exp AND Exp                  { $$ = new_node(@$.first_line, "Exp", 3, $1, $2, $3); }
    |       Exp OR Exp                   { $$ = new_node(@$.first_line, "Exp", 3, $1, $2, $3); }
    |       Exp RELOP Exp                { $$ = new_node(@$.first_line, "Exp", 3, $1, $2, $3); }
    |       Exp PLUS Exp                 { $$ = new_node(@$.first_line, "Exp", 3, $1, $2, $3); }
    |       Exp MINUS Exp                { $$ = new_node(@$.first_line, "Exp", 3, $1, $2, $3); }
    |       Exp STAR Exp                 { $$ = new_node(@$.first_line, "Exp", 3, $1, $2, $3); }
    |       Exp DIV Exp                  { $$ = new_node(@$.first_line, "Exp", 3, $1, $2, $3); }
    |       LP Exp RP                    { $$ = new_node(@$.first_line, "Exp", 3, $1, $2, $3); }
    |       MINUS Exp                    { $$ = new_node(@$.first_line, "Exp", 2, $1, $2); }
    |       NOT Exp                      { $$ = new_node(@$.first_line, "Exp", 2, $1, $2); }
    |       ID LP Args RP                { $$ = new_node(@$.first_line, "Exp", 4, $1, $2, $3, $4); }
    |       ID LP RP                     { $$ = new_node(@$.first_line, "Exp", 3, $1, $2, $3); }
    |       Exp LB Exp RB                { $$ = new_node(@$.first_line, "Exp", 4, $1, $2, $3, $4); }
    |       Exp DOT ID                   { $$ = new_node(@$.first_line, "Exp", 3, $1, $2, $3); }
    |       ID                           { $$ = new_node(@$.first_line, "Exp", 1, $1); }
    |       INT                          { $$ = new_node(@$.first_line, "Exp", 1, $1); }
    |       FLOAT                        { $$ = new_node(@$.first_line, "Exp", 1, $1); }
    ; 
Args :      Exp COMMA Args               { $$ = new_node(@$.first_line, "Args", 3, $1, $2, $3); }
    |       Exp                          { $$ = new_node(@$.first_line, "Args", 1, $1); }
    ; 
%%

