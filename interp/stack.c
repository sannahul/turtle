#include "stack.h"

void initialise_stack(Stack *s)
{
   s->tp = (Elem *)calloc(1, sizeof(Elem));
   if(s->tp == NULL) {
      ERROR("Can't allocate space.");
   }
}

void Push(Stack *s, double n)
{
   Elem *e;
   e = (Elem *)calloc(1, sizeof(Elem));
   if(e == NULL) {
      ERROR("Can't allocate space.");
   }
   e->prev = s->tp;
   s->tp->f = n;
   s->tp = e;
}

double Pop(Stack *s)
{
   s->tp = s->tp->prev;
   if(s->tp == NULL) {
      ERROR("Value set to the variable is invalid.");
   }
   return s->tp->f;
}

int empty_stack(Stack *s)
{
   s->tp = s->tp->prev;
   if(s->tp == NULL) {
      return 1;
   }
   return 0;
}

void free_stack(Elem *e)
{
   Elem *temp;
   if(e == NULL) {
      return;
   }
   temp = e->prev;
   free(e);
   free_stack(temp);
}

