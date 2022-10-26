#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <glib.h>
#include <dirent.h>
#include <time.h>
#include <gtk/gtk.h>

gboolean settime(gpointer data);
gboolean setcpurate(gpointer data);
gboolean setmemuse(gpointer data);
char* B_2_higher(long b);
void myshutdown(void);
void myreboot(void);
gboolean setsysinfo(gpointer label);
gboolean drawcpuusing_callback(GtkWidget* widget);
gboolean drawcpuusing(gpointer widget);
gboolean setmem_rat(gpointer data);
gboolean setmem_fra(gpointer data);
gboolean drawmemusing_callback(GtkWidget* widget);
gboolean drawmemusing(gpointer widget);
gboolean setswap_rat(gpointer data);
gboolean setswap_fra(gpointer data);
gboolean drawswapusing_callback(GtkWidget* widget);
gboolean drawswapusing(gpointer widget);
void setprocinfo(void);
void setpidstat(char(*info)[1000], char* stat_info);
void searchproc(void);
void killproc(void);
void refreshproc(void);
void runproc(void);
char* gettxtdetailed(const char* pid);

GtkWidget* main_window;
GtkWidget* scrolled_window;
GtkWidget* vbox,* hbox;
GtkWidget* table;
GtkWidget* cpuuse_frame;
GtkWidget* cpuuse_draw;
GtkWidget* memuse_frame;
GtkWidget* memuse_draw;
GtkWidget* swapuse_frame;
GtkWidget* swapuse_draw;
GtkWidget* clist;
GtkWidget* entry01;
GtkWidget* notebook;
GtkWidget* button01;
GtkWidget* button02;
GtkWidget* button03;
GtkWidget* button04;
GtkWidget* time_label,* cpu_label,* mem_label;
GtkWidget* title_label;
GtkWidget* label01;
GtkWidget* label02;
long tt,newtt,idle,newidle;  //cpu各类时间
char titleinfo[100];   //页面标题
int cpu_curve_start = 20; //cpu曲线
long usage,usage_data[120]; //cpu数据记录
int mem_curve_start = 20; //mem曲线
long memusage, memusage_data[120]; //mem数据记录
int swap_curve_start = 20; //swap曲线
long swapusage, swapusage_data[120]; //swap数据记录
int proc_num;

