#ifndef GW_ACTIONMANAGER_HPP
#define GW_ACTIONMANAGER_HPP

#include "State.hpp"

#define NUM_ACTIONS 4

namespace gw
{
	class ActionManager
	{
	public:
		enum ActionType
		{
			MOVEUP = 0,
			MOVEDOWN,
			MOVELEFT,
			MOVERIGHT
		};

		ActionManager() {}
		~ActionManager() {}

		// apply() - applies the given action to the current state
		State* apply(ActionType a, State* current)
		{
			switch(a)
			{
			case MOVEUP:
				return moveUp(current);
			case MOVEDOWN:
				return moveDown(current);
			case MOVELEFT:
				return moveLeft(current);
			case MOVERIGHT:
				return moveRight(current);
			};
		}
	private:
		State* moveUp(State* current)
		{
			State* next = new State(*current);

			if (next->getGridElem(next->getAgentX(), next->getAgentY() - 1) == State::EMPTY)
			{
				next->setAgentY(next->getAgentY() - 1);
			}

			return next;
		}
		State* moveDown(State* current)
		{
			State* next = new State(*current);

			if (next->getGridElem(next->getAgentX(), next->getAgentY() + 1) == State::EMPTY)
			{
				next->setAgentY(next->getAgentY() + 1);
			}

			return next;
		}
		State* moveLeft(State* current)
		{
			State* next = new State(*current);

			if (next->getGridElem(next->getAgentX() - 1, next->getAgentY()) == State::EMPTY)
			{
				next->setAgentX(next->getAgentX() - 1);
			}

			return next;
		}
		State* moveRight(State* current)
		{
			State* next = new State(*current);

			if (next->getGridElem(next->getAgentX() + 1, next->getAgentY()) == State::EMPTY)
			{
				next->setAgentX(next->getAgentX() + 1);
			}

			return next;
		}
	};
}

#endif