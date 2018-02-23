#pragma once

#include <list>
#include <GWCA/GWCA.h>

namespace gbr::Tank::Utilities::Pathing {

	class Square {
	private:
		static const int GridWidth = 20;
		static const int DiagonalDistance = 1.4 * GridWidth;
		static const int AvoidDistance = 72;

		int RoundToGrid(float f) {
			return (((int)f + GridWidth / 2) / GridWidth) * GridWidth;
		}

		static bool Square::IsPointInTrapezoid(float pointX, float pointY, GW::PathingTrapezoid& trap);
	public:
		int x;
		int y;
		const Square* parent;
		int distanceToReach;

		Square(GW::GamePos pos, const Square* parent, int distanceFromParent) : Square(RoundToGrid(pos.x), RoundToGrid(pos.y), parent, distanceFromParent) {
		}

		Square(int x, int y, const Square* parent, int distanceFromParent) {
			this->x = x;
			this->y = y;
			this->parent = parent;

			if (parent == nullptr)
				this->distanceToReach = 0;
			else
				this->distanceToReach = parent->distanceToReach + distanceFromParent;
		}

		bool IsCloseTo(const Square& square) const { return GetHeuristicCost(square) < 130 * 130; }
		bool IsSame(const Square& square) const { return x == square.x && y == square.y; }
		int GetScore() const  { return distanceToReach; }
		bool IsWalkable() const;
		std::vector<Square> GetAdjacentSquares() const;
		float GetHeuristicCost(const Square& destination) const;

	};

	class PathSearch {
	private:
		static std::list<Square*>::reverse_iterator GetLowestCostSquare(std::list<Square*>& squares, const Square& destination);
	public:
		static GW::GamePos FindNextBestWaypoint(GW::GamePos destination);
	};
}