int main(int argc, char *argv[]){
	gtk_init(&argc, &argv);
	/*创建主窗口*/
	main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(main_window), "my system monitor");
	gtk_window_set_default_size(GTK_WINDOW(main_window), 600, 700);
	gtk_container_set_border_width(GTK_CONTAINER(main_window), 10);
	gtk_window_set_position(GTK_WINDOW(main_window), GTK_WIN_POS_CENTER);
	gtk_window_set_policy(GTK_WINDOW(main_window),TRUE, TRUE, TRUE);
	g_signal_connect(G_OBJECT(main_window), "delete_event", G_CALLBACK(gtk_main_quit), NULL);


	/*创建笔记本控件*/
	notebook = gtk_notebook_new();
	gtk_notebook_set_tab_pos(GTK_NOTEBOOK(notebook), GTK_POS_TOP);

	/*页面1：主机信息*/
	/*显示主机名称，系统启动时间，持续运行时间，系统版本号，CPU型号，CPU主频*/
	/*关机功能，重启功能*/
	vbox = gtk_vbox_new(FALSE, 10);
	hbox = gtk_hbox_new(FALSE, 10);

	scrolled_window = gtk_scrolled_window_new(NULL, NULL);  //信息显示滑动窗口
	gtk_widget_set_size_request(scrolled_window, 500, 550);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	label01 = gtk_label_new(" ");
	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(scrolled_window), label01);
	g_timeout_add(1000, (GtkFunction)setsysinfo, (gpointer)label01);
	button01 = gtk_button_new_with_label("Shutdown");  //关机按钮
	g_signal_connect(G_OBJECT(button01), "clicked", G_CALLBACK(myshutdown), NULL);
	button02 = gtk_button_new_with_label("Reboot");  //重启按钮
	g_signal_connect(G_OBJECT(button02), "clicked", G_CALLBACK(myreboot), NULL);
	gtk_box_pack_start(GTK_BOX(hbox), button01, TRUE, FALSE, 5);
	gtk_box_pack_end(GTK_BOX(hbox), button02, TRUE, FALSE, 5);
	gtk_box_pack_start(GTK_BOX(vbox), scrolled_window, TRUE, FALSE, 5);
	gtk_box_pack_end(GTK_BOX(vbox), hbox, TRUE, FALSE, 0);

	sprintf(titleinfo, "System");
	title_label = gtk_label_new(titleinfo);
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), vbox, title_label);

	/*页面2：CPU使用率图形化*/
	cpuuse_frame = gtk_frame_new("CPU Using rate");
	gtk_container_set_border_width(GTK_CONTAINER(cpuuse_frame), 5);
	gtk_widget_set_size_request(cpuuse_frame, 520, 300);
	cpuuse_draw = gtk_drawing_area_new();
	gtk_widget_set_size_request(cpuuse_draw, 0, 0);
	g_signal_connect(G_OBJECT(cpuuse_draw), "expose_event", G_CALLBACK(drawcpuusing_callback), NULL);
	gtk_container_add(GTK_CONTAINER(cpuuse_frame), cpuuse_draw);

	sprintf(titleinfo, "CPU");
	title_label = gtk_label_new(titleinfo);
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), cpuuse_frame, title_label);

	/*页面3：内存使用率图形化和swap使用率图形化*/
	vbox = gtk_vbox_new(FALSE, 0);
	
	hbox = gtk_hbox_new(FALSE, 0);  //构造memuse控件
	gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, FALSE, 5);
	memuse_frame = gtk_frame_new("Memory Using rate");
	gtk_container_set_border_width(GTK_CONTAINER(memuse_frame), 5);
	gtk_widget_set_size_request(memuse_frame, 520, 270);
	gtk_box_pack_start(GTK_BOX(hbox), memuse_frame, TRUE, FALSE, 5);
	memuse_draw = gtk_drawing_area_new();
	gtk_widget_set_size_request(memuse_draw, 0, 0);
	g_signal_connect(G_OBJECT(memuse_draw), "expose_event", G_CALLBACK(drawmemusing_callback), NULL);
	gtk_container_add(GTK_CONTAINER(memuse_frame), memuse_draw);

	hbox = gtk_hbox_new(FALSE, 0);  //构造文字说明控件
	label01 = gtk_label_new("memory using rate");
	label02 = gtk_label_new("memory using fraction");
	gtk_box_pack_start(GTK_BOX(hbox), label01, TRUE, FALSE, 5);
	gtk_box_pack_start(GTK_BOX(hbox), label02, TRUE, FALSE, 5);
	g_timeout_add(1000, (GtkFunction)setmem_rat, (gpointer)label01);
	g_timeout_add(1000, (GtkFunction)setmem_fra, (gpointer)label02);
	gtk_widget_set_size_request(hbox, 550, 20);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, FALSE, 5);

	hbox = gtk_hbox_new(FALSE, 0);  //构造swapuse控件
	gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, FALSE, 5);
	swapuse_frame = gtk_frame_new("Swap Using rate");
	gtk_container_set_border_width(GTK_CONTAINER(swapuse_frame), 5);
	gtk_widget_set_size_request(swapuse_frame, 520, 270);
	gtk_box_pack_start(GTK_BOX(hbox), swapuse_frame, TRUE, FALSE, 5);
	swapuse_draw = gtk_drawing_area_new();
	gtk_widget_set_size_request(swapuse_draw, 0, 0);
	g_signal_connect(G_OBJECT(swapuse_draw), "expose_event", G_CALLBACK(drawswapusing_callback), NULL);
	gtk_container_add(GTK_CONTAINER(swapuse_frame), swapuse_draw);

	hbox = gtk_hbox_new(FALSE, 0);  //构造文字说明控件
	label01 = gtk_label_new("swap using rate");
	label02 = gtk_label_new("swap using fraction");
	gtk_box_pack_start(GTK_BOX(hbox), label01, TRUE, FALSE, 5);
	gtk_box_pack_start(GTK_BOX(hbox), label02, TRUE, FALSE, 5);
	g_timeout_add(1000, (GtkFunction)setswap_rat, (gpointer)label01);
	g_timeout_add(1000, (GtkFunction)setswap_fra, (gpointer)label02);
	gtk_widget_set_size_request(hbox, 550, 20);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, FALSE, 5);

	sprintf(titleinfo, "Memory");
	title_label = gtk_label_new(titleinfo);
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), vbox, title_label);

	/*页面4：进程信息*/
	vbox = gtk_vbox_new(FALSE, 0);
	scrolled_window = gtk_scrolled_window_new(NULL, NULL);  //信息显示滑动窗口
	gtk_widget_set_size_request(scrolled_window, 500, 550);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	clist = gtk_clist_new(6);  //构造列信息框架
	setprocinfo();
	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(scrolled_window), clist);
	gtk_box_pack_start(GTK_BOX(vbox), scrolled_window, TRUE, TRUE, 5);
	hbox = gtk_hbox_new(FALSE, 10);
	entry01 = gtk_entry_new();   //输入框与相关按钮
	gtk_entry_set_max_length(GTK_ENTRY(entry01), 0);
	button01 = gtk_button_new_with_label("Search");
	button02 = gtk_button_new_with_label("Kill");
	button03 = gtk_button_new_with_label("Refresh");
	button04 = gtk_button_new_with_label("Run");
	g_signal_connect(G_OBJECT(button01), "clicked", G_CALLBACK(searchproc), NULL);
	g_signal_connect(G_OBJECT(button02), "clicked", G_CALLBACK(killproc), NULL);
	g_signal_connect(G_OBJECT(button03), "clicked", G_CALLBACK(refreshproc), NULL);
	g_signal_connect(G_OBJECT(button04), "clicked", G_CALLBACK(runproc), NULL);
	gtk_widget_set_size_request(entry01, 150, 30);
	gtk_widget_set_size_request(button01, 70, 30);
	gtk_widget_set_size_request(button02, 70, 30);
	gtk_widget_set_size_request(button03, 70, 30);
	gtk_widget_set_size_request(button04, 70, 30);
	gtk_box_pack_start(GTK_BOX(hbox), entry01, FALSE, FALSE, 5);
	gtk_box_pack_start(GTK_BOX(hbox), button01, FALSE, FALSE, 5);
	gtk_box_pack_start(GTK_BOX(hbox), button02, FALSE, FALSE, 5);
	gtk_box_pack_start(GTK_BOX(hbox), button03, FALSE, FALSE, 5);
	gtk_box_pack_start(GTK_BOX(hbox), button04, FALSE, FALSE, 5);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 5);

	sprintf(titleinfo, "Process");
	title_label = gtk_label_new(titleinfo);
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), vbox, title_label);

	/*创建状态栏*/
	time_label = gtk_label_new("Time");
	cpu_label = gtk_label_new("CPU rate");
	mem_label = gtk_label_new("Memory using");

	/*创建表格布局容器*/
	table = gtk_table_new(30, 3, TRUE);
	gtk_table_attach_defaults(GTK_TABLE(table), notebook, 0, 3, 0, 28);
	gtk_table_attach_defaults(GTK_TABLE(table), time_label, 0, 1, 28, 30);
	gtk_table_attach_defaults(GTK_TABLE(table), cpu_label, 1, 2, 28, 30);
	gtk_table_attach_defaults(GTK_TABLE(table), mem_label, 2, 3, 28, 30);

	gtk_container_add(GTK_CONTAINER(main_window), table);
	g_timeout_add(1000, settime, NULL);
	g_timeout_add(1000, setcpurate, (gpointer)cpu_label);
	g_timeout_add(1000, setmemuse, (gpointer)mem_label);
	gtk_widget_show_all(main_window);
	gtk_main();
	return 0;
}


