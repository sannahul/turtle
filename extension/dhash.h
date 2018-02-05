#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PRIME 5381
#define CONSTANT 5
#define strsame(A,B) (strcmp(A, B)==0)

struct item {
   /* the name of the function */
   char *name;
   /* the place of the start of the function */
   int n;
};
typedef struct item Item;

struct hashtable {
   /* num is the size of the hashtable. */
   int num;
   /* taken is the number of strings in the hashtable. */
   int taken;
   /* full defines how full the hashtable can get.
      For example: 0.6 means 60% full. */
   double full;
   Item *list;
};
typedef struct hashtable hashtable;

void initialise_table(int size, double f, hashtable *h);
char *allocate(char *word);
int insert(char *word, int line, hashtable *h);
int search(char *word, hashtable *h);
void free_table(hashtable *h);
void resize_table(hashtable *h);
int resize(int size);
int isprime(int n);
unsigned int hash1(char *str, int size);
unsigned int hash2(char *str, int size);
