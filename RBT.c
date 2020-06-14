#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<stdbool.h>
#include "structs.h"
#include "fun.h"




/*##################### PATIENT ######################*/



pR_ptr Patient_Init(char* id,char* Firstname,char* Lastname,char* diseaseID,char* country,char* entryDate,char* exitDate,int age){
    pR_ptr pr=malloc(sizeof(struct Record));
    pr->recordID=id;
    pr->patientFirstName=Firstname;
    pr->patientLastName=Lastname;
    pr->diseaseID=diseaseID;
    pr->country=country;
    pr->entryDate=entryDate;
    pr->exitDate=exitDate;
    pr->age=age;
    return pr;
}

void Print_Patient_Info(pR_ptr pr){
    if(pr==NULL){
        printf("NULL\n");
        return;
    }
    printf("Patient's info:\nID: %s, First name: %s, Last Name: %s, Disease: %s, Country: %s, Entry date: %s, Exit date: ",pr->recordID,\
    pr->patientFirstName,pr->patientLastName,pr->diseaseID,pr->country,pr->entryDate);
    if(pr->exitDate==NULL){
        printf("-\n");
    }else{
        printf("%s\n",pr->exitDate);
    }
}


/*##################### NODE ######################*/


node_ptr initializeNode(pR_ptr patient){
    node_ptr n=malloc(sizeof(struct Node));
    n->data=patient;
    n->left=sentinel;
    n->right=sentinel;
    n->parent=sentinel;
    n->color=RED;
    return n;
}

void Print_Node_Info(node_ptr n){
    if(n==sentinel){
        printf("null\n");
        return;
    }
    if(n->data!=NULL){
        Print_Patient_Info(n->data);
        //printf("Node Color: %s\n\n",n->color?"BLACK": "RED");
    }
} 

    
void deleteNode(node_ptr node,int option){
    //if option=1 delete patients
    if(option){
        free(node->data->recordID);
        free(node->data->patientFirstName);
        free(node->data->patientLastName);
        free(node->data->diseaseID);
        free(node->data->country);
        free(node->data->entryDate);
        if(node->data->exitDate!=NULL){ 
            free(node->data->exitDate);
        }
        free(node->data);
    }
    free(node);
}


/*##################### RBT ######################*/



node_ptr get_RBT_root(rbt_ptr RBT){
    return RBT->Root;
}

void createSentinel(){
    sentinel=malloc(sizeof(struct Node));
    sentinel->color=BLACK;
    sentinel->left=sentinel->right=sentinel->parent=sentinel;
}

rbt_ptr initializeRBT(){
    rbt_ptr p = malloc(sizeof(struct RBT));
    p->Root = sentinel;
    return p;
}


node_ptr RBT_InsertID(node_ptr root,node_ptr n){
    if(root==sentinel){
        return n;
    }
    if(strcmp(n->data->recordID,root->data->recordID)<=0){
        root->left=RBT_InsertID(root->left,n);
        root->left->parent=root;
    }else if(strcmp(n->data->recordID,root->data->recordID)){
        root->right=RBT_InsertID(root->right,n);
        root->right->parent=root;
    }
    return root;
}


node_ptr RBT_InsertDate(node_ptr root,node_ptr n){
    if(root==sentinel){
        return n;
    }
    if(compareDate(n->data->entryDate,root->data->entryDate)<=0){
        root->left=RBT_InsertDate(root->left,n);
        root->left->parent=root;
    }else if(compareDate(n->data->entryDate,root->data->entryDate)>0){
        root->right=RBT_InsertDate(root->right,n);
        root->right->parent=root;
    }
    return root;
}


void setColor(node_ptr n,bool color){
    n->color=color;
}


void printRBT(node_ptr node,int* count){
    if(node==sentinel){
        return;
    }
    (*count)++;
    printf("%d)",*count);
    Print_Node_Info(node);
    printRBT(node->left,count);
    printRBT(node->right,count);
}


