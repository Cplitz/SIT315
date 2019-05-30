#include <map>
#include <vector>
#include "buffer.h"

#pragma once


struct TrafficCongestion {
	// Map containing the congestion data for each hour in the format [hour => [light_id => [measurement1, measurement2, measurement3, ...], [light_id => ....], ...]
	// Example structure:
	// [6 =>	[1 => [832,400,244,...],
	//			[2 => [722,100,942,...],
	//			[...],
	// [7 =>	[1 => [832,400,244,...],
	//			[2 => [722,100,942,...],
	//			[...],
	// [...],
	std::map<int, std::map<int, std::vector<int>>> data;	
	int num_lights;

	int sum(int hour);															// Calcualtes the sum of all traffic within a given hour across all traffic lights
	int sum(int hour, int light_id);											// Calulates the sum of all traffic within a given hour across a given traffic light id
	int avg(int hour);															// Calculates the average congestion of all traffic lights in a given hour
	std::vector<std::pair<int, int>> getTotals(int hour);						// Retrieves a vector containing a list of car totals for a given hour for each traffic light
};
