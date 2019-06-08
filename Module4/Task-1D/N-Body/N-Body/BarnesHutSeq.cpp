#include "pch.h"
#include "BarnesHutSeq.h"


// Constructor
BarnesHutNBody::BarnesHutNBody(Vector3 fbl, Vector3 ntr, int max_depth, int nb, int type) : fbl(fbl), ntr(ntr), max_depth(max_depth), NBodySimulation(type, nb) {
	BHTree = &Octree(fbl, ntr, max_depth);
}

// Calculate accelerations for each body
void BarnesHutNBody::computeAccelerations() {
	BHTree = &Octree(fbl, ntr, max_depth);

	for (int i = 0; i < bodies; i++) {
		BHTree->add({ positions[i], masses[i] });
	}
	BHTree->calculateCenterOfMass();

	for (int i = 0; i < bodies; ++i) {
		// Don't update static bodies
		if (staticBodies[i] == true) continue;

		accelerations[i] = BHTree->calcTreeAccel(Element{ positions[i], masses[i] });

		// Soften large accelerations
		if (accelerations[i].mod() > SOFTENING_FACTOR)
			accelerations[i] = accelerations[i].unit() * SOFTENING_FACTOR;
	}
}



