#ifndef __STRCT__
#define __STRCT__

#include<stdbool.h>

typedef struct Record* pR_ptr;
typedef struct Node* node_ptr;
typedef struct RBT* rbt_ptr;


typedef struct Record {
    char* recordID;
    char* patientFirstName;
    char* patientLastName;
    char* diseaseID;
    char* country;
    char* entryDate;
    char* exitDate;
    int age;
}patientRecord;


enum Color {RED, BLACK};

struct Node{
    patientRecord* data;
    node_ptr left,right,parent;
    bool color;
};

struct RBT{
    node_ptr Root;
};

node_ptr sentinel;


typedef struct Entry{
    char* key;
    rbt_ptr RBT;  
}Entry;

typedef struct Bucket{
    Entry* data;
    int counter;
    struct Bucket* next;
}Bucket;


typedef struct HashTable{
    int size;
    Bucket** array;
}HashTable;


typedef struct heap_data{
    char* name;
    int counter;
}heap_data;


typedef struct heap_node{
    struct heap_node* parent;
    struct heap_node* left;
    struct heap_node* right;
    heap_data* data;
}heap_node;


typedef struct Heap{
    heap_node* heap_root;
}Heap;


typedef struct QueueNode { 
    heap_node* heap_node; 
    struct QueueNode* next;
    struct QueueNode* previous; 
}QueueNode; 
  

typedef struct Queue { 
    struct QueueNode *front, *rear; 
}Queue; 
  
 
typedef struct CountryData{
    char* name;
    char** datesArr;
    int numDates;
    struct CountryData* next;
}CountryData;


typedef struct workerFifos{
    char* readfifo;
    char* writefifo;
    int read_fd;
    int write_fd;
    int pid;
}workerFifos;


typedef struct ageRanges{
    int count;
    int index;
}ageRanges;


typedef struct file_desc{
    int fd;
    int type; //0 for stats, 1 for clients
}file_desc;


typedef struct circ_buffer{
    file_desc** fd_array;
    int buffer_end; 
    int size;  
    int count;           
    int head;       
    int tail;
}circ_buffer;

typedef struct arguments{
    circ_buffer* circ_buf;
    //int servWait;
}arguments;

typedef struct countryList{
    char* countryName;
    struct countryList* next;
}countryList;

typedef struct workerInfo{
    int port_num;
    countryList* countries;
}workerInfo;

#endif