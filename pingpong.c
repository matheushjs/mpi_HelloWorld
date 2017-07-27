#include <mpi.h>
#include <stdio.h>

int main(int argc, char *argv[]){
	int dummy = 1, myRank, commSize;
	double beg, end;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &myRank);
	MPI_Comm_size(MPI_COMM_WORLD, &commSize);

	if(commSize < 2) return 0;

	if(myRank == 0){
		beg = MPI_Wtime();
		MPI_Send(&dummy, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
		MPI_Recv(&dummy, 1, MPI_INT, 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		end = MPI_Wtime();
		printf("Ping: %.15lf\n", end - beg);
	} else if (myRank == 1)  {
		MPI_Recv(&dummy, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		MPI_Send(&dummy, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
	}

	MPI_Finalize();
	return 0;
}
