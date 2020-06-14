#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <signal.h>
#include <fcntl.h>
#include <wait.h>
#include "RBT.h"
#include "fun.h"



workerFifos** fifoArrayGlob;
int numWorkersGlob;
CountryData** countryArrayGlob;
char* strbuffersizeGlob;
char* bufferGlob;
char* input_dirGlob;
int buffersizeGlob;
int* cnumArrGlob;
int exiting;
int signaled;
int Total;
int Successful;
int Failed;
char* lineGlob;
char* serverIPGlob;
char* serverPortGlob;
DIR* pDirGlob;

void kidkillerhandler(int num){
    signaled=1;
    pid_t p;
    int status;
    char* buf=malloc(buffersizeGlob);
    memset(buf,0,buffersizeGlob);

    p=waitpid(-1, &status, WNOHANG);
    int idx=searchPID(fifoArrayGlob,p,numWorkersGlob);
    if(idx!=-1){
        if(exiting==1){
            free(buf);
            return;
        }
        printf("You killed a kid...smh\n");
        char scnum[4];
        sprintf(scnum,"%d",cnumArrGlob[idx]);
        printf("Honey im pregnant\n");
        __pid_t pid=fork();
        if(pid==0){
            char* args[]={"./worker",fifoArrayGlob[idx]->readfifo,fifoArrayGlob[idx]->writefifo,strbuffersizeGlob,scnum,input_dirGlob,NULL};
            execvp(args[0],args);
        }

        if ( ( fifoArrayGlob[idx]->read_fd = open ( fifoArrayGlob[idx]->readfifo , O_WRONLY)) < 0){
            perror (" fife open error " ); exit (1) ; 
        }
        if ( ( fifoArrayGlob[idx]->write_fd = open ( fifoArrayGlob[idx]->writefifo , O_RDONLY)) < 0){
            perror (" fife open error " ); exit (1) ; 
        }
        //read(fifoArrayGlob[idx]->write_fd,bufferGlob,buffersizeGlob);
        //if(strcmp(bufferGlob,"ready")==0){
            //printf("Worker %d is ready\n",fifoArrayGlob[idx]->pid);
       // }
        
        fifoArrayGlob[idx]->pid=pid;
        CountryData* temp=countryArrayGlob[idx];
        memset(buf,0,buffersizeGlob);
        while(temp!=NULL){
            strcpy(buf,temp->name);
            write(fifoArrayGlob[idx]->read_fd,buf,buffersizeGlob);
            temp=temp->next;
        }
        strcpy(bufferGlob,serverIPGlob);
        write(fifoArrayGlob[idx]->read_fd,bufferGlob,buffersizeGlob);
        memset(bufferGlob,0,buffersizeGlob);
        strcpy(bufferGlob,serverPortGlob);
        write(fifoArrayGlob[idx]->read_fd,bufferGlob,buffersizeGlob);
        sleep(1);
        while(1){
            read(fifoArrayGlob[idx]->write_fd,buf,buffersizeGlob);
            if(strcmp(buf,"done")==0){
                printf("\n\n");
                break;
            }
            printf("\n\n\n%s\n",buf);
            memset(buf,0,buffersizeGlob);
            read(fifoArrayGlob[idx]->write_fd,buf,buffersizeGlob);
            printf("\n%s\n",buf);
            memset(buf,0,buffersizeGlob);
            read(fifoArrayGlob[idx]->write_fd,buf,buffersizeGlob);
            int* agenums=malloc(4*sizeof(int));
            memcpy(agenums,buf,4*sizeof(int));
            printf("\nAge range 0-20 years: %d\n",agenums[0]);
            printf("\nAge range 21-40 years: %d\n",agenums[1]);
            printf("\nAge range 41-60 years: %d\n",agenums[2]);
            printf("\nAge range 61+ years: %d\n",agenums[3]);
            free(agenums);
            memset(buf,0,buffersizeGlob);
        }
    }
    free(buf);
}

