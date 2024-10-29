#include <stdio.h>
#include <pthread.h>
#include <stdatomic.h>

#define MAX 100000
int count = 0;

atomic_flag lock = ATOMIC_FLAG_INIT;

void spin_lock() {
    while (atomic_flag_test_and_set(&lock)) {
        // Busy-wait 
    }
}

void spin_unlock() {
    lock = (atomic_flag) ATOMIC_FLAG_INIT;
}

void* increasing(void* args) {
    for (int i = 0; i < MAX; i++) {
        spin_lock();
        count++;
        spin_unlock();
    }
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
