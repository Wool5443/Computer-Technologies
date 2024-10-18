#include <pthread.h>
#include <stdio.h>

int a = 0;

typedef void* (*ThreadRoutine)(void* args);

void* my_thread(void* dummy)
{
    pthread_t mythid = pthread_self();

    a++;

    printf("Thread: %lu, a = %d\n", mythid, a);

    return NULL;
}

#define NUM_OF_THREADS 90000
int main()
{
    pthread_t threads[NUM_OF_THREADS] = {};

    for (size_t i = 0; i < NUM_OF_THREADS; i++)
    {
        int result = pthread_create(&threads[i], NULL, my_thread, NULL);

        if (result != 0)
        {
            perror("ERROR CREATING THREAD ");
            return -1;
        }
    }

    for (size_t i = 0; i < NUM_OF_THREADS; i++)
    {
        pthread_join(threads[i], NULL);
    }

    return 0;
}