gboolean settime(gpointer data){
	time_t times;
	struct tm* p_time;
	time(&times);
	p_time = localtime(&times);
	gchar* text_data = g_strdup_printf("%04d/%02d/%02d", \
		(1900 + p_time->tm_year), (1 + p_time->tm_mon), (p_time->tm_mday));
	gchar* text_time = g_strdup_printf("%02d:%02d:%02d", \
		(p_time->tm_hour), (p_time->tm_min), (p_time->tm_sec));
	gchar* text_markup = g_strdup_printf("%s\n%s", text_time, text_data);
	gtk_label_set_markup(GTK_LABEL(time_label), text_markup);
	return TRUE;
}

gboolean setcpurate(gpointer data) {
	FILE* fp;
	char cpuinfo[100];
	fp = fopen("/proc/stat", "r");
	fgets(cpuinfo, sizeof(cpuinfo), fp);
	fclose(fp);
	char cpuname[50];
	long Nuser, Nnice, Nsystem, Nidle, Niowait, Nirq, Nsoftirq, ext1, ext2, ext3;
	sscanf(cpuinfo, "%s%ld%ld%ld%ld%ld%ld%ld%ld%ld%ld", cpuname, &Nuser, &Nnice, &Nsystem, &Nidle, &Niowait, &Nirq, &Nsoftirq, &ext1, &ext2, &ext3);
	newidle = Nidle;
	newtt = Nuser + Nnice + Nsystem + Nidle + Niowait + Nirq + Nsoftirq + ext1 + ext2 + ext3;
	if (tt > 0) usage = ((newtt - tt) - (newidle - idle)) * 100 / (newtt - tt);
	char cpurate[50];
	sprintf(cpurate, "CPU rate:\n%ld%%", usage);
	gtk_label_set_text(GTK_LABEL(data), cpurate);
	tt = newtt;
	idle = newidle;
	return TRUE;
}

gboolean setmemuse(gpointer data) {
	FILE* fp;
	char totalmeminfo[50], freememinfo[50];
	char temp1[20], temp2[20];
	long ttm, frem;
	fp = fopen("/proc/meminfo", "r");
	fgets(totalmeminfo, sizeof(totalmeminfo), fp);
	fgets(freememinfo, sizeof(freememinfo), fp);
	fclose(fp);
	sscanf(totalmeminfo, "%s%ld%s", temp1, &ttm, temp2);
	sscanf(freememinfo, "%s%ld%s", temp1, &frem, temp2);
	long usem = ttm - frem;  //目前为KB
	char cum[20];
	strcpy(cum, B_2_higher(usem * 1024));
	char usingmem[50];
	sprintf(usingmem, "Memory using:\n%s", cum);
	gtk_label_set_text(GTK_LABEL(data), usingmem);
	return TRUE;
}

gboolean setmem_rat(gpointer data) {
	FILE* fp;
	char totalmeminfo[50], freememinfo[50];
	char temp1[20], temp2[20];
	long ttm, frem;
	fp = fopen("/proc/meminfo", "r");
	fgets(totalmeminfo, sizeof(totalmeminfo), fp);
	fgets(freememinfo, sizeof(freememinfo), fp);
	fclose(fp);
	sscanf(totalmeminfo, "%s%ld%s", temp1, &ttm, temp2);
	sscanf(freememinfo, "%s%ld%s", temp1, &frem, temp2);
	memusage = (long)((ttm - frem)*100 / ttm);
	char usingmem[50];
	sprintf(usingmem, "Memory using rate:%ld%%", memusage);
	gtk_label_set_text(GTK_LABEL(data), usingmem);
	return TRUE;
}

