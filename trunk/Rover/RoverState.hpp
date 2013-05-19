#ifndef RV_STATE_HPP
#define RV_STATE_HPP

#include <vector>
#include <map>
#include <iostream>
#include <fstream>
#include <random>
#include <string>
#include <sstream>

#define NUM_TIME_PERIODS 4

#define NUM_ROVER_ACTIONS 5
#define MOVE_FORWARD 0
#define TURN_LEFT 1
#define TURN_RIGHT 2
#define WAIT 3
#define COLLECT_SAMPLE 4

#define NUM_REWARDS 4
#define COLLECT 0
#define BUMP 1
#define PITFALL 2
#define ACTION_FAIL 3

namespace Rover
{
	enum Terrain
	{
		Flat = 0,
		Pit,
		Rock,
		Station,
		Error
	};
	enum TimeOfDay
	{
		Morning = 0,
		Noon,
		Dusk,
		Night,
		Unspecified
	};
	enum Direction
	{
		North = 0,
		East,
		South,
		West,
		None
	};

	class State;
	struct StateAverages
	{
		State* state;

		float agentPower;
		float samplesLeft;
		float timeOfDay;
		float windDirection;
		float agentDirection;
		float qvals[NUM_ROVER_ACTIONS];

		int count;
	};

	class State
	{
	public:
		// Static Rewards
		static int rewards[NUM_REWARDS];

		// Static Power Vars
		static int powerConsume[NUM_ROVER_ACTIONS];
		static int powerIncrease[NUM_TIME_PERIODS];
		static int stationPowerRate;

		// Static Timesteps
		static int stepsPerPeriod;

		// Static Normalizers
		static float maxQVal;
		static float maxAvgPower;
		static float maxAvgNumSamp;
		static float maxAvgTOD;
		static float maxAvgWDir;
		static float maxAvgADir;
		static float maxAvgQVals[NUM_ROVER_ACTIONS];
		static int maxAvgCount;

		static std::map<std::string, StateAverages> stateAvgs;

		State() : _timesteps(0), _agentPower(0), _curTOD(TimeOfDay::Morning), 
			_windDir(Direction::None), _agentDir(Direction::None)
		{

		}
		State(std::string filename) : _timesteps(0), _atPit(false)
		{
			std::ifstream stateFile(filename);

			if (stateFile.is_open())
			{
				int iDir, iDir2, iTOD;
				stateFile >> _mapWidth >> _mapHeight >> _agentXY.first >> _agentXY.second >> 
					_agentPower >> stationPowerRate >> iDir >> iTOD >> iDir2 >> stepsPerPeriod;
				stateFile >> powerConsume[0] >> powerConsume[1] >> powerConsume[2] >> 
					powerConsume[3] >> powerConsume[4];
				stateFile >> powerIncrease[0] >> powerIncrease[1] >> powerIncrease[2] >>
					powerIncrease[3];

				_agentDir = itod(iDir);
				_windDir = itod(iDir2);
				_curTOD = itotod(iTOD);

				_map.resize(_mapHeight);
				for (int i = 0; i < _mapHeight; i++)
				{
					_map[i].resize(_mapWidth);
					for (int j = 0; j < _mapWidth; j++)
					{
						char c;
						stateFile >> c;
						switch (c)
						{
						case '-':
							_map[i][j] = Terrain::Flat;
							break;
						case 'P':
							_map[i][j] = Terrain::Pit;
							break;
						case 'R':
							_map[i][j] = Terrain::Rock;
							break;
						case 'S':
							_map[i][j] = Terrain::Station;
							break;
						default:
							_map[i][j] = Terrain::Error;
							break;
						};
					}
				}
				
				while (stateFile.good())
				{
					std::pair<int, int> sxy;

					stateFile >> sxy.first >> sxy.second;
					_samplesXY.push_back(sxy);
				}
			}

			for (int i = 0; i < NUM_ROVER_ACTIONS; i++)
			{
				qvals[i] = 0;
			}
		}
		State(const State &copy) : _mapWidth(copy._mapWidth), _mapHeight(copy._mapHeight), 
			_agentPower(copy._agentPower), _timesteps(copy._timesteps), _curTOD(copy._curTOD),
			_agentDir(copy._agentDir), _atPit(copy._atPit), _windDir(copy._windDir)
		{
			_map.resize(_mapHeight);
			for (int i = 0; i < _mapHeight; i++)
			{
				_map[i].resize(_mapWidth);
				for (int j = 0; j < _mapWidth; j++)
				{
					_map[i][j] = copy._map[i][j];
				}
			}

			_agentXY.first = copy._agentXY.first;
			_agentXY.second = copy._agentXY.second;

			for (int k = 0; k < (int)copy._samplesXY.size(); k++)
			{
				std::pair<int, int> sample;

				sample.first = copy._samplesXY[k].first;
				sample.second = copy._samplesXY[k].second;

				_samplesXY.push_back(sample);
			}

			for (int l = 0; l < NUM_ROVER_ACTIONS; l++)
			{
				qvals[l] = copy.qvals[l];
			}
		}
		~State()
		{

		}

