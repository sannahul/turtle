#include <stdio.h>
#include <stdlib.h>

#define ERROR(PHRASE) { fprintf(stderr, "ERROR: %s\n", PHRASE); exit(2); }

struct stackelem {
   double f;
   struct stackelem *prev;
};
typedef struct stackelem Elem;

struct thestack {
   Elem *tp;
};
typedef struct thestack Stack;

void initialise_stack(Stack *s);
void Push(Stack *s, double n);
double Pop(Stack *s);
void free_stack(Elem *e);
int empty_stack(Stack *s);
