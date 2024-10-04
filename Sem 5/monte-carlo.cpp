#include <iostream>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>

int M = 200000000;		//количество точек
int N = 1;

double start = 0;		//начало отрезка
double end = 1;		//конец отрезка

double result = 0;	//результат интегрирования
pthread_t* threads;

double f(double x) { return 3*x*x;}			//функция

/*Нахождение максимума функции */
double max(double start, double end)
{
	double max_f = f(start);
	double cur = start;
	while(cur<=end)
	{
		if(f(cur)>=max_f)
			max_f = f(cur);
		cur += 0.01;
	}
	return max_f;
}
/* собственно метод */
double integrate(double start, double end)
{
    int K = 0;    	//попавшие точки
    double S = 0; 	//площадь прямоугольника
    double q, y;	//координаты
    double max_f = max(start, end);
    for(int i=0; i<M; i++) {
	q = start + rand()*(end - start)/RAND_MAX;
	y = rand()*max_f/RAND_MAX;
	if (y < f(q))					//проверка принадлежности точки области
	    K++;
    }
    S=(end-start)*(max_f);
    return S*K/M;
}

void* func_thread(void* arg)
{
    double I = integrate(start, end); 		//интегрирование
    result+=I;

	return NULL;
}

int main(int argc, char *argv[])
{
    char *b;
    if (argc < 4)
    {
	std::cout<<"Usage: "<<argv[0]<< "<number of threads> <start> <end>"<<"\n";
	return 1;
    }
    N = strtol(argv[1],&b,10);
    start = strtod(argv[2],&b);
    end = strtod(argv[3],&b);

/*создание процессов */
    if((threads=(pthread_t*)malloc(N*sizeof(pthread_t))) == 0)
    {
	std::cout<<"Error creating threads"<<"\n";
	return 1;
    }
    	for(int i=0; i<N; i++)
		if((pthread_create(&threads[i], NULL, func_thread, NULL) != 0))
		{
	    		std::cout<<"Can`t create thread # "<<i<<"\n";
	    		exit(-1);
		}

    	for(int i=0; i<N; i++)			//ожидание всех процессов
		pthread_join(threads[i], NULL);

    	std::cout<<"\n"<<"MAX(f) = "<<max(start, end)<<"\n";			//результат интегрирования и максимума функции
    	std::cout<<"I = "<< result/N <<"\n\n";

	free(threads);
    	return 0;
}
