#pragma warning(disable: 4996)

#include "pch.h"
#include <CL/cl.hpp>
#include <mpi.h>
#include <time.h>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <queue>
/*
#define ARRAY_SIZE 10000

std::vector<int> arr(ARRAY_SIZE);

int np, rank;

void swap(int &first, int &second);
void mpi_quicksort(std::vector<int> &in, int, int);

int main() {
	int exec_start, exec_end, globalPivot;

	// Initialise MPI 
	MPI_Init(NULL, NULL);

	// Number of process/nodes
	MPI_Comm_size(MPI_COMM_WORLD, &np);

	// Retrieve rank of this process/node
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	// Master initialises array with random values
	if (rank == 0) {
		printf("+---------------------------------+\n");
		printf("|        MPI only Quicksort		  |\n");
		printf("+---------------------------------+\n");

		// Initialise unsorted array with random values
		for (int i = 0; i < ARRAY_SIZE; i++) {
			arr[i] = rand() % 1000;
		}
		globalPivot = arr[ARRAY_SIZE - 1];

		// Print unsorted
		printf("Unsorted:\n");
		for (int i = 0; i < ARRAY_SIZE; i++) {
			printf(" %d ", arr[i]);
		}
		printf("\n");


		exec_start = clock();
	}
	// Broadcast the pivot to all nodes
	MPI_Bcast(&globalPivot, 1, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Barrier(MPI_COMM_WORLD);

	// Determine portion of array to send to each process
	int split = ARRAY_SIZE / np;
	int leftover = ARRAY_SIZE % split;
	int * sendCounts = new int[np] { 0 };
	int * displs = new int[np] { 0 };
	for (int i = 0; i < np; i++) {
		for (int j = 0; j < np; j++) {
			displs[i] += sendCounts[j];
		}

		// Give final process leftover elements
		if (i == np - 1) { sendCounts[i] = split + leftover; }
		else { sendCounts[i] = split; }
	}
	int rcvCount = sendCounts[rank];

	// Scatter data to nodes
	std::vector<int> subArr(rcvCount);
	MPI_Scatterv(arr.data(), sendCounts, displs, MPI_INT, subArr.data(), rcvCount, MPI_INT, 0, MPI_COMM_WORLD);

	// sort
	mpi_quicksort(subArr, 0, rcvCount - 1);

	// Count elements less than and greater than globalPivot
	int left = 0;
	int right = 0;
	int leftScan = 0;
	int rightScan = 0;
	for (int i = 0; i < rcvCount; i++) {
		if (subArr[i] < globalPivot) {
			left++;
		}
		else if (subArr[i] > globalPivot) {
			right++;
		}
	}
	MPI_Barrier(MPI_COMM_WORLD);

	// Generate inclusive cumulative sums using MPI_Scan
	int leftTotal = 0;
	int rightTotal = 0;
	MPI_Scan(&left, &leftTotal, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
	MPI_Scan(&right, &rightTotal, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);

	// Gather back the sorted sub-array data
	MPI_Gatherv(subArr.data(), rcvCount, MPI_INT, arr.data(), sendCounts, displs, MPI_INT, 0, MPI_COMM_WORLD);

	// Gather back the cumulative sum data into arrays on the host
	std::vector<int> leftCumulative(np, 0);
	std::vector<int> rightCumulative(np, 0);
	MPI_Gather(&leftTotal, 1, MPI_INT, leftCumulative.data(), 1, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Gather(&rightTotal, 1, MPI_INT, rightCumulative.data(), 1, MPI_INT, 0, MPI_COMM_WORLD);

	// Host sorts data using scan results
	if (rank == 0) {
		// Use a priority queue to sort the array (counrers won't sort the array completely without recursion or without taking at least O(n log n)+ time)
		std::priority_queue<int, std::vector<int>, std::greater<int>> pq;
		for (int i = 0; i < ARRAY_SIZE; i++) {
			pq.push(arr[i]);
		}
		for (int i = 0; i < ARRAY_SIZE; i++) {
			arr[i] = pq.top();
			pq.pop();
		}



		exec_end = clock();
	}



	// Print sorted array
	if (rank == 0) {
		// Print unsorted
		printf("Sorted:\n");
		for (int i = 0; i < ARRAY_SIZE; i++) {
			printf(" %d ", arr[i]);
		}
		printf("\n");

		printf("\nTotal Execution Time: %dms", exec_end - exec_start);
	}


	// End MPI
	MPI_Finalize();

	return 0;
}

void swap(int &first, int &second) {
	int temp = first;
	first = second;
	second = temp;
}

void mpi_quicksort(std::vector<int> &in, int a, int b) {

	if (a >= b) { return; }

	int left = a;
	int right = b - 1;

	while (left <= right) {
		while (left <= right and arr[left] < arr[b]) { left++; }
		while (left <= right and arr[right] > arr[b]) { right--; }

		if (left <= right) {
			swap(arr[left], arr[right]);
			left++;
			right--;
		}
	}
	swap(arr[left], arr[b]);

	mpi_quicksort(arr, a, left - 1);
	mpi_quicksort(arr, left + 1, b);
}*/
