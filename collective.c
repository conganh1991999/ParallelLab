#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "mpi.h"

#define MATRIX 100

void print_matrix(int matrix[MATRIX][MATRIX]);

int main(int argc, char** argv)
{
    int i, j, k, sum;
    int output[MATRIX][MATRIX];
    int matA[MATRIX][MATRIX]; //matrixA[row][col]

    int rank, size; //MATRIX % size == 0

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int A[MATRIX], matB[MATRIX][MATRIX]; //matrixB[col][row]

    if(rank == 0){
    	int n1, n2;
    	for(n1 = 0; n1 < MATRIX; n1++){
	    	for(n2 = 0; n2 < MATRIX; n2++){
	    		matA[n1][n2] = 1;
				matB[n2][n1] = 1;
	    	}
    	}
    }

    MPI_Bcast(matB, MATRIX*MATRIX, MPI_INTEGER, 0,  MPI_COMM_WORLD);

    clock_t begin = clock();

    for(i = 0; i < MATRIX; i++){
		if(rank==0)
	    	MPI_Scatter(matA[i], MATRIX, MPI_INT, A, MATRIX, MPI_INT, 0, MPI_COMM_WORLD);

		MPI_Bcast(A, MATRIX, MPI_INTEGER, 0, MPI_COMM_WORLD);

		for(j = 0; j < MATRIX/size; j++){
	    	sum = 0;
	    	for(k = 0; k < MATRIX; k++){
	        	sum += A[k]*matB[rank+size*j][k];
	    	}
	    	//printf("sum = %d, insert to output[%d][%d]\n", sum, i, index);
	    	MPI_Gather(&sum, 1, MPI_INTEGER, &output[i][rank+size*j], 1, MPI_INTEGER, 0, MPI_COMM_WORLD);
		}
    }

    if (rank == 0){
		clock_t end = clock();
		//print_matrix(matA);
        //print_matrix(output);
        printf("execution time = %f\n", (double)((end - begin)/CLOCKS_PER_SEC));
    }

    MPI_Finalize();

    return 0;
}

void print_matrix(int matrix[MATRIX][MATRIX])
{
    int r, c;
    for(r = 0; r < MATRIX; r++) {
        for (c = 0; c < MATRIX; c++) {
            printf("%d ", matrix[r][c]);
        }
        printf ("\n");
    }
}
