
#include <sys/stat.h>
#include <stdio.h>
int main()
{	
    struct stat st = {0};
    if(stat("./coleworld",&st) == -1){
		printf("not thre");
        mkdir("./coleworld", 0700);
    }
	return 0;
}