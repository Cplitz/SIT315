// Task1-3D.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>
#include <time.h>
#include <mpi.h>
#include <queue>
#include "buffer.h"
#include "congestion.h"
#include <thread>
#include <chrono>
#include <fstream>
#include <string>
#include <map>
#include <iomanip>
#include <cstdio>


#define BUFFER_SIZE 10			// Number of elements that can be stored at once in the bounded buffer in each process
#define CONSUMER_THREADS 2		// Number of consumer threads to spawn in each process
#define PRODUCER_THREADS 2		// Number of producer threads to spawn in each process
#define NUM_TRAFFIC_LIGHTS 100	// Number of traffic lights when creating random data
#define MAX_CARS 300			// Maximum number of cars per interval when creating random data
#define TOP_CONGESTED 5			// Top N number of traffic lights to display each hour in the reports

#define CHARS_PER_LINE 24		// Number of chars in each line of the input file
#define FILE_HEADER_SIZE 24		// Number of chars in the header of the file that we don't want in the producer threads (Traffic light information)

// Forward declarations
TrafficData getTrafficData(char*);
void readTrafficData(int);
void processTrafficData(int);
void generateRandomTrafficData();

// Globals
int np, rank;

Buffer _buffer(BUFFER_SIZE);		// The bounded buffer on each process
TrafficCongestion congestion;		// The congestion data that is updated by the consumer threads
std::mutex mtx;						// The mutex variable
std::condition_variable cv;			// The conditional variable that handles automatic locks and unlocks during wait times

MPI_File infile;					// The MPI_File object used for creating file views and reading the input data
MPI_Offset filesize;				// The size of the input file
int bufsize, lines;					// The buffer size in each process and the number of lines in the file
char * filebuf;						// The file read buffer that reads in each processes independent portion of the input file using the bufsize

