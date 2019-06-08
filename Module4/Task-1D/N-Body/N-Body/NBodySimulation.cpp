#include "pch.h"
#include "NBodySimulation.h"
#include <omp.h>

// Constructor
NBodySimulation::NBodySimulation(int type, int nb) {
	// Determine number of bodies based on simulation type
	switch (type) {
	case 0:
		if (nb < 2)
			throw "Need at least 2 bodies to perform simulation";
		break;
	case 1:
		nb = 4; break;
	case 2:
		nb = 4; break;
	case 3:
		if (nb < 2)
			throw "Need at least 2 bodies to perform simulation";
		break;
	case 4:
		nb = 3; break;

	}
	bodies = nb;

	// Resize vectors
	masses.resize(bodies);
	positions.resize(bodies);
	fill(positions.begin(), positions.end(), ZERO);
	velocities.resize(bodies);
	fill(velocities.begin(), velocities.end(), ZERO);
	accelerations.resize(bodies);
	fill(accelerations.begin(), accelerations.end(), ZERO);
	staticBodies.resize(bodies);
	fill(staticBodies.begin(), staticBodies.end(), false);

	// Determine simulation based on type
	switch (type) {
	case 0:
		initRandomBodies(); break;
	case 1:
		initFigure8(); break;
	case 2:
		initFigure8(10000); break;
	case 3:
		initOrbit(); break;
	case 4:
		initPlanetSunMoon(); break;
	}
}

// Get positions
std::vector<Vector3> NBodySimulation::getPositions() {
	return positions;
}

// Get bodies
int NBodySimulation::getBodies() {
	return bodies;
}

// Simulate
void NBodySimulation::simulate() {
	computeAccelerations();
	computePositions();
	computeVelocities();
}

// User simulation with random bodies (random masses, positions, velocities)
void NBodySimulation::initRandomBodies() {
	// Init static centre body
	masses[0] = CENTRE_MASS;
	positions[0] = { 0.001, 0.001, 0.001 };
	velocities[0] = ZERO;
	staticBodies[0] = true;

	// Init other bodies with random values
	for (int i = 1; i < bodies; i++) {
		masses[i] = randrange(MIN_BODY_MASS, MAX_BODY_MASS, 1);
		positions[i] = Vector3(randrange(-250, 250, 1), randrange(-250, 250, 1), randrange(-100, 100, 1));
		velocities[i] = Vector3(randrange(-0.5, 0.5, 4), randrange(-0.5, 0.5, 4), randrange(-0.5, 0.5, 4));
	}
}

// Simple Figure 8 simulation with slight (or large) interferences from moving figure 8 bodies
void NBodySimulation::initFigure8(float movingMasses) {
	// Init first immovable body
	masses[0] = 10000;
	positions[0] = Vector3(-100, 0, 0.01);
	velocities[0] = ZERO;
	staticBodies[0] = true;

	// Init second immovable body
	masses[1] = 10000;
	positions[1] = Vector3(100, 0, 0);
	velocities[1] = ZERO;
	staticBodies[1] = true;

	// Init first moving body (orbits in x-y plane)
	masses[2] = movingMasses;
	positions[2] = Vector3(-200, 0, -0.01);
	velocities[2] = Vector3(0, 1.75, 0);

	// Init second moving body (orbits in x-z plane)
	masses[3] = movingMasses;
	positions[3] = Vector3(200, 0, -0.001);
	velocities[3] = Vector3(0, 0, 1.85);
}

// Orbit simulation
void NBodySimulation::initOrbit() {
	// Init immovable center body
	masses[0] = 10000;
	positions[0] = { 0.001, 0.001, 0.001 };
	velocities[0] = ZERO;
	staticBodies[0] = true;

	// Position rest of bodies in a circular pattern around the center body and initial z-velocity
	float PI = 3.14159265;
	float r = 100;
	for (int i = 1; i < bodies; i++) {
		int degrees = 360.f / (bodies - 1) * (i - 1);
		masses[i] = randrange(0.1, 2, 2);
		positions[i] = Vector3(r * cos(degrees * PI / 180), r * sin(degrees * PI / 180), 0.0001 * i);
		velocities[i] = Vector3(0, 0, 1.5);
	}
}

// Basic nested orbiting simulation (sun simulated as immovable, planet orbits the sun, moon orbits the planet)
void NBodySimulation::initPlanetSunMoon() {
	// Sun
	masses[0] = 10000;
	positions[0] = { 0.001, 0.001, 0.001 };
	velocities[0] = ZERO;
	staticBodies[0] = true;

	// Planet
	masses[1] = 5000;
	positions[1] = Vector3(-800, 0, 0.0001);
	velocities[1] = Vector3(0, 1, 0);

	// Moon
	masses[2] = 10;
	positions[2] = Vector3(-800, 150, -0.0001);
	velocities[2] = Vector3(1.75, 0.5, 0);
}

// Helper function to return a random float with set precision ([0,1] with precision 3 can net 1000 possible values as opposed to rand() % 1 which will only give 1)
float NBodySimulation::randrange(float min, float max, int precision) {
	float multiplier = pow(10, precision);

	float newmin = min * multiplier;
	float newmax = max * multiplier;

	return (rand() % (int)(newmax - newmin)) / multiplier + min;

}

// Calculate velocities based on accelerations and previous velocity (v = u + at)
void NBodySimulation::computeVelocities() {
#pragma omp for parallel num_threads(32)
	for (int i = 0; i < bodies; ++i) {
		velocities[i] = velocities[i] + accelerations[i] * TIME_STEP;
	}
}

// Calculate positions based on velocities and accelerations (r = r0 + ut + 0.5 * at^2)
void NBodySimulation::computePositions() {
#pragma omp for parallel num_threads(32)
	for (int i = 0; i < bodies; ++i) {
		positions[i] = positions[i] + (velocities[i] * TIME_STEP) + accelerations[i] * 0.5 * pow(TIME_STEP, 2);
	}
}