node_ptr RBT_RotateLeft(node_ptr root,node_ptr n){
    node_ptr pt_right=n->right;
    n->right=pt_right->left;
    if(n->right!=sentinel){
        n->right->parent=n;
    }
    pt_right->parent = n->parent; 
  
    if (n->parent == sentinel){
        root = pt_right; 
    }
    else if (n == n->parent->left){ 
        n->parent->left = pt_right; 
    }
    else{
        n->parent->right = pt_right; 
    }
    pt_right->left = n; 
    n->parent = pt_right; 
    return root;
}

node_ptr RBT_RotateRight(node_ptr root,node_ptr n){
    node_ptr pt_left=n->left;
    n->left = pt_left->right; 
    
    if (n->left != sentinel){ 
        n->left->parent = n; 
    }
    pt_left->parent = n->parent; 
    
    if (n->parent == sentinel){ 
        root = pt_left; 
    }
    else if (n == n->parent->left){
        n->parent->left = pt_left; 
    }
    else{
        n->parent->right = pt_left; 
    }
    pt_left->right = n; 
    n->parent = pt_left; 
    return root;
}




void swapColors(node_ptr n1,node_ptr n2){
    bool temp=n1->color;
    n1->color=n2->color;
    n2->color=temp;
    
}


node_ptr RBT_InsertFIX(node_ptr root ,node_ptr n){
   
    node_ptr parent_pt = sentinel; 
    node_ptr grand_parent_pt = sentinel; 
    while ((n != root) && (n->color != BLACK) && (n->parent->color == RED)) 
    { 
  
        parent_pt = n->parent; 
        grand_parent_pt = n->parent->parent; 
  
        /*  Case : A 
            Parent of n is left child of Grand-parent of n */
        if (parent_pt == grand_parent_pt->left) 
        { 
            node_ptr uncle_pt = grand_parent_pt->right; 
  
            /* Case : 1 
               The uncle of n is also red 
               Only Recoloring required */
            if (uncle_pt != sentinel && uncle_pt->color == RED) 
            { 
                grand_parent_pt->color = RED; 
                parent_pt->color = BLACK; 
                uncle_pt->color = BLACK; 
                n = grand_parent_pt; 
            } 
  
            else
            { 
                /* Case : 2 
                   n is right child of its parent 
                   Left-rotation required */
                if (n == parent_pt->right) 
                { 
                    root=RBT_RotateLeft(root, parent_pt); 
                    n = parent_pt; 
                    parent_pt = n->parent; 
                } 
  
                /* Case : 3 
                   n is left child of its parent 
                   Right-rotation required */
                root=RBT_RotateRight(root, grand_parent_pt); 
                swapColors(parent_pt, grand_parent_pt); 
                n = parent_pt; 
            } 
        } 
  
        /* Case : B 
           Parent of n is right child of Grand-parent of n */
        else
        { 
            node_ptr uncle_pt = grand_parent_pt->left; 
  
            /*  Case : 1 
                The uncle of n is also red 
                Only Recoloring required */
            if ((uncle_pt != sentinel) && (uncle_pt->color == RED)) 
            { 
                grand_parent_pt->color = RED; 
                parent_pt->color = BLACK; 
                uncle_pt->color = BLACK; 
                n = grand_parent_pt; 
            } 
            else
            { 
                /* Case : 2 
                   n is left child of its parent 
                   Right-rotation required */
                if (n == parent_pt->left) 
                { 
                    root=RBT_RotateRight(root, parent_pt); 
                    n = parent_pt; 
                    parent_pt = n->parent; 
                } 
  
                /* Case : 3 
                   n is right child of its parent 
                   Left-rotation required */
                root=RBT_RotateLeft(root, grand_parent_pt); 
                swapColors(parent_pt, grand_parent_pt); 
                n = parent_pt; 
            } 
        } 
    } 
  
    root->color = BLACK; 
    return root;
}

node_ptr searchID(node_ptr root,char* key) 
{ 
    // Base Cases: root is null or key is present at root 
    if (root == sentinel || strcmp(root->data->recordID,key)==0){ 
       return root; 
    }else if(strcmp(root->data->recordID,key)<0){   // Key is greater than root's key 
       return searchID(root->right, key); 
    }else{                                    
        return searchID(root->left, key);     // Key is smaller than root's key 
    }
} 


