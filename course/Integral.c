#include <omp.h> 
#include <stdio.h> 
#include <stdlib.h>   

static long num_steps = 100000; 
double step; 

int main(void) 
{
  int i; double x, pi = 0; double * sum[4];
  step = 4.0/(double) num_steps;
  
  omp_set_num_threads(4);

  #pragma omp parallel 
  for (i=omp_get_thread_num()*num_steps;i<omp_get_thread_num()*(num_steps+1);i++) {
    x = (i+0.5)*step;
    sum[omp_get_thread_num()] = sum[omp_get_thread_num()] + 4.0/(1.0+x*x);
  }          

  pi = step * sum[0];
  pi += step * sum[1];
  pi += step * sum[2];
  pi += step * sum[3];

  printf("pi=%f\n",pi);

  return EXIT_SUCCESS;
}