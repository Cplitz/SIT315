// Task1-3D.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>
#include <time.h>
#include <queue>
#include "buffer.h"
#include "congestion.h"
#include <thread>
#include <chrono>
#include <time.h>
#include <fstream>
#include <string>
#include <map>
#include <iomanip>


#define BUFFER_SIZE 10			// Number of elements that can be stored at once in the bounded buffer
#define CONSUMER_THREADS 2		// Number of consumer threads to spawn
#define PRODUCER_THREADS 2		// Number of producer threads to spawn
#define PRODUCER_THREAD_WAIT 1	// Milliseconds to sleep between each production of data
#define CONSUMER_THREAD_WAIT 1	// Milliseconds to sleep between each consumption of data
#define NUM_TRAFFIC_LIGHTS 100	// Number of traffic lights when creating random data
#define MAX_CARS 300			// Maximum number of cars per interval when creating random data
#define TOP_CONGESTED 5			// Top N number of traffic lights to display each hour in the reports

// Forward declarations
TrafficData getNextTrafficData(std::ifstream&);
void getTrafficData(int);
void processTrafficData(int);

// Globals
Buffer _buffer(BUFFER_SIZE);
std::ifstream infile;
TrafficCongestion congestion;

/* PROGRAM START */
int main()
{


	/* OPTIONAL: Generate random data file */
	/*std::ofstream outFile;
	outFile.open("C:\\Users\\Cameron\\Desktop\\OneDrive\\UNI\\2019-Sem1\\ProgrammingParadigms\\GitHub\\Module2\\Task1-3D\\input.txt");

	int i = 0;
	int timestamp = 1552543200;

	// Create random seed
	srand(time(NULL));

	// Generate random data for 8 hours
	while (i < 8*12) {
		for (int light_id = 1; light_id <= NUM_TRAFFIC_LIGHTS; light_id++) {
			TrafficData data = { timestamp, light_id, rand() % MAX_CARS };
			outFile << data.timestamp << " " << data.light_id << " " << data.cars << std::endl;
		}

		timestamp += 300;
		i++;
	}
	outFile.close();*/
	

	// Open the input file for reading
	infile.open("C:\\Users\\Cameron\\Desktop\\OneDrive\\UNI\\2019-Sem1\\ProgrammingParadigms\\GitHub\\Module2\\Task1-3D\\input.txt");

	// Initialise producer and consumer thread arrays
	std::thread producerThreads[PRODUCER_THREADS];
	std::thread consumerThreads[CONSUMER_THREADS];

	// Spawn producer threads that get traffic data from input file and place in the buffer
	for (int i = 0; i < PRODUCER_THREADS; i++) {
		producerThreads[i] = std::thread(getTrafficData, i);
	}

	// Spawn consumer threads that process traffic data from the buffer
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

	// Close the input file
	infile.close();

	std::cout << std::endl;
	std::cout << std::endl;

	/* REPORTING */
	int hour, avg;
	float percent;
	std::string percentStatus;

	// Iterate over each hour in the congestion map
	for (std::map<int, std::map<int, std::vector<TrafficData>>>::iterator it = congestion.data.begin(); it != congestion.data.end(); it++) {
		hour = it->first;

		// Get the sorted totals for each traffic light in the hour
		std::vector<std::pair<int, int>> totals = congestion.getTotals(hour);

		// Get the average congestion for the hour
		avg = congestion.avg(hour);

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


/*
 * Producer function.
 *
 * This function is responsible for retrieving a data line from the input file and placing it into the buffer
 *
 * @param thread_id Useful for tracking the thread in debugging
 */
void getTrafficData(int thread_id) {
	std::string line;
	TrafficData data;

	// Continue producing until the end of the input file is reached
	while (!infile.eof()) {

		// Retrieve next data line. 
		//	NOTE: No mutex lock needed here as the file contents are not being modified
		data = getNextTrafficData(infile);

		// If we reached a blank line continue
		if (data == EMPTY_TRAFFIC)
			continue;

		// Enter data into the buffer
		_buffer.produce(data);
	
		// Console debugging
		printf("Producer Thread %d: %d %d %d\n", thread_id, data.timestamp, data.light_id, data.cars);

		/* OPTIONAL: Alter the sleep value to observe different buffer effects */
		std::this_thread::sleep_for(std::chrono::milliseconds(PRODUCER_THREAD_WAIT));
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

	while (true) {
		
		// Consume TrafficData removing it from the buffer
		TrafficData data = _buffer.consume();

		// Once the producer threads are synchronised (no data left to produce) the buffer will exit, causing the consumers to be notified and return an EMPTY_TRAFFIC object.
		// This signals that no more consuming needs to take place.
		if (data == EMPTY_TRAFFIC)
			break;


		// Add congestion data to congestion struct object
		if (congestion.data.find(data.getHour()) == congestion.data.end()) {
			congestion.data[data.getHour()] = std::map<int, std::vector<TrafficData>>{};
		}

		if (congestion.data[data.getHour()].find(data.light_id) == congestion.data[data.getHour()].end()) {
			congestion.data[data.getHour()][data.light_id] = std::vector<TrafficData>{};
		}

		congestion.data[data.getHour()][data.light_id].push_back(data);

		// Console Debugging
		printf("Consumer Thread %d: %d %d %d\n", thread_id, data.timestamp, data.light_id, data.cars);

		/* OPTIONAL: Alter the sleep value to observe different buffer effects */
		std::this_thread::sleep_for(std::chrono::milliseconds(PRODUCER_THREAD_WAIT));
	}
}

/*
 * This function retrieves a line from infile and attempts to convert it into a TrafficData object
 *
 * @param infile A reference to the input filestream object
 *
 * @returns A TrafficData object containing data retrieved from the input filestream
 */
TrafficData getNextTrafficData(std::ifstream& infile) {

	TrafficData data;
	std::string line;

	// Get the next line from the file
	std::getline(infile, line);

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