gboolean setmem_fra(gpointer data) {
	FILE* fp;
	char totalmeminfo[50], freememinfo[50];
	char temp1[20], temp2[20];
	long ttm, frem;
	fp = fopen("/proc/meminfo", "r");
	fgets(totalmeminfo, sizeof(totalmeminfo), fp);
	fgets(freememinfo, sizeof(freememinfo), fp);
	fclose(fp);
	sscanf(totalmeminfo, "%s%ld%s", temp1, &ttm, temp2);
	sscanf(freememinfo, "%s%ld%s", temp1, &frem, temp2);
	long usem = ttm - frem;  //目前为KB
	char charum[20],chartm[20];
	strcpy(charum, B_2_higher(usem * 1024));
	strcpy(chartm, B_2_higher(ttm * 1024));
	char usingmem[50];
	sprintf(usingmem, "%s/%s", charum,chartm);
	gtk_label_set_text(GTK_LABEL(data), usingmem);
	return TRUE;
}

gboolean setswap_rat(gpointer data) {
	long ttswap, freswap;
	char temp1[20], temp2[20];
	FILE* fp;
	fp = fopen("/proc/meminfo", "r");
	char info01[30] = "SwapTotal";
	char info02[30] = "SwapFree";
	char temp[50];
	int flag = 0;
	while (flag != 2) {
		fgets(temp, sizeof(temp), fp);
		if (fp == NULL) break;
		if (strncmp(temp, info01, 9) == 0) {
			sscanf(temp, "%s%ld%s", temp1, &ttswap, temp2);
			flag++;
		}
		else if (strncmp(temp, info02, 7) == 0) {
			sscanf(temp, "%s%ld%s", temp1, &freswap, temp2);
			flag++;
		}
	}
	fclose(fp);
	swapusage = (long)((ttswap - freswap) * 100 / ttswap);
	char usingswap[50];
	sprintf(usingswap, "Swap using rate:%ld%%", swapusage);
	gtk_label_set_text(GTK_LABEL(data), usingswap);
	return TRUE;
}

gboolean setswap_fra(gpointer data) {
	long ttswap, freswap;
	char temp1[20], temp2[20];
	FILE* fp;
	fp = fopen("/proc/meminfo", "r");
	char info01[30] = "SwapTotal";
	char info02[30] = "SwapFree";
	char temp[50];
	int flag = 0;
	while (flag != 2) {
		fgets(temp, sizeof(temp), fp);
		if (fp == NULL) break;
		if (strncmp(temp, info01, 9) == 0) {
			sscanf(temp, "%s%ld%s", temp1, &ttswap, temp2);
			flag++;
		}
		else if (strncmp(temp, info02, 7) == 0) {
			sscanf(temp, "%s%ld%s", temp1, &freswap, temp2);
			flag++;
		}
	}
	fclose(fp);
	long useswap = ttswap - freswap;
	char charus[20], charts[20];
	strcpy(charus, B_2_higher(useswap * 1024));
	strcpy(charts, B_2_higher(ttswap * 1024));
	char usingswap[50];
	sprintf(usingswap, "%s/%s", charus, charts);
	gtk_label_set_text(GTK_LABEL(data), usingswap);
	return TRUE;
}

char* B_2_higher(long b){
	static char g[10];
	if (b > 999999999)
		sprintf(g, "%.2f GB", (float)b / 1073741824);
	else {
		if (b > 999999) sprintf(g, "%.2f MB", (float)b / 1048576);
		else {
			if (b > 999) sprintf(g, "%.2f KB", (float)b / 1024);
			else sprintf(g, "%ld B", b);
		}
	}
	return g;
}

void myshutdown(void) {
	system("shutdown -h now");
	return;
}

void myreboot(void) {
	system("shutdown -r now");
	return;
}

