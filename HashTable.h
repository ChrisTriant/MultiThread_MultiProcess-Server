#ifndef __HASHTABLE__
#define __HASHTABLE__
#include"structs.h"
HashTable* init_HashTable(int size);
Bucket* init_Bucket(int size);
Entry* init_Entry(char* key,rbt_ptr rbt);
int hash_fun(char* key,int size);
void HashTableInsert(HashTable* HT,Entry* entry,int bucketSize);
void printHashTable(HashTable* HT);
char** getNamesHashTable(HashTable* HT,int* num);
int searchHashTable(HashTable* HT,char* key);
rbt_ptr GetRBT_HT(HashTable* HT,char* key);
void deleteHashTable(HashTable* HT);
#endif