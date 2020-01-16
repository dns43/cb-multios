#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
int main(){
	int n = -1;
	printf("%d %x\n",n ,n);
/*	char* in;
	scanf("%s",in);
	printf("%s %s\n", in , in);*/
 
    char data[128];
 
    if(read(0, data, 128) < 0){
     	write(2, "An error occurred in the read.\n", 31);
    	exit(0);
    }
	printf("%s",data);
    if(atoi(data) == -1){
	printf("success, %s is %d \n", data, atoi(data));
    }
    exit(0);
}


