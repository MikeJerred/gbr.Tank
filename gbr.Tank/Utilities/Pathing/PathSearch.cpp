#include <algorithm>
#include <GWCA/GWCA.h>
#include <GWCA/Managers/AgentMgr.h>
#include <GWCA/Managers/MapMgr.h>

#include "PathSearch.h"

namespace gbr::Tank::Utilities::Pathing {

	std::list<Square*>::reverse_iterator PathSearch::GetLowestCostSquare(std::list<Square*>& squares, const Square& destination) {
		return std::min_element(squares.rbegin(), squares.rend(), [&](Square* a, Square* b) {
			return a->GetHeuristicCost(destination) < b->GetHeuristicCost(destination);
		});
	}

	GW::GamePos PathSearch::FindNextBestWaypoint(GW::GamePos destPos) {
		auto destination = Square(destPos, nullptr, 0);

		std::list<Square*> openSquares { new Square(GW::Agents::GetPlayer()->pos, nullptr, 0) };
		auto closedSquares = std::list<const Square*>();

		while (closedSquares.size() < 800 && openSquares.size() > 0) {
			auto squareToCalculateIterator = GetLowestCostSquare(openSquares, destination);
			auto squareToCalculate = *squareToCalculateIterator;
			closedSquares.push_back(squareToCalculate);
			openSquares.erase(std::next(squareToCalculateIterator).base());

			// path found!
			if (squareToCalculate->IsCloseTo(destination)) {
				auto current = const_cast<const Square*>(squareToCalculate);
				auto parent = current->parent;

				if (!parent)
					return GW::GamePos(current->x, current->y);

				while (parent->parent) {
					current = current->parent;
					parent = parent->parent;
				}

				return GW::GamePos(current->x, current->y);
			}

			for (auto& square : squareToCalculate->GetAdjacentSquares()) {
				auto openListIterator = std::find_if(openSquares.begin(), openSquares.end(), [=](const Square* s) { return s->IsSame(square); });
				if (openListIterator != openSquares.end()) {
					if ((*openListIterator)->GetScore() > square.GetScore()) {
						delete *openListIterator;
						*openListIterator = new Square(square);
					}
				}
				else if (!std::any_of(closedSquares.begin(), closedSquares.end(), [=](const Square* s) { return s->IsSame(square); }) && square.IsWalkable()) {
					openSquares.push_back(new Square(square));
				}
			}

		}

		return destPos;
	}



	

	bool Square::IsPointInTrapezoid(float pointX, float pointY, GW::PathingTrapezoid& trap) {
		int nY = trap.YB - trap.YT;

		return pointY <= trap.YT
			&& pointY >= trap.YB
			&& (pointY - trap.YB) * (trap.XTL - trap.XBL) <= nY * (pointX - trap.XBL)
			&& (pointY - trap.YB) * (trap.XBR - trap.XTR) <= nY * (pointX - trap.XBR);
	}

	bool Square::IsWalkable() const {
		auto pmaps = GW::Map::GetPathingMap();
		bool isOnMap = false;
		for (auto pmap : pmaps) {
			for (size_t i = 0; i < pmap.trapezoidcount; ++i) {
				GW::PathingTrapezoid& trap = pmap.trapezoids[i];

				if (!IsPointInTrapezoid(x, y, trap)) {
					isOnMap = true;
					break;
				}
			}

			if (isOnMap)
				break;
		}

		if (!isOnMap)
			return false;

		auto agentArray = GW::Agents::GetAgentArray();
		if (!agentArray.valid())
			return true;

		auto pos = GW::GamePos(x, y);
		static const int sqRange = AvoidDistance * AvoidDistance;
		for (auto agent : agentArray) {
			if (agent
				&& agent->GetIsCharacterType()
				&& !agent->GetIsDead()
				&& agent->pos.SquaredDistanceTo(pos) <= sqRange) {
				return false;
			}
		}

		// todo : take into account the movement of melee enemies

		return true;
	}

	std::vector<Square> Square::GetAdjacentSquares() const {
		return std::vector<Square> {
			Square(x - GridWidth / 2, y + GridWidth / 2, this, DiagonalDistance),
			Square(x                , y + GridWidth / 2, this, GridWidth),
			Square(x + GridWidth / 2, y + GridWidth / 2, this, DiagonalDistance),
			Square(x - GridWidth / 2, y                , this, GridWidth),
			Square(x + GridWidth / 2, y                , this, GridWidth),
			Square(x - GridWidth / 2, y - GridWidth / 2, this, DiagonalDistance),
			Square(x                , y - GridWidth / 2, this, GridWidth),
			Square(x + GridWidth / 2, y - GridWidth / 2, this, DiagonalDistance)
		};
	}

	float Square::GetHeuristicCost(const Square& destination) const {
		float deltaX = x - destination.x;
		float deltaY = y - destination.y;
		return deltaX * deltaX + deltaY * deltaY;
	}
}