		State& operator=(const State &src)
		{
			_mapWidth = src._mapWidth;
			_mapHeight = src._mapHeight; 
			_agentPower = src._agentPower;
			_timesteps = src._timesteps;
			_curTOD = src._curTOD;
			_agentDir = src._agentDir;
			_atPit = src._atPit;

			_map.resize(_mapHeight);
			for (int i = 0; i < _mapHeight; i++)
			{
				_map.resize(_mapWidth);
				for (int j = 0; j < _mapWidth; j++)
				{
					_map[i][j] = src._map[i][j];
				}
			}

			_agentXY.first = src._agentXY.first;
			_agentXY.second = src._agentXY.second;

			for (int k = 0; k < (int)src._samplesXY.size(); k++)
			{
				std::pair<int, int> sample;

				sample.first = src._samplesXY[k].first;
				sample.second = src._samplesXY[k].second;

				_samplesXY.push_back(sample);
			}

			for (int l = 0; l < NUM_ROVER_ACTIONS; l++)
			{
				qvals[l] = src.qvals[l];
			}

			return *this;
		}
		bool operator==(const State &src)
		{
			if (_mapWidth != src._mapWidth ||
				_mapHeight != src._mapHeight ||
				_agentPower != src._agentPower ||
				_timesteps != src._timesteps ||
				_curTOD != src._curTOD ||
				_agentDir != src._agentDir ||
				_atPit != src._atPit ||
				_agentXY.first != src._agentXY.first ||
				_agentXY.second != src._agentXY.second ||
				_samplesXY.size() != src._samplesXY.size())
				return false;

			for (int i = 0; i < _mapHeight; i++)
			{
				for (int j = 0; j < _mapWidth; j++)
				{
					if (_map[i][j] != src._map[i][j])
						return false;
				}
			}
		}

		const bool isTerminalState()
		{
			return _samplesXY.size() == 0 || _atPit || _agentPower <= 0;
		}
		const bool isPowerEmpty() { return _agentPower <= 0; }

