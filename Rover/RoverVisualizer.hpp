#ifndef ROVER_VISUALIZER_HPP
#define ROVER_VISUALIZER_HPP

#include "RoverPolicyGenerator.hpp"
#include "SFML\Graphics.hpp"

namespace Rover
{
	struct Line
	{
		sf::Vertex pts[2];
	};
	struct AvgViewElem
	{
		Line agentPower;
		Line samplesLeft;
		Line timeOfDay;
		Line windDirection;
		Line agentDirection;
		Line qvals[NUM_ROVER_ACTIONS];
	};
	struct TrajWeights
	{
		int NorthCount;
		int SouthCount;
		int EastCount;
		int WestCount;
		int IdleCount;
		int RunsTerminatedCount;
	};
	struct LineP
	{
		Line line;
		int tx1;
		int tx2;
		int ty1;
		int ty2;
	};

	class Visualizer : public sf::Drawable
	{
	public:
		Visualizer(int winWidth, int winHeight, PolicyGenerator* pg) : 
			_tileWidth(winWidth / (float)pg->getBaseState()->getMapWidth()), 
			_tileHeight(winHeight / (float)pg->getBaseState()->getMapHeight()), 
			_agentView(3), _sampleCount(0), _arrowWidth(40.0f), _arrowHeight(40.0f),
			_windView(3), UpdatedPG(false), _moveCount(0), _maxMoveWeight(0)
		{
			_pgPtr = pg;

			initGridView();
			initSampleView();
			initAgentView();
			initAvgView();
			initWindView();
			initLinePlot();
			initPaths();
		}
		~Visualizer()
		{

		}

