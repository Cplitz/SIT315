#pragma once
#include <vector>
#include <map>
#include <SFML/Graphics.hpp>
#include "Vector3.h"
#include <cmath>
#include <omp.h>
#include <chrono>
#include <thread>

class SimulationUI {
	sf::RenderWindow window;										// The window to render in
	sf::View view; 													// The main view (used for zooming, moving, etc)
	sf::View uiView;												// The UI View (used for overlay)
	sf::View controlsView;											// The controls view (used for displaying the controls)
	sf::Texture controlsUI;											// Stores the controls texture - loaded in main()

	std::vector<std::vector<Vector3>> frameData;					// Holds the positional data per frame of each body in the simulation
	int frame;														// The current frame during playback
	int framerate = 60;												// The current framerate during playback
	bool centerView = true;											// Boolean indicating if the camera is locked on the 0th body
	bool paused = false;											// Boolean indicating if the playback is paused
	bool lastPausedState = false;									// Boolean holding the last paused state (used to switch between controls menu and restore state)
	bool showControls = false;										// Boolean indicating if the controls menu should be displayed

	bool showTrackUI = false;										// Boolean indicating if the body tracking UI should be displayed
	std::string trackTextEntered;									// String containing the entered text in the tracking UI
	bool trackingBody = false;										// Boolean indicating if we are currently tracking a body or not
	int trackBodyNum;												// Integer indicating the index of the body we are tracking
	std::vector<int> trackFrames;									// List of frame numbers that are included in the body tracking (for drawing)

	std::map<float, std::pair<const char*, int>> CalcShapeSizes(int);// Calculates and orders shape sizes based on z-value
	void DrawOverlay(int);											// Responsible for drawing UI overlay elements dependent on state

public:		
	int num_bodies;													// Number of bodies
	std::vector<sf::Color> colors;									// Holds the shape color data

	// Constructor
	SimulationUI();

	// Run the simulation
	void RunSimulation();

	// Draws the simulation
	void DrawSimulation(int);						

	// Handles all keyboard input during playback
	void HandleKeyboardInput(sf::Keyboard::Key);					

	// Adds a new frame with position data
	void addFrame(std::vector<Vector3>);
};
