void countryInsert(CountryData** cd,int w,char* name);
CountryData* countryListInsert(CountryData* countryNode,char* name);
int searchCountryArray(CountryData** cd,int size,char* name);
int searchPID(workerFifos** fifoArr,int pid,int arrsize);
int compareDate(char* d1,char* d2);
void quickSort(char** strArr, int low, int high);
node_ptr InsertFileRecord(FILE* file,node_ptr rbt_root,HashTable* DiseaseHT,HashTable* CountryHT,HashTable* statsHT,int bucketSize,char* rec_country,char*date);
void workerStats(HashTable* HT,int write_fd,int buffersize);
void listCountries(HashTable* HT);
void numPatient_Adm_Dis(HashTable* HT,char* disease,char* date1,char* date2,char* country,int option,int clientsock,int buffersize);
void topk_AgeRanges(HashTable* HT,char* k,char* country,char* disease,char* date1,char* date2,int clientsock,int buffersize);
void diseaseFrequency(HashTable* HT,char* disease,char* date1,char* date2,char* country,int option,int clientsock,int buffersize);
countryList* serverListInsert(countryList* node,char* name);

