#include <stdio.h>
#include <math.h>
#include <pthread.h>
#include <stdlib.h>
double l = 0, s = 0;
double sum[1000];
void * calc(void * q)
{
    int n = *(int *)q;
    double i = 0;
    for (i = s * n; i < s * n + s; i = i + l)
        sum[n] = sum[n] + sqrt(1-i*i)* l;
}

int main()
{
    double summa = 0, m = 0, n = 0;
    int i = 0;
    printf("Enter number of threads and precision\n");
    scanf("%lf%lf", &n, &m);
    pthread_t tid[1000];
    l = 1 / m;
    s = 1 / n;
    for (i = 0; i < n; i++)
    {
        void * q = (void *)&i;
        pthread_create(&tid[i], NULL, calc, q);
        pthread_join(tid[i], NULL);
        summa = summa + sum[i];
    }
    printf("pi is %.15lf\n", summa * 4);
    return 0;
}
