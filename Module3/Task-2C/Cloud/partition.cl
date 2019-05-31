__kernel void computeCounts(__global int* arr,
	const int pivot,
	const int size,
	const int num_groups,
	const int elements_per_group,
	const int start,
	__global int* leftCumulative,
	__global int* rightCumulative) {

	// Thread IDs
	int l_id = get_local_id(0);
	int group_id = get_group_id(0);

	// Do not process further if the thread is outside the bounds of the last group (happens when the last group has less items to process)
	if (group_id == num_groups - 1 && size % elements_per_group > 0) {
		if (l_id >= size % elements_per_group) {
			return;
		}
	}

	// Get the element corresponsing to this thread
	int element = arr[start + (group_id * elements_per_group) + l_id];

	// Count elements that will be left of pivot and right of pivot
	if (element < pivot) {
		atomic_inc(&leftCumulative[group_id]);
	}
	else if (element > pivot) {
		atomic_inc(&rightCumulative[group_id]);
	}

}

__kernel void partition(const __global int* arr,
	__global int* outArr,
	const int size,
	const int start,
	const int elements_per_group,
	const int pivot,
	const __global int* leftCumulativeSum,
	const __global int* rightCumulativeSum) {

	// Global ID
	int g_id = get_global_id(0);

	// Get threadElements (adjust if this is the last group and the element count was not divisible by the number of groups)
	int threadElements = elements_per_group;
	if (g_id == get_global_size(0) - 1 && size % elements_per_group > 0) {
		threadElements = size % elements_per_group;
	}

	// Define thread start and end
	int threadStart = start + (g_id * elements_per_group);
	int threadEnd = threadStart + threadElements;

	// Define left and right write indexes
	int left = start + leftCumulativeSum[g_id];
	int right = start + size - rightCumulativeSum[g_id + 1];

	// For each element in the thread group, write to the correct position
	for (int i = threadStart; i < threadEnd; i++) {
		if (arr[i] < pivot) {
			outArr[left++] = arr[i];
		}
		else if (arr[i] > pivot) {
			outArr[right++] = arr[i];
		}
	}
}