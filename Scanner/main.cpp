#include <iostream>
#include <mpi/mpi.h>

#include <cstdlib>
#include <ctime>

using namespace std;

int *getVec(int size, int salt){
	int *result = (int *) malloc(sizeof(int) * size);
	srand(time(NULL) + salt*11*17*23);
	for(size--; size >= 0; size--)
		result[size] = rand()%100000;
	return result;
}

void printVec(int *vec, int size){
	for(int i = 0; i < size; i++)
		cout << vec[i] << " ";
	cout << "\n";
}

int getSum(int *vec, int size){
	int sum = 0;
	for(size--; size >= 0; size--)
		sum += vec[size];
	return sum;
}

int main(int argc, char *argv[]){
	int myRank, commSize;
	int size;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &myRank);
	MPI_Comm_size(MPI_COMM_WORLD, &commSize);

	if(argc != 2){
		if(myRank == 0){
			cout << "Invalid number of arguments.\n";
		}
		MPI_Finalize();
		return 0;
	}

	size = atoi(argv[1]);

	int *vec = getVec(size, myRank);
	int sum = getSum(vec, size);
	int prefSum;

	MPI_Scan(&sum, &prefSum, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);

	int dummy;
	if(myRank != 0)
		MPI_Recv(&dummy, 1, MPI_INT, myRank-1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

	cout << "Rank: " << myRank << "\n";
	cout << "My Sum: " << sum << "\n";
	cout << "My Prefix: " << prefSum << "\n";
	cout << "\n\n";

	if(myRank != commSize-1)
		MPI_Send(&dummy, 1, MPI_INT, myRank+1, 0, MPI_COMM_WORLD);

	free(vec);
	MPI_Finalize();
	return 0;
}

