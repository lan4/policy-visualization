#include <iostream>
#include "RoverState.hpp"
#include "RoverPolicyGenerator.hpp"
#include "RoverVisualizer.hpp"
#include "SFML\System.hpp"
#include "SFML\Window.hpp"
#include "SFML\Graphics.hpp"

int main()
{
	Rover::PolicyGenerator pg("rtest.txt");
	Rover::State* s = 0;
	Rover::State* s2 = 0;
	Rover::State::rewards[0] = 100;
	Rover::State::rewards[1] = -10;
	Rover::State::rewards[2] = -100;
	Rover::State::rewards[3] = -10;
	int windowWidth = 800, windowHeight = 600;
	Rover::Visualizer vi(windowWidth, windowHeight, &pg);
	Rover::Visualizer::DrawRawView = true;
	//Rover::Visualizer::DrawVStatView = true;
	//Rover::Visualizer::DrawPaths = true;

	sf::RenderWindow window(sf::VideoMode(windowWidth, windowHeight), "PolicyVi");
	bool stopPG = false;
	sf::Clock timer;
	bool first = true, asFastAsPossible = false;
	float updateRate = 10.0f;

	while (window.isOpen())
	{
		sf::Event e;
		while (window.pollEvent(e))
		{
			if (e.type == sf::Event::Closed)
				window.close();
			else if (e.type == sf::Event::KeyReleased)
			{
				switch (e.key.code)
				{
				case sf::Keyboard::Space:
					stopPG = !stopPG;
					break;
				case sf::Keyboard::Num0:
					asFastAsPossible = true;
					break;
				case sf::Keyboard::Num1:
					updateRate = 10.0f;
					asFastAsPossible = false;
					break;
				case sf::Keyboard::Num2:
					updateRate = 30.0f;
					asFastAsPossible = false;
					break;
				case sf::Keyboard::Num3:
					updateRate = 60.0f;
					asFastAsPossible = false;
					break;
				case sf::Keyboard::S:
					Rover::Visualizer::DrawStatView = !Rover::Visualizer::DrawStatView;
					break;
				case sf::Keyboard::V:
					Rover::Visualizer::DrawVStatView = !Rover::Visualizer::DrawVStatView;
					break;
				case sf::Keyboard::L:
					Rover::Visualizer::DrawLinePlot = !Rover::Visualizer::DrawLinePlot;
					break;
				case sf::Keyboard::R:
					Rover::Visualizer::DrawRawView = !Rover::Visualizer::DrawRawView;
					break;
				case sf::Keyboard::P:
					Rover::Visualizer::DrawPaths = !Rover::Visualizer::DrawPaths;
					break;
				};
			}
		}

		if (timer.getElapsedTime().asSeconds() > 1 / updateRate || first || asFastAsPossible)
		{
			if (!stopPG)
			{
				pg.stepEpisode(s, s2);
				vi.UpdatedPG = true;
			}
			else
			{
				vi.UpdatedPG = false;
				s = pg.getBaseState();
			}

			timer.restart();

			if (first)
				first = false;

		}
		else
			vi.UpdatedPG = false;

		vi.update(s, s2);
		window.clear(sf::Color::White);
		window.draw(vi);

		window.display();
	}

	return 0;
}