gboolean setsysinfo(gpointer label) {
	char hostname[50];
	char system_startup_time[50];
	char system_continued_time[50];
	char system_version[50];
	char CPU_model[100];
	char CPU_dominant_frequency[50];
	char temp01[50];
	char temp02[50];
	char temp03[50];
	char temp04[50];
	char temp05[100];
	int len;
	FILE* fp;
	fp = fopen("/proc/sys/kernel/hostname", "r");  //获取主机名称
	fgets(temp01, sizeof(temp01), fp);
	fclose(fp);
	len = strlen(temp01);
	temp01[len - 1] = '\0';
	sprintf(hostname, "Hostname: %s", temp01);
	fp = fopen("/proc/uptime", "r");   //获取系统启动时间和运行时间
	fgets(temp02, sizeof(temp02), fp);
	fclose(fp);
	double st, ft,ct;
	sscanf(temp02, "%lf%lf", &st, &ft);
	ct = st - ft/4;
	long h, m, s;
	h = (long)(st / 3600);
	m = (long)(st - h * 3600) / 60;
	s = (long)(st - h*3600 - m*60);
	sprintf(system_startup_time, "System startup time: %02ldh %02ldmin %02ldsec", h,m,s);
	h = (long)(ct / 3600);
	m = (long)(ct - h * 3600) / 60;
	s = (long)(ct - h * 3600 - m * 60);
	sprintf(system_continued_time, "System continued time: %02ldh %02ldmin %02ldsec", h, m, s);
	fp = fopen("/proc/sys/kernel/ostype", "r");   //获取系统内核
	fgets(temp03, sizeof(temp03), fp);
	fclose(fp);
	fp = fopen("/proc/sys/kernel/osrelease", "r");   //获取系统版本号
	fgets(temp04, sizeof(temp04), fp);
	fclose(fp);
	len = strlen(temp03);
	temp03[len-1] = '\0';
	len = strlen(temp04);
	temp04[len-1] = '\0';
	sprintf(system_version, "System version: %s %s", temp03,temp04);
	fp = fopen("/proc/cpuinfo", "r");   //获取CPU信息
	char info01[30] = "model name";
	char info02[30] = "cpu MHz";
	int flag = 0;
	while (flag != 2) {
		fgets(temp05, sizeof(temp05), fp);
		if (fp == NULL) break;
		if (strncmp(temp05, info01, 10) == 0) {
			int i;
			len = strlen(temp05); 
			for (i = 13; i <= len; i++) {
				temp05[i - 13] = temp05[i];
			}
			len = strlen(temp05);
			temp05[len-1] = '\0';
			sprintf(CPU_model, "CPU model name: %s", temp05);
			flag++;
		}
		else if (strncmp(temp05, info02, 7) == 0) {
			double hz;
			sscanf(temp05, "%s%s%s%lf", temp01, temp02,temp03,&hz);
			sprintf(CPU_dominant_frequency, "CPU dominant frequency: %.2lfMHZ", hz);
			flag++;
		}
	}
	fclose(fp);
	char totalinfo[300];
	sprintf(totalinfo, "\n%s\n\n\n%s\n\n\n%s\n\n\n%s\n\n\n%s\n\n\n%s\n", hostname,system_startup_time,system_continued_time,system_version,CPU_model,CPU_dominant_frequency);
	gtk_label_set_text(GTK_LABEL(label), totalinfo);
	return TRUE;
}

gboolean drawcpuusing_callback(GtkWidget* widget) {
	static int flag = 0;
	drawcpuusing((gpointer)widget);
	if (flag == 0) {
		g_timeout_add(1000, (GtkFunction)drawcpuusing, (gpointer)widget);
		flag = 1;
	}
	return TRUE;
}

gboolean drawcpuusing(gpointer widget) {
	GtkWidget* cpu_curve = (GtkWidget*)widget;
	GdkColor color;
	GdkGC* gc = cpu_curve->style->fg_gc[GTK_WIDGET_STATE(widget)];
	static int flag = 0;
	static int now_pos = 0;
	int draw_pos = 0;

	color.red = 0;
	color.green = 0;
	color.blue = 0;
	gdk_gc_set_rgb_fg_color(gc, &color);
	gdk_draw_rectangle(cpu_curve->window, gc, TRUE, 15, 30, 480, 220);  //填充背景

	color.red = 0;
	color.green = 20000;
	color.blue = 0;
	gdk_gc_set_rgb_fg_color(gc, &color);
	for (int i = 30; i <= 230; i += 20)
		gdk_draw_line(cpu_curve->window, gc, 15, i, 495, i);  //画横线
	for (int i = 15; i <= 480; i += 20)
		gdk_draw_line(cpu_curve->window, gc, i + cpu_curve_start, 30, i + cpu_curve_start, 230);  //画纵线

	cpu_curve_start -= 4;  //重复画线
	if (cpu_curve_start == 0)
		cpu_curve_start = 20;

	if (flag == 0) {  //初始化数据
		for (int i = 0; i < 120; i++) {
			usage_data[i] = 0;
			flag = 1;
		}
	}
	usage_data[now_pos] = usage; //添加数据
	now_pos++;
	if (now_pos == 120)
		now_pos = 0;

	color.red = 0;
	color.green = 65535;
	color.blue = 0;
	gdk_gc_set_rgb_fg_color(gc, &color);
	draw_pos = now_pos;
	for (int i = 0; i < 119; i++) {  //画线
		gdk_draw_line(cpu_curve->window, gc,
			15 + i * 4, 230 - 2 * usage_data[draw_pos % 120],
			15 + (i + 1) * 4, 230-2 * usage_data[(draw_pos + 1) % 120]);
		draw_pos++;
		if (draw_pos == 120)
			draw_pos = 0;
	}

	color.red = 25000;
	color.green = 25000;
	color.blue = 25000;
	gdk_gc_set_rgb_fg_color(gc, &color);
	return TRUE;
}

gboolean drawmemusing_callback(GtkWidget* widget) {
	static int flag = 0;
	drawmemusing((gpointer)widget);
	if (flag == 0) {
		g_timeout_add(1000, (GtkFunction)drawmemusing, (gpointer)widget);
		flag = 1;
	}
	return TRUE;
}

