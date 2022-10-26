#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(){
    char rdbuf[4096], wrbuf[4096];
    int fd = open("/dev/mychrdriver",O_RDWR|O_NOCTTY|O_NONBLOCK);
    if(fd < 0){
        perror("open device fail.\n");
        return -1;
    }
    ssize_t readret = read(fd,rdbuf,sizeof(rdbuf));
    if(readret < 0){
        perror("read devinfo error.\n");
        return -1;
    }
    printf("The device initial information: %s\n", rdbuf); //读取设备初始化信息

    printf("please input your information:\n");
    scanf("%[^\n]",wrbuf); //写入设备缓冲区的信息
    ssize_t writeret = write(fd,wrbuf,sizeof(wrbuf));
    if(writeret < 0){
        perror("write devinfo error.\n");
        return -1;
    }
    printf("write done.\n");
    readret = read(fd,rdbuf,sizeof(rdbuf));
    if(readret < 0){
        perror("read devinfo error.\n");
        return -1;
    }
    printf("the devbuf info is \"%s\".\n",rdbuf);
    close(fd);
    return 0;
}