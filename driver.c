#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <time.h>
#include <limits.h>
#include <float.h>
#include <string.h>

#include "util.h"
#include "test_order.h"
#include "cblas/cblas.h"
#include "perf/perf.h"


void test_matrix_print(void)
{
    double M[4 * 6];
    M[0] = 11; M[4] = 12; M[8] = 13;  M[12] = 14; M[16] = 15; M[20] = 16;
    M[1] = 21; M[5] = 22; M[9] = 23;  M[13] = 24; M[17] = 25; M[21] = 26;
    M[2] = 31; M[6] = 32; M[10] = 33; M[14] = 34; M[18] = 35; M[22] = 36;
    M[3] = 41; M[7] = 42; M[11] = 43; M[15] = 44; M[19] = 45; M[23] = 46;

    tdp_matrix_print(4, 6, M, 4, stdout);
    tdp_matrix_print(2, 2, M+9, 4, stdout);
}

void test_matrix_allocate(void)
{
    double *m = tdp_matrix_new(10, 10);
    tdp_matrix_print(10, 10, m, 10, stdout);
    free(m);
}

void test_matrix_one(void)
{
    double *m = tdp_matrix_new(10, 10);
    tdp_matrix_one(10, 10, 12.0, m, 10);
    tdp_matrix_print(10, 10, m, 10, stdout);
    free(m);

    m = tdp_matrix_new(2, 10);
    tdp_matrix_one(2, 10, -42.0, m, 2);
    tdp_matrix_print(2, 10, m, 2, stdout);
    free(m);

    m = tdp_matrix_new(7, 4);
    tdp_matrix_one(7, 4, -13.0, m, 7);
    tdp_matrix_print(7, 4, m, 7, stdout);
    free(m);

}

void test_vector_ddot(void)
{
    double X[] = { 1, 2, 3, 4, 5, 6 };
    assert( DEQUAL( cblas_ddot(6, X, 1, X, 1), 91.0, 0.01) );
}

void test_dgemm_order(void)
{
  double a[2*2] ={1.0,0.0,2.0,1.0};
  double b[2*2] ={3.0,1.0,8.0,9.0};
  double c[2*2] ={0.0,0.0,0.0,0.0};
  cblas_dgemm_k(CblasColMajor,CblasTrans,CblasNoTrans,2,2,2,1,a,2,b,2,0,c,2);
  affiche(2,2,c,2);
  printf("\n");
  cblas_dgemm_i(CblasColMajor,CblasTrans,CblasNoTrans,2,2,2,1,a,2,b,2,0,c,2);
  affiche(2,2,c,2);
  printf("\n");
  cblas_dgemm_j(CblasColMajor,CblasTrans,CblasNoTrans,2,2,2,1,a,2,b,2,0,c,2);
  affiche(2,2,c,2);
}

#define NB_ITER 1000

void bench_vector_ddot_incONE(void)
{
    int m = 50;
    while ( m < 1000000 ) {
        double *v1, *v2;
        v1 = tdp_vector_new(m); v2 = tdp_vector_new(m);
        tdp_vector_rand(m, 42., DBL_MAX, v1); tdp_vector_rand(m, -37.0, 500.0, v2);

        perf_t p1, p2;
        perf(&p1);
        for (int i = 0; i < NB_ITER; ++i)
            cblas_ddot(m, v1, 1, v2, 1);
        perf(&p2);

        perf_diff(&p1, &p2);
        printf("m = %6d ", m);
        uint64_t nb_op = 2 * m * NB_ITER;
        printf("Mflops = %8g | time(µs) = ", perf_mflops(&p2, nb_op));
        perf_printmicro(&p2);

        free(v1); free(v2);
        m *= 1.25;
    }
}


void bench_matrix_dgemm_incONE(void)
{
    int m = 50;
    while ( m < 1000000 ) {
      double *m1, *m2, *result;
      m1 = tdp_matrix_new(m,m); m2 = tdp_matrix_new(m,m); result = tdp_matrix_new(m,m);
        tdp_matrix_rand(m, m, m1, 42., DBL_MAX); tdp_matrix_rand(m, m, m2, -37.0, 500.0);

        perf_t p1, p2;
        perf(&p1);
        for (int i = 0; i < NB_ITER; ++i)
	  cblas_dgemm_scalar(CblasColMajor,CblasTrans,CblasNoTrans,2,2,2,1,m1,2,m2,2,0,result,2);
        perf(&p2);

        perf_diff(&p1, &p2);
        printf("m = %6d ", m);
        uint64_t nb_op = 2 * m * NB_ITER;
        printf("Mflops = %8g | time(µs) = ", perf_mflops(&p2, nb_op));
        perf_printmicro(&p2);

        free(m1); free(m2);
        m *= 1.25;
    }
}

