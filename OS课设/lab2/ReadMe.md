更换内核操作
gedit /boot/grub/grub.cfg 查看内核信息

menuentry底下还有submenu，submenu底下就包含了 带有缩进的 menuentry。以下是计数规则：

第一个menuentry 的序号是0，第二个submenu的序号是1，以此类推
在submenu中，第一个menuentry的序号是0，第二个是1，以此类推

接着，sudo gedit /etc/default/grub

修改
GRUB_DEFAULT=“1 >0”

然后reboot即可

或者简单的：
在开机界面按住“shift”按键
