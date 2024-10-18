#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
void *thread_function(void *arg)
{
    fprintf(stdout, "child thread pid is %d ", (int)getpid());
    /* Бесконечный цикл. */
    while (1)
        ;
    return NULL;
}

int main()
{
    pthread_t thread;
    fprintf(stdout, "main thread pid is %d ", (int)getpid());
    pthread_create(&thread, NULL, &thread_function, NULL);
    /* Бесконечный цикл. */
    while (1)
        ;
    return 0;
}
