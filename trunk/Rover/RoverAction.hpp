#ifndef RV_ACTION_HPP
#define RV_ACTION_HPP

#include "RoverState.hpp"

namespace Rover
{
	enum ActionName
	{
		MoveForward = 0,
		TurnLeft,
		TurnRight,
		Wait,
		CollectSample
	};

	class Action
	{
	public:
		Action() {}
		~Action() {}

		State* apply(ActionName action, State* currentState, float &reward)
		{
			switch (action)
			{
			case ActionName::MoveForward:
				return moveForward(currentState, reward);
			case ActionName::TurnLeft:
				return turnLeft(currentState, reward);
			case ActionName::TurnRight:
				return turnRight(currentState, reward);
			case ActionName::Wait:
				return wait(currentState, reward);
			case ActionName::CollectSample:
				return collectSample(currentState, reward);
			};

			return 0;
		}
		State* apply(int action, State* currentState, float &reward)
		{
			switch (action)
			{
			case 0:
				return moveForward(currentState, reward);
			case 1:
				return turnLeft(currentState, reward);
			case 2:
				return turnRight(currentState, reward);
			case 3:
				return wait(currentState, reward);
			case 4:
				return collectSample(currentState, reward);
			};

			return 0;
		}
	private:
		State* moveForward(State* currentState, float &reward)
		{
			State* s = new State(*currentState);

			if (!s->isPowerEmpty())
			{
				switch (s->moveAgentForward())
				{
				case 0:
					reward = (float)0;
					break;
				case 1:
					reward = (float)State::rewards[BUMP];
					break;
				case 2:
					reward = (float)State::rewards[PITFALL];
					break;
				};

				if (s->isPowerEmpty())
					reward = (float)State::rewards[ACTION_FAIL];
			}
			else
				reward = (float)State::rewards[ACTION_FAIL];

			s->advanceTime();
			return s;
		}
		State* turnLeft(State* currentState, float &reward)
		{
			State* s = new State(*currentState);

			if (!s->isPowerEmpty())
			{
				s->turnAgent(true);
				reward = (float)0;

				if (s->isPowerEmpty())
					reward = (float)State::rewards[ACTION_FAIL];
			}
			else
				reward = (float)State::rewards[ACTION_FAIL];

			s->advanceTime();
			return s;
		}
		State* turnRight(State* currentState, float &reward)
		{
			State* s = new State(*currentState);

			if (!s->isPowerEmpty())
			{
				s->turnAgent(false);
				reward = (float)0;

				if (s->isPowerEmpty())
					reward = (float)State::rewards[ACTION_FAIL];
			}
			else
				reward = (float)State::rewards[ACTION_FAIL];

			s->advanceTime();
			return s;
		}
		State* wait(State* currentState, float &reward)
		{
			State* s = new State(*currentState);
			reward = (float)-5;

			if (s->isPowerEmpty())
				reward = (float)State::rewards[ACTION_FAIL];

			s->advanceTime();
			return s;
		}
		State* collectSample(State* currentState, float &reward)
		{
			State* s = new State(*currentState);

			if (!s->isPowerEmpty())
			{
				if (s->agentCollectSample())
					reward = (float)State::rewards[COLLECT];
				else
					reward = (float)State::rewards[ACTION_FAIL];

				if (s->isPowerEmpty())
					reward = (float)State::rewards[ACTION_FAIL];
			}
			else
				reward = (float)State::rewards[ACTION_FAIL];

			s->advanceTime();
			return s;
		}
	};
}

#endif