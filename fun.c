#include<stdio.h>
#include <stdlib.h>
#include<string.h>
#include"structs.h"
#include<unistd.h>
#include "RBT.h"
#include "HashTable.h"

void countryInsert(CountryData** cd,int w,char* name){
    char* new_name=malloc(strlen(name)+1);
    strcpy(new_name,name);
    CountryData* new_data=malloc(sizeof(CountryData));
    new_data->name=new_name;
    new_data->next=NULL;
    CountryData* temp=cd[w];
    if(cd[w]==NULL){
        cd[w]=new_data;
    }else{
        while(temp->next!=NULL){
            temp=temp->next;
        }
        temp->next=new_data;
    }
}

CountryData* countryListInsert(CountryData* countryNode,char* name){
    char* new_name=malloc(strlen(name)+1);
    strcpy(new_name,name);
    CountryData* new_data=malloc(sizeof(CountryData));
    new_data->name=new_name;
    new_data->next=NULL;
    CountryData* temp=countryNode;
    if(countryNode==NULL){
        countryNode=new_data;
    }else{
        while(temp->next!=NULL){
            temp=temp->next;
        }
        temp->next=new_data;
    }
    return countryNode;
}

int searchCountryArray(CountryData** cd,int size,char* name){
    CountryData* temp;
    for(int i=0;i<size;i++){
        temp=cd[i];
        while (temp!=NULL){
            if(strcmp(temp->name,name)==0){
                return i;
            }else{
                temp=temp->next;
            }
        }
        
    }
    return -1;
}

int searchPID(workerFifos** fifoArr,int pid,int arrsize){
    for(int i=0;i<arrsize;i++){
        if(fifoArr[i]->pid==pid){
            return i;
        }
    }
    return -1;
}

int compareDate(char* d1,char* d2){      
    //date comparison function
    /*returns 1 if date1 > date2,
      -1 if date1 < date2,
      and 0 if date1 == date2.
    */
    char* date1=malloc(strlen(d1)+1);
    strcpy(date1,d1);
    char* date2=malloc(strlen(d2)+1);
    strcpy(date2,d2);
    int day1;                                  
    int month1;                                
    int year1;
    int day2;
    int month2;
    int year2;
    char* token1;
    char* token2;
    token1=strtok(date1,"-");
    day1=atoi(token1);
    token1=strtok(NULL,"-");
    month1=atoi(token1);
    token1=strtok(NULL,"\0");
    year1=atoi(token1);

    token2=strtok(date2,"-");
    day2=atoi(token2);
    token2=strtok(NULL,"-");
    month2=atoi(token2);
    token2=strtok(NULL,"\0");
    year2=atoi(token2);

    if(year1 > year2){
        free(date1);
        free(date2);
        return 1;
    }else if(year1 < year2){
        free(date1);
        free(date2);
        return -1;
    }else{
        if(month1 > month2){
            free(date1);
            free(date2);
            return 1;
        }else if(month1 < month2){
            free(date1);
            free(date2);
            return -1;
        }else{
            if(day1 > day2){
                free(date1);
                free(date2);
                return 1;
            }else if(day1 < day2){
                free(date1);
                free(date2);
                return -1;
            }else{
                free(date1);
                free(date2);
                return 0;
            }
        }
    }
    free(date1);
    free(date2);
}




/*##############QRUICKSORT#################*/


void swap(char** date1,char** date2){
    char* temp;
    temp=*date1;
    *date1=*date2;
    *date2=temp;
}

int partition (char** strArr, int low, int high){ 
    char* pivot = strArr[high];    // pivot 
    int i = (low - 1);  // Index of smaller element 
  
    for (int j = low; j <= high- 1; j++) 
    { 
        // If current element is smaller than the pivot 
        if (compareDate(strArr[j],pivot)<0)
        { 
            i++;    // increment index of smaller element 
            swap(&strArr[i], &strArr[j]); 
        } 
    } 
    swap(&strArr[i + 1], &strArr[high]); 
    return (i + 1); 
}

