#include "pch.h"
#include "BruteForce.h"
#include "Vector3.h"


// Calculate accelerations for each body
void BruteForceNBody::computeAccelerations() {
	for (int i = 0; i < bodies; ++i) {
		// Don't update static bodies
		if (staticBodies[i] == true) continue;

		accelerations[i] = ZERO;
		for (int j = 0; j < bodies; ++j) {
			if (i != j) {
				double temp = GC * masses[j] / pow((positions[i] - positions[j]).mod(), 3);
				accelerations[i] = accelerations[i] + (positions[j] - positions[i]) * temp;

				// Soften large accelerations
				if (accelerations[i].mod() > SOFTENING_FACTOR)
					accelerations[i] = accelerations[i].unit() * SOFTENING_FACTOR;
			}
		}
	}
}
