#include <algorithm>
#include <GWCA/GWCA.h>
#include <GWCA/Managers/AgentMgr.h>

#include "RunUtility.h"

namespace gbr::Tank::Utilities {
	concurrency::task<void> RunUtility::FollowWaypoints(std::vector<std::vector<GW::GamePos>>& waypoints) {
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

		return FollowWaypoints(closest);
	}

	concurrency::task<void> RunUtility::FollowWaypoints(std::vector<GW::GamePos>& waypoints) {
		auto playerPos = GW::Agents::GetPlayer()->pos;

		auto currentIterater = std::min_element(waypoints.cbegin(), waypoints.cend(), [=](GW::GamePos a, GW::GamePos b) {
			return playerPos.SquaredDistanceTo(a) < playerPos.SquaredDistanceTo(b);
		});

		for (; currentIterater < waypoints.cend(); currentIterater++) {
			auto currentWaypoint = *currentIterater;

			GW::Agents::Move(currentWaypoint);
			while (GW::Agents::GetPlayer()->pos.DistanceTo(currentWaypoint) > GW::Constants::Range::Adjacent) {
				co_await Sleep(50);
			}
		}
	}

	concurrency::task<void> FollowWaypointsWithoutStuck(std::vector<GW::GamePos>& waypoints) {
		auto playerPos = GW::Agents::GetPlayer()->pos;

		auto currentIterater = std::min_element(waypoints.cbegin(), waypoints.cend(), [=](GW::GamePos a, GW::GamePos b) {
			return playerPos.SquaredDistanceTo(a) < playerPos.SquaredDistanceTo(b);
		});

		for (; currentIterater < waypoints.cend(); currentIterater++) {
			auto currentWaypoint = *currentIterater;

			GW::Agents::Move(currentWaypoint);
			auto stuckTick = GetTickCount() + 10000;
			while (GW::Agents::GetPlayer()->pos.DistanceTo(currentWaypoint) > GW::Constants::Range::Adjacent) {
				co_await Sleep(50);

				if (GetTickCount() > stuckTick) {
					// assume we are stuck, try to use hos to get unstuck
					GW::SkillbarMgr::
				}
			}
		}
	}
}