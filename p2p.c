#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "mpi.h"

#define MATRIX 100

void print_matrix(int matrix[MATRIX][MATRIX]);

int main(int argc, char** argv)
{
    int i, j, k;

    int rank, size; //MATRIX % size == 0

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int matA[MATRIX][MATRIX];
    int matB[MATRIX][MATRIX];
    int matC[MATRIX][MATRIX];

    MPI_Status status;

    if(rank == 0){
    	int n1, n2;
    	for(n1 = 0; n1 < MATRIX; n1++){
	        for(n2 = 0; n2 < MATRIX; n2++){
	    	    matA[n1][n2] = 1;
		        matB[n1][n2] = 1;
	        }
    	}

	    int p;
	    for(p = 1; p < size; p++){
            MPI_Send(&matA[p*MATRIX/size][0], MATRIX*MATRIX/size, MPI_INTEGER, p, 0, MPI_COMM_WORLD);
            MPI_Send(matB, MATRIX*MATRIX, MPI_INTEGER, p, 1, MPI_COMM_WORLD);
	    }
    }
    else{
        MPI_Recv(&matA[rank*MATRIX/size][0], MATRIX*MATRIX/size, MPI_INTEGER, 0, 0, MPI_COMM_WORLD, &status);
	    MPI_Recv(matB, MATRIX*MATRIX, MPI_INTEGER, 0, 1, MPI_COMM_WORLD, &status);
    }

    clock_t begin = clock();

    for(i = rank*MATRIX/size; i < (rank+1)*MATRIX/size; i++){
        for(j = 0; j < MATRIX; j++){
            matC[i][j] = 0;
            for(k = 0; k < MATRIX; k++){
                matC[i][j] += matA[i][k]*matB[k][j];
            }
        }
    }

    if(rank != 0){
        MPI_Send(&matC[rank*MATRIX/size][0], MATRIX*MATRIX/size, MPI_INTEGER, 0, 0, MPI_COMM_WORLD);
    }
    else{
        int p;
        for(p = 1; p < size; p++){
            MPI_Recv(&matC[p*MATRIX/size][0], MATRIX*MATRIX/size, MPI_INTEGER, p, 0, MPI_COMM_WORLD, &status);
        }
    }

    if (rank == 0){
        clock_t end = clock();
        //print_matrix(matC);
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
