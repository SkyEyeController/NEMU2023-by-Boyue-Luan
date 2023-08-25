#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>

enum
{
	NOTYPE = 256,EQ,NUM,REG,HEXN,NOTEQUAL,AND,OR,NOT,POINT,NEG
	/* TODO: Add more token types */

};

static struct rule
{
	char *regex;
	int token_type;
} rules[] = {

	/* TODO: Add more rules.
	 * Pay attention to the precedence level of different rules.
	 */

	{" +", NOTYPE},			// spaces
	{"\\+", '+'},			// plus
	{"==", EQ},				// equal
	{"0x[0-9,a-f]+", HEXN}, // hexnum
	{"[0-9]+", NUM},		// 10 num
	{"\\$[a-z]{2,3}", REG}, // reg
	{"\\(", '('},
	{"\\)", ')'},
	{"-", '-'},
	{"\\*", '*'},
	{"\\/", '/'},
	{"!=", NOTEQUAL},
	{"&&", AND},
	{"\\|\\|", OR},
	{"!", '!'},
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]))

static regex_t re[NR_REGEX]; // 存放编译后的正则表达式的空间

/* Rules are used for many times.
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
			Assert(ret == 0, "regex compilation failed: %s\n%s", error_msg, rules[i].regex);
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
	printf("%s\n", e);
	printf("%c", e[position]);
	while (e[position] != '\0')
	{
		/* Try all rules one by one. */
		for (i = 0; i < NR_REGEX; i++)
		{
			if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0)
			{
				char *substr_start = e + position;
				int substr_len = pmatch.rm_eo;

				Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s", i, rules[i].regex, position, substr_len, substr_len, substr_start);
				position += substr_len;

				/* TODO: Now a new token is recognized with rules[i]. Add codes
				 * to record the token in the array `tokens'. For certain types
				 * of tokens, some extra actions should be performed.
				 */
				if (substr_len < 32)
					switch (rules[i].token_type)
					{
					case NUM:
						tokens[nr_token].type = NUM;
						strncpy(tokens[nr_token].str, &e[position - substr_len], substr_len);
						nr_token++;
						break;
					case REG:
						tokens[nr_token].type = REG;
						strncpy(tokens[nr_token].str, &e[position - substr_len], substr_len);
						nr_token++;
						break;
					case HEXN:
						tokens[nr_token].type = HEXN;
						strncpy(tokens[nr_token].str, &e[position - substr_len], substr_len);
						nr_token++;
						break;
					case '+':
						tokens[nr_token++].type = '+';
						break;
					case '-':
						tokens[nr_token++].type = '-';
						break;
					case '*':
						tokens[nr_token++].type = '*';
						break;
					case '/':
						tokens[nr_token++].type = '/';
						break;
					case '(':
						tokens[nr_token++].type = '(';
						break;
					case ')':
						tokens[nr_token++].type = ')';
						break;
					case AND:
						tokens[nr_token].type = AND;
						strcpy(tokens[nr_token].str, "&&");
						nr_token++;
						break;
					case OR:
						tokens[nr_token].type = OR;
						strcpy(tokens[nr_token].str, "||");
						nr_token++;
						break;
					case EQ:
						tokens[nr_token].type = EQ;
						strcpy(tokens[nr_token].str, "==");
						nr_token++;
						break;
					case NOTEQUAL:
						tokens[nr_token].type = NOTEQUAL;
						strcpy(tokens[nr_token].str, "!=");
						nr_token++;
						break;
					case '!':
						tokens[nr_token].type = '!';
						nr_token++;
						break;
					case NOTYPE:
					break;
					default:
						assert(0);
					}
				else
				{
					printf("Buffer Overflow,system wrong!");
					return false;
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
bool check_parenthese(int p, int q)
{
	int l = 0, r = 0;
	if (tokens[p].type == '(' && tokens[q].type == ')')
	{
		int i = p;
		for (i = p; i <= q; i++)
		{
			if (tokens[i].type == '(')
				l++;
			if (tokens[i].type == ')')
				r++;
			if (i != q && l == r)
				return false;
		}
		if (l == r)
		{
			return true;
		}
		else
			return false;
	}
	return false;
}
uint32_t dominate_operator(int p, int q)
{
	int result = -1;
	int i = p;
	bool tokenflag = 0;
	for (i = p; i <= q; i++)
	{
		switch (tokens[i].type)
		{
		case '(':
			tokenflag = true;
			break;
		case ')':
			tokenflag = false;
			break;
		case '+':
		case '-':
			if (tokenflag)
				break;
			result = i;
			break;
		case '*':
		case '/':
			if (tokenflag)
				break;
			if (result == -1)
				result = i;
			if (tokens[result].type == '+' || tokens[result].type == '-')
				break;
			if (tokens[result].type == '*' || tokens[result].type == '/')
			{
				result = i;
				break;
			}
			break;
		}
	}
	if (tokenflag)
		result = -2;
	return result;
}
uint32_t eval(int p, int q)
{
	if (p > q)
	{
		/*Bad expression*/
		printf("Wrong caculate,system failed");
		assert(0);
	}
	else if (p == q)
	{
		/*Single token
		For now this token should be a number
		return the value of the number*/
		// check for the 10 or hex or reg
		int result = 0;
		if (tokens[p].type == NUM)
		{
			sscanf(tokens[p].str, "%d", &result);
			return result;
		}
		else if (tokens[p].type == HEXN) // hex num 0x123456
		{
			int i = 2;
			for (i = 2; tokens[p].str[i] != 0; i++)
			{
				result *= 16;
				result += tokens[p].str[i] <= '9' ? tokens[p].str[i] - '0' : tokens[p].str[i] - 'a' + 10;
			}
			return result;
		}
		else if (tokens[p].type == REG)
		{
			if (!strcmp(tokens[p].str, "$eax"))
			{
				return cpu.eax;
			}
			else if (!strcmp(tokens[p].str, "$ecx"))
			{
				return cpu.ecx;
			}
			else if (!strcmp(tokens[p].str, "$edx"))
			{
				return cpu.edx;
			}
			else if (!strcmp(tokens[p].str, "$ebx"))
			{
				return cpu.ebx;
			}
			else if (!strcmp(tokens[p].str, "$esp"))
			{
				return cpu.esp;
			}
			else if (!strcmp(tokens[p].str, "$ebp"))
			{
				return cpu.ebp;
			}
			else if (!strcmp(tokens[p].str, "$esi"))
			{
				return cpu.esi;
			}
			else if (!strcmp(tokens[p].str, "$edi"))
			{
				return cpu.edi;
			}
			else if (!strcmp(tokens[p].str, "$eip"))
			{
				return cpu.eip;
			}
			else
			{
				return 0;
			}
		}
		else
		{
			printf("hex or num or reg ERROR,system failed");
			assert(0);
		}
	}
	else if (check_parenthese(p, q))
		return eval(p + 1, q - 1);
	else // calc
	{
		int op = dominate_operator(p, q);
		printf("%d\n", op);
		if (op == -2)
			assert(0);
		else if (op == -1) // 指针解引用问题或负号问题
		{
			if (tokens[p].type == NEG)
			{
				int result = 0;
				sscanf(tokens[q].str, "%d", &result);
				return -result;
			}
			else if (tokens[p].type == POINT)
			{
				if (tokens[p].type == POINT)
				{
					int result = 0;
					if (!strcmp(tokens[p + 2].str, "$eax"))
					{
						result = swaddr_read(cpu.eax, 4);
						return result;
					}
					else if (!strcmp(tokens[p + 2].str, "$ecx"))
					{
						result = swaddr_read(cpu.ecx, 4);
						return result;
					}
					else if (!strcmp(tokens[p + 2].str, "$edx"))
					{
						result = swaddr_read(cpu.edx, 4);
						return result;
					}
					else if (!strcmp(tokens[p + 2].str, "$ebx"))
					{
						result = swaddr_read(cpu.ebx, 4);
						return result;
					}
					else if (!strcmp(tokens[p + 2].str, "$esp"))
					{
						result = swaddr_read(cpu.esp, 4);
						return result;
					}
					else if (!strcmp(tokens[p + 2].str, "$ebp"))
					{
						result = swaddr_read(cpu.ebp, 4);
						return result;
					}
					else if (!strcmp(tokens[p + 2].str, "$esi"))
					{
						result = swaddr_read(cpu.esi, 4);
						return result;
					}
					else if (!strcmp(tokens[p + 2].str, "$edi"))
					{
						result = swaddr_read(cpu.edi, 4);
						return result;
					}
					else if (!strcmp(tokens[p + 2].str, "$eip"))
					{
						result = swaddr_read(cpu.eip, 4);
						return result;
					}
				}
			}
		}
		else if (tokens[p].type == '!')
		{
			int result = 0;
			sscanf(tokens[q].str, "%d", &result);
			return !result;
		}

		int val1 = eval(p, op - 1);
		printf("%d\n",val1);
		int val2 = eval(op + 1, q);
		printf("%d\n",val2);
		switch (tokens[op].type)
		{
		case '+':
			return val1 + val2;
		case '-':
			return val1 - val2;
		case '*':
			return val1 * val2;
		case '/':
			return val1 / val2;
		case OR:
			return val1 || val2;
		case AND:
			return val1 && val2;
		case EQ:
			if (val1 == val2)
			{
				return 1;
			}
			else
			{
				return 0;
			}
		case NOTEQUAL:
			if (val1 != val2)
			{
				return 1;
			}
			else
			{
				return 0;
			}
		default:
			assert(0);
		}
	}
	return 0;
}

uint32_t expr(char *e, bool *success)
{
	if (!make_token(e))
	{
		*success = false;
		return 0;
	}
	int i;
	for (i = 0; i < nr_token; i++)
	{
		if (tokens[i].type == '*' && (i == 0 || (tokens[i - 1].type != NUM && tokens[i - 1].type != HEXN && tokens[i - 1].type != ')')))
		{
			tokens[i].type = POINT;
		}
		if (tokens[i].type == '-' && (i == 0 || (tokens[i - 1].type != NUM && tokens[i - 1].type != HEXN && tokens[i - 1].type != ')')))
		{
			tokens[i].type = NEG;
		}
	}
	return eval(0, nr_token - 1);

	/* TODO: Insert codes to evaluate the expression. */

	return 0;
}
