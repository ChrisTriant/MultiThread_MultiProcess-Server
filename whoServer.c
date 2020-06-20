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
#include "structs.h"
//#include "fun.h"

#define CLIENT_READBUFFER_SIZE 150


void perror2(char* s,int e){
    fprintf(stderr," %s : %s \n",s,strerror(e));
}


void* statistic_n_clients(void* arguments);
circ_buffer* circ_buff_init(circ_buffer* cb, int size);
int circ_buf_push(circ_buffer* circ_buf,file_desc* new_fd);
file_desc* circ_buf_pop(circ_buffer* circ_buf);
countryList* serverListInsert(countryList* node,char* name);
int fd_socket_connect(int port,char* IP);
int searchCountryList(workerInfo** worker_info,int Workers,char* country);

int Workers;
int countWorkers;
int readWorkers;
int availableThreads;
int idx;
int bufferlen;

pthread_mutex_t wait_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t print_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t work_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t cb_mutex = PTHREAD_MUTEX_INITIALIZER;
//pthread_mutex_t waitServ_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cvar;
pthread_cond_t cb_cvar;
//pthread_cond_t cvarServ;

workerInfo** worker_info;
char* workerIP;

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
    pthread_cond_init (&cb_cvar , NULL ) ; /* Initialize condition variable */


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

    listen(sock,5);


    Workers=-1;
    countWorkers=-1;
    readWorkers=0;

