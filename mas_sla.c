#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "mpi.h"

#define noPoints 1000000
#define MASTER 0

double P[noPoints][2]; 

int isInCircle(double x, double y){
    double origin_dist = x*x + y*y;
    
    if (origin_dist <= 1.0)
	return 1;
    return 0;
}

int main (int argc, char *argv[])
{
	int notasks, noworkers, taskid;	
	int source, dest;
	int points, avepoint, extra, offset;
	int i, rc;
	int collect, sum;
	double start, end;

	MPI_Status status;
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &taskid);
	MPI_Comm_size(MPI_COMM_WORLD, &notasks);
	
	if (notasks < 2) {
		printf("Need at least two MPI tasks. Quitting...\n");
		MPI_Abort(MPI_COMM_WORLD, rc);
		exit(1);
	}

	noworkers = notasks-1;

	/* master task */
	if (taskid == MASTER)
	{
		printf("Program has started with %d tasks.\n", notasks);
		printf("Random points...\n");
		
		srand((unsigned int)(time(NULL))); 
  
		for (i = 0; i < noPoints; i++) {
			P[i][0] = (double)((double)rand() / (double)RAND_MAX); 
			P[i][1] = (double)((double)rand() / (double)RAND_MAX);
		}
		
		start = MPI_Wtime();

		collect = 0; sum = 0;

		//Send point data to the worker tasks
		avepoint = noPoints / noworkers;
		extra = noPoints % noworkers;
		offset = 0;
		for (dest = 1; dest <= noworkers; dest++)
		{
			points = (dest <= extra) ? avepoint+1 : avepoint;
			//printf("Sending %d points to task %d offset=%d\n", points, dest, offset);	
			MPI_Send(&points, 1, MPI_INT, dest, 1, MPI_COMM_WORLD);
			MPI_Send(&P[offset][0], points*2, MPI_DOUBLE, dest, 1, MPI_COMM_WORLD);
			offset += points;
		}

		//Reduce result from workers
		MPI_Reduce(&collect, &sum, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
		end = MPI_Wtime();
		
		//Print result
		printf("Computing time = %lf \n",(end - start));
		printf("%d points in circle, Pi = %f\n", sum, (double)sum*4.0/(double)noPoints);
		printf("Done.\n");
	}

	/*worker task*/
	if (taskid > MASTER)
	{
		collect = 0;

		MPI_Recv(&points, 1, MPI_INT, MASTER, 1, MPI_COMM_WORLD, &status);
		MPI_Recv(&P[0][0], points*2, MPI_DOUBLE, MASTER, 1, MPI_COMM_WORLD, &status);
		
		for(i = 0; i < points; i++)
			collect += isInCircle(P[i][0], P[i][1]); 
			
		MPI_Reduce(&collect, &sum, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
	}

	MPI_Finalize();
}