void sigusr1(int signo, siginfo_t *si, void *data) {
    (void)signo;
    (void)data;
    //printf("Signal %d from pid %lu\n", (int)si->si_signo,(unsigned long)si->si_pid);
    int pid=(int)si->si_pid;
    char* buffer=malloc(buffersizeGlob);
    int pid_idx=searchPID(fifoArrayGlob,pid,numWorkersGlob);
    if(pid_idx==-1){
        return;
    }
    while(1){
            read(fifoArrayGlob[pid_idx]->write_fd,buffer,buffersizeGlob);
            if(strcmp(buffer,"done")==0){
                printf("\n\n");
                break;
            }
            printf("\n\n\n%s\n",buffer);
            memset(buffer,0,buffersizeGlob);
            read(fifoArrayGlob[pid_idx]->write_fd,buffer,buffersizeGlob);
            printf("\n%s\n",buffer);
            memset(buffer,0,buffersizeGlob);
            read(fifoArrayGlob[pid_idx]->write_fd,buffer,buffersizeGlob);
            int* agenums=malloc(4*sizeof(int));
            memcpy(agenums,buffer,4*sizeof(int));
            printf("\nAge range 0-20 years: %d\n",agenums[0]);
            printf("\nAge range 21-40 years: %d\n",agenums[1]);
            printf("\nAge range 41-60 years: %d\n",agenums[2]);
            printf("\nAge range 61+ years: %d\n",agenums[3]);
            free(agenums);
            memset(buffer,0,buffersizeGlob);
    }
    free(buffer);
    signaled=1;
}

void handlerINT(int num){
    exiting=1;
    signaled=1;
    char* buffer=malloc(buffersizeGlob);
    memset(buffer,0,buffersizeGlob);
    for(int w=0;w<numWorkersGlob;w++){
        kill(fifoArrayGlob[w]->pid,SIGKILL);                                              
    }
    closedir(pDirGlob);
    free(input_dirGlob);
    int pid=getpid();
    char* spid=malloc(6);
    sprintf(spid,"%d",pid);
    char* log_file=malloc(sizeof("log_file")+6);
    strcpy(log_file,"log_file");
    strcat(log_file,spid);
    FILE* log=fopen(log_file,"w");
    for(int w=0;w<numWorkersGlob;w++){
        close(fifoArrayGlob[w]->read_fd);
        close(fifoArrayGlob[w]->write_fd);

        while(countryArrayGlob[w]!=NULL){
            CountryData* temp=countryArrayGlob[w];
            countryArrayGlob[w]=countryArrayGlob[w]->next;
            fprintf(log,"%s\n",temp->name);
            free(temp->name);
            free(temp);
        }
        remove(fifoArrayGlob[w]->readfifo);
        remove(fifoArrayGlob[w]->writefifo);
        free(fifoArrayGlob[w]->readfifo);
        free(fifoArrayGlob[w]->writefifo);
        free(fifoArrayGlob[w]);
    }
    fprintf(log,"TOTAL:%d\nSUCCESSFUL:%d\nFAILED:%d\n",Total,Successful,Failed);
    free(spid);
    free(log_file);
    free(lineGlob);
    free(buffer);
    free(cnumArrGlob);
    free(countryArrayGlob);
    free(fifoArrayGlob);
    free(bufferGlob);
    fclose(log);
    free(sentinel);
    exit(0);
        
}

