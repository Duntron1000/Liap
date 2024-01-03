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

/* Declare New lval Struct */
typedef struct lval {
    int type;
    double num;
    /* Error and sybol types have some string data */
    char* err;
    char* sym;
    /* Count and Pointer to a list of "lval*" */
    int count;
    struct lval** cell;
} lval;

/* Create Enumeration of Possible lval Types */
enum { LVAL_ERR, LAVL_NUM, LAVL_SYM, LAVL_SEXPR };

/* Construct a pointer to a new Number lval */
lval* lval_num(double x) {
    lval * v = malloc(sizeof(lval));
    v->type = LVAL_NUM;
    v->num = x;
    return v;
}

/* Construct a pointer to a new Error lval */
lval* lval_err(char* m) {
    lval* v malloc(sizeof(lval));
    v->type = LVAL_ERR;
    v->err = malloc(strlen(m) + 1);
    strcpy(v->err, m);
    return v;
}

/* Construct a pointer to a new Symbol lval */
lval* lval_sym(char* s) {
    lval* v = maloc (sizeof(lval));
    v->type = LVAL_SYM;
    v->sym = malloc(strlen(s) + 1);
    strcpy(v->sym, s);
    return v;
}

/* A pointer to a new empy Sexpr laval */
lval* lval_sexpr(void) {
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_SEXPR;
    v->count = 0;
    v->cell = Null;
    return v;
}

void lval_del(lval* v) {

    switch (v->type) {
        /* Do nothing special for number type */
        case LVAL_NUM: break;

        /* For Err or SYM free the string data */
        case LVAL_ERR: free(v->err): break;
        case LVAL_SYM: free(v->sym): break;

        /* For Sexpr then delete all elements inside */
        case LVAL_SEXPR:
            for (int i = 0; i < v->count; i++) {
                lval_del(v->cell[i]);
            }
            /* Also free the memory allocated to contain the pointers */
            free(v->cells);
        break;
    }

    /* Free the memory allocated for the "lval" struct itself */
    free(v(;
}

/* Print an "lval" */
void lval_print(lval v) {
    switch (v.type) {
        /* In the case the type is a number print it */
        /* Then 'break' out of the switch. */
        case LVAL_NUM: printf("%f", v.num); break;

        /* In the case the type is an error */
        case LVAL_ERR:
            /* Check what type of error it is and print is */
            if (v.err == LERR_DIV_ZERO) {
                printf("Error: Division by Zero!");
            }
            if (v.err == LERR_BAD_OP) {
                printf("Error: Invalid Operator!");
            }
            if (v.err == LERR_BAD_NUM) {
                printf("ERROR: Invalid Number!");
            }
        break;
    }
}

/* print and "lval" followed by a new line */
void lval_println(lval v) { lval_print(v); putchar('\n'); }

/* Use operator string to see which operation to perform */
lval eval_op(lval x, char* op, lval y) {

    /* If either value is an error return it */
    if (x.type == LVAL_ERR) {return x; }
    if (y.type == LVAL_ERR) {return y; }

    /* Otherwise do maths on ther number values */
    if(strcmp(op, "+") == 0 || strcmp(op, "add") == 0) {return lval_num(x.num + y.num); }
    if(strcmp(op, "-") == 0 || strcmp(op, "sub") == 0) {return lval_num(x.num - y.num); }
    if(strcmp(op, "*") == 0 || strcmp(op, "mul") == 0) {return lval_num(x.num * y.num); }
    if(strcmp(op, "min") == 0 ) {return lval_num(fmin(x.num, y.num)); }
    if(strcmp(op, "max") == 0 ) {return lval_num(fmax(x.num, y.num)); }
    if(strcmp(op, "^") == 0) {return lval_num(pow(x.num, y.num)); }
    if(strcmp(op, "%") == 0) {return lval_num(fmod(x.num, y.num)); }
    if(strcmp(op, "/") == 0 || strcmp(op, "div") == 0) {
        return y.num == 0
            ? lval_err(LERR_DIV_ZERO)
            : lval_num(x.num / y.num);
    }
    return lval_err(LERR_BAD_OP);
}

lval eval(mpc_ast_t* t) {
    /* If tagged as number return it directly */
    if (strstr(t->tag, "number")) {
        /* Check if there is some error in conversion */
        errno = 0;
        long x = strtof(t->contents, NULL);
        return errno != ERANGE ? lval_num(x) : lval_err(LERR_BAD_NUM);
    }

    /* The operator is always the second child. */
    char* op = t->children[1]->contents;

    /* We store the third child in `x` */
    lval x = eval(t->children[2]);

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
    mpc_parser_t* Symbol = mpc_new("symbol");
    mpc_parser_t* Sexpr = mpc_new("sexpr");
    mpc_parser_t* Expr = mpc_new("expr");
    mpc_parser_t* Lispy = mpc_new("lispy");

    /* Define them with the following language */
    mpca_lang(MPCA_LANG_DEFAULT,
        "                                                       \
            number   : /-?[0-9]+/ ;                             \
            symbol   : '+' | '-' | '*' | '/' | '%' | '^'        \
                     | \"add\" | \"sub\" | \"mul\" | \"div\"    \
                     | \"min\" | \"max\" ;                      \
            sexpr    : '(' <expr>* ')' ;                        \
            expr     : <number> | <symbol> | <sexpr> ;          \
            lispy    : /^/ <expr> /$/ ;                         \
        ",
        Number, Symbol, Sexpr, Expr, Lispy);

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
            lval result = eval(r.output);
            lval_println(result);
            mpc_ast_delete(r.output);
        } else {
            /* Otherwise Print the Error */
            mpc_err_print(r.error);
            mpc_err_delete(r.error);
        }

        /* Free retrieved input */
        free(input);

    }

    mpc_cleanup(5, Number, Symbol, Sexpr, Expr, Lispy);

    return 0;
}
