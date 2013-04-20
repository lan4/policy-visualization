
#include "world.h"

using std::ifstream;

World::World(const char* worldfile)
{
    ifstream infile;
    infile.open(worldfile);
    infile >> sizeX >> sizeY;

    init_board();

    int rx, ry;
    for (ry = sizeY-1; ry >= 0; ry--)
    {
        for (rx = 0; rx < sizeX; rx++)
        {
            infile >> board[rx][ry];
        }
    }

    infile.close();

    find_goal();
    find_start();
}

World::~World()
{
    free_board();
}

int World::getsizeX()
{
    return sizeX;
}

int World::getsizeY()
{
    return sizeY;
}

int World::getgoalX()
{
    return goalX;
}

int World::getgoalY()
{
    return goalY;
}

int World::getstartX()
{
    return startX;
}

int World::getstartY()
{
    return startY;
}

char World::get(int x, int y)
{
    return board[x][y];
}

void World::init_board()
{
    board = (char**) malloc(sizeof(char*) * sizeX);
    int i;
    for (i = 0; i < sizeX; i++)
    {
        board[i] = (char*) malloc(sizeof(char) * sizeY);
    }
}

void World::free_board()
{
    int i;
    for (i = 0; i < sizeX; i++)
    {
        free(board[i]);
    }
    free(board);
}

void World::find_goal()
{
    int i, j;
    for (i = 0; i < sizeX; i++)
    {
        for (j = 0; j < sizeY; j++)
        {
            if (board[i][j] == GOAL)
            {
                goalX = i;
                goalY = j;
            }
        }
    }
}

void World::find_start()
{
    int i, j;
    for (i = 0; i < sizeX; i++)
    {
        for (j = 0; j < sizeY; j++)
        {
            if (board[i][j] == START)
            {
                startX = i;
                startY = j;
            }
        }
    }
}
