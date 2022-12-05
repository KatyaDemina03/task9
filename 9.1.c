#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

long int val = 0;

void *increment(void *arg) {
    long i, num = *((long *)arg);
    for (i = 0; i < num; i++) {
        val += 1;
    }
    return NULL;
}

void *decrement(void *arg) {
    long i, num = *((long *)arg);
    for (i = 0; i < num; i++) {
        val -= 1;
    }
    return NULL;
}

int main() {
    pthread_t id1, id2;
    long n;
    printf("\tTest Task1 Conditions\n");
    printf("n: ");
    scanf("%ld", &n);
    printf("Let do %ld pair operations\n", n);
    printf("Before work num = %ld\n", val);
    printf("\tStart ScoreThreads...\n");

    if (pthread_create(&id1, NULL, increment, (void*)&n)) {
        fprintf(stderr, "ERROR!!! Cannot Create Thread\n");
        exit(EXIT_FAILURE);
    }
    if (pthread_create(&id2, NULL, decrement, (void*)&n)) {
        fprintf(stderr, "ERROR!!! Cannot Create Thread\n");
        exit(EXIT_FAILURE);
    }

    pthread_join(id1, NULL);
    pthread_join(id2, NULL);

    printf("After work num = %ld\n", val);

    return 0;
}
}