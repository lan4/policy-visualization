#include <fstream>
#include <cstdlib>

class World
{
public:
    World(const char *worldfile);
    ~World();

    int getsizeX();
    int getsizeY();

    char get(int x, int y);

    int getgoalX();
    int getgoalY();

    int getstartX();
    int getstartY();

    const char static WALL = 'w';
    const char static OPEN = '.';
    const char static GOAL = 'g';
    const char static START = 's';

private:
    int sizeX;
    int sizeY;

    int goalX;
    int goalY;
    int startX;
    int startY;

    char** board;

    void init_board();
    void free_board();

    void find_goal();
    void find_start();
};
