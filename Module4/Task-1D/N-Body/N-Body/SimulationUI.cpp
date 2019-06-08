#include "pch.h"
#include "SimulationUI.h"

SimulationUI::SimulationUI() {
	view = sf::View(sf::FloatRect(0, 0, 500, 500));
	uiView = sf::View(sf::FloatRect(0.f, 0.f, 500.f, 500.f));
	controlsView = sf::View(sf::FloatRect(0.f, 0.f, 500.f, 500.f));
	controlsUI.loadFromFile("../img/controlsUI.png");
}

void SimulationUI::RunSimulation() {
	window.create(sf::VideoMode(500.f, 500.f), "N-Body Simulation Playback");
	window.requestFocus();

	frame = 0;
	while (window.isOpen())
	{
		// Process events
		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::KeyPressed)
				HandleKeyboardInput(event.key.code);

			if (event.type == sf::Event::MouseWheelScrolled)
				view.zoom(1 / (event.mouseWheelScroll.delta * 0.1 + 1));

			if (event.type == sf::Event::Closed)
				window.close();
		}


		// Draw Simulation
		window.clear();
		DrawSimulation(frame);
		window.display();

		// Stay on the same frame if paused
		if (paused) --frame;

		// Loop
		if (++frame >= frameData.size())
			frame = 0;

		// Framerate control
		std::this_thread::sleep_for(std::chrono::milliseconds(1000 / framerate));
	}
}


void SimulationUI::addFrame(std::vector<Vector3> frame) {
	frameData.push_back(frame);
}

std::map<float, std::pair<const char*, int>> SimulationUI::CalcShapeSizes(int frame) {
	// Calculate render sizes (a map is used here to automatically order the data according to each bodies z position, used for SFML overdrawing later)
#pragma omp parallel for
	std::map<float, std::pair<const char*, int>> sizes;
	for (int i = 0; i < num_bodies; i++) {
		// Calc size
		float size;
		if (frameData[frame][i].z < -100)
			size = 1.f;
		else if (frameData[frame][i].z > 400)
			size = 40.f;
		else
			size = (((frameData[frame][i].z + 100) * (15.f - 0.01f)) / 200) + 1.f;

		// Add to map
		sizes[size] = std::pair<const char*, int>("body", i);
	}

	// TRACKING: Add pixel trail using the same z-ordering technique
#pragma omp parallel for
	for (int i = 0; i < trackFrames.size(); i++) {
		Vector3 f = frameData[trackFrames[i]][trackBodyNum];
		sizes[f.z] = std::pair<const char*, int>("pixel", trackFrames[i]);
	}

	return sizes;
}

void SimulationUI::DrawSimulation(int frame) {
	std::map<float, std::pair<const char*, int>> sizes = CalcShapeSizes(frame);

	// Draw the background
	sf::Texture starsTexture;
	starsTexture.loadFromFile("../img/stars.png");
	starsTexture.setRepeated(true);
	window.draw(sf::Sprite(starsTexture, sf::IntRect(0, 0, 1000, 1000)));

	// z-ordering - SFML uses overdrawing rather than a depth vector, so smaller objects (i.e. further away) must be drawn first		
#pragma omp parallel for
	for (auto i = sizes.begin(); i != sizes.end(); i++) {
		float size = i->first;
		const char* type = i->second.first;

		// Draw trail pixels
		if (type == "pixel") {
			int frameNum = i->second.second;

			sf::RectangleShape pixel(sf::Vector2f(1, 1));
			pixel.setFillColor(sf::Color::Green);
			pixel.setPosition(sf::Vector2f(frameData[frameNum][trackBodyNum].x + 500, frameData[frameNum][trackBodyNum].y + 500));

			window.draw(pixel);
		}
		// Draw bodies
		else if (type == "body") {
			int bodyNum = i->second.second;

			sf::CircleShape shape(size);
			shape.setFillColor(colors[bodyNum]);
			shape.setOrigin(shape.getRadius(), shape.getRadius());
			shape.setPosition(sf::Vector2f(frameData[frame][bodyNum].x + 500, frameData[frame][bodyNum].y + 500));

			// Give the body a recognisable outline if it is being tracked
			if (trackingBody && bodyNum == trackBodyNum) {
				shape.setOutlineColor(sf::Color::Green);
				shape.setOutlineThickness(2);
				trackFrames.push_back(frame);
			}

			window.draw(shape);
		}
	}

	// Display drawn objects and set the view
	if (centerView)
		view.setCenter(sf::Vector2f(frameData[frame][0].x + 500, frameData[frame][0].y + 500));

	// Draw the UI overlay and other UI elements if necessary
	DrawOverlay(frame);

	// Reset the view back
	window.setView(view);
}

