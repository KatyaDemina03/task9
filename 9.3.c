Main
#include <stdio.h>
#include <stdlib.h>

#include "task.h"
//#include "tarray.h"

int main(void) {
    int array_size = 7;
    printf("Task3. Read/Write Lock\n");
    {
        TPARAM init;
        TARRAY arr;
        tarray_init(&arr, array_size);
        init.arr = &arr;
        init.num_readers = 6;
        init.num_writers = 3;
        init.num_reps = 10;
        task_solution(&init);
        tarray_dest(&arr);
    }

    printf("\n\nNext Session...\n");
    array_size = 5;
    {
        TPARAM init;
        TARRAY arr;
        tarray_init(&arr, array_size);
        init.arr = &arr;
        init.num_readers = 3;
        init.num_writers = 2;
        init.num_reps = 5;
        task_solution(&init);
        tarray_dest(&arr);
    }

    return EXIT_SUCCESS;
}

Task
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include "task.h"
#include "threads.h"

typedef struct {
    pthread_t * thread;
    TARG * arg;
} MAKE_RES;

static pthread_t make_detached(TARRAY * arr);
static MAKE_RES * make_writers(TARG * targ);
static MAKE_RES * make_readers(TARG * targ);


void task_solution(TPARAM * init) {
    pthread_t thread;
    TARG arg_writer, arg_reader;
    MAKE_RES * writer;
    MAKE_RES * reader;
    int i;

    printf("Work with %d writer threads\n", init->num_writers);
    printf("Work with %d reader threads\n", init->num_readers);
    printf("All threads make %d reaps\n", init->num_reps);
    printf("Initial array: ");
    tarray_print(init->arr, &stdout);
    printf("\n");
    thread = make_detached(init->arr);
    //sleep(5);
    arg_writer.arr = init->arr;
    arg_writer.num_reps = init->num_reps;
    arg_writer.num_thread = init->num_writers;
    writer = make_writers(&arg_writer);
    arg_reader.arr = init->arr;
    arg_reader.num_reps = init->num_reps;
    arg_reader.num_thread = init->num_readers;
    reader = make_readers(&arg_reader);

    for (i = 0; i < init->num_writers; i++) {
        if (pthread_join(writer->thread[i], NULL) != 0) {
            fprintf(stderr, "Writer Thread %d Waiting Error\n", i);
            exit(EXIT_FAILURE);
        }
    }
    for (i = 0; i < init->num_readers; i++) {
        if (pthread_join(reader->thread[i], NULL) != 0) {
            fprintf(stderr, "Reader Thread %d Waiting Error\n", i);
            exit(EXIT_FAILURE);
        }
    }

    free(writer->thread);  free(writer->arg);  free(writer);
    free(reader->thread);  free(reader->arg);  free(reader);

    if(pthread_cancel(thread)) {
        fprintf(stderr, "ERROR! Cannot stop detached thread!\n");
        exit(EXIT_FAILURE);
    }

    printf("Result array : ");
    tarray_print(init->arr, &stdout);
    printf("\n");
    sleep(5);
}


static pthread_t make_detached(TARRAY * arr) {
    pthread_attr_t attr;
    pthread_t thread;
    int a;

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    if (!pthread_attr_getdetachstate(&attr, &a)) {
        printf("%d\t%d\t", a, PTHREAD_CREATE_DETACHED);
        if (a == PTHREAD_CREATE_DETACHED) {
            printf("Really detached\n");
        }
    }
    if (pthread_create(&thread, &attr, &detach_thread, (void*)arr)) {
        fprintf(stderr, "Error while Creation Detached Thread\n");
        exit(EXIT_FAILURE);
    }
    pthread_attr_destroy(&attr);

    return thread;
}


