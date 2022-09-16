#include "mmu.h"
#include <stdlib.h>
#include <stdio.h>
int sumofElements(int A[],int size){
    int i,sum=0;
    for(i=0;i<size;i++){
        sum+=A[i];
    }
    return sum;
}
int main(){
    int A[]={2,4,5,8,1};
    int* x=A;
    printf("%d",x);
    printf("%d",x+1);
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