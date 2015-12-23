#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<time.h>
#include "mergesort.h"
#include<pthread.h>
#include<errno.h>

// Generate a file for testing
// 1GB BLOCK
// Generate a test file
long __BLOCKSIZE = 300000000*4;
size_t __UINT_SIZE = sizeof(unsigned int);

//THREAD AND MUTEX
int num_threads = 4;
pthread_t threads[4];
pthread_mutex_t read_mutex;
pthread_mutex_t increment_mutex;
struct Args{
    unsigned int* buffer;
    char* filename;
    int size;
};
void gen(){
    srand(time(NULL));
    FILE *fout;
    fout = fopen("in.dat", "wb");
    if(!fout){
        printf("Error\n");
    }
    size_t int_count = __BLOCKSIZE/__UINT_SIZE;
    unsigned int *buffer = (unsigned int*) malloc(__BLOCKSIZE);
    int i;
    long start = 0;
    int leftover = 0;
    // Create 1gb file file
    long end = (long)7500042000*4;
    while(start < end){
        if(start+__BLOCKSIZE > end){
            leftover = 1;
            break;
        }
        for(i=0;i<int_count;i++){
            buffer[i] = (unsigned int)rand()%2000004500;
        }
        fwrite(buffer,sizeof(unsigned int),int_count,fout);
        start += __BLOCKSIZE;
    }
    if(leftover){
       int bytes_left = (unsigned int)end - start;
       printf("%d\n", (bytes_left));
       int int_left = bytes_left / __UINT_SIZE;
       for(i=0;i<int_left;++i){
            buffer[i] = rand()%25600;
       }
       fwrite(buffer,sizeof(unsigned int),int_left,fout);
    }
    free(buffer);
    fclose(fout);
}
// Print array of integers
void print(unsigned int *buf, int len){
    int i;
    for(i=0;i<len;++i){
        printf("%d\n", buf[i]);
    }
}
//Multhread function for spliting files into chunks
void* m_sort_to_file(void* args){
    struct Args* m_args = (struct Args*)args;
    FILE* fout = fopen(m_args->filename, "wb");
    mergeSort(m_args->buffer,m_args->size);
    fwrite(m_args->buffer, __UINT_SIZE, m_args->size, fout);
    free(m_args->buffer);
    fclose(fout);
    return NULL;
}
int m_file_to_chunks(char* filename){
    FILE* fin;
    fin = fopen(filename, "rb");
    if(fin == NULL){
        printf("Error\n");
        return 0;
    }
    int int_count = __BLOCKSIZE/__UINT_SIZE;
    int i = 1, chunks = 0;
    int j;
    int size;
    struct Args args[num_threads];
    do{
        for(j=0;j<num_threads;++j){
            unsigned int* buffer = (unsigned int*)malloc(__BLOCKSIZE);
            size = fread(buffer, __UINT_SIZE, int_count, fin);
            if(size == 0){
                break;
            }
            char filename[100];
            sprintf(filename,"sort%d.dat", i);
            args[j].filename = filename;
            args[j].size = size;
            args[j].buffer = buffer;
            chunks++;
            i++;
            pthread_create(&threads[j],0,m_sort_to_file, &args[j]);
        }
        for(j=0;j<num_threads;++j){
            pthread_join(threads[j], NULL);
        }
    }while(size != 0);
    fclose(fin);
    return chunks;
}
// Single threaded
int file_to_chunks(char* filename){
    FILE* fin;
    fin = fopen(filename, "rb");
    if(fin == NULL){
        printf("Error\n");
        return 0;
    }
    // Number of elements
    int int_count =__BLOCKSIZE/__UINT_SIZE;
    unsigned int *readBuffer = (unsigned int*)malloc(__BLOCKSIZE);
    int size, i = 1, chunks = 0;
    while((size = fread(readBuffer, sizeof(unsigned int), int_count ,fin)) != 0){
        mergeSort(readBuffer,size);
        char filename[100];
        sprintf(filename,"sort%d.dat",i);
        printf("writing %s", filename);
        FILE* fout = fopen(filename,"wb");
        fwrite(readBuffer, __UINT_SIZE, size, fout);
        fclose(fout);
        i++;
        chunks++;
    }
    free(readBuffer);
    fclose(fin);
    return chunks;
}
//Read and test the file if everything is sorted!
void readFromFile(char* filename,int blocksize){
    FILE* fin = fopen(filename, "rb");
    unsigned int* block = (unsigned int*)malloc(blocksize); 
    int size = fread(block, __UINT_SIZE, blocksize/__UINT_SIZE, fin);
    if(size == 0){
        printf("Empty file!");
        return;
    }
    test(block, size, fin);
    
    free(block);
    fclose(fin);
}
struct Merge_args{
    char** filenames;
    int length;
    int *id;
    int depth;
};
// Multithreaded merge function
void* m_merge_chunks(void* args){
    //Set up the arguments from the struct
    struct Merge_args *merge_args = (struct Merge_args*)args;
    char** filenames = merge_args->filenames;
    int length = merge_args->length;
    int* id = merge_args->id;
    int depth = merge_args->depth;
    // How many threads we are using, since this function is recursive
    // we should not use more than num_threads
    if(length == 1){
       return *filenames; 
    }
    int block_per_file = __BLOCKSIZE/2;
    int int_count = block_per_file/__UINT_SIZE;
    int leftLength = length/2;
    int rightLength = length - leftLength;
    char** leftChunks = filenames + 0;
    char** rightChunks = filenames + leftLength;
    //Each time going through recursively, we want to increase the depth
    // Depth one is the most top
    depth++;
    //Creating our arguments
    struct Merge_args new_args[2];
    new_args[0].filenames = leftChunks;
    new_args[0].id = id;
    new_args[0].length = leftLength;
    new_args[0].depth = depth;

    new_args[1].filenames = rightChunks;
    new_args[1].id = id;
    new_args[1].length = rightLength;
    new_args[1].depth = depth + 1;
    // The return value (char* ) from a function will be store in 
    // newFile(left) and newFile2(right)
    char* newFile;
    char* newFile2;
    if(depth == 1){
        int i;
        for(i=0;i<2;++i){
            pthread_create(&threads[i], 0, m_merge_chunks, &new_args[i]);
        }
        for(i=0;i<2;++i){
            if(i == 0){
                pthread_join(threads[i],(void*)&newFile);
            }
            else if(i == 1){
                pthread_join(threads[i],(void*)&newFile2);
            }
        }
    }else{
        newFile = (char*)m_merge_chunks(new_args);
        newFile2 = (char*)m_merge_chunks(new_args+1);
    }
    char* result = (char*)malloc(sizeof(char)*100);
    if(depth == 1)
        sprintf(result,"final.dat");
    else{
        //id is a shared variable
        pthread_mutex_lock(&increment_mutex);
        *id = *id + 1;
        pthread_mutex_unlock(&increment_mutex);
        sprintf(result,"sorted%d.dat",*id);
    }
    FILE* fout = fopen(result, "wb");
    FILE* chunk1 = fopen(newFile, "rb");
    FILE* chunk2 = fopen(newFile2, "rb");
    if(fout == NULL || chunk1 == NULL || chunk2 == NULL)
        printf("error opening file\n");

    unsigned int* buffer = (unsigned int*)malloc(__BLOCKSIZE);
    twoway_merge(buffer, buffer+int_count, int_count, int_count, buffer, chunk1, chunk2,fout);
    remove(newFile);
    remove(newFile2);
    if(leftLength > 1)
        free(newFile);
    if(rightLength > 1){
        free(newFile2);
    }
    free(buffer);
    fclose(chunk1);
    fclose(chunk2);
    fclose(fout);
    return result;
}
char* merge_chunks(char** filenames, int length, int *id, int depth){
    // Only 1 chunk
    if(length == 1){
       printf("filename %s\n", *filenames);
       return *filenames; 
    }
    int block_per_file = __BLOCKSIZE/2;
    int int_count = block_per_file/__UINT_SIZE;
    int leftLength = length/2;
    int rightLength = length - leftLength;
    char** leftChunks = filenames + 0;
    char** rightChunks = filenames + leftLength;
    depth++;
    char* newFile = merge_chunks(leftChunks, leftLength, id,depth);
    char* newFile2 = merge_chunks(rightChunks, rightLength, id, depth);
    char* result = (char*)malloc(sizeof(char)*100);
    if(depth == 1)
        sprintf(result,"final.dat");
    else{
        *id = *id + 1;
        sprintf(result,"sorted%d.dat",*id);
    }
    FILE* fout = fopen(result, "wb");
    FILE* chunk1 = fopen(newFile, "rb");
    FILE* chunk2 = fopen(newFile2, "rb");
    if(fout == NULL || chunk1 == NULL || chunk2 == NULL)
        printf("error opening file\n");

    unsigned int* buffer = (unsigned int*)malloc(__BLOCKSIZE);
    twoway_merge(buffer, buffer+int_count, int_count, int_count, buffer, chunk1, chunk2,fout);
    remove(newFile);
    remove(newFile2);

    if(leftLength > 1 && rightLength > 1){
        free(newFile);
        free(newFile2);
    }
    free(buffer);
    fclose(chunk1);
    fclose(chunk2);
    fclose(fout);
    return result;
}
int main(int argc, char** argv){
    if(argc == 1)
        readFromFile("final.dat", __BLOCKSIZE);
    if(argc != 2)
        return 0;
    int chunks = m_file_to_chunks(argv[1]);
    char** files = (char**)malloc(sizeof(char*)*chunks);
    int i, depth = 0;
    for(i=0;i<chunks;++i){
        char *filename = (char*)malloc(100);
        sprintf(filename, "sort%d.dat",i+1);
        files[i] = filename;
    }
    int id = 0;
    struct Merge_args args = {files, chunks, &id,0};
    m_merge_chunks(&args);

    for(i=0;i<chunks;++i){
        free(files[i]);
    }
    free(files);
    return 0;
}