int main(int argc,char** argv){
    signal(SIGCHLD,kidkillerhandler);
    signal(SIGQUIT,handlerINT);
    signal(SIGINT,handlerINT);
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = sigusr1;
    sigaction(SIGUSR1, &sa,NULL);

    exiting=0;
    signaled=0;
    Total=0;
    Successful=0;
    Failed=0;
    int i=1;
    char serverPort[6];
    char serverIP[20];

    i=1;
    while(argv[i]!=NULL && strcmp(argv[i],"-s")!=0){
        i++;
    }
    if(argv[i]!=NULL){
        if(argv[i+1]!=NULL){
            strcpy(serverIP,argv[i+1]);
        }else{
            printf("ERROR! No server IP was provided\n");
            exit(-1);
        }
    }else{
        printf("ERROR! No server IP was provided\n");
        exit(-1);
    }

    i=1;
    while(argv[i]!=NULL && strcmp(argv[i],"-p")!=0){
        i++;
    }
    if(argv[i]!=NULL){
        if(argv[i+1]!=NULL){
            strcpy(serverPort,argv[i+1]);
        }else{
            printf("ERROR! No server port was provided\n");
            exit(-1);
        }
    }else{
        printf("ERROR! No server port was provided\n");
        exit(-1);
    }
    i=1;
    int numWorkers = 1;  //default value set to 1
    while(argv[i]!=NULL && strcmp(argv[i],"-w")!=0){
        i++;
    }
    if(argv[i]!=NULL){
        if(argv[i+1]!=NULL){
            numWorkers=atoi(argv[i+1]);
            if(numWorkers==0){
                return -1;
            }
        }else{
            printf("Number of Workers was default set to 1.\n");
        }
    }else{
        printf("Number of Workers was default set to 1.\n");
    }
    i=1;
    int buffersize=150;
    while(argv[i]!=NULL && strcmp(argv[i],"-b")!=0){
        i++;
    }
    if(argv[i]!=NULL){
        if(argv[i+1]!=NULL){
            buffersize=atoi(argv[i+1]);
            if(buffersize<150){
                printf("Buffer size is too small and will be set to 150\n");
            }
        }else{
            printf("Size of buffer was default set to 150.\n");
        }
    }else{
        printf("Size of buffer was default set to 150.\n");
    }

    i=1;
    struct dirent *pDirent;
    DIR* pDir;
    while(argv[i]!=NULL && strcmp(argv[i],"-i")!=0){
        i++;
    }
    if(argv[i]==NULL){
        printf("ERROR! No input directory given.\nExiting...\n");
        return -1;
    }
    char* input_dir=NULL;
    if(argv[i+1]!=NULL){
        input_dir=malloc(strlen(argv[i+1])+1);
        strcpy(input_dir,argv[i+1]);
        pDir=opendir(input_dir);
        if (pDir == NULL) {
            printf ("Cannot open directory '%s'\n", argv[i+1]);
            return -1;
        }
    }else{
        printf("ERROR! No input directory given.\nExiting...\n");
        return -1;
    }

    int numCountries=0;
    while ((pDirent = readdir(pDir)) != NULL){
        if(strcmp(pDirent->d_name,".")!=0 && strcmp(pDirent->d_name,"..")!=0){
            numCountries++;
            //printf ("[%s]\n", pDirent->d_name);
        }
    }

    rewinddir(pDir);
    if(numWorkers>numCountries){
        numWorkers=numCountries;
        printf("Workers reduced to: %d\n\n",numWorkers);
    }
    CountryData** countryArray = malloc(numWorkers*sizeof(CountryData*));
    for(int w=0;w<numWorkers;w++){
        countryArray[w]=NULL;
    }
    int w=0;
    int* cnumArr=malloc(numWorkers*sizeof(int));
    for(int w=0;w<numWorkers;w++){
        cnumArr[w]=0;
    }
    for(int c=0; c<numCountries; c++){
        pDirent = readdir(pDir);
        while(strcmp(pDirent->d_name,".")==0 || strcmp(pDirent->d_name,"..")==0){
                pDirent = readdir(pDir);
                continue;
        }
        countryInsert(countryArray,w,pDirent->d_name);
        cnumArr[w]++;
        w++;
        if(w==numWorkers){
            w=0;
        }
    }
    for(int w=0;w<numWorkers;w++){
        CountryData* temp=countryArray[w];
        printf("Worker %d: ",w);
        while(temp!=NULL){
            printf("%s ",temp->name);
            temp=temp->next;
        }
        printf("\n");
    }
    
    workerFifos** fifoArray=malloc(numWorkers*sizeof(workerFifos*));
    char strbuffersize[5];
    sprintf(strbuffersize,"%d",buffersize);

    for(int w=0;w<numWorkers;w++){                          //for every worker create two pipes (read and write)
        char sw[5];
        sprintf(sw,"%d",w);
        fifoArray[w]=malloc(sizeof(workerFifos));
        fifoArray[w]->readfifo=malloc(11*sizeof(char));
        fifoArray[w]->writefifo=malloc(11*sizeof(char));
        strcpy(fifoArray[w]->readfifo,"rfifo");
        strcat(fifoArray[w]->readfifo,sw);
        strcpy(fifoArray[w]->writefifo,"wfifo");
        strcat(fifoArray[w]->writefifo,sw);
        mkfifo(fifoArray[w]->readfifo,0666);
        mkfifo(fifoArray[w]->writefifo,0666);
        char scnum[4];
        sprintf(scnum,"%d",cnumArr[w]);
        __pid_t pid=fork();
        if(pid==0){
            char* args[]={"./worker",fifoArray[w]->readfifo,fifoArray[w]->writefifo,strbuffersize,scnum,input_dir,NULL};
            execvp(args[0],args);
        }
        fifoArray[w]->pid=pid;
        if ( ( fifoArray[w]->read_fd = open ( fifoArray[w]->readfifo , O_WRONLY)) < 0){
            perror (" fife open error " ); exit (1) ; 
        }
        if ( ( fifoArray[w]->write_fd = open ( fifoArray[w]->writefifo , O_RDONLY)) < 0){
            perror (" fife open error " ); exit (1) ; 
        }
        char* buffer=malloc(buffersize);

        CountryData* temp=countryArray[w];
        while(temp!=NULL){
            memset(buffer,0,buffersize);
            strcpy(buffer,temp->name);
            write(fifoArray[w]->read_fd,buffer,buffersize);
            temp=temp->next;
        }
        strcpy(buffer,serverIP);
        write(fifoArray[w]->read_fd,buffer,buffersize);
        memset(buffer,0,buffersize);
        strcpy(buffer,serverPort);
        write(fifoArray[w]->read_fd,buffer,buffersize);
        free(buffer);
    }

    for(int w=0;w<numWorkers;w++){
        char* buffer=malloc(buffersize);
        memset(buffer,0,buffersize);
        while(1){
            read(fifoArray[w]->write_fd,buffer,buffersize);
            if(strcmp(buffer,"done")==0){
                printf("\n\n");
                break;
            }
            printf("\n\n\n%s\n",buffer);
            memset(buffer,0,buffersize);
            read(fifoArray[w]->write_fd,buffer,buffersize);
            printf("\n%s\n",buffer);
            memset(buffer,0,buffersize);
            read(fifoArray[w]->write_fd,buffer,buffersize);
            int* agenums=malloc(4*sizeof(int));
            memcpy(agenums,buffer,4*sizeof(int));
            printf("\nAge range 0-20 years: %d\n",agenums[0]);
            printf("\nAge range 21-40 years: %d\n",agenums[1]);
            printf("\nAge range 41-60 years: %d\n",agenums[2]);
            printf("\nAge range 61+ years: %d\n",agenums[3]);
            free(agenums);
            memset(buffer,0,buffersize);
        }
        free(buffer);
    }
    cnumArrGlob=cnumArr;
    buffersizeGlob=buffersize;
    fifoArrayGlob=fifoArray;
    numWorkersGlob=numWorkers;
    countryArrayGlob=countryArray;
    strbuffersizeGlob=strbuffersize;
    input_dirGlob=input_dir;
    serverIPGlob=serverIP;
    serverPortGlob=serverPort;

    /*###########PROMPT############*/
    char* line;
    char* token;
    unsigned int current_nodeid = 0;
    size_t linelen=150;
    line=malloc(sizeof(char)*linelen);
    lineGlob=line;
    pDirGlob=pDir;
    char* buffer=malloc(buffersize);
    bufferGlob=buffer;

    printf(">");  
    while(1){          
        if(signaled==1){
            clearerr(stdin);
            signaled=0;
        }             
        getline(&line,&linelen,stdin);
        

        token=strtok(line,"\n");
        token=strtok(line," ");
        memset(buffer,0,buffersize);
        strcpy(buffer,token);
        if(strcmp(buffer,"/listCountries")==0){
            Total++;
            for(int w=0;w<numWorkers;w++){
                write(fifoArray[w]->read_fd,buffer,buffersize);
            }
            Successful++;
        }else if(strcmp(buffer,"/searchPatientRecord")==0){
            Total++;
            token=strtok(NULL," ");
            if(token==NULL){
                Failed++;
                printf("Invalid input! Please provide the correct parameters\n");
                continue;
            }
            for(int w=0;w<numWorkers;w++){
                write(fifoArray[w]->read_fd,buffer,buffersize);
            }
            memset(buffer,0,buffersize);
            strcpy(buffer,token);
            for(int w=0;w<numWorkers;w++){
                write(fifoArray[w]->read_fd,buffer,buffersize);
            }
            memset(buffer,0,buffersize);
            int found=0;
            for(int w=0;w<numWorkers;w++){
                read(fifoArray[w]->write_fd,buffer,buffersize);
                if(strcmp(buffer,"404")==0){
                    printf("Record not found in worker with pid: %d\n",fifoArray[w]->pid);
                }else{
                    printf("Record found in worker with pid: %d\n",fifoArray[w]->pid);
                    Successful++;
                    found++;
                    printf("\n%s\n\n\n\n",buffer);
                }
            }
            if(!found){
                Failed++;
                printf("No patient with this ID was found\n");
            }
            
        }else if(strcmp(buffer,"/diseaseFrequency")==0){
            Total++;
            token=strtok(NULL," ");
            if(token==NULL){
                printf("Invalid input! Please provide the correct parameters\n");
                Failed++;
                continue;
            }
            char* disease = malloc(strlen(token)+1);
            strcpy(disease,token);
            token=strtok(NULL," ");
            if(token==NULL){
                free(disease);
                printf("Invalid input! Please provide the correct parameters\n");
                Failed++;
                continue;
            }
            char* date1=malloc(strlen(token)+1);
            strcpy(date1,token);
            token=strtok(NULL," ");
            if(token==NULL){
                free(disease);
                free(date1);
                printf("Invalid input! Please provide the correct parameters\n");
                Failed++;
                continue;
            }
            char* date2=malloc(strlen(token)+1);
            strcpy(date2,token);
            token=strtok(NULL," ");
            char* country;
            if(token==NULL){
                country=malloc(5);
                strcpy(country,"NULL");
                for(int w=0;w<numWorkers;w++){
                    write(fifoArray[w]->read_fd,buffer,buffersize);
                    memset(buffer,0,buffersize);
                    strcpy(buffer,disease);
                    write(fifoArray[w]->read_fd,buffer,buffersize);
                    memset(buffer,0,buffersize);
                    strcpy(buffer,date1);
                    write(fifoArray[w]->read_fd,buffer,buffersize);
                    memset(buffer,0,buffersize);
                    strcpy(buffer,date2);
                    write(fifoArray[w]->read_fd,buffer,buffersize);
                    memset(buffer,0,buffersize);
                    strcpy(buffer,country);
                    write(fifoArray[w]->read_fd,buffer,buffersize);
                    Successful++;
                }
            }else{
                country=malloc(strlen(token)+1);
                strcpy(country,token);
                int worker=searchCountryArray(countryArray,numWorkers,country);
                if(worker==-1){
                    printf("This country does not exist.\n");
                    Failed++;
                }else{
                    strcpy(buffer,"/diseaseFrequency");
                    write(fifoArray[worker]->read_fd,buffer,buffersize);
                    memset(buffer,0,buffersize);
                    strcpy(buffer,disease);
                    write(fifoArray[worker]->read_fd,buffer,buffersize);
                    memset(buffer,0,buffersize);
                    strcpy(buffer,date1);
                    write(fifoArray[worker]->read_fd,buffer,buffersize);
                    memset(buffer,0,buffersize);
                    strcpy(buffer,date2);
                    write(fifoArray[worker]->read_fd,buffer,buffersize);
                    memset(buffer,0,buffersize);
                    strcpy(buffer,country);
                    write(fifoArray[worker]->read_fd,buffer,buffersize);
                    Successful++;
                }
            }
            free(disease);
            free(date1);
            free(date2);
            free(country);

        }else if(strcmp(buffer,"/numPatientAdmissions")==0){
            Total++;
            token=strtok(NULL," ");
            if(token==NULL){
                printf("Invalid input! Please provide the correct parameters\n");
                Failed++;
                continue;
            }
            char* disease = malloc(strlen(token)+1);
            strcpy(disease,token);
            token=strtok(NULL," ");
            if(token==NULL){
                free(disease);
                printf("Invalid input! Please provide the correct parameters\n");
                Failed++;
                continue;
            }
            char* date1=malloc(strlen(token)+1);
            strcpy(date1,token);
            token=strtok(NULL," ");
            if(token==NULL){
                free(disease);
                free(date1);
                printf("Invalid input! Please provide the correct parameters\n");
                Failed++;
                continue;
            }
            char* date2=malloc(strlen(token)+1);
            strcpy(date2,token);
            token=strtok(NULL," ");
            char* country;
            if(token==NULL){
                country=malloc(5);
                strcpy(country,"NULL");
                for(int w=0;w<numWorkers;w++){
                    strcpy(buffer,"/numPatientAdmissions");
                    write(fifoArray[w]->read_fd,buffer,buffersize);
                    memset(buffer,0,buffersize);
                    strcpy(buffer,disease);
                    write(fifoArray[w]->read_fd,buffer,buffersize);
                    memset(buffer,0,buffersize);
                    strcpy(buffer,date1);
                    write(fifoArray[w]->read_fd,buffer,buffersize);
                    memset(buffer,0,buffersize);
                    strcpy(buffer,date2);
                    write(fifoArray[w]->read_fd,buffer,buffersize);
                    memset(buffer,0,buffersize);
                    strcpy(buffer,country);
                    write(fifoArray[w]->read_fd,buffer,buffersize);
                    Successful++;
                }
            }else{
                country=malloc(strlen(token)+1);
                strcpy(country,token);
                int worker=searchCountryArray(countryArray,numWorkers,country);
                if(worker==-1){
                    printf("This country does not exist.\n");
                    Failed++;
                }else{
                    write(fifoArray[worker]->read_fd,buffer,buffersize);
                    memset(buffer,0,buffersize);
                    strcpy(buffer,disease);
                    write(fifoArray[worker]->read_fd,buffer,buffersize);
                    memset(buffer,0,buffersize);
                    strcpy(buffer,date1);
                    write(fifoArray[worker]->read_fd,buffer,buffersize);
                    memset(buffer,0,buffersize);
                    strcpy(buffer,date2);
                    write(fifoArray[worker]->read_fd,buffer,buffersize);
                    memset(buffer,0,buffersize);
                    strcpy(buffer,country);
                    write(fifoArray[worker]->read_fd,buffer,buffersize);
                    Successful++;
                }
            }
            free(disease);
            free(date1);
            free(date2);
            free(country);

        }else if(strcmp(buffer,"/numPatientDischarges")==0){
            Total++;
            token=strtok(NULL," ");
            if(token==NULL){
                printf("Invalid input! Please provide the correct parameters\n");
                Failed++;
                continue;
            }
            char* disease = malloc(strlen(token)+1);
            strcpy(disease,token);
            token=strtok(NULL," ");
            if(token==NULL){
                free(disease);
                printf("Invalid input! Please provide the correct parameters\n");
                Failed++;
                continue;
            }
            char* date1=malloc(strlen(token)+1);
            strcpy(date1,token);
            token=strtok(NULL," ");
            if(token==NULL){
                free(disease);
                free(date1);
                printf("Invalid input! Please provide the correct parameters\n");
                Failed++;
                continue;
            }
            char* date2=malloc(strlen(token)+1);
            strcpy(date2,token);
            token=strtok(NULL," ");
            char* country;
            if(token==NULL){
                country=malloc(5);
                strcpy(country,"NULL");
                for(int w=0;w<numWorkers;w++){
                    strcpy(buffer,"/numPatientDischarges");
                    write(fifoArray[w]->read_fd,buffer,buffersize);
                    memset(buffer,0,buffersize);
                    strcpy(buffer,disease);
                    write(fifoArray[w]->read_fd,buffer,buffersize);
                    memset(buffer,0,buffersize);
                    strcpy(buffer,date1);
                    write(fifoArray[w]->read_fd,buffer,buffersize);
                    memset(buffer,0,buffersize);
                    strcpy(buffer,date2);
                    write(fifoArray[w]->read_fd,buffer,buffersize);
                    memset(buffer,0,buffersize);
                    strcpy(buffer,country);
                    write(fifoArray[w]->read_fd,buffer,buffersize);
                    Successful++;
                }
            }else{
                country=malloc(strlen(token)+1);
                strcpy(country,token);
                int worker=searchCountryArray(countryArray,numWorkers,country);
                if(worker==-1){
                    printf("This country does not exist.\n");
                    Failed++;
                }else{
                    write(fifoArray[worker]->read_fd,buffer,buffersize);
                    memset(buffer,0,buffersize);
                    strcpy(buffer,disease);
                    write(fifoArray[worker]->read_fd,buffer,buffersize);
                    memset(buffer,0,buffersize);
                    strcpy(buffer,date1);
                    write(fifoArray[worker]->read_fd,buffer,buffersize);
                    memset(buffer,0,buffersize);
                    strcpy(buffer,date2);
                    write(fifoArray[worker]->read_fd,buffer,buffersize);
                    memset(buffer,0,buffersize);
                    strcpy(buffer,country);
                    write(fifoArray[worker]->read_fd,buffer,buffersize);
                    Successful++;
                }
            }
            free(disease);
            free(date1);
            free(date2);
            free(country);

        }else if(strcmp(buffer,"/topk-AgeRanges")==0){
            Total++;
            token=strtok(NULL," ");
            if(token==NULL){
                printf("Invalid input! Please provide the correct parameters\n");
                Failed++;
                continue;
            }
            char* k = malloc(strlen(token)+1);
            strcpy(k,token);
            token=strtok(NULL," ");
            if(token==NULL){
                printf("Invalid input! Please provide the correct parameters\n");
                Failed++;
                continue;
            }
            char* country = malloc(strlen(token)+1);
            strcpy(country,token);
            token=strtok(NULL," ");
            if(token==NULL){
                printf("Invalid input! Please provide the correct parameters\n");
                Failed++;
                continue;
            }
            char* disease = malloc(strlen(token)+1);
            strcpy(disease,token);
            token=strtok(NULL," ");
            if(token==NULL){
                free(disease);
                printf("Invalid input! Please provide the correct parameters\n");
                Failed++;
                continue;
            }
            char* date1=malloc(strlen(token)+1);
            strcpy(date1,token);
            token=strtok(NULL," ");
            if(token==NULL){
                free(disease);
                free(date1);
                printf("Invalid input! Please provide the correct parameters\n");
                Failed++;
                continue;
            }
            char* date2=malloc(strlen(token)+1);
            strcpy(date2,token);
            
            int worker=searchCountryArray(countryArray,numWorkers,country);
            if(worker==-1){
                printf("This country does not exist.\n");
                Failed++;
            }else{
                write(fifoArray[worker]->read_fd,buffer,buffersize);
                memset(buffer,0,buffersize);
                strcpy(buffer,k);
                write(fifoArray[worker]->read_fd,buffer,buffersize);
                memset(buffer,0,buffersize);
                strcpy(buffer,country);
                write(fifoArray[worker]->read_fd,buffer,buffersize);
                memset(buffer,0,buffersize);
                strcpy(buffer,disease);
                write(fifoArray[worker]->read_fd,buffer,buffersize);
                memset(buffer,0,buffersize);
                strcpy(buffer,date1);
                write(fifoArray[worker]->read_fd,buffer,buffersize);
                memset(buffer,0,buffersize);
                strcpy(buffer,date2);
                write(fifoArray[worker]->read_fd,buffer,buffersize);
                Successful++;
            }
            free(k);
            free(country);
            free(disease);
            free(date1);
            free(date2);
            

        }else if(strcmp(buffer,"/exit")==0){
                exiting=1;
                for(int w=0;w<numWorkers;w++){
                    write(fifoArray[w]->read_fd,buffer,buffersize);    //comment these lines if you dont want the workers to free their memory before SIGKILL
                    memset(buffer,0,buffersize);                       //
                    read(fifoArray[w]->write_fd,buffer,5);             //
                    if(strcmp(buffer,"exit")==0){                      //
                        kill(fifoArray[w]->pid,SIGKILL);
                    }                                                  //
                    strcpy(buffer,"/exit");
                }
                closedir(pDir);
                free(input_dir);
                int pid=getpid();
                char* spid=malloc(6);
                sprintf(spid,"%d",pid);
                char* log_file=malloc(sizeof("log_file")+6);
                strcpy(log_file,"log_file");
                strcat(log_file,spid);
                FILE* log=fopen(log_file,"w");
                for(int w=0;w<numWorkers;w++){
                    close(fifoArray[w]->read_fd);
                    close(fifoArray[w]->write_fd);

                    while(countryArray[w]!=NULL){
                        CountryData* temp=countryArray[w];
                        countryArray[w]=countryArray[w]->next;
                        fprintf(log,"%s\n",temp->name);
                        free(temp->name);
                        free(temp);
                    }
                    remove(fifoArray[w]->readfifo);
                    remove(fifoArray[w]->writefifo);
                    free(fifoArray[w]->readfifo);
                    free(fifoArray[w]->writefifo);
                    free(fifoArray[w]);
                }
                fprintf(log,"TOTAL:%d\nSUCCESSFUL:%d\nFAILED:%d\n",Total,Successful,Failed);
                free(spid);
                free(log_file);
                free(line);
                free(buffer);
                free(cnumArr);
                free(countryArray);
                free(fifoArray);
                fclose(log);
                free(sentinel);
                exit(0); 
        }
        sleep(1);
        printf(">");  
    }

}


