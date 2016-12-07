#include "allocator.c"



int main(void) {
  int *a;
  long *b;
  int tmp[100000];
  for (int i=0;i<100000;i++)
  {
    tmp[i]=mymalloc(sizeof(int));
    printf("%d /n",i);
  }
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

  //cleanup();
}