gboolean drawmemusing(gpointer widget) {
	GtkWidget* mem_curve = (GtkWidget*)widget;
	GdkColor color;
	GdkGC* gc = mem_curve->style->fg_gc[GTK_WIDGET_STATE(widget)];
	static int flag = 0;
	static int now_pos = 0;
	int draw_pos = 0;

	color.red = 0;
	color.green = 0;
	color.blue = 0;
	gdk_gc_set_rgb_fg_color(gc, &color);
	gdk_draw_rectangle(mem_curve->window, gc, TRUE, 15, 30, 480, 220);  //填充背景

	color.red = 0;
	color.green = 20000;
	color.blue = 0;
	gdk_gc_set_rgb_fg_color(gc, &color);
	for (int i = 30; i <= 230; i += 20)
		gdk_draw_line(mem_curve->window, gc, 15, i, 495, i);  //画横线
	for (int i = 15; i <= 480; i += 20)
		gdk_draw_line(mem_curve->window, gc, i + mem_curve_start, 30, i + mem_curve_start, 230);  //画纵线

	mem_curve_start -= 4;  //重复画线
	if (mem_curve_start == 0)
		mem_curve_start = 20;

	if (flag == 0) {  //初始化数据
		for (int i = 0; i < 120; i++) {
			memusage_data[i] = 0;
			flag = 1;
		}
	}
	memusage_data[now_pos] = memusage; //添加数据
	now_pos++;
	if (now_pos == 120)
		now_pos = 0;

	color.red = 0;
	color.green = 65535;
	color.blue = 0;
	gdk_gc_set_rgb_fg_color(gc, &color);
	draw_pos = now_pos;
	for (int i = 0; i < 119; i++) {  //画线
		gdk_draw_line(mem_curve->window, gc,
			15 + i * 4, 230 - 2 * memusage_data[draw_pos % 120],
			15 + (i + 1) * 4, 230 - 2 * memusage_data[(draw_pos + 1) % 120]);
		draw_pos++;
		if (draw_pos == 120)
			draw_pos = 0;
	}

	color.red = 25000;
	color.green = 25000;
	color.blue = 25000;
	gdk_gc_set_rgb_fg_color(gc, &color);
	return TRUE;
}

gboolean drawswapusing_callback(GtkWidget* widget) {
	static int flag = 0;
	drawswapusing((gpointer)widget);
	if (flag == 0) {
		g_timeout_add(1000, (GtkFunction)drawswapusing, (gpointer)widget);
		flag = 1;
	}
	return TRUE;
}

gboolean drawswapusing(gpointer widget) {
	GtkWidget* swap_curve = (GtkWidget*)widget;
	GdkColor color;
	GdkGC* gc = swap_curve->style->fg_gc[GTK_WIDGET_STATE(widget)];
	static int flag = 0;
	static int now_pos = 0;
	int draw_pos = 0;

	color.red = 0;
	color.green = 0;
	color.blue = 0;
	gdk_gc_set_rgb_fg_color(gc, &color);
	gdk_draw_rectangle(swap_curve->window, gc, TRUE, 15, 30, 480, 220);  //填充背景

	color.red = 0;
	color.green = 20000;
	color.blue = 0;
	gdk_gc_set_rgb_fg_color(gc, &color);
	for (int i = 30; i <= 230; i += 20)
		gdk_draw_line(swap_curve->window, gc, 15, i, 495, i);  //画横线
	for (int i = 15; i <= 480; i += 20)
		gdk_draw_line(swap_curve->window, gc, i + swap_curve_start, 30, i + swap_curve_start, 230);  //画纵线

	swap_curve_start -= 4;  //重复画线
	if (swap_curve_start == 0)
		swap_curve_start = 20;

	if (flag == 0) {  //初始化数据
		for (int i = 0; i < 120; i++) {
			swapusage_data[i] = 0;
			flag = 1;
		}
	}
	swapusage_data[now_pos] = swapusage; //添加数据
	now_pos++;
	if (now_pos == 120)
		now_pos = 0;

	color.red = 0;
	color.green = 65535;
	color.blue = 0;
	gdk_gc_set_rgb_fg_color(gc, &color);
	draw_pos = now_pos;
	for (int i = 0; i < 119; i++) {  //画线
		gdk_draw_line(swap_curve->window, gc,
			15 + i * 4, 230 - 2 * swapusage_data[draw_pos % 120],
			15 + (i + 1) * 4, 230 - 2 * swapusage_data[(draw_pos + 1) % 120]);
		draw_pos++;
		if (draw_pos == 120)
			draw_pos = 0;
	}

	color.red = 25000;
	color.green = 25000;
	color.blue = 25000;
	gdk_gc_set_rgb_fg_color(gc, &color);
	return TRUE;
}

