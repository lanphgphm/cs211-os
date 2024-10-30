#define _GNU_SOURCE
#include <err.h>
#include <errno.h>
#include <linux/futex.h>
#include <stdatomic.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <unistd.h>
#include <pthread.h>
#include <stdatomic.h>

/* Global lock variable */
#define MAX 100000
int count = 0;
uint32_t lock = 1; // Initialize to 1 to represent "unlocked" state

static int futex(uint32_t *uaddr, int futex_op, uint32_t val,
        const struct timespec *timeout, uint32_t *uaddr2, uint32_t val3)
{
    return syscall(SYS_futex, uaddr, futex_op, val,
                    timeout, uaddr2, val3);
}

/* Acquire the futex pointed to by 'futexp': wait for its value to
    become 1, and then set the value to 0. */

static void acquire(uint32_t *futexp)
{
    long s;
    uint32_t one; // interestingly, if declare const uint32_t, it will not work perfectly.

    /* atomic_compare_exchange_strong(ptr, oldval, newval)
        atomically performs the equivalent of:

            if (*ptr == *oldval)
                *ptr = newval;

        It returns true if the test yielded true and *ptr was updated. */

    while (1) {
        one = 1; 
        /* Is the futex available? */
        if (atomic_compare_exchange_strong(futexp, &one, 0))
            break;      /* Yes */

        /* Futex is not available; wait. */

        s = futex(futexp, FUTEX_WAIT, 0, NULL, NULL, 0);
        if (s == -1 && errno != EAGAIN)
            err(EXIT_FAILURE, "futex-FUTEX_WAIT");
    }
}

/* Release the futex pointed to by 'futexp': if the futex currently
    has the value 0, set its value to 1 and then wake any futex waiters,
    so that if the peer is blocked in fwait(), it can proceed. */

static void release(uint32_t *futexp)
{
    long s;
    const uint32_t zero = 0;

    /* atomic_compare_exchange_strong() was described
        in comments above. */

    if (atomic_compare_exchange_strong(futexp, &zero, 1)) {
        s = futex(futexp, FUTEX_WAKE, 1, NULL, NULL, 0);
        if (s  == -1)
            err(EXIT_FAILURE, "futex-FUTEX_WAKE");
    }
}


void* increasing(void* args) {
    for (int i = 0; i < MAX; i++) {
        acquire(&lock);
        count++;
        release(&lock);
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