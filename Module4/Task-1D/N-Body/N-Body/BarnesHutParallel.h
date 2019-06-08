#pragma once
#include "Vector3.h"
#include <vector>
#include "BarnesHutSeq.h"


// Simulation class
class BarnesHutNBodyParallel : public BarnesHutNBody {

private:

	// Calculate accelerations for each body
	void computeAccelerations();

public:

	// Constructor
	BarnesHutNBodyParallel(Vector3 fbl, Vector3 ntr, int max_depth, int type, int nb);
};

