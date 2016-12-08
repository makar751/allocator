#include "allocator.c"



int main(void) {
  int *a;
  long *b;
  int g[1000000];
  g[0] = mymalloc(sizeof(int)*4);
  for(int i=0;i<1000000;i++)
    g[i]=i;
  for(int i=0;i<1000000;i++)
    printf("%d \n",g[i]);
    a = mymalloc(sizeof(int));
    b = mymalloc(sizeof(long));
  printf("first malloc comp \n");
    myfree(a);
    myfree(b);
  printf("first free comp \n");
    a = mymalloc(sizeof(int));
    b = mymalloc(sizeof(long));
    a = myrealloc(a, sizeof(long));
    b = mycalloc(4, 4);
  printf("second malloc comp \n");
}