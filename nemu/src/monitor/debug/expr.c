#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>

enum {

  TK_NOTYPE = 256, TK_EQ,
  /*TODO: Add more token types */
  /*Finished*/
  TK_AND, TK_OR, TK_NOT, TK_DEC, TK_HEX, 
  TK_REG,  TK_MINUS, TK_POINTER
};

static struct rule {
  char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},						// spaces
  {"\\+", '+'},								// plus
  {"\\-", '-'},								// minus
  {"\\*", '*'},								// mult
  {"/", '/'},								// divide
  {"\\(", '('},								// left parenthesis
  {"\\)", ')'},								// right parenthesis
  {"&&", TK_AND},							// and
  {"\\|\\|", TK_OR},						// or
  {"!", TK_NOT},							// not
  {"0[xX][0-9a-fA-F]+", TK_HEX},			// hexadecimal
  {"[0-9][0-9]*", TK_DEC},					// decimal
  {"\\$[a-zA-Z][a-zA-Z]+", TK_REG},			// register 
  {"==", TK_EQ}								// equal
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )

static regex_t re[NR_REGEX];
int invalid;	//1 when regex is invalid

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

Token tokens[32];
int nr_token;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
	    char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);
        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */
        if(rules[i].token_type == TK_NOTYPE)	break;
		tokens[nr_token].type = rules[i].token_type;
        switch (rules[i].token_type){
		  /* Put numbers into tokens. */
          case TK_DEC: case TK_HEX:{
		    strncpy(tokens[nr_token].str, substr_start, substr_len);
			tokens[nr_token].str[substr_len] = '\0';
			break;
		  }
		  /* Removing the $ before the register */
		  case TK_REG:{
		    strncpy(tokens[nr_token].str, substr_start + 1, substr_len - 1);
			tokens[nr_token].str[substr_len - 1] = '\0';
			break;
		  }
		  default:{
		    ;
		  } 
		  
        }
        ++nr_token;
        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}


/* TODO: Check if surrounded by parentheses */
bool check_parentheses(int p, int q) {
  int count = 0;
  bool not_match = false;
  for(int i = p; i <= q; i++) {
    if(tokens[i].type == '(')	++count;
    else if(tokens[i].type == ')'){
      --count;
      if(count < 0) {
        printf("Parentheses does not match.\n");
		invalid = 1;
        return 0;
      }
      else if (count == 0 && i != q){
        not_match = true;
      }
    }
  }
  
  if(count > 0){
    printf("Parentheses does not match.\n");
    invalid = 1;
	return 0;
  }
  if(tokens[p].type == '('  && tokens[q].type == ')' && !not_match)
    return true;
  else	return false;
}

/* TODO: Check if the token is an operator*/
bool is_operator(int type) {
  switch(type) {
    case '+': case '-': case '*': case '/':
    case TK_EQ: case TK_AND: case TK_OR: 
    case TK_MINUS: case TK_POINTER:
      return true;
    default:
      return false;
  }
}

/* TODO: Check the operator is in an parentheses*/
bool inside_par(int pos, int begin, int end) {
  int count = 0;
  while(begin < pos) {
    if(tokens[begin].type == '(')	++count;
    else if (tokens[begin].type == ')')	--count;
    ++begin;
  }
  if (count > 0) return true;
  else return false;
}

/* TODO: Check priority of the operator. */
int check_priority(int type1, int type2){
  int priority1, priority2;
  switch(type1){
    case TK_NOT: case TK_POINTER: case TK_MINUS:{
	  priority1 = 1;
	  break;
	}
	case '*': case '/':{
	  priority1 = 2;
	  break;
    }
    case '+': case '-':{
	  priority1 = 3;
	  break;
	}
	case TK_EQ:{
	  priority1 = 4;
	  break;
	}
	case TK_AND:{
	  priority1 = 5;
	  break;
	}
	case TK_OR:{
	  priority1 = 6;
	  break;
	}
	default: assert(0);  
  }
  switch(type2){
    case TK_NOT: case TK_POINTER: case TK_MINUS:{
	  priority2 = 1;
	  break;
	}
	case '*': case '/':{
	  priority2 = 2;
	  break;
	}
    case '+': case '-':{
	  priority2 = 3;
	  break;
	}
	case TK_EQ:{
	  priority2 = 4;
	  break;
	}
	case TK_AND:{
	  priority2 = 5;
	  break;
	}
	case TK_OR:{
	  priority2 = 6;
	  break;
	}
	default: assert(0);  
  }
  return priority1 - priority2;
}

enum{LEFT, RIGHT};

/* TODO: Check associativity. */
int associate(int type){
  switch(type) {
	case '+': case '-': case '*': case '/':
	case TK_EQ: case TK_AND: case TK_OR:
	  return LEFT;
	case TK_NOT: case TK_POINTER: case TK_MINUS:
      return RIGHT;
	default:	return 3;
  }
}

