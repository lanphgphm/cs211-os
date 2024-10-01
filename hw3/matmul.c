#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <time.h> // for random seeds and timing functions

typedef struct {
    int offset;  
    int workload;  
    int n;  
    int **A;  
    int **B;  
    int **C;  
} thread_args_t;

void generate_mat(int n, int** M, int seed){ 
    srand(seed); 
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            M[i][j] = rand() % 11;  // to keep numbers small
        }
    }
}

void base_matmul(int n, int** A, int** B, int** C){
    for (int i = 0; i < n; i++){
        for (int j = 0; j < n; j++) { 
            C[i][j] = 0; 
            for (int k = 0; k < n; k++){
                C[i][j] += A[i][k] * B[k][j]; 
            }
        }
    }
}

void divide_workload(int n, int k, int* workload){
    // each thread multiplies a few rows of the matrix 
    for (int i = 0; i < k-1; i++){
        workload[i] = n/k; 
    }
    workload[k-1] = n - (k-1)*(n/k); 
}

void* inthread_calculate(void* argList){
    thread_args_t* args = (thread_args_t*)argList;

    int offset = args->offset;
    int workload = args->workload;
    int n = args->n;
    int **A = args->A;
    int **B = args->B;
    int **C = args->C;

    for (int i = offset; i < offset + workload; i++){
        for (int j = 0; j < n; j++){
            C[i][j] = 0; 
            for (int tmp = 0; tmp < n; tmp++){
                // replace tmp = k, avoid confusion
                C[i][j] += A[i][tmp] * B[tmp][j];
            }
        }
    }

    return NULL; 
}

void multi_matmul(int n, int k, int** A, int** B, int** C, int* workload){
    pthread_t threads[k]; 
    thread_args_t* args[k];
    int offset = 0; // starting row 

    for (int i = 0; i < k; i++){
        args[i] = malloc(sizeof(thread_args_t)); 
        args[i]->offset = offset;
        args[i]->workload = workload[i];
        args[i]->n = n;
        args[i]->A = A;
        args[i]->B = B;
        args[i]->C = C;

        pthread_create(&threads[i], NULL, inthread_calculate, (void*) args[i]); 
        offset += workload[i]; 
    }

    for (int i = 0; i < k; i++) {
        pthread_join(threads[i], NULL); 
    }
}

void print_matrix(int n, int** M) {
    for (int i = 0; i < n; i++) {       
        for (int j = 0; j < n; j++) {     
            printf("%d ", M[i][j]);   
        }
        printf("\n"); 
    }
    printf("\n");
}


int main(int argc, char* args[]){ 
    if (argc < 3){ 
        printf("Usage must include 2 numbers, first number is n (size of matrix), second number is k (number of threads)");
        return 1;  
    }

    int n = atoi(args[1]); 
    int k = atoi(args[2]);

    // ---------------------------------------------
    // note: CANNOT A[n][k] because n, k must be known at compile time
    // our usecase is n, k defined at runtime
    int** A = (int**) malloc(n*sizeof(int*)); 
    int** B = (int**) malloc(n*sizeof(int*)); 
    int** C = (int**) malloc(n*sizeof(int*)); 

    for (int i = 0; i < n; i++) {
        A[i] = (int*) malloc(n*sizeof(int));
        B[i] = (int*) malloc(n*sizeof(int));
        C[i] = (int*) malloc(n*sizeof(int));
    }
    // ---------------------------------------------

    generate_mat(n, A, 42); 
    generate_mat(n, B, 37);

    int workload[k];
    divide_workload(n, k, workload); 

    // printf("Input matrix A: \n");
    // print_matrix(n, A); 
    // printf("Input matrix B: \n");
    // print_matrix(n, B); 

    clock_t start2 = clock();
    base_matmul(n, A, B, C);
    clock_t end2 = clock();
    // printf("Result of single-threaded multiplication: \n"); 
    // print_matrix(n, C); 

    clock_t start1 = clock();
    multi_matmul(n, k, A, B, C, workload);
    clock_t end1 = clock();
    // printf("Result of multi-threaded multiplication: \n"); 
    // print_matrix(n, C); 

    printf("Time taken for single-threaded matmul: %f\n", ((double) (end2 - start2)) / CLOCKS_PER_SEC); 
    printf("Time taken for multi-threaded matmul: %f\n", ((double) (end1 - start1)) / CLOCKS_PER_SEC);

    // ---------------------------------------------
    for (int i = 0; i < n; i++) {
        free(A[i]);
        free(B[i]);
        free(C[i]);
    }

    free(A);
    free(B);
    free(C);
    // ---------------------------------------------

    return 0; 
}