#include "pch.h"
#include <iostream>
#include <chrono>
#include <vector>

#include "Vector3.h"
#include "BruteForce.h"
#include "BarnesHutSeq.h"
#include "BarnesHutParallel.h"
#include "SimulationUI.h"
							

/** MAIN **/
int main(int argc, char *argv[]) {
	int num_bodies, num_steps;
	// Get simulation choice
	int simulation_choice;
	std::cout
		<< "SELECT SIMULATION" << std::endl
		<< "0: User choice" << std::endl
		<< "1: Figure 8 simulation" << std::endl
		<< "2: Figure 8 heavy simulation" << std::endl
		<< "3: Orbit simulation" << std::endl
		<< "4: Planet/Sun/Moon simulation" << std::endl;
	std::cin >> simulation_choice;

	// Get other required data (bodies if required, and steps)
	if (simulation_choice == 0 || simulation_choice == 3) {
		std::cout << "Number of bodies: ";
		std::cin >> num_bodies;
	}
	std::cout << "Number of steps: ";
	std::cin >> num_steps;

	// Choose simulation method
	int simulation_method_choice;
	std::cout
		<< "SELECT SIMULATION ALGORITHM" << std::endl
		<< "0: Brute Force" << std::endl
		<< "1: Barnes Hut Octree (Sequential)" << std::endl
		<< "2: Barnes Hut Octree (Parallel)" << std::endl;
	std::cin >> simulation_method_choice;

	// Run simulation data
	SimulationUI ui;
	int exec_start = clock();
	switch (simulation_method_choice) {

		// Brute Force
		case 0:
		{
			BruteForceNBody nb(simulation_choice, num_bodies);
			for (int i = 0; i < num_steps; i++) {
				nb.simulate();
				ui.addFrame(nb.getPositions());
				std::cout << "Generating Step: " + std::to_string(i) + std::string("/") + std::to_string(num_steps) << std::endl;
			}
			ui.num_bodies = nb.getBodies();
			break;
		}

		// Barnes Hut Sequential
		case 1: 
		{
			BarnesHutNBody nb({ -1500, -1500, -1500 }, { 1500, 1500, 1500 }, 20, num_bodies, simulation_choice);
			for (int i = 0; i < num_steps; i++) {
				nb.simulate();
				ui.addFrame(nb.getPositions());
				std::cout << "Generating Step: " + std::to_string(i) + std::string("/") + std::to_string(num_steps) << std::endl;
			}
			ui.num_bodies = nb.getBodies();
			break;
		}

		// Barnes Hut Parallel
		case 2: 
		{
			BarnesHutNBodyParallel nb({ -1500, -1500, -1500 }, { 1500, 1500, 1500 }, 1000, num_bodies, simulation_choice);
			for (int i = 0; i < num_steps; i++) {
				nb.simulate();
				ui.addFrame(nb.getPositions());
				std::cout << "Generating Step: " + std::to_string(i) + std::string("/") + std::to_string(num_steps) << std::endl;
			}
			ui.num_bodies = nb.getBodies();
			break;
		}

		// Default (Brute Force)
		default:
		{
			BruteForceNBody nb(simulation_choice, num_bodies); 
			for (int i = 0; i < num_steps; i++) {
				nb.simulate();
				ui.addFrame(nb.getPositions());
				std::cout << "Generating Step: " + std::to_string(i) + std::string("/") + std::to_string(num_steps) << std::endl;
			}
			ui.num_bodies = nb.getBodies();
			break;
		}
	}		
	int exec_end = clock();
	std::cout << "I took " << exec_end - exec_start << "ms" << std::endl;

	// Init random colours for bodies
	for (int i = 0; i < ui.num_bodies; i++) {
		ui.colors.push_back(sf::Color(rand() % 255, rand() % 255, rand() % 255));
	}


	// Start playback
	std::cout << "Running playback!" << std::endl << "Press ***ESC*** in the playback window to show playback controls" << std::endl;
	ui.RunSimulation();

	
	
	return 0;
}








