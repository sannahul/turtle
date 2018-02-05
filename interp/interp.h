#include "neillsdl2.h"
#include "stack.h"
#include <string.h>
#include <ctype.h>

#define ALPHABET 26
#define ARGUMENTS 2
#define INITIALTOKENS 50
#define MAXWORD 1000
#define NOT_SET 0
#define SET 1
#define LOOP 2
#define ROUND 0.5
#define HALF_CIRCLE 180
#define FULL_COLOUR 255
#define MAXTOKENSIZE 10
#define DELAY 20
#define strsame(A,B) (strcmp(A, B)==0)
#define ERROR(PHRASE) { fprintf(stderr, "ERROR: %s\n", PHRASE); exit(2); }

struct var {
   /* set = 0 means variable has not been set
      set = 1 means variable has been set
      set = 2 means variable is being used in loop */
   int set;
   /* the value set to the variable */
   double value;
};
typedef struct var Variable;

struct token {
   char *word;
   /* the line number in input file */
   int line;
};
typedef struct token Token;

struct prog {
   /* array of words read from the argv[1] file */
   Token *wds;
   /* size of the array */
   int length;
   /* current token */
   int cw; 
   /* to keep count on opening and closing
      curly braces to make sure they match */
   int count;
 /* the values of all possible variables A-Z.
    all variables are set to 0 if the user
    hasn't set the variable him/herself. */
   Variable vars[ALPHABET];
};
typedef struct prog Program;

struct turtle {
   SDL_Simplewin *sw;
   /* current y and x */
   double cy;
   double cx;
   /* next y and x */
   double ny;
   double nx;
   /* angle of movement */
   double angle;
};
typedef struct turtle Turtle;

void read_words(char *f, Program *p);
int new_lines(FILE *fp);
void allocate_list(Program *p);
void reallocate_list(Program *p);
void check_arg(int a);
FILE *open_file(char *f);
void initialise_variables(Program *p);
void prog(Program *p, Turtle *t);
void initialise_turtle(Turtle *t, SDL_Simplewin *sw);
void instrclst(Program *p, Turtle *t);
void instruction(Program *p, Turtle *t);
void forward(Program *p, Turtle *t);
void left_turn(Program *p, Turtle *t);
void right_turn(Program *p, Turtle *t);
void var(Program *p);
double varnum(Program *p);
int is_number(char *word);
void do_loop(Program *p, Turtle *t);
int is_int(double n);
void loop(Program *p, Turtle *t, int a, int b, int v);
void set(Program *p);
void polish(Program *p, Stack *s, int c);
int is_op(char *w);
double calculate(char *w, double n1, double n2);
void move(Turtle *t);
void check_count(Program *p);
void free_list(Program *p);
