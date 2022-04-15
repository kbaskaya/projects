/*

	Implement your CUDA kernel in this file

*/
#include "cardiacsim_kernels.h"
#include "stdio.h"

__global__ void karrayinit(double **E, double **E_prev, double **R, int n)
{
	int row = blockIdx.y * blockDim.y + threadIdx.y + 1;
	int col = blockIdx.x * blockDim.x + threadIdx.x + 1;
	if (col == 1 && row <= n + 2)
	{
		E[row] = (double *)(E + (n + 2)) + row * (n + 2);
		E_prev[row] = (double *)(E_prev + (n + 2)) + row * (n + 2);
		R[row] = (double *)(R + (n + 2)) + row * (n + 2);
	}
	if (col == 2 && row == 1)
	{
		E[0] = (double *)(E + (n + 2));
		E_prev[0] = (double *)(E_prev + (n + 2));
		R[0] = (double *)(R + (n + 2));
	}
	if (col == 3 && row == 1)
	{
		E[n + 1] = (double *)(E + (n + 2)) + (n + 1) * (n + 2);
		E_prev[n + 1] = (double *)(E_prev + (n + 2)) + (n + 1) * (n + 2);
		R[n + 1] = (double *)(R + (n + 2)) + (n + 1) * (n + 2);
	}
}

__global__ void k1halos(double **E_prev, const int n, const int m)
{

	int row = blockIdx.y * blockDim.y + threadIdx.y + 1;
	int col = blockIdx.x * blockDim.x + threadIdx.x + 1;
	if (col <= n && row <= n)
	{
		if (row == 1)
		{
			E_prev[0][col] = E_prev[2][col];
		}
		else if (row == n)
		{
			E_prev[n + 1][col] = E_prev[n - 1][col];
		}
		if (col == 1)
		{
			E_prev[row][0] = E_prev[row][2];
		}
		else if (col == n)
		{
			E_prev[row][n + 1] = E_prev[row][n - 1];
		}
	}
}

__global__ void k1pde(double **E, double **E_prev, const double alpha, const int n, const int m)
{
	int row = blockIdx.y * blockDim.y + threadIdx.y + 1;
	int col = blockIdx.x * blockDim.x + threadIdx.x + 1;
	if (row <= n && col <= n)
	{
		E[row][col] = E_prev[row][col] + alpha * (E_prev[row][col + 1] + E_prev[row][col - 1] - 4 * E_prev[row][col] + E_prev[row + 1][col] + E_prev[row - 1][col]);
	}
}

__global__ void k1ode(double **E, double **E_prev, double **R,
					  const double alpha, const int n, const int m, const double kk,
					  const double dt, const double a, const double epsilon,
					  const double M1, const double M2, const double b)
{
	int row = blockIdx.y * blockDim.y + threadIdx.y + 1;
	int col = blockIdx.x * blockDim.x + threadIdx.x + 1;
	if (row <= n && col <= n)
	{
		E[row][col] = E[row][col] - dt * (kk * E[row][col] * (E[row][col] - a) * (E[row][col] - 1) + E[row][col] * R[row][col]);
		R[row][col] = R[row][col] + dt * (epsilon + M1 * R[row][col] / (E[row][col] + M2)) * (-R[row][col] - kk * E[row][col] * (E[row][col] - b - 1));
	}
}

__global__ void kswap(double **E, double **E_prev, double **R)
{
	double **tmp = E;
	E = E_prev;
	E_prev = tmp;
}

