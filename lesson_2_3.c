#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>

void incr (int* num) {
    (*num)++;
    return;
}

int main() {
    long long number = 0;
    pthread_t id = 0;
    for (long long i = 0; i < 10000; i++) {
        pthread_create(&id, NULL, (void * (*)(void *)) incr, (void *) &number);
    }
    
    printf("number %lld\n", number);
    return 0;
}