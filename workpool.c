#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "mpi.h"

#define PI 3.14159265358979

int pid, nump;
int points, rounds, remain_r;
MPI_Status status;

//Function: Check point is in circle or not?
int isInCircle(double x, double y) {
    double origin_dist = x*x + y*y;
    if(origin_dist <= 1.0)
        return 1;
    return 0;
}

//Master function
void master () {
    int count = 0, i = 0;
    int turns[nump];

    double start = MPI_Wtime();

    int min = nump - 1;

    if(remain_r < nump)
        min = remain_r;

    for(i = 1; i <= min; i++) {
        int round = 1;
        MPI_Send(&round, 1, MPI_INT, i, 2, MPI_COMM_WORLD);
        round += 1;
    }

    remain_r -= min - 1;

    while(remain_r > 0) {
        remain_r--;

        int value;
        MPI_Recv(&value, 1, MPI_INT, MPI_ANY_SOURCE, 3, MPI_COMM_WORLD, &status);

        count += value;

        int round = min + 1;
        if(remain_r > 0) MPI_Send(&round, 1, MPI_INT, status.MPI_SOURCE, 2, MPI_COMM_WORLD);
        round += 1;
    }

    double end = MPI_Wtime();

    for(i = 1; i < nump; i++)
        MPI_Send(&i, 1, MPI_INT, i, 0, MPI_COMM_WORLD);

    for(i = 1; i < nump; i++) {
        int value;
        MPI_Recv(&value, 1, MPI_INT, i, 4, MPI_COMM_WORLD, &status);
        turns[status.MPI_SOURCE] = value;
    }

    //Print results:
    double pi = (double)count / (double)(rounds * points) * 4.0;

    printf("Total point = %d\n", rounds * points);
    printf("PI = %f\n", pi);
    printf("Error = %f\n", fabs(PI - pi));
    printf("Execution time = %fs\n", end - start);
    printf("Processes turns:\n");
    for(i = 1; i < nump; i ++)
        printf("Process[%d] calculated %d turns\n", i, turns[i]);
}

//Slave function
void slave () {

    int turns = 0;

    while(1) {

        int round;

        MPI_Recv(&round, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        if(status.MPI_TAG == 0) {
            break;
        }
        else if(status.MPI_TAG == 2) {
            srand(round);
            int count = 0, i = 0;

            for(i = 0; i < points; i++) {
                double x = (double)rand() / (double)RAND_MAX;
                double y = (double)rand() / (double)RAND_MAX;
                count += isInCircle(x, y);
            }

            MPI_Send(&count, 1, MPI_INT, 0, 3, MPI_COMM_WORLD);
            turns++;
        }
    }

    MPI_Send(&turns, 1, MPI_INT, 0, 4, MPI_COMM_WORLD);
}

//MAIN:
int main (int argc, char *argv[]) {

    setbuf(stdout, NULL);

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &pid);
    MPI_Comm_size(MPI_COMM_WORLD, &nump);

    if(nump < 2) {
        printf("Need at least two MPI tasks. Quitting...\n");
        int rc;
        MPI_Abort(MPI_COMM_WORLD, rc);
        exit(1);
    }

    if(pid == 0) {
        printf("Enter number of points is calculated by each round: ");
        scanf("%d", &points);
        printf("Enter number of rounds (expected > 500): ");
        scanf("%d", &rounds);

        remain_r = rounds;
    }

    MPI_Bcast(&points, 1, MPI_INT, 0, MPI_COMM_WORLD);

    if(pid == 0)
        master();
    else
        slave();

    MPI_Finalize();

    return 0;
}
