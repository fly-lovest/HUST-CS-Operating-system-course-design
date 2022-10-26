/*系统调用号：333*/

asmlinkage long sys_simpletest(void);

asmlinkage long sys_simpletest(void){
    printk("This is a simple test!");
    return 0;
}