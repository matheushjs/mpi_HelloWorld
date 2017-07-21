#include <mpi.h>
#include <stdio.h>
#include <unistd.h>

int main(int argc, char *argv[]){
	int rank, size, i;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	if(rank != 0){
		MPI_Send(&rank, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
	} else {
		for(i = 1; i < size; i++){
			MPI_Recv(&rank, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			printf("process %d is alive \n", rank);
			sleep(1);
		}
		printf("And I am process 0\n");
	}

	MPI_Finalize();
	return 0;
}
