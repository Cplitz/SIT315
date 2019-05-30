#include "pch.h"
#include "congestion.h"


int TrafficCongestion::sum(int hour, int light_id) {
	int sum = 0;

	if (data.find(hour) == data.end()) {
		return 0;
	}

	if (data[hour].find(light_id) == data[hour].end()) {
		return 0;
	}

	for (int i = 0; i < data[hour][light_id].size(); i++) {
		sum += data[hour][light_id][i];
	}

	return sum;
}


int TrafficCongestion::sum(int hour) {
	int sum = 0;

	for (std::map<int, std::vector<int>>::iterator it = data[hour].begin(); it != data[hour].end(); it++) {
		for (int i = 0; i < data[hour][it->first].size(); i++) {
			sum += data[hour][it->first][i];
		}
	}

	return sum;
}

int TrafficCongestion::avg(int hour) {
	float num_cars = sum(hour);

	int num_hours = data[hour].size();

	return num_cars / num_hours;
}

std::vector<std::pair<int, int>> TrafficCongestion::getTotals(int hour) {


	std::vector<std::pair<int, int>> totals;

	for (int j = 1; j <= num_lights; j++) {
		totals.push_back(std::pair<int, int>{ sum(hour, j), j });
	}

	std::sort(totals.rbegin(), totals.rend());

	return totals;
}