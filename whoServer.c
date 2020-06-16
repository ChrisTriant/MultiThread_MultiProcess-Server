#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <unistd.h>
void perror2(char* s,int e){
    fprintf(stderr," %s : %s \n",s,strerror(e));
}


typedef struct file_desc{
    int fd;
    int type; //0 for stats, 1 for clients
}file_desc;

typedef struct circ_buffer{
    file_desc** fd_array;
    int head;
    int tail;
    int size;
}circ_buffer;


void* statistic_n_clients(void* arguments);
int circ_buf_push(circ_buffer* circ_buf,file_desc* new_fd);
file_desc* circ_buf_pop(circ_buffer* circ_buf);

int Workers;
int countWorkers;
int readWorkers;

pthread_mutex_t wait_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t print_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cvar;

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

    circ_buffer* circ_buf=malloc(sizeof(circ_buffer));
    circ_buf->fd_array=malloc(bufferSize*sizeof(int*));
    for(int i=0;i<bufferSize;i++){
        circ_buf->fd_array[i]=NULL;
    }
    circ_buf->head=circ_buf->tail=0;
    circ_buf->size=bufferSize;

    int err;
    pthread_cond_init (&cvar , NULL ) ; /* Initialize condition variable */

    pthread_t* thread_Arr=malloc(numThreads*sizeof(pthread_t));

    for(int i=0;i<numThreads;i++){
        if ( err = pthread_create (thread_Arr+i , NULL , statistic_n_clients , ( void *) circ_buf ) ) {
            /* Create a thread */
            perror2 ( " pthread_create " , err ) ;
            exit (1) ;
        }
    }

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
    pthread_t thread;

    /*Accept*/

    while(1){
        if(countWorkers==0){
            sleep(1);
            printf("All the statistics have been received.\n");
            break;
        }
        mysock=accept(sock,(struct sockaddr*) 0,0);
        if(mysock==-1){
            perror("Accept failed");
        }else{
            file_desc* fd=malloc(sizeof(file_desc));
            fd->fd=mysock;
            fd->type=0;
            if(circ_buf_push(circ_buf,fd)<0){
                perror("Buffer is full\n");
                free(fd);
                continue;
            }
            if ( err = pthread_mutex_lock (&wait_mutex)){ /* Lock mutex */
                perror2(" pthread_mutex_lock " , err ); 
                exit(1); 
            }
            pthread_cond_signal (&cvar) ;
            if ( err = pthread_mutex_unlock (&wait_mutex)){ /* Lock mutex */
                perror2(" pthread_mutex_unlock " , err ); 
                exit(1); 
            }
            // int err;
            // if(err = pthread_create (&thread, NULL,statistic_n_clients,(void*)circ_buf)){     /* Create a thread */
            //     perror2 ( " pthread_create " , err ) ;
            //     exit (1) ;
            // }

            while(readWorkers==0){
                //wait for the number of workers to be received for the first time
            }
            countWorkers--;
        }
    }

    free(circ_buf->fd_array);
    free(circ_buf);
    
}


void* statistic_n_clients(void* buf){
    
    int err;
    if ( err = pthread_mutex_lock (&wait_mutex)){ /* Lock mutex */
        perror2(" pthread_mutex_lock " , err ); 
        exit(1); 
    }
    pthread_cond_wait(&cvar,&wait_mutex); /* Wait for signal */
    if ( err = pthread_mutex_unlock (&wait_mutex)){ /* Lock mutex */
        perror2(" pthread_mutex_unlock " , err ); 
        exit(1); 
    }
    printf("I woke up\n");
    getchar();
    int rval;
    circ_buffer* circ_buf= (circ_buffer*)buf;
    int bufferlen;
    file_desc* mysock;
    mysock=circ_buf_pop(circ_buf);
    // if ( err = pthread_mutex_unlock(&wait_mutex)) { /* Unlock mutex */
    //     perror2 ( " pthread_mutex_unlock " , err );
    //     exit (1);
    // }
 
    if(mysock->type==0){
        read(mysock->fd,&Workers,sizeof(int));
            if(readWorkers==0){
                countWorkers=Workers;
                readWorkers++;
            }
            printf("workers: %d\n",Workers);
            rval=read(mysock->fd,&bufferlen,sizeof(int));
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
    return NULL;    
}



int circ_buf_push(circ_buffer* circ_buf,file_desc* new_fd){
    int next;

    next = circ_buf->head + 1;  // next is where head will point to after this write.
    if (next >= circ_buf->size)
        next = 0;

    if (next == circ_buf->tail)  // if the head + 1 == tail, circular buffer is full
        return -1;


    circ_buf->fd_array[circ_buf->head] = new_fd;  // Load data and then move
    circ_buf->head = next;             // head to next data offset.
    return 0;  // return success to indicate successful push.
}


file_desc* circ_buf_pop(circ_buffer* circ_buf){
    int next;

    if (circ_buf->head == circ_buf->tail)  // if the head == tail, we don't have any data
        return NULL;

    next = circ_buf->tail + 1;  // next is where tail will point to after this read.
    if(next >= circ_buf->size)
        next = 0;

    file_desc* data = circ_buf->fd_array[circ_buf->tail];  // Read data and then move
    circ_buf->tail = next;              // tail to next offset.
    return data;
}