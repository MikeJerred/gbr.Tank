#include <algorithm>
#include <GWCA/GWCA.h>
#include <GWCA/Managers/AgentMgr.h>
#include <GWCA/Managers/MapMgr.h>
#include <GWCA/Managers/SkillbarMgr.h>

#include "Pathing/PathSearch.h"
#include "AgentUtility.h"
#include "RunUtility.h"

namespace gbr::Tank::Utilities {
	awaitable<void> RunUtility::FollowWaypoints(std::vector<std::vector<GW::GamePos>>& waypoints, std::function<void()> afterSleepCheck) {
		auto playerPos = GW::Agents::GetPlayer()->pos;

		auto closest = *std::min_element(waypoints.cbegin(), waypoints.cend(), [=](std::vector<GW::GamePos> listA, std::vector<GW::GamePos> listB) {

			auto closestA = *std::min_element(listA.cbegin(), listA.cend(), [=](GW::GamePos a, GW::GamePos b) {
				return playerPos.SquaredDistanceTo(a) < playerPos.SquaredDistanceTo(b);
			});

			auto closestB = *std::min_element(listA.cbegin(), listA.cend(), [=](GW::GamePos a, GW::GamePos b) {
				return playerPos.SquaredDistanceTo(a) < playerPos.SquaredDistanceTo(b);
			});

			return playerPos.SquaredDistanceTo(closestA) < playerPos.SquaredDistanceTo(closestB);
		});

		co_await FollowWaypoints(closest, afterSleepCheck);
	}

	awaitable<void> RunUtility::FollowWaypoints(std::vector<GW::GamePos>& waypoints, std::function<void()> afterSleepCheck) {
		auto playerPos = GW::Agents::GetPlayer()->pos;

		auto currentIterater = std::min_element(waypoints.cbegin(), waypoints.cend(), [=](GW::GamePos a, GW::GamePos b) {
			return playerPos.SquaredDistanceTo(a) < playerPos.SquaredDistanceTo(b);
		});

		for (; currentIterater < waypoints.cend(); currentIterater++) {
			auto currentWaypoint = *currentIterater;

			GW::Agents::Move(currentWaypoint);
			while (GW::Agents::GetPlayer()->pos.DistanceTo(currentWaypoint) > GW::Constants::Range::Adjacent) {
				co_await Sleep(50);

				if (GW::Map::GetInstanceType() == GW::Constants::InstanceType::Loading)
					return;

				if (afterSleepCheck)
					afterSleepCheck();
			}
		}
	}

	awaitable<void> RunUtility::FollowWaypointsWithoutStuck(std::vector<GW::GamePos>& waypoints, const std::function<awaitable<void>()> afterSleepCheck, int stuckTimeout) {
		auto playerPos = GW::Agents::GetPlayer()->pos;

		auto currentIterater = std::min_element(waypoints.cbegin(), waypoints.cend(), [=](GW::GamePos a, GW::GamePos b) {
			return playerPos.SquaredDistanceTo(a) < playerPos.SquaredDistanceTo(b);
		});

		static const int sqRange = 200 * 200;

		for (; currentIterater < waypoints.cend(); currentIterater++) {
			auto currentWaypoint = *currentIterater;

			while (GW::Agents::GetPlayer()->pos.SquaredDistanceTo(currentWaypoint) > sqRange) {
				GW::Agents::Move(Pathing::PathSearch::FindNextBestWaypoint(currentWaypoint));

				co_await Sleep(100);
				if (afterSleepCheck)
					co_await afterSleepCheck();

				if (GW::Map::GetInstanceType() == GW::Constants::InstanceType::Loading)
					return;

				if (GW::Agents::GetPlayer()->pos.SquaredDistanceTo(currentWaypoint) <= sqRange)
					break;

				if (AgentUtility::GetEnemiesInRange(currentWaypoint.x, currentWaypoint.y, 100).size() > 0
					&& GW::Agents::GetPlayer()->pos.SquaredDistanceTo(currentWaypoint) <= GW::Constants::SqrRange::Area) {
					break;
				}

				int stuckFor = 0;
				while (GW::Agents::GetPlayer()->MoveX < 1.0f && GW::Agents::GetPlayer()->MoveY < 1.0f) {
					// we have stopped moving
					GW::Agents::Move(Pathing::PathSearch::FindNextBestWaypoint(currentWaypoint));

					co_await Sleep(50);
					if (afterSleepCheck)
						co_await afterSleepCheck();

					if (GW::Agents::GetPlayer()->pos.SquaredDistanceTo(currentWaypoint) <= sqRange)
						break;

					stuckFor++;

					if (stuckFor > 50) {
						// assume we are stuck, try to use hos to get unstuck
						GW::SkillbarMgr::UseSkillByID((DWORD)GW::Constants::SkillID::Heart_of_Shadow, FindBestHoSTarget(currentWaypoint));

						co_await Sleep(150);
						if (afterSleepCheck)
							co_await afterSleepCheck();

						break;
					}
				}
			}
		}
	}

	DWORD RunUtility::FindBestHoSTarget(GW::GamePos& posToGoTowards) {
		static const int hosRange = 320;
		static const float pi = 3.1415926;

		auto playerPos = GW::Agents::GetPlayer()->pos;
		auto agents = AgentUtility::GetEnemiesInRange(playerPos.x, playerPos.y, GW::Constants::Range::Earshot);

		auto idealAngle = playerPos.AngleBetween(posToGoTowards);

		float bestAngle = wrapAngle(idealAngle + pi/2.0);
		DWORD bestAgentId = 0;

		for (auto agent : agents) {
			auto angle = playerPos.AngleBetween(agent->pos);
			if (abs(idealAngle - angle) < abs(idealAngle - bestAngle)) {
				bestAgentId = agent->Id;
				bestAngle = angle;
			}
		}

		return bestAgentId;
	}
}