/* PROGRAM START */
int main()
{
	int exec_start, exec_end;

	// Initialise MPI 
	MPI_Init(NULL, NULL);

	// Number of process/nodes
	MPI_Comm_size(MPI_COMM_WORLD, &np);

	// Retrieve rank of this process/node
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	if (rank == 0) {
		/* OPTIONAL: Generate random data file */
		//generateRandomTrafficData();	

		exec_start = clock();
	}
	MPI_Barrier(MPI_COMM_WORLD); // ensure the data is generated before other processes access it

	// Open the file as read-only using MPI file IO
	MPI_File_open(MPI_COMM_WORLD, "../Cloud/input.txt", MPI_MODE_RDONLY, MPI_INFO_NULL, &infile);
	
	// Get the size of the file and the number of lines
	MPI_File_get_size(infile, &filesize);	
	filesize = filesize / sizeof(char);
	lines = (filesize / CHARS_PER_LINE) - 1;

	// Calculate the general buffer size for each process
	bufsize = (lines / np) * CHARS_PER_LINE;

	// Setup the elementary type and filetype for MPI_File_set_view (contiguous blocks of CHARS_PER_LINE characters = one line in the file)
	MPI_Datatype etype, filetype, contig;
	MPI_Type_contiguous(CHARS_PER_LINE, MPI_CHAR, &contig);
	MPI_Type_create_resized(contig, 0, CHARS_PER_LINE * sizeof(char), &filetype);
	MPI_Type_commit(&filetype);
	etype = MPI_CHAR;

	
	// Set the file view for each process using a displacement of fileheader + rank * bufsize (each process will receive a chunk of the file but not the file header)
	int fileHeader = sizeof(char) * FILE_HEADER_SIZE;
	MPI_File_set_view(infile, fileHeader + (rank * bufsize), etype, filetype, "native", MPI_INFO_NULL);

	// Allocate leftover lines to final process by increasing its buffer
	if (lines % np > 0 && rank == np - 1) {
		bufsize += CHARS_PER_LINE * (lines % np);
	}
	
	// Initialise the filebuf and read in data from file handle
	filebuf = new char[bufsize];
	MPI_File_read_all(infile, filebuf, bufsize, MPI_CHAR, MPI_STATUS_IGNORE);

	// Get the number of lights from the file header to pass to the TrafficCongestion struct
	char * headerbuf = new char[FILE_HEADER_SIZE];
	MPI_Type_contiguous(FILE_HEADER_SIZE, MPI_CHAR, &contig);
	MPI_Type_create_resized(contig, 0, FILE_HEADER_SIZE * sizeof(char), &filetype);
	MPI_Type_commit(&filetype);
	MPI_File_set_view(infile, 0, etype, filetype, "native", MPI_INFO_NULL);
	MPI_File_read_all(infile, headerbuf, FILE_HEADER_SIZE, MPI_CHAR, MPI_STATUS_IGNORE);
	std::string num_lights_str = headerbuf;
	num_lights_str.erase(0, 16);

	congestion.num_lights = std::stoi(num_lights_str);
	
	// Close the file handle
	MPI_File_close(&infile);

	// Initialise producer and consumer thread arrays
	std::thread producerThreads[PRODUCER_THREADS];
	std::thread consumerThreads[CONSUMER_THREADS];

	// Spawn producer threads that read traffic data from filebuf and place in the bounded-buffer
	for (int i = 0; i < PRODUCER_THREADS; i++) {
		producerThreads[i] = std::thread(readTrafficData, i);
	}
	
	// Spawn consumer threads that process traffic data from the bounded-buffer
	for (int i = 0; i < CONSUMER_THREADS; i++) {
		consumerThreads[i] = std::thread(processTrafficData, i);
	}

	// Synchronize producer threads
	for (int i = 0; i < PRODUCER_THREADS; i++) {
		producerThreads[i].join();
	}
	
	// Exit the buffer signalling that no more data will be entering
	_buffer.exit();

	// Synchronize consumer threads
	for (int i = 0; i < CONSUMER_THREADS; i++) {
		consumerThreads[i].join();
	}
	MPI_Barrier(MPI_COMM_WORLD);

	// Gather congestion data produced by processes (iterate across each hour and light id
	TrafficCongestion congestionMerged;	
	congestionMerged.num_lights = congestion.num_lights;
	for (int hour = 0; hour < 24; hour++) {
		for (int light_id = 1; light_id < congestion.num_lights; light_id++) {
			// Retrieve the vector of measurements for this hour and light id
			std::vector<int> vec = congestion.data[hour][light_id];

			// Retrieve the size of each vector on the root process so we can use MPI_Gatherv to retrieve the contents
			int * rcvCounts = new int[np] { 0 };
			int size = vec.size();
			MPI_Gather(&size, 1, MPI_INT, rcvCounts, 1, MPI_INT, 0, MPI_COMM_WORLD);
			MPI_Barrier(MPI_COMM_WORLD);
		
			if (rank == 0) {

				// Calculate the total number of elements and the displacements
				int total = 0;
				int * displs = new int[np] { 0 };
				for (int i = 0; i < np; i++) {
					total += rcvCounts[i];

					for (int j = 0; j < i; j++) {
						displs[i] += rcvCounts[j];
					}
				}

				// Gather the vector data onto the root
				std::vector<int> rootVec(total);
				MPI_Gatherv(vec.data(), vec.size(), MPI_INT, rootVec.data(), rcvCounts, displs, MPI_INT, 0, MPI_COMM_WORLD);
				
				// Add the data to the auxillary object
				congestionMerged.data[hour][light_id] = rootVec;
			}
			else {
				MPI_Gatherv(vec.data(), vec.size(), MPI_INT, NULL, NULL, NULL, MPI_INT, 0, MPI_COMM_WORLD);
			}
			MPI_Barrier(MPI_COMM_WORLD);
			
		}
	}
	
	

	// REPORTING
	if (rank == 0) {
		int hour, avg;
		float percent;
		std::string percentStatus;

		// Iterate over each hour in the congestion map
		for (std::map<int, std::map<int, std::vector<int>>>::iterator it = congestionMerged.data.begin(); it != congestionMerged.data.end(); it++) {
			hour = it->first;

			// Get the sorted totals for each traffic light in the hour
			std::vector<std::pair<int, int>> totals = congestionMerged.getTotals(hour);

			// Get the average congestion for the hour
			avg = congestionMerged.avg(hour);

			if (avg > 0) {
				// Determine am or pm
				std::string ampm;
				if (it->first < 12) {
					if (hour == 0)
						hour = 12;
					ampm = "AM";
				}
				else {
					if (hour != 12)
						hour = hour % 12;
					ampm = "PM";
				}

				// Beign printing report to console
				std::cout << hour << ampm << " Report:" << std::endl;


				std::cout << "Average cars: " << avg << std::endl;

				// Print the top N most congested cars
				for (int i = 0; i < TOP_CONGESTED; i++) {

					// Calculate the percentage increase/decrease from the average congestion value
					percent = ((totals[i].first / (float)avg) - 1) * 100;
					if (percent >= 0) percentStatus = "increase";
					else percentStatus = "decrease";
					percent = abs(percent);

					// Print traffic light values
					std::cout << "TRAFFIC LIGHT " << totals[i].second << ": " << totals[i].first << " cars" << "(" << std::setprecision(3) << percent << "% " << percentStatus << " on Average cars this hour)" << std::endl;
				}
				std::cout << std::endl;
			}			
		}


		exec_end = clock();
		printf("\nTotal execution time: %dms", exec_end - exec_start);
	}
	
	
	MPI_Finalize();
	
}


/*
 * Producer function.
 *
 * This function is responsible for retrieving a data line from the input file and placing it into the buffer
 *
 * @param thread_id Useful for tracking the thread in debugging
 */
