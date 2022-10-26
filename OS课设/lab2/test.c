#include <unistd.h>
#include <stdio.h>
#include <sys/syscall.h>

int main(){
    int ret = syscall(333);
    printf("the sys_simpletest return %d\n",ret);

    ret = syscall(334, "text.txt", "text-copy.txt");
    printf("the sys_mycp return %d\n",ret);

    return 0;
}