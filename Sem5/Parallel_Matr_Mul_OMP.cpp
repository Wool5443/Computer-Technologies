#include <stdio.h>
#include "omp.h"
#include <stdlib.h>

int main()
{
   int n=4000, m=4000;
   // scanf("%d %d", &n, &m);
   int i, j;
   int *a = (int *)malloc(sizeof(int) * n * m);
   int *b = (int *)malloc(sizeof(int) * m * n);
   int *c = (int *)malloc(sizeof(int) * n * n);
   omp_set_dynamic(0);     // ��������� ���������� openmp ������ ����� ������� �� ����� ����������
   omp_set_num_threads(4); // ���������� ����� ������� � 4

   // �������������� �������
   for (i = 0; i < n * m; i++)
   {
      a[i] = i;
      b[i] = i;
   }
   // for (i = 0; i < n; i++)
   // {
   //    for (j = 0; j < m; j++)
   //       printf("%d ", a[i * n + j]);
   //    printf("\n");
   // }
   for (i = 0; i < n * n; i++)
   {
      c[i] = 0;
   }
   // �����������
#pragma omp parallel for
   for (i = 0; i < n; i++)
      for (j = 0; j < n; j++)
         for (int k = 0; k < m; k++)
            c[n * i + j] += a[n * i + k] * b[n * k + j];

   // for (i = 0; i < n; i++)
   // {
   //    for (j = 0; j < n; j++)
   //       printf("%d ", c[i * n + j]);
   //    printf("\n");
   // }
   return 0;
}
