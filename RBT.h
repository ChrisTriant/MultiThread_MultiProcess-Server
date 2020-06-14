#ifndef __RBT__
#define __RBT__
#include<stdbool.h>
#include"structs.h"

node_ptr initializeNode(pR_ptr patient);
rbt_ptr initializeRBT();
void createSentinel();
pR_ptr Patient_Init(char* id,char* Firstname,char* Lastname,char* diseaseID,char* country,char* entryDate,char* exitDate,int age);
void Print_Patient_Info(pR_ptr pR);
void Print_Node_Info(node_ptr n);
node_ptr RBT_InsertID(node_ptr root,node_ptr n);
node_ptr RBT_InsertDate(node_ptr root,node_ptr n);
node_ptr get_RBT_root(rbt_ptr rbt);
//void RBT_RotateLeft(node_ptr,node_ptr );
//void RBT_RotateRight(node_ptr,node_ptr );
node_ptr RBT_InsertFIX(node_ptr,node_ptr);
node_ptr RBT_DeleteNode(node_ptr,node_ptr);
void swapColors(node_ptr,node_ptr );
node_ptr searchID(node_ptr ,char*);
node_ptr searchDate(node_ptr ,char*);
void printRBT(node_ptr,int*);
void deleteRBT(rbt_ptr rbt,int option);
void countPeriod(node_ptr root,char* date1,char* date2,int* counter,int option,char* virus_country,int exit_option);
void countSick(node_ptr root,int* counter);
void count_Adm_Dis(node_ptr root,char* date1,char* date2,int* counter,char* virus,int option);
void countCountrySick(node_ptr root,int* agenumArr);
void countAges(node_ptr root,char* country,char* date1,char* date2,ageRanges* agenumArr);
#endif