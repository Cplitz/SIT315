// Task1-2C.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>
#include <thread>
#include <time.h>

#define NUM_ELEMENTS 100000					// The number of elements to sort in each random dataset
#define ELEMENT_THRESHOLD NUM_ELEMENTS / 2	// The element threshold defines a number of elements where threads should stop being created and sequential quickSort should be used instead
#define NUM_EXECUTIONS 100					// The number of test instances to run to generate benchmarking metrics

void swap(int&, int&);
int partition(int[], int, int);
void quickSortThreaded(int[], int, int);
void quickSort(int[], int, int);

int main()
{
	// Random seed
	srand(time(NULL));

	// Initialise execution time sums
	float single = 0, threaded = 0;

	// Run program NUM_EXECUTIONs times
	for (int z = 0; z < NUM_EXECUTIONS; z++) {

		// Initilaise execution time variables and dataset arrays
		int exec_start, exec_end, rand_num, singleArr[NUM_ELEMENTS], threadedArr[NUM_ELEMENTS];

		// Populate arrays with random data
		for (int i = 0; i < NUM_ELEMENTS; i++) {
			// Expected case using random data
			rand_num = rand() % 10000;
			singleArr[i] = rand_num;
			threadedArr[i] = rand_num;

			// Worst case using sorted data (beware of stack overflows from an enormous recursion call stack)
			//singleArr[i] = i;
			//threadedArr[i] = i;
		}

		/************************
		 *     SEQUENTIAL       *
		 ************************/
		exec_start = clock();
		quickSort(std::ref(singleArr), 0, NUM_ELEMENTS - 1);
		exec_end = clock();
		single += exec_end - exec_start;

		/************************
		 *     std::thread      *
		 ************************/
		exec_start = clock();
		quickSortThreaded(std::ref(threadedArr), 0, NUM_ELEMENTS - 1);
		exec_end = clock();
		threaded += exec_end - exec_start;
	}

	// Print metrics to console
	printf("Average Execution Time (ELEMENT_THRESHOLD: %d, NUM_ELEMENTS: %d)\nSingle: %2fms\nThreaded: %2fms\n",
		ELEMENT_THRESHOLD,
		NUM_ELEMENTS,
		single / NUM_EXECUTIONS, 
		threaded / NUM_EXECUTIONS
	);

}


/*
 * Swaps two elements in an array
 *
 * @param first The element to swap with second
 * @param second The element to swap with first
 */
void swap(int& first, int& second) {
	int temp = first;
	first = second;
	second = temp;
}

/*
 * Chooses a pivot element (at index high) and separates the array between low and high into items smaller than the pivot on the left and items larger than the pivot on the right
 *
 * @param arr A reference to the unsorted array to be sorted
 * @param low The lower bound index of the array
 * @param high The higher bound index of the array
 *
 * @returns The new index of the pivot element 
 */
int partition(int arr[], int low, int high) {
	int pivot = arr[high];

	int i = low - 1;

	for (int j = low; j <= high - 1; j++) {
		if (arr[j] <= pivot) {
			swap(arr[++i], arr[j]);
		}
	}
	swap(arr[i + 1], arr[high]);

	return i + 1;
}

/*
 * Parallel implementation of quicksort which sorts an array using threads until ELEMENT_THRESHOLD is reached, then falls back to sequential quicksort
 *
 * @param arr A reference to the unsorted array
 * @param low The lower bound index of the array
 * @param high The higher bound index of the array
 */
void quickSortThreaded(int arr[], int low, int high) {
	if (low < high) {
		int pi = partition(std::ref(arr), low, high);

		if (high - low < ELEMENT_THRESHOLD) {
			quickSort(std::ref(arr), low, pi - 1);
			quickSort(std::ref(arr), pi + 1, high);
		}
		else {
			std::thread threadLow(quickSortThreaded, std::ref(arr), low, pi - 1);
			std::thread threadHigh(quickSortThreaded, std::ref(arr), pi + 1, high);

			threadLow.join();
			threadHigh.join();
		}
	}
}

/*
 * Sequential (Standard) implementation of quicksort which sorts an array using a partitioning algorithm and recursion
 *
 * @param arr A reference to the unsorted array
 * @param low The lower bound index of the array
 * @param high The higher bound index of the array
 */
void quickSort(int arr[], int low, int high) {
	if (low < high) {
		int pi = partition(std::ref(arr), low, high);

		quickSort(std::ref(arr), low, pi - 1);
		quickSort(std::ref(arr), pi + 1, high);
	}
}