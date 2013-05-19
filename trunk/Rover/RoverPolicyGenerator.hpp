#ifndef RV_POLYGEN_HPP
#define RV_POLYGEN_HPP

#include "RoverState.hpp"
#include "RoverAction.hpp"

#define MIN_EPSILON 5

namespace Rover
{
	class PolicyGenerator
	{
	public:
		PolicyGenerator() : discount(0.5f), learning(0.5f), _epsilon(50), numRuns(0)
		{

		}
		PolicyGenerator(const char* statefile) : discount(0.5f), learning(0.5f), _epsilon(50), numRuns(0)
		{
			_baseState = new State(statefile);
		}
		~PolicyGenerator() { }

		void runEpisode()
		{
			State* curState = State::retrieve(new State(*_baseState));
			State* nextState;
			int actionIndex;
			float reward;

			while (!curState->isTerminalState())
			{
				nextState = State::retrieve(_actionManager.apply(actionIndex = getBestAction(curState), curState, reward));
				curState->qvals[actionIndex] += learning * (reward + discount * maxQ(curState) - curState->qvals[actionIndex]);
				
				if (curState->qvals[actionIndex] > State::maxQVal)
					State::maxQVal = curState->qvals[actionIndex];
				
				curState = nextState;
			}

			_epsilon--;
			if (_epsilon < MIN_EPSILON)
				_epsilon = MIN_EPSILON;

			numRuns++;
		}
		void runEpisodes(int n)
		{
			for (int i = 0; i < n; i++)
			{
				runEpisode();
			}
		}
		bool stepEpisode(State* &s, State* &s2)
		{
			static State* curState = State::retrieve(new State(*_baseState));
			State* nextState;
			int actionIndex;
			float reward;

			if (curState->isTerminalState())
			{
				curState = State::retrieve(new State(*_baseState));
				s = new State(*curState);
				s2 = new State(*curState);
				_epsilon--;
				if (_epsilon < MIN_EPSILON)
					_epsilon = MIN_EPSILON;
				numRuns++;

				return true;
			}
			else
			{
				nextState = State::retrieve(_actionManager.apply(actionIndex = getBestAction(curState), curState, reward));
				curState->qvals[actionIndex] += learning * (reward + discount * maxQ(nextState) - curState->qvals[actionIndex]);
				
				if (curState->qvals[actionIndex] > State::maxQVal)
					State::maxQVal = curState->qvals[actionIndex];
				
				s2 = new State(*curState);
				curState = nextState;
				s = new State(*curState);
				return false;
			}
		}

		State* getBaseState()
		{
			return _baseState;
		}

		float discount, learning;
		int numRuns;
	private:
		int getBestAction(State* s)
		{
			std::vector<int> ivals;
			int maxI = 0;
			float maxQ = s->qvals[0];
			ivals.push_back(maxI);
			
			for (int i = 1; i < NUM_ROVER_ACTIONS; i++)
			{
				if (maxQ < s->qvals[i])
				{
					maxI = i;
					maxQ = s->qvals[i];
					ivals.erase(ivals.begin(), ivals.end());
					ivals.push_back(i);
				}
				else if (maxQ == s->qvals[i])
				{
					ivals.push_back(i);
				}
			}

			int e = rand() % 100;

			if (e < _epsilon)
			{
				int r = rand() % NUM_ROVER_ACTIONS;

				return r;
			}
			else if (ivals.size() == 1 && e > _epsilon)
				return maxI;
			else
			{
				int r = rand() % ivals.size();

				return ivals[r];
			}
		}
		float maxQ(State* s)
		{
			float maxVal = s->qvals[0];

			for (int i = 1; i < NUM_ROVER_ACTIONS; i++)
			{
				if (maxVal < s->qvals[i])
					maxVal = s->qvals[i];
			}

			return maxVal;
		}

		State* _baseState;
		Action _actionManager;
		int _epsilon;
	};
}

#endif