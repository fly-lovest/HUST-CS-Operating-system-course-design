#### box控件

##### GtkWidget *gtk_hbox_new(gboolean homogeneous,gint spacing);
##### GtkWidget *gtk_vbox_new(gboolean homogeneous,gint spacing);

第一个参数称为homogeneous.用这一参数来确定所有增加的构件具有同样的大小。例如，在一个横向组装盒中建立5个按钮，分别带有标号：fred、joe、sue、karen和Bartholomew the Great。如果homogeneous参数为FALSE，组装盒建立5个不同大小的按钮，正好在按钮上容纳各自的标号的正文。如果homogeneous参数为TRUE，组装盒计算最大的构件大小，然后按照最大的构件的大小对所有按钮分配同样大小的空间。

​    第二个参数称为spacing。用它来确定插入组装盒的构件之间的空间。将spacing参数设置为0表示在插入的构件之间不留空间。



##### void gtk_box_pack_start(GtkBox *box,GtkWidget *child,gboolean expend,gboolean fill,guint padding);

##### void gtk_box_pack_end(GtkBox *box,GtkWidget *child,gboolean expend,gboolean fill,guint padding);

box     组装盒的名称

child     子构件的名称

expend   构件周围是否还有可扩充的空间

fill     构件是否需要充分利用构件周围空间

padding   构件周围要保留多少个填充的像元

函数gtk_box_pack_start将构件放在顶部（对于纵向组装盒）或左边（对于横向组装盒。函数gtk_box_pack_end将构件放在底部（对于纵向组装盒）或右边（对于横向组装盒）。

gtk_box_paxk_start和gtk_box_pack_end有5个参数，其中3个参数用来确定如何将构件放在组装盒内。

expand参数表示在所有构件加入组装盒以后，构件周围是否还有可扩充的空间。如果用homogenous格式建立组装盒，则忽略这一参数。因为homegeneous参数表示构件在组装盒内使用同样大小的空间。

fill参数表示构件是否需要充分利用构件周围空间。将此参数设置为TRUE允许构件稍稍扩大一点，以充分利用组装盒分配给它的空间。将此参数设置为FALSE强制构件只使用它需要的空间。多余的空间围绕构件的周围分布。

padding参数表示在构件周围要保留多少个填充的像元。多数情况下将它的值设置为0。



#### table控件

##### GtkWidget *gtk_table_new( guint rows, guint columns, gboolean homogeneous );

rows: 行数
coumns: 列数
homogeneous：容器内表格的大小是否相等
返回值：表格布局容器指针

##### void gtk_table_attach_defaults(GtkTable *table, GtkWidget *widget,guint left_attach,guint right_attach,guint top_attach,guint bottom_attach );


table:  容纳控件的容器 
widget: 要添加的控件
后四个参数为控件摆放的坐标，规则如下：

![img](https://img-blog.csdn.net/20150115175902717)



#### fram控件

[(7条消息) GTK 框架(Frames)_大步朝前-CSDN博客](https://blog.csdn.net/qq61394323/article/details/37918267)



#### 滑动窗口控件ScrolledWindow

[(7条消息) GTK 滚动窗口构件(GtkScrolledWindow)_u012314708的博客-CSDN博客](https://blog.csdn.net/u012314708/article/details/50433408)

