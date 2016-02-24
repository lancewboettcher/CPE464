#include <stdio.h> 
#include <stdlib.h> 

int main() {

   char c[1000];
   FILE *fptr;
   fptr=fopen("numbers.txt","w");
   if(fptr==NULL){
      printf("Error!");
      exit(1);
   }
   int i = 0;
   for ( i = 0; i < 1001; i++) {
      fprintf(fptr,"%d ",i);
      if (i % 10 == 0) 
         fprintf(fptr, "\n");
   }
   fclose(fptr);
   return 0;
}
