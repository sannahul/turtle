#include "extension.h"

int main(int argc, char **argv)
{
   Program p;
   Turtle t;
   hashtable h;
   check_arg(argc);
   read_words(argv[1], &p);
   p.functions = &h;
   initialise_table(HASHSIZE, FULL, p.functions);
   p.cw = 0;
   p.count = 0;
   prog(&p, &t);
   free(p.functions->list);
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
   p->wds[i].line = r+1;
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

/* initialise the value of the variables to 0.
   mark that the variables have not been set
   by putting set to 0 */
void initialise_variables(Program *p)
{
   int i;
   for(i = 0; i < ALPHABET; i++) {
      p->vars[i].set = NOT_SET;
      p->vars[i].value = 0;
   }
}

void prog(Program *p, Turtle *t)
{
   SDL_Simplewin sw;
   if(strsame(p->wds[p->cw].word, "FUNCTION")) {
      p->cw++;
      store_function(p);
      p->cw++;
      prog(p, t);
   } else if(strsame(p->wds[p->cw].word, "{")) {
      initialise_variables(p);
      initialise_turtle(t, &sw);
      p->count++;
      p->cw++;
      instrclst(p, t);
      p->cw++;
      check_count(p);
      atexit(SDL_Quit);
      return;
   } else if(strsame(p->wds[p->cw].word, "\0")) {
      free(p->functions->list);
      free_list(p);
      ERROR("Main function missing?");
   } else {
      printf("Found in line %d:\n", p->wds[p->cw].line);
      free(p->functions->list);
      free_list(p);
      ERROR("Missing { at the beginning?");
   }
}

/* initialise SDL_Simplewin, define the colour and 
   the starting coordinates. initialise the angle of
   movement to 0 */
void initialise_turtle(Turtle *t, SDL_Simplewin *sw)
{
   t->sw = sw;
   Neill_SDL_Init(t->sw);
   Neill_SDL_SetDrawColour(t->sw, FULL_COLOUR, FULL_COLOUR, 0);
   t->cx = WWIDTH / 2;
   t->cy = WHEIGHT / 2;
   t->nx = t->cx;
   t->ny = t->cy;
   t->angle = 0;
}

/* check the name of the function and that a function
   with the same name doesn't already exist */
void store_function(Program *p)
{
   int n, line;
   char word[MAXTOKENSIZE];
   strcpy(word, p->wds[p->cw].word);
   line = p->wds[p->cw].line;
   if(!check_fname(word)) {
      printf("Found in line %d:\n", p->wds[p->cw].line);
      free(p->functions->list);
      free_list(p);
      ERROR("Only uppercase letters allowed in function name.");
   }
   p->cw++;
   n = p->cw + 1;
   if(!insert(word, n, p->functions)) {
      printf("Found in line %d:\n", line);
      free(p->functions->list);
      free_list(p);
      ERROR("Function with this name already exists.");
   }
   check_function(p, word);
}

int check_fname(char *word)
{
   int length, i;
   char c;
   length = strlen(word);
   for(i = 0; i < length; i++) {
      c = word[i];
      if(!isupper(c)) {
         return 0;
      }
   }
   return 1;
}

/* check that the opening and closing brackets match
   and that the function doesn't call itself */
void check_function(Program *p, char *word)
{
   if(!strsame(p->wds[p->cw].word, "{")) {
      printf("Found in line %d:\n", p->wds[p->cw].line);
      free(p->functions->list);
      free_list(p);
      ERROR("Missing { after function name.");
   }
   p->count++;
   p->cw++;
   while(p->count > 0) {
      if(strsame(p->wds[p->cw].word, word)) {
         printf("Found in line %d:\n", p->wds[p->cw].line);
         free(p->functions->list);
         free_list(p);
         ERROR("Function is not allowed to call itself.");
      }
      if(strsame(p->wds[p->cw].word, "{")) {
         p->count++;
      }
      if(strsame(p->wds[p->cw].word, "}")) {
         p->count--;
      }
      p->cw++;
   }
   p->cw--;
}

void instrclst(Program *p, Turtle *t)
{
   Neill_SDL_Events(t->sw);
   if(t->sw->finished) {
      return;
   }
   if(strsame(p->wds[p->cw].word, "}")) {
      p->count--;
      return;
   }
   instruction(p, t);
   p->cw++;
   instrclst(p, t);
}

void instruction(Program *p, Turtle *t)
{
   Neill_SDL_Events(t->sw);
   if(t->sw->finished) {
      return;
   }
   if(strsame(p->wds[p->cw].word, "FD")) {
      p->cw++;
      forward(p, t);
      return;
   }
   if(strsame(p->wds[p->cw].word, "LT")) {
      p->cw++;
      left_turn(p, t);
      return;
   }
   if(strsame(p->wds[p->cw].word, "RT")) {
      p->cw++;
      right_turn(p, t);
      return;
   }
   if(strsame(p->wds[p->cw].word, "DO")) {
      p->cw++;
      do_loop(p, t);
      return;
   }
   if(strsame(p->wds[p->cw].word, "SET")) {
      p->cw++;
      set(p);
      return;
   }
   if(strsame(p->wds[p->cw].word, "FUNC")) {
      p->cw++;
      function(p, t);
      return;
   }
   if(strsame(p->wds[p->cw].word, "\0")) {
      printf("Found in line %d:\n", p->wds[p->cw].line);
      free(p->functions->list);
      free_list(p);
      ERROR("Closing bracket missing.");
   }
   printf("Found in line %d:\n", p->wds[p->cw].line);
   free(p->functions->list);
   free_list(p);
   ERROR("Invalid instruction.");
}

/* calculate the new coordinates and draw a line
   between the old and new coordinates */
void forward(Program *p, Turtle *t)
{
   double n, a, b;
   n = varnum(p);
   a = n * sin(M_PI / HALF_CIRCLE * t->angle);
   b = n * cos(M_PI / HALF_CIRCLE * t->angle);
   t->ny = t->cy - b;
   t->nx = t-> cx + a;

   if(t->ny > WHEIGHT || t->nx > WWIDTH) {
      free(p->functions->list);
      free_list(p);
      ERROR("Not allowed to go outside the window.");
   }
   if(t->ny < 0 || t->nx < 0) {
      free(p->functions->list);
      free_list(p);
      ERROR("Not allowed to go outside the window.");
   }
   move(t);
}

void left_turn(Program *p, Turtle *t)
{
   double n;
   n = varnum(p);
   t->angle -= n;
}

void right_turn(Program *p, Turtle *t)
{
   double n;
   n = varnum(p);
   t->angle += n;
}

void var(Program *p)
{
   char c;
   if(strsame(p->wds[p->cw].word, "\0")) {
      printf("Found in line %d:\n", p->wds[p->cw].line);
      free(p->functions->list);
      free_list(p);
      ERROR("Expecting a variable.");
   }
   if(strlen(p->wds[p->cw].word) != 1) {
      printf("Found in line %d:\n", p->wds[p->cw].line);
      free(p->functions->list);
      free_list(p);
      ERROR("Variable name invalid (too long).");
   }
   c = p->wds[p->cw].word[0];
   if(!isupper(c)) {
      printf("Found in line %d:\n", p->wds[p->cw].line);
      free(p->functions->list);
      free_list(p);
      ERROR("Variable name invalid.");
   }
}

double varnum(Program *p)
{
   double n;
   char c;
   if(strsame(p->wds[p->cw].word, "\0")) {
      printf("Found in line %d:\n", p->wds[p->cw].line);
      free(p->functions->list);
      free_list(p);
      ERROR("Expecting a number or variable.");
   }
   c = p->wds[p->cw].word[0];
   if(isalpha(c)) {
      var(p);
      if(p->vars[c - 'A'].set == NOT_SET) {
         printf("Found in line %d:\n", p->wds[p->cw].line);
         free(p->functions->list);
         free_list(p);
         ERROR("Variable used has not been set.");
      }
      n = p->vars[c - 'A'].value;
   } else {
      if(!is_number(p->wds[p->cw].word)) {
         printf("Found in line %d:\n", p->wds[p->cw].line);
         free(p->functions->list);
         free_list(p);
         ERROR("Expecting a number or variable.");
      }
      n = atof(p->wds[p->cw].word);
   }
   return n;
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

void do_loop(Program *p, Turtle *t)
{
   double a, b;
   int c, v;
   var(p);
   /* c is the variable (A-Z) */
   c = p->wds[p->cw].word[0];
   /* v is the location of the variable in the vars array */
   v = c - 'A';
   p->cw++;
   if(!strsame(p->wds[p->cw].word, "FROM")) {
      printf("Found in line %d:\n", p->wds[p->cw].line);
      free(p->functions->list);
      free_list(p);
      ERROR("Expecting FROM");
   }
   p->cw++;
   a = varnum(p);
   if(!is_int(a)) {
      printf("Found in line %d:\n", p->wds[p->cw].line);
      free(p->functions->list);
      free_list(p);
      ERROR("Value after FROM must be integer.");
   }
   /* make sure the var is not being used inside another
      loop at the same time */
   if(p->vars[v].set == LOOP) {
      printf("Found in line %d:\n", p->wds[p->cw].line);
      free(p->functions->list);
      free_list(p);
      ERROR("Can't have nested loops dependent on the same variable.");
   }
   p->cw++;
   if(!strsame(p->wds[p->cw].word, "TO")) {
      printf("Found in line %d:\n", p->wds[p->cw].line);
      free(p->functions->list);
      free_list(p);
      ERROR("Expecting TO");
   }
   p->cw++;
   b = varnum(p);
   if(!is_int(b)) {
      printf("Found in line %d:\n", p->wds[p->cw].line);
      free(p->functions->list);
      free_list(p);
      ERROR("Value after TO must be integer.");
   }
   /* if the upper boundary  of the loop is lower 
      than the start value of the loop */
   if(b <= a) {
      printf("Found in line %d:\n", p->wds[p->cw].line);
      free(p->functions->list);
      free_list(p);
      ERROR("Value after FROM must be smaller than value after TO.");
   }
   p->cw++;
   if(!strsame(p->wds[p->cw].word, "{")) {
      printf("Found in line %d:\n", p->wds[p->cw].line);
      free(p->functions->list);
      free_list(p);
      ERROR("Expecting {");
   }
   /* set the value of the variable & mark it as being used in loop */
   p->vars[v].value = a;
   p->vars[v].set = LOOP;
   p->cw++;
   loop(p, t, a, b, v);
   /* mark the value as set */
   p->vars[v].set = SET;
}

/* check that there are only zeros after
   the decimal point */
int is_int(double n)
{
   int length, i, count = 0;
   char c;
   char word[MAXWORD];
   sprintf(word, "%f", n);
   length = strlen(word); 
   for(i = 0; i < length; i++) {
      c = word[i];
      if(count > 0) {
         if(c != '0') {
            return 0;
         }
      }
      if(c == '.') {
         count++;
      }
   }
   return 1;
}

void loop(Program *p, Turtle *t, int a, int b, int v)
{
   /* start keeps track of the start of the loop */
   int i, start = p->cw;
   for(i = a; i <= b; i++) {
      p->count++;
      p->vars[v].value = (double)i;
      instrclst(p, t);
      if(i < b) {
         p->cw = start;
      }
   }
}

void set(Program *p)
{
   int c;
   Stack s;
   var(p);
   /* c is the location of the variable in
      the vars array */
   c = p->wds[p->cw].word[0];
   c = c - 'A';
   p->cw++;
   if(strsame(p->wds[p->cw].word, ":=")) {
      p->cw++;
      initialise_stack(&s);
      polish(p, &s, c);
      free_stack(s.tp);
   } else {
      printf("Found in line %d:\n", p->wds[p->cw].line);
      free(p->functions->list);
      free_list(p);
      ERROR("Expecting :=");
   }
}

void polish(Program *p, Stack *s, int c)
{
   double n1, n2, result;

   if(strsame(p->wds[p->cw].word, "\0")) {
      printf("Found in line %d:\n", p->wds[p->cw].line);
      free_stack(s->tp);
      free(p->functions->list);
      free_list(p);
      ERROR("Expecting variable, number, operator or ;"); 
   } else if(strsame(p->wds[p->cw].word, ";")) {
      result = Pop(s);
      if(!empty_stack(s)) {
         printf("Found in line %d:\n", p->wds[p->cw].line);
         free_stack(s->tp);
         free(p->functions->list);
         free_list(p);
         ERROR("Value set to the variable is invalid.");
      }
      p->vars[c].value = result;
      p->vars[c].set = SET;
      return;
   } else if(is_op(p->wds[p->cw].word)) {
      n2 = Pop(s);
      n1 = Pop(s);
      if(strsame(p->wds[p->cw].word, "/")) {
         /* to avoid comparing floating point number to 0.
            the values are not #defined, as the code is more
            self explanatory if the values are shown here. */
         if(n2 > -0.00000001 && n2 < 0.00000001) {
            printf("Found in line %d:\n", p->wds[p->cw].line);
            free_stack(s->tp);
            free(p->functions->list);
            free_list(p);
            ERROR("Not allowed to divide by 0.");
         }
      }
      result = calculate(p->wds[p->cw].word, n1, n2);
      Push(s, result);
      p->cw++;
      polish(p, s, c);
   } else {
      result = varnum(p);
      Push(s, result);
      p->cw++;
      polish(p, s, c);
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

double calculate(char *w, double n1, double n2)
{
   if(strsame(w, "+")) {
      return n1 + n2;
   }
   if(strsame(w, "-")) {
      return n1 - n2;
   }
   if(strsame(w, "*")) {
      return n1 * n2;
   }
   if(strsame(w, "/")) {
      return n1 / n2;
   }
   return 0;
}

/* jump to the start of the function in the token array,
   parse and interprete the instructions and return to
   the correct place of the token array */
void function(Program *p, Turtle *t)
{
   int jump_to, return_to;
   jump_to = search(p->wds[p->cw].word, p->functions);
   if(jump_to < 0) {
      printf("Found in line %d:\n", p->wds[p->cw].line);
      free(p->functions->list);
      free_list(p);
      ERROR("Trying to call a function that hasn't been set.");
   }
   return_to = p->cw;
   p->cw = jump_to;
   p->count++;
   instrclst(p, t);
   p->cw = return_to;
}

/* draw line from old coordinates to the new ones */
void move(Turtle *t)
{
   int x1, y1, x2, y2;
   /* 0.5 is added so that the number rounds correctly
      when converted to integer */
   x1 = (int)(t->cx + ROUND);
   y1 = (int)(t->cy + ROUND);
   x2 = (int)(t->nx + ROUND);
   y2 = (int)(t->ny + ROUND);
   SDL_RenderDrawLine(t->sw->renderer, x1, y1, x2, y2);
   t->cy = t->ny;
   t->cx = t->nx;
   SDL_RenderPresent(t->sw->renderer);
   SDL_UpdateWindowSurface(t->sw->win); 

   SDL_Delay(20);
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
      free(p->functions->list);
      free_list(p);
      ERROR("Too many closing brackets.");
   }
   if(p->count == 0) {
      if(!strsame(p->wds[p->cw].word, "\0")) {
         printf("Found in line %d:\n", p->wds[p->cw].line);
         free(p->functions->list);
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

