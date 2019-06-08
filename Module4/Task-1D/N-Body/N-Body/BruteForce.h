#pragma once
#include <vector>
#include "Vector3.h"
#include "NBodySimulation.h"

// Simulation class
class BruteForceNBody : public NBodySimulation {

	// Calculate accelerations for each body
	void computeAccelerations();

public:

	// Constructor
	BruteForceNBody(int type, int nb = 3) : NBodySimulation(type, nb) {};
};