void bench_matrix_product_order(void){
  int m=100;
  double *m1, *m2, *result;
  //  for(m=100; m<=1000;m++){
  while(m<=1000){
    m1 = tdp_matrix_new(m,m); m2 = tdp_matrix_new(m,m); result = tdp_matrix_new(m,m);
    tdp_matrix_rand(m, m, m1, 42., DBL_MAX); tdp_matrix_rand(m, m, m2, -37.0, 500.0);
    printf("m = %6d \n", m);
    printf("(i,j,k) Order :\n");
    perf_t p1, p2;
    perf(&p1);
    for (int i = 0; i < NB_ITER; ++i)
      cblas_dgemm_i(CblasColMajor,CblasTrans,CblasNoTrans,
		    m,m,m,1,m1,m,m2,m,0,result,m);
    perf(&p2);

    perf_diff(&p1, &p2);
    //tdp_matrix_print(m,m,m1,m,stdout);
    uint64_t nb_op = 2 * m  * NB_ITER * m*m;
    printf("Mflops = %8g | time(µs) = ", perf_mflops(&p2, nb_op));
    perf_printmicro(&p2);
    printf("(j,i,k) Order :\n");
    perf(&p1);
    for (int i = 0; i < NB_ITER; ++i)
      cblas_dgemm_j(CblasColMajor,CblasTrans,CblasNoTrans,
		    m,m,m,1,m1,m,m2,m,0,result,m);
    perf(&p2);

    perf_diff(&p1, &p2);
    //	tdp_matrix_print(m,m,m1,m,stdout);
    nb_op = 2 * m * NB_ITER* m*m;
    printf("Mflops = %8g | time(µs) = ", perf_mflops(&p2, nb_op));
    perf_printmicro(&p2);

    printf("(k,i,j) Order :\n");
    perf(&p1);
    for (int i = 0; i < NB_ITER; ++i)
      cblas_dgemm_k(CblasColMajor,CblasTrans,CblasNoTrans,
		    m,m,m,1,m1,m,m2,m,0,result,m);
    perf(&p2);

    perf_diff(&p1, &p2);
    /* tdp_matrix_print(m,m,m1,m,stdout); */
    nb_op = 2 * m * NB_ITER * m*m;
    printf("Mflops = %8g | time(µs) = ", perf_mflops(&p2, nb_op));
    perf_printmicro(&p2);
	
    free(m1); free(m2);
    m = m*10;

  }
}

static void cblas_dgemm_scalar_mock(
    const enum CBLAS_ORDER Order, const enum CBLAS_TRANSPOSE TransA,
    const enum CBLAS_TRANSPOSE TransB, const int M, const int N,
    const int K, const double alpha, const double *A,
    const int lda, const double *B, const int ldb,
    const double beta, double *C, const int ldc)
{
    (void) alpha;
    (void) beta;
    assert( Order == CblasColMajor );

    if (TransA == CblasTrans && TransB == CblasNoTrans) {
        for (int j = 0; j < N; ++j)
            for (int i = 0; i < M; ++i) {
                C[j*ldc+i] = 0;
                for (int k = 0; k < K; ++k)
                    C[j*ldc+i] += A[i*lda+k] * B[j*ldb+k];
            }
        return;
    }
    assert( "Unsupported Transpose Configuration" && 1 == 0 );
}

void cblas_dgemm_scalar() __attribute__((weak, alias("cblas_dgemm_scalar_mock")));

void test_dgemm_scalar(void)
{
    int m = 4, n = 5, k = 3;
    double A[k*m], B[k*n], C[m*n], D[m*n];

    A[0] = 1.0; A[3] = 2.0;  A[6] = 3.0; A[ 9] = 4.0;
    A[1] = 8.0; A[4] = 7.0;  A[7] = 6.0; A[10] = 5.0;
    A[2]  = 9.0;A[5] = 10.0; A[8] = 11.0;A[11] = 12.0;

    B[0] = 13.0; B[3] = 18.0; B[6] = 19.0; B[ 9] = 24.0; B[12] = 25.0;
    B[1] = 14.0; B[4] = 17.0; B[7] = 20.0; B[10] = 23.0; B[13] = 26.0;
    B[2] = 15.0; B[5] = 16.0; B[8] = 21.0; B[11] = 22.0; B[14] = 27.0;

    D[0] = 260.0; D[4] = 298.0; D[ 8] = 368.0; D[12] = 406.0; D[16] = 476.0;
    D[1] = 274.0; D[5] = 315.0; D[ 9] = 388.0; D[13] = 429.0; D[17] = 502.0;
    D[2] = 288.0; D[6] = 332.0; D[10] = 408.0; D[14] = 452.0; D[18] = 528.0;
    D[3] = 302.0; D[7] = 349.0; D[11] = 428.0; D[15] = 475.0; D[19] = 554.0;

    cblas_dgemm_scalar(CblasColMajor, CblasTrans, CblasNoTrans,
                       m, n, k, 0.0, A, k, B, k, 0.0, C, m);

    assert( memcmp(C, D, m*n*sizeof C[0]) == 0 );
}

int main(int argc, char **argv)
{
  (void) argv;

  srand(time(NULL) + (long)&argc);
  test_matrix_print();
  test_matrix_allocate();
  test_matrix_one();
  test_vector_ddot();
  //test_dgemm_scalar(); 
  bench_vector_ddot_incONE(); 
  /* bench_matrix_dgemm_incONE(); */
  bench_matrix_product_order();
  return EXIT_SUCCESS;
}
