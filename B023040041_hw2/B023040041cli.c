#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
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
} cl;

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
void client();
int clientSock=-1;
int main()
{
    struct sockaddr_in serverAddr;
    pthread_t thread;						 	//建立thread參數
    clientSock=socket(AF_INET,SOCK_STREAM,0);
    if(clientSock<0)
    {
        printf("socket creation failed\n");
        exit(-1);
    }
    printf("socket create successfully.\n");

    memset(&serverAddr,0,sizeof(serverAddr));
    serverAddr.sin_family=AF_INET;
    serverAddr.sin_port=htons((u_short) SRVPORT);
    serverAddr.sin_addr.s_addr=htons(INADDR_ANY);
    socklen_t server_addr_length = sizeof(serverAddr);
    if(connect(clientSock,(struct sockaddr*)&serverAddr,server_addr_length)<0)
    {
        printf("Connect error.\n");
        exit(-1);
    }
    printf("Connect successful.\n");
    client();
    return 0;
}
void client()
{
    char buffer[1024];
    char command[20];
    char file[20];
    char trd[10];
    char oa;
    char na[20];
    int groupc;
    struct node * r=malloc(sizeof(struct node));
    Node* y=(struct AOS *)malloc(sizeof(struct AOS));
    printf("Input Username : ");
    fgets(buffer, 1024, stdin);
    int i;
    for(i=0; i<1024; i++)
        if(buffer[i]=='\n')
            buffer[i]='\0';
    for(i=0; i<1024; buffer[i]=buffer[i+1],i++);
    printf("Choose which group belongs to you? 1.AOS 2.CSE 3.Other :");
    scanf(" %d",&cl.group);
    r->group=cl.group;
    while(1)
    {
        printf("[ Command ]\nnew\nread\nwrite\nchange\ninformation\n");
        scanf("%s",cl.command);
        strcpy(r->command,cl.command);
        switch(cl.command[0])
        {
        case 'n':
            scanf("%s",cl.file);
            scanf("%s",cl.right);
            strcpy(r->name,buffer);
            r->group=cl.group;
            strcpy(r->file,cl.file);
            strcpy(r->right,cl.right);
            write(clientSock,r,sizeof(struct node));
            break;
        case 'r':
            fgets(r->file,sizeof(r->file),stdin);
            for(i=0; i<20; i++)
                if(r->file[i]=='\n')
                    r->file[i]='\0';
            for(i=0; i<20; r->file[i]=r->file[i+1],i++);
            write(clientSock,r,sizeof(struct node));
            memset(buffer,0,1024);
            recv(clientSock,buffer,sizeof(buffer),0);
            if(strcmp(buffer,"ok\n")==0)
            {
                FILE * fp = fopen(r->file,"wb");			//開檔
                if(NULL == fp )
                    printf("Server can't find this file.\n");
                else
                {
                    memset(&buffer,0,sizeof(buffer));
                    int lengt = 0;
                    while(1)	//一直一直接收檔案buffer 直到recv=0表示檔案收完
                    {
                        lengt = recv(clientSock,buffer,1024,0);
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
                    fclose(fp);
                    printf("Recieve File:\t %s Finished\n",r->file);
                }
            }
            else
                printf("%s",buffer);
            break;
        case 'w':
            scanf("%s",cl.file);
            scanf("%c",&r->ao);
            scanf("%c",&r->ao);
            strcpy(r->file,cl.file);
            write(clientSock,r,sizeof(struct node));
            recv(clientSock,buffer,sizeof(buffer),0);
            if(strcmp(buffer,"ok")==0)
            {
                FILE * fp = fopen(r->file,"rb");
                if(NULL == fp )
                    printf("File:\t%s Not Found\n",r->file);
                else
                {
                    memset(&buffer,0,1024);
                    int file_block_length = 0;
                    while( (file_block_length = fread(buffer,sizeof(char),1024,fp))>0)
                    {
                        if(send(clientSock,buffer,file_block_length,0)<0)//發送buffer中的字串到server端
                        {
                            printf("Send File:\t%s Failed\n",r->file);
                            break;
                        }
                        memset(&buffer,0,1024);
                    }
                    strcpy(buffer,"over");
                    send(clientSock,buffer,sizeof(buffer),0);
                    fclose(fp);
                    printf("%s Transfer Finished\n",r->file);
                }
            }
            else
                printf("%s",buffer);
            break;
        case 'c':
            scanf("%s",cl.file);
            scanf("%s",cl.right);
            strcpy(r->file,cl.file);
            strcpy(r->right,cl.right);
            write(clientSock,r,sizeof(struct node));
            break;
        case 'i':
            scanf("%s",cl.file);
            strcpy(r->file,cl.file);
            write(clientSock,r,sizeof(struct node));
            recv(clientSock,y,sizeof(struct AOS),0);
            printf("%s %s ",y->r,y->n);
            switch(y->g)
            {
            case 1:
                printf("AOS ");
                break;
            case 2:
                printf("CSE ");
                break;
            case 3:
                printf("Other ");
                break;
            }
            printf("%d %s %s\n",y->read,y->date,y->f);
            break;
        }
    }
}