		void update(State* curState, State* prevState)
		{
			_sampleCount = curState->getSampleSize();
			for (int k = 0; k < curState->getSampleSize(); k++)
			{
				std::pair<int, int> xy = curState->getSampleLoc(k);
				_sampleView[k].setOrigin(_sampleView[k].getRadius() / 2.0f, 
					_sampleView[k].getRadius() / 2.0f);
				_sampleView[k].setPosition(_tileWidth * xy.first + 
					_sampleView[k].getRadius() / 2.0f, 
					_tileHeight * xy.second + 
					_sampleView[k].getRadius() / 2.0f);
			}

			_agentView.setPosition(_tileWidth * curState->getAgentX() + (_tileWidth / 2.0f),
			_tileHeight * curState->getAgentY() + (_tileHeight / 2.0f));
			_agentView.setRotation(90.0f * curState->getAgentDir());

			_windView.setPosition(_tileWidth * curState->getAgentX() + (_tileWidth / 2.0f),
			_tileHeight * curState->getAgentY() + (_tileHeight / 2.0f));
			_windView.setRotation(90.0f * curState->getWindDir());

			if (DrawStatView)
			{
				for (auto ele = Rover::State::stateAvgs.begin(); ele != Rover::State::stateAvgs.end(); ++ele)
				{
					int i = ele->second.state->getAgentY();
					int j = ele->second.state->getAgentX();
					_avgView[i][j].agentPower.pts[1].position.y = _avgView[i][j].agentPower.pts[0].position.y;
					_avgView[i][j].samplesLeft.pts[1].position.y = _avgView[i][j].samplesLeft.pts[0].position.y;
					_avgView[i][j].timeOfDay.pts[1].position.y = _avgView[i][j].timeOfDay.pts[0].position.y;
					_avgView[i][j].windDirection.pts[1].position.y = _avgView[i][j].windDirection.pts[0].position.y;
					_avgView[i][j].agentDirection.pts[1].position.y = _avgView[i][j].agentDirection.pts[0].position.y;
					for (int k = 0; k < NUM_ROVER_ACTIONS; k++)
					{
						_avgView[i][j].qvals[k].pts[1].position.y = _avgView[i][j].qvals[k].pts[0].position.y;
					}

					_avgView[i][j].agentPower.pts[1].position.y -= (ele->second.agentPower) / State::maxAvgPower * (_tileHeight / 2.0f);
					_avgView[i][j].samplesLeft.pts[1].position.y -= (ele->second.samplesLeft) / State::maxAvgNumSamp * (_tileHeight / 2.0f);
					_avgView[i][j].timeOfDay.pts[1].position.y -= (ele->second.timeOfDay) / State::maxAvgTOD * (_tileHeight / 2.0f);
					_avgView[i][j].windDirection.pts[1].position.y -= (ele->second.windDirection) / State::maxAvgWDir * (_tileHeight / 2.0f);
					_avgView[i][j].agentDirection.pts[1].position.y -= (ele->second.agentDirection) / State::maxAvgADir * (_tileHeight / 2.0f);
					for (int k = 0; k < NUM_ROVER_ACTIONS; k++)
					{
						_avgView[i][j].qvals[k].pts[1].position.y -= (ele->second.qvals[k]) / State::maxAvgQVals[k] * (_tileHeight / 2.0f);
					}
				}
			}

			if (DrawVStatView)
			{
				for (auto ele = Rover::State::stateAvgs.begin(); ele != Rover::State::stateAvgs.end(); ++ele)
				{
					int i = ele->second.state->getAgentY();
					int j = ele->second.state->getAgentX();
					_avgWindView[i][j].setRotation(90.0f * ele->second.windDirection);
					_avgAgentView[i][j].setRotation(90.0f * ele->second.agentDirection);

					sf::Color curColW = _avgWindView[i][j].getFillColor();
					sf::Color curColA = _avgAgentView[i][j].getFillColor();
					_avgWindView[i][j].setFillColor(sf::Color(curColW.r, curColW.g, curColW.b, 128));
					_avgAgentView[i][j].setFillColor(sf::Color(curColA.r, curColA.g, curColA.b, 128));
				}
			}

			if (UpdatedPG)
			{
				if (prevState->getAgentX() == curState->getAgentX() && 
					prevState->getAgentY() == curState->getAgentY())
				{
					_linePlotView[curState->getAgentY()][curState->getAgentX()].IdleCount++;
				}
				else
				{
					if (prevState->getAgentX() < curState->getAgentX())
					{
						_linePlotView[prevState->getAgentY()][prevState->getAgentX()].EastCount++;

						if (_linePlotView[prevState->getAgentY()][prevState->getAgentX()].EastCount > _maxMoveWeight)
							_maxMoveWeight = _linePlotView[prevState->getAgentY()][prevState->getAgentX()].EastCount;
					}
					else if (prevState->getAgentX() > curState->getAgentX())
					{
						_linePlotView[prevState->getAgentY()][prevState->getAgentX()].WestCount++;

						if (_linePlotView[prevState->getAgentY()][prevState->getAgentX()].WestCount > _maxMoveWeight)
							_maxMoveWeight = _linePlotView[prevState->getAgentY()][prevState->getAgentX()].WestCount;
					}
					else if (prevState->getAgentY() < curState->getAgentY())
					{
						_linePlotView[prevState->getAgentY()][prevState->getAgentX()].NorthCount++;

						if (_linePlotView[prevState->getAgentY()][prevState->getAgentX()].NorthCount > _maxMoveWeight)
							_maxMoveWeight = _linePlotView[prevState->getAgentY()][prevState->getAgentX()].NorthCount;
					}
					else if (prevState->getAgentY() > curState->getAgentY())
					{
						_linePlotView[prevState->getAgentY()][prevState->getAgentX()].SouthCount++;

						if (_linePlotView[prevState->getAgentY()][prevState->getAgentX()].SouthCount > _maxMoveWeight)
							_maxMoveWeight = _linePlotView[prevState->getAgentY()][prevState->getAgentX()].SouthCount;
					}

					_moveCount++;

					LineP l;
					l.tx1 = prevState->getAgentX();
					l.ty1 = prevState->getAgentY();
					l.tx2 = curState->getAgentX();
					l.ty2 = curState->getAgentY();
					l.line.pts[0].position.x = l.tx1 * _tileWidth + _tileWidth / 2.0f;
					l.line.pts[0].position.y = l.ty1 * _tileHeight + _tileHeight / 2.0f;
					l.line.pts[1].position.x = l.tx2 * _tileWidth + _tileWidth / 2.0f;
					l.line.pts[1].position.y = l.ty2 * _tileHeight + _tileHeight / 2.0f;
					l.line.pts[0].color = sf::Color::Magenta;
					l.line.pts[1].color = sf::Color::Magenta;
					_curPath->push_back(l);
				}

				if (curState->isTerminalState())
				{
					_linePlotView[curState->getAgentY()][curState->getAgentX()].RunsTerminatedCount++;

					_paths.push_back(*_curPath);
					_curPath = new std::vector<LineP>();
				}
			}

			if (DrawLinePlot)
			{
				float count = RelativeCompare ? _maxMoveWeight : _moveCount;

				for (int i = 0; i < (int)_linePlotRowShapes.size(); i++)
				{
					for (int j = 0; j < (int)_linePlotRowShapes[i].size(); j++)
					{
						if (j < _pgPtr->getBaseState()->getMapWidth())
						{
							float height = _linePlotView[i][j].WestCount / 
								(float)_maxMoveWeight * 
								(_tileWidth < _tileHeight ? _tileWidth : _tileHeight);
							sf::Vector2f pt0 = _linePlotRowShapes[i][j].getPoint(0);
							sf::Vector2f pt1 = _linePlotRowShapes[i][j].getPoint(1);
							pt0.y = -height / 2.0f;
							pt1.y = height / 2.0f;

							_linePlotRowShapes[i][j].setPoint(0, pt0);
							_linePlotRowShapes[i][j].setPoint(1, pt1);
						}

						if (j - 1 >= 0)
						{
							float height = _linePlotView[i][j - 1].EastCount / 
								(float)_maxMoveWeight * 
								(_tileWidth < _tileHeight ? _tileWidth : _tileHeight);
							sf::Vector2f pt2 = _linePlotRowShapes[i][j].getPoint(2);
							sf::Vector2f pt3 = _linePlotRowShapes[i][j].getPoint(3);
							pt3.y = -height / 2.0f;
							pt2.y = height / 2.0f;

							_linePlotRowShapes[i][j].setPoint(2, pt2);
							_linePlotRowShapes[i][j].setPoint(3, pt3);
						}
					}
				}

				for (int i = 0; i < (int)_linePlotColShapes.size(); i++)
				{
					for (int j = 0; j < (int)_linePlotColShapes[i].size(); j++)
					{
						if (i - 1 >= 0)
						{
							float width = _linePlotView[i - 1][j].SouthCount / 
								(float)_maxMoveWeight * 
								(_tileWidth < _tileHeight ? _tileWidth : _tileHeight);
							sf::Vector2f pt0 = _linePlotColShapes[i][j].getPoint(2);
							sf::Vector2f pt1 = _linePlotColShapes[i][j].getPoint(3);
							pt0.x = -width / 2.0f;
							pt1.x = width / 2.0f;

							_linePlotColShapes[i][j].setPoint(2, pt0);
							_linePlotColShapes[i][j].setPoint(3, pt1);
						}

						if (i < _pgPtr->getBaseState()->getMapWidth())
						{
							float width = _linePlotView[i][j].NorthCount / 
								(float)_maxMoveWeight * 
								(_tileWidth < _tileHeight ? _tileWidth : _tileHeight);
							sf::Vector2f pt2 = _linePlotColShapes[i][j].getPoint(0);
							sf::Vector2f pt3 = _linePlotColShapes[i][j].getPoint(1);
							pt3.x = -width / 2.0f;
							pt2.x = width / 2.0f;

							_linePlotColShapes[i][j].setPoint(0, pt2);
							_linePlotColShapes[i][j].setPoint(1, pt3);
						}
					}
				}

				for (int i = 0; i < (int)_linePlotCirShapes.size(); i++)
				{
					float y = _tileHeight * i + (_tileHeight / 2.0f);
					for (int j = 0; j < (int)_linePlotCirShapes[i].size(); j++)
					{
						float x = _tileWidth * j + _tileWidth / 2.0f;
						float radius = _linePlotView[i][j].IdleCount / (float)State::maxAvgCount *
							(_tileWidth < _tileHeight ? _tileWidth : _tileHeight);

						_linePlotCirShapes[i][j].setRadius(radius);
						_linePlotCirShapes[i][j].setOrigin(radius / 2.0f, radius / 2.0f);
						_linePlotCirShapes[i][j].setPosition(x - radius / 2.0f, y - radius / 2.0f);
					}
				}
			}
		}
		virtual void draw(sf::RenderTarget &target, sf::RenderStates states) const
		{
			if (DrawRawView)
			{
				for (int i = 0; i < _pgPtr->getBaseState()->getMapHeight(); i++)
					for (int j = 0; j < _pgPtr->getBaseState()->getMapWidth(); j++)
						target.draw(_gridView[i][j]);

				for (int k = 0; k < _sampleCount; k++)
				{
					target.draw(_sampleView[k]);
				}

				if (!DrawVStatView)
				{
					target.draw(_agentView);
					target.draw(_windView);
				}
			}

			if (DrawStatView)
			{
				for (int i = 0; i < _pgPtr->getBaseState()->getMapHeight(); i++)
				{
					for (int j = 0; j < _pgPtr->getBaseState()->getMapWidth(); j++)
					{
						target.draw((sf::Vertex*)(_avgView[i][j].agentPower.pts), 2, sf::PrimitiveType::Lines);
						target.draw((sf::Vertex*)(_avgView[i][j].samplesLeft.pts), 2, sf::PrimitiveType::Lines);
						target.draw((sf::Vertex*)(_avgView[i][j].timeOfDay.pts), 2, sf::PrimitiveType::Lines);
						target.draw((sf::Vertex*)(_avgView[i][j].windDirection.pts), 2, sf::PrimitiveType::Lines);
						target.draw((sf::Vertex*)(_avgView[i][j].agentDirection.pts), 2, sf::PrimitiveType::Lines);
						target.draw((sf::Vertex*)(_avgView[i][j].qvals[0].pts), 2, sf::PrimitiveType::Lines);
						target.draw((sf::Vertex*)(_avgView[i][j].qvals[1].pts), 2, sf::PrimitiveType::Lines);
						target.draw((sf::Vertex*)(_avgView[i][j].qvals[2].pts), 2, sf::PrimitiveType::Lines);
						target.draw((sf::Vertex*)(_avgView[i][j].qvals[3].pts), 2, sf::PrimitiveType::Lines);
						target.draw((sf::Vertex*)(_avgView[i][j].qvals[4].pts), 2, sf::PrimitiveType::Lines);
					}
				}
			}

			if (DrawVStatView)
			{
				for (int i = 0; i < (int)_avgWindView.size(); i++)
				{
					for (int j = 0; j < (int)_avgWindView[i].size(); j++)
					{
						target.draw(_avgWindView[i][j]);
					}
				}

				for (int i = 0; i < (int)_avgAgentView.size(); i++)
				{
					for (int j = 0; j < (int)_avgAgentView[i].size(); j++)
					{
						target.draw(_avgAgentView[i][j]);
					}
				}
			}

			if (DrawLinePlot)
			{
				for (int i = 0; i < (int)_linePlotRowShapes.size(); i++)
				{
					for (int j = 0; j < (int)_linePlotRowShapes[i].size(); j++)
					{
						target.draw(_linePlotRowShapes[i][j]);
					}
				}

				for (int i = 0; i < (int)_linePlotColShapes.size(); i++)
				{
					for (int j = 0; j < (int)_linePlotColShapes[i].size(); j++)
					{
						target.draw(_linePlotColShapes[i][j]);
					}
				}

				for (int i = 0; i < (int)_linePlotCirShapes.size(); i++)
				{
					for (int j = 0; j < (int)_linePlotCirShapes[i].size(); j++)
					{
						target.draw(_linePlotCirShapes[i][j]);
					}
				}
			}

			if (DrawPaths)
			{
				for (int i = 0; i < (int)_paths.size(); i++)
				{
					for (int j = 0; j < (int)_paths[i].size(); j++)
					{
						target.draw(_paths[i][j].line.pts, 2, sf::PrimitiveType::Lines);
					}
				}

				for (int i = 0; i < (int)_curPath->size(); i++)
				{
					target.draw((*_curPath)[i].line.pts, 2, sf::PrimitiveType::Lines);
				}
			}
		}

