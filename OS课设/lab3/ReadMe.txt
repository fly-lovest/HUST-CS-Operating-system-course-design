在/dev下sudo rmmod mychrdriver：删除错误的字符设备
模块添加步骤：
在源文件目录下
编写好makefile后，sudo make
sudo insmod mychrdriver.ko
cat /proc/devices  //查看设备号
sudo mknod /dev/mychrdriver c 243 0
测试
sudo ./driver_test