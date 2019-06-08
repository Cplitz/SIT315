#pragma once
#include "Vector3.h"
#include "NBodySimulation.h"
#include "Octree.h"



// Simulation class
class BarnesHutNBody : public NBodySimulation {

protected:
	Vector3 fbl;
	Vector3 ntr;
	int max_depth;

	Octree * BHTree;

	// Calculate accelerations for each body
	virtual void computeAccelerations();


public:

	// Constructor
	BarnesHutNBody(Vector3 fbl, Vector3 ntr, int max_depth, int type, int nb);
};

