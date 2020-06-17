#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include <dirent.h>
#include<unistd.h>
#include <sys/stat.h>
#include<signal.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include<wait.h>
#include "RBT.h"
#include "HashTable.h"
#include"fun.h"


char* buffer;
HashTable* diseaseGlob;
HashTable* countryGlob;
rbt_ptr rbtGlob;
int cnumGlob;
char** countryArrGlob;
CountryData* countryListGlob;
char* input_dirGlob;
int readfdGlob;
int writefdGlob;
int buffersizeGlob;
int bucketSizeGlob;
int Total;
int Successful;
int Failed;



void handlerINT(int num){
    sleep(1);
    int pid=getpid();
    char* spid=malloc(6);
    sprintf(spid,"%d",pid);
    char* log_file=malloc(sizeof("log_file")+6);
    strcpy(log_file,"log_file");
    strcat(log_file,spid);
    FILE* log=fopen(log_file,"w");    
    for(int c=0;c<cnumGlob;c++){
        fprintf(log,"%s\n",countryArrGlob[c]);
        free(countryArrGlob[c]);
        CountryData* tempList=countryListGlob;
        countryListGlob=countryListGlob->next;
        for(int d=0;d<tempList->numDates;d++){
            free(tempList->datesArr[d]);
        }
        free(tempList->datesArr);
        free(tempList);
    }
    fprintf(log,"TOTAL:%d\nSUCCESSFUL:%d\nFAILED:%d\n",Total,Successful,Failed);
    fclose(log);
    free(countryArrGlob);
    deleteHashTable(diseaseGlob);
    deleteHashTable(countryGlob);
    deleteRBT(rbtGlob,1);
    close(writefdGlob);
    close(readfdGlob);
    printf("No no plz don't noooo... X_X  %d\n",getpid());
    exit(0);
}

void handlerUPDATE(int num){
    CountryData* tempList=countryListGlob;
    int sendsig=0;
    for(int c=0;c<cnumGlob;c++){
        char* temp_dir=malloc(strlen(input_dirGlob)+strlen(countryArrGlob[c])+2);
            strcpy(temp_dir,input_dirGlob);
            strcat(temp_dir,"/");
            strcat(temp_dir,countryArrGlob[c]);
            struct dirent *pDirent;
            DIR* pDir;
            pDir=opendir(temp_dir);
            FILE* file;
            int numfiles=0;
            while ((pDirent = readdir(pDir)) != NULL){
                if(strcmp(pDirent->d_name,".")!=0 && strcmp(pDirent->d_name,"..")!=0){
                    numfiles++;
                }
            }
            if(numfiles>tempList->numDates){
                int f=tempList->numDates;
                int found=0;
                rewinddir(pDir);
                tempList->datesArr=realloc(tempList->datesArr,numfiles*sizeof(char*));

                while ((pDirent = readdir(pDir)) != NULL){
                    found=0;
                    if(strcmp(pDirent->d_name,".")==0 || strcmp(pDirent->d_name,"..")==0){
                        continue;
                    }
                    for(int d=0;d<tempList->numDates;d++){
                        if(strcmp(tempList->datesArr[d],pDirent->d_name)==0){
                            found=1;
                            break;
                        }
                    }
                    if(found==0){
                        printf("A new file was found: %s\n",pDirent->d_name);
                        sendsig=1;
                        tempList->datesArr[f]=malloc(strlen(pDirent->d_name)+1);
                        strcpy(tempList->datesArr[f],pDirent->d_name);
                        f++;

                    }
                }
                int oldnum=tempList->numDates;
                for(int nf=tempList->numDates;nf<numfiles;nf++){
                    printf("new file %s\n",tempList->datesArr[nf]);
                }
                tempList->numDates=numfiles;
                for(int j=oldnum;j<tempList->numDates;j++){
                    HashTable* statsHT=init_HashTable(10);
                    char* filepath=malloc(strlen(temp_dir)+strlen(tempList->datesArr[j])+2);
                    strcpy(filepath,temp_dir);
                    strcat(filepath,"/");
                    strcat(filepath,tempList->datesArr[j]);
                    if((file=fopen(filepath,"r"))!=NULL){
                        while (!feof(file)){
                            rbtGlob->Root=InsertFileRecord(file,rbtGlob->Root,diseaseGlob,countryGlob,statsHT,bucketSizeGlob,countryArrGlob[c],tempList->datesArr[j]);
                        }
                        fclose(file);
                    }else{
                        printf("Could not open file\n");
                    }
                    free(filepath);

                    memset(buffer,0,buffersizeGlob);
                    strcpy(buffer,tempList->datesArr[j]);
                    write(writefdGlob,buffer,buffersizeGlob);
                    memset(buffer,0,buffersizeGlob);
                    strcpy(buffer,countryArrGlob[c]);
                    write(writefdGlob,buffer,buffersizeGlob);
                    workerStats(statsHT,writefdGlob,buffersizeGlob);
                    deleteHashTable(statsHT); 
                }
                memset(buffer,0,buffersizeGlob);
                strcpy(buffer,"done");
                write(writefdGlob,buffer,buffersizeGlob);
                memset(buffer,0,buffersizeGlob);
            }else{
                printf("No new files in this directory: %s\n",tempList->name);
            }
            tempList=tempList->next;
    }
    if(sendsig==1){
        kill(getppid(),SIGUSR1);
    }
}