		// Action Transition Functions
		void advanceTime(int timestep=1)
		{
			if (timestep < 1)
				return;

			// Passive Power Absorption
			_agentPower += powerIncrease[_curTOD];

			// Station Power Absorption
			if ((_agentXY.second + 1 < _mapHeight  && 
				_map[_agentXY.second + 1][_agentXY.first] == Terrain::Station) ||
				(_agentXY.first + 1 < _mapWidth && 
				_map[_agentXY.second][_agentXY.first + 1] == Terrain::Station) ||
				(_agentXY.second - 1 >= 0 && 
				_map[_agentXY.second - 1][_agentXY.first] == Terrain::Station) ||
				(_agentXY.first - 1 >= 0 && 
				_map[_agentXY.second][_agentXY.first - 1] == Terrain::Station))
				_agentPower += stationPowerRate;

			// Change Time of Day
			_timesteps += timestep;
			if (_timesteps >= stepsPerPeriod)
			{
				switch (_curTOD)
				{
				case TimeOfDay::Morning:
					_curTOD = TimeOfDay::Noon;
					break;
				case TimeOfDay::Noon:
					_curTOD = TimeOfDay::Dusk;
					break;
				case TimeOfDay::Dusk:
					_curTOD = TimeOfDay::Night;
					break;
				case TimeOfDay::Night:
					_curTOD = TimeOfDay::Morning;
					break;
				};

				_timesteps = 0;
			}

			// Change Wind Direction
			int r = rand() % 5;
			switch (r)
			{
			case 0:
				_windDir = Direction::North;
				break;
			case 1:
				_windDir = Direction::East;
				break;
			case 2:
				_windDir = Direction::South;
				break;
			case 3:
				_windDir = Direction::West;
				break;
			case 4:
				_windDir = Direction::None;
			}
		}
		int moveAgentForward()
		{
			_agentPower -= powerConsume[MOVE_FORWARD];

			int ax = _agentXY.first, ay = _agentXY.second, ax2 = ax, ay2 = ay;
			bool aew = _agentDir == _windDir;

			switch (_agentDir)
			{
			case Direction::North:
				ay++;
				ay2 = ay + 1;
				break;
			case Direction::East:
				ax++;
				ax2 = ax + 1;
				break;
			case Direction::South:
				ay--;
				ay2 = ay - 1;
				break;
			case Direction::West:
				ax--;
				ax2 = ax - 1;
				break;
			};

			if (ax >= _mapWidth || ax < 0 || ay >= _mapHeight || ay < 0)
			{
				return 1; // BUMP
			}

			if (aew)
			{
				
				if (_map[ay][ax] == Terrain::Flat)
				{
					if (ax2 >= _mapWidth || ax2 < 0 || ay2 >= _mapHeight || ay2 < 0)
					{
						_agentXY.first = ax;
						_agentXY.second = ay;
						return 1; // BUMP
					}

					if (_map[ay2][ax2] == Terrain::Flat)
					{
						_agentXY.first = ax2;
						_agentXY.second = ay2;
						return 0;
					}
					else if (_map[ay2][ax2] == Terrain::Pit)
					{
						_agentXY.first = ax2;
						_agentXY.second = ay2;
						_atPit = true;
						return 2; // PITFALL
					}
					else
					{
						_agentXY.first = ax;
						_agentXY.second = ay;
						return 1; // BUMP
					}
				}
				else if (_map[ay][ax] == Terrain::Pit)
				{
					_agentXY.first = ax;
					_agentXY.second = ay;
					_atPit = true;
					return 2; // PITFALL
				}
				else
				{
					return 1; // BUMP
				}
			}
			else
			{
				if (_map[ay][ax] == Terrain::Flat)
				{
					_agentXY.first = ax;
					_agentXY.second = ay;
					return 0;
				}
				else if (_map[ay][ax] == Terrain::Pit)
				{
					_agentXY.first = ax;
					_agentXY.second = ay;
					_atPit = true;
					return 2; // PITFALL
				}
				else
				{
					return 1; // BUMP
				}
			}
		}
		void turnAgent(bool turnLeft)
		{
			_agentPower -= turnLeft ? powerConsume[TURN_LEFT] : powerConsume[TURN_RIGHT];

			int ddiff = _windDir - _agentDir;

			if ((ddiff == 2 || ddiff == -2) && _windDir != Direction::None)
					_agentDir = _windDir; 
			else
			{
				if (turnLeft)
				{
					switch (_agentDir)
					{
					case Direction::North:
						_agentDir = Direction::West;
						break;
					case Direction::East:
						_agentDir = Direction::North;
						break;
					case Direction::South:
						_agentDir = Direction::East;
						break;
					case Direction::West:
						_agentDir = Direction::South;
						break;
					};
				}
				else
				{
					switch (_agentDir)
					{
					case Direction::North:
						_agentDir = Direction::East;
						break;
					case Direction::East:
						_agentDir = Direction::South;
						break;
					case Direction::South:
						_agentDir = Direction::West;
						break;
					case Direction::West:
						_agentDir = Direction::North;
						break;
					};
				}
			}
		}
		bool agentCollectSample()
		{
			_agentPower -= powerConsume[COLLECT_SAMPLE];

			int si = -1, sx, sy;
			sx = _agentXY.first;
			sy = _agentXY.second;

			switch (_agentDir)
			{
			case Direction::North:
				sy++;
				break;
			case Direction::East:
				sx++;
				break;
			case Direction::South:
				sy--;
				break;
			case Direction::West:
				sx--;
				break;
			};

			for (int i = 0; i < (int)_samplesXY.size(); i++)
			{
				if (_samplesXY[i].first == sx && _samplesXY[i].second == sy)
				{
					si = i;
				}
			}

			if (si < 0)
			{
				return false;
			}
			else
			{
				_samplesXY.erase(_samplesXY.begin() + si);
				return true;
			}
		}
		void wait()
		{
			_agentPower -= powerConsume[WAIT];
		}

