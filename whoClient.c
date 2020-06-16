#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>



int main(int argc,char** argv){
    int servPort;
    char* servIP;
    int numThreads;
    int bufferSize;
    int queryFD;

    int i=1;
    while(argv[i]!=NULL && strcmp(argv[i],"-q")!=0){
        i++;
    }
    if(argv[i]!=NULL){
        if(argv[i+1]!=NULL){
            queryFD=open(argv[i+1],'r');
        }else{
            printf("ERROR! No query file was provided\n");
            exit(-1);
        }
    }else{
        printf("ERROR! No query file was provided\n");
        exit(-1);
    }

    i=1;
    while(argv[i]!=NULL && strcmp(argv[i],"-sp")!=0){
        i++;
    }
    if(argv[i]!=NULL){
        if(argv[i+1]!=NULL){
            servPort=atoi(argv[i+1]);
        }else{
            printf("ERROR! No query port number was provided\n");
            exit(-1);
        }
    }else{
        printf("ERROR! No query port number was provided\n");
        exit(-1);
    }

    i=1;
    while(argv[i]!=NULL && strcmp(argv[i],"-sip")!=0){
        i++;
    }
    if(argv[i]!=NULL){
        if(argv[i+1]!=NULL){
            servIP=malloc(strlen(argv[i+1])+1);
            strcpy(servIP,argv[i+1]);
        }else{
            printf("ERROR! No server IP address was provided\n");
            exit(-1);
        }
    }else{
        printf("ERROR! No server IP address was provided\n");
        exit(-1);
    }

    i=1;
    numThreads = 1;  //default value set to 1
    while(argv[i]!=NULL && strcmp(argv[i],"-w")!=0){
        i++;
    }
    if(argv[i]!=NULL){
        if(argv[i+1]!=NULL){
            numThreads=atoi(argv[i+1]);
            if(numThreads==0){
                printf("Error! Number of Threads has to be at least 1.\n");
                return -1;
            }
        }else{
            printf("Number of Threads was default set to 1.\n");
        }
    }else{
        printf("Number of Threads was default set to 1.\n");
    }

}