int main(int argc,char** argv){
    printf("A new worker was born\n");
    signal(SIGINT,handlerINT);
    signal(SIGQUIT,handlerINT);
    signal(SIGUSR1,handlerUPDATE);
    char* readfifo=argv[1];
    char* writefifo=argv[2];
    int buffersize=atoi(argv[3]);
    buffersizeGlob=buffersize;
    int cnum=atoi(argv[4]);
    char* input_dir=argv[5];
    input_dirGlob=input_dir;
    int read_fd;
    Total=0;
    Successful=0;
    Failed=0;
    char* serverIP;
    int serverPort;
    int sock;
    struct sockaddr_in server;

    sock=socket(AF_INET,SOCK_STREAM,0);

    if(sock<0){
        perror("Failed to create a socket");
         exit(1);
    }

    server.sin_family=AF_INET;
    server.sin_addr.s_addr=INADDR_ANY;
    server.sin_port=htons(serverPort);





    if ( ( read_fd = open ( readfifo , O_RDONLY)) < 0){
       perror (" fife open error " ); exit (1) ; 
    }
    int write_fd;
    if ( ( write_fd = open ( writefifo , O_WRONLY)) < 0){
        perror (" fife open error " ); exit (1) ; 
    }

    char** countryArr=malloc(cnum*sizeof(char*));
    int numWorkers;
    read(read_fd,&numWorkers,sizeof(int));
    CountryData* countryList=NULL;
    for(int i=0;i<cnum;i++){
        countryArr[i]=malloc(buffersize*sizeof(char));
        read(read_fd,countryArr[i],buffersize);
        countryList=countryListInsert(countryList,countryArr[i]);
    }



    buffer=malloc(buffersize);
    read(read_fd,buffer,buffersize);
    serverIP=malloc(strlen(buffer)+1);
    strcpy(serverIP,buffer);
    read(read_fd,buffer,buffersize);
    serverPort=atoi(buffer);
    memset(buffer,0,buffersize);

    struct hostent* foundhost ;
    struct in_addr myaddress ;

    inet_aton ( serverIP , &myaddress ) ;
    foundhost = gethostbyaddr (( const char *) & myaddress , sizeof ( myaddress ) , AF_INET ) ;

    if(foundhost==0){
        perror("Hosting failed");
        free(buffer);
         exit(1);
    }
    
    memcpy(&server.sin_addr,foundhost->h_addr_list[0],foundhost->h_length);
    server.sin_port=htons(serverPort);
    if(connect(sock,(struct sockaddr*)&server,sizeof(server))){
        perror("Connection failed");
        free(buffer);
        close(sock);
        exit(1);
    }

    int workersock;
    struct sockaddr_in workerserver;
    int clientsock;

    /*Creating a socket for the workers to listen to*/

    workersock=socket(AF_INET,SOCK_STREAM,0);
    if(workersock<0){
        perror("Failed to create a socket");
        exit(1);
    }

    workerserver.sin_family=AF_INET;
    workerserver.sin_addr.s_addr=INADDR_ANY;
    workerserver.sin_port=0;

    socklen_t len=sizeof(workerserver);

    /*Bind & Listen*/

    if(bind(workersock,(struct sockaddr*) &workerserver,sizeof(workerserver))){
        perror("Bind failed");
        exit(1);
    }

    listen(sock,5);

    if(getsockname(workersock,(struct sockaddr*)&workerserver,&len)==-1){
        perror("Bind failed");
        exit(1);
    }
    printf("\nI listen into port: %d\n",ntohs(workerserver.sin_port));

    int port_num=ntohs(workerserver.sin_port);

    
    int buff=buffersize;
    while(1){
        if(write(sock,&port_num,sizeof(int))>0){             //attempt the connect
            break;
        }
    }
    write(sock,&numWorkers,sizeof(int));
    write(sock,&buff,sizeof(int));

    int countryHashtableNumOfEntries=10;
    int diseaseHashtableNumOfEntries=10;
    int bucketSize=64;

    createSentinel();
    rbt_ptr ID_RBT = initializeRBT();
    HashTable* DiseaseHT=init_HashTable(diseaseHashtableNumOfEntries);
    HashTable* CountryHT=init_HashTable(countryHashtableNumOfEntries);
    CountryData* testList=countryList;
    for(int i=0; i<cnum;i++){

        char* temp_dir=malloc(strlen(input_dir)+strlen(countryArr[i])+2);
        strcpy(temp_dir,input_dir);
        strcat(temp_dir,"/");
        strcat(temp_dir,countryArr[i]);
        struct dirent *pDirent;
        DIR* pDir;
        pDir=opendir(temp_dir);
        FILE* file;

        int numDates=0;
        while ((pDirent = readdir(pDir)) != NULL){
            if(strcmp(pDirent->d_name,".")!=0 && strcmp(pDirent->d_name,"..")!=0){
                numDates++;
            }
        }
        rewinddir(pDir);

        char** filenameArr=malloc(numDates*sizeof(char*));
        for(int c=0; c<numDates; c++){
            pDirent = readdir(pDir);
            while(strcmp(pDirent->d_name,".")==0 || strcmp(pDirent->d_name,"..")==0){
                    pDirent = readdir(pDir);
                    continue;
            }
            filenameArr[c]=malloc(strlen(pDirent->d_name)+1);
            strcpy(filenameArr[c],pDirent->d_name);
        }
        quickSort(filenameArr,0,numDates-1);
        testList->datesArr=filenameArr;
        testList->numDates=numDates;
        printf("\n\n");

        for(int j=0;j<numDates;j++){

            HashTable* statsHT=init_HashTable(10);
            char* filepath=malloc(strlen(temp_dir)+strlen(pDirent->d_name)+2);
            strcpy(filepath,temp_dir);
            strcat(filepath,"/");
            strcat(filepath,filenameArr[j]);
            
            if((file=fopen(filepath,"r"))!=NULL){
                while (!feof(file)){
                    ID_RBT->Root=InsertFileRecord(file,ID_RBT->Root,DiseaseHT,CountryHT,statsHT,bucketSize,countryArr[i],filenameArr[j]);
                }
                fclose(file);
            }else{
                printf("Could not open file\n");
            }
            free(filepath);

            memset(buffer,0,buffersize);
            strcpy(buffer,filenameArr[j]);
            //write(write_fd,buffer,buffersize);
            write(sock,buffer,buffersize);
            memset(buffer,0,buffersize);
            strcpy(buffer,countryArr[i]);
            //write(write_fd,buffer,buffersize);
            write(sock,buffer,buffersize);
            //workerStats(statsHT,write_fd,buffersize);
            workerStats(statsHT,sock,buffersize);
            deleteHashTable(statsHT);  
        }
        memset(buffer,0,buffersize);
        strcpy(buffer,"new");
        write(sock,buffer,buffersize);
        testList=testList->next;
        free(temp_dir);
        closedir(pDir);
    }
    memset(buffer,0,buffersize);
    strcpy(buffer,"done");
    //write(write_fd,buffer,buffersize);
    write(sock,buffer,buffersize);
    close(sock);
    memset(buffer,0,buffersize);
    rbtGlob=ID_RBT;
    diseaseGlob=DiseaseHT;
    countryGlob=CountryHT;
    cnumGlob=cnum;
    countryArrGlob=countryArr;
    countryListGlob=countryList;
    readfdGlob=read_fd;
    writefdGlob=write_fd;
    bucketSizeGlob=bucketSize;

    memset(buffer,0,buffersize);
    while(1){
        read(read_fd,buffer,buffersize);
        if(strcmp(buffer,"/listCountries")==0){
            listCountries(CountryHT);
            Total++;
            Successful++;
        }else if(strcmp(buffer,"/searchPatientRecord")==0){
            Total++;
            memset(buffer,0,buffersize);
            read(read_fd,buffer,buffersize);           
            node_ptr rec=searchID(ID_RBT->Root,buffer);
            if(rec!=sentinel){
                memset(buffer,0,buffersize);
                strcpy(buffer,rec->data->recordID);
                strcat(buffer," ");
                strcat(buffer,rec->data->patientFirstName);
                strcat(buffer," ");
                strcat(buffer,rec->data->patientLastName);
                strcat(buffer," ");
                strcat(buffer,rec->data->diseaseID);
                strcat(buffer," ");
                strcat(buffer,rec->data->country);
                strcat(buffer," ");
                char s_age[4];
                sprintf(s_age,"%d",rec->data->age);
                strcat(buffer,s_age);
                strcat(buffer," ");
                strcat(buffer,rec->data->entryDate);
                strcat(buffer," ");
                if(rec->data->exitDate!=NULL){
                    strcat(buffer,rec->data->exitDate);
                }else{
                    strcat(buffer,"--");
                }
                Successful++;
            }else{
                memset(buffer,0,buffersize);
                strcpy(buffer,"404");
                Failed++;
            }
            write(write_fd,buffer,buffersize);

        }else if(strcmp(buffer,"/diseaseFrequency")==0){
            char *disease,*date1,*date2,*country;
            read(read_fd,buffer,buffersize);
            disease=malloc(strlen(buffer)+1);
            strcpy(disease,buffer);
            read(read_fd,buffer,buffersize);
            date1=malloc(strlen(buffer)+1);
            strcpy(date1,buffer);
            read(read_fd,buffer,buffersize);
            date2=malloc(strlen(buffer)+1);
            strcpy(date2,buffer);
            read(read_fd,buffer,buffersize);
            country=malloc(strlen(buffer)+1);
            strcpy(country,buffer);
            if(strcmp(country,"NULL")!=0){
                diseaseFrequency(DiseaseHT,disease,date1,date2,country,2);
            }else{
                diseaseFrequency(CountryHT,disease,date1,date2,country,1);
            }
            free(disease);
            free(date1);
            free(date2);
            free(country);

        }else if(strcmp(buffer,"/numPatientAdmissions")==0){
            char *disease,*date1,*date2,*country;
            read(read_fd,buffer,buffersize);
            disease=malloc(strlen(buffer)+1);
            strcpy(disease,buffer);
            read(read_fd,buffer,buffersize);
            date1=malloc(strlen(buffer)+1);
            strcpy(date1,buffer);
            read(read_fd,buffer,buffersize);
            date2=malloc(strlen(buffer)+1);
            strcpy(date2,buffer);
            read(read_fd,buffer,buffersize);
            country=malloc(strlen(buffer)+1);
            strcpy(country,buffer);
            numPatient_Adm_Dis(CountryHT,disease,date1,date2,country,0);
            free(disease);
            free(date1);
            free(date2);
            free(country);

        }else if(strcmp(buffer,"/numPatientDischarges")==0){
            char *disease,*date1,*date2,*country;
            read(read_fd,buffer,buffersize);
            disease=malloc(strlen(buffer)+1);
            strcpy(disease,buffer);
            read(read_fd,buffer,buffersize);
            date1=malloc(strlen(buffer)+1);
            strcpy(date1,buffer);
            read(read_fd,buffer,buffersize);
            date2=malloc(strlen(buffer)+1);
            strcpy(date2,buffer);
            read(read_fd,buffer,buffersize);
            country=malloc(strlen(buffer)+1);
            strcpy(country,buffer);
            numPatient_Adm_Dis(CountryHT,disease,date1,date2,country,1);
            free(disease);
            free(date1);
            free(date2);
            free(country);

        }else if(strcmp(buffer,"/topk-AgeRanges")==0){
            char *k,*disease,*date1,*date2,*country;
            read(read_fd,buffer,buffersize);
            k=malloc(strlen(buffer)+1);
            strcpy(k,buffer);
            read(read_fd,buffer,buffersize);
            country=malloc(strlen(buffer)+1);
            strcpy(country,buffer);
            read(read_fd,buffer,buffersize);
            disease=malloc(strlen(buffer)+1);
            strcpy(disease,buffer);
            read(read_fd,buffer,buffersize);
            date1=malloc(strlen(buffer)+1);
            strcpy(date1,buffer);
            read(read_fd,buffer,buffersize);
            date2=malloc(strlen(buffer)+1);
            strcpy(date2,buffer);
            topk_AgeRanges(DiseaseHT,k,country,disease,date1,date2);
            free(k);
            free(disease);
            free(date1);
            free(date2);
            free(country);

        }else if(strcmp(buffer,"/exit")==0){
            int pid=getpid();
            char* spid=malloc(6);
            sprintf(spid,"%d",pid);
            char* log_file=malloc(sizeof("log_file")+6);
            strcpy(log_file,"log_file");
            strcat(log_file,spid);
            FILE* log=fopen(log_file,"w");
            for(int c=0;c<cnum;c++){
                fprintf(log,"%s\n",countryArr[c]);
                free(countryArr[c]);
                CountryData* tempList=countryList;
                countryList=countryList->next;
                for(int d=0;d<tempList->numDates;d++){
                    free(tempList->datesArr[d]);
                }
                free(tempList->datesArr);
                free(tempList);
            }
            fprintf(log,"TOTAL:%d\nSUCCESSFUL:%d\nFAILED:%d\n",Total,Successful,Failed);
            free(countryArr);
            fclose(log);
            deleteHashTable(DiseaseHT);
            deleteHashTable(CountryHT);
            deleteRBT(ID_RBT,1);
            printf("exiting %d\n",getpid());
            char exit[]="exit";
            write(write_fd,exit,strlen(exit)+1);   
            free(buffer);
            while(1);       
        }
    }





}