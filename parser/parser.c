#include "parser.h"

int main(int argc, char **argv)
{
   Program p;

   check_arg(argc);
   read_words(argv[1], &p);
   p.cw = 0;
   p.count = 0;
   prog(&p);
   free_list(&p);
   return 0;
}

/* read words from the user's input file
   into an array. store the line number
   for each word. */
void read_words(char *f, Program *p)
{
   FILE *fp;
   char word[MAXWORD];
   int i = 0, r = 1;
   fp = open_file(f);
   p->length = INITIALTOKENS;
   allocate_list(p);
   /* check if there are empty lines before
      the start of the actual program */
   r += new_lines(fp);

   while(fscanf(fp, "%s", word) == 1) {
      if(strlen(word) > MAXTOKENSIZE) {
         printf("Found in line %d:\n", r);
         free_list(p);
         ERROR("Invalid word (too long).");
      }
      if(i == p->length-1) {
         reallocate_list(p);
      }
      strcpy(p->wds[i].word, word);
      p->wds[i].line = r;
      r += new_lines(fp);
      i++;
   }
   p->wds[i].line = r;
   fclose(fp);
}

/* count the number of empty lines
   before a word in a file */
int new_lines(FILE *fp)
{
   int n = 0;
   char c = getc(fp);

   while(isspace(c)) {
      if(c == '\n' || c == '\r') {
         n++;
      }
      c = getc(fp);
   }
   /* empty lines at the end of the file ignored */
   if(c == EOF) {
      return 0;
   }
   fseek(fp, -1L, SEEK_CUR);
   return n;
}

/* calloc space for the list of tokens.
   initialise all the words into empty strings. */
void allocate_list(Program *p)
{
   int i;
   p->wds = (Token *)calloc(p->length, sizeof(Token));
   if(p->wds == NULL) {
      ERROR("Can't allocate space.");
   }
   for(i = 0; i < p->length; i++) {
      p->wds[i].word = (char *)calloc(MAXTOKENSIZE, sizeof(char));
      if(p->wds[i].word == NULL) {
         ERROR("Can't allocate space.");
      }
   }
   for(i = 0; i < p->length; i++) {
      strcpy(p->wds[i].word, "\0");
   }
}

/* double the size of the original token list
   and copy the words from the original into
   the new one. free the old list */
void reallocate_list(Program *p)
{
   Program temp;
   int i;
   temp = *p;
   p->length = temp.length * 2;
   allocate_list(p);
   for(i = 0; i < temp.length; i++) {
      strcpy(p->wds[i].word, temp.wds[i].word);
      p->wds[i].line = temp.wds[i].line;
   }
   free_list(&temp);
}

void check_arg(int a)
{
   if(a < ARGUMENTS) {
      ERROR("Invalid usage (too few arguments).");
   }
   if(a > ARGUMENTS) {
      ERROR("Invalid usage (too many arguments).");
   }
}     

FILE *open_file(char *f)
{
   FILE *fp;
   fp = fopen(f, "r");
   if(fp == NULL) {
      ERROR("Unable to open the file.");
   }
   return fp;
}

void prog(Program *p)
{
   if(!strsame(p->wds[p->cw].word, "{")) {
      printf("Found in line %d:\n", p->wds[p->cw].line);
      free_list(p);
      ERROR("Missing { at the beginning.");
   }
   p->cw++;
   p->count++;
   instrclst(p);
   p->cw++;
   check_count(p);
}

void instrclst(Program *p)
{
   if(strsame(p->wds[p->cw].word, "}")) {
      p->count--;
      return;
   }
   instruction(p);
   p->cw++;
   instrclst(p);
}

void instruction(Program *p)
{
   if(strsame(p->wds[p->cw].word, "FD")) {
      p->cw++;
      varnum(p);
      return;
   }
   if(strsame(p->wds[p->cw].word, "LT")) {
      p->cw++;
      varnum(p);
      return;
   }
   if(strsame(p->wds[p->cw].word, "RT")) {
      p->cw++;
      varnum(p);
      return;
   }
   if(strsame(p->wds[p->cw].word, "DO")) {
      p->cw++;
      do_loop(p);
      return;
   }
   if(strsame(p->wds[p->cw].word, "SET")) {
      p->cw++;
      set(p);
      return;
   }
   if(strsame(p->wds[p->cw].word, "\0")) {
      printf("Found in line %d:\n", p->wds[p->cw].line);
      free_list(p);
      ERROR("Closing bracket missing.");
   }
   printf("Found in line %d:\n", p->wds[p->cw].line);
   free_list(p);
   ERROR("Invalid instruction.");
}

