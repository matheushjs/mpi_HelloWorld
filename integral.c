#include <stdio.h>
#include <mpi.h>
#include <math.h>

/* Integration using the trapezoidal rule.
 */

enum Tags {
	TAG_A,
	TAG_B,
	TAG_P
};

double primitive(double x){
	return sin(x);
}

double integrand(double x){
	return cos(x);
}

double integrate(double (*func)(double), double a, double b, int npartitions){
	double result;
	double interval;

	interval = (b - a) / (double) npartitions;
	result = (func(a) + func(b)) / 2.0;
	for(a = a + interval; a < b; a += interval){
		result += func(a);
	}
	
	return result * interval;
}

int nextpow2(int n){
	int i;
	for(i = 1; i < n; i <<= 1);
	return i;
}

int main(int argc, char *argv[]){
	int i, myRank, commSize, p_share, step, lim;
	double result, a_share, b_share;

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &commSize);
	MPI_Comm_rank(MPI_COMM_WORLD, &myRank);

	if(myRank == 0){
		double a_in, b_in, delta;
		int p_in;
		
		// Receive input from user
		printf("Type [initial x] [final x] [number of partitions]:\n");
		scanf("%lf %lf %d\n", &a_in, &b_in, &p_in);
		
		// Divide & send work
		delta = (b_in - a_in) / (double) commSize;
		p_share = p_in / commSize; // let it round down
		for(i = 1; i < commSize; i++){
			a_share = a_in + i * delta;
			b_share = (i != commSize-1) ? a_share + delta : b_in; // because we rounded down earlier

			MPI_Send(&a_share, 1, MPI_DOUBLE, i, TAG_A, MPI_COMM_WORLD);
			MPI_Send(&b_share, 1, MPI_DOUBLE, i, TAG_B, MPI_COMM_WORLD);
			MPI_Send(&p_share, 1, MPI_INT, i, TAG_P, MPI_COMM_WORLD);
		}

		// Do your part of the work
		result = integrate(integrand, a_in, a_in+delta, p_share);

		// Receive others' work
		lim = nextpow2(commSize);
		step = lim / 2;
		while(myRank < step){
			for(i = myRank + step; i < lim && i < commSize; i += step){
				MPI_Recv(&a_share, 1, MPI_DOUBLE, i, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
				result += a_share;
			}
			lim = step;
			step /= 2;
		}

		printf("Integration result: %lf\n", result);
		printf("Primitive result: %lf\n", primitive(b_in) - primitive(a_in));
	} else {
		// Receive work from node 0
		MPI_Recv(&a_share, 1, MPI_DOUBLE, 0, TAG_A, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		MPI_Recv(&b_share, 1, MPI_DOUBLE, 0, TAG_B, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		MPI_Recv(&p_share, 1, MPI_INT, 0, TAG_P, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		
		// do work
		result = integrate(integrand, a_share, b_share, p_share);

		// Receive others' work
		lim = nextpow2(commSize);
		step = lim / 2;
		while(myRank < step){
			for(i = myRank + step; i < lim && i < commSize; i += step){
				MPI_Recv(&a_share, 1, MPI_DOUBLE, i, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
				result += a_share;
			}
			lim = step;
			step /= 2;
		}

		// Send work forward
		for(step = nextpow2(commSize)/2; myRank < step; step /= 2);
		MPI_Send(&result, 1, MPI_DOUBLE, myRank - step, 0, MPI_COMM_WORLD);
	}

	MPI_Finalize();
	return 0;
}
