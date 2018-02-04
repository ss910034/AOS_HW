#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <time.h> //標頭檔
#define SRVPORT 10005
#define CONNECT_NUM 5
#define MAX_NUM 80

struct node
{
    int group;
    char command[20];
    char name[20];
    char right[10];
    char file[20];
    char ao;
};
typedef struct node p;
struct AOS
{
    char r[20];
    char n[20];
    char f[20];
    char date[30];
    int g;
    int read;
    int write;
    struct AOS *next;
};
typedef struct AOS Node;
Node* ahead = NULL;
Node* chead = NULL;
Node* ohead = NULL;
Node* rhead = NULL;
Node* whead = NULL;
static void push(char na[],char new_name[],char rights[],int group,char time[]);
static int check(int gr,char name[]);
static int checkw(int gr,char name[]);
static void information(char name[],int gr);
static int reading(char new_name[],int rw);
static void readed(char new_name[]);
static int writing(char new_name[],int rw);
static void written(char new_name[]);
static void change(char file[],char right[]);
static int isEmpty(int gr);
int serverSock=-1;
int connfd[10];
void quit()										//輸入quit 結束Server程式
{
    char msg[10];
    while(1)
    {
        scanf("%s",msg);
        if(strcmp("quit",msg)==0)
        {
            printf("Byebye...\n");
            close(serverSock);
            exit(0);
        }
    }
}
void ser(int n);
int main()
{
    pthread_t thread;							//建立thread參數
    struct sockaddr_in serverAddr,cliaddr;
    char buffer[1024];
    socklen_t len;
    serverSock=socket(AF_INET,SOCK_STREAM,0);
    if(serverSock<0)
    {
        printf("socket creation failed\n");
        exit(-1);
    }
    printf("socket create successfully.\n");
    memset(&serverAddr,0,sizeof(serverAddr));
    serverAddr.sin_family=AF_INET;
    serverAddr.sin_port=htons((u_short) SRVPORT);
    serverAddr.sin_addr.s_addr=htons(INADDR_ANY);
    if(bind(serverSock,(struct sockaddr*)&serverAddr,sizeof(serverAddr))==-1)
    {
        printf("Bind error.\n");
        exit(-1);
    }
    printf("Bind successful.\n");

    if(listen(serverSock,10)==-1)
        printf("Listen error!\n");
    printf("Start to listen!\n");

    pthread_create(&thread,NULL,(void*)(&quit),NULL); //建立引線

    int i;
    for(i=0; i<10; connfd[i]=-1,i++);							//初始化 -1表示空閒的引線
    printf("Waiting connect.....\n");
    while(1)									//用無窮迴圈來等待使用者加入
    {
        len=sizeof(cliaddr);
        for(i=0; i<10; i++)						//找出未被使用的引線
            if(connfd[i]==-1)
                break;

        connfd[i]=accept(serverSock,(struct sockaddr*)&cliaddr,&len);	//accept
        pthread_create(malloc(sizeof(pthread_t)),NULL,(void*)(&ser),(void*)(intptr_t)i); //建立該號碼的引線
    }
    return 0;
}
void ser(int n)
{
    char buffer[1024];
    while(1)
    {
        p* r=malloc(sizeof(struct node));
        if(read(connfd[n],r,sizeof(struct node))<0)
        {
            printf("read error.\n");
            exit(1);
        }
        if(strcmp(r->command,"new")==0)
        {
            FILE * fp = fopen(r->file,"w");
            if(NULL == fp )
                printf("File can not open to write\n");
            else
                fclose(fp);
            time_t t1 = time(NULL);
            struct tm *nPtr = localtime(&t1);
            char now[30];
            strftime(now, 30, "%B %d %Y ", nPtr);
            push(r->name,r->file,r->right,r->group,now);
        }
        if(strcmp(r->command,"read")==0)
        {
            FILE * fp = fopen(r->file,"rb");
            if(NULL == fp )
            {
                printf("File:%s Not Found\n",r->file);
                strcpy(buffer,"Server don't find this file.\n");
                send(connfd[n],buffer,sizeof(buffer),0);
            }
            else
            {
                int read=check(r->group,r->file);
                int wring=writing(r->file,1);
                if(read==0)
                {
                    strcpy(buffer,"You don't have permission to read this!\n");
                    send(connfd[n],buffer,sizeof(buffer),0);
                }
                else if(wring==0)
                {
                    strcpy(buffer,"Someone is writting,you should wait a minute!\n");
                    send(connfd[n],buffer,sizeof(buffer),0);
                }
                else
                {
                    memset(buffer,0,1024);
                    strcpy(buffer,"ok\n");
                    send(connfd[n],buffer,sizeof(buffer),0);
                    int x=reading(r->file,0);
                    int file_block_length = 0;
                    while( (file_block_length = fread(buffer,sizeof(char),1024,fp))>0)
                    {

                        if(send(connfd[n],buffer,sizeof(buffer),0)<0)//發送buffer中的字串到server端
                        {
                            printf("Send File:\t%s Failed\n",r->file);
                            break;
                        }
                        memset(&buffer,0,1024);
                    }
                    strcpy(buffer,"over");
                    send(connfd[n],buffer,sizeof(buffer),0);
                    readed(r->file);
                    fclose(fp);
                    printf("%s Transfer Finished\n",r->file);
                }
            }
        }
        if(strcmp(r->command,"write")==0)
        {
            FILE *fp = fopen(r->file,"rb");
            if(NULL == fp )
            {
                printf("File:\t%s Not Found\n", r->file);
                strcpy(buffer,"Server don't find this file.\n");
                send(connfd[n],buffer,sizeof(buffer),0);
            }
            else
            {

                int write=checkw(r->group,r->file);
                int reing=reading(r->file,1);
                int wring=writing(r->file,1);
                if(write==0)
                {
                    strcpy(buffer,"You don't have permission to write this!\n");
                    send(connfd[n],buffer,sizeof(buffer),0);
                }
                else if(reing==0)
                {
                    strcpy(buffer,"Someone is reading,you should wait a minute!\n");
                    send(connfd[n],buffer,sizeof(buffer),0);
                }
                else if(wring==0)
                {
                    strcpy(buffer,"Someone is writting,you should wait a minute!\n");
                    send(connfd[n],buffer,sizeof(buffer),0);
                }
                else
                {
                    strcpy(buffer,"ok");
                    if(r->ao=='a') fp = fopen(r->file,"ab");
                    if(r->ao=='o') fp = fopen(r->file,"wb");
                    send(connfd[n],buffer,sizeof(buffer),0);
                    writing(r->file,0);
                    memset(&buffer,0,sizeof(buffer));
                    int lengt = 0;
                    while(1)	//一直一直接收檔案buffer 直到recv=0表示檔案收完
                    {
                        lengt = recv(connfd[n],buffer,1024,0);
                        if(strcmp(buffer,"over")==0)
                            break;
                        if(lengt < 0)						//判斷錯誤
                        {
                            printf("Recieve Data Failed!\n");
                            break;
                        }
                        int write_length = fwrite(buffer,sizeof(char),lengt,fp);//將收到內容寫入檔案
                        if (write_length<lengt)
                        {
                            printf("File:\t%s Write Failed\n",r->file);
                            break;
                        }
                        memset(&buffer,0,sizeof(buffer));
                    }
                    written(r->file);
                    printf("Recieve File:\t %s Finished\n",r->file);
                    fclose(fp);
                }
            }
        }
        if(strcmp(r->command,"change")==0)
            change(r->file,r->right);
        if(strcmp(r->command,"information")==0)
            information(r->file,n);
    }
}
static void change(char file[],char right[])
{
    int has;
    Node* tmp=ahead;
    Node* tmp1=chead;
    Node* tmp2=ohead;
    while(tmp!=NULL)
    {
        if(strcmp(tmp->f,file)==0)
        {
            has++;
            break;
        }
        tmp = tmp->next;
    }
    while(tmp1!=NULL)
    {
        if(strcmp(tmp1->f,file)==0)
        {
            has++;
            break;
        }
        tmp1 = tmp1->next;
    }
    while(tmp2!=NULL)
    {
        if(strcmp(tmp2->f,file)==0)
        {
            has++;
            break;
        }
        tmp2 = tmp2->next;
    }
    if(has)
    {
        if(right[0]=='r') tmp->read=1;
        else tmp->read=0;
        if(right[1]=='w') tmp->write=1;
        else tmp->write=0;
        if(right[2]=='r') tmp1->read=1;
        else tmp1->read=0;
        if(right[3]=='w') tmp1->write=1;
        else tmp1->write=0;
        if(right[4]=='r') tmp2->read=1;
        else tmp2->read=0;
        if(right[5]=='w') tmp2->write=1;
        else tmp2->write=0;
        strcpy(tmp->r,right);
        strcpy(tmp1->r,right);
        strcpy(tmp2->r,right);
    }
    else
        printf("No this file can change the mod.\n");
}
static int writing(char new_name[],int rw)
{
    if(rw==1)
    {
        int has=0;
        Node* ptr = whead;
        if(ptr==NULL) return 1;
        while(ptr!=NULL)
        {
            if(strcmp(ptr->f,new_name)==0)
            {
                has++;
                break;
            }
            ptr = ptr->next;
        }
        if(has)
            return 0;
    }
    else
    {
        Node *newr;
        newr = (struct AOS *)malloc(sizeof(struct AOS));
        strcpy(newr->f,new_name);
        newr->next=whead;
        whead=newr;
        return 1;
    }
}
static void written(char new_name[])
{
    Node* ptr = whead,*tmp=whead;
    if(whead->next==NULL)
        whead=NULL;
    else
    {
        while(ptr!=NULL)
        {
            if(strcmp(ptr->f,new_name)==0)
                break;
            tmp=ptr;
            ptr = ptr->next;
        }
        if(tmp==whead)
            whead=tmp->next;
        else if(ptr->next==NULL)
            tmp->next=NULL;
        else
            tmp->next=ptr->next;
    }

}
static int reading(char new_name[],int rw)
{
    if(rw==1)
    {
        int has=0;
        Node* ptr = rhead;
        if(ptr==NULL) return 1;
        while(ptr!=NULL)
        {
            if(strcmp(ptr->f,new_name)==0)
            {
                has++;
                break;
            }
            ptr = ptr->next;
        }
        if(has)
            return 0;
    }
    else
    {
        Node *newr;
        newr = (struct AOS *)malloc(sizeof(struct AOS));
        strcpy(newr->f,new_name);
        newr->next=rhead;
        rhead=newr;
        return 1;
    }
}
static void readed(char new_name[])
{
    Node* ptr = rhead,*tmp=rhead;
    if(rhead->next==NULL)
        rhead=NULL;
    else
    {
        while(ptr!=NULL)
        {
            if(strcmp(ptr->f,new_name)==0)
                break;
            tmp=ptr;
            ptr = ptr->next;
        }
        if(tmp==rhead)
            rhead=tmp->next;
        else if(ptr->next==NULL)
            tmp->next=NULL;
        else
            tmp->next=ptr->next;
    }
}
static void push(char na[],char new_name[],char rights[],int group,char time[])
{
    Node *newa,*newc,*newo;
    newa = (struct AOS *)malloc(sizeof(struct AOS));
    strcpy(newa->n,na);
    strcpy(newa->r,rights);
    strcpy(newa->f,new_name);
    strcpy(newa->date,time);
    newa->g=group;
    newa->read=(rights[0]=='r')?1:0;
    newa->write=(rights[1]=='w')?1:0;
    newa->next=ahead;
    ahead=newa;

    newc = (struct AOS *)malloc(sizeof(struct AOS));
    strcpy(newc->n,na);
    strcpy(newc->r,rights);
    strcpy(newc->f,new_name);
    strcpy(newc->date,time);
    newc->g=group;
    newc->read=(rights[2]=='r')?1:0;
    newc->write=(rights[3]=='w')?1:0;
    newc->next=ahead;
    chead=newc;

    newo = (struct AOS *)malloc(sizeof(struct AOS));
    strcpy(newo->n,na);
    strcpy(newo->r,rights);
    strcpy(newo->f,new_name);
    strcpy(newo->date,time);
    newo->g=group;
    newo->read=(rights[4]=='r')?1:0;
    newo->write=(rights[5]=='w')?1:0;
    newo->next=ohead;
    ohead=newo;
}
static void information(char name[],int gr)
{
    Node* n = (struct AOS *)malloc(sizeof(struct AOS));
    char buffer[1024];
    int has=0;
    Node* ptr;
    if(isEmpty(gr)) printf("The queue is empty.\n");
    switch(gr)
    {
    case 1:
        ptr = ahead;
        break;
    case 2:
        ptr = chead;
        break;
    case 3:
        ptr = ohead;
        break;
    }
    while(ptr!=NULL)
    {
        if(strcmp(ptr->f,name)==0)
        {
            has++;
            break;
        }
        ptr = ptr->next;
    }
    if(has)
    {
        strcpy(n->r,ptr->r);
        strcpy(n->n,ptr->n);
        n->g=ptr->g;
        strcpy(n->f,ptr->f);
        strcpy(n->date,ptr->date);
        int bytes=0,l=0;
        FILE *fp=fopen(ptr->f,"rb");
        while((l = fread(buffer,sizeof(char),1024,fp))>0)
        {
            bytes+=l;
        }
        fclose(fp);
        n->read=bytes;
    }
    send(connfd[gr],n,sizeof(struct AOS),0);
}
static int check(int gr,char name[])
{
    Node* ptr;
    switch(gr)
    {
    case 1:
        ptr=ahead;
        break;
    case 2:
        ptr=chead;
        break;
    case 3:
        ptr=ohead;
        break;
    }
    if(isEmpty(gr))printf("The queue is empty\n");
    while(ptr!=NULL)
    {
        if(strcmp(ptr->f,name)==0)
            return ptr->read;
        ptr = ptr->next;
    }
}
static int checkw(int gr,char name[])
{
    Node* ptr;
    switch(gr)
    {
    case 1:
        ptr=ahead;
        break;
    case 2:
        ptr=chead;
        break;
    case 3:
        ptr=ohead;
        break;
    }
    if(isEmpty(gr))printf("The queue is empty\n");
    while(ptr!=NULL)
    {
        if(strcmp(ptr->f,name)==0)
            return ptr->write;
        ptr = ptr->next;
    }
}
int isEmpty(int gr)
{
    switch(gr)
    {
    case 1:
        return (ahead == NULL);
        break;
    case 2:
        return (chead == NULL);
        break;
    case 3:
        return (ohead == NULL);
        break;
    }

}

