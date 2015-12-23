#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<math.h>
#include<time.h>

void copyArr(unsigned int *src, int size, unsigned int* des){
    int i;
    for(i=0;i<size;++i){
        des[i] = src[i];
    }
}

void merge(unsigned int *left,unsigned int* right, int leftLength, int rightLength, unsigned int* arr){
    int i = 0, j = 0, k = 0;

    unsigned int *leftCopy = (unsigned int*)malloc(leftLength*sizeof(unsigned int));
    copyArr(left,leftLength, leftCopy);
    while(i < leftLength && j < rightLength){
        /* printf("%d compare to %d\n", leftCopy[i], right[j]); */
        if(leftCopy[i] <= right[j]){
            arr[k++] = leftCopy[i++];
        }
        else{
            arr[k++] = right[j++];
        }
    }
    for(i;i<leftLength;++i){
        arr[k++] = leftCopy[i];
    }
    for(j;j<rightLength;++j){
        arr[k++] = right[j];
    }
    free(leftCopy);
}
void twoway_merge(unsigned int*left, unsigned int* right, int leftLength, int rightLength, unsigned int* arr,
        FILE* file1, FILE*file2, FILE* finalFile){
    int i = 0, j = 0, k = 0;
    int total = leftLength+rightLength;
    
    unsigned int *leftCopy = (unsigned int*)malloc(leftLength*sizeof(unsigned int));
    unsigned int *rightCopy = (unsigned int*)malloc(rightLength*sizeof(unsigned int));
    int leftsize = fread(leftCopy, sizeof(unsigned int),leftLength,file1);
    int rightsize = fread(rightCopy, sizeof(unsigned int),rightLength,file2);
    // copyArr(left,leftLength, leftCopy);
    // copyArr(right,rightLength, rightCopy);
    int file1Done = 0;
    while(i < leftsize && j < rightsize){
        if(leftCopy[i] <= rightCopy[j]){
            arr[k++] = leftCopy[i++];
        }
        else{
            arr[k++] = rightCopy[j++];
        }

        if(k == total){
            fwrite(arr, sizeof(unsigned int),total,finalFile);
            k = 0;
        }
        if(i == leftsize){
            i=0;
            leftsize = fread(leftCopy, sizeof(unsigned int), leftLength, file1);
            if(leftsize == 0){
                file1Done = 1;
                break;
            }
        }
        if(j == rightsize){
            j=0;
            rightsize = fread(rightCopy, sizeof(unsigned int), rightLength, file2);
            if(rightsize == 0){
                break;
            }
        }
    }

    if(!file1Done){
        while(i < leftsize){
            arr[k++] = leftCopy[i++];
            if(k == total){
                fwrite(arr, sizeof(unsigned int), k, finalFile);
                k =0;
            }
            if(i == leftsize){
                i = 0;
                leftsize = fread(leftCopy, sizeof(unsigned int), leftLength, file1);
                if(leftsize == 0){
                    fwrite(arr, sizeof(unsigned int), k, finalFile);
                    break;
                }
            }
        }
    }
    else{
        while(j < rightsize){
            arr[k++] = rightCopy[j++];
            if(k == total){
                fwrite(arr, sizeof(unsigned int), total, finalFile);
                k =0;
            }
            if(j == rightsize){
                j = 0;
                rightsize = fread(rightCopy, sizeof(unsigned int), rightLength, file2);
                if(rightsize == 0){
                    fwrite(arr, sizeof(unsigned int), k, finalFile);
                    break;
                }
            }
        }
    }
    free(rightCopy);
    free(leftCopy);
}
void n_merge(char *file, int blocksize){
    int files_size = 0;
    while(file != NULL)
        files_size++;
    int block_per_file = blocksize/files_size;
    printf("files_size %d blocksize %d", files_size, block_per_file);
}
void mergeSort(unsigned int arr[], int length){
    // Split the array into 2
    if(length <= 1)
        return;
    int leftLength = length/2;
    int rightLength = length - leftLength; 
    unsigned int * leftresult = arr;
    unsigned int * rightresult = arr + leftLength;
    mergeSort(leftresult, leftLength);
    mergeSort(rightresult, rightLength);
    merge(leftresult,rightresult, leftLength, rightLength, arr);
}
void test(unsigned int *arr, int length, FILE* file){
    int i;
    for(i=0;i<length-1;++i){
       if(arr[i] > arr[i+1]){
           printf("[Failed!] Test failed!\n");
           return;
       }
       if(i == length-2){
           i=0;
           length = fread(arr,sizeof(unsigned int), length, file);
           if(length == 0)
               break;
       }
    }
    printf("[Success!] Test successful!\n");
}

// int main(int argc, char** args){
//     int arr[12] = {1,2,6,7,8,9,1,4,5,9,10,16};
//     srand(time(NULL));
//     int sort[100000];
//     int i;
//     for(i=0;i<100000;++i){
//         sort[i] = rand()%1000000;
//     }
//     mergeSort(arr,100000);
//     print(arr, 100000);
//     test(arr, 100000);
//     return 0;
// }
