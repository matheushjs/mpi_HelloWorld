#include <mpi.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#define VECSIZE 100000

void randomize(int *vec, int size){
	srand(0);
	while(size != 0){
		*vec = rand();
		vec++, size--;
	}
}

int sumvec(int *vec, int size){
	int sum = 0;
	while(size != 0){
		sum += *vec;
		vec++, size--;
	}
	return sum;
}

int main(int argc, char *argv[]){
	int sum, recv_sum, i, my_rank, comm_size, proof, chunkSize, init_idx;
	int *vec, *data;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
	MPI_Comm_size(MPI_COMM_WORLD, &comm_size);

	chunkSize = VECSIZE / comm_size; // Let it round down

	// Last ranked node will be the master
	if(my_rank == comm_size-1){
		// Initialize data
		data = malloc(sizeof(int) * VECSIZE);
		randomize(data, VECSIZE);
		proof = sumvec(data, VECSIZE);
		printf("Proof is: %d\n", proof);

		// Distribute data
		for(i = 0; i < comm_size-1; i++)
			MPI_Send(data + (chunkSize * i), chunkSize, MPI_INT, i, 0, MPI_COMM_WORLD);

		// Do your part of the work
		init_idx = (chunkSize*my_rank);
		sum = sumvec(data + init_idx, VECSIZE - init_idx);

		// Sum the other node's works
		for(i = 0; i < comm_size-1; i++){
			MPI_Recv(&recv_sum, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			sum += recv_sum;
			printf("Node summed up %d\n", recv_sum);
		}

		// Confirm correctness
		if(sum == proof){
			printf("Hello World!! The sum is: %d\n", sum);
		}

		free(data);

	} else {
		// Allocate vec
		vec = malloc(sizeof(int) * chunkSize);

		// Receive data
		MPI_Recv(vec, chunkSize, MPI_INT, comm_size-1, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

		// Do work
		sum = sumvec(vec, chunkSize);

		// Send work
		MPI_Send(&sum, 1, MPI_INT, comm_size-1, 0, MPI_COMM_WORLD);

		free(vec);
	}

	MPI_Finalize();
	return 0;
}
