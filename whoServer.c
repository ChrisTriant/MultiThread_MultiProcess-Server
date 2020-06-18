#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <unistd.h>
#include "structs.h"
#include "fun.h"
void perror2(char* s,int e){
    fprintf(stderr," %s : %s \n",s,strerror(e));
}


void* statistic_n_clients(void* arguments);
circ_buffer* circ_buff_init(circ_buffer* cb, int size);
int circ_buf_push(circ_buffer* circ_buf,file_desc* new_fd);
file_desc* circ_buf_pop(circ_buffer* circ_buf);
countryList* serverListInsert(countryList* node,char* name);

int Workers;
int countWorkers;
int readWorkers;
int availableThreads;
int idx;

pthread_mutex_t wait_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t print_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t work_mutex = PTHREAD_MUTEX_INITIALIZER;
//pthread_mutex_t waitServ_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cvar;
//pthread_cond_t cvarServ;

workerInfo** worker_info;

int main(int argc,char** argv){
    int queryPortNum;
    int statisticsPortNum;
    int numThreads;
    int bufferSize;


    int i=1;
    while(argv[i]!=NULL && strcmp(argv[i],"-s")!=0){
        i++;
    }
    if(argv[i]!=NULL){
        if(argv[i+1]!=NULL){
            statisticsPortNum=atoi(argv[i+1]);
        }else{
            printf("ERROR! No statistics port number was provided\n");
            exit(-1);
        }
    }else{
        printf("ERROR! No statistics port number was provided\n");
        exit(-1);
    }

    i=1;
    while(argv[i]!=NULL && strcmp(argv[i],"-q")!=0){
        i++;
    }
    if(argv[i]!=NULL){
        if(argv[i+1]!=NULL){
            queryPortNum=atoi(argv[i+1]);
        }else{
            printf("ERROR! No query port number was provided\n");
            exit(-1);
        }
    }else{
        printf("ERROR! No query port number was provided\n");
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
    i=1;
    bufferSize=1;
    while(argv[i]!=NULL && strcmp(argv[i],"-b")!=0){
        i++;
    }
    if(argv[i]!=NULL){
        if(argv[i+1]!=NULL){
            bufferSize=atoi(argv[i+1]);
            if(bufferSize<1){
                printf("Buffer size is too small and will be set to 1\n");
            }
        }else{
            printf("Size of buffer was default set to 1.\n");
        }
    }else{
        printf("Size of buffer was default set to 1.\n");
    }

    circ_buffer* circ_buf=circ_buff_init(circ_buf,bufferSize);
    for(int i=0;i<bufferSize;i++){
        circ_buf->fd_array[i]=NULL;
    }


    arguments* args=malloc(sizeof(arguments));
    args->circ_buf=circ_buf;
    //args->servWait=0;

    availableThreads=0;
    int err;
    pthread_cond_init (&cvar , NULL ) ; /* Initialize condition variable */

    pthread_t* thread_Arr=malloc(numThreads*sizeof(pthread_t));

    for(int i=0;i<numThreads;i++){
        if ( err = pthread_create (thread_Arr+i , NULL , statistic_n_clients , ( void *) args ) ) {
            /* Create a thread */
            perror2 ( " pthread_create " , err ) ;
            exit (1) ;
        }
    }

    //int threadCounter=numThreads;


    int sock;
    struct sockaddr_in server;
    int mysock;

    /*Creating a socket for the workers*/

    sock=socket(AF_INET,SOCK_STREAM,0);
    if(sock<0){
        perror("Failed to create a socket");
        exit(1);
    }

    server.sin_family=AF_INET;
    server.sin_addr.s_addr=INADDR_ANY;
    server.sin_port=htons(statisticsPortNum);

    /*Bind & Listen*/

    if(bind(sock,(struct sockaddr*) &server,sizeof(server))){
        perror("Bind failed");
        exit(1);
    }

    listen(sock,bufferSize);


    Workers=-1;
    countWorkers=-1;
    readWorkers=0;


    /*Accept*/

    while(1){
        if(countWorkers==0){
            sleep(1);
            printf("All the statistics have been received.\n");
            break;
        }
        mysock=accept(sock,(struct sockaddr*) 0,0);
        if(mysock==-1){
            perror("\nACCEPT FAILED\n");
        }else{
            file_desc* fd=malloc(sizeof(file_desc));
            fd->fd=mysock;
            fd->type=0;
            while(availableThreads==0);
            if(circ_buf_push(circ_buf,fd)<0){
                perror("Buffer is full\n");
                free(fd);
                continue;
            } 

            // if(threadCounter==0){
            //     pthread_t thread;
            //     args->servWait=1;
            //     if(err = pthread_create (&thread, NULL,statistic_n_clients,(void*)args)){     /* Create a thread */
            //         perror2 ( " pthread_create " , err ) ;
            //         exit (1) ;
            //     }
            //     if ( err = pthread_mutex_lock (&waitServ_mutex)){ /* Lock mutex */
            //         perror2(" pthread_mutex_lock " , err ); 
            //         exit(1); 
            //     }
            //     pthread_cond_wait (&cvarServ,&waitServ_mutex) ;
            //     args->servWait=0;
            //     if ( err = pthread_mutex_unlock (&waitServ_mutex)){ /* Unlock mutex */
            //         perror2(" pthread_mutex_unlock " , err ); 
            //         exit(1); 
            //     }
            // }else{
            //     threadCounter--;
            // }

            if ( err = pthread_mutex_lock (&wait_mutex)){ /* Lock mutex */
                perror2(" pthread_mutex_lock " , err ); 
                exit(1); 
            }
            pthread_cond_signal (&cvar) ;
            if ( err = pthread_mutex_unlock (&wait_mutex)){ /* Lock mutex */
                perror2(" pthread_mutex_unlock " , err ); 
                exit(1); 
            }


            while(readWorkers==0){
                //wait for the number of workers to be received for the first time
            }
            countWorkers--;
        }
    }

    for(int i=0;i<Workers;i++){
        countryList* temp=worker_info[i]->countries;
        printf("\nPort number: %d\nCountries:\n",worker_info[i]->port_num);
        while(temp!=NULL){
            printf("%s\n",temp->countryName);
            temp=temp->next;
        }
    }

    free(circ_buf->fd_array);
    free(circ_buf);
    for(int i=0;i<Workers;i++){
        countryList* temp=worker_info[i]->countries;
        while(worker_info[i]->countries!=NULL){
            temp=worker_info[i]->countries;
            worker_info[i]->countries=worker_info[i]->countries->next;
            free(temp->countryName);
            free(temp);
        }
        free(worker_info[i]);
    }
    
}


void* statistic_n_clients(void* argum){
    while(1){
        int err;

        arguments* args=(arguments*)argum;


        if ( err = pthread_mutex_lock (&wait_mutex)){ /* Lock mutex */
            perror2(" pthread_mutex_lock " , err ); 
            exit(1); 
        }
        availableThreads++;
        pthread_cond_wait(&cvar,&wait_mutex); /* Wait for signal */
        availableThreads--;
        printf("\nI woke up\n");

        circ_buffer* circ_buf= args->circ_buf;
        file_desc* mysock;
        mysock=circ_buf_pop(circ_buf);
        if(mysock==NULL){
            printf("Buffer is empty\n");
        }
        if ( err = pthread_mutex_unlock (&wait_mutex)){ /* Lock mutex */
            perror2(" pthread_mutex_unlock " , err ); 
            exit(1); 
        }


        int bufferlen;
        int rval;
        int port_num;
        int index;
    
        if(mysock->type==0){
            read(mysock->fd,&port_num,sizeof(int));
            read(mysock->fd,&Workers,sizeof(int));
                if ( err = pthread_mutex_lock (&work_mutex)){ /* Lock mutex */
                    perror2(" pthread_mutex_lock " , err ); 
                    exit(1); 
                }
                if(readWorkers==0){
                    countWorkers=Workers;
                    idx=Workers;
                    worker_info=malloc(Workers*sizeof(workerInfo*));
                    readWorkers++;
                }
                index=Workers-idx;
                idx--;
                worker_info[index]=malloc(sizeof(workerInfo));
                worker_info[index]->port_num=port_num;
                if ( err = pthread_mutex_unlock (&work_mutex)){ /* Lock mutex */
                    perror2(" pthread_mutex_unlock " , err ); 
                    exit(1); 
                }
                
                rval=read(mysock->fd,&bufferlen,sizeof(int));
                int readCountry=0;
            while(1){
                if(rval<0){
                    perror("Reading error");
                }else if(!rval){
                    printf("Message ended\n");
                    break;
                }else{
                    char* buf=malloc(bufferlen);
                    memset(buf,0,bufferlen);
                    read(mysock->fd,buf,bufferlen);
                    if(strcmp(buf,"new")==0){
                        readCountry=0;
                        read(mysock->fd,buf,bufferlen);
                    }
                    if(strcmp(buf,"done")==0){
                        printf("\n\n");
                        break;
                    }
                    if ( err = pthread_mutex_lock (&print_mutex)){ /* Lock mutex */
                        perror2(" pthread_mutex_lock " , err ); 
                        exit(1); 
                    }
                    printf("\n\n\n%s\n",buf);
                    memset(buf,0,bufferlen);
                    read(mysock->fd,buf,bufferlen);
                    printf("\n%s\n",buf);
                    if(readCountry==0){
                        worker_info[index]->countries=serverListInsert(worker_info[index]->countries,buf);
                        readCountry=1;
                    }
                    memset(buf,0,bufferlen);
                    read(mysock->fd,buf,bufferlen);
                    int* agenums=malloc(4*sizeof(int));
                    memcpy(agenums,buf,4*sizeof(int));
                    printf("\nAge range 0-20 years: %d\n",agenums[0]);
                    printf("\nAge range 21-40 years: %d\n",agenums[1]);
                    printf("\nAge range 41-60 years: %d\n",agenums[2]);
                    printf("\nAge range 61+ years: %d\n",agenums[3]);
                    if ( err = pthread_mutex_unlock (&print_mutex)){ /* Lock mutex */
                        perror2(" pthread_mutex_unlock " , err ); 
                        exit(1); 
                    }
                    free(agenums);
                    memset(buf,0,bufferlen);
                }
            }
        }
        close(mysock->fd);
        free(mysock);
    }
    return NULL;    
}



circ_buffer* circ_buff_init(circ_buffer* cb, int size){
    cb=malloc(sizeof(circ_buffer));
    cb->fd_array = malloc(size * sizeof(file_desc*));
    cb->buffer_end = size;
    cb->size = size;
    cb->count = 0;
    cb->head = 0;
    cb->tail = 0;
    return cb;
}


int circ_buf_push(circ_buffer* circ_buf,file_desc* new_fd){
    if(circ_buf->count == circ_buf->size)
            return -1;
        circ_buf->fd_array[circ_buf->head]=new_fd;
        circ_buf->head ++;
        if(circ_buf->head == circ_buf->buffer_end)
            circ_buf->head = 0;
        circ_buf->count++;
        return 0;
}


file_desc* circ_buf_pop(circ_buffer* circ_buf){
    if(circ_buf->count == 0)
        return NULL;
    file_desc* data=circ_buf->fd_array[circ_buf->tail];
    circ_buf->tail++;
    if(circ_buf->tail == circ_buf->buffer_end)
        circ_buf->tail = 0;
    circ_buf->count--;
    return data;
}

countryList* serverListInsert(countryList* node,char* name){
    char* new_name=malloc(strlen(name)+1);
    strcpy(new_name,name);
    countryList* new_data=malloc(sizeof(CountryData));
    new_data->countryName=new_name;
    new_data->next=NULL;
    countryList* temp=node;
    if(node==NULL){
        node=new_data;
    }else{
        while(temp->next!=NULL){
            temp=temp->next;
        }
        temp->next=new_data;
    }
    return node;
}