__global__ void k2(double **E, double **E_prev, double **R, const double alpha, const int n, const int m, const double kk, const double dt, const double a, const double epsilon, const double M1, const double M2, const double b, const int init)
{
	int row = blockIdx.y * blockDim.y + threadIdx.y + 1;
	int col = blockIdx.x * blockDim.x + threadIdx.x + 1;

	if (init == 0 && threadIdx.x == 0 && row <= n)
	{
		E[0] = (double *)(E + n + 2);
		E_prev[0] = (double *)(E_prev + n + 2);
		R[0] = (double *)(R + n + 2);
		E[row] = (double *)(E + n + 2) + row * (n + 2);
		E_prev[row] = (double *)(E_prev + n + 2) + row * (n + 2);
		R[row] = (double *)(R + n + 2) + row * (n + 2);
		E[row - 1] = (double *)(E + n + 2) + (row - 1) * (n + 2);
		E_prev[row - 1] = (double *)(E_prev + n + 2) + (row - 1) * (n + 2);
		R[row - 1] = (double *)(R + n + 2) + (row - 1) * (n + 2);
		E[row + 1] = (double *)(E + n + 2) + (row + 1) * (n + 2);
		E_prev[row + 1] = (double *)(E_prev + n + 2) + (row + 1) * (n + 2);
		R[row + 1] = (double *)(R + n + 2) + (row + 1) * (n + 2);
		E[n + 1] = (double *)(E + n + 2) + (n + 1) * (n + 2);
		E_prev[n + 1] = (double *)(E_prev + n + 2) + (n + 1) * (n + 2);
		R[n + 1] = (double *)(R + n + 2) + (n + 1) * (n + 2);
	} //////////

	__syncthreads();
	if (col <= n && row <= n)
	{
		if (row == 1)
		{
			E_prev[0][col] = E_prev[2][col];
		}
		else if (row == n)
		{
			E_prev[n + 1][col] = E_prev[n - 1][col];
		}
		if (col == 1)
		{
			E_prev[row][0] = E_prev[row][2];
		}
		else if (col == n)
		{
			E_prev[row][n + 1] = E_prev[row][n - 1];
		}
	} ///////////////////////////////////////////
	__syncthreads();
	if (row <= n && col <= n)
	{
		E[row][col] = E_prev[row][col] + alpha * (E_prev[row][col + 1] + E_prev[row][col - 1] - 4 * E_prev[row][col] + E_prev[row + 1][col] + E_prev[row - 1][col]);
		////////////////////////////////////////////
		E[row][col] = E[row][col] - dt * (kk * E[row][col] * (E[row][col] - a) * (E[row][col] - 1) + E[row][col] * R[row][col]);
		R[row][col] = R[row][col] + dt * (epsilon + M1 * R[row][col] / (E[row][col] + M2)) * (-R[row][col] - kk * E[row][col] * (E[row][col] - b - 1));
	}
}

__global__ void k3(double **E, double **E_prev, double **R, const double alpha, const int n, const int m, const double kk, const double dt, const double a, const double epsilon, const double M1, const double M2, const double b, const int init)
{
	int row = blockIdx.y * blockDim.y + threadIdx.y + 1;
	int col = blockIdx.x * blockDim.x + threadIdx.x + 1;

	if (init == 0 && threadIdx.x == 0 && row <= n)
	{
		E[0] = (double *)(E + n + 2);
		E_prev[0] = (double *)(E_prev + n + 2);
		R[0] = (double *)(R + n + 2);
		E[row] = (double *)(E + n + 2) + row * (n + 2);
		E_prev[row] = (double *)(E_prev + n + 2) + row * (n + 2);
		R[row] = (double *)(R + n + 2) + row * (n + 2);
		E[row - 1] = (double *)(E + n + 2) + (row - 1) * (n + 2);
		E_prev[row - 1] = (double *)(E_prev + n + 2) + (row - 1) * (n + 2);
		R[row - 1] = (double *)(R + n + 2) + (row - 1) * (n + 2);
		E[row + 1] = (double *)(E + n + 2) + (row + 1) * (n + 2);
		E_prev[row + 1] = (double *)(E_prev + n + 2) + (row + 1) * (n + 2);
		R[row + 1] = (double *)(R + n + 2) + (row + 1) * (n + 2);
		E[n + 1] = (double *)(E + n + 2) + (n + 1) * (n + 2);
		E_prev[n + 1] = (double *)(E_prev + n + 2) + (n + 1) * (n + 2);
		R[n + 1] = (double *)(R + n + 2) + (n + 1) * (n + 2);
	} //////////

	__syncthreads();
	if (col <= n && row <= n)
	{
		if (row == 1)
		{
			E_prev[0][col] = E_prev[2][col];
		}
		else if (row == n)
		{
			E_prev[n + 1][col] = E_prev[n - 1][col];
		}
		if (col == 1)
		{
			E_prev[row][0] = E_prev[row][2];
		}
		else if (col == n)
		{
			E_prev[row][n + 1] = E_prev[row][n - 1];
		}
	} ///////////////////////////////////////////

	__syncthreads();
	if (row <= n && col <= n)
	{
		double ee = E_prev[row][col] + alpha * (E_prev[row][col + 1] + E_prev[row][col - 1] - 4 * E_prev[row][col] + E_prev[row + 1][col] + E_prev[row - 1][col]);
		double rr = R[row][col];
		double e2 = ee - dt * (kk * ee * (ee - a) * (ee - 1) + ee * rr);
		E[row][col] = e2;
		R[row][col] = rr + dt * (epsilon + M1 * rr / (e2 + M2)) * (-rr - kk * e2 * (e2 - b - 1));
	}
}

