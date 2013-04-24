#ifndef GW_POLICYGENERATOR_HPP
#define GW_POLICYGENERATOR_HPP

#include <iostream>
#include <fstream>
#include <vector>
#include <cstdlib>
#include "State.hpp"
#include "ActionManager.hpp"
#include "SFML\System.hpp"
#include "SFML\Graphics.hpp"

//class PolicyVisualizer;

namespace gw
{
	class PolicyGenerator
	{
	public:
		PolicyGenerator() : _discountFactor(0.5f), _learningRate(0.5f)
		{
			initArrays();
		}
		PolicyGenerator(const char* worldFile, const char* rewardFile) : _discountFactor(0.5f), _learningRate(0.5f)
		{
			_baseState = new State(worldFile);
			initArrays(NUM_ACTIONS, _baseState->getGridWidth() * _baseState->getGridHeight());

			std::ifstream infile(rewardFile);
			if (infile.is_open() && infile.good())
			{
				for (int i = 0; i < _rewards.size(); i++)
				{
					for (int j = 0; j < NUM_ACTIONS; j++)
						infile >> _rewards[i][j];
				}
			}
		}
		~PolicyGenerator() {}

		void runEpisode()
		{
			State* curState = new State(*_baseState);//randomState();
			State* nextState;
			ActionManager::ActionType nextAction;

			while (!curState->isGoalState())
			{
				nextState = _actions.apply(nextAction = getBestAction(curState), curState);
				_qvals[curState->getStateIndex()][nextAction] += _learningRate * (_rewards[curState->getStateIndex()][nextAction] + 
					_discountFactor * maxQVal(nextState) - _qvals[curState->getStateIndex()][nextAction]);
				curState = nextState;
			}
		}
		void runNEpisodes(int n = 1)
		{
			for (int i = 0; i < n; i++)
				runEpisode();
		}
		void normalizeQVals()
		{
			float qMax = maxQVal(0);
			for (int i = 1; i < _qvals.size(); i++)
			{
				float newMax = maxQVal(i);
				if (qMax < newMax)
					qMax = newMax;
			}

			for (int i = 0; i < _qvals.size(); i++)
				for (int j = 0; j < NUM_ACTIONS; j++)
					_qvals[i][j] /= qMax;
		}


		void draw(sf::RenderWindow* window)
		{
			for (int i = 0; i < _baseState->getGridHeight(); i++)
				for (int j = 0; j < _baseState->getGridWidth(); j++)
					window->draw(_viGrid[i][j]);
		}
		void updateVisualGrid()
		{
			for (int i = 0; i < _baseState->getGridHeight(); i++)
				for (int j = 0; j < _baseState->getGridWidth(); j++)
				{
					if ((i != _baseState->getAgentY() || j != _baseState->getAgentX()) && (i != _baseState->getGoalY()  || j != _baseState->getGoalX()))
						_viGrid[i][j].setFillColor(sf::Color((int)(255 * maxQVal(i * _baseState->getGridWidth() + j)), 0, 0, 255));
				}
		}
	private:
		void initArrays(int width = DEFAULT_GRID_SIZE, int height = DEFAULT_GRID_SIZE)
		{
			_qvals.resize(height);
			_rewards.resize(height);
			for (int i = 0; i < height; i++)
			{
				_qvals[i].resize(width);
				_rewards[i].resize(width);
			}

			_viGrid.resize(_baseState->getGridHeight());
			for (int i = 0; i < _baseState->getGridHeight(); i++)
			{
				_viGrid[i].resize(_baseState->getGridWidth());
				float height = 50.0f * i + 10 * i + 5;
				for (int j = 0; j < _baseState->getGridWidth(); j++)
				{
					_viGrid[i][j] = sf::RectangleShape(sf::Vector2f(50.0f, 50.0f));
					//_viGrid[i][j].setOrigin(25, 25);
					_viGrid[i][j].setPosition(50.0f * j + 10 * j + 5, height);
					_viGrid[i][j].setOutlineThickness(5);
					_viGrid[i][j].setOutlineColor(sf::Color::Black);

					if (_baseState->getGridElem(j, i) == State::BLOCKED)
					{
						_viGrid[i][j].setFillColor(sf::Color::Black);
						_viGrid[i][j].setOutlineColor(sf::Color::Yellow);
					}
				}
			}

			_viGrid[_baseState->getAgentY()][_baseState->getAgentX()].setFillColor(sf::Color::Blue);
			_viGrid[_baseState->getGoalY()][_baseState->getGoalX()].setFillColor(sf::Color::Red);

			_viGrid[_baseState->getAgentY()][_baseState->getAgentX()].setOutlineColor(sf::Color::White);
			_viGrid[_baseState->getGoalY()][_baseState->getGoalX()].setOutlineColor(sf::Color::White);
		}
		ActionManager::ActionType getBestAction(State* s)
		{
			int si = s->getStateIndex();
			bool isMax[NUM_ACTIONS];
			int numSame = 0;

			float qMax = _qvals[si][0];
			int iMax = 0;
			for (int i = 1; i < _qvals[si].size(); i++)
			{
				if (qMax < _qvals[si][i])
				{
					qMax = _qvals[si][i];
					iMax = i;
				}
			}

			for (int j = 0; j < _qvals[si].size(); j++)
			{
				if (qMax >= _qvals[si][j])
				{
					isMax[j] = true;
					numSame++;
				}
			}

			if (numSame > 1)
			{
				int action = rand() % numSame;

				for (int k = 0, m = 0; k < numSame;)
				{
					if (isMax[m])
					{
						if (k == action)
						{
							switch(m)
							{
							case 0:
								return ActionManager::MOVEUP;
							case 1:
								return ActionManager::MOVEDOWN;
							case 2:
								return ActionManager::MOVELEFT;
							case 3:
								return ActionManager::MOVERIGHT;
							};
						}
						else
							k++;
					}

					m++;
				}
			}
			else
			{
				switch(iMax)
				{
				case 0:
					return ActionManager::MOVEUP;
				case 1:
					return ActionManager::MOVEDOWN;
				case 2:
					return ActionManager::MOVELEFT;
				case 3:
					return ActionManager::MOVERIGHT;
				};
			}
		}
		float maxQVal(State* state)
		{
			float qMax = _qvals[state->getStateIndex()][0];
			for (int i = 1; i < NUM_ACTIONS; i++)
			{
				if (qMax < _qvals[state->getStateIndex()][i])
				{
					qMax = _qvals[state->getStateIndex()][i];
				}
			}

			return qMax;
		}
		float maxQVal(int stateIndex)
		{
			float qMax = _qvals[stateIndex][0];
			for (int i = 1; i < NUM_ACTIONS; i++)
			{
				if (qMax < _qvals[stateIndex][i])
				{
					qMax = _qvals[stateIndex][i];
				}
			}

			return qMax;
		}
		State* randomState()
		{
			State* newState = new State(*_baseState);

			bool isValid = false;
			while (!isValid)
			{
				newState->setAgentX(rand() % newState->getGridWidth());
				newState->setAgentY(rand() % newState->getGridHeight());

				isValid = newState->getGridElem(newState->getAgentX(), newState->getAgentY()) == State::GridElem::EMPTY;
			}

			return newState;
		}

		State* _baseState;
		std::vector<std::vector<float>> _qvals;
		std::vector<std::vector<int>> _rewards;
		float _discountFactor, _learningRate;
		ActionManager _actions;

		std::vector<std::vector<sf::RectangleShape>> _viGrid;
	};
}

#endif