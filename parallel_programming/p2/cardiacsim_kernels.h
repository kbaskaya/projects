//#include <cuda_runtime.h>

__global__ void karrayinit(double **E, double **E_prev, double **R, int n);

__global__ void k1halos(double **E_prev, const int n, const int m);

__global__ void k1pde(double **E, double **E_prev, const double alpha, const int n, const int m);

__global__ void k1ode(double **E,  double **E_prev,double **R,
	       const double alpha, const int n, const int m, const double kk,
	       const double dt, const double a, const double epsilon,
	       const double M1,const double  M2, const double b);
		   
__global__ void kswap(double **E, double **E_prev, double **R);

__global__ void k2(double **E,  double **E_prev,double **R, const double alpha, const int n, const int m, const double kk, const double dt, const double a, const double epsilon, const double M1,const double  M2, const double b, const int init);

__global__ void k3(double **E,  double **E_prev,double **R, const double alpha, const int n, const int m, const double kk, const double dt, const double a, const double epsilon, const double M1,const double  M2, const double b, const int init);

__global__ void k4(double **E,  double **E_prev,double **R, const double alpha, const int n, const int m, const double kk, const double dt, const double a, const double epsilon, const double M1,const double  M2, const double b, const int init, const int bx, const int by);