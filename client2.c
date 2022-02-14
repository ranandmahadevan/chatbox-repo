#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include<sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include<unistd.h>
#include <pthread.h>
#include<arpa/inet.h>

char name[32];
int clientfd,flag=0;

void overwrite_stdout(){
    printf("%s",">");
    fflush(stdout);//used claer the flush
}

//fn to trim the message 
void trim_str(char* array,int length)
{
    for(int i= 0; i<length;i++)
    {
        if(array[i]=='\n')
        {
            array[i] = '\0';
            break;
        }
    }
}

//fn to send the message
void send_message()
{
    char message[2048];
    char buffer[2048];
    while(1)
    {
        overwrite_stdout();
        fgets(message,2048,stdin);
        trim_str(message,2048);
     if(strcmp(message,"exit")==0)
     {
        send(clientfd,message,strlen(message),0);
        break;
     }
     else
     {

        sprintf(buffer,"%s: %s\n",name,message);     
        send(clientfd,buffer,strlen(buffer),0);

     }
    bzero(message,2048);
    bzero(buffer,2048);
    }
    flag=1;

}

//fn to receive the message
void receive_message()
{
    char message[2048];
    while(1)
    {
        int receive = recv(clientfd,message,2048,0);
    if(receive>0)
    {
        printf("%s",message);
        overwrite_stdout();
    }
    else if(receive==0)
        break;
    bzero(message,2048);
    }
}

void main(int argc, char *argv[])
{

    struct sockaddr_in serveraddress;
    pthread_t sendmessage, receivemessage;

    //to check the port 
    if(argc!=3)
    {
        printf("usage; %s",argv[0]);
        exit(1);
    }

    printf("\nenter the name:");
    fgets(name,32,stdin);


    // socket creation 
    clientfd = socket(AF_INET,SOCK_STREAM,0);


    //nulling the serveraddress
    memset(&serveraddress,0,sizeof(serveraddress));
    //initialize 
    serveraddress.sin_family = AF_INET;
    serveraddress.sin_port = htons(atoi(argv[2]));
   serveraddress.sin_addr.s_addr = inet_addr(argv[1]);    

    // connect the client to the server
    if(connect(clientfd ,(struct sockaddr*)&serveraddress,sizeof(serveraddress)))
    {
        printf("not Connected\n");
    }

    trim_str(name,32);
    send(clientfd,name,32,0);
    printf("--------CHATBOX----------\n");
    //sending message
    pthread_create(&sendmessage,NULL,(void*)send_message,NULL);

    //receive message
    pthread_create(&receivemessage, NULL,(void*) receive_message,NULL);

    while(1)
    {
        if(flag){
            printf("\nbye\n");
            break;
        }
    }

    close(clientfd);

}
