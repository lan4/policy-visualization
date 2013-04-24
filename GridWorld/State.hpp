#ifndef GW_STATE_HPP
#define GW_STATE_HPP

#include <vector>
#include <assert.h>
#include <iostream>
#include <fstream>

#define DEFAULT_GRID_SIZE 10

namespace gw
{
	class State
	{
	public:
		enum GridElem
		{
			ERROR = 0,
			EMPTY,
			BLOCKED
		};

		State() : _gridWidth(DEFAULT_GRID_SIZE), _gridHeight(DEFAULT_GRID_SIZE), 
			_agentX(0), _agentY(0), _goalX(0), _goalY(0), _numBlocked(0)
		{
			initGrid();
		}
		State(int width, int height, int sX, int sY, int gX, int gY) : _gridWidth(width), _gridHeight(height), 
			_agentX(sX), _agentY(sY), _goalX(gX), _goalY(gY), _numBlocked(0)
		{
			initGrid(width, height);
		}
		State(const char* stateFile)
		{
			_numBlocked = 0;
			std::ifstream infile(stateFile);
			if (infile.is_open())
			{
				if (infile.good())
				{
					infile >> _gridWidth >> _gridHeight >> _agentX >> _agentY >> _goalX >> _goalY;
					initGrid(_gridWidth, _gridHeight);

					char gridc;
					for (int i = 0; i < _gridHeight; i++)
					{
						for (int j = 0; j < _gridWidth; j++)
						{
							infile >> gridc;

							switch (gridc)
							{
							case '.':
								_grid[i][j] = GridElem::EMPTY;
								break;
							case 'x':
							case 'X':
								_grid[i][j] = GridElem::BLOCKED;
								_numBlocked++;
								break;
							default:
								_grid[i][j] = GridElem::ERROR;
								break;
							};
						}
					}
				}
			}

			infile.close();
		}
		State(const State &copy) : _gridWidth(copy._gridWidth), _gridHeight(copy._gridHeight), 
			_agentX(copy._agentX), _agentY(copy._agentY), _goalX(copy._goalX), _goalY(copy._goalY)
		{
			_grid.resize(copy._gridHeight);
			for (int i = 0; i < _gridHeight; i++)
			{
				_grid[i].resize(copy._gridWidth);
				for (int j = 0; j < _gridWidth; j++)
					_grid[i][j] = copy._grid[i][j];
			}
		}
		~State() {}

		State& operator=(const State &src)
		{
			_gridWidth = src._gridWidth;
			_gridHeight = src._gridHeight;

			_grid.resize(_gridHeight);
			for (int i = 0; i < _gridHeight; i++)
			{
				_grid[i].resize(_gridWidth);
				for (int j = 0; j < _gridWidth; j++)
					_grid[i][j] = src._grid[i][j];
			}

			_agentX = src._agentX;
			_agentY = src._agentY;
			_goalX = src._goalX;
			_goalY = src._goalY;
		}

		int getAgentX() { return _agentX; }
		int getAgentY() { return _agentY; }
		void setAgentX(int x) 
		{
			if (x < 0)
				_agentX = 0;
			else if (x >= _gridWidth)
				_agentX = _gridWidth;
			else
				_agentX = x; 
		}
		void setAgentY(int y) 
		{ 
			if (y < 0)
				_agentY = 0;
			else if (y >= _gridHeight)
				_agentY = _gridHeight;
			else
				_agentY = y;  
		}
		const int getGridWidth() { return _gridWidth; }
		const int getGridHeight() { return _gridHeight; }
		int getGoalX() { return _goalX; }
		int getGoalY() { return _goalY; }

		GridElem getGridElem(int x, int y)
		{
			if (x >= 0 && x < _gridWidth && y >= 0 && y < _gridHeight)
				return _grid[y][x];
			else 
				return GridElem::ERROR;
		}
		void setGridElem(int x, int y, GridElem ge)
		{
			if (x >= 0 && x < _gridWidth && y >= 0 && y < _gridHeight)
			{
				GridElem oldge = _grid[y][x];
				_grid[y][x] = ge;

				if (oldge == GridElem::BLOCKED && ge != GridElem::BLOCKED)
					_numBlocked--;
				else if (oldge != GridElem::BLOCKED && ge == GridElem::BLOCKED)
					_numBlocked++;
			}
		}
		
		const bool isGoalState() { return _agentX == _goalX && _agentY == _goalY; }
		const int getStateIndex() { return _agentX + _agentY * _gridWidth; }

		const int getNumBlocked() { return _numBlocked; }
	private:
		void initGrid(int width = DEFAULT_GRID_SIZE, int height = DEFAULT_GRID_SIZE)
		{
			_grid.resize(height);
			for (int i = 0; i < _grid.size(); i++)
				_grid[i].resize(width);
		}

		int _gridWidth, _gridHeight;
		std::vector<std::vector<GridElem>> _grid;
		int _agentX, _agentY, _goalX, _goalY;
		int _numBlocked;
	};
}

#endif