node_ptr searchDate(node_ptr root,char*key){
    if (root == sentinel || compareDate(root->data->entryDate,key)==0){ 
       return root; 
    }else if(compareDate(key,root->data->entryDate)>0){   // Key is greater than root's key 
       return searchDate(root->right, key); 
    }else{                                    
        return searchDate(root->left, key);     // Key is smaller than root's key 
    }
}

void countPeriod(node_ptr root,char* date1,char* date2,int* counter,int option,char* virus_country,int exit_option){
    /*Counts all the patients that have been sick between date1 and date2(if they do not have an exit date they are not counted).
      if option is not zero,then the patient is also checked for details such as diseaseID(option 1) or country(option 2)
      if exit_option == 1 then the uncured patients are also counted*/
    if(root==sentinel){
        return;
    }
    if(compareDate(root->data->entryDate,date1)>=0){
        if(root->data->exitDate!=NULL){
            if(compareDate(root->data->exitDate,date2)<=0){
                if(option==0){
                    (*counter)++;
                }else if(option==1){
                    if(strcmp(root->data->diseaseID,virus_country)==0){
                        (*counter)++;
                    }
                }else if(option==2){
                    if(strcmp(root->data->country,virus_country)==0){
                        (*counter)++;
                    }
                }
            }
        }else{
            if(exit_option==1){
                if(option==0){
                    (*counter)++;
                }else if(option==1){
                    if(strcmp(root->data->diseaseID,virus_country)==0){
                        (*counter)++;
                    }
                }else if(option==2){
                    if(strcmp(root->data->country,virus_country)==0){
                        (*counter)++;
                    }
                }
            }
        }
        countPeriod(root->left,date1,date2,counter,option,virus_country,exit_option);
        countPeriod(root->right,date1,date2,counter,option,virus_country,exit_option);
    }else{
        countPeriod(root->right,date1,date2,counter,option,virus_country,exit_option);
    }
}

void count_Adm_Dis(node_ptr root,char* date1,char* date2,int* counter,char* virus,int option){       //count admissions(option=0) or discharges(option==1)

    if(root==sentinel){
        return;
    }
    if(option==0){
        if(compareDate(root->data->entryDate,date1)>=0 && compareDate(root->data->entryDate,date2)<=0){
            if(strcmp(root->data->diseaseID,virus)==0){
                (*counter)++;
            }
        }
    }else{
        if(root->data->exitDate!=NULL && compareDate(root->data->exitDate,date1)>=0 && compareDate(root->data->exitDate,date2)<=0){
            if(strcmp(root->data->diseaseID,virus)==0){
                (*counter)++;
            }
        }
    }
    count_Adm_Dis(root->left,date1,date2,counter,virus,option);
    count_Adm_Dis(root->right,date1,date2,counter,virus,option);
}



void countSick(node_ptr root,int* counter){
    if(root==sentinel){
        return;
    }
    if(root->data->exitDate==NULL){
        (*counter)++;
    }
    countSick(root->left,counter);
    countSick(root->right,counter);
}

void countCountrySick(node_ptr root,int* agenumArr){
    if(root==sentinel){
        return;
    }
    if(root->data->age<=20){
        agenumArr[0]++;
    }else if(root->data->age>=20 && root->data->age<=40){
        agenumArr[1]++;
    }else if(root->data->age>=41 && root->data->age<=60){
        agenumArr[2]++;
    }else{
        agenumArr[3]++;
    }
    countCountrySick(root->left,agenumArr);
    countCountrySick(root->right,agenumArr);
}

void countAges(node_ptr root,char* country,char* date1,char* date2,ageRanges* agenumArr){
    if(root==sentinel){
        return;
    }
    if(compareDate(root->data->entryDate,date1)>=0){
        if(compareDate(root->data->entryDate,date2)<=0){
            if(strcmp(root->data->country,country)==0){
                if(root->data->age<=20){
                    agenumArr[0].count++;
                }else if(root->data->age>=20 && root->data->age<=40){
                    agenumArr[1].count++;
                }else if(root->data->age>=41 && root->data->age<=60){
                    agenumArr[2].count++;
                }else{
                    agenumArr[3].count++;
                }
            }
        }
        countAges(root->left,country,date1,date2,agenumArr);
        countAges(root->right,country,date1,date2,agenumArr);
    }else{
        countAges(root->right,country,date1,date2,agenumArr);
    }
    
}


