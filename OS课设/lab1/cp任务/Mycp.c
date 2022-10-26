#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#define MAX_BUFFER 4096

int main(int argc, char *argv[]){
    int srcfd,dstfd;
    char buf[MAX_BUFFER];
    if(argc != 3){
        perror("please input correctly!\n");
        exit(1);
    }
    if(!strcmp(argv[1],argv[2])){
        perror("the two files are same!\n");
        exit(1);
    }
    if((srcfd=open(argv[1],O_RDONLY))==-1){
        perror("the source file open fail!\n");
        exit(1);
    }
    if((dstfd=open(argv[2],O_WRONLY|O_CREAT,0600))==-1){
        perror("the destination file open fail!\n");
        exit(1);
    }
    while (1)
    {
        ssize_t readsize,writesize;
        readsize = read(srcfd,buf,MAX_BUFFER);
        if(readsize == 0) break;
        else if(readsize == -1){
            perror("read file error!\n");
            exit(1);
        }
        else{
            writesize = write(dstfd,buf,readsize);
            if(writesize == -1){
                perror("write file error!\n");
                exit(1);
            }
        }
    }
    printf("copy done!\n");
    close(srcfd);
    close(dstfd);
    exit(0);
}