		static bool DrawRawView;
		static bool DrawStatView;
		static bool DrawVStatView;
		static bool DrawLinePlot;
		static bool DrawPaths;
		static bool RelativeCompare;
		bool UpdatedPG;
	private:
		void initGridView()
		{
			_gridView.resize(_pgPtr->getBaseState()->getMapHeight());
			for (int i = 0; i < _pgPtr->getBaseState()->getMapHeight(); i++)
			{
				_gridView[i].resize(_pgPtr->getBaseState()->getMapWidth());
				float y = _tileHeight * i;
				for (int j = 0; j < _pgPtr->getBaseState()->getMapWidth(); j++)
				{
					sf::RectangleShape rectShape(sf::Vector2f(_tileWidth, _tileHeight));
					float x = _tileWidth * j;
					rectShape.setPosition(x, y);
			
					switch (_pgPtr->getBaseState()->getTerrainAt(j, i))
					{
					case Rover::Terrain::Flat:
						rectShape.setFillColor(sf::Color::White);
						break;
					case Rover::Terrain::Pit:
						rectShape.setFillColor(sf::Color::Red);
						break;
					case Rover::Terrain::Rock:
						rectShape.setFillColor(sf::Color::Black);
						break;
					case Rover::Terrain::Station:
						rectShape.setFillColor(sf::Color::Green);
						break;
					default:
						rectShape.setFillColor(sf::Color::Cyan);
						break;
					};

					_gridView[i][j] = rectShape;
				}
			}
		}
		void initSampleView()
		{
			for (int i = 0; i < _pgPtr->getBaseState()->getSampleSize(); i++)
			{
				sf::CircleShape circ((_tileWidth < _tileHeight) ? _tileWidth / 2.0f : _tileHeight / 2.0f);
				circ.setFillColor(sf::Color::Yellow);
				_sampleView.push_back(circ);
			}
		}
		void initAgentView()
		{
			float size = (_tileWidth < _tileHeight) ? _tileWidth : _tileHeight;
			_agentView.setPoint(0, sf::Vector2f(0, -size / 2.0f));
			_agentView.setPoint(1, sf::Vector2f(size / 2.0f, size / 2.0f));
			_agentView.setPoint(2, sf::Vector2f(-size / 2.0f, size / 2.0f));
			_agentView.setFillColor(sf::Color::Blue);

			_avgAgentView.resize(_pgPtr->getBaseState()->getMapHeight());
			for (int i = 0; i < _pgPtr->getBaseState()->getMapHeight(); i++)
			{
				_avgAgentView[i].resize(_pgPtr->getBaseState()->getMapWidth());
				float ty = _tileHeight * (i + 1) - (_tileHeight / 2.0f);
				for (int j = 0; j < _pgPtr->getBaseState()->getMapWidth(); j++)
				{
					float tx = _tileWidth * j + _tileWidth / 2.0f;
					sf::ConvexShape arrow(3);
					arrow.setPoint(0, sf::Vector2f(0, -size / 2.0f));
					arrow.setPoint(1, sf::Vector2f(size / 4.0f, 0));
					arrow.setPoint(2, sf::Vector2f(-size / 4.0f, 0));
					arrow.setFillColor(sf::Color(sf::Color::Blue.r, sf::Color::Blue.g, sf::Color::Blue.b, 0));
					arrow.setPosition(tx, ty);

					_avgAgentView[i][j] = arrow;
				}
			}
		}
		void initAvgView()
		{
			int offsetFactor = 3;
			_avgView.resize(_pgPtr->getBaseState()->getMapHeight());
			for (int i = 0; i < _pgPtr->getBaseState()->getMapHeight(); i++)
			{
				_avgView[i].resize(_pgPtr->getBaseState()->getMapWidth());
				float y = _tileHeight * i + (_tileHeight / 2.0f);
				for (int j = 0; j < _pgPtr->getBaseState()->getMapWidth(); j++)
				{
					float x = _tileWidth * j + (_tileWidth / 2.0f) - offsetFactor * 5;

					AvgViewElem ave;
					ave.agentPower.pts[0].position = sf::Vector2f(x = x + 1, y);
					ave.samplesLeft.pts[0].position = sf::Vector2f(x = x + offsetFactor, y);
					ave.timeOfDay.pts[0].position = sf::Vector2f(x = x + offsetFactor, y);
					ave.windDirection.pts[0].position = sf::Vector2f(x = x + offsetFactor, y);
					ave.agentDirection.pts[0].position = sf::Vector2f(x = x + offsetFactor, y);
					ave.qvals[0].pts[0].position = sf::Vector2f(x = x + offsetFactor, y);
					ave.qvals[1].pts[0].position = sf::Vector2f(x = x + offsetFactor, y);
					ave.qvals[2].pts[0].position = sf::Vector2f(x = x + offsetFactor, y);
					ave.qvals[3].pts[0].position = sf::Vector2f(x = x + offsetFactor, y);
					ave.qvals[4].pts[0].position = sf::Vector2f(x = x + offsetFactor, y);

					x = _tileWidth * j + (_tileWidth / 2.0f) - offsetFactor * 5;
					ave.agentPower.pts[1].position = sf::Vector2f(x = x + 1, y);
					ave.samplesLeft.pts[1].position = sf::Vector2f(x = x + offsetFactor, y);
					ave.timeOfDay.pts[1].position = sf::Vector2f(x = x + offsetFactor, y);
					ave.windDirection.pts[1].position = sf::Vector2f(x = x + offsetFactor, y);
					ave.agentDirection.pts[1].position = sf::Vector2f(x = x + offsetFactor, y);
					ave.qvals[0].pts[1].position = sf::Vector2f(x = x + offsetFactor, y);
					ave.qvals[1].pts[1].position = sf::Vector2f(x = x + offsetFactor, y);
					ave.qvals[2].pts[1].position = sf::Vector2f(x = x + offsetFactor, y);
					ave.qvals[3].pts[1].position = sf::Vector2f(x = x + offsetFactor, y);
					ave.qvals[4].pts[1].position = sf::Vector2f(x = x + offsetFactor, y);

					ave.agentPower.pts[0].color = sf::Color::Black;
					ave.samplesLeft.pts[0].color = sf::Color::Black;
					ave.timeOfDay.pts[0].color = sf::Color::Black;
					ave.windDirection.pts[0].color = sf::Color::Black;
					ave.agentDirection.pts[0].color = sf::Color::Black;
					ave.qvals[0].pts[0].color = sf::Color::Black;
					ave.qvals[1].pts[0].color = sf::Color::Black;
					ave.qvals[2].pts[0].color = sf::Color::Black;
					ave.qvals[3].pts[0].color = sf::Color::Black;
					ave.qvals[4].pts[0].color = sf::Color::Black;

					ave.agentPower.pts[1].color = sf::Color::Black;
					ave.samplesLeft.pts[1].color = sf::Color::Black;
					ave.timeOfDay.pts[1].color = sf::Color::Black;
					ave.windDirection.pts[1].color = sf::Color::Black;
					ave.agentDirection.pts[1].color = sf::Color::Black;
					ave.qvals[0].pts[1].color = sf::Color::Black;
					ave.qvals[1].pts[1].color = sf::Color::Black;
					ave.qvals[2].pts[1].color = sf::Color::Black;
					ave.qvals[3].pts[1].color = sf::Color::Black;
					ave.qvals[4].pts[1].color = sf::Color::Black;

					_avgView[i][j] = ave;
				}
			}
		}
		void initWindView()
		{
			float size = (_tileWidth < _tileHeight) ? _tileWidth : _tileHeight;
			_windView.setPoint(0, sf::Vector2f(0, -size / 2.0f));
			_windView.setPoint(1, sf::Vector2f(size / 4.0f, 0));
			_windView.setPoint(2, sf::Vector2f(-size / 4.0f, 0));
			_windView.setFillColor(sf::Color::Cyan);

			_avgWindView.resize(_pgPtr->getBaseState()->getMapHeight());
			for (int i = 0; i < _pgPtr->getBaseState()->getMapHeight(); i++)
			{
				_avgWindView[i].resize(_pgPtr->getBaseState()->getMapWidth());
				float ty = _tileHeight * i + (_tileHeight / 2.0f);
				for (int j = 0; j < _pgPtr->getBaseState()->getMapWidth(); j++)
				{
					float tx = _tileWidth * j + _tileWidth / 2.0f;
					sf::ConvexShape arrow(3);
					arrow.setPoint(0, sf::Vector2f(0, -size / 2.0f));
					arrow.setPoint(1, sf::Vector2f(size / 8.0f, 0));
					arrow.setPoint(2, sf::Vector2f(-size / 8.0f, 0));
					arrow.setFillColor(sf::Color(sf::Color::Cyan.r, sf::Color::Cyan.b, sf::Color::Cyan.g, 0));
					arrow.setPosition(tx, ty);

					_avgWindView[i][j] = arrow;
				}
			}
		}
		void initLinePlot()
		{
			_linePlotView.resize(_pgPtr->getBaseState()->getMapHeight());
			for (int i = 0; i < _pgPtr->getBaseState()->getMapHeight(); i++)
			{
				_linePlotView[i].resize(_pgPtr->getBaseState()->getMapWidth());
				for (int j = 0; j < _pgPtr->getBaseState()->getMapWidth(); j++)
				{
					TrajWeights tw;
					tw.NorthCount = 0;
					tw.SouthCount = 0;
					tw.EastCount = 0;
					tw.WestCount = 0;
					tw.IdleCount = 0;
					tw.RunsTerminatedCount = 0;

					_linePlotView[i][j];
				}
			}

			_linePlotRowShapes.resize(_pgPtr->getBaseState()->getMapHeight());
			for (int i = 0; i < _pgPtr->getBaseState()->getMapHeight(); i++)
			{
				_linePlotRowShapes[i].resize(_pgPtr->getBaseState()->getMapWidth() + 1);
				float y = _tileHeight * i + (_tileHeight / 2.0f);
				for (int j = 0; j < _pgPtr->getBaseState()->getMapWidth() + 1; j++)
				{
					float x1 = _tileWidth * (j - 1) + _tileWidth / 2.0f;
					float x2 = _tileWidth * j + _tileWidth / 2.0f;

					_linePlotRowShapes[i][j] = sf::ConvexShape(4);
					_linePlotRowShapes[i][j].setFillColor(sf::Color::Magenta);
					_linePlotRowShapes[i][j].setPoint(0, sf::Vector2f(_tileWidth / 2.0f, 0.0f));
					_linePlotRowShapes[i][j].setPoint(1, sf::Vector2f(_tileWidth / 2.0f, 0.0f));
					_linePlotRowShapes[i][j].setPoint(2, sf::Vector2f(-_tileWidth / 2.0f, 0.0f));
					_linePlotRowShapes[i][j].setPoint(3, sf::Vector2f(-_tileWidth / 2.0f, 0.0f));
					_linePlotRowShapes[i][j].setPosition((x1 + x2) / 2, y);
				}
			}

			_linePlotColShapes.resize(_pgPtr->getBaseState()->getMapHeight() + 1);
			for (int i = 0; i < _pgPtr->getBaseState()->getMapHeight() + 1; i++)
			{
				_linePlotColShapes[i].resize(_pgPtr->getBaseState()->getMapWidth());
				float y1 = _tileHeight * (i - 1) + (_tileHeight / 2.0f);
				float y2 = _tileHeight * i + (_tileHeight / 2.0f);
				for (int j = 0; j < _pgPtr->getBaseState()->getMapWidth(); j++)
				{
					float x = _tileWidth * j + _tileWidth / 2.0f;

					_linePlotColShapes[i][j] = sf::ConvexShape(4);
					_linePlotColShapes[i][j].setFillColor(sf::Color::Magenta);
					_linePlotColShapes[i][j].setPoint(0, sf::Vector2f(0.0f, -_tileHeight / 2.0f));
					_linePlotColShapes[i][j].setPoint(1, sf::Vector2f(0.0f, -_tileHeight / 2.0f));
					_linePlotColShapes[i][j].setPoint(2, sf::Vector2f(0.0f, _tileHeight / 2.0f));
					_linePlotColShapes[i][j].setPoint(3, sf::Vector2f(0.0f, _tileHeight / 2.0f));
					_linePlotColShapes[i][j].setPosition(x, (y1 + y2) / 2);
				}
			}

			_linePlotCirShapes.resize(_pgPtr->getBaseState()->getMapHeight());
			for (int i = 0; i < _pgPtr->getBaseState()->getMapHeight(); i++)
			{
				_linePlotCirShapes[i].resize(_pgPtr->getBaseState()->getMapWidth());
				float y = _tileHeight * i + (_tileHeight / 2.0f);
				for (int j = 0; j < _pgPtr->getBaseState()->getMapWidth(); j++)
				{
					float x = _tileWidth * j + _tileWidth / 2.0f;

					_linePlotCirShapes[i][j] = sf::CircleShape();
					_linePlotCirShapes[i][j].setPosition(x, y);
					_linePlotCirShapes[i][j].setFillColor(sf::Color::Magenta);
				}
			}
		}
		void initPaths()
		{
			_curPath = new std::vector<LineP>();
		}