void setprocinfo(void) {
	gtk_clist_set_column_title(GTK_CLIST(clist), 0, "PID");
	gtk_clist_set_column_title(GTK_CLIST(clist), 1, "Name");
	gtk_clist_set_column_title(GTK_CLIST(clist), 2, "State");
	gtk_clist_set_column_title(GTK_CLIST(clist), 3, "PPID");
	gtk_clist_set_column_title(GTK_CLIST(clist), 4, "Priority");
	gtk_clist_set_column_title(GTK_CLIST(clist), 5, "Memory size");
	gtk_clist_set_column_width(GTK_CLIST(clist), 0, 50);
	gtk_clist_set_column_width(GTK_CLIST(clist), 1, 150);
	gtk_clist_set_column_width(GTK_CLIST(clist), 2, 70);
	gtk_clist_set_column_width(GTK_CLIST(clist), 3, 70);
	gtk_clist_set_column_width(GTK_CLIST(clist), 4, 70);
	gtk_clist_set_column_width(GTK_CLIST(clist), 5, 100);
	gtk_clist_column_titles_show(GTK_CLIST(clist));
	DIR* dir;
	struct dirent* dirinfo;
	int fd;
	char pid_path[50];
	char pid_info[1000];
	char* one_file;
	char ttinfo[6][1000];
	gchar* listinfo[6];
	proc_num = 0;
	dir = opendir("/proc");
	while ((dirinfo = readdir(dir))!= NULL) {
		if (((dirinfo->d_name)[0] >= '0') && ((dirinfo->d_name)[0] <= '9')) {  //只打开数字pid的文件
			sprintf(pid_path, "/proc/%s/stat", dirinfo->d_name);
			fd = open(pid_path, O_RDONLY);
			read(fd, pid_info, 1000);
			close(fd);
			one_file = pid_info;
			setpidstat(ttinfo, one_file);
			for (int i = 0; i < 6; i++) {
				listinfo[i] = g_locale_to_utf8(ttinfo[i], -1, NULL, NULL, NULL);
			}
			gtk_clist_append(GTK_CLIST(clist), listinfo);
			proc_num++;
		}
	}
	closedir(dir);
	return;
}

void setpidstat(char(*info)[1000], char* stat_info) {
	int i;

	for (i = 0; i < 1000; i++) {  //获取pid
		if (stat_info[i] == ' ') break;
	}
	stat_info[i] = '\0';
	strcpy(info[0], stat_info);
	i += 2;
	stat_info += i;

	for (i = 0; i < 1000; i++) {  //获取name
		if (stat_info[i] == ')') break;
	}
	stat_info[i] = '\0';
	strcpy(info[1], stat_info);
	i += 2;
	stat_info += i;

	for (i = 0; i < 1000; i++) {  //获取state
		if (stat_info[i] == ' ') break;
	}
	stat_info[i] = '\0';
	strcpy(info[2], stat_info);
	i += 1;
	stat_info += i;

	for (i = 0; i < 1000; i++) {  //获取ppid
		if (stat_info[i] == ' ') break;
	}
	stat_info[i] = '\0';
	strcpy(info[3], stat_info);
	i += 1;
	stat_info += i;

	int j = 0;
	for (i = 0; i < 1000; i++) {  //获取priority
		if (stat_info[i] == ' ') j++;
		if (j == 13) break;
	}
	stat_info[i] = '\0';
	i += 1;
	stat_info += i;
	for (i = 0; i < 1024; i++) {
		if (stat_info[i] == ' ') break;
	}
	stat_info[i] = '\0';
	strcpy(info[4], stat_info);
	i += 1;
	stat_info += i;

	j = 0;
	for (i = 0; i < 1000; i++) {  //获取size
		if (stat_info[i] == ' ') j++;
		if (j == 5) break;
	}
	stat_info[i] = '\0';
	i += 1;
	stat_info += i;
	for (i = 0; i < 1024; i++) {
		if (stat_info[i] == ' ') break;
	}
	stat_info[i] = '\0';
	long size = atoi(stat_info);  //单位为pagesize
	int page = getpagesize();  //单位为B
	size = size * page;
	strcpy(info[5], B_2_higher(size));
	return;
}

void searchproc(void) {
	const gchar* entry_txt;
	gchar* list_txt;
	entry_txt = gtk_entry_get_text(GTK_ENTRY(entry01));
	gint i = 0;
	while (gtk_clist_get_text(GTK_CLIST(clist), i, 0, &list_txt)) {
		if (strcmp(entry_txt, list_txt) == 0) break;
		i++;
	}
	if (i >= proc_num) {
		GtkWidget* win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
		GtkWidget* label = gtk_label_new("Search fail!");
		gtk_widget_set_size_request(win, 200, 180);
		gtk_container_add(GTK_CONTAINER(win), label);
		gtk_window_set_title(GTK_WINDOW(win), "ERROR");
		gtk_window_set_position(GTK_WINDOW(win), GTK_WIN_POS_CENTER);
		gtk_widget_show_all(win);
		return;
	}
	else {
		GtkWidget* win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
		char detailed[1024];
		strcpy(detailed, gettxtdetailed(entry_txt));
		GtkWidget* label = gtk_label_new(detailed);
		gtk_widget_set_size_request(win, 300, 450);
		gtk_container_add(GTK_CONTAINER(win), label);
		gtk_window_set_title(GTK_WINDOW(win), "DETAILED");
		gtk_window_set_position(GTK_WINDOW(win), GTK_WIN_POS_CENTER);
		gtk_widget_show_all(win);
	}
	gtk_clist_select_row(GTK_CLIST(clist), i, 0);
	return;
}