void quickSort(char** strArr, int low, int high){
    if (low < high) 
    { 
        /* pi is partitioning index, arr[p] is now 
           at right place */
        int pi = partition(strArr, low, high); 
  
        // Separately sort elements before 
        // partition and after partition 
        quickSort(strArr, low, pi - 1); 
        quickSort(strArr, pi + 1, high); 
    } 
} 
/*#########################################*/

void workerStats(HashTable* HT,int write_fd,int buffersize){
    Bucket* bucket;
    Entry entry;
    int* agenumArr=malloc(4*sizeof(int));
    for(int s=0;s<4;s++){
        agenumArr[s]=0;
    }
    for(int i=0;i<HT->size;i++){
        bucket=bucket=HT->array[i];
            while(bucket!=NULL){
                for(int j=0;j<bucket->counter;j++){
                    memcpy(&entry,&bucket->data[j],sizeof(Entry));
                    countCountrySick(entry.RBT->Root,agenumArr);
                }
                bucket=bucket->next;
            }
    }
    char* buffer=malloc(buffersize);
    memset(buffer,0,buffersize);
    memcpy(buffer,agenumArr,4*sizeof(int));
    write(write_fd,buffer,buffersize);
    free(agenumArr);
    free(buffer);
}
int cmpfunc (const void* a, const void* b) {
   return ((ageRanges*)a)->count - ((ageRanges*)b)->count;
}

/*###############COMMANDS#################*/
void listCountries(HashTable* HT){
    Bucket* bucket;
    Entry entry;
    int pid=getpid();
    for(int i=0;i<HT->size;i++){
        bucket=bucket=HT->array[i];
            while(bucket!=NULL){
                for(int j=0;j<bucket->counter;j++){
                    memcpy(&entry,&bucket->data[j],sizeof(Entry));
                    printf("%s ",entry.key);
                    printf("%d\n",pid);
                }
                bucket=bucket->next;
            }
    }
}

void numPatient_Adm_Dis(HashTable* HT,char* disease,char* date1,char* date2,char* country,int option){   //numPatient Admissions(option=0) and Discharges(option=1) 
    if(strcmp(country,"NULL")!=0){
        int hash_key=hash_fun(country,HT->size);
        Bucket* bucket=HT->array[hash_key];
        Entry entry;
        while(bucket!=NULL){
            for(int i=0;i<bucket->counter;i++){
                memcpy(&entry,&bucket->data[i],sizeof(Entry));
                if(strcmp(country,entry.key)==0){
                    int counter=0;
                    count_Adm_Dis(entry.RBT->Root,date1,date2,&counter,disease,option);
                    printf("%s %d\n",entry.key,counter);
                    return;
                }
            }
            bucket=bucket->next;
        }
    }else{
        Bucket* bucket;
        Entry entry;
        for(int i=0;i<HT->size;i++){
        bucket=bucket=HT->array[i];
            while(bucket!=NULL){
                for(int j=0;j<bucket->counter;j++){
                    memcpy(&entry,&bucket->data[j],sizeof(Entry));
                    int counter=0;
                    count_Adm_Dis(entry.RBT->Root,date1,date2,&counter,disease,option);
                    printf("%s %d\n",entry.key,counter);
                }
                bucket=bucket->next;
            }
        }
    }
}

