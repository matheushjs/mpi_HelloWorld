#include <iostream>
#include <vector>
#include <algorithm>

#include <mpi.h>
#include <cstdlib>
#include <ctime>
#include <cstdio>

/* Problem description
 *
 * We have a set of N numbers that we'd like to sort.
 * Though, these N numbers are generated separately from each node.
 * For this problem, we can apply the Odd-even transposition sort in parallel,
 *   as suggested in the Parallel Programming book by Peter Pacheco.
 */

using namespace std;

typedef unsigned int uint;

// Generates a vector of random integers between 0, inclusive, and 'mod', exclusive.
int *random_vec(int n, int mod){
	int *vec = (int *) malloc(n * sizeof(int));
	srand(time(NULL));
	for(; n > 0; n--)
		vec[n-1] = rand()%mod;
	return vec;
}

// Prints a vector with 'n' elements.
void print(int *vec, int n){
	for(int i = 0; i < n; i++)
		cout << vec[i] << ' ';
	cout << '\n';
}

// Cosequential merging of vectors 'vec1' and 'vec2'.
// If 'getLowest' is true, the returned vector contains the 'size' smallest elements from vec1 and vec1.
// If 'getLowest' is false, the largest elements are returned.
// Both 'vec1' and 'vec2' are assumed to be crescently sorted.
int *merge(int *vec1, int *vec2, int size, bool getLowest){
	int *merged = (int *) malloc(sizeof(int) * size);
	int i, m, n;

	if(getLowest){
		for(i=0, m=0, n=0; i < size; i++){
			if(vec1[m] < vec2[n]){
				merged[i] = vec1[m++];
			} else {
				merged[i] = vec2[n++];
			}
		}
	} else {
		// Largest elements are on the end of each vector, so merging begins there.
		for(i=size-1, m=size-1, n=size-1; i >= 0; i--){
			if(vec1[m] > vec2[n]){
				merged[i] = vec1[m--];
			} else {
				merged[i] = vec2[n--];
			}
		}
	}

	return merged;
}

int *parallel_oddeven_sort(int myRank, int commSize, int *localVec, int vecSize){
	int *aux, *buf;
	int partner;

	buf = (int *) malloc(vecSize * sizeof(int));

	// commSize phases.
	for(int i = 0; i < commSize; i++){
		if(i%2 == 0){
			// Even phase
			// 0->1
			if(myRank%2 == 0){
				partner = myRank+1; // commSize being even is enforced for now.
			} else {
				partner = myRank-1;
			}
		} else {
			// Odd phase
			// 1->2
			if(myRank == 0){
				partner = commSize - 1;
			} else if(myRank%2 == 0) {
				partner = myRank-1;
			} else {
				partner = (myRank+1)%commSize;
			}
		}

		// Exchange elements
		MPI_Sendrecv(localVec, vecSize, MPI_INT, partner, 0,
					 buf, vecSize, MPI_INT, partner, 0,
					 MPI_COMM_WORLD, MPI_STATUS_IGNORE);

		// Merge elements
		aux = merge(localVec, buf, vecSize, myRank < partner ? true : false);
		free(localVec);
		localVec = aux;
	}

	free(buf);
	return localVec;
}

int main(int argc, char *argv[]){
	int myRank, commSize;

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &commSize);
	MPI_Comm_rank(MPI_COMM_WORLD, &myRank);

	// Check argument
	if(argc != 2){
		if(myRank == 0)
			cout << "Invalid number of arguments. Please provide a vector size.\n";

		MPI_Finalize();
		return 0;
	}

	int fullSize = atoi(argv[1]);

	// Check if commSize is even
	if(commSize%2 != 0){
		if(myRank == 0)
			cout << "Number of nodes must be even.\n";
		MPI_Finalize();
		return 0;
	}

	// Check if commSize divides size.
	if(fullSize%commSize != 0){
		if(myRank == 0)
			cout << "Number of nodes should evenly divide the vector size.\n";

		MPI_Finalize();
		return 0;
	}

	// Use barriers for taking measures
	double beg, end;
	MPI_Barrier(MPI_COMM_WORLD);
	beg = MPI_Wtime();

	// Create and sort the node's individual vector
	int mySize = fullSize/commSize;
	int *myVec = random_vec(mySize, 1000);
	sort(myVec, myVec+mySize);

	if(commSize > 1)
		myVec = parallel_oddeven_sort(myRank, commSize, myVec, mySize);

	// Barrier for measures again
	MPI_Barrier(MPI_COMM_WORLD);
	end = MPI_Wtime();

	free(myVec);

	// Print result.
	if(myRank == 0){
		printf("Time spent: %.10lf\n", end - beg);
	}

	MPI_Finalize();
	return 0;
}
