#pragma once
#include <vector>
#include "Vector3.h"

// Simulation class
class NBodySimulation {

protected:
	const float MIN_BODY_MASS = 1.f;		// Minimum mass for a body (user simulation)
	const float MAX_BODY_MASS = 1000.f;		// Maximum mass for a body (user simulation)
	const float CENTRE_MASS = 10000.f;		// Maximum mass of the static centre body (user simulation)
	const float SOFTENING_FACTOR = 0.1f;	// Maximum acceleration vector size (softens acceleration values that are abrnomally high due to representing bodies as points)
	const float GC = 0.1;					// Gravitational Constant
	const float TIME_STEP = 1;				// Time between each step

	int bodies;								// Number of bodies to simulate
	std::vector<float> masses;				// List containing masses of all bodies
	std::vector<Vector3> positions;			// List containing position vectors of all bodies
	std::vector<Vector3> velocities;		// List containing velocity vectors of all bodies
	std::vector<Vector3> accelerations;		// List containing acceleration vectors of all bodies
	std::vector<bool> staticBodies;			// List indicating whether or not each body is static (immovable) or not

	// User simulation with random bodies (random masses, positions, velocities)
	void initRandomBodies();

	// Simple Figure 8 simulation with slight (or large) interferences from moving figure 8 bodies
	void initFigure8(float movingMasses = 1);

	// Orbit simulation
	void initOrbit();

	// Basic nested orbiting simulation (sun simulated as immovable, planet orbits the sun, moon orbits the planet)
	void initPlanetSunMoon();

	// Helper function to return a random float with set precision ([0,1] with precision 3 can net 1000 possible values as opposed to rand() % 1 which will only give 1)
	float randrange(float min, float max, int precision = 0);

	// Calculate accelerations for each body
	virtual void computeAccelerations() = 0;

	// Calculate velocities based on accelerations and previous velocity (v = u + at)
	void computeVelocities();

	// Calculate positions based on velocities and accelerations (r = r0 + ut + 0.5 * at^2)
	void computePositions();

public:

	// Constructor
	NBodySimulation(int type, int nb = 3);

	// Get positions
	std::vector<Vector3> getPositions();

	// Get bodies
	int getBodies();

	// Simulate
	void simulate();
};