#include<pthread.h>
#include<stdio.h> 

#define MAX 100000
// Shared variable
int count = 0;

void* increasing(void* args) {
    for (int i = 0; i < MAX; i++)
        count++; 
    return NULL;
}

int main(int argc, char* args[]) {
    pthread_t even_thread, odd_thread;
    pthread_create(&even_thread, NULL, increasing, NULL);
    pthread_create(&odd_thread, NULL, increasing, NULL);
    pthread_join(even_thread, NULL);
    pthread_join(odd_thread, NULL);

    printf("Finally, n = %d\n", count);
    return 0;
}