		std::string getId()
		{
			std::stringstream ss;
			ss << _agentXY.first << _agentXY.second << _agentPower
				<< _curTOD << _windDir << _agentDir;

			for (int i = 0; i < (int)_samplesXY.size(); i++)
			{
				ss << _samplesXY[i].first << _samplesXY[i].second;
			}

			return ss.str();
		}
		std::string getPartialID()
		{
			std::stringstream ss;
			ss << _agentXY.first << _agentXY.second;

			return ss.str();
		}
		const int getMapWidth() { return _mapWidth; }
		const int getMapHeight() { return _mapHeight; }
		const Terrain getTerrainAt(const int x, const int y)
		{
			if (x < _mapWidth && x >= 0 && y < _mapHeight && y >=0)
				return _map[y][x];
			else
				return Terrain::Error;
		}
		const int getSampleSize()
		{
			return _samplesXY.size();
		}
		std::pair<int, int> getSampleLoc(int i)
		{
			return _samplesXY[i];
		}
		const int getAgentX() { return _agentXY.first; }
		const int getAgentY() { return _agentXY.second; }
		const Direction getAgentDir() { return _agentDir; }
		const Direction getWindDir() { return _windDir; }

		float qvals[NUM_ROVER_ACTIONS];

		static State* retrieve(State* s)
		{
			if (_searchedStates.find(s->getId()) == _searchedStates.end())
			{
				_searchedStates[s->getId()] = s;
			}

			if (stateAvgs.find(s->getPartialID()) == stateAvgs.end())
			{
				StateAverages avg;
				avg.agentPower = (float)s->_agentPower;
				avg.samplesLeft = (float)s->_samplesXY.size();
				avg.timeOfDay = (float)s->_curTOD;
				avg.windDirection = (float)s->_windDir;
				avg.agentDirection = (float)s->_agentDir;

				for (int i = 0; i < NUM_ROVER_ACTIONS; i++)
				{
					avg.qvals[i] = s->qvals[i];
				}

				avg.count = 1;

				avg.state = new State(*s);
				stateAvgs[s->getPartialID()] = avg;

				if (maxAvgPower < avg.agentPower)
					maxAvgPower = avg.agentPower;
				if (maxAvgNumSamp < avg.samplesLeft)
					maxAvgNumSamp = avg.samplesLeft;
				if (maxAvgTOD < avg.timeOfDay)
					maxAvgTOD = avg.timeOfDay;
				if (maxAvgWDir < avg.windDirection)
					maxAvgWDir = avg.windDirection;
				if (maxAvgADir < avg.agentDirection)
					maxAvgADir = avg.agentDirection;
				for (int i = 0; i < NUM_ROVER_ACTIONS; i++)
					if (maxAvgQVals[i] < abs(avg.qvals[i]))
						maxAvgQVals[i] = abs(avg.qvals[i]);
			}
			else
			{
				stateAvgs[s->getPartialID()].agentPower = ((float)s->_agentPower + 
					stateAvgs[s->getPartialID()].count * stateAvgs[s->getPartialID()].agentPower) / 
					(stateAvgs[s->getPartialID()].count + 1);
				stateAvgs[s->getPartialID()].samplesLeft = ((float)s->_samplesXY.size() + 
					stateAvgs[s->getPartialID()].count * stateAvgs[s->getPartialID()].samplesLeft) / 
					(stateAvgs[s->getPartialID()].count + 1);
				stateAvgs[s->getPartialID()].timeOfDay = ((float)s->_curTOD + 
					stateAvgs[s->getPartialID()].count * stateAvgs[s->getPartialID()].timeOfDay) / 
					(stateAvgs[s->getPartialID()].count + 1);
				stateAvgs[s->getPartialID()].windDirection = ((float)s->_windDir + 
					stateAvgs[s->getPartialID()].count * stateAvgs[s->getPartialID()].windDirection) / 
					(stateAvgs[s->getPartialID()].count + 1);
				stateAvgs[s->getPartialID()].agentDirection = ((float)s->_agentDir + 
					stateAvgs[s->getPartialID()].count * stateAvgs[s->getPartialID()].agentDirection) / 
					(stateAvgs[s->getPartialID()].count + 1);

				for (int i = 0; i < NUM_ROVER_ACTIONS; i++)
				{
					stateAvgs[s->getPartialID()].qvals[i] += s->qvals[i];
				}

				stateAvgs[s->getPartialID()].count++;

				if (maxAvgPower < stateAvgs[s->getPartialID()].agentPower)
					maxAvgPower = stateAvgs[s->getPartialID()].agentPower;
				if (maxAvgNumSamp < stateAvgs[s->getPartialID()].samplesLeft)
					maxAvgNumSamp = stateAvgs[s->getPartialID()].samplesLeft;
				if (maxAvgTOD < stateAvgs[s->getPartialID()].timeOfDay)
					maxAvgTOD = stateAvgs[s->getPartialID()].timeOfDay;
				if (maxAvgWDir < stateAvgs[s->getPartialID()].windDirection)
					maxAvgWDir = stateAvgs[s->getPartialID()].windDirection;
				if (maxAvgADir < stateAvgs[s->getPartialID()].agentDirection)
					maxAvgADir = stateAvgs[s->getPartialID()].agentDirection;
				for (int i = 0; i < NUM_ROVER_ACTIONS; i++)
					if (maxAvgQVals[i] < abs(stateAvgs[s->getPartialID()].qvals[i]))
						maxAvgQVals[i] = abs(stateAvgs[s->getPartialID()].qvals[i]);
			}
			maxAvgCount++;

			return _searchedStates[s->getId()];
		}
		static void reset()
		{
			_searchedStates.erase(_searchedStates.begin(), _searchedStates.end());
		}
	private:
		Direction itod(int i)
		{
			switch (i)
			{
			case 0:
				return Direction::North;
			case 1:
				return Direction::East;
			case 2:
				return Direction::South;
			case 3:
				return Direction::West;
			default:
				return Direction::None;
			};
		}
		TimeOfDay itotod(int i)
		{
			switch (i)
			{
			case 0:
				return TimeOfDay::Morning;
			case 1:
				return TimeOfDay::Noon;
			case 2:
				return TimeOfDay::Dusk;
			case 3:
				return TimeOfDay::Night;
			default:
				return TimeOfDay::Unspecified;
			};
		}

		int _mapWidth, _mapHeight, _agentPower, _timesteps;
		std::vector<std::vector<Terrain>> _map;
		std::vector<std::pair<int, int>> _samplesXY;
		std::pair<int, int> _agentXY;
		TimeOfDay _curTOD;
		Direction _windDir, _agentDir;
		bool _atPit;

		static std::map<std::string, State*> _searchedStates;
	};

	int State::rewards[NUM_REWARDS];
	int State::powerConsume[NUM_ROVER_ACTIONS];
	int State::powerIncrease[NUM_TIME_PERIODS];
	int State::stationPowerRate;
	int State::stepsPerPeriod;
	float State::maxQVal;
	float State::maxAvgPower;
	float State::maxAvgNumSamp;
	float State::maxAvgTOD;
	float State::maxAvgWDir;
	float State::maxAvgADir;
	float State::maxAvgQVals[NUM_ROVER_ACTIONS];
	int State::maxAvgCount;
	std::map<std::string, State*> State::_searchedStates;
	std::map<std::string, StateAverages> State::stateAvgs;
}

#endif