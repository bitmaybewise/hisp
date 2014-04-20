#include "mpc.h"

#ifdef _WIN32

#include <string.h>

static char buffer[2048];

/* Fake readline function */
char* readline(char* prompt) {
  fputs("hisp> ", stdout);
  fgets(buffer, 2048, stdin);
  char* cpy = malloc(strlen(buffer)+1);
  strcpy(cpy, buffer);
  cpy[strlen(cpy)-1] = '\0';
  return cpy;
}

/* Fake add_history function */
void add_history(char* unused) {}

/* Otherwise include the editline headers */
#else

#include <editline/readline.h>
#include <editline/history.h>

#endif

long eval_op(long x, char* op, long y) {
  if (strcmp(op, "+") == 0 || strcmp(op, "add") == 0) {
    return x + y;
  }
  if (strcmp(op, "-") == 0 || strcmp(op, "sub") == 0) {
    return x - y;
  }
  if (strcmp(op, "*") == 0 || strcmp(op, "mul") == 0) {
    return x * y;
  }
  if (strcmp(op, "/") == 0 || strcmp(op, "div") == 0) {
    return x / y;
  }
  if (strcmp(op, "%") == 0) {
    return x % y;
  }
  if (strcmp(op, "^") == 0) {
    return pow(x,y);
  }
  if (strcmp(op, "min") == 0) {
    return x < y ? x : y;
  }
  if (strcmp(op, "max") == 0) {
    return x > y ? x : y;
  }

  return 0;
}

long eval(mpc_ast_t* t) {
  /* if tagged as number return it directly, otherwise expression */
  if (strstr(t->tag, "number")) {
    return atoi(t->contents);
  }

  /* the operator is always second child */
  char* op = t->children[1]->contents;

  long x = eval(t->children[2]);

  int i = 3;
  while (strstr(t->children[i]->tag, "expr")) {
    x = eval_op(x, op, eval(t->children[i]));
    i++;
  }

  return x;
}

int main(int argc, char **argv) {
  /* Create some parsers */
  mpc_parser_t *Number   = mpc_new("number");
  mpc_parser_t *Operator = mpc_new("operator");
  mpc_parser_t *Expr     = mpc_new("expr");
  mpc_parser_t *Function = mpc_new("function");
  mpc_parser_t *Hisp     = mpc_new("hisp");

  /* Define them with the following Language */
  mpca_lang(MPC_LANG_DEFAULT,
      "                                                                               \
      number   : /-?[0-9]+/ ;                                                         \
      operator : '+' | '-' | '*' | '/' | '%' | '^'  ;                                 \
      function : /add|sub|mul|div|min|max/ ;                                                  \
      expr     : <number> | '(' <operator> <expr>+ ')' | '(' <function> <expr>+ ')' ; \
      hisp     : /^/ <operator> <expr>+ /$/  | /^/ <function> <expr>+ /$/ ;           \
      ",
      Number, Operator, Function, Expr, Hisp);

  puts("Hercules Lisp Version 0.0.0.0.3");
  puts("Press Ctrl+c to Exit\n");

  while(1) {
    char *input = readline("hisp> ");
    add_history(input);

    /* Attempt to parse the user input */
    mpc_result_t r;
    if(mpc_parse("<stdin>", input, Hisp, &r)) {
      /* On success print the AST */
      long result = eval(r.output);
      printf("%li\n", result);
      mpc_ast_delete(r.output);
    } else {
      /* Otherwise print the error */
      mpc_err_print(r.error);
      mpc_err_delete(r.error);
    }

    free(input);
  }

  /* Undefine and delete our parsers */
  mpc_cleanup(4, Number, Operator, Function, Expr, Hisp);

  return 0;
}
