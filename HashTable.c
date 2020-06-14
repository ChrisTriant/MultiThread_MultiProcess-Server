#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"structs.h"
#include "RBT.h"


HashTable* init_HashTable(int size){
    //create and initialize a hash table
    HashTable* HT = malloc(sizeof(HashTable));
    HT->size=size;
    HT->array=malloc(sizeof(Bucket*)*size);
    for(int i=0;i<size;i++){
        HT->array[i]=NULL;
    }
    return HT;
}

Bucket* init_Bucket(int bucketSize){
    //create and initialize a bucket
    Bucket* bucket=malloc(sizeof(Bucket));
    bucket->counter=0;
    bucket->next=NULL;
    bucket->data=malloc(bucketSize-sizeof(int)-sizeof(Bucket*));
    return bucket;
}


Entry* init_Entry(char* key,rbt_ptr rbt){
    //inititalize the entry with a key and a pointer to a rb tree
    Entry* entry=malloc(sizeof(Entry));
    entry->key=key;
    entry->RBT=rbt;
}

void insertEntryToBucket(Bucket* bucket,Entry* entry,int bucketSize){
    //mem copy the given entry inside the first bucket that has enough space
    if(bucket->counter==(bucketSize-sizeof(int)-sizeof(Bucket*))/sizeof(Entry)){
        if(bucket->next!=NULL){
            insertEntryToBucket(bucket->next,entry,bucketSize);
        }else{
            Bucket* newBucket=init_Bucket(bucketSize);
            bucket->next=newBucket;
            insertEntryToBucket(newBucket,entry,bucketSize);
        }
    }else{
        memcpy(&bucket->data[bucket->counter],entry,sizeof(Entry));
        bucket->counter++;
    }
}

int hash_fun(char* key,int size){
    int total=0;
    for(int i=0;i<strlen(key);i++){
        total+=key[i];
    }
    return total%size;
}


void HashTableInsert(HashTable* HT,Entry* entry,int bucketSize){
    //insert an entry into the hash table
    int hash_key=hash_fun(entry->key,HT->size);
    if(HT->array[hash_key]==NULL){
        Bucket* new_bucket=init_Bucket(bucketSize);
        HT->array[hash_key]=new_bucket;
        insertEntryToBucket(new_bucket,entry,bucketSize);
    }else{
        insertEntryToBucket(HT->array[hash_key],entry,bucketSize);
    }
}

void deleteHashTable(HashTable* HT){
    Entry* entry=malloc(sizeof(Entry));
    Bucket* bucket;
    for(int i=0;i<HT->size;i++){
        bucket=HT->array[i];
        while(bucket!=NULL){
            for(int j=0;j<bucket->counter;j++){
                memcpy(entry,&bucket->data[j],sizeof(Entry));
                int count=0;
                deleteRBT(entry->RBT,0);
            }
            Bucket* temp=bucket;
            bucket=bucket->next;
            free(temp->data);
            free(temp);
        }
    }
    free(entry);
    free(HT->array);
    free(HT);
}

void printHashTable(HashTable* HT){
    //print the content of a hash table

    Entry* entry=malloc(sizeof(Entry));
    Bucket* bucket;
    for(int i=0;i<HT->size;i++){
        bucket=HT->array[i];
        while(bucket!=NULL){
            for(int j=0;j<bucket->counter;j++){
                memcpy(entry,&bucket->data[j],sizeof(Entry));
                printf("Name: %s\n",entry->key);
            }
            bucket=bucket->next;
        }
        printf("\n");
    }
    free(entry);
}

char** getNamesHashTable(HashTable* HT,int* numNames){
    char** namesArr;
    int num=0;
    Entry* entry=malloc(sizeof(Entry));
    Bucket* bucket;
    for(int i=0;i<HT->size;i++){
        bucket=HT->array[i];
        while(bucket!=NULL){
            for(int j=0;j<bucket->counter;j++){
                num++;
            }
            bucket=bucket->next;
        }
    }
    namesArr=malloc(num*sizeof(char*));
    *numNames=num;
    num=0;;
    for(int i=0;i<HT->size;i++){
        bucket=HT->array[i];
        while(bucket!=NULL){
            for(int j=0;j<bucket->counter;j++){
                memcpy(entry,&bucket->data[j],sizeof(Entry));
                namesArr[num]=malloc(strlen(entry->key)+1);
                strcpy(namesArr[num],entry->key);
                num++;
            }
            bucket=bucket->next;
        }
    }
    free(entry);
    return namesArr;
}



int searchHashTable(HashTable* HT,char* key){
    /*return 0 if the key is no found in the hashtable
      or 1 if it is found*/     
    int hash_key=hash_fun(key,HT->size);
    if(HT->array[hash_key]!=NULL){
        Bucket* bucket=HT->array[hash_key];
        Entry entry;
        while(bucket!=NULL){
            for(int i=0;i<bucket->counter;i++){
                memcpy(&entry,&bucket->data[i],sizeof(Entry));
                if(strcmp(key,entry.key)==0){
                    return 1;
                }
            }
            bucket=bucket->next;
        }
    }
    return 0;
}


rbt_ptr GetRBT_HT(HashTable* HT,char* key){
    //retrieve a rb tree's pointer from the hash table                
    int hash_key=hash_fun(key,HT->size);
    if(HT->array[hash_key]!=NULL){
        Bucket* bucket=HT->array[hash_key];
        Entry entry;
        while(bucket!=NULL){
            for(int i=0;i<bucket->counter;i++){
                memcpy(&entry,&bucket->data[i],sizeof(Entry));
                if(strcmp(key,entry.key)==0){
                    return entry.RBT;
                }
            }
            bucket=bucket->next;
        }
    }
    return NULL;
}