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

#define ARRAY_SIZE 1000
#define OPENCL_MAX_GROUP_SIZE 64

std::vector<int> arr(ARRAY_SIZE);

int np, rank, err;
cl::Program program;
cl::Context context;
cl::Device device;

void opencl_quicksort(std::vector<int> &in, int, int);
void insertionsort(std::vector<int> &in, int, int);
cl::Program CreateProgram(const char*);

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
		printf("|    MPI+OpenCL only Quicksort    |\n");
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

	/****************************
	 *		OPENCL SETUP		*
	 ****************************/
	// Get the program
	program = CreateProgram("../Cloud/partition.cl");

	// Get the context from the program
	context = program.getInfo<CL_PROGRAM_CONTEXT>();

	// Get the device from the context
	std::vector<cl::Device> devices = context.getInfo<CL_CONTEXT_DEVICES>();
	device = devices.front();

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

	// GPU-Quicksort
	opencl_quicksort(subArr, 0, rcvCount - 1);


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


void opencl_quicksort(std::vector<int> &in, int start, int end) {

	int num_elements = end - start + 1;

	// Base condition
	if (num_elements < 2) {
		return;
	}
	else if (num_elements < 1000) {
		insertionsort(in, start, end);
		return;
	}

	// Get elements per group based on the group size and determine number of groups
	int elements_per_group = num_elements >= OPENCL_MAX_GROUP_SIZE ? OPENCL_MAX_GROUP_SIZE : num_elements;
	int groups = num_elements % elements_per_group == 0 ? num_elements / elements_per_group : num_elements / elements_per_group + 1;

	// Store the pivot
	int pivot = in[end];

	/**************************
	 *   COMPUTE COUNTS       *
	 **************************/
	 // Init cumulative arrays
	std::vector<int> leftCumulative(groups, 0);
	std::vector<int> rightCumulative(groups, 0);

	// Init buffers for computeCounts kernel
	cl::Buffer inBuf(context, CL_MEM_READ_ONLY | CL_MEM_HOST_NO_ACCESS | CL_MEM_COPY_HOST_PTR, sizeof(int) * ARRAY_SIZE, in.data(), &err);
	cl::Buffer leftCumulativeBuf(context, CL_MEM_READ_WRITE | CL_MEM_HOST_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(int) * groups, leftCumulative.data(), &err);
	cl::Buffer rightCumulativeBuf(context, CL_MEM_READ_WRITE | CL_MEM_HOST_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(int) * groups, rightCumulative.data(), &err);

	// Init the kernel and set args
	cl::Kernel kernel1(program, "computeCounts", &err);
	err = kernel1.setArg(0, inBuf);
	err = kernel1.setArg(1, sizeof(int), (void *)&pivot);
	err = kernel1.setArg(2, sizeof(int), (void *)&num_elements);
	err = kernel1.setArg(3, sizeof(int), (void *)&groups);
	err = kernel1.setArg(4, sizeof(int), (void *)&elements_per_group);
	err = kernel1.setArg(5, sizeof(int), (void *)&start);
	err = kernel1.setArg(6, leftCumulativeBuf);
	err = kernel1.setArg(7, rightCumulativeBuf);

	// Init the queue and execute the kernel
	cl::CommandQueue queue(context, device);
	err = queue.enqueueNDRangeKernel(kernel1, cl::NullRange, cl::NDRange(groups * elements_per_group), cl::NDRange(elements_per_group));

	// Read buffers
	err = queue.enqueueReadBuffer(leftCumulativeBuf, CL_TRUE, 0, sizeof(int) * groups, leftCumulative.data());
	err = queue.enqueueReadBuffer(rightCumulativeBuf, CL_TRUE, 0, sizeof(int) * groups, rightCumulative.data());

	cl::finish();

	// Global synchronization / global cumulative sum
	std::vector<int> leftCumulativeSum(groups + 1, 0);
	std::vector<int> rightCumulativeSum(groups + 1, 0);
	for (int i = 1; i <= groups; i++) {
		leftCumulativeSum[i] = leftCumulative[i - 1] + leftCumulativeSum[i - 1];
		rightCumulativeSum[i] = rightCumulative[i - 1] + rightCumulativeSum[i - 1];
	}

	/**************************
	 *		 PARTITION		  *
	 **************************/
	 // Init buffers for globalPartition kernel
	std::vector<int> out = in;
	cl::Buffer inBuf2(context, CL_MEM_READ_ONLY | CL_MEM_HOST_NO_ACCESS | CL_MEM_COPY_HOST_PTR, sizeof(int) * ARRAY_SIZE, in.data(), &err);
	cl::Buffer auxBuf(context, CL_MEM_WRITE_ONLY | CL_MEM_HOST_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(int) * ARRAY_SIZE, out.data(), &err);
	cl::Buffer leftCumulativeSumBuf(context, CL_MEM_READ_ONLY | CL_MEM_HOST_NO_ACCESS | CL_MEM_COPY_HOST_PTR, sizeof(int) * groups + 1, leftCumulativeSum.data(), &err);
	cl::Buffer rightCumulativeSumBuf(context, CL_MEM_READ_ONLY | CL_MEM_HOST_NO_ACCESS | CL_MEM_COPY_HOST_PTR, sizeof(int) * groups + 1, rightCumulativeSum.data(), &err);

	// Init kernel and set args
	cl::Kernel kernel2(program, "partition", &err);
	err = kernel2.setArg(0, inBuf2);
	err = kernel2.setArg(1, auxBuf);
	err = kernel2.setArg(2, sizeof(int), (void *)&num_elements);
	err = kernel2.setArg(3, sizeof(int), (void *)&start);
	err = kernel2.setArg(4, sizeof(int), (void *)&elements_per_group);
	err = kernel2.setArg(5, sizeof(int), (void *)&pivot);
	err = kernel2.setArg(6, leftCumulativeSumBuf);
	err = kernel2.setArg(7, rightCumulativeSumBuf);

	// Execute the kernel
	err = queue.enqueueNDRangeKernel(kernel2, cl::NullRange, cl::NDRange(groups));

	// Read output buffer
	err = queue.enqueueReadBuffer(auxBuf, CL_TRUE, 0, sizeof(int) * ARRAY_SIZE, out.data());

	cl::finish();


	// Fill gap with pivots
	int pivotIdxLeft = start + leftCumulativeSum[groups];
	int pivotIdxRight = start + num_elements - rightCumulativeSum[groups] - 1;
	for (int i = pivotIdxLeft; i <= pivotIdxRight; i++) {
		out[i] = pivot;
	}

	// Copy back to in
	int outCount = 0;
	//for (int i = start; i <= end; i++) {
	//	in[i] = out[outCount++];
	//}
	in = out;

	//pivotIdxLeft += start;
	//pivotIdxRight += start;
	// Recursive call on sub-arrays - choose smallest first to limit recursion tree to log2n size
	if (pivotIdxLeft - 1 - start <= end - (pivotIdxRight + 1)) {
		opencl_quicksort(in, start, pivotIdxLeft - 1);
		opencl_quicksort(in, pivotIdxRight + 1, end);
	}
	else {
		opencl_quicksort(in, pivotIdxRight + 1, end);
		opencl_quicksort(in, start, pivotIdxLeft - 1);
	}

	return;
}

void insertionsort(std::vector<int> &in, int start, int end) {
	int i, key, j;
	for (i = start + 1; i <= end; i++)
	{
		key = in[i];
		j = i - 1;

		while (j >= 0 && in[j] > key)
		{
			in[j + 1] = in[j];
			j--;
		}
		in[j + 1] = key;
	}
}

cl::Program CreateProgram(const char* filename) {
	// Get the platform
	std::vector<cl::Platform> platforms;
	cl::Platform::get(&platforms);
	cl::Platform platform = platforms.front();

	// Get the device
	std::vector<cl::Device> devices;
	platform.getDevices(CL_DEVICE_TYPE_GPU, &devices);
	cl::Device device = devices.front();

	// Read in the kernel file (.cl)
	std::ifstream kernelFile(filename);
	std::string src(std::istreambuf_iterator<char>(kernelFile), (std::istreambuf_iterator<char>()));

	// Get the sources
	cl::Program::Sources sources(1, std::make_pair(src.c_str(), src.length() + 1));

	// Create the context
	cl::Context context(device, nullptr, nullptr, nullptr, &err);

	// Create the program
	cl::Program program(context, sources, &err);

	// Build the program
	err = program.build();

	cl::STRING_CLASS log = program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device);
	return program;
}