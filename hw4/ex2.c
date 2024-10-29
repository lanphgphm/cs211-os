#include<pthread.h>
#include<stdio.h> 
#include<time.h> 

#define MAX 100000
// Shared variable
int count = 0;
pthread_mutex_t mutexLck; 

void* increasing(void* args) {
    for (int i = 0; i < MAX; i++){
        pthread_mutex_lock(&mutexLck); 
        count++; 
        pthread_mutex_unlock(&mutexLck); 
    }
    return NULL;
}

int main(int argc, char* args[]) {
    struct timespec start, end; 
    clock_gettime(CLOCK_MONOTONIC, &start); 

    pthread_mutex_init(&mutexLck, NULL); 
        pthread_t even_thread, odd_thread;
        pthread_create(&even_thread, NULL, increasing, NULL);
        pthread_create(&odd_thread, NULL, increasing, NULL);
        pthread_join(even_thread, NULL);
        pthread_join(odd_thread, NULL);
    pthread_mutex_destroy(&mutexLck); 

    clock_gettime(CLOCK_MONOTONIC, &end); 
    double elapsed_time = (end.tv_sec - start.tv_sec) + 
                          (end.tv_nsec - start.tv_nsec) / 1e9;

    printf("Finally, n = %d\n", count);
    printf("Elapsed time: %.6f sec\n", elapsed_time); 
    return 0;
}

