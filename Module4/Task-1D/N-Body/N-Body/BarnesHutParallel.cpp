#include "pch.h"
#include "BarnesHutParallel.h"
#include <omp.h>

// Constructor
BarnesHutNBodyParallel::BarnesHutNBodyParallel(Vector3 fbl, Vector3 ntr, int max_depth, int nb, int type) : BarnesHutNBody(fbl, ntr, max_depth, nb, type) {}

// Calculate accelerations for each body
void BarnesHutNBodyParallel::computeAccelerations() {
	BHTree = &Octree(fbl, ntr, max_depth);

	// Can't compute this easily in parallel (at least not with OpenMP) without using the Least Essential Tree (LET) method - very complex
	for (int i = 0; i < bodies; i++) {
		BHTree->add({ positions[i], masses[i] });
	}
	BHTree->calculateCenterOfMass();

	// Parallelise the acceleration calculation - each calc is independent of one another and the tree state does not change (each element already has full access to the tree)
	#pragma omp parallel for num_threads(4)
	for (int i = 0; i < bodies; ++i) {
		// Don't update static bodies
		if (staticBodies[i] == true) continue;

		accelerations[i] = BHTree->calcTreeAccel(Element{ positions[i], masses[i] });

		// Soften large accelerations
		if (accelerations[i].mod() > SOFTENING_FACTOR)
			accelerations[i] = accelerations[i].unit() * SOFTENING_FACTOR;
	}
}