/* TODO: Find the dominant operator in an expression. */
int dominant_operator(int p, int q) {
  for(int ix = p; ix <= q; ix++){
    printf("%s ", tokens[ix].str);
  }
  printf("\n");	
  int i = p;
  while((i <= q && !is_operator(tokens[i].type)) || inside_par(i, p, q)){
    ++i;
  }
  if(i > q) {
    printf("Syntax Error.\n");
	invalid = 1;
	return 0;
  }
  int dominant_pos = i;
  for(++i; i <= q; ++i){
    if(!is_operator(tokens[i].type) || inside_par(i, p, q))
	  continue;

	  int cur_type = tokens[i].type;
	  int dominant_type = tokens[dominant_pos].type;
	  if(check_priority(cur_type, dominant_type) > 0 ||
		  (check_priority(cur_type, dominant_type) == 0 && 
		  associate(cur_type) == LEFT))
	    dominant_pos = i;
  }
  return dominant_pos;
}

/* TODO: Read the content in the register*/
uint32_t read_reg(char *reg_name){
  for(char *pstr = reg_name; *pstr != '\0'; ++pstr){
    *pstr = tolower(*pstr);
  }
  int i;
  for(i = R_EAX; i <= R_EDI; i++){
    /* searching through eax-edi */
	if(!strcmp(reg_name, regsl[i])){
	  return reg_l(i);
	}  
  }
  for(i = R_AX; i <= R_DI; i++){
    /* searching through ax-di */
	if(!strcmp(reg_name, regsw[i])){
	  return reg_w(i);
	}
  }
  for(i = R_AL; i <= R_BH; i++){
    /* searching through al-bh */
	if(!strcmp(reg_name, regsb[i])){
	  return reg_b(i);
	}
  }
  if(!strcmp(reg_name, "eip")){
    return cpu.eip;
  }
  else{
    printf("Wrong register input.\n");
	invalid = 1;
	return 0;
  }
}



/* TODO: Evaluating the expression. */
uint32_t eval(int p, int q) {
 if(p > q) {
    /* Bad Expression */
    printf("Bad Expression.\n");
	pause();
	invalid = 1;
    return 0;
  }
  else if (p == q) {
    /* Single Token */
	switch(tokens[p].type){
	  case TK_DEC:{	//when the token is a decimal number
	    return atoi(tokens[p].str);
	  }
	  case TK_HEX:{
	    uint32_t temp;
        sscanf(tokens[p].str, "%x", &temp);
	    return temp;
 	  }
	  case TK_REG:{
	    return read_reg(tokens[p].str);
 	  }
	}
  }
  else if (check_parentheses(p, q) == true) {
    return eval(p + 1, q - 1);
  }
  /*else if (check_parentheses(p, q) == false) {
    invalid = 1;
	return 0;
  }*/
  else {
    if(tokens[p].type == '*'){
	  tokens[p].type = TK_POINTER;
	}
	if(tokens[p].type == '-'){
	  printf("asd\n");
	  tokens[p].type = TK_MINUS;
	}
	uint32_t  op = dominant_operator(p, q);
    uint32_t  op_type = tokens[op].type;
	if(op_type != TK_MINUS || op_type != TK_POINTER)
		printf("%c\n",op_type);
	else if(op_type == TK_MINUS)
		printf("m\n");
	else if(op_type == TK_POINTER)
		printf("p\n");
	if(invalid == 1)	return 0;
    if (p == op) {
      uint32_t val1 = eval(op + 1, q);
      switch(op_type) {
        case TK_MINUS:		{return -val1; break;}
        case TK_POINTER:	{return vaddr_read(val1, 4);break;}
        default:;
	  }
    }
    else {
      uint32_t val1 = eval(p, op - 1);
      uint32_t val2 = eval(op + 1, q);
      switch(op_type) {
        case '+':		return val1 + val2;
        case '-':		return val1 - val2;
        case '*':		return val1 * val2;
        case '/':		return val1 / val2;
		case TK_AND:	return val1 && val2;
		case TK_OR:		return val1 || val2; 
        default:		assert(0);
	  }
    }
  }
  printf("error.\n");
  invalid = 1;
  return 0;
}

uint32_t expr(char *e, bool *success) {
  if(!make_token(e)) {
	printf("dsa");
    *success = false;
    return 0;
  }

  /* TODO: Insert codes to evaluate the expression. */
  invalid = 0;
  
  /* Step 1: Distinguishing '*' and '-'. */
  if(tokens[0].type == '*'){
    tokens[0].type = TK_POINTER;
  }
  if(tokens[0].type == '-'){
    tokens[0].type = TK_MINUS;
  }
  for(int i = 1; i < nr_token; i++){
    int prev_type = tokens[i - 1].type;
	bool is_num = (prev_type == TK_DEC) || (prev_type == TK_HEX) || 
	              (prev_type == TK_REG) || (prev_type == ')');
    if(!is_num){
      if(tokens[i].type == '*'){
	    tokens[i].type = TK_POINTER;
	  }
	  if(tokens[i].type == '-'){
		tokens[i].type = TK_MINUS;
	  }
	}
  }
  
  /* Step 2: Evaluating the regular expression. */
  uint32_t temp_result = eval(0, nr_token - 1);
  
  /* Step 3: Test whether any mismatch occurred. */
  if(invalid == 1){
    return 0;
  }
  else{
	*success = true;
    return temp_result;
  }

  return 0;
  
}

