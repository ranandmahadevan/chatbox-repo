#include<stdio.h>
#include<string.h>
#include<arpa/inet.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<unistd.h>
#include<pthread.h>

static int id=0;
static int client_ct=0;//client counter
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

//struct for the client 
typedef struct{
    int fd,cid;
    struct sockaddr_in address;
    char name[32];
}client;

client *clients[100];

void client_add(client* cli)
{
    pthread_mutex_lock(&lock);
    for(int i=0; i<100; i++){
        if(!clients[i]){
            clients[i] = cli;
            break;
        }

    }
    pthread_mutex_unlock(&lock);
}


void remove_client(int cid)
{
    pthread_mutex_lock(&lock);
    for(int i=0;i<100;i++)
    {
        if(clients[i]){
            if(clients[i]->cid == cid){
                clients[i]=NULL;
                break;
            }
        }
    }
    pthread_mutex_unlock(&lock);

}


void send_message(char* message, int cid)
{
    pthread_mutex_lock(&lock);
    for(int i=0;i<100;i++)
    {
        if(clients[i])
        if(clients[i]->cid!=cid)
        {
            write(clients[i]->fd,message,strlen(message));
        }
    }
    pthread_mutex_unlock(&lock);    
}

void* clienthandler(void *arg)
{
  
    char name[32],message[2048];
    client *temp = (client*)arg;
    client_ct++;

    bzero(message,sizeof(message));
    //name
            if(recv(temp->fd,name,32,0)>0){
            strcpy(temp->name,name);  
            sprintf(message,"%s joined the chatbox\n ",temp->name);
            printf("%s",message);
            send_message(message,temp->cid);

            }



    bzero(message,sizeof(message));
    while(1)
    {
        if(recv(temp->fd,message,sizeof(message),0))
        {
        if((strcmp(message,"exit")==0)||strcmp(message,"EXIT")==0)
        {
            sprintf(message,"%s left the chatbox\n",temp->name);
            printf("%s",message);
            send_message(message,temp->cid);
            break;
        }
        else{
            printf("%s",message);
            send_message(message,temp->cid);
        }
        }

        bzero(message,2048);

    }
    close(temp->fd);

    remove_client(temp->cid);
    client_ct--;
    pthread_detach(pthread_self());
    return NULL;
}

void main(int argc,char** argv)
{
    if(argc!=2)
    {
        printf("\nenter the valid port number");
        exit(1);
    }
    int serverfd,clientfd;
    struct sockaddr_in serveraddress, clientaddress;
    pthread_t pid;

    serverfd = socket(AF_INET,SOCK_STREAM,0);

    //zeroing
    bzero(&serveraddress,sizeof(serveraddress));

    //initializing
    serveraddress.sin_family = AF_INET;
    serveraddress.sin_port =  htons(atoi(argv[1]));
    serveraddress.sin_addr.s_addr = htonl(INADDR_ANY);
    //bind fd with serversocket
    if(bind(serverfd,(struct sockaddr*)&serveraddress,sizeof(serveraddress))==-1)
    {
        printf("socket did'nt bind");
        exit(1);
    }


    //listen
    if(listen(serverfd,20)<0)
    {
        printf("listening failed");
        exit(1);
    }

    //zeroing the clientsocket
    bzero(&clientaddress,sizeof(clientaddress));

    printf("--------chatbox has been created--------\n");
    while(1)
    {
        //accepting the client
        socklen_t clientlen = sizeof(clientaddress);
        clientfd = accept(serverfd,(struct sockaddr*)&clientaddress,&clientlen);

        if(client_ct+1==100)
        {
            printf("the chatbox is full");
            close(clientfd);
            continue;
        }

        //adding the client
        client *cli = (client*)malloc(sizeof(client)) ;
        cli->address = clientaddress;
        cli->fd = clientfd;
        cli->cid = id++;

        //add the queue
        client_add(cli);

        pthread_create(&pid,NULL,&clienthandler,(void*)cli);
    }
    
    close(serverfd);
}