#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "mpc.h"

/* If we are compiling on widows compile these functions */
#ifdef _WIN32
#include <string.h>

static char buffer[2048];

/* Fake readline function */
char * readline(char* prompt) {
    fputs(propt, stdout);
    fgets(buffer, 2048, stdin);
    char* cpy = malloc(strlen(buffer)+1);
    strcpy(coy, buffer);
    cpy[strlen(cpy)-1] = '\0';
    return cpy;
}

/* Fake add_history function */
void add_history(char* unused) {}

/* Otherwise include the ediline headers */
#elif __linux__
#include <editline/readline.h>
#include <editline/history.h>
static char* message = "PS: You Are On linux (:\n";
#else
#include <editline/readline.h>
#include <editline/history.h>
static char* message = "";
#endif

/* Use operator string to see which operation to perform */
long eval_op(long x, char* op, long y) {
    if(strcmp(op, "+") == 0 || strcmp(op, "add") == 0) {return x + y; }
    if(strcmp(op, "-") == 0 || strcmp(op, "sub") == 0) {return x - y; }
    if(strcmp(op, "*") == 0 || strcmp(op, "mul") == 0) {return x * y; }
    if(strcmp(op, "/") == 0 || strcmp(op, "div") == 0) {return x / y; }
    if(strcmp(op, "^") == 0) {return pow(x, y); }
    if(strcmp(op, "%") == 0) {return x % y; }
    return 0;
}

long eval(mpc_ast_t* t) {
    /* If tagged as number return it directly */
    if (strstr(t->tag, "number")) {
        return atoi(t->contents);
    }

    /* The operator is always the second child. */
    char* op = t->children[1]->contents;

    /* We store the third child in `x` */
    long x = eval(t->children[2]);

    /* Iterate the remaining children and combining. */
    int i = 3;
    while(strstr(t->children[i]->tag, "expr")){
        x = eval_op(x, op, eval(t->children[i]));
        i++;
    }
    return x;
}

/* Calculate the number of leaves in a tree */
int numLeaves(mpc_ast_t* t) {
    if (t->children_num == 0) { 
        return 1;
    }
    if (t->children_num >= 1){
        int total = 0;
        for (int i = 1; i < t->children_num - 1; i++) {
            total += numLeaves(t->children[i]);
        }
        return total;
    }
    return 0;
}

int main(int argc, char** argv) {
    /* Create Some Parsers */
    mpc_parser_t* Number = mpc_new("number");
    mpc_parser_t* Operator = mpc_new("operator");
    mpc_parser_t* Expr = mpc_new("expr");
    mpc_parser_t* Lispy = mpc_new("lispy");

    /* Define them with the following language */
    mpca_lang(MPCA_LANG_DEFAULT,
        "                                                       \
            number   : /-?[0-9]+/ ;                             \
            operator : '+' | '-' | '*' | '/' | '%' | '^'        \
                     | \"add\" | \"sub\" | \"mul\" | \"div\" ;  \
            expr     : <number> | '(' <operator> <expr>+ ')' ;  \
            lispy    : /^/  <operator> <expr>+ /$/ ;            \
        ",
        Number, Operator, Expr, Lispy);

    /* Print Version and Exit information */
    puts("Xylophone Version 0.0.0.0.3");
    puts("Press Ctrl+c to Exit");
    printf("%s\n", message);

    /* In a never ending loop */
    while(1){

        /* Output our prompt and get input */
        char* input = readline("X> ");

        /* Add input to history */
        add_history(input);

        /* Attempt to parse the user Input */
        mpc_result_t r;
        if (mpc_parse("<stdin>", input, Lispy, &r)) {
            /* On Success Print the AST */
            mpc_ast_print(r.output);
            long result = eval(r.output);
            printf("%li\n", result);
            printf("That tree has %d branches\n", numBranches(r.output));
            mpc_ast_delete(r.output);
        } else {
            /* Otherwise Print the Error */
            mpc_err_print(r.error);
            mpc_err_delete(r.error);
        }

        /* Free retrieved input */
        free(input);

    }

    mpc_cleanup(4, Number, Operator, Expr, Lispy);

    return 0;
}