void SimulationUI::DrawOverlay(int frame) {
	// Set the view to the uiView to prepare drawing
	window.setView(uiView);

	/** FPS SLIDER **/
	sf::RectangleShape slider(sf::Vector2f(30, 300));
	sf::RectangleShape sliderValue(sf::Vector2f(50, 50));
	sf::Font font;
	font.loadFromFile("../fonts/arial.ttf");
	sf::Text text;

	slider.setFillColor(sf::Color(255, 255, 255, 50));
	slider.setPosition(sf::Vector2f(uiView.getSize().x - 50, uiView.getSize().y / 2 - 150));

	sliderValue.setFillColor(sf::Color::White);
	sliderValue.setPosition(sf::Vector2f(uiView.getSize().x - 60, uiView.getSize().y / 2 + 150 - framerate - 25));

	text.setFont(font);
	text.setFillColor(sf::Color::Black);
	text.setCharacterSize(15);
	text.setString(std::to_string(framerate) + "\nFPS");
	text.setPosition(sf::Vector2f(uiView.getSize().x - 50, uiView.getSize().y / 2 + 150 - framerate - 20));

	window.draw(slider);
	window.draw(sliderValue);
	window.draw(text);

	/** PLAYBACK PROGRESS **/
	sf::RectangleShape totalPlayback(sf::Vector2f(300, 20));
	sf::RectangleShape currentPlayback(sf::Vector2f((float)frame / frameData.size() * 300, 20));

	totalPlayback.setFillColor(sf::Color(255, 0, 0, 50));
	totalPlayback.setOutlineColor(sf::Color::White);
	totalPlayback.setOutlineThickness(2);
	totalPlayback.setPosition(sf::Vector2f(uiView.getSize().x / 2 - 150, 30));

	currentPlayback.setFillColor(sf::Color::Green);
	currentPlayback.setPosition(sf::Vector2f(uiView.getSize().x / 2 - 150, 30));

	text.setFillColor(sf::Color::White);
	text.setOutlineColor(sf::Color::Black);
	text.setOutlineThickness(1);
	text.setString(std::to_string((int)((float)frame / frameData.size() * 100)) + "%");
	text.setOrigin(sf::Vector2f(text.getLocalBounds().width / 2, 0));
	text.setPosition(sf::Vector2f(uiView.getSize().x / 2, 30));

	window.draw(totalPlayback);
	window.draw(currentPlayback);
	window.draw(text);

	/* PAUSED */
	if (paused) {
		sf::Text pauseText;
		pauseText.setFont(font);
		pauseText.setCharacterSize(50);
		pauseText.setFillColor(sf::Color(255, 255, 255, 50));
		pauseText.setString("PAUSED");
		pauseText.setOrigin(sf::Vector2f(pauseText.getLocalBounds().width / 2, pauseText.getLocalBounds().height / 2));
		pauseText.setPosition(sf::Vector2f(uiView.getSize().x / 2, uiView.getSize().y / 2));

		window.draw(pauseText);
	}


	/* TRACKER UI */
	if (showTrackUI) {
		sf::RectangleShape background({ 500, 500 });
		background.setFillColor({ 105, 105, 105, 175 });

		sf::RectangleShape textbox(sf::Vector2f(200, 40));
		textbox.setFillColor(sf::Color::White);
		textbox.setOutlineColor(sf::Color::Black);
		textbox.setOutlineThickness(3);
		textbox.setOrigin(sf::Vector2f(textbox.getLocalBounds().width / 2, textbox.getLocalBounds().height / 2));
		textbox.setPosition(sf::Vector2f(uiView.getSize().x / 2, uiView.getSize().y / 2));

		sf::Text heading;
		heading.setFont(font);
		heading.setCharacterSize(15);
		heading.setFillColor(sf::Color::White);
		heading.setString("Type a number and press ENTER \nto track a body");
		heading.setPosition(sf::Vector2f(uiView.getSize().x / 2 - 103, uiView.getSize().y / 2 - 60));

		sf::Text trackText;
		trackText.setFont(font);
		trackText.setCharacterSize(30);
		trackText.setFillColor(sf::Color::Black);
		trackText.setString(trackTextEntered);
		trackText.setPosition(sf::Vector2f(uiView.getSize().x / 2 - 100, uiView.getSize().y / 2 - 20));

		window.draw(background);
		window.draw(heading);
		window.draw(textbox);
		window.draw(trackText);
	}

	/* CONTROLS DISPLAY */
	if (showControls) {
		window.setView(controlsView);
		window.draw(sf::Sprite(controlsUI));
	}

}

