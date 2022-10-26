/*系统调用号：334*/

asmlinkage long sys_mycp(const char *src_file, const char *dst_file);

asmlinkage long sys_mycp(const char *src_file, const char *dst_file){
    mm_segment_t old_fs = get_fs();
    set_fs(get_ds());
    int srcfd,dstfd;
    char buf[4096];
    ssize_t readsize,writesize;
    if((srcfd = sys_open(src_file, O_RDONLY, 0)) == -1){
        printk("open srcfile fail\n");
        return 1;
    }
    if((dstfd = sys_open(dst_file, O_WRONLY|O_CREAT,0600)) == -1){
        printk("open dstfile fail\n");
        return 2;
    }
    while(1){
        readsize = sys_read(srcfd,buf,4096);
        if(readsize == 0) break;
        else if(readsize == -1){
            printk("read file error\n");
            return 3;
        }
        else{
            writesize = sys_write(dstfd,buf,readsize);
            if(writesize == -1){
                printk("write file error\n");
                return 4;
            }
        }
    }
    sys_close(srcfd);
    sys_close(dstfd);
    printk("copy done!\n");
    set_fs(old_fs);
    return 0;
}