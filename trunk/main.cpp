#include <SFML/Graphics.hpp>

#include <iostream>
#include "world.h"

using std::cout;
using std::endl;

using sf::Vector2f;

int main(int argc, char* argv[])
{
    if (argc == 2)
       cout << "Load file from: " << argv[1] << endl;
    else
    {
        cout << "No World File Specified\nExiting" << endl;
        exit(1);
    }

    World map(argv[1]);

    int grid_width = map.getsizeX();
    int grid_height = map.getsizeY();

    int win_width = 600;
    int win_height = 600;

    int margin = 5;

    Vector2f tile_size(win_width / grid_width - margin, win_height / grid_height - margin);

    std::vector<sf::RectangleShape> grid;

    int grid_x, grid_y, win_x, win_y;
    for (grid_x = 0; grid_x < grid_width; grid_x++)
    {
        for (grid_y = 0; grid_y < grid_height; grid_y++)
        {
            sf::RectangleShape tmp;
            tmp.setSize(tile_size);

            win_x = grid_x * (win_width / grid_width) + (margin / 2);
            win_y = (grid_height - grid_y - 1) * (win_height / grid_height) + (margin / 2);
            Vector2f position(win_x, win_y);
            tmp.setPosition(position);

            if (map.get(grid_x, grid_y) == World::WALL)
                tmp.setFillColor(sf::Color::Black);
            else if (map.get(grid_x, grid_y) == World::OPEN)
                tmp.setFillColor(sf::Color::White);
            else if (map.get(grid_x, grid_y) == World::GOAL)
                tmp.setFillColor(sf::Color::Green);
            else if (map.get(grid_x, grid_y) == World::START)
                tmp.setFillColor(sf::Color::Blue);

            grid.push_back(tmp);
        }
    }

    sf::RenderWindow window(sf::VideoMode(win_width, win_height), "Grid World");
    window.setFramerateLimit(15);

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        window.clear();
        unsigned int i;
        for (i = 0; i < grid.size(); i++)
        {
            window.draw(grid[i]);
        }
        window.display();
    }

    return 0;
}
