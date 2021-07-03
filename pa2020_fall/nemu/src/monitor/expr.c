#include "nemu.h"
#include "cpu/reg.h"
#include "memory/memory.h"

#include <stdlib.h>

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>

enum
{
    EQ = 256,
    AND,
    OR,
    LEFT_SHIFT,
    RIGHT_SHIFT,
    GE,
    LE,
	NOTYPE,
	NUM,
	REG,
	SYMB,
    HEX
};

static struct rule
{
	char *regex;
	int token_type;
} rules[] = {

	/* TODO: Add more rules.
	 * Pay attention to the precedence level of different rules.
	 */

	{" +", NOTYPE}, // white space
	{"&&", AND},
	{"\\|\\|", OR},
	{"==", EQ},
	{"<<", LEFT_SHIFT},
	{">>", RIGHT_SHIFT},
	{"\\+", '+'},
	{"-", '-'},
	{"\\*", '*'},
	{"/", '/'},
	{"%", '%'},
	{"&", '&'},
	{"\\^", '^'},
	{"\\|", '|'},
	{"!", '!'},
	{"~", '~'},
	{">", '>'},
	{"<", '<'},
	{"0x([0-9]|[a-f])+", HEX},
	{"[0-9]+", NUM},
	{"\\(", '('},
	{"\\)", ')'},
	{"\\$([a-z]|_)+", REG},
	{"([a-z]|[A-Z]|_)([a-z]|[A-Z]|_|[0-9])*", SYMB}
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]))

static regex_t re[NR_REGEX];

/* Rules are used for more times.
 * Therefore we compile them only once before any usage.
 */
void init_regex()
{
	int i;
	char error_msg[128];
	int ret;

	for (i = 0; i < NR_REGEX; i++)
	{
		ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
		if (ret != 0)
		{
			regerror(ret, &re[i], error_msg, 128);
			assert(ret != 0);
		}
	}
}

typedef struct token
{
	int type;
	char str[32];
} Token;

Token tokens[32];
int nr_token;

static bool make_token(char *e)
{
	int position = 0;
	int i;
	regmatch_t pmatch;

	nr_token = 0;

	while (e[position] != '\0')
	{
		/* Try all rules one by one. */
		for (i = 0; i < NR_REGEX; i++)
		{
			if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0)
			{
				char *substr_start = e + position;
				int substr_len = pmatch.rm_eo;

				//printf("match regex[%d] at position %d with len %d: %.*s\n", i, position, substr_len, substr_len, substr_start);
				position += substr_len;

				/* TODO: Now a new token is recognized with rules[i]. 
				 * Add codes to perform some actions with this token.
				 */
                if(substr_len > 31) {
                    printf("too long expression!\n");
                    return false;
                }
                
				switch (rules[i].token_type)
				{
				case NOTYPE:
				    break;
				default:
					tokens[nr_token].type = rules[i].token_type;
					char* i = tokens[nr_token].str;
					char* j = substr_start;
					for(int k = 0; k < substr_len; ++k) {
					    *(i++) = *(j++);
					}
					*i = 0;
					nr_token++;
				}

				break;
			}
		}

		if (i == NR_REGEX)
		{
			printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
			return false;
		}
	}

	return true;
}

// typedef struct {
//     int buf[100];
//     int top;
// } Stack;
// void init_stack(Stack* st) {
//     st.top = -1;
// }
// void push(Stack* st, int x) {
//     st.buf[++st.top] = x;
// }
// int pop(Stack* st) {
//     return st.buf[st.top--];
// }
// int top(Stack* st) {
//     return st.buf[st.top];
// }
// bool empty(Stack* st) {
//     return st.top == -1;
// }

int par_match(Token* tokens, int num, int left_pos) {
    // return the position of the matched right
    int left = 1;
    for(int i = left_pos + 1; i < num; ++i) {
        if(tokens[i].type == '(')
            ++left;
        else if(tokens[i].type == ')')
            if(!--left)    return i;
    }
    return -1;
}

bool chk_par(Token* tokens, int num, bool* success) {
    if(tokens[0].type != '(')   return false;
    int right_pos = par_match(tokens, num, 0);
    if(right_pos < 0) {
        *success = false;
        return false;
    }
    return right_pos == num-1;
}

unsigned char op_order[263];
bool op_prior(int a, int b) {
    return op_order[a] >= op_order[b];
}

