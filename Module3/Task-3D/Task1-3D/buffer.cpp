#include "pch.h"
#include "buffer.h"


Buffer::Buffer(int size)
{
	// Initialise class members
	next_in = 0;
	next_out = 0;
	_size = size;
	_emptySlots = _size;
	endOfData = false;

	// Set buffer values to empty slots
	for (int i = 0; i < size; i++) {
		_values.push_back(EMPTY_TRAFFIC);
	}
}

Buffer::~Buffer()
{
}


bool Buffer::isEmpty() {
	return _emptySlots == _size;
}

bool Buffer::isFull() {
	return _emptySlots == 0;
}

void Buffer::produce(TrafficData data) {
	// Lock the critical section while the data is added to the buffer
	std::unique_lock<std::mutex> lck(mtx);

	// If the buffer is already full, unlock and wait
	while (isFull())
		cv.wait(lck);

	// Critical section is locked again by the conditional variable and data is added to the buffer
	push_next(data);

	// Recalcualte empty slots
	calculateEmptySlots();

	// Unlock the crtical section and notify other threads
	cv.notify_all();
}

TrafficData Buffer::consume() {
	// Lock the critical section while the data is removed from the buffer
	std::unique_lock<std::mutex> lck(mtx);

	// If the buffer has no data and there is still more data to come, unlock and wait for more data
	while (isEmpty() && !end())
		cv.wait(lck);

	// To break from the consumer thread, the thread needs to know if the buffer is empty and no more data is coming.
	// In the event that consume() was entered before the buffer could be exited and there was no data in the buffer, the conditional variable would wait forever unless notified by the exit() method
	if (isEmpty() && end())
		return EMPTY_TRAFFIC;

	// Remove the next object from the buffer
	TrafficData data = pop_next();

	// Recalculate empty slots
	calculateEmptySlots();

	// Unlock the crtical section and notify other threads
	cv.notify_all();

	// Return the data
	return data;
}


void Buffer::push_next(TrafficData data) {
	// Creates a circular effect, as data reaches the end of the bounded buffer, new data will be re-entered at the start ONLY IF there is available space
	if (next_in >= _size) {
		next_in = 0;
	}

	// Incrememt the next_in variable after assiging the buffer index
	_values[next_in++] = data;	
}

TrafficData Buffer::pop_next() {
	TrafficData data;

	if (next_out >= _size) {
		next_out = 0;
	}

	// Replace the buffer slot with an empty slot and incrememnt the next_out variable
	data = _values[next_out];
	_values[next_out++] = EMPTY_TRAFFIC;
	
	// Return the consumed data
	return data;
}

void Buffer::calculateEmptySlots() {
	int count = 0;
	for (int i = 0; i < _size; i++) {
		if (_values[i] == EMPTY_TRAFFIC) {
			count++;
		}
	}

	_emptySlots = count;
}

void Buffer::exit() {
	endOfData = true;

	// IMPORTANT: Without this, the consumer threads will enter an infinite wait loop as endOfData will never equate to true as they have locked the critical section.
	cv.notify_all();
}

bool Buffer::end() {
	return endOfData;
}


inline bool TrafficData::operator==(const TrafficData & rhs) const
{
	// If all fields match then return true
	return	timestamp == rhs.timestamp &&
			light_id == rhs.light_id &&
			cars == rhs.cars;
}

/*
 * Retrieves the hour as an integer of the unix timestamp
 */
int TrafficData::getHour() {
	time_t t = timestamp;
	struct tm tm;
	gmtime_s(&tm, &t);
	char date[3];
	strftime(date, sizeof(date), "%H", &tm);

	return std::stoi(date);
}