;
    int gotIP=0;
    struct sockaddr_in acceptStrct;
    socklen_t addr_size = sizeof(acceptStrct);

    /*Accept*/

    printf("\n\nWaiting for the master to start...\n\n");

    while(1){
        if(countWorkers==0){
            sleep(1);
            printf("All the statistics have been received.\n");
            break;
        }
        mysock=accept(sock,(struct sockaddr*)&acceptStrct,&addr_size);
        if(!gotIP){
            workerIP=malloc(strlen(inet_ntoa(acceptStrct.sin_addr))+1);
            strcpy(workerIP,inet_ntoa(acceptStrct.sin_addr));
            gotIP=1;
            printf("Workers IP received: %s\n",workerIP);
        }
        if(mysock==-1){
            perror("\nACCEPT FAILED\n");
        }else{
            file_desc* fd=malloc(sizeof(file_desc));
            fd->fd=mysock;
            fd->type=0;
            while(availableThreads==0);
            if(circ_buf->count==circ_buf->size){
                if ( err = pthread_mutex_lock (&cb_mutex)){ /* Lock mutex */
                    perror2(" pthread_mutex_lock " , err ); 
                    exit(1); 
                }
                pthread_cond_wait (&cb_cvar,&cb_mutex) ;
                if ( err = pthread_mutex_unlock (&cb_mutex)){ /* Lock mutex */
                    perror2(" pthread_mutex_unlock " , err ); 
                    exit(1); 
                }
            }
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

    printf("\n\nWHOSERVER IS READY\n\n");

    //listen to clients

    int clientsock;
    struct sockaddr_in clientserver;
    int myclientsock;

    /*Creating a socket for the workers*/

    clientsock=socket(AF_INET,SOCK_STREAM,0);
    if(clientsock<0){
        perror("Failed to create a socket");
        exit(1);
    }

    clientserver.sin_family=AF_INET;
    clientserver.sin_addr.s_addr=INADDR_ANY;
    clientserver.sin_port=htons(queryPortNum);

    /*Bind & Listen*/

    if(bind(clientsock,(struct sockaddr*) &clientserver,sizeof(clientserver))){
        perror("Bind failed");
        exit(1);
    }

    listen(clientsock,5);

    while(1){
        myclientsock=accept(sock,(struct sockaddr*)&acceptStrct,&addr_size);
        if(myclientsock==-1){
            perror("\nACCEPT FAILED\n");
        }else{
            file_desc* fd=malloc(sizeof(file_desc));
            fd->fd=myclientsock;
            fd->type=1;
            while(availableThreads==0);
            
            if(circ_buf_push(circ_buf,fd)<0){
                perror("Buffer is full\n");
                //free(fd);
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
    free(worker_info);
    
}


void* statistic_n_clients(void* argum){

    arguments* args=(arguments*)argum;
    while(1){
        int err;


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
        if ( err = pthread_mutex_lock (&cb_mutex)){ /* Lock mutex */
                    perror2(" pthread_mutex_lock " , err ); 
                    exit(1); 
        }
        usleep(100);
        mysock=circ_buf_pop(circ_buf);
        pthread_cond_signal (&cb_cvar) ;
        if ( err = pthread_mutex_unlock (&cb_mutex)){ /* Lock mutex */
            perror2(" pthread_mutex_unlock " , err ); 
            exit(1); 
        }

        if(mysock==NULL){
            printf("Buffer is empty\n");
        }
        if ( err = pthread_mutex_unlock (&wait_mutex)){ /* Lock mutex */
            perror2(" pthread_mutex_unlock " , err ); 
            exit(1); 
        }


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
                    idx=0;
                    worker_info=malloc(Workers*sizeof(workerInfo*));
                    for(int w=0;w<Workers;w++){
                        worker_info[w]=malloc(sizeof(workerInfo));
                        worker_info[w]->countries=NULL;
                    }
                    readWorkers++;
                }
                index=idx;
                idx++;
                worker_info[index]->port_num=port_num;
                
                if ( err = pthread_mutex_unlock (&work_mutex)){ /* Unlock mutex */
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
                        pthread_mutex_lock (&work_mutex);
                        worker_info[index]->countries=serverListInsert(worker_info[index]->countries,buf);
                        pthread_mutex_unlock (&work_mutex);
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
        }else{
            char* token;
            char* buffer=malloc(bufferlen);
            char* query=malloc(CLIENT_READBUFFER_SIZE);
            read(mysock->fd,query,CLIENT_READBUFFER_SIZE);
            write(mysock->fd,&bufferlen,sizeof(int));
            char* temp=malloc(CLIENT_READBUFFER_SIZE);
            strcpy(temp,query);
            token=strtok(temp,"\n");
            token=strtok(temp," ");
            memset(buffer,0,bufferlen);
            strcpy(buffer,token);
            if(strcmp(buffer,"/listCountries")==0){
                //Total++;
                for(int w=0;w<Workers;w++){
                    int fd=fd_socket_connect(worker_info[w]->port_num,workerIP);
                    write(fd,buffer,bufferlen);
                    close(fd);
                }
                ////Successful++;
            }else if(strcmp(buffer,"/searchPatientRecord")==0){
                //Total++;
                token=strtok(NULL," ");
                if(token==NULL){
                    //Failed++;
                    printf("Invalid input! Please provide the correct parameters\n");
                    continue;
                }
                int *fd_array=malloc(Workers*sizeof(int));
                for(int w=0;w<Workers;w++){
                    fd_array[w]=fd_socket_connect(worker_info[w]->port_num,workerIP);
                    write(fd_array[w],buffer,bufferlen);
                }
                memset(buffer,0,bufferlen);
                strcpy(buffer,token);
                for(int w=0;w<Workers;w++){
                    write(fd_array[w],buffer,bufferlen);
                }
                memset(buffer,0,bufferlen);
                int found=0;
                //printf("query: %s\n",query);
                char* clientbuf=malloc(bufferlen);
                memset(clientbuf,0,bufferlen);
                for(int w=0;w<Workers;w++){
                    int pid;
                    read(fd_array[w],&pid,sizeof(int));
                    read(fd_array[w],buffer,bufferlen);
                    if(strcmp(buffer,"404")==0){
                        sprintf(clientbuf,"\nRecord not found in worker with pid: %d\n",pid);
                        write(mysock->fd,clientbuf,bufferlen);
                    }else{
                        sprintf(clientbuf,"\nRecord found in worker with pid: %d\n",pid);
                        write(mysock->fd,clientbuf,bufferlen);
                        ////Successful++;
                        found++;
                        //printf("\n%s\n\n",buffer);
                        write(mysock->fd,buffer,bufferlen);
                    }
                    memset(clientbuf,0,bufferlen);
                }
                if(!found){
                    ////Failed++;
                    sprintf(clientbuf,"No patient with this ID was found\n");
                    write(mysock->fd,clientbuf,bufferlen);
                }
                write(mysock->fd,"end_of_message",bufferlen);
                for(int i=0;i<Workers;i++){
                    close(fd_array[i]);
                }
                free(clientbuf);
                free(fd_array);
            }else if(strcmp(buffer,"/diseaseFrequency")==0){
                ////Total++;
                token=strtok(NULL," ");
                if(token==NULL){
                    printf("Invalid input! Please provide the correct parameters\n");
                    ////Failed++;
                    continue;
                }
                char* disease = malloc(strlen(token)+1);
                strcpy(disease,token);
                token=strtok(NULL," ");
                if(token==NULL){
                    free(disease);
                    printf("Invalid input! Please provide the correct parameters\n");
                    ////Failed++;
                    continue;
                }
                char* date1=malloc(strlen(token)+1);
                strcpy(date1,token);
                token=strtok(NULL," ");
                if(token==NULL){
                    free(disease);
                    free(date1);
                    printf("Invalid input! Please provide the correct parameters\n");
                    ////Failed++;
                    continue;
                }
                char* date2=malloc(strlen(token)+1);
                strcpy(date2,token);
                token=strtok(NULL," ");
                char* country;
                if(token==NULL){
                    country=malloc(5);
                    strcpy(country,"NULL");
                    int* fd_array=malloc(Workers*sizeof(int));
                    for(int w=0;w<Workers;w++){
                        fd_array[w]=fd_socket_connect(worker_info[w]->port_num,workerIP);
                        write(fd_array[w],buffer,bufferlen);
                        memset(buffer,0,bufferlen);
                        strcpy(buffer,disease);
                        write(fd_array[w],buffer,bufferlen);
                        memset(buffer,0,bufferlen);
                        strcpy(buffer,date1);
                        write(fd_array[w],buffer,bufferlen);
                        memset(buffer,0,bufferlen);
                        strcpy(buffer,date2);
                        write(fd_array[w],buffer,bufferlen);
                        memset(buffer,0,bufferlen);
                        strcpy(buffer,country);
                        write(fd_array[w],buffer,bufferlen);
                        ////Successful++;
                    }
                    if ( err = pthread_mutex_lock (&print_mutex)){ /* Lock mutex */
                        perror2(" pthread_mutex_lock " , err ); 
                        exit(1); 
                    }
                    printf("query: %s\n",query);
                    for(int w=0;w<Workers;w++){
                        while(1){
                            read(fd_array[w],buffer,bufferlen);
                            if(strcmp(buffer,"end_of_message")==0){
                                break;
                            }
                            printf("%s\n",buffer);
                        }
                        close(fd_array[w]);
                    }
                    if ( err = pthread_mutex_unlock (&print_mutex)){ /* Unlock mutex */
                        perror2(" pthread_mutex_unlock " , err ); 
                        exit(1); 
                    }
                    free(fd_array);
                }else{
                    country=malloc(strlen(token)+1);
                    strcpy(country,token);
                    int workerPort=searchCountryList(worker_info,Workers,country);
                    if(workerPort==-1){
                        printf("This country does not exist.\n");
                        ////Failed++;
                    }else{
                        int fd=fd_socket_connect(workerPort,workerIP);
                        strcpy(buffer,"/diseaseFrequency");
                        write(fd,buffer,bufferlen);
                        memset(buffer,0,bufferlen);
                        strcpy(buffer,disease);
                        write(fd,buffer,bufferlen);
                        memset(buffer,0,bufferlen);
                        strcpy(buffer,date1);
                        write(fd,buffer,bufferlen);
                        memset(buffer,0,bufferlen);
                        strcpy(buffer,date2);
                        write(fd,buffer,bufferlen);
                        memset(buffer,0,bufferlen);
                        strcpy(buffer,country);
                        write(fd,buffer,bufferlen);
                        ////Successful++;
                        if ( err = pthread_mutex_lock (&print_mutex)){ /* Lock mutex */
                            perror2(" pthread_mutex_lock " , err ); 
                            exit(1); 
                        }
                        printf("query: %s\n",query);
                        while(1){
                            read(fd,buffer,bufferlen);
                            if(strcmp(buffer,"end_of_message")==0){
                                break;
                            }
                            printf("%s\n",buffer);
                        }
                        if ( err = pthread_mutex_unlock (&print_mutex)){ /* Lock mutex */
                            perror2(" pthread_mutex_unlock " , err ); 
                            exit(1); 
                        }
                        close(fd);
                    }
                }
                free(disease);
                free(date1);
                free(date2);
                free(country);

            }else if(strcmp(buffer,"/numPatientAdmissions")==0){
                //Total++;
                token=strtok(NULL," ");
                if(token==NULL){
                    printf("Invalid input! Please provide the correct parameters\n");
                    ////Failed++;
                    continue;
                }
                char* disease = malloc(strlen(token)+1);
                strcpy(disease,token);
                token=strtok(NULL," ");
                if(token==NULL){
                    free(disease);
                    printf("Invalid input! Please provide the correct parameters\n");
                    //Failed++;
                    continue;
                }
                char* date1=malloc(strlen(token)+1);
                strcpy(date1,token);
                token=strtok(NULL," ");
                if(token==NULL){
                    free(disease);
                    free(date1);
                    printf("Invalid input! Please provide the correct parameters\n");
                    //Failed++;
                    continue;
                }
                char* date2=malloc(strlen(token)+1);
                strcpy(date2,token);
                token=strtok(NULL," ");
                char* country;
                if(token==NULL){
                    country=malloc(5);
                    strcpy(country,"NULL");
                    int* fd_array=malloc(Workers*sizeof(int));
                    for(int w=0;w<Workers;w++){
                        fd_array[w]=fd_socket_connect(worker_info[w]->port_num,workerIP);
                        strcpy(buffer,"/numPatientAdmissions");
                        write(fd_array[w],buffer,bufferlen);
                        memset(buffer,0,bufferlen);
                        strcpy(buffer,disease);
                        write(fd_array[w],buffer,bufferlen);
                        memset(buffer,0,bufferlen);
                        strcpy(buffer,date1);
                        write(fd_array[w],buffer,bufferlen);
                        memset(buffer,0,bufferlen);
                        strcpy(buffer,date2);
                        write(fd_array[w],buffer,bufferlen);
                        memset(buffer,0,bufferlen);
                        strcpy(buffer,country);
                        write(fd_array[w],buffer,bufferlen);
                        //Successful++;
                    }
                    if ( err = pthread_mutex_lock (&print_mutex)){ /* Lock mutex */
                        perror2(" pthread_mutex_lock " , err ); 
                        exit(1); 
                    }
                    printf("query: %s\n",query);
                    for(int i=0;i<Workers;i++){
                        while(1){
                            read(fd_array[i],buffer,bufferlen);
                            if(strcmp(buffer,"end_of_message")==0){
                                break;
                            }
                            printf("%s\n",buffer);
                        }
                        close(fd_array[i]);
                    }
                    if ( err = pthread_mutex_unlock (&print_mutex)){ /* Unlock mutex */
                        perror2(" pthread_mutex_unlock " , err ); 
                        exit(1); 
                    }
                    free(fd_array);
                }else{
                    country=malloc(strlen(token)+1);
                    strcpy(country,token);
                    int workerPort=searchCountryList(worker_info,Workers,country);
                    if(workerPort==-1){
                        printf("This country does not exist.\n");
                        //Failed++;
                    }else{
                        int fd=fd_socket_connect(workerPort,workerIP);
                        write(fd,buffer,bufferlen);
                        memset(buffer,0,bufferlen);
                        strcpy(buffer,disease);
                        write(fd,buffer,bufferlen);
                        memset(buffer,0,bufferlen);
                        strcpy(buffer,date1);
                        write(fd,buffer,bufferlen);
                        memset(buffer,0,bufferlen);
                        strcpy(buffer,date2);
                        write(fd,buffer,bufferlen);
                        memset(buffer,0,bufferlen);
                        strcpy(buffer,country);
                        write(fd,buffer,bufferlen);
                        //Successful++;
                        if ( err = pthread_mutex_lock (&print_mutex)){ /* Lock mutex */
                            perror2(" pthread_mutex_lock " , err ); 
                            exit(1); 
                        }
                        printf("query: %s\n",query);
                        while(1){
                            read(fd,buffer,bufferlen);
                            if(strcmp(buffer,"end_of_message")==0){
                                break;
                            }
                            printf("%s\n",buffer);
                        }
                        if ( err = pthread_mutex_unlock (&print_mutex)){ /* Unlock mutex */
                            perror2(" pthread_mutex_unlock " , err ); 
                            exit(1); 
                        }
                        close(fd);
                    }
                }
                free(disease);
                free(date1);
                free(date2);
                free(country);

            }else if(strcmp(buffer,"/numPatientDischarges")==0){
                //Total++;
                token=strtok(NULL," ");
                if(token==NULL){
                    printf("Invalid input! Please provide the correct parameters\n");
                    //Failed++;
                    continue;
                }
                char* disease = malloc(strlen(token)+1);
                strcpy(disease,token);
                token=strtok(NULL," ");
                if(token==NULL){
                    free(disease);
                    printf("Invalid input! Please provide the correct parameters\n");
                    //Failed++;
                    continue;
                }
                char* date1=malloc(strlen(token)+1);
                strcpy(date1,token);
                token=strtok(NULL," ");
                if(token==NULL){
                    free(disease);
                    free(date1);
                    printf("Invalid input! Please provide the correct parameters\n");
                    //Failed++;
                    continue;
                }
                char* date2=malloc(strlen(token)+1);
                strcpy(date2,token);
                token=strtok(NULL," ");
                char* country;
                if(token==NULL){
                    country=malloc(5);
                    strcpy(country,"NULL");
                    int* fd_array=malloc(Workers*sizeof(int));
                    for(int w=0;w<Workers;w++){
                        fd_array[w]=fd_socket_connect(worker_info[w]->port_num,workerIP);
                        strcpy(buffer,"/numPatientDischarges");
                        write(fd_array[w],buffer,bufferlen);
                        memset(buffer,0,bufferlen);
                        strcpy(buffer,disease);
                        write(fd_array[w],buffer,bufferlen);
                        memset(buffer,0,bufferlen);
                        strcpy(buffer,date1);
                        write(fd_array[w],buffer,bufferlen);
                        memset(buffer,0,bufferlen);
                        strcpy(buffer,date2);
                        write(fd_array[w],buffer,bufferlen);
                        memset(buffer,0,bufferlen);
                        strcpy(buffer,country);
                        write(fd_array[w],buffer,bufferlen);
                        //Successful++;
                    }
                    if ( err = pthread_mutex_lock (&print_mutex)){ /* Lock mutex */
                        perror2(" pthread_mutex_lock " , err ); 
                        exit(1); 
                    }
                    printf("query: %s\n",query);
                    for(int w=0;w<Workers;w++){
                        while(1){
                            read(fd_array[w],buffer,bufferlen);
                            if(strcmp(buffer,"end_of_message")==0){
                                break;
                            }
                            printf("%s\n",buffer);
                        }
                        close(fd_array[w]);
                    }
                    if ( err = pthread_mutex_unlock (&print_mutex)){ /* Unlock mutex */
                        perror2(" pthread_mutex_unlock " , err ); 
                        exit(1); 
                    }
                    free(fd_array);
                }else{
                    country=malloc(strlen(token)+1);
                    strcpy(country,token);
                    int workerPort=searchCountryList(worker_info,Workers,country);
                    if(workerPort==-1){
                        printf("This country does not exist.\n");
                        //Failed++;
                    }else{
                        int fd=fd_socket_connect(workerPort,workerIP);
                        write(fd,buffer,bufferlen);
                        memset(buffer,0,bufferlen);
                        strcpy(buffer,disease);
                        write(fd,buffer,bufferlen);
                        memset(buffer,0,bufferlen);
                        strcpy(buffer,date1);
                        write(fd,buffer,bufferlen);
                        memset(buffer,0,bufferlen);
                        strcpy(buffer,date2);
                        write(fd,buffer,bufferlen);
                        memset(buffer,0,bufferlen);
                        strcpy(buffer,country);
                        write(fd,buffer,bufferlen);
                        //Successful++;
                        if ( err = pthread_mutex_lock (&print_mutex)){ /* Lock mutex */
                            perror2(" pthread_mutex_lock " , err ); 
                            exit(1); 
                        }
                        printf("query: %s\n",query);
                        while(1){
                            read(fd,buffer,bufferlen);
                            if(strcmp(buffer,"end_of_message")==0){
                                break;
                            }
                            printf("%s\n",buffer);
                        }
                        if ( err = pthread_mutex_unlock (&print_mutex)){ /* Unlock mutex */
                            perror2(" pthread_mutex_unlock " , err ); 
                            exit(1); 
                        }
                        close(fd);
                    }
                }
                free(disease);
                free(date1);
                free(date2);
                free(country);

            }else if(strcmp(buffer,"/topk-AgeRanges")==0){
                //Total++;
                token=strtok(NULL," ");
                if(token==NULL){
                    printf("Invalid input! Please provide the correct parameters\n");
                    //Failed++;
                    continue;
                }
                char* k = malloc(strlen(token)+1);
                strcpy(k,token);
                token=strtok(NULL," ");
                if(token==NULL){
                    printf("Invalid input! Please provide the correct parameters\n");
                    //Failed++;
                    continue;
                }
                char* country = malloc(strlen(token)+1);
                strcpy(country,token);
                token=strtok(NULL," ");
                if(token==NULL){
                    printf("Invalid input! Please provide the correct parameters\n");
                    //Failed++;
                    continue;
                }
                char* disease = malloc(strlen(token)+1);
                strcpy(disease,token);
                token=strtok(NULL," ");
                if(token==NULL){
                    free(disease);
                    printf("Invalid input! Please provide the correct parameters\n");
                    //Failed++;
                    continue;
                }
                char* date1=malloc(strlen(token)+1);
                strcpy(date1,token);
                token=strtok(NULL," ");
                if(token==NULL){
                    free(disease);
                    free(date1);
                    printf("Invalid input! Please provide the correct parameters\n");
                    //Failed++;
                    continue;
                }
                char* date2=malloc(strlen(token)+1);
                strcpy(date2,token);
                
                int workerPort=searchCountryList(worker_info,Workers,country);
                if(workerPort==-1){
                    printf("This country does not exist.\n");
                    //Failed++;
                }else{
                    int fd=fd_socket_connect(workerPort,workerIP);
                    write(fd,buffer,bufferlen);
                    memset(buffer,0,bufferlen);
                    strcpy(buffer,k);
                    write(fd,buffer,bufferlen);
                    memset(buffer,0,bufferlen);
                    strcpy(buffer,country);
                    write(fd,buffer,bufferlen);
                    memset(buffer,0,bufferlen);
                    strcpy(buffer,disease);
                    write(fd,buffer,bufferlen);
                    memset(buffer,0,bufferlen);
                    strcpy(buffer,date1);
                    write(fd,buffer,bufferlen);
                    memset(buffer,0,bufferlen);
                    strcpy(buffer,date2);
                    write(fd,buffer,bufferlen);
                    //Successful++;
                    if ( err = pthread_mutex_lock (&print_mutex)){ /* Lock mutex */
                        perror2(" pthread_mutex_lock " , err ); 
                        exit(1); 
                    }
                    printf("query: %s\n",query);
                    while(1){
                        read(fd,buffer,bufferlen);
                        if(strcmp(buffer,"end_of_message")==0){
                            break;
                        }
                        printf("%s\n",buffer);
                    }
                    if ( err = pthread_mutex_unlock (&print_mutex)){ /* Unlock mutex */
                        perror2(" pthread_mutex_unlock " , err ); 
                        exit(1); 
                    }
                    close(fd);
                }
                free(k);
                free(country);
                free(disease);
                free(date1);
                free(date2);
                

            }

//####################################

            free(query);
            free(temp);
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

int fd_socket_connect(int port,char* IP){
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
        server.sin_port=htons(port);

        struct hostent* foundhost ;
        struct in_addr myaddress ;

        inet_aton ( IP , &myaddress ) ;
        foundhost = gethostbyaddr (( const char *) & myaddress , sizeof ( myaddress ) , AF_INET ) ;

        if(foundhost==0){
            perror("Hosting failed");
            return -1;
        }
            
        memcpy(&server.sin_addr,foundhost->h_addr_list[0],foundhost->h_length);
        server.sin_port=htons(port);
        while(connect(sock,(struct sockaddr*)&server,sizeof(server))<0){
            perror("Connection failed");
            usleep(10000);
        }
        
        return sock;
}



int searchCountryList(workerInfo** worker_info,int Workers,char* country){

    for(int i=0;i<Workers;i++){
        countryList* temp=worker_info[i]->countries;
        while(temp!=NULL){
            if(strcmp(temp->countryName,country)==0){
                return worker_info[i]->port_num;
            }
            temp=temp->next;
        }
    }
    return -1;
}