node_ptr NodeSuccessor(node_ptr node){

    if(node->right!=sentinel) {
        node_ptr temp=node->right;
        while(temp->left!=sentinel){
            temp=temp->left;
        }
        return temp;
    }
}

node_ptr RBT_DeleteFIX(node_ptr root,node_ptr n){
    node_ptr temp;
    while(n!=root && n->color==BLACK){

        if(n==n->parent->left){
            temp=n->parent->right;
            if(temp->color==RED){
                temp->color=BLACK;
                n->parent->color=RED;
                root=RBT_RotateLeft(root,n->parent);
                temp=n->parent->right;
            }
            if(temp->left->color==BLACK && temp->right->color==BLACK){
                temp->color=RED;
                n=n->parent;
            }else if(temp->right->color==BLACK){
                temp->left->color=BLACK;
                temp->color=RED;
                root=RBT_RotateRight(root,temp);
                temp=n->parent->right;

            }
            temp->color=n->parent->color;
            n->parent->color=BLACK;
            temp->right->color=BLACK;
            root=RBT_RotateLeft(root,n->parent);
            n=root;

        }else{
            temp=n->parent->left;
            if(temp->color==RED){
                temp->color=BLACK;
                n->parent->color=RED;
                root=RBT_RotateRight(root,n->parent);
                temp=n->parent->left;
            }
            if(temp->right->color==BLACK && temp->left->color==BLACK){
                temp->color=RED;
                n=n->parent;
            }else if(temp->left->color==BLACK){
                temp->right->color=BLACK;
                temp->color=RED;
                root=RBT_RotateLeft(root,temp);
                temp=n->parent->left;

            }
            temp->color=n->parent->color;
            n->parent->color=BLACK;
            temp->left->color=BLACK;
            root=RBT_RotateRight(root,n->parent);
            n=root;
        }
    }
    n->color=BLACK;
    return root;
}



node_ptr RBT_Transplant(node_ptr root,node_ptr u,node_ptr v){
    if(u->parent==sentinel){
        root=v;
    }else if(u==u->parent->left){
        u->parent->left=v;
    }else{
        u->parent->right=v;
    }
    v->parent=u->parent;
    return root;
}


node_ptr RBT_DeleteNode(node_ptr root,node_ptr n) {
    node_ptr temp1;
    node_ptr temp2;
    temp1=n;
    bool origcolor=temp1->color;
    if(n->left==sentinel){
        temp2=n->right;
        root=RBT_Transplant(root,n,n->right);
    }else if(n->right==sentinel){
        temp2=n->left;
        root=RBT_Transplant(root,n,n->left);
    }else{
        temp1=NodeSuccessor(n);
        origcolor=temp1->color;
        temp2=temp1->right;
        if(temp1->parent==n){
            temp2->parent==temp1;
        }else{
            root=RBT_Transplant(root,temp1,temp1->right);
            temp1->right=n->right;
            temp1->right->parent=temp1;
        }
        root=RBT_Transplant(root,n,temp1);
        temp1->left=n->left;
        temp1->left->parent=temp1;
        temp1->color=n->color;
    }
    if(origcolor==BLACK){
        root=RBT_DeleteFIX(root,temp2);
    }
    printf("Deleting patient:\n");
    // Print_Node_Info(n);
    deleteNode(n,1);
    return root;
}



void deleteRBTnodes(node_ptr node,int option){
    if(node==sentinel){
        return;
    }
    deleteRBTnodes(node->left,option);
    deleteRBTnodes(node->right,option);
    deleteNode(node,option);
}


void deleteRBT(rbt_ptr rbt,int option){
    deleteRBTnodes(rbt->Root,option);
    free(rbt);
}