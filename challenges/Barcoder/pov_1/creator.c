#include "cgc_povbarcode.h"
#include <stdio.h>
#include <string.h>

int main(){
	char* ptr = POVBARCODE;
	for(int i = 0; i<sizeof(POVBARCODE); i++){
		printf("%c", ptr[i]);
	}
}