void diseaseFrequency(HashTable* HT,char* disease,char* date1,char* date2,char* country,int option){
    if(strcmp(country,"NULL")!=0){
        int hash_key=hash_fun(disease,HT->size);
        if(HT->array[hash_key]!=NULL){
            Bucket* bucket=HT->array[hash_key];
            Entry entry;
            while(bucket!=NULL){
                for(int i=0;i<bucket->counter;i++){
                    memcpy(&entry,&bucket->data[i],sizeof(Entry));
                    if(strcmp(disease,entry.key)==0){
                        int counter=0;
                        countPeriod(entry.RBT->Root,date1,date2,&counter,option,country,2);
                        printf("%s %d\n\n",disease,counter);
                        return;
                    }
                }
                bucket=bucket->next;
            }
        }else{
            printf("No cases for this country.\n");
        }
    }else{
        Bucket* bucket;
        Entry entry;
        int counter=0;
        for(int i=0;i<HT->size;i++){
            bucket=bucket=HT->array[i];
            while(bucket!=NULL){
                for(int j=0;j<bucket->counter;j++){
                    memcpy(&entry,&bucket->data[j],sizeof(Entry));
                    countPeriod(entry.RBT->Root,date1,date2,&counter,option,disease,2);
                }
                bucket=bucket->next;
            }
        }
        printf("%s %d\n\n",disease,counter);
    }
}

void topk_AgeRanges(HashTable* HT,char* k,char* country,char* disease,char* date1,char* date2){
    Bucket* bucket;
    Entry entry;
    ageRanges* agenumArr=malloc(4*sizeof(ageRanges));
    for(int s=0;s<4;s++){
        agenumArr[s].index=s;
        agenumArr[s].count=0;
    }
    int hash_key=hash_fun(country,HT->size);
    bucket=HT->array[hash_key];
    while(bucket!=NULL){
        for(int i=0;i<bucket->counter;i++){
            memcpy(&entry,&bucket->data[i],sizeof(Entry));
            if(strcmp(disease,entry.key)==0){
                break;
            }
        }
        if(strcmp(disease,entry.key)==0){
            break;
        }
            bucket=bucket->next;
    }
    countAges(entry.RBT->Root,country,date1,date2,agenumArr);
    qsort(agenumArr,4,sizeof(ageRanges),cmpfunc);
    int sum=0;
    for(int i=0;i<4;i++){
        sum+=agenumArr[i].count;
    }
    int topk=atoi(k);
    if(topk<0 || topk >4){
        topk=4;
    }
    int perc=0; 
    for(int i=atoi(k)-1;i>=0;i--){
        switch (agenumArr[i].index){
            case 0:
                if(sum){
                    perc=100*((float)agenumArr[i].count/(float)sum);
                }
                printf("\nAge range 0-20 years: %d (%d%%)\n",agenumArr[i].count,perc);
                break;
            case 1:
                if(sum){
                    perc=100*((float)agenumArr[i].count/(float)sum);
                }
                printf("\nAge range 21-40 years: %d (%d%%)\n",agenumArr[i].count,perc);
                break;
            case 2:
                if(sum){
                    perc=100*((float)agenumArr[i].count/(float)sum);
                }
                printf("\nAge range 41-60 years: %d (%d%%)\n",agenumArr[i].count,perc);
                break;
            case 3:
                if(sum){
                    perc=100*((float)agenumArr[i].count/(float)sum);
                }
                printf("\nAge range 61+ years: %d (%d%%)\n",agenumArr[i].count,perc);
                break;
            default:
                break;  
        }

    }
    printf("\n\n");
}