void killproc(void) {
	const gchar* entry_txt;
	gchar* list_txt;
	entry_txt = gtk_entry_get_text(GTK_ENTRY(entry01));
	gint i = 0;
	while (gtk_clist_get_text(GTK_CLIST(clist), i, 0, &list_txt)) {
		if (strcmp(entry_txt, list_txt) == 0) break;
		i++;
	}
	gtk_clist_select_row(GTK_CLIST(clist), i, 0);
	if (entry_txt == NULL) return;
	int ret = kill(atoi(entry_txt), SIGKILL);
	if (ret) {
		GtkWidget* killfailwin = gtk_window_new(GTK_WINDOW_TOPLEVEL);
		GtkWidget* killfaillabel = gtk_label_new("Kill fail!");
		gtk_widget_set_size_request(killfailwin, 300, 180);
		gtk_container_add(GTK_CONTAINER(killfailwin), killfaillabel);
		gtk_window_set_title(GTK_WINDOW(killfailwin), "ERROR");
		gtk_window_set_position(GTK_WINDOW(killfailwin), GTK_WIN_POS_CENTER);
		gtk_widget_show_all(killfailwin);
	}
	return;
}

void refreshproc(void) {
	gtk_clist_freeze(GTK_CLIST(clist));
	gtk_clist_clear(GTK_CLIST(clist));
	setprocinfo();
	gtk_clist_thaw(GTK_CLIST(clist));
	gtk_clist_select_row(GTK_CLIST(clist), 0, 0);
	gtk_entry_set_text(GTK_ENTRY(entry01), "");
	return;
}

void runproc(void) {
	const gchar* entry_txt;
	entry_txt = gtk_entry_get_text(GTK_ENTRY(entry01));
	char txt[30];
	sprintf(txt, "%s&", entry_txt);
	int ret = system(txt);
	if (ret == -1||ret==127) {
		GtkWidget* win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
		GtkWidget* label = gtk_label_new("Run fail!");
		gtk_widget_set_size_request(win, 200, 180);
		gtk_container_add(GTK_CONTAINER(win), label);
		gtk_window_set_title(GTK_WINDOW(win), "ERROR");
		gtk_window_set_position(GTK_WINDOW(win), GTK_WIN_POS_CENTER);
		gtk_widget_show_all(win);
		return;
	}
	return;
}

char* gettxtdetailed(const char* pid) {
	char temp[1024] = "";
	char* p = temp;
	char pid_path[50];
	char pid_info[1000];
	char* stat_info;
	sprintf(pid_path, "/proc/%s/stat", pid);
	int fd = open(pid_path, O_RDONLY);
	read(fd, pid_info, 1000);
	close(fd);
	stat_info = pid_info;
	int i;

	for (i = 0; i < 1000; i++) {  //获取pid
		if (stat_info[i] == ' ') break;
	}
	stat_info[i] = '\0';
	strcat(temp, "PID: ");
	strcat(temp, stat_info);
	strcat(temp, "\n\n");
	i += 2;
	stat_info += i;

	for (i = 0; i < 1000; i++) {  //获取name
		if (stat_info[i] == ')') break;
	}
	stat_info[i] = '\0';
	strcat(temp, "Name: ");
	strcat(temp, stat_info);
	strcat(temp, "\n\n");
	i += 2;
	stat_info += i;

	for (i = 0; i < 1000; i++) {  //获取state
		if (stat_info[i] == ' ') break;
	}
	stat_info[i] = '\0';
	strcat(temp, "State: ");
	strcat(temp, stat_info);
	strcat(temp, "\n\n");
	i += 1;
	stat_info += i;

	for (i = 0; i < 1000; i++) {  //获取ppid
		if (stat_info[i] == ' ') break;
	}
	stat_info[i] = '\0';
	strcat(temp, "PPID: ");
	strcat(temp, stat_info);
	strcat(temp, "\n\n");
	i += 1;
	stat_info += i;

	for (i = 0; i < 1000; i++) {  //获取pgid
		if (stat_info[i] == ' ') break;
	}
	stat_info[i] = '\0';
	strcat(temp, "PGID: ");
	strcat(temp, stat_info);
	strcat(temp, "\n\n");
	i += 1;
	stat_info += i;

	for (i = 0; i < 1000; i++) {  //获取sid
		if (stat_info[i] == ' ') break;
	}
	stat_info[i] = '\0';
	strcat(temp, "SID: ");
	strcat(temp, stat_info);
	strcat(temp, "\n\n");
	i += 1;
	stat_info += i;

	for (i = 0; i < 1000; i++) {  //获取tty_nr
		if (stat_info[i] == ' ') break;
	}
	stat_info[i] = '\0';
	strcat(temp, "TTY_NR: ");
	strcat(temp, stat_info);
	strcat(temp, "\n\n");
	i += 1;
	stat_info += i;

	for (i = 0; i < 1000; i++) {  //获取tty_pgrp
		if (stat_info[i] == ' ') break;
	}
	stat_info[i] = '\0';
	strcat(temp, "TTY_PGRP: ");
	strcat(temp, stat_info);
	strcat(temp, "\n\n");
	i += 1;
	stat_info += i;

	for (i = 0; i < 1000; i++) {  //获取task_flag
		if (stat_info[i] == ' ') break;
	}
	stat_info[i] = '\0';
	strcat(temp, "Task Flags: ");
	strcat(temp, stat_info);
	strcat(temp, "\n\n");
	i += 1;
	stat_info += i;

	return p;
}