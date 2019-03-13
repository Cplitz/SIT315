// Task1-1P.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>
#include <time.h>
#include <thread>

#define MATRIX_SIZE 100		// N * N size of matrix
#define NUM_THREADS 4		// Number of threads to use to solve the multiplication
#define NUM_EXECUTIONS 1000	// Number of times to execute the problem (to find suitable averages)

/**
 * Function which calculates a portion of a result matrix's rows from a matrix multiplication
 *
 * @param C A two-demensional result array matrix which should be passed in by reference
 * @param A The first array to be multiplied
 * @param B The second array to be multiplied (C = A * B)
 * @oaran iStart The index of the first row to calculate in the output matrix C
 * @param iEnd The index of the row to stop calculations on
*/
void threaded_matrix_multiplication(int C[MATRIX_SIZE][MATRIX_SIZE], int A[MATRIX_SIZE][MATRIX_SIZE], int B[MATRIX_SIZE][MATRIX_SIZE], int iStart, int iEnd) {
	for (int i = iStart; i < iEnd; i++) {
		for (int j = 0; j < MATRIX_SIZE; j++) {
			C[i][j] = 0;
			for (int k = 0; k < MATRIX_SIZE; k++) {
				C[i][j] += A[i][k] * B[k][j];
			}
		}
	}
}

int main() {
	
	// Initialise benchmark metrics
	float single = 0, threaded = 0, openmp = 0;

	// Execute the problem NUM_EXECUTIONS times
	for (int z = 0; z < NUM_EXECUTIONS; z++) {

		// Initialise A and B matrices
		int A[MATRIX_SIZE][MATRIX_SIZE];
		int B[MATRIX_SIZE][MATRIX_SIZE];

		// Assign random values between 0 and 4 inclusive
		for (int i = 0; i < MATRIX_SIZE; i++) {
			for (int j = 0; j < MATRIX_SIZE; j++) {
				A[i][j] = rand() % 5;
				B[i][j] = rand() % 5;
			}
		}

		/****************************
		 *         Sequence		    *
		 ****************************/

		// Initialise C matrix for multiplication of A and B
		int C[MATRIX_SIZE][MATRIX_SIZE];

		// Perform multiplication 
		int exec_start = clock();
		for (int i = 0; i < MATRIX_SIZE; i++) {
			for (int j = 0; j < MATRIX_SIZE; j++) {
				C[i][j] = 0;
				for (int k = 0; k < MATRIX_SIZE; k++) {
					C[i][j] += A[i][k] * B[k][j];
				}
			}
		}
		int exec_end = clock();

		// Add execution time to the sum
		single += exec_end - exec_start;

		/****************************
		 *      Multithreading      *
		 ****************************/

		// Initialise array of threads and solution array D
		std::thread threads[NUM_THREADS];
		int D[MATRIX_SIZE][MATRIX_SIZE];

		// Calculate the number of rows each thread will work on
		int num_rows = MATRIX_SIZE / NUM_THREADS, iStart, iEnd;

		exec_start = clock();
		for (int i = 0; i < NUM_THREADS; i++) {

			// Calculate index of start and end rows
			iStart = i * num_rows;
			iEnd = i * num_rows + num_rows;

			// Take on remainder load if last thread
			if (i == NUM_THREADS - 1 && iEnd < MATRIX_SIZE) {
				iEnd = MATRIX_SIZE;
			}

			// Start the thread
			threads[i] = std::thread(threaded_matrix_multiplication, std::ref(D), std::ref(A), std::ref(B), iStart, iEnd);
		}

		// Join threads together
		for (int i = 0; i < NUM_THREADS; i++) {
			threads[i].join();
		}
		exec_end = clock();
		threaded += exec_end - exec_start;

		/*****************************
		 *			OpenMP		     *
		 *****************************/

		// Initialise result matrix E
		int E[MATRIX_SIZE][MATRIX_SIZE];

		// Preface for loop with #pragma omp parallel for directive to start parallel execution
		exec_start = clock();
		#pragma omp parallel for
		for (int i = 0; i < MATRIX_SIZE; i++) {
			for (int j = 0; j < MATRIX_SIZE; j++) {
				E[i][j] = 0;
				for (int k = 0; k < MATRIX_SIZE; k++) {
					E[i][j] += A[i][k] * B[k][j];
				}
			}
		}
		exec_end = clock();
		openmp += exec_end - exec_start;
	}
	
	// Print metrics
	printf("Average Exec Time (NUM_THREADS = %d, MATRIX_SIZE = %d * %d, NUM_EXECUTIONS = %d)\nSingle: %2fms\nThreaded: %2fms\nOpenMP: %2fms\n", NUM_THREADS, MATRIX_SIZE, MATRIX_SIZE, NUM_EXECUTIONS, single / NUM_EXECUTIONS, threaded / NUM_EXECUTIONS, openmp / NUM_EXECUTIONS);
	
	return 0;
}