		PolicyGenerator* _pgPtr;
		float _tileWidth, _tileHeight;
		float _arrowWidth, _arrowHeight;
		int _sampleCount;
		int _moveCount, _maxMoveWeight;
		std::vector<std::vector<sf::RectangleShape>> _gridView;
		std::vector<sf::CircleShape> _sampleView;
		sf::ConvexShape _agentView;
		sf::ConvexShape _windView;
		std::vector<std::vector<AvgViewElem>> _avgView;
		std::vector<std::vector<sf::ConvexShape>> _avgWindView;
		std::vector<std::vector<sf::ConvexShape>> _avgAgentView;
		std::vector<std::vector<TrajWeights>> _linePlotView; //Actually the Data
		std::vector<std::vector<sf::ConvexShape>> _linePlotRowShapes;
		std::vector<std::vector<sf::ConvexShape>> _linePlotColShapes;
		std::vector<std::vector<sf::CircleShape>> _linePlotCirShapes;
		std::vector<std::vector<LineP>> _paths;
		std::vector<LineP>* _curPath;
	};

	bool Visualizer::DrawRawView;
	bool Visualizer::DrawStatView;
	bool Visualizer::DrawVStatView;
	bool Visualizer::DrawLinePlot;
	bool Visualizer::DrawPaths;
	bool Visualizer::RelativeCompare;
}

#endif