#include<stdio.h>
#include<stdlib.h>
#include<sys/ipc.h>
#include<sys/shm.h>
#include<sys/types.h>
#include<sys/sem.h>
#include<unistd.h>
#include<sys/wait.h>
#include<gtk/gtk.h>
#define SHMKEY 123   //readbuf与writebuf进程之间的共享内存key值
#define SIZEKEY 321  //写进程向读进程传递的放置长度公共内存的key值
#define SEMKEY 111  //公共信号灯集key值
#define SHMNUM 10   //定义共享内存数
int shmid;
int sizeid;
int semid;  //0号指示empty，1号指示full
pid_t preadbuf,pwritebuf;
union semun{
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};

GtkWidget *window;
GtkWidget *label;
GtkWidget *button;
GtkWidget *box;

void main_process(void){
    //创建共享内存组
    if((shmid = shmget(SHMKEY, 40960, IPC_CREAT|0666)) == -1){   //40960 = 4K * 10
        printf("get shared memory fail!\n");
        exit(1);
    }
    //用于读进程获取剩余的待写字节数
    if ((sizeid = shmget(SIZEKEY, 4, IPC_CREAT|0666)) == -1){
        printf("get size shared memory fail！\n");
        exit(1);
    }
    //创建信号灯组
    if((semid = semget(SEMKEY, 2, IPC_CREAT|0666)) == -1){
        printf("get semaphore fail!\n");
        exit(1);
    }
    union semun sem_arg[2];
    sem_arg[0].val = SHMNUM;
    sem_arg[1].val = 0;
    if(semctl(semid, 0, SETVAL, sem_arg[0]) == -1){
        printf("initialize semaphroe 'empty' fail!");
        exit(1);
    }
    if(semctl(semid, 1, SETVAL, sem_arg[1]) == -1){
        printf("initialize semaphroe 'full' fail!");
        exit(1);
    }
    //创建进程readbuf和writebuf
    if ((pwritebuf = fork()) == 0) {	 //写共享内存进程
        printf("p_getbuf create!\n");
        execv("./get",NULL);
	}
    else {
        if ((preadbuf = fork()) == 0) { //读共享内存进程
            printf("p_putbuf create!\n");
            execv("./put",NULL);
        }
        else{   //父进程
            //等待子进程结束
            wait(NULL);
            wait(NULL);
            //删除信号灯和共享内存组
            semctl(semid, 0, IPC_RMID, NULL);
            shmctl(shmid, IPC_RMID, 0);
            shmctl(sizeid, IPC_RMID, 0);
            gtk_label_set_text(GTK_LABEL(label),"ALL DONE!");
            gtk_button_set_label(GTK_BUTTON(button),"Again");
        }
    }
}


int main(int argc,char *argv[]){
    gtk_init(&argc, &argv);
    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window),"main_window");
    g_signal_connect (G_OBJECT (window), "destroy",G_CALLBACK(gtk_main_quit), NULL);//关闭窗口操作
    gtk_container_set_border_width (GTK_CONTAINER (window), 150);

    box = gtk_vbox_new(FALSE,0);

    button = gtk_button_new_with_label ("Start");
    g_signal_connect (G_OBJECT (button), "clicked",G_CALLBACK (main_process), NULL);

    label = gtk_label_new("Begin!");
    gtk_box_pack_start (GTK_BOX(box),label,FALSE,FALSE,15);
    gtk_box_pack_start (GTK_BOX(box),button,FALSE,FALSE,0);

    gtk_container_add (GTK_CONTAINER (window), box);
    gtk_widget_show_all (window);
     
    gtk_main ();
    return 0;
}