void var(Program *p)
{
   char c;
   if(strsame(p->wds[p->cw].word, "\0")) {
      printf("Found in line %d:\n", p->wds[p->cw].line);
      free_list(p);
      ERROR("Expecting a variable.");
   }
   if(strlen(p->wds[p->cw].word) != 1) {
      printf("Found in line %d:\n", p->wds[p->cw].line);
      free_list(p);
      ERROR("Variable name invalid (too long).");
   }
   c = p->wds[p->cw].word[0];
   if(!isupper(c)) {
      printf("Found in line %d:\n", p->wds[p->cw].line);
      free_list(p);
      ERROR("Variable name invalid.");
   }
}

void varnum(Program *p)
{
   char c = p->wds[p->cw].word[0];
   if(isalpha(c)) {
      var(p);
   } else {
      if(!is_number(p->wds[p->cw].word)) {
         printf("Found in line %d:\n", p->wds[p->cw].line);
         free_list(p);
         ERROR("Expecting a number or variable.");
      }
   }
}

/* return 1 if the string is a number
   (float or integer, positive or negative. */
int is_number(char *word)
{
   int i, length, count = 0;
   char c;
   length = strlen(word);
   for(i = 0; i < length; i++) {
      c = word[i];
      if(!isdigit(c) && c != '.' && c != '-') {
         return 0;
      }
      if(c == '.') {
         /* number starting or ending with . 
            is not accepted */
         if(i == 0 || i == length-1) {
            return 0;
         }
         /* if there is more than one decimal point */
         if(count > 0) {
            return 0;
         }
         /* "-." is not accepted */
         if(word[i-1] == '-') {
            return 0;
         }
         count++;
      }
      /* if the word contains a minus sign somewhere
         else than at the beginning */
      if(c == '-' && i > 0) {
         return 0;
      }
   }
   return 1;
}

void do_loop(Program *p)
{
   var(p);
   p->cw++;
   if(!strsame(p->wds[p->cw].word, "FROM")) {
      printf("Found in line %d:\n", p->wds[p->cw].line);
      free_list(p);
      ERROR("Expecting FROM");
   }
   p->cw++;
   varnum(p);
   p->cw++;
   if(!strsame(p->wds[p->cw].word, "TO")) {
      printf("Found in line %d:\n", p->wds[p->cw].line);
      free_list(p);
      ERROR("Expecting TO");
   }
   p->cw++;
   varnum(p);
   p->cw++;
   if(!strsame(p->wds[p->cw].word, "{")) {
      printf("Found in line %d:\n", p->wds[p->cw].line);
      free_list(p);
      ERROR("Expecting {");
   }
   p->count++;
   p->cw++;
   instrclst(p);

}

void set(Program *p)
{
   var(p);
   p->cw++;
   if(strsame(p->wds[p->cw].word, ":=")) {
      p->cw++;
      polish(p);
   } else {
      printf("Found in line %d:\n", p->wds[p->cw].line);
      free_list(p);
      ERROR("Expecting :=");
   }
}

void polish(Program *p)
{
   if(strsame(p->wds[p->cw].word, "\0")) {
      printf("Found in line %d:\n", p->wds[p->cw].line);
      free_list(p);
      ERROR("Expecting variable, number, operator or ;");
   } else if(strsame(p->wds[p->cw].word, ";")) {
      return;
   } else if(is_op(p->wds[p->cw].word)) {
      p->cw++;
      polish(p);
   } else {
      varnum(p);
      p->cw++;
      polish(p);
   }
}

int is_op(char *w)
{
   if(strsame(w, "+") || strsame(w, "-")) {
      return 1;
   }
   if(strsame(w, "*") || strsame(w, "/")) {
      return 1;
   }
   return 0;
}

/* check that the opening and closing curly
   brackets match at the end of the program */
void check_count(Program *p)
{
   if(strsame(p->wds[p->cw].word, "}")) {
      p->count--;
   }
   if(p->count < 0) {
      printf("Found in line %d:\n", p->wds[p->cw].line);
      free_list(p);
      ERROR("Too many closing brackets.");
   }
   if(p->count == 0) {
      if(!strsame(p->wds[p->cw].word, "\0")) {
         printf("Found in line %d:\n", p->wds[p->cw].line);
         free_list(p);
         ERROR("Invalid usage of brackets.");
      }
   }
}

/* free the space allocated for each word.
   free the entire list at the end */
void free_list(Program *p)
{
   int i;
   for(i = 0; i < p->length; i++) {
      free(p->wds[i].word);
   }
   free(p->wds);
}

