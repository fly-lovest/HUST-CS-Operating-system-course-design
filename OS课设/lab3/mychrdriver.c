#include <linux/kernel.h> 
#include <linux/module.h> 
#include <linux/fs.h> 
#include <linux/init.h> 
#include <linux/uaccess.h>
#define MAIN_DEVICE_NUM 0 //为0时，调用注册函数为动态注册；其他情况为静态注册的设备号
#define DEVNAME "mychrdriver"

static int mdevnum = 0;
static char buffer_k[4096] = "the initial information."; //不用buf命名因为与read、write中要有区分
static int open_mutex = 0;

static int mychrd_open(struct inode *inode,struct file *file);
static int mychrd_release(struct inode *inode,struct file *file);
static ssize_t mychrd_read(struct file *file, char __user *buf, size_t size, loff_t *f_pos);
static ssize_t mychrd_write(struct file *file, const char __user *buf, size_t size, loff_t *f_pos);

static struct file_operations mychrd_fops = {
    .open = mychrd_open,
    .release = mychrd_release,
    .read = mychrd_read,
    .write = mychrd_write
};


static int mychrd_open(struct inode *inode,struct file *file){  //驱动子函数open
    printk("<1>the main device number is %d, the secondary device number is %d.\n",MAJOR(inode->i_rdev),MINOR(inode->i_rdev)); //查看主次设备号
    if(open_mutex){  //进程互斥
        printk("<1>this char device is busy.\n");
        return -1;
    }
    else{
        open_mutex++;
        try_module_get(THIS_MODULE); //打开模块
        printk("<1>open this device successfully.\n");
    }
    return 0;
}

static int mychrd_release(struct inode *inode,struct file *file){  //驱动子函数release
    open_mutex--;
    module_put(THIS_MODULE);
    printk("<1>release this device successfully.\n");
    return 0;
}

static ssize_t mychrd_read(struct file *file, char __user *buf, size_t size, loff_t *f_pos){  //驱动子函数read
    int result = copy_to_user(buf, buffer_k, sizeof(buffer_k));
    if(result != 0){
        printk("<1>read error.\n");
        return -1;
    }
    printk("<1>read success.\n");
    return 0;
}

static ssize_t mychrd_write(struct file *file, const char __user *buf, size_t size, loff_t *f_pos){  //驱动子函数write
    int result = copy_from_user(buffer_k, buf, sizeof(buf));
    if(result != 0){
        printk("<1>write error.\n");
        return -1;
    }
    printk("<1>write success.\n");
    return 0;
}


static int __init mychrd_init(void){   //模块加载函数
    printk("<1>init my char device.\n");
    int result = register_chrdev(MAIN_DEVICE_NUM, DEVNAME, &mychrd_fops);
    if(result < 0){
        printk("<1>register fail.\n");
        return -1;
    }
    if(mdevnum == 0){
        mdevnum = result;
        printk("<1>register done.\n");
        printk("<1>the main device number is %d.\n",mdevnum);
    }
    return 0;
}

static void __exit mychrd_exit(void){   //设备卸载函数
    unregister_chrdev(mdevnum, DEVNAME);
    printk("<1>unregister done.\n");
    printk("<1>after unregister,the main device number is %d.\n",mdevnum);
    return 0;
}

module_init(mychrd_init);
module_exit(mychrd_exit);

MODULE_LICENSE("GPL"); //模块许可