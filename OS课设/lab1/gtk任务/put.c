#include<stdio.h>
#include<stdlib.h>
#include<sys/ipc.h>
#include<sys/shm.h>
#include<sys/types.h>
#include<sys/sem.h>
#include<unistd.h>
#include<gtk/gtk.h>
#define SHMKEY 123   //readbuf与writebuf进程之间的共享内存key值
#define SIZEKEY 321  //写进程向读进程传递的放置长度公共内存的key值
#define SEMKEY 111  //公共信号灯集key值
#define SHMNUM 10   //定义共享内存组数
#define SHMSIZE 4096  //共享内存一组为4K
int shmid;
int sizeid;
int semid;  //0号指示empty，1号指示full
char *bufaddr;
int *sizeaddr;
void P(int semid, int index)
{
    struct sembuf sem;
    sem.sem_num = index;
    sem.sem_op = -1;
    sem.sem_flg = 0;
    semop(semid, &sem, 1);
    return;
}
void V(int semid, int index)
{
    struct sembuf sem;
    sem.sem_num = index;
    sem.sem_op = 1;
    sem.sem_flg = 0;
    semop(semid, &sem, 1);
    return;
}

char buf[1000];
GtkWidget *label;
GtkWidget *window;
GtkWidget *button;
pthread_t p1;

void *putbuf(void){
    if((shmid = shmget(SHMKEY, 40960, IPC_CREAT|0666)) == -1){
        printf("putbuf：get shared memory fail!\n");
        exit(1);
    }
    bufaddr = shmat(shmid,NULL,0);
    if ((sizeid = shmget(SIZEKEY, 4, IPC_CREAT|0666)) == -1){
        printf("putbuf：get size shared memory fail！\n");
        exit(1);
    }
    sizeaddr = shmat(sizeid,NULL,0);
    if ((semid = semget(SEMKEY, 2, IPC_CREAT|0666))== -1)
    {
        printf("putbuf：get semaphore fail!\n!");
        exit(1);
    }

    FILE *fp = fopen("text-copy.txt","w");
    int j = 0;
    char *now_out;
    while(1){
        P(semid,1);
        now_out = bufaddr + (j % SHMNUM) * SHMSIZE;
        if(*(now_out + SHMSIZE - 3) == 'E' && *(now_out + SHMSIZE - 2) == 'O' && *(now_out + SHMSIZE - 1) == 'F'){
            sprintf(buf,"now put buf num：%d",(int)(j+1));
            sleep(1);  //方便看清过程变化
            gtk_label_set_text(GTK_LABEL(label),buf);  //修改记数
            printf("now put buf num：%d\n",j + 1);
            if(*(sizeaddr) == 0){
                fwrite(now_out,1,SHMSIZE,fp);
            }
            else{
                fwrite(now_out,1,*(sizeaddr),fp);
            }
            V(semid,0);
            break;
        }
        sprintf(buf,"now put buf num：%d",(int)(j+1));
        sleep(1);  //方便看清过程变化
        gtk_label_set_text(GTK_LABEL(label),buf);  //修改记数

        printf("now put buf num：%d\n",j + 1);
        fwrite(now_out,1,SHMSIZE,fp);
        j++;
        V(semid,0);
    }
    printf("Putbuf done!\n");
}

int main (int argc, char *argv[])
{
    gtk_init(&argc, &argv);
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);//生成一个样式为GTK_WINDOW_TOPLEVEL的窗口
    gtk_window_set_title(GTK_WINDOW(window),"get_window");//设置窗口标题 
    gtk_window_set_default_size(GTK_WINDOW(window), 500, 400);//设置窗口默认大小
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);//设置窗口在显示器中的位置为居中

    g_signal_connect(G_OBJECT(window),"destroy",G_CALLBACK(gtk_main_quit),NULL);
   
    label=gtk_label_new("Start!");//创建标签
    int ret=pthread_create(&p1,NULL,&putbuf,NULL);
    gtk_container_add(GTK_CONTAINER(window), label); 	// 将按钮放在布局容器里

    gtk_widget_show(label);//显示标签
    gtk_widget_show(window);//显示生成的这个窗口
    gtk_main();//进入消息循环

    return 0;
}