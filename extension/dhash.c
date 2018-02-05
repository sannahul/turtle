#include "dhash.h"

void initialise_table(int size, double f, hashtable *h)
{
   h->list = (Item *)calloc(size, sizeof(Item));
   if(h->list == NULL) {
      fprintf(stderr, "Cannot allocate space.\n");
      exit(1);
   }
   h->num = size;
   h->full = f;
   h->taken = 0;
}

/* Calloc space for the word. Return a char pointer. */
char *allocate(char *word)
{
   char *w;
   int length = strlen(word);
   w = (char*)calloc(length+1, sizeof(char));
   if(w == NULL) {
      fprintf(stderr, "Cannot allocate space.\n");
      exit(1);
   }
   strcpy(w, word);
   return w;
}

/* Double the size of the hashtable and rehash
   the items from the old table to the new bigger
   one. */
void resize_table(hashtable *h)
{
   int i, size;
   hashtable temp;
   size = resize(h->num);
   temp = *h;
   initialise_table(size, temp.full, h);
   for(i = 0; i < temp.num; i++) {
      if(!strsame(temp.list[i].name, "\0")) {
         insert(temp.list[i].name, temp.list[i].n, h);
      }
   }
   free(temp.list);
}

/* Insert the word and int into the hashtable using double
   hashing. Resize the hashtable if it is too full. Don't
   insert the word and return 0 if the word is already in 
   the hashtable. Return 1 if the insertion was successful. */
int insert(char *word, int n, hashtable *h)
{
   int k;
   if(h->taken > (double)(h->num * h->full)) {;
      resize_table(h);
   }
   k = hash1(word, h->num);
   while(h->list[k].name != NULL) {
      if(strsame(h->list[k].name, word)) {
         return 0;
      }
      k += hash2(word, h->num);
      if(k >= h->num) {
         k = k - h->num;
      } 
   }
   h->list[k].name = allocate(word);
   h->list[k].n = n;
   h->taken++;
   return 1;
}

/* Return a prime number that is roughly double
   the size of the original int. */
int resize(int size)
{
   int new;
   new = size + size;
   while(!isprime(new)) {
      new++;
   }
   return new;
}

/* Check if int is prime. If yes, return 1. */
int isprime(int n)
{
   int i;
   for(i = 2; i < n; i++) {
      if(n % i == 0 && i != n) {
         return 0;
      }
   }
   return 1;
}

/* Hash functions found here:
   http://www.cse.yorku.ca/~oz/hash.html */
unsigned int hash1(char *str, int size)
{
   unsigned int hash = PRIME;
   int c;
   while((c = *str++) != '\0') {
      hash = ((hash << CONSTANT) + hash) + c;
   }
   return hash % size;
}

unsigned int hash2(char *str, int size)
{
   unsigned int hash = 0;
   int c;
   while ((c = *str++) != '\0'){
      hash += c;
   }
   /* This is to make sure that the second hash function
      never returns zero. However, that would only happen
      if the string passed to it is "0". */
   if(hash == 0) {
      hash++;
   }
   return hash % size;
}

/* Return the number of the word. Return -1
   if the word is not in the hashtable. */
int search(char *word, hashtable *h)
{
   int key, steps = 1;
   key = hash1(word, h->num);
   if(h->list[key].name == NULL) {
      return -1;
   }
   while(!strsame(h->list[key].name, word)) {
      steps++;
      key += hash2(word, h->num);
      if(key >= h->num) {
         key = key - h->num;
      }
      if(h->list[key].name == NULL || steps > h->num) {
         return -1;
      }
   }
   return h->list[key].n;
}

/* Go through the hashtable and free every cell.
   Free the whole array at the end. */
void free_table(hashtable *h)
{
   int i;
   for(i = 0; i < h->num; i++) {
      free(h->list[i].name);
   }
   free(h->list);
}