node_ptr InsertFileRecord(FILE* file,node_ptr rbt_root,HashTable* DiseaseHT,HashTable* CountryHT,HashTable* statsHT,int bucketSize,char* rec_country,char* date){

    char line[100];
    char* token;
    char* recordID;
    char* patientFirstName;
    char* patientLastName;
    char* diseaseID;
    char* country;
    char* entryDate;
    char* exitDate;
    char* status;
    int age;
                char* t=fgets(line,100,file);
                if(t==NULL){
                    return rbt_root;
                }
                token=strtok(line," ");
                if(token==NULL){
                    printf("Insuffiecient line info. Skipping...\n");
                    return rbt_root;
                }
			    recordID=malloc(strlen(token)+1);
                strcpy(recordID,token);
                
                token = strtok(NULL," ");
                if(token==NULL){
                    printf("Insuffiecient line info. Skipping...\n");
                    free(recordID);
                    return rbt_root;
                }
                status=malloc(strlen(token)+1);
                strcpy(status,token);

                token = strtok(NULL," ");
                if(token==NULL){
                    printf("Insuffiecient line info. Skipping...\n");
                    free(recordID);
                    free(status);
                    return rbt_root;
                }
                patientFirstName=malloc(strlen(token)+1);
                strcpy(patientFirstName,token);
                token = strtok(NULL, " ");
                if(token==NULL){
                    printf("Insuffiecient line info. Skipping...\n");
                    free(recordID);
                    free(status);
                    free(patientFirstName);
                    return rbt_root;
                }
                patientLastName=malloc(strlen(token)+1);
                strcpy(patientLastName,token);
                token = strtok(NULL, " ");
                if(token==NULL){
                    printf("Insuffiecient line info. Skipping...\n");
                    free(recordID);
                    free(status);
                    free(patientFirstName);
                    free(patientLastName);
                    return rbt_root;
                }
                diseaseID=malloc(strlen(token)+1);
                strcpy(diseaseID,token);

                token = strtok(NULL, " ");
                if(token==NULL){
                    printf("Insuffiecient line info. Skipping...\n");
                    free(recordID);
                    free(status);
                    free(patientFirstName);
                    free(patientLastName);
                    free(diseaseID);
                    return rbt_root;
                }
                age=atoi(token);
                token = strtok(NULL, " ");
                if(age<0||age>120){
                    printf("Invalid age. Skipping...\n");
                    free(recordID);
                    free(status);
                    free(patientFirstName);
                    free(patientLastName);
                    free(diseaseID);
                    return rbt_root;
                }

                country=malloc(strlen(rec_country)+1);
                strcpy(country,rec_country);
                

                
                if(strcmp(status,"ENTER")==0){
                    entryDate=malloc(strlen(date)+1);
                    strcpy(entryDate,date);
                    exitDate=NULL;
                    if(rbt_root!=NULL){
                    node_ptr temp_node=searchID(rbt_root,recordID);
                        if(temp_node!=sentinel){      //id already in rbt
                            if(temp_node->data->entryDate!=NULL){
                                printf("This record already exists! Skipping...\n");
                                free(recordID);
                                free(status);
                                free(patientFirstName);
                                free(patientLastName);
                                free(diseaseID);
                                free(entryDate);
                                free(country);
                                return rbt_root;
                            }
                        }
                    }
                }else if(strcmp(status,"EXIT")==0){
                    exitDate=malloc(strlen(date)+1);
                    strcpy(exitDate,date);
                    if(rbt_root!=NULL){
                    node_ptr temp_node=searchID(rbt_root,recordID);
                        if(temp_node!=sentinel){      //id already in rbt
                            if(temp_node->data->exitDate!=NULL){
                                printf("Record already exists.\n");
                                if(compareDate(exitDate,temp_node->data->exitDate)>0){
                                    free(temp_node->data->exitDate);
                                    temp_node->data->exitDate=exitDate;
                                    printf("Exit date updated.\n");
                                }
                                return rbt_root;
                            }else{
                                if(compareDate(exitDate,temp_node->data->entryDate)<0){
                                    printf("ERROR\nInvalid exit date! Skipping...\n");
                                    free(recordID);
                                    free(status);
                                    free(patientFirstName);
                                    free(patientLastName);
                                    free(diseaseID);
                                    free(exitDate);
                                    free(country);
                                    return rbt_root;
                                }else{
                                    temp_node->data->exitDate=exitDate; //update record
                                    free(recordID);
                                    free(status);
                                    free(patientFirstName);
                                    free(patientLastName);
                                    free(diseaseID);
                                    free(country);
                                    return rbt_root;
                                } 
                            }
                        }else{
                            printf("ERROR\nNo entry for this record. Skipping...\n");
                                    free(recordID);
                                    free(status);
                                    free(patientFirstName);
                                    free(patientLastName);
                                    free(diseaseID);
                                    free(exitDate);
                                    free(country);
                                    return rbt_root;
                        }
                    }
                }else{
                    printf("Invalid status! Skipping...\n");
                    free(recordID);
                    free(status);
                    free(patientFirstName);
                    free(patientLastName);
                    free(diseaseID);
                    free(country);
                    return rbt_root;
                }
                
                free(status);
                
                pR_ptr patient=Patient_Init(recordID,patientFirstName,patientLastName,diseaseID,country,entryDate,exitDate,age);
                node_ptr node=initializeNode(patient);
                rbt_root=RBT_InsertID(rbt_root,node);
                rbt_root=RBT_InsertFIX(rbt_root,node);

                if(searchHashTable(DiseaseHT,patient->diseaseID)==0){
                    rbt_ptr new_RBT = initializeRBT();
                    node_ptr disease_node=initializeNode(patient);
                    new_RBT->Root=RBT_InsertDate(new_RBT->Root,disease_node);
                    new_RBT->Root=RBT_InsertFIX(new_RBT->Root,disease_node);
                    Entry* diseaseEntry=init_Entry(patient->diseaseID,new_RBT);
                    HashTableInsert(DiseaseHT,diseaseEntry,bucketSize);
                    free(diseaseEntry);
                }else{
                    rbt_ptr diseaseRBT=GetRBT_HT(DiseaseHT,patient->diseaseID);
                    if(diseaseRBT!=NULL){
                        node_ptr disease_node=initializeNode(patient);
                        diseaseRBT->Root=RBT_InsertDate(diseaseRBT->Root,disease_node);
                        diseaseRBT->Root=RBT_InsertFIX(diseaseRBT->Root,disease_node);
                    }else{
                        printf("You can't see me\n");
                    }
                }
                if(searchHashTable(CountryHT,patient->country)==0){
                    rbt_ptr new_RBT = initializeRBT();
                    node_ptr country_node=initializeNode(patient);
                    new_RBT->Root=RBT_InsertDate(new_RBT->Root,country_node);
                    new_RBT->Root=RBT_InsertFIX(new_RBT->Root,country_node);
                    Entry* countryEntry=init_Entry(patient->country,new_RBT);
                    HashTableInsert(CountryHT,countryEntry,bucketSize);
                    free(countryEntry);
                }else{
                    rbt_ptr countryRBT=GetRBT_HT(CountryHT,patient->country);
                    if(countryRBT!=NULL){
                        node_ptr country_node=initializeNode(patient);
                        countryRBT->Root=RBT_InsertDate(countryRBT->Root,country_node);
                        countryRBT->Root=RBT_InsertFIX(countryRBT->Root,country_node);
                    }else{
                        printf("You can't see me\n");
                    }
                }
                if(searchHashTable(statsHT,patient->diseaseID)==0){
                    rbt_ptr new_RBT = initializeRBT();
                    node_ptr stats_node=initializeNode(patient);
                    new_RBT->Root=RBT_InsertDate(new_RBT->Root,stats_node);
                    new_RBT->Root=RBT_InsertFIX(new_RBT->Root,stats_node);
                    Entry* statsEntry=init_Entry(patient->country,new_RBT);
                    HashTableInsert(statsHT,statsEntry,bucketSize);
                    free(statsEntry);
                }else{
                    rbt_ptr statsRBT=GetRBT_HT(statsHT,patient->diseaseID);
                    if(statsRBT!=NULL){
                        node_ptr stats_node=initializeNode(patient);
                        statsRBT->Root=RBT_InsertDate(statsRBT->Root,stats_node);
                        statsRBT->Root=RBT_InsertFIX(statsRBT->Root,stats_node);
                    }else{
                        printf("You can't see me\n");
                    }
                }
                //printf("A new node has been inserted into the RED-BLACK TREE\n");
                //Print_Node_Info(node);
                return rbt_root;
}


