#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define ARGUMENTS 2
#define INITIALTOKENS 50
#define MAXTOKENSIZE 10
#define MAXWORD 1000
#define strsame(A,B) (strcmp(A, B)==0)
#define ERROR(PHRASE) { fprintf(stderr, "ERROR: %s\n", PHRASE); exit(2); }

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
   /* current word */
   int cw; 
   /* to keep count on opening and closing
      curly braces to make sure they match */
   int count;
};
typedef struct prog Program;

void read_words(char *f, Program *p);
int new_lines(FILE *fp);
void allocate_list(Program *p);
void reallocate_list(Program *p);
void check_arg(int a);
FILE *open_file(char *f);
void prog(Program *p);
void instrclst(Program *p);
void instruction(Program *p);
void var(Program *p);
void varnum(Program *p);
int is_number(char *word);
void do_loop(Program *p);
void set(Program *p);
void polish(Program *p);
int is_op(char *w);
void check_count(Program *p);
void free_list(Program *p);
