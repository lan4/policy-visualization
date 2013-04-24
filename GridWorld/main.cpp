#include <iostream>
#include "State.hpp"
#include "PolicyGenerator.hpp"
#include "SFML\System.hpp"
#include "SFML\Window.hpp"

int main()
{
	gw::PolicyGenerator pg("test.txt", "reward.txt");
	pg.runNEpisodes(100);
	pg.updateVisualGrid();

	sf::RenderWindow window(sf::VideoMode(800, 600), "PolicyVi");
	sf::RectangleShape test(sf::Vector2f(2, 2));
	test.setPosition(50, 100);
	while (window.isOpen())
	{
		sf::Event e;
		while (window.pollEvent(e))
		{
			if (e.type == sf::Event::Closed)
				window.close();
		}

		pg.draw(&window);
		//window.draw(test);
		window.display();
	}

	return 0;
}