__global__ void k4(double **E, double **E_prev, double **R, const double alpha, const int n, const int m, const double kk, const double dt, const double a, const double epsilon, const double M1, const double M2, const double b, const int init, const int bx, const int by)
{
	int row = blockIdx.y * blockDim.y + threadIdx.y + 1;
	int col = blockIdx.x * blockDim.x + threadIdx.x + 1;
	if (init == 0 && threadIdx.x == 0 && row <= n && col <= n)
	{
		E[0] = (double *)(E + n + 2);
		E_prev[0] = (double *)(E_prev + n + 2);
		R[0] = (double *)(R + n + 2);
		E[row] = (double *)(E + n + 2) + row * (n + 2);
		E_prev[row] = (double *)(E_prev + n + 2) + row * (n + 2);
		R[row] = (double *)(R + n + 2) + row * (n + 2);
		E[row - 1] = (double *)(E + n + 2) + (row - 1) * (n + 2);
		E_prev[row - 1] = (double *)(E_prev + n + 2) + (row - 1) * (n + 2);
		R[row - 1] = (double *)(R + n + 2) + (row - 1) * (n + 2);
		E[row + 1] = (double *)(E + n + 2) + (row + 1) * (n + 2);
		E_prev[row + 1] = (double *)(E_prev + n + 2) + (row + 1) * (n + 2);
		R[row + 1] = (double *)(R + n + 2) + (row + 1) * (n + 2);
		E[n + 1] = (double *)(E + n + 2) + (n + 1) * (n + 2);
		E_prev[n + 1] = (double *)(E_prev + n + 2) + (n + 1) * (n + 2);
		R[n + 1] = (double *)(R + n + 2) + (n + 1) * (n + 2);
	} //////////

	extern __shared__ double shared_E_prev[];

	int lrow = threadIdx.y + 1;
	int lcol = threadIdx.x + 1;
	__syncthreads();
	if (col <= n && row <= n)
	{
		shared_E_prev[lrow * (bx + 2) + lcol] = E_prev[row][col];
		if (lrow == 1)
		{
			shared_E_prev[lcol] = row == 1 ? E_prev[2][col] : E_prev[row - 1][col];
		}

		if (lrow == by || row == n)
		{
			shared_E_prev[(lrow + 1) * (bx + 2) + lcol] = row == n ? E_prev[n - 1][col] : E_prev[row + 1][col];
		}

		if (lcol == 1)
		{
			shared_E_prev[lrow * (bx + 2)] = col == 1 ? E_prev[row][2] : E_prev[row][col - 1];
		}

		if (lcol == bx || col == n)
		{
			shared_E_prev[lrow * (bx + 2) + lcol + 1] = col == n ? E_prev[row][n - 1] : E_prev[row][col + 1];
		}

	} ///////////////////////////////////////////
	__syncthreads();
	if (row <= n && col <= n)
	{
		double ee = shared_E_prev[lrow * (bx + 2) + lcol] + alpha * (shared_E_prev[lrow * (bx + 2) + lcol + 1] + shared_E_prev[lrow * (bx + 2) + lcol - 1] - 4 * shared_E_prev[lrow * (bx + 2) + lcol] + shared_E_prev[(lrow + 1) * (bx + 2) + lcol] + shared_E_prev[(lrow - 1) * (bx + 2) + lcol]);
		double rr = R[row][col];
		double e2 = ee - dt * (kk * ee * (ee - a) * (ee - 1) + ee * rr);
		E[row][col] = e2;
		R[row][col] = rr + dt * (epsilon + M1 * rr / (e2 + M2)) * (-rr - kk * e2 * (e2 - b - 1));
	}
}