void SimulationUI::HandleKeyboardInput(sf::Keyboard::Key key) {
	switch (key) {

		// Camera unlock/lock
	case sf::Keyboard::Z:
		centerView = !centerView; break;

		// Move camera up
	case sf::Keyboard::W:
		if (!centerView) view.move(sf::Vector2f(0.0f, -10.0f)); break;

		// Move camera left
	case sf::Keyboard::A:
		if (!centerView) view.move(sf::Vector2f(-10.0f, 0.0f)); break;

		// Move camera down
	case sf::Keyboard::S:
		if (!centerView) view.move(sf::Vector2f(0.0f, 10.0f)); break;

		// Move camera right
	case sf::Keyboard::D:
		if (!centerView) view.move(sf::Vector2f(10.0f, 0.0f)); break;

		// Reset playback to the first frame
	case sf::Keyboard::R:
		frame = 0; trackFrames.clear(); break;

		// Show track body UI
	case sf::Keyboard::T:
		if (!showControls) showTrackUI = !showTrackUI; trackTextEntered = ""; break;

		// Increase framerate
	case sf::Keyboard::Up:
		framerate < 300 ? framerate += 5 : framerate = 300; break;

		// Decrease framerate
	case sf::Keyboard::Down:
		framerate > 5 ? framerate -= 5 : framerate = 5; break;

		// Pause playback
	case sf::Keyboard::Space:
		paused = !paused; break;

		// Show controls UI
	case sf::Keyboard::Escape:
		showControls = !showControls;
		if (showControls) {
			lastPausedState = paused;
			paused = true;
		}
		else {
			paused = lastPausedState;
		}
		break;

		// Numbers for selecting which body to track
	case sf::Keyboard::Num0:
		if (!showControls) trackTextEntered += "0"; break;
	case sf::Keyboard::Num1:
		if (!showControls) trackTextEntered += "1"; break;
	case sf::Keyboard::Num2:
		if (!showControls) trackTextEntered += "2"; break;
	case sf::Keyboard::Num3:
		if (!showControls) trackTextEntered += "3"; break;
	case sf::Keyboard::Num4:
		if (!showControls) trackTextEntered += "4"; break;
	case sf::Keyboard::Num5:
		if (!showControls) trackTextEntered += "5"; break;
	case sf::Keyboard::Num6:
		if (!showControls) trackTextEntered += "6"; break;
	case sf::Keyboard::Num7:
		if (!showControls) trackTextEntered += "7"; break;
	case sf::Keyboard::Num8:
		if (!showControls) trackTextEntered += "8"; break;
	case sf::Keyboard::Num9:
		if (!showControls) trackTextEntered += "9"; break;

		// Confirm body number to track in track body UI
	case sf::Keyboard::Enter:
		if (showTrackUI) {
			trackingBody = false;
			int body;
			try {
				body = std::stoi(trackTextEntered);
			}
			catch (...) {
				showTrackUI = false;
				trackingBody = false;
				trackFrames.clear();
				break;
			}

			if (body >= 1 && body <= num_bodies) {
				trackBodyNum = body - 1;
				trackingBody = true;
				trackFrames.clear();
			}
			showTrackUI = false;
		}
		break;

		// Remove last character on track body UI
	case sf::Keyboard::Backspace:
		if (showTrackUI) {
			trackTextEntered = trackTextEntered.substr(0, trackTextEntered.size() - 1);
		}
	}

	// Update the window view
	window.setView(view);

}