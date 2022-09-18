#include "mmu.h"
#include <stdlib.h>
#include <stdio.h>
#define MB (1024 * 1024)
#define KB (1024)
#define PAGE_SIZE (4 * 1024) // 4 KB
unsigned char code_ro_data[10 * MB];
int sumofElements(int A[],int size){
    int i,sum=0;
    for(i=0;i<size;i++){
        sum+=A[i];
    }
    return sum;
}
int main(){
    code_ro_data[10 * PAGE_SIZE] = 'c';   // write 'c' at first byte in ro_mem
	code_ro_data[10 * PAGE_SIZE + 1] = 'd';
    printf("code_ro_data is%s\n",code_ro_data);
    // char x='1';
    //  char* y=&x;
    // int l=y;
    // printf("%i",y);
    // printf("%i",l);
    // printf("%c\n",x);

    // int x=33;
    // int* y=&x;
    // printf("%d",*y);
    // printf("%d\n",x>>1);
    // int size=sizeof(A)/sizeof(A[0]);
    // int total=sumofElements(A,size);
    // printf("%d\n",total);
    // for(int i=0;i<5;i++){
    //      printf("%d\n",*(&A[i]));
    //      printf("%d\n",*(A+i));
    // }
   
    // printf("%d\n",q);
    // printf("%d\n",&a);
    // printf("%d\n",&p);
}