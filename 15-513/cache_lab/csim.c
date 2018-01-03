 /***********************************
 *        Name: Hongyi Liang        *
 *        Andrew ID:hongyil         *
 *        Project:cache lab         *
 ************************************/

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include "cachelab.h"

//declartion of 64-bit address type
typedef unsigned long long int addr_type;

//the contents of cache lines(based on recitation ppt)
typedef struct{
    int valid;        //valid bit
    int LRU;          //Least Recently Used
    int tag;          //tag bits
}cache_line;

//set is 1D array of cache lines
typedef struct{  
    cache_line *cacheLines;  
}cache_set; 

//cache is 2D array of cache lines
typedef struct{  
    cache_set *cacheSets; 
}cache;

//simply count cache hits,misses,evictions
int hits,misses,evictions;  

//initialize a new cache for simulation
//allocate space for s:sets;E:lines per set;
//block offsets are not used;
cache init_cache(int s,int E,int b){

    int i,j;
    cache my_cache;  
    
    my_cache.cacheSets=(cache_set*)malloc(sizeof(cache_set)*s);
    
    for (i=0;i<s;i++){
        my_cache.cacheSets[i].cacheLines=(cache_line*)malloc(sizeof(cache_line)*E);        
        for (j=0;j<E;j++){
            my_cache.cacheSets[i].cacheLines[j].valid=0;
            my_cache.cacheSets[i].cacheLines[j].LRU=0;
            my_cache.cacheSets[i].cacheLines[j].tag=0;    
        }
    }
    return my_cache;
}

//to find the evition line,LRU replacement policy.
int find_LRU_line(cache_set my_cache_set,int E){

    int i;
    int max_LRU_index=0;
    int max_LRU=my_cache_set.cacheLines[0].LRU;

    //compare a[0] to other elements
    for(i=0;i<E;i++){
        if(my_cache_set.cacheLines[i].LRU>max_LRU){
            max_LRU_index=i;
            max_LRU=my_cache_set.cacheLines[i].LRU;
        }
    }
    return max_LRU_index;
}

//return the tag bits of input address
//pattern: ---t---s---b--- simply right shift (s+b) bits
addr_type find_addr_tag(addr_type address,int s,int b){
 
    return address>>(s+b);
}

//return the set bits
//right shift b bits and extract the last s bits
addr_type find_cache_set(addr_type address,int s,int b){

    int mask1=(1<<s)-1;
    return (address>>b)&(mask1);
}

//to simulatate the behavior of caches
void run_cache(cache my_cache,addr_type address,int s,int E,int b) {

	int i,j;
    bool missed=true;        //status:cache miss=true    
    bool evicted=true; 
    int evict_index;         //if eviction is needed,find the evicted line
    addr_type set;
    addr_type addr_tag;

    set=find_cache_set(address,s,b);
    addr_tag=find_addr_tag(address,s,b);
	
    //if cache hit
    for (i=0;i<E;i++){
        my_cache.cacheSets[set].cacheLines[i].LRU+=1;
        if(my_cache.cacheSets[set].cacheLines[i].valid==1){
            if (my_cache.cacheSets[set].cacheLines[i].tag==addr_tag){
                hits++;
                //if cache hit,missed=false
                missed=false;
                my_cache.cacheSets[set].cacheLines[i].LRU=0;
            }
        }
    }
    
    //if cache miss:possible cache evictions
    if (missed==true){
        misses+=1;
        for(j=0;j<E;j++){
            //valid=0:empty line
            if(my_cache.cacheSets[set].cacheLines[j].valid==0){
                my_cache.cacheSets[set].cacheLines[j].valid=1;
                my_cache.cacheSets[set].cacheLines[j].LRU=0;
                my_cache.cacheSets[set].cacheLines[j].tag=addr_tag;
                //eviction is not needed 
                evicted=false;
                break;  
            }
        }

        //if missed and valid=1:eviction is needed
        //LRU replacement policy to find victim line
        if (evicted==true){
            evictions+=1;
            evict_index=find_LRU_line(my_cache.cacheSets[set],E);
            my_cache.cacheSets[set].cacheLines[evict_index].LRU=0;
            my_cache.cacheSets[set].cacheLines[evict_index].tag=addr_tag;
        }
    }

    return;
}

//free malloc space in case segmentation fault
void free_cache(cache my_cache,int s){

    int i;

    //free the space for each lines and sets
    for (i=0;i<s;i++){
        if (my_cache.cacheSets[i].cacheLines!=NULL){
            free(my_cache.cacheSets[i].cacheLines);
        }
    }
    
    if (my_cache.cacheSets!=NULL){   
        free(my_cache.cacheSets);
    }

    return;
}

int main(int argc,char **argv){

    char c;              //for getopt()
    char identifier;     //for fscanf
    char *file;          
    int s,S,E,b;         //# of sets;lines;blocks
    int size;
    FILE *trace;         //a pointer to file 
    cache my_cache;    
    addr_type address;
	
    //parse command line arguments
    //s,E,b should be integers>0
    while((c=getopt(argc,argv,"s:E:b:t:"))!=-1){
        switch(c){
        case 's':
            s=atoi(optarg);
            if(s<=0){
                printf("Error:not correct s\n");
                exit(1);
            }
            break;
        case 'E':
            E=atoi(optarg);
            if(E<=0){
                printf("Error:not correct E\n");
                exit(1);
            }
            break;
        case 'b':
            b=atoi(optarg);
            if(b<=0){
                printf("Error:not correct b\n");
                exit(1);
            }
            break;
        case 't':
            file=optarg;
            break;
        default:
            printf("not correct parameters\n");
            exit(0);
        }
    }
	
    //S=2**s,number of sets
	S=1<<s;
    my_cache=init_cache(S,E,b);
  
    //reading trace file
    trace=fopen(file,"r");
    
    while(fscanf(trace," %c %llx,%d",&identifier,&address,&size)!= EOF){
        switch(identifier){
        //instruction: load or store data
        case 'L':
        case 'S':
            run_cache(my_cache,address,s,E,b);
            break;
        default:
            break;
        }
    }
    
    //results summary
    //close trace file and free the space
    printSummary(hits,misses,evictions);	   
    fclose(trace);
    free_cache(my_cache,S);
    return 0;
}