extern CPU_STATE cpu;
extern uint32_t look_up_symtab(char *sym, bool *success);
uint32_t eval(Token* tokens, int32_t num, bool* success) {
    if(!*success)   return 0;
    if(num <= 0)    assert(0);
    if(num == 1) {
        switch(tokens->type) {
            case NUM:   return atoi(tokens->str);
            case HEX:   return strtol(tokens->str, NULL, 16);
            case SYMB:  return look_up_symtab(tokens->str, success);
            case REG:
                if(!strcmp(tokens->str + 1, "eax")) return cpu.eax;
                if(!strcmp(tokens->str + 1, "ebx")) return cpu.ebx;
                if(!strcmp(tokens->str + 1, "ecx")) return cpu.ecx;
                if(!strcmp(tokens->str + 1, "edx")) return cpu.edx;
                if(!strcmp(tokens->str + 1, "edi")) return cpu.edi;
                if(!strcmp(tokens->str + 1, "esi")) return cpu.esi;
                if(!strcmp(tokens->str + 1, "ebp")) return cpu.ebp;
                if(!strcmp(tokens->str + 1, "esp")) return cpu.esp;
                if(!strcmp(tokens->str + 1, "eip")) return cpu.eip;
#ifdef IA32_SEG
                if(!strcmp(tokens->str + 1, "cs")) return cpu.cs.val;
                if(!strcmp(tokens->str + 1, "ds")) return cpu.ds.val;
                if(!strcmp(tokens->str + 1, "ss")) return cpu.ss.val;
                if(!strcmp(tokens->str + 1, "es")) return cpu.es.val;
                if(!strcmp(tokens->str + 1, "gdtr_base")) return cpu.gdtr.base;
                if(!strcmp(tokens->str + 1, "gdtr_limit")) return cpu.gdtr.limit;
                if(!strcmp(tokens->str + 1, "cr0")) return cpu.cr0.val;
#endif
#ifdef IA32_PAGE
                if(!strcmp(tokens->str + 1, "cr3")) return cpu.cr3.val;
#endif
            default:
                *success = false;
                return 0;
        }
    }
    if(chk_par(tokens, num, success))
        return eval(tokens + 1, num - 2, success);
        
    //search for dominant operator
    int pos = 0; int op = 0;
    for(int i = 0; i < num-1; ++i) {
        if(tokens[i].type == '(') {
            i = par_match(tokens, num, i) + 1;
            if(i >= num-1)  break;
        }
        if(tokens[i].type < 263 && op_prior(op, tokens[i].type)) {
            op = tokens[i].type;
            pos = i;
        }
    }
    
    if(!pos || (tokens[pos-1].type != NUM && tokens[pos-1].type != HEX
      && tokens[pos-1].type != REG && tokens[pos-1].type != SYMB && tokens[pos-1].type != ')')) {
        //single-oprand operator
        uint32_t val = eval(tokens + pos + 1, num - pos - 1, success);
        switch(op) {
            case '-':   return -val;
            case '*':   return vaddr_read(val, 0, 4);
            case '!':   return !val;
            case '~':   return ~val;
            default:
                *success = false;
                return 0;
        }
    } else {
        //double-oprand operator
        uint32_t val1 = eval(tokens, pos, success);
        uint32_t val2 = eval(tokens + pos + 1, num - pos - 1, success);
        switch(op) {
            case '+':   return val1 + val2;
            case '-':   return val1 - val2;
            case '*':   return val1 * val2;
            case '/':   return val1 / val2;
            case '%':   return val1 % val2;
            case '&':   return val1 & val2;
            case '|':   return val1 | val2;
            case '^':   return val1 ^ val2;
            case '>':   return val1 > val2;
            case '<':   return val1 < val2;
            case AND:   return val1 && val2;
            case OR:    return val1 || val2;
            case EQ:    return val1 == val2;
            case LEFT_SHIFT:    return val1 << val2;
            case RIGHT_SHIFT:   return val1 >> val2;
            default:
                *success = false;
                return 0;
        }
    }
    return 0;
}

uint32_t expr(char *e, bool *success)
{
    //**************
    // if(e[0] == '0' && e[1] == 'x') {
    //     *success = true;
    //     int r = 0;
    //     for(int i = 2; e[i]; ++i, r <<= 4) {
    //         if(e[i] <= '9')
    //             r += e[i] - '0';
    //         else
    //             r += e[i] - 'a' + 10;
    //     }
    //     return r >> 4;
    // }
    //**************
    *success = true;
	if (!make_token(e))
	{
		*success = false;
		return 0;
	}
	//operator priority order
	op_order[0] = 10;
	op_order['!'] = 9;
	op_order['~'] = 9;
    op_order['*'] = 8;
    op_order['/'] = 8;
    op_order['%'] = 8;
    op_order['+'] = 7;
    op_order['-'] = 7;
    op_order[LEFT_SHIFT] = 6;
    op_order[RIGHT_SHIFT] = 6;
    op_order['>'] = op_order['<'] = op_order[GE] = op_order[LE] = 5;
    op_order['&'] = 4;
    op_order['^'] = 3;
    op_order['|'] = 2;
    op_order[AND] = 1;
    op_order[OR] = 0;
	return eval(tokens, nr_token, success);

// 	printf("\nPlease implement expr at expr.c\n");
// 	fflush(stdout);
// 	assert(0);
}
