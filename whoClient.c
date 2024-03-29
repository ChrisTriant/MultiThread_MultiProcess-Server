#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define SERVER_WRITEBUFFER_SIZE 150

void perror2(char* s,int e){
    fprintf(stderr," %s : %s \n",s,strerror(e));
}

void* query_fun(void* arguments);
int server_connect();


typedef struct circ_line_buffer{
    char** queryArray;     
    int buffer_end; 
    int size;  
    int count;           
    int head;       
    int tail;       
} circ_line_buffer;

// typedef struct query_arguments{
//     char* query;

// }query_arguments;

int threadsAvailabe;
int exit_program;
circ_line_buffer* circ_buff_init(circ_line_buffer *cb, int size);
int circ_buf_push(circ_line_buffer* circ_buf,char* new_line);
char* circ_buf_pop(circ_line_buffer* circ_buf);

pthread_mutex_t wait_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t print_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t circ_mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t cvar;


int servPort;
char* servIP;

int main(int argc,char** argv){

    int numThreads;
    int bufferSize;
    FILE* queryFile;

    int i=1;
    while(argv[i]!=NULL && strcmp(argv[i],"-q")!=0){
        i++;
    }
    if(argv[i]!=NULL){
        if(argv[i+1]!=NULL){
            if((queryFile=fopen(argv[i+1],"r"))==NULL){
                printf("Could not open file\n");
                exit(1);
            }
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

    circ_line_buffer* circ_buf; 
    circ_buf=circ_buff_init(circ_buf,numThreads);


    int forbroadcast=0;
    char line[150];
    int err;
    int threads_ready=0;
    threadsAvailabe=0;
    exit_program=0;
    int threadsCreated=0;

    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_cond_init (&cvar , NULL ) ; /* Initialize condition variable */

    while(!feof(queryFile)){
        if(threads_ready==0){
            if(forbroadcast<numThreads){
                char* l=fgets(line,sizeof(line),queryFile);
                if(l==NULL || strcmp(line,"\n")==0){
                    break;
                }
                forbroadcast++;
                pthread_t thread;
                char*query=malloc(strlen(line)+1);
                strcpy(query,line);
                circ_buf_push(circ_buf,query);
                if(err = pthread_create (&thread, &attr,query_fun,(void*)circ_buf)){     /* Create a thread */
                    perror2 ( " pthread_create " , err ) ;
                    exit (1) ;
                }
                threadsCreated++;
                continue;
            }else{
                forbroadcast=0;
                threads_ready=1;
            }
        }else{
            if(forbroadcast<numThreads){
                char* l=fgets(line,sizeof(line),queryFile);
                if(l==NULL || strcmp(line,"\n")==0){
                    break;
                }
                forbroadcast++;
                char*query=malloc(strlen(line)+1);
                strcpy(query,line);
                if(circ_buf_push(circ_buf,query)<0){
                    printf("buffer is full\n");
                }
                continue;
            }else{
                forbroadcast=0;
            }
        }
        while(threadsAvailabe!=threadsCreated);             //wait for all the threads to reach their wait state
        usleep(10000);
        pthread_cond_broadcast(&cvar);
        while(threadsAvailabe!=threadsCreated);
        usleep(10000);
        //printf("\nBroadCasted\n");
    }

    if(circ_buf->count>0){              
        while(threadsAvailabe!=threadsCreated);             //if there are still lines in the buffer
        usleep(10000);
        pthread_cond_broadcast(&cvar);
        while(threadsAvailabe!=threadsCreated);
        usleep(10000);
    }

    sleep(1);
    exit_program=1;
    while(threadsAvailabe!=threadsCreated);
    usleep(10000);
    printf("Goodbye\n");
    pthread_cond_broadcast(&cvar);
    usleep(1000);
    pthread_cond_destroy(&cvar);
    pthread_mutex_destroy(&wait_mutex);
    pthread_mutex_destroy(&print_mutex);
    pthread_attr_destroy(&attr);
    
    free(circ_buf->queryArray);
    free(circ_buf);
    free(servIP);
}


void* query_fun(void* args){

    circ_line_buffer* circ_buf=(circ_line_buffer*)args;
    while(1){
        int err;


        if ( err = pthread_mutex_lock (&wait_mutex)){ /* Lock mutex */
            perror2(" pthread_mutex_lock " , err ); 
            exit(1); 
        }
        threadsAvailabe++;
        pthread_cond_wait(&cvar,&wait_mutex); /* Wait for signal */

        if(exit_program==1){
            //pthread_detach(pthread_self());
            pthread_exit(NULL);
        }

        
        if ( err = pthread_mutex_lock (&circ_mutex)){ /* Lock mutex */
            perror2(" pthread_mutex_lock " , err ); 
            exit(1); 
        }
        threadsAvailabe--;
        char* query=circ_buf_pop(circ_buf);

        if ( err = pthread_mutex_unlock (&circ_mutex)){ /* Unlock mutex */
            perror2(" pthread_mutex_unlock " , err ); 
            exit(1); 
        }
        if ( err = pthread_mutex_unlock (&wait_mutex)){ /* Unlock mutex */
            perror2(" pthread_mutex_unlock " , err ); 
            exit(1); 
        }
        if ( err = pthread_mutex_lock (&print_mutex)){ /* Lock mutex */
            perror2(" pthread_mutex_lock " , err ); 
            exit(1); 
        }
        if(query!=NULL){
            int sock=server_connect();
            printf("\n%s\n",query);
            write(sock,query,SERVER_WRITEBUFFER_SIZE);
            int bufferlen;
            read(sock,&bufferlen,sizeof(int));
            char* buffer=malloc(bufferlen);
            while(1){
                read(sock,buffer,bufferlen);
                if(strcmp(buffer,"end_of_message")==0){
                    break;
                }
                printf("%s\n",buffer);
            }
            free(buffer);
            free(query);
            close(sock);
        }else{
            printf("No more queries\n");
        }
        if ( err = pthread_mutex_unlock (&print_mutex)){ /* Unlock mutex */
            perror2(" pthread_mutex_unlock " , err ); 
            exit(1); 
        }
    }

}



circ_line_buffer* circ_buff_init(circ_line_buffer* cb, int size){
    cb=malloc(sizeof(circ_line_buffer));
    cb->queryArray = malloc(size * sizeof(char*));
    cb->buffer_end = size;
    cb->size = size;
    cb->count = 0;
    cb->head = 0;
    cb->tail = 0;
    return cb;
}


int circ_buf_push(circ_line_buffer* circ_buf,char* new_line){
    if(circ_buf->count == circ_buf->size)
            return -1;
        circ_buf->queryArray[circ_buf->head]=new_line;
        circ_buf->head ++;
        if(circ_buf->head == circ_buf->buffer_end)
            circ_buf->head = 0;
        circ_buf->count++;
        return 0;
}


char* circ_buf_pop(circ_line_buffer* circ_buf){
  if(circ_buf->count == 0)
        return NULL;
    char* data=circ_buf->queryArray[circ_buf->tail];
    circ_buf->tail++;
    if(circ_buf->tail == circ_buf->buffer_end)
        circ_buf->tail = 0;
    circ_buf->count--;
    return data;
}

int server_connect(){
            //connect to the server
        int sock;
        struct sockaddr_in server;
        sock=socket(AF_INET,SOCK_STREAM,0);
        if(sock<0){
            perror("Failed to create a socket");
            return -1;
        }

        server.sin_family=AF_INET;
        server.sin_addr.s_addr=INADDR_ANY;
        server.sin_port=htons(servPort);

        struct hostent* foundhost ;
        struct in_addr myaddress ;

        inet_aton ( servIP , &myaddress ) ;
        foundhost = gethostbyaddr (( const char *) & myaddress , sizeof ( myaddress ) , AF_INET ) ;

        if(foundhost==0){
            perror("Hosting failed");
            return -1;
        }
            
        memcpy(&server.sin_addr,foundhost->h_addr_list[0],foundhost->h_length);
        server.sin_port=htons(servPort);
        while(connect(sock,(struct sockaddr*)&server,sizeof(server))<0){
            perror("Connection failed");
        }
        return sock;
}