#pragma once
#include <queue>
#include <mutex>
#include <condition_variable>
#include <map>
#include <string>

#define EMPTY_TRAFFIC TrafficData {-1, -1, -1}	// Represents a free slot in the buffer

/*
 * TrafficData struct.
 *
 * This struct holds important information about the traffic data as well as some useful operations.
 */
struct TrafficData {
	int timestamp;
	int light_id;	
	int cars;

	inline bool operator==(const TrafficData& rhs) const;
	int getHour();
};




/*
 * Buffer class.
 *
 * The Buffer class is custom-made to keep track of producers inputting data and consumers retrieving data from a bounded-buffer.
 * The class is responsible for using mutex locks to ensure thread safety during produce and consume operations.
 */
class Buffer
{
private:
	int _size;							// The number of elements the buffer can store at one time
	std::vector<TrafficData> _values;	// A vector of TrafficData objects containing the elements within the buffer
	int _emptySlots;					// The number of empty slots (represented by EMPTY_TRAFFIC)

	int next_in;						// The next slot available for production
	int next_out;						// The next slot ready to be consumed

	bool endOfData;						// Signals that there is no more data to accept into the buffer

	std::mutex mtx;						// The mutex variable
	std::condition_variable cv;			// The conditional variable that handles automatic locks and unlocks during wait times

	void push_next(TrafficData data);	// Adds a TrafficData object into the active buffer region
	TrafficData pop_next();				// Removes a TrafficData object from the end of the active buffer region

	void calculateEmptySlots();			// Calculates the amount of empty slots updating _emptySlots so that isEmpty() and isFull() are O(1) operations

public:
	Buffer(int size);					// Constructor taking a buffer size integer
	~Buffer();							// Destructor
		
	void produce(TrafficData data);		// Produces a buffer slot
	TrafficData consume();				// Consumes a buffer slot and returns its value

	bool isEmpty();						// Returns true when the buffer is empty (all EMPTY_TRAFFIC slots)
	bool isFull();						// Returns true when the buffer is full (no EMPTY_TRAFFIC slots)

	void exit();						// Signals that there is no more data to accept into the buffer and notifies via the conditional variable
	bool end();							// Returns endOfData
};