static MAKE_RES * make_writers(TARG * targ) {
    MAKE_RES * res;
    TARG * arg;
    int i;

    res = (MAKE_RES *) malloc(sizeof(MAKE_RES));
    if (!res) {
        fprintf(stderr, "Allocation memory error\n");
        exit(EXIT_FAILURE);
    }
    res->thread = (pthread_t *) calloc(targ->num_thread, sizeof(pthread_t));
    res->arg = (TARG *) calloc(targ->num_thread, sizeof(TARG));
    if ((res->thread == NULL) || (res->arg == NULL)) {
        fprintf(stderr, "Allocation memory error\n");
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < targ->num_thread; i++) {
        res->arg[i].num_reps = targ->num_reps;
        res->arg[i].num_thread = i;
        res->arg[i].arr = targ->arr;
        if (pthread_create(&(res->thread[i]), NULL, &write_thread, &(res->arg[i]))) {
            fprintf(stderr, "Writer Thread %d Creation Error\n", i);
            exit(EXIT_FAILURE);
        }
    }

    return res;
}


static MAKE_RES * make_readers(TARG * targ) {
    MAKE_RES * res;
    TARG * arg;
    int i;

    res = (MAKE_RES *) malloc(sizeof(MAKE_RES));
    if (!res) {
        fprintf(stderr, "Allocation memory error\n");
        exit(EXIT_FAILURE);
    }
    res->thread = (pthread_t *) calloc(targ->num_thread, sizeof(pthread_t));
    res->arg = (TARG *) calloc(targ->num_thread, sizeof(TARG));
    if ((res->thread == NULL) || (res->arg == NULL)) {
        fprintf(stderr, "Allocation memory error\n");
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < targ->num_thread; i++) {
        res->arg[i].num_reps = targ->num_reps;
        res->arg[i].num_thread = i;
        res->arg[i].arr = targ->arr;
        if (pthread_create(&(res->thread[i]), NULL, &read_thread, &(res->arg[i]))) {
            fprintf(stderr, "Reader Thread %d Creation Error\n", i);
            exit(EXIT_FAILURE);
        }
    }

    return res;
}
My threads 

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include "threads.h"
#include "tarray.h"

void * write_thread(void * arg) {
    TARG * targ = (TARG *)arg;
    int k, ind;
    double val;
    printf("\tWriter Thread %d is started\n", targ->num_thread);
    sleep(targ->num_thread);
    srand(time(NULL));
    for (k = 0; k < targ->num_reps; k++) {
        ind = rand() % targ->arr->size;
        val = (double)rand() / RAND_MAX;
        tarray_set(targ->arr, ind, val);
        printf("\tWriter Thread No %d, res[%d] = %g\n", targ->num_thread, ind, val);
        sleep(ind);
    }
    printf("\tWriter Thread %d is stopped\n", targ->num_thread);
    return NULL;
}

void * read_thread(void * arg) {
    TARG * targ = (TARG *)arg;
    int k, ind;
    double res;
    printf("\t\tReader Thread %d is started\n", targ->num_thread);
    sleep(targ->num_thread);
    srand(time(NULL));
    for (k = 0; k < targ->num_reps; k++) {
        ind = rand() % targ->arr->size;
        res = tarray_get(targ->arr, ind);
        printf("\t\tReader Thread No %d, res[%d] = %g\n", targ->num_thread, ind, res);
        sleep(ind);
    }
    printf("\t\tReader Thread %d is stopped\n", targ->num_thread);
    return NULL;
}


void * detach_thread(void * arg) {
    TARRAY arr = *(TARRAY*)arg;

    if (pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL)) {
        fprintf(stderr, "Cannot change cancel state\n");
        exit(EXIT_FAILURE);
    }
    if (pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL)) {
        fprintf(stderr, "Cannot change cancel type\n");
        exit(EXIT_FAILURE);
    }
    printf("\tDetached Thread is Started\n");
    while (1) {
        printf("Current state:");
        tarray_print(&arr, &stdout);
        printf("\n");
        sleep(1);
    }
    printf("\tDetached Thread is Stopped\n");
    return NULL;
}

