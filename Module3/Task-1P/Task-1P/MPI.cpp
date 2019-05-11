// Task-1P.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include "main.h"
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

using namespace std;

#define MAX 4

int mpi_run()
{
	int exec_start, exec_end;

	// Initialise MPI 
	MPI_Init(NULL, NULL);

	// Number of process/nodes
	int np = 0;
	MPI_Comm_size(MPI_COMM_WORLD, &np);

	// Calculate number of rows for each process and elements per process
	int process_rows = MAX / np;
	int elements_per_process = process_rows * MAX;

	// Retrieve rank of this process/node
	int rank = 0;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	// Pre-allocate matrices
	int sendA[MAX][MAX] = { 0 };
	int b[MAX][MAX] = { 0 };
	int c[MAX][MAX] = { 0 };

	// Master 
	if (rank == 0) {
		printf("+--------------------------+\n");
		printf("|  MPI only Implementation |\n");
		printf("+--------------------------+\n");

		// Initialise matrices A and B with random values
		for (int i = 0; i < MAX; i++) {
			for (int j = 0; j < MAX; j++) {
				sendA[i][j] = rand() % 3;
				b[i][j] = rand() % 3;
			}
		}

		// Print A and B
		printf("A:\n");
		for (int i = 0; i < MAX; i++) {
			for (int j = 0; j < MAX; j++) {
				printf(" %d ", sendA[i][j]);
			}
			printf("\n");
		}
		printf("B:\n");
		for (int i = 0; i < MAX; i++) {
			for (int j = 0; j < MAX; j++) {
				printf(" %d ", b[i][j]);
			}
			printf("\n");
		}

		exec_start = clock();
	}

	// Broadcast matrix B to all nodes
	MPI_Bcast(&b, MAX*MAX, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Barrier(MPI_COMM_WORLD);

	// Pre-allocate receving arrays and the solution (part) array C
	int *rcvA = new int[process_rows * MAX];
	int *partC = new int[process_rows * MAX]{ 0 };

	// Scatter the A matrix to all nodes
	MPI_Scatter(sendA, elements_per_process, MPI_INT, rcvA, elements_per_process, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Barrier(MPI_COMM_WORLD);
	
	/* MATRIX MULTIPLICATION ON SCATTERED MATRIX PART */
	for (int i = 0; i < process_rows; i++) {
		for (int j = 0; j < MAX; j++) {
			for (int k = 0; k < MAX; k++) {
				partC[i*MAX + j] += rcvA[i*MAX + k] * b[k][j];	
			}
		}
	}


	// Gather back the solution parts 
	MPI_Gather(partC, elements_per_process, MPI_INT, c, elements_per_process, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Barrier(MPI_COMM_WORLD);

	// Master processes the received parts (prints values)
	if (rank == 0) {
		exec_end = clock();

		printf("Total execution time: %dms\n", exec_end - exec_start);

		// Print matrix
		printf("C:\n");
		for (int i = 0; i < MAX; i++) {
			for (int j = 0; j < MAX; j++) {
				printf(" %d ", c[i][j]);
			}
			printf("\n");
		}
	}

	// Finalise MPI
	MPI_Finalize();

	return 0;
}