void readTrafficData(int thread_id) {
	
	TrafficData data;
	int processLines, threadLines, bufLine;

	// Calculate the lines allocated to this process 
	processLines = bufsize / CHARS_PER_LINE;

	// Calculate the lines allocated to this thread
	threadLines = processLines / PRODUCER_THREADS;
	if (processLines % PRODUCER_THREADS > 0 && thread_id == PRODUCER_THREADS - 1) {
		threadLines += processLines % PRODUCER_THREADS;
	}
	
	// Continue producing until the end of the input file is reached
	char * line = new char[CHARS_PER_LINE];
	for (int i = 0; i < threadLines; i++) {

		// Get the current buffer line index
		int bufLine = (thread_id * (processLines / PRODUCER_THREADS)) + i;
		
		// Copy the line in the buffer to the line char array
		for (int j = 0; j < CHARS_PER_LINE; j++) {
			line[j] = filebuf[j + (bufLine * CHARS_PER_LINE)];
		}
		
		// Retrieve TrafficData from the line
		data = getTrafficData(line);
		
		// Enter data into the buffer
		_buffer.produce(data);
		
		// Console debugging
		printf("Producer Thread (PROCESS: %i|THREAD: %i) - %d %d %d\n", rank, thread_id, data.timestamp, data.light_id, data.cars);
	}
	
	
	
}


/*
 * Consumer function.
 *
 * This function is responsible for retrieving a TrafficData object from the buffer and processing it
 *
 * @param thread_id Useful for tracking the thread in debugging
 */
void processTrafficData(int thread_id) {

	TrafficData data;

	while (true) {
		
		// Consume TrafficData removing it from the buffer
		data = _buffer.consume();

		// Once the producer threads are synchronised (no data left to produce) the buffer will exit, causing the consumers to be notified and return an EMPTY_TRAFFIC object.
		// This signals that no more consuming needs to take place.
		if (data == EMPTY_TRAFFIC)
			break;


		// Add congestion data to congestion struct object (This is a race condition - lock the critical section)
		std::unique_lock<std::mutex> lck(mtx);
		if (congestion.data.find(data.getHour()) == congestion.data.end()) {
			congestion.data[data.getHour()] = std::map<int, std::vector<int>>{};
		}

		if (congestion.data[data.getHour()].find(data.light_id) == congestion.data[data.getHour()].end()) {
			congestion.data[data.getHour()][data.light_id] = std::vector<int>{};
		}

		congestion.data[data.getHour()][data.light_id].push_back(data.cars);
		cv.notify_all();

		// Console Debugging
		printf("Consumer Thread (PROCESS: %i|THREAD: %i) - %d %d %d\n", rank, thread_id, data.timestamp, data.light_id, data.cars);
	}
}

/*
 * This function retrieves a line from infile and attempts to convert it into a TrafficData object
 *
 * @param infile A reference to the input filestream object
 *
 * @returns A TrafficData object containing data retrieved from the input filestream
 */
TrafficData getTrafficData(char* buf) {

	TrafficData data;
	std::string line(buf, CHARS_PER_LINE);

	// If the line is empty return EMPTY_TRAFFIC
	if (line == "")
		return EMPTY_TRAFFIC;
	
	// Tokenize the string using the " " delimiter to extract the TrafficData values	
	std::string token;
	std::string tokens[3];
	std::string delimiter = " ";
	int pos = 0;	
	int i = 0;
	while ((pos = line.find(delimiter)) != std::string::npos) {
		token = line.substr(0, pos);
		tokens[i++] = token;
		line.erase(0, pos + delimiter.length());
	}
	tokens[i] = line;
	

	// Assign the TrafficData values
	data.timestamp = std::stoi(tokens[0]);
	data.light_id = std::stoi(tokens[1]);
	data.cars = std::stoi(tokens[2]);

	return data;
}

void generateRandomTrafficData() {
	
	// Open/Create the file
	std::ofstream outFile;
	outFile.open("../Cloud/input.txt", std::fstream::out);

	// num_traffic_lights currently capped at 6 digits, can change this
	if (NUM_TRAFFIC_LIGHTS > 999999) {
		throw "Number of traffic lights exceeeds the maximum of 999999";
	}
	// max cars in a single measurement currently capped at 4 digits, can change this
	if (MAX_CARS > 9999) {
		throw "Number of cars exceeds the maximum size of 9999";
	}

	// Left pad the number of lights to make the file header consistent
	char num_lights[7];
	snprintf(num_lights, sizeof(num_lights), "%06d", NUM_TRAFFIC_LIGHTS);
	outFile << "TRAFFIC LIGHTS: " << num_lights << std::endl;

	int i = 0;
	int timestamp = 1552543200;

	// Create random seed
	srand(time(NULL));

	// Generate random data for 8 hours
	while (i < 8 * 12) {
		for (int light_id = 1; light_id <= NUM_TRAFFIC_LIGHTS; light_id++) {
			TrafficData data = { timestamp, light_id, rand() % MAX_CARS };

			// Left pad the cars and light id integers with 0s to make the line size consistent
			char cars[5];
			char lightId[7];
			snprintf(cars, sizeof(cars), "%04d", data.cars);
			snprintf(lightId, sizeof(lightId), "%06d", data.light_id);

			// Write to file
			outFile << data.timestamp << " " << lightId << " " << cars << std::endl;
		}

		timestamp += 300;
		i++;
	}
	outFile.close();
}