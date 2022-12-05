Main
#include <stdio.h>
#include <stdlib.h>

#define ONE_THREAD
#undef ONE_THREAD

#ifdef ONE_THREAD
#include "integr.h"
#else
#include "my_threads.h"
#endif

double fun(double x) {
    return 4.0 - x*x;
}

int main(void) {
    double res;
    printf("Task 1. Integral Calculation\n");
#ifdef ONE_THREAD
    printf("Test Application without Threads\n");
	res = NIntegrate(fun, 0.0, 2.0, 1.0e-6);
#else
    printf("Threaded Application\n");
    {
        TARG arg = {5, 0.0, 2.0, 1.0e-6, fun};
        res = Thread_NIntegrate(&arg);
    }
#endif
    printf("\tRes: %g\n", res);

    return EXIT_SUCCESS;
}

Threads 

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "my_threads.h"
#include "integr.h"


typedef struct {
    double * res;
    pthread_mutex_t *m;
    TARG arg;
} THREAD_ARG;


static void * integr_thread(void * arg) {
    THREAD_ARG data = *(THREAD_ARG*) arg;
    double res;
#ifdef DEBUG
    printf("Score Thread %d is started\n", data.arg.n);
#endif
    res = NIntegrate(data.arg.f, data.arg.a, data.arg.b, data.arg.eps);
    pthread_mutex_lock(data.m);
    *data.res += res;
#ifdef DEBUG
    printf("Score Thread %d is in critical section\n", data.arg.n);
#endif
    pthread_mutex_unlock(data.m);
#ifdef DEBUG
    printf("Score Thread %d is finished\n", data.arg.n);
#endif
    return NULL;
}


double Thread_NIntegrate(const TARG * arg) {
    int i;
    double h, result;
    pthread_mutex_t total_res_mutex;
    pthread_t *thread;
    THREAD_ARG *arr;


    result = 0;
    h = (arg->b - arg->a) / arg->n;
    if (pthread_mutex_init(&total_res_mutex, NULL)) {
        fprintf(stderr, "ERROR! %s\n", "Cannot initialize mutex!");
        exit(EXIT_FAILURE);
    }
    thread = (pthread_t*)malloc(arg->n * sizeof(pthread_t));
    arr = (THREAD_ARG*)malloc(arg->n * sizeof(THREAD_ARG));
    if ((thread == NULL) || (arr == NULL)) {
        fprintf(stderr, "Allocation Memory Error\n");
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < arg->n; i++) {
        arr[i].res = &result;
        arr[i].m = &total_res_mutex;
        arr[i].arg.n = i;
        arr[i].arg.a = arg->a + i*h;
        arr[i].arg.b = arr[i].arg.a + h;
        arr[i].arg.eps = arg->eps;
        arr[i].arg.f = arg->f;
#ifdef DEBUG
        printf("Interval [%g, %g]\n", arr[i].arg.a, arr[i].arg.b);
#endif
        if (pthread_create(&thread[i], NULL, &integr_thread, &arr[i]) != 0) {
            fprintf(stderr, "Score Thread %d Creation Error\n", i);
            exit(EXIT_FAILURE);
        }
    }

    for (i = 0; i < arg->n; i++) {
        if (pthread_join(thread[i], NULL) != 0) {
            fprintf(stderr, "Score Thread %d Waiting Error\n", i);
            exit(EXIT_FAILURE);
        }
    }

    pthread_mutex_destroy(&total_res_mutex);
    free(thread); free(arr);
    return result;
}

Integer
#include "integr.h"
#include <stdio.h>
#include <math.h>

double integrate(double (*f)(double x), double a, double b, unsigned long n) {
    double res, h;
    unsigned long i;
    h = (b - a) / n;
    res = 0;
    for (i = 0; i < n; i++) {
        res += f(a + (i + 0.5)*h);
    }
    res *= h;
    return res;
}

double NIntegrate(double (*f)(double x), double a, double b, double eps) {
    double i0, i1, error;
    unsigned long n = 10;

    i0 = integrate(f, a, b, n);
    do {
        n *= 2;
        i1 = integrate(f, a, b, n);
        error = fabs(i0 - i1);
        i0 = i1;
    } while